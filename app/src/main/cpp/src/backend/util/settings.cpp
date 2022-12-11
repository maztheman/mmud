#include "settings.h"

#include "core/session.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

#include <yaml-cpp/yaml.h>

std::filesystem::path Settings::Dir;
std::filesystem::path Settings::IniFile;

Settings::Settings()
{
}


Settings::~Settings()
{
}

static std::string ReadToFile(std::filesystem::path file)
{
	std::ifstream t(file);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	return str;
}

void Settings::ReadInit(kms::session_t* session)
{

	std::ifstream t(IniFile);
	YAML::Node doc = YAML::Load(t);


	const auto& val =  doc["incoming"];
	if (val.IsSequence())
	{
		for(auto& scripts : val)
		{
			session->Commands().AddScripts(session, ReadToFile(Dir / scripts.as<std::string>()), "incoming");
		}
	}
	

	/*std::fstream ini(IniFile, std::ios::in);
	if (!ini) {
		return;
	}

	session->ResetAlias();
	session->Commands().ResetScripts();

	std::string line;
	while (std::getline(ini, line)) {
		std::stringstream ss(line);
		std::string verb,verb_value, file, function;
		std::getline(ss, verb, '=');
		std::getline(ss, verb_value, '=');
		std::stringstream verbSS(verb_value);
		std::getline(verbSS, file, ',');
		std::getline(verbSS, function, ',');
		if (verb == "alias") {
			//session->AddAlias(ReadToFile(Dir / file), function);
		} else if (verb == "incoming") {
			session->Commands().AddScripts(session, ReadToFile(Dir / file), function);
		}
	}*/

}
