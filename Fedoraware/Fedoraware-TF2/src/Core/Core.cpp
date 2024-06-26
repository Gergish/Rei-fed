#include "Core.h"

#include "../Hooks/HookManager.h"
#include "../Hooks/PatchManager/PatchManager.h"

#include "../Features/NetVarHooks/NetVarHooks.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Vars.h"
#include "../Features/TickHandler/TickHandler.h"

#include "../Features/Menu/Menu.h"
#include "../Features/Menu/ConfigManager/ConfigManager.h"
#include "../Features/Commands/Commands.h"

#include "../Features/Visuals/Materials/Materials.h"

#include "../Utils/Events/Events.h"
#include "../Utils/Minidump/Minidump.h"

void CCore::OnLoaded()
{
	F::ConfigManager.LoadConfig(F::ConfigManager.GetCurrentConfig(), false);
	F::Menu.ConfigLoaded = true;

	I::Cvar->ConsoleColorPrintf(Vars::Menu::Theme::Accent.Map["default"], "%s Loaded!\n", Vars::Menu::CheatName.Map["default"].c_str()); // lol
	I::MatSystemSurface->PlaySound("vo\\items\\wheatley_sapper\\wheatley_sapper_attached14.mp3");
}

void CCore::Load()
{
	g_SteamInterfaces.Init();
	g_Interfaces.Init();
	g_ConVars.Init();

	// Check the DirectX version
	const int dxLevel = g_ConVars.FindVar("mat_dxlevel")->GetInt();
	if (dxLevel < 90)
	{
		I::Cvar->ConsoleColorPrintf(Color_t(255, 0, 0, 255), "Failed to load, please remove -dxlevel from your launch arguments and try again.\n");
		MessageBoxA(nullptr, "Your DirectX version is too low!\nPlease use dxlevel 90 or higher", "Error", MB_ICONERROR);
		bHasFailed = true;
		return; // stop the loading process, we failed
	}

	// Initialize hooks & memory stuff
	g_HookManager.Init();
	g_PatchManager.Init();
	F::NetHooks.Init();

	F::Commands.Init();
	F::Ticks.Reset();
	F::Materials.ReloadMaterials();

	// all events @ https://github.com/tf2cheater2013/gameevents.txt
	g_Events.Setup({ "client_beginconnect", "client_connected", "client_disconnect", "game_newmap", "teamplay_round_start", "player_activate", "player_spawn", "player_changeclass", "player_hurt", "vote_cast", "item_pickup" });

	OnLoaded();
}

void CCore::Unload()
{
	G::UnloadWndProcHook = true;
	if (bHasFailed)
		return;

	Vars::Visuals::World::SkyboxChanger.Value = "Off";
	Vars::Visuals::ThirdPerson::Active.Value = false;

	Sleep(100);

	g_Events.Destroy();
	g_HookManager.Release();
	g_PatchManager.Restore();

	Sleep(100);

	F::Visuals.RestoreWorldModulation(); //needs to do this after hooks are released cuz UpdateWorldMod in FSN will override it
	if (auto cl_wpn_sway_interp = g_ConVars.FindVar("cl_wpn_sway_interp"))
		cl_wpn_sway_interp->SetValue(0.f);
	if (auto cl_wpn_sway_scale = g_ConVars.FindVar("cl_wpn_sway_scale"))
		cl_wpn_sway_scale->SetValue(0.f);

	I::Cvar->ConsoleColorPrintf(Vars::Menu::Theme::Accent.Value, "%s Unloaded!\n", Vars::Menu::CheatName.Value.c_str());
	I::MatSystemSurface->PlaySound("vo\\items\\wheatley_sapper\\wheatley_sapper_hacked02.mp3");
}

bool CCore::ShouldUnload()
{
	const bool unloadKey = GetAsyncKeyState(VK_F11) & 0x8000;
#ifdef _DEBUG
	return unloadKey && Utils::IsGameWindowInFocus() || bHasFailed;
#else
	return unloadKey && !F::Menu.IsOpen && Utils::IsGameWindowInFocus() || bHasFailed;
#endif // _DEBUG
}