#pragma once
#include "net/socket.hpp"
#include "net/telnet.hpp"
#include "container/concurrentqueue.h"
#include "core/console_session.h"
#include "util/commands.hpp"
#include "util/settings.h"

#include <iostream>
#include <thread>
#include <regex>
#include <array>

namespace kms {

	class session_t 
	{
		using lua_script_v = protected_vector_t<lua_script_t>;
		using queue_type = moodycamel::ConcurrentQueue<std::string>;

		kms::socket_t					m_socket;
		kms::telnet_t					m_telnet;
		queue_type						m_bufferedWrite;
		console_session_t				m_console;
		commands_t						m_commands;
		std::thread						m_recvThread;
		std::thread						m_sendThread;
		std::array<char, 256>			m_inputBuffer;
		//lua_script_v					m_aliases;
		std::atomic_bool				m_IsDeleted{false};
		
		
		static void readMud(session_t& session) {
			CCharVector data(4096, 0);
			size_t index = 0;
			for(;;)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
				if (!session.m_socket.getIsValid())
				{
				    break;
				}
				if (session.m_socket.checkForRead())
				{
					auto rc = session.m_socket.recv(data, index);
					if (rc > 0)
					{
						index += static_cast<size_t>(rc);
					}
					else if (rc == 0)
					{
						//connection closed??
					}
					else
					{
						fmt::print(stderr, "\r\n{}\r\n", "Connection Lost");
						break;
					}
					if (index > 0) {
						if (session.m_telnet.process(data, index, session.m_console, session.m_commands, session.m_bufferedWrite)) {
							index = 0;
							data.resize(4096, 0);
						}
					}
				}
			}
			fmt::print(stderr, "read thread closed [{}]\n", session.m_console.getSessionName());
			session.m_socket.close();
		}

		static void writeMud(session_t& session)
		{
			for(;;)
			{
				if (!session.m_socket.getIsValid())
				{
				    break;
				}
				if (session.m_socket.checkForWrite() == 0)
				{
				    continue;
				}

				//dont care about CoW...
				std::string sLine;

				if (session.m_bufferedWrite.try_dequeue(sLine))
				{
					if (sLine.empty() == false)
					{
						unsigned char nTest = static_cast<unsigned char>(sLine[0]);
						if (nTest == 255)
						{
						}
						else
						{
							sLine += "\r\n";
						}
					}
					else
					{
						sLine += "\r\n";
					}

					session.m_socket.send(sLine);
				}
			}
			fmt::print(stderr, "write thread closed [{}]\n", session.m_console.getSessionName());
		}

		static void ticker(queue_type& inputQueue, std::atomic_bool& bShouldContinue)
		{
			for(;bShouldContinue;)
			{
				for(int i = 0;bShouldContinue && i < 50; i++)
				{//its like this so on close we dont wait full 5s to die
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}

				if (bShouldContinue)
				{
					inputQueue.enqueue("");
				}
			}
			fmt::print(stderr, "[ticker] closed\n");
		}

	public:
		using ro_strings = console_session_t::ro_strings;

		session_t(std::string sessionName)
		: m_console(sessionName)
		, m_commands(m_bufferedWrite)
		{
			Settings::ReadInit(this);
		}

		~session_t() 
		{
			ResetAlias();
			if (m_recvThread.joinable())
			{
				m_recvThread.join();
			}

			if (m_sendThread.joinable())
			{
				m_sendThread.join();
			}
			fmt::print(stderr, "session_t closed [{}]\n", m_console.getSessionName());
		}

		bool connect(const std::string& sAddress, int nPort)
		{
			return m_socket.connectTcp(sAddress, nPort);
		}

		void SendToServer(std::string sText)
		{
			m_bufferedWrite.enqueue(sText);
		}

		void addInput(std::string sText)
		{
			m_console.setInput(sText);
		}

		ro_strings& readText(ro_strings& texts)
		{
			return m_console.readText(texts);
		}

		const std::string& getSessionName() const
		{
			return m_console.getSessionName();
		}

		void close()
		{
			m_console.inputQueue.enqueue("exit");
			m_commands.Close();
		}

		std::array<char, 256>& getInputBuffer()
		{
			return m_inputBuffer;
		} 

		std::atomic_bool& scrollToBottom()
		{
			return m_console.scrollToBottom();
		}

		void play()
		{
			std::thread r(readMud, std::ref(*this));
			std::thread w(writeMud, std::ref(*this));

			m_recvThread.swap(r);
			m_sendThread.swap(w);

			std::atomic_bool bShouldContinue{true};

			//std::thread tickerThread(ticker, std::ref(m_console.inputQueue), std::ref(bShouldContinue));

			std::string sInput;
			while(1) 
			{	
				if (m_console.inputQueue.try_dequeue(sInput))
				{
					if (sInput == "exit") {
						m_bufferedWrite.enqueue("quit");
						break;
					} else if (sInput == "/reload") {
						m_commands.ResetScripts();
						Settings::ReadInit(this);
						continue;
					}


					bool rc = false;
					//this is where alias should be processed man...
					/*for (auto& alias : m_aliases) {
						if (alias->OnIncoming(sInput)) {
							rc = true;
							break;
						}
					}*/
					if (rc == false) {
						m_bufferedWrite.enqueue(sInput);
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}

			bShouldContinue = false;

			std::this_thread::sleep_for(std::chrono::milliseconds(30));

			m_socket.close();

			/*if (tickerThread.joinable())
			{
				tickerThread.join();
			}*/
		}

		void AddAlias(const std::string& sCode, std::string sFunction)
		{
			//m_aliases.push_back(new lua_script_t(sCode, sFunction));
		}

		void ResetAlias()
		{
			/*
			for (auto& p : m_aliases) {
				delete p;
			}
			m_aliases.clear();
			*/
		}

		commands_t& Commands() 
		{
			return m_commands;
		}

		std::atomic_bool& isDeleted() 
		{
			return m_IsDeleted;
		}
	private:


	
	};


}
