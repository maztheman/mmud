#include "lua_script.h"

#include "core/session.hpp"
#include "settings.h"

#include <fmt/format.h>

#include <string_view>

using namespace std::string_view_literals;


static constexpr std::string_view sBaseLibrary = R"gxx(
jmc = {
	send        = function(text) 
		kms_server_send(text) 
	end,
	setVariable = function(vr, val) 
		kms_set_variable(vr, val) 
	end,
	getVariable = function(vr) 
		return kms_get_variable(vr) 
	end,
	getRandom   = function(f,l) 
		return kms_get_random(f,l) 
	end,
	print		= function(text) 
		kms_print(text) 
	end
}
)gxx"sv;

namespace kms {

void session_wrapper::serverSend(std::string text)
{
	fmt::print(stderr, "[SCRIPT:serverSend] [{}]\n", text);
	mSession->SendToServer(text);
}

void session_wrapper::setVariable(std::string var, std::string val)
{
	fmt::print(stderr, "[SCRIPT:SetVariable] {} = {}\n", var, val);
	mSession->Commands().Variables()[var] = val;
}

std::string session_wrapper::getVariable(std::string var)
{
	auto val = mSession->Commands().Variables()[var];
	fmt::print(stderr, "[SCRIPT:GetVariable] {} = {}\n", var, val);
	return val;
}

int32_t session_wrapper::getRandomNumber(int32_t first, int32_t last)
{
	std::uniform_int_distribution<int32_t> gen(first, last);
	int32_t retval = gen(mRando);
	fmt::print(stderr, "[SCRIPT:GetRandomNumber({},{})] -> {}\n", first, last, retval);
	return retval;
}

void session_wrapper::printString(std::string text)
{
	fmt::print("[client]: {}\n", text);
}

lua_script_t::lua_script_t(const std::string& sCode, std::string sFunction, session_t* session)
	: mFunction(sFunction)
	, mSessionWrapper(session)
{
	mState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::package, sol::lib::table);
	mState.script(sBaseLibrary);
	mState.set_function("kms_server_send", &session_wrapper::serverSend, mSessionWrapper);
	mState.set_function("kms_set_variable", &session_wrapper::setVariable, mSessionWrapper);
	mState.set_function("kms_get_variable", &session_wrapper::getVariable, mSessionWrapper);
	mState.set_function("kms_get_random", &session_wrapper::getRandomNumber, mSessionWrapper);
	mState.set_function("kms_print", &session_wrapper::printString, mSessionWrapper);

	try
	{
		mState.script(sCode);
	}
	catch(const std::exception& e)
	{
		fmt::print(stderr, "Fatal Error in script: {}\n", e.what());
	}
}


lua_script_t::~lua_script_t()
{
	
}

static std::string& trimFront(std::string& s)
{
	while(!s.empty() && isspace(s[0]))
	{
		s.erase(s.begin());
	}
	return s;
}

static std::string& trimBack(std::string& s)
{
	while(!s.empty() && isspace(s.back()))
	{
		s.pop_back();
	}
	return s;
}

static std::string& trimAll(std::string& s)
{
	return trimFront(trimBack(s));
}

bool lua_script_t::OnIncoming(std::string text)
{
	try
	{
		if (!trimAll(text).empty())
		{
			sol::function func = mState[mFunction];
			int retval = func(text);
			return retval != 0;
		}
	}
	catch(const std::exception& e)
	{
		fmt::print(stderr, "Fatal Error onIncoming {}\n", e.what());
	}
	
	
/*
	lua_getglobal(mState, mFunction.c_str());
	lua_pushstring(mState, text.c_str());
	int result = lua_pcall(mState, 1, LUA_MULTRET, 0);
	if (result != 0) {
		std::string error = lua_tostring(mState, -1);
		fmt::print(stderr, "{}\n", error);
	}
	int retval = static_cast<int>(lua_tonumber(mState, -1));
	lua_pop(mState, 1);*/
	return false;
}

}