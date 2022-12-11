#pragma once

#include <utility>

#ifdef _WIN32
//this header is clearly for windows
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <fmt/format.h>
#pragma comment (lib, "WS2_32.lib")

namespace kms {
	//REQUIRED: socket init singleton
	class socket_init
	{
		WSADATA m_wsaData;
		int		m_iResult;

		static socket_init m_instance;

		socket_init()
			: m_iResult(-1)
		{
			m_iResult = WSAStartup(MAKEWORD(2,2), &m_wsaData);
		}
		
		~socket_init() {
			WSACleanup();//might clean up too fast
		}

	public:

		static const socket_init& GetInstance() {
			return m_instance;
		}

		bool GetIsValid() const {
			return m_iResult == 0;
		}

	};
}

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <fmt/format.h>
#include <linux/in.h>

namespace kms {
	class socket_init
	{
		socket_init() = default;
	public:
		
		static const socket_init& GetInstance()
		{
			static socket_init instance;
			return instance;
		}

		bool GetIsValid() const {
			return true;
		}
	};
}

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define closesocket close

#define TIMEVAL timeval
#define ioctlsocket ioctl

#endif

//REQUIRED: Initialize it.
inline bool SocketInit() { return kms::socket_init::GetInstance().GetIsValid(); }

typedef SOCKET					kms_socket_t;

#define KMS_INVALID_SOCKET		INVALID_SOCKET
#define KMS_SOCKET_ERROR		SOCKET_ERROR
