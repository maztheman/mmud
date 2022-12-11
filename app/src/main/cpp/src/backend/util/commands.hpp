#pragma once

#include "container/concurrentqueue.h"
#include "container/protected_vector.h"

#include "lua_script.h"

#include <regex>
#include <map>
#include <thread>


namespace kms {
	class commands_t 
	{
		using queue_type = moodycamel::ConcurrentQueue<std::string>;
		using new_queue_type = moodycamel::ConcurrentQueue<std::string>;
		using map_type = std::map<std::string, std::string>;
		using lua_script_v = protected_vector_t<lua_script_t>;
		using out_script_v = std::vector<lua_script_t*>;

		std::string GetSimpleMob(std::string sMob)
		{
			auto itc = std::find(sMob.begin(), sMob.end(), ' ');
			return (itc != sMob.end()) ? std::string(sMob.begin(), itc) : sMob;
		}

		void OnIncoming(std::string sIncoming);

	public:
		commands_t(queue_type& bufferedWrite)
			: m_bufferedWrite(bufferedWrite)
			, m_bufferedIncoming(100U)
		{
			const auto func = [this]() {
				std::string sText;
				sText.reserve(4096);
				for (;m_Continue;) {
					if (m_bufferedIncoming.try_dequeue(sText)) {
						OnIncoming(sText);
					}
				}
			};

			std::thread th(func);
			m_tIncoming.swap(th);
		}

		~commands_t()
		{
			ResetScripts();
			m_Continue = false;
			if (m_tIncoming.joinable())
			{
				m_tIncoming.join();
			}
		}

		map_type& Variables() 
		{
			return m_variables;
		}

		void AddScripts(session_t* session, const std::string& sCode, std::string sFunction)
		{
			m_IncomingScripts.add(std::make_unique<lua_script_t>(sCode, sFunction, session));
		}

		void ResetScripts()
		{
			m_IncomingScripts.clear();
		}

		void AddIncoming(std::string sText) 
		{
			m_bufferedIncoming.enqueue(sText);
		}

		void Close()
		{
			m_Continue = false;
		}
	private:
		queue_type&	m_bufferedWrite;
		map_type	m_variables;
		lua_script_v m_IncomingScripts;
		std::thread m_tIncoming;
		new_queue_type	m_bufferedIncoming;
		std::atomic_bool m_Continue{true};
	};
}
