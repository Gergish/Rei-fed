#include "Aimbot.h"
#include "../Vars.h"

#include "AimbotHitscan/AimbotHitscan.h"
#include "AimbotProjectile/AimbotProjectile.h"
#include "AimbotMelee/AimbotMelee.h"
#include "../Misc/Misc.h"

bool CAimbot::ShouldRun(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	// Don't run if aimbot is disabled
	//if (!Vars::Aimbot::Global::Active.Value) { return false; }

	// Don't run in menus
	if (I::EngineVGui->IsGameUIVisible() || I::VGuiSurface->IsCursorVisible())
		return false;

	// Don't run if we are frozen in place.
	if (G::Frozen)
		return false;

	if (!pLocal->IsAlive() ||
		pLocal->IsTaunting() ||
		pLocal->IsBonked() ||
		pLocal->m_bFeignDeathReady() ||
		pLocal->IsCloaked() ||
		pLocal->IsInBumperKart() ||
		pLocal->IsAGhost())
		return false;

	switch (G::CurItemDefIndex)
	{
		case Soldier_m_RocketJumper:
		case Demoman_s_StickyJumper: 
			return false;
	}

	switch (pWeapon->GetWeaponID())
	{
		case TF_WEAPON_PDA:
		case TF_WEAPON_PDA_ENGINEER_BUILD:
		case TF_WEAPON_PDA_ENGINEER_DESTROY:
		case TF_WEAPON_PDA_SPY:
		case TF_WEAPON_PDA_SPY_BUILD:
		case TF_WEAPON_BUILDER:
		case TF_WEAPON_INVIS:
		case TF_WEAPON_BUFF_ITEM:
		case TF_WEAPON_GRAPPLINGHOOK:
			return false;
	}

/*	//	weapon data check for null damage
	if (CTFWeaponInfo* sWeaponInfo = pWeapon->GetTFWeaponInfo())
	{
		WeaponData_t sWeaponData = sWeaponInfo->m_WeaponData[0];
		if (sWeaponData.m_nDamage < 1)
			return false;
	}*/

	return true;
}

bool CAimbot::Run(CUserCmd* pCmd)
{
	G::AimPos = Vec3();

	if (F::Misc.bMovementStopped || F::Misc.bFastAccel)
		return false;

	const auto pLocal = g_EntityCache.GetLocal();
	const auto pWeapon = g_EntityCache.GetWeapon();
	if (!pLocal || !pWeapon)
		return false;

	if (!ShouldRun(pLocal, pWeapon))
		return false;

	if (F::AimbotGlobal.IsSandvich())
		G::CurWeaponType = EWeaponType::PROJECTILE;

	const bool bAttacking = G::IsAttacking;
	switch (G::CurWeaponType)
	{
		case EWeaponType::HITSCAN:
			F::AimbotHitscan.Run(pLocal, pWeapon, pCmd); break;
		case EWeaponType::PROJECTILE:
			F::AimbotProjectile.Run(pLocal, pWeapon, pCmd); break;
		case EWeaponType::MELEE:
			F::AimbotMelee.Run(pLocal, pWeapon, pCmd); break;
	}
	if (!bAttacking && G::IsAttacking)
		return true;
	return false;
}
