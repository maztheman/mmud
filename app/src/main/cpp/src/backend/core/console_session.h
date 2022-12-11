#pragma once

#include "container/concurrentqueue.h"
#include "container/protected_vector.h"

#include <shared_mutex>

namespace kms {

class console_session_t
{
public:
	using ro_strings = base_protected_vector_t<std::string>::READ_ONLY_VECTOR;


	console_session_t(std::string sessionName)
	: m_sessionName(std::move(sessionName))
	{

	}

	~console_session_t();

	constexpr static bool isTTY() 
	{
		return true;
	}

	void writeText(std::string text);
	void setLocalEcho(bool bValue);
	void setInput(std::string input);
	ro_strings& readText(ro_strings& texts) const;

	const std::string& getSessionName() const 
	{
		return m_sessionName;
	}

	std::atomic_bool& scrollToBottom()
	{
		return m_scrollToBottom;
	}

	moodycamel::ConcurrentQueue<std::string> inputQueue;

private:
	std::string m_sessionName;
	base_protected_vector_t<std::string> m_texts;
	bool m_localEcho{false};
	std::string m_input;
	std::atomic_bool m_scrollToBottom{true};
};

}