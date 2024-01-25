#pragma once

#include "../../../Utils/Color/Color.h"

#include <filesystem>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

class CConfigManager
{
	void SaveJson(const char* name, bool val);
	void SaveJson(const char* name, const std::string& val);
	void SaveJson(const char* name, int val);
	void SaveJson(const char* name, float val);
	void SaveJson(const char* name, const Color_t& val);
	void SaveJson(const char* name, const Gradient_t& val);
	void SaveJson(const char* name, const Vec3& val);
	void SaveJson(const char* name, const Chams_t& val);
	void SaveJson(const char* name, const Glow_t& val);
	void SaveJson(const char* name, const DragBox_t& val);
	void SaveJson(const char* name, const WindowBox_t& val);

	void LoadJson(const char* name, std::string& val);
	void LoadJson(const char* name, bool& val);
	void LoadJson(const char* name, int& val);
	void LoadJson(const char* name, float& val);
	void LoadJson(const char* name, Color_t& val);
	void LoadJson(const char* name, Gradient_t& val);
	void LoadJson(const char* name, Vec3& val);
	void LoadJson(const char* name, Chams_t& val);
	void LoadJson(const char* name, Glow_t& val);
	void LoadJson(const char* name, DragBox_t& val);
	void LoadJson(const char* name, WindowBox_t& val);

	std::string CurrentConfig = "default";
	std::string CurrentVisuals = "default";
	std::string ConfigPath;
	std::string VisualsPath;

public:
	const std::string ConfigExtension = ".fw";

	CConfigManager();
	bool SaveConfig(const std::string& configName, bool bNotify = true);
	bool LoadConfig(const std::string& configName, bool bNotify = true);
	bool SaveVisual(const std::string& configName, bool bNotify = true);
	bool LoadVisual(const std::string& configName, bool bNotify = true);
	void RemoveConfig(const std::string& configName);
	void RemoveVisual(const std::string& configName);

	boost::property_tree::ptree ColorToTree(const Color_t& color);
	void TreeToColor(const boost::property_tree::ptree& tree, Color_t& out);
	boost::property_tree::ptree VecToTree(const Vec3& vec);
	void TreeToVec(const boost::property_tree::ptree& tree, Vec3& out);

	const std::string GetCurrentConfig() { return CurrentConfig; }
	const std::string GetCurrentVisuals() { return CurrentVisuals; }
	const std::string& GetConfigPath() { return ConfigPath; }
	const std::string& GetVisualsPath() { return VisualsPath; }
};

inline CConfigManager g_CFG;