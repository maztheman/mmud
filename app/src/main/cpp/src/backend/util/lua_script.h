#pragma once
#include <string>

#include <sol/sol.hpp>

#include <random>

namespace kms {

class session_t;


struct session_wrapper
{
	session_wrapper(session_t* session)
	: mSession(session)
	{

	}

	void serverSend(std::string text);
	void setVariable(std::string var, std::string val);
	std::string getVariable(std::string var);
	int32_t getRandomNumber(int32_t first, int32_t last);
	void printString(std::string text);

	session_t* mSession;
	std::mt19937 mRando;
};

class lua_script_t
{
	sol::state  mState;
	std::string mFunction;
	session_wrapper  mSessionWrapper;
public:
	lua_script_t(const std::string& sCode, std::string sFunction, session_t* session);
	~lua_script_t();

	bool OnIncoming(std::string text);
};

}