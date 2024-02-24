#include "FakeLag.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"

bool CFakeLag::IsAllowed(CBaseEntity* pLocal)
{
	const int iMaxSend = std::min(24 - G::ShiftedTicks, 22);
	const bool bVar = Vars::CL_Move::FakeLag::Enabled.Value || bPreservingBlast || bUnducking;
	const bool bChargePrio = (iMaxSend > 0 && G::ChokeAmount < iMaxSend) || !G::ShiftedTicks;
	const bool bAttacking = G::IsAttacking && Vars::CL_Move::FakeLag::UnchokeOnAttack.Value;
	const bool bNotAir = Vars::CL_Move::FakeLag::Options.Value & (1 << 2) && !pLocal->OnSolid();

	if (!bVar || !bChargePrio || bAttacking || bNotAir)
		return false;

	if (bPreservingBlast || bUnducking)
		return true;

	if (G::ShiftedGoal != G::ShiftedTicks)
		return false;
	
	const bool bMoving = !(Vars::CL_Move::FakeLag::Options.Value & (1 << 0)) || pLocal->m_vecVelocity().Length2D() > 10.f;
	if (!bMoving)
		return false;

	if (Vars::CL_Move::FakeLag::Mode.Value == 1 && !F::KeyHandler.Down(Vars::CL_Move::FakeLag::Key.Value))
		return false;

	switch (Vars::CL_Move::FakeLag::Type.Value) 
	{
	case FL_Plain:
	case FL_Random:
		return G::ChokeAmount < G::ChokeGoal;
	case FL_Adaptive:
	{
		const Vec3 vDelta = vLastPosition - pLocal->m_vecOrigin();
		return vDelta.Length2DSqr() < 4096.f;
	}
	}

	return false;
}

void CFakeLag::PreserveBlastJump()
{
	bPreservingBlast = false;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost() || !pLocal->IsPlayer() || G::ShiftedTicks == G::MaxShift)
		return;

	const bool bVar = Vars::CL_Move::FakeLag::RetainBlastJump.Value && Vars::Misc::AutoJump.Value; // don't bother if we aren't bhopping
	static bool bOldSolid = false; const bool bPlayerReady = pLocal->OnSolid() || bOldSolid; bOldSolid = pLocal->OnSolid();
	const bool bCanPreserve = pLocal->m_iClass() == ETFClass::CLASS_SOLDIER && pLocal->m_nPlayerCondEx2() & TFCondEx2_BlastJumping;
	const bool bValid = G::Buttons & IN_JUMP && !pLocal->IsDucking();

	bPreservingBlast = bVar && bPlayerReady && bCanPreserve && bValid;
}

void CFakeLag::Unduck(CUserCmd* pCmd)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost())
		return;

	const bool bVar = Vars::CL_Move::FakeLag::Options.Value & (1 << 1);
	const bool bPlayerReady = pLocal->IsPlayer() && pLocal->OnSolid() && pLocal->IsDucking() && !(pCmd->buttons & IN_DUCK);

	bUnducking = bVar && bPlayerReady;
}

void CFakeLag::Prediction(CUserCmd* pCmd)
{
	PreserveBlastJump();
	Unduck(pCmd);
}

void CFakeLag::Run(CUserCmd* pCmd, bool* pSendPacket)
{
	if (Vars::CL_Move::FakeLag::Mode.Value == 2 && F::KeyHandler.Pressed(Vars::CL_Move::FakeLag::Key.Value))
		Vars::CL_Move::FakeLag::Enabled.Value = !Vars::CL_Move::FakeLag::Enabled.Value;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal)
		return;

	Prediction(pCmd);

	// Set the selected choke amount (if not random)
	switch (Vars::CL_Move::FakeLag::Type.Value)
	{
	case FL_Plain: G::ChokeGoal = Vars::CL_Move::FakeLag::Value.Value; break;
	case FL_Random: if (!G::ChokeGoal) G::ChokeGoal = Utils::RandIntSimple(Vars::CL_Move::FakeLag::Min.Value, Vars::CL_Move::FakeLag::Max.Value); break;
	case FL_Adaptive: G::ChokeGoal = 22; break;
	}

	// Are we even allowed to choke?
	if (!IsAllowed(pLocal))
	{
		vLastPosition = pLocal->m_vecOrigin();
		G::ChokeAmount = G::ChokeGoal = 0;
		iAirTicks = 0;
		bUnducking = false;
		return;
	}

	*pSendPacket = false;
	G::ChokeAmount++;

	if (!pLocal->OnSolid())
		iAirTicks++;
}