#include "TickHandler.h"

#include "../../Hooks/HookManager.h"
#include "../../Hooks/Hooks.h"
#include "../NetworkFix/NetworkFix.h"
#include "../PacketManip/AntiAim/AntiAim.h"

void CTickshiftHandler::Reset()
{
	bSpeedhack = G::DoubleTap = G::Recharge = G::Warp = false;
	G::ShiftedTicks = G::ShiftedGoal = 0;
}

void CTickshiftHandler::Recharge(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	G::Recharge = false;
	bool bPassive = false;

	static float iPassiveTick = 0.f;
	if (Vars::CL_Move::Doubletap::PassiveRecharge.Value && I::GlobalVars->tickcount >= iPassiveTick)
	{
		bPassive = true;
		iPassiveTick = I::GlobalVars->tickcount + 67 - Vars::CL_Move::Doubletap::PassiveRecharge.Value;
	}

	if (iDeficit && G::ShiftedTicks < G::MaxShift)
	{
		bPassive = true;
		iDeficit--;
	}
	else if (iDeficit)
		iDeficit = 0;

	if (!Vars::CL_Move::Doubletap::RechargeTicks.Value && !bPassive
		|| G::DoubleTap || G::Warp || G::ShiftedTicks == G::MaxShift || bSpeedhack)
		return;

	G::Recharge = true;
	if (bGoalReached)
		G::ShiftedGoal = G::ShiftedTicks + 1;
}

void CTickshiftHandler::Teleport(CUserCmd* pCmd)
{
	G::Warp = false;
	if (!G::ShiftedTicks || G::DoubleTap || G::Recharge || bSpeedhack)
		return;

	G::Warp = Vars::CL_Move::Doubletap::Warp.Value;
	if (G::Warp && bGoalReached)
		G::ShiftedGoal = std::max(G::ShiftedTicks - Vars::CL_Move::Doubletap::WarpRate.Value + 1, 0);
}

void CTickshiftHandler::Doubletap(const CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (G::ShiftedTicks < std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift)
		|| G::WaitForShift || G::Warp || G::Recharge || bSpeedhack
		|| !G::CanPrimaryAttack || (G::CurWeaponType == EWeaponType::MELEE ? !(pCmd->buttons & IN_ATTACK) : !G::IsAttacking) || G::RocketJumping)
		return;

	G::DoubleTap = Vars::CL_Move::Doubletap::Doubletap.Value;
	if (G::DoubleTap && Vars::CL_Move::Doubletap::AntiWarp.Value)
		G::AntiWarp = true;
	if (G::DoubleTap && bGoalReached)
		G::ShiftedGoal = G::ShiftedTicks - std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift);
}

int CTickshiftHandler::GetTicks(CBaseEntity* pLocal)
{
	if (G::DoubleTap && G::ShiftedGoal < G::ShiftedTicks)
		return G::ShiftedTicks - G::ShiftedGoal;

	if (G::ShiftedTicks < std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift)
		|| G::WaitForShift || G::Warp || G::Recharge || bSpeedhack || G::RocketJumping
		|| !Vars::CL_Move::Doubletap::Doubletap.Value)
		return 0;

	return std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift);
}

bool CTickshiftHandler::ValidWeapon(CBaseCombatWeapon* pWeapon)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_BUILDER:
	case TF_WEAPON_INVIS:
	case TF_WEAPON_GRAPPLINGHOOK:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_LUNCHBOX:
	case TF_WEAPON_BUFF_ITEM:
	case TF_WEAPON_ROCKETPACK:
	case TF_WEAPON_JAR_GAS:
	case TF_WEAPON_LASER_POINTER:
	case TF_WEAPON_MEDIGUN:
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_JAR:
		return false;
	}

	return true;
}

void CTickshiftHandler::Speedhack(CUserCmd* pCmd)
{
	bSpeedhack = Vars::CL_Move::SpeedEnabled.Value;
	if (!bSpeedhack)
		return;

	G::DoubleTap = G::Warp = G::Recharge = false;
}

void CTickshiftHandler::CLMoveFunc(float accumulated_extra_samples, bool bFinalTick)
{
	static auto CL_Move = g_HookManager.GetMapHooks()["CL_Move"];
	if (!CL_Move)
	{
		CL_Move = g_HookManager.GetMapHooks()["CL_Move"];
		return;
	}

	G::ShiftedTicks--;
	if (G::ShiftedTicks < 0)
		return;
	if (G::WaitForShift > 0)
		G::WaitForShift--;

	if (G::ShiftedTicks < std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift))
		G::WaitForShift = 1;

	bGoalReached = bFinalTick && G::ShiftedTicks == G::ShiftedGoal;

	CL_Move->Original<void(__cdecl*)(float, bool)>()(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::MoveMain(float accumulated_extra_samples, bool bFinalTick)
{
	if (auto pWeapon = g_EntityCache.GetWeapon())
	{
		const int iWeaponID = pWeapon->GetWeaponID();
		if (iWeaponID != TF_WEAPON_PIPEBOMBLAUNCHER && iWeaponID != TF_WEAPON_CANNON)
		{
			if (!ValidWeapon(pWeapon))
				G::WaitForShift = 2;
			else if (G::IsAttacking || !G::CanPrimaryAttack || pWeapon->IsInReload())
				G::WaitForShift = Vars::CL_Move::Doubletap::TickLimit.Value;
		}
	}
	else
		G::WaitForShift = 2;

	static auto sv_maxusrcmdprocessticks = g_ConVars.FindVar("sv_maxusrcmdprocessticks");
	G::MaxShift = sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24;
	if (F::AntiAim.AntiAimOn())
		G::MaxShift -= 3;

	while (G::ShiftedTicks > G::MaxShift)
		CLMoveFunc(accumulated_extra_samples, false); // skim any excess ticks

	G::ShiftedTicks++; // since we now have full control over CL_Move, increment.
	if (G::ShiftedTicks <= 0)
	{
		G::ShiftedTicks = 0;
		return;
	}

	if (bSpeedhack)
	{
		for (int i = 0; i < Vars::CL_Move::SpeedFactor.Value; i++)
			CLMoveFunc(accumulated_extra_samples, i == Vars::CL_Move::SpeedFactor.Value);
		return;
	}

	G::ShiftedGoal = std::clamp(G::ShiftedGoal, 0, G::MaxShift);
	if (G::ShiftedTicks > G::ShiftedGoal) // normal use/doubletap/teleport
	{
		while (G::ShiftedTicks > G::ShiftedGoal)
			CLMoveFunc(accumulated_extra_samples, G::ShiftedTicks - 1 == G::ShiftedGoal);
		G::AntiWarp = false;
		if (G::Warp)
			iDeficit = 0;

		G::DoubleTap = G::Warp = false;
	}
	// else recharge
}

void CTickshiftHandler::MovePre(CBaseEntity* pLocal)
{
	CUserCmd* pCmd = G::CurrentUserCmd;
	if (!pLocal || !pCmd)
		return;

	Recharge(pCmd, pLocal);
	Teleport(pCmd);
	Speedhack(pCmd);
}

void CTickshiftHandler::MovePost(CBaseEntity* pLocal, CUserCmd* pCmd)
{
	if (!pLocal)
		return;

	Doubletap(pCmd, pLocal);
}

void CTickshiftHandler::Run(float accumulated_extra_samples, bool bFinalTick, CBaseEntity* pLocal)
{
	F::NetworkFix.FixInputDelay(bFinalTick);

	MovePre(pLocal);
	MoveMain(accumulated_extra_samples, bFinalTick);
}