#pragma once
#include <string>
#include <filesystem>

namespace kms {
	class session_t;
}

class Settings
{

	Settings();
	~Settings();
public:

	static std::filesystem::path Dir;
	static std::filesystem::path IniFile;

	static void ReadInit(kms::session_t* session);

};

