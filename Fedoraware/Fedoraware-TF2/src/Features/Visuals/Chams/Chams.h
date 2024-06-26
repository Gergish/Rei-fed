#pragma once
#include "../../Feature.h"

class CChams
{
	Chams_t GetStruct(
		std::vector<std::string> VisibleMaterial = { "Original" },
		std::vector<std::string> OccludedMaterial = {},
		Color_t VisibleColor = { 255, 255, 255, 255 },
		Color_t OccludedColor = { 255, 255, 255, 255 }
	);
	bool GetPlayerChams(CBaseEntity* pEntity, CBaseEntity* pLocal, Chams_t* pChams, bool bFriendly, bool bEnemy);
	bool GetChams(CBaseEntity* pLocal, CBaseEntity* pEntity, Chams_t* pChams);

	void StencilBegin(IMatRenderContext* pRenderContext, bool bTwoModels = false);
	void StencilVisible(IMatRenderContext* pRenderContext, bool bTwoModels = false);
	void StencilOccluded(IMatRenderContext* pRenderContext);
	void StencilEnd(IMatRenderContext* pRenderContext, bool bTwoModels = false);

	void DrawModel(CBaseEntity* pEntity, Chams_t chams, IMatRenderContext* pRenderContext, bool bTwoModels = true);

	void RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);
	void RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

public:
	void RenderMain(CBaseEntity* pLocal);
	void RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

	bool RenderViewmodel(void* ecx, int flags, int* iReturn);
	bool RenderViewmodel(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

	bool bRendering = false;
	bool bExtra = false;

	std::unordered_map<int, bool> mEntities = {};
};

ADD_FEATURE(CChams, Chams)