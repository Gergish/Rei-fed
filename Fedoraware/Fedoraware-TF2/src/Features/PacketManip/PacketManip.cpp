#include "PacketManip.h"
#include "../Visuals/FakeAngle/FakeAngle.h"

bool CPacketManip::WillTimeOut()
{
	return I::ClientState->chokedcommands >= 21;
}

bool CPacketManip::AntiAimCheck(CBaseEntity* pLocal)
{
	if (!pLocal)
		return false;

	if ((G::DoubleTap || G::Warp) && G::ShiftedTicks == G::ShiftedGoal)
		return false;

	return I::ClientState->chokedcommands < 3 && F::AntiAim.ShouldRun(pLocal);
}

void CPacketManip::Run(CBaseEntity* pLocal, CUserCmd* pCmd, bool* pSendPacket)
{
	F::FakeAngle.DrawChams = Vars::CL_Move::Fakelag::Fakelag.Value || F::AntiAim.AntiAimOn();

	*pSendPacket = true;
	const bool bTimeout = WillTimeOut(); // prevent overchoking by just not running anything below if we believe it will cause us to time out

	if (!bTimeout)
		F::FakeLag.Run(pLocal, pCmd, pSendPacket);
	else
		G::ChokeAmount = 0;

	if (!bTimeout && AntiAimCheck(pLocal) && !G::PSilentAngles)
		*pSendPacket = false;
}