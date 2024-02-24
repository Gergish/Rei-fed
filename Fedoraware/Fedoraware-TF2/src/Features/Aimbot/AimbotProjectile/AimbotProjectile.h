#pragma once
#include "../../Feature.h"

#include "../AimbotGlobal/AimbotGlobal.h"

class CAimbotProjectile
{
	struct Solution_t
	{
		float m_flPitch = 0.0f;
		float m_flYaw = 0.0f;
		float m_flTime = 0.0f;
	};

	// new funcs
	std::vector<Target_t> GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);
	std::vector<Target_t> SortTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);

	int GetHitboxPriority(int nHitbox, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target);
	bool CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, const float flVelocity, const float flGravity, CBaseCombatWeapon* pWeapon, Solution_t& out);
	bool TestAngle(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target, const Vec3& vOriginal, const Vec3& vPredict, const Vec3& vAngles, const int& iSimTime);
	int CanHit(Target_t& target, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, float* flTimeTo, std::vector<DrawBox>* bBoxes);
	bool RunMain(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);

	//bool GetSplashTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& outTarget); implement splash

	void Aim(CUserCmd* pCmd, Vec3& vAngle);
	Vec3 Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod = Vars::Aimbot::Projectile::AimMethod.Value);

	bool bLastTickHeld = false;
	bool bLastTickReload = false;
	bool bFlameThrower = false;

public:
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);

	int bLastTickCancel = 0;
};

ADD_FEATURE(CAimbotProjectile, AimbotProjectile)