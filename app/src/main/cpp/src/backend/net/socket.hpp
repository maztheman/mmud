#pragma once
#include <sstream>


namespace kms {
		
	class addrinfo_t 
	{
		struct addrinfo*	m_pResult;
		int					m_rc;
	public:
		addrinfo_t(const std::string& sAddress, int port, struct addrinfo& hints) 
			: m_pResult(NULL)
			, m_rc(-1)
		{
			using std::stringstream;

			std::string sPort;
			stringstream ss;
			ss << port;
			sPort = ss.str();

			m_rc = getaddrinfo(sAddress.c_str(), sPort.c_str(), &hints, &m_pResult);
		}

		~addrinfo_t() 
		{
			if (m_rc == 0) {
				freeaddrinfo(m_pResult);
				m_pResult = NULL;
			}
		}
		
		bool getIsValid() const {
			return m_rc == 0;
		}

		int getFamily() const {
			return m_pResult->ai_family;
		}
		int getSockType() const {
			return m_pResult->ai_socktype;
		}
		int getProtocol() const {
			return m_pResult->ai_protocol;
		}

		struct sockaddr * getAddr() const {
			return m_pResult->ai_addr;
		}

		auto getAddrLength() const -> auto {
			return m_pResult->ai_addrlen;
		}
	};

	class socket_t
	{
		//provided by the platform.hpp
		kms_socket_t	m_socket;
	public:
		socket_t()
			: m_socket(KMS_INVALID_SOCKET)
		{
		}

		~socket_t()
		{
			close();
		}

		void close() {
			if (getIsValid()) {
				::closesocket(m_socket);
				m_socket = KMS_INVALID_SOCKET;
			}
		}

		void setNonBlockingMode()
		{
			unsigned long val = 1;
			ioctlsocket(m_socket, FIONBIO, &val);
		}

		bool getIsValid() const {
			return m_socket != KMS_INVALID_SOCKET;
		}

		bool connectTcp(const std::string& sAddress, int port)
		{

			//straight from MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/bb530741(v=vs.85).aspx
			struct addrinfo hints;

			memset(&hints, 0, sizeof(hints));

			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			addrinfo_t addr(sAddress, port, hints);

			if (!addr.getIsValid()) {
				//log_error("socket_t::Connect, getaddrinfo failed: %ld", rc);
				return false;
			}

			m_socket = socket(addr.getFamily(), addr.getSockType(), addr.getProtocol());

			if (!getIsValid()) {
				return false;
			}

			int rc = connect(m_socket, addr.getAddr(), addr.getAddrLength());
			if (rc == KMS_SOCKET_ERROR) {
				close();
			}

			return getIsValid();
		}

		auto recv(CCharVector& data, size_t index = 0) -> auto {
			memset(&data[index], 0, data.size() - index);
			return ::recv(m_socket, &data[index], data.size() - index, 0);
		}

		bool send(const std::string& buffer)
		{
			return KMS_SOCKET_ERROR != ::send(m_socket, buffer.c_str(), buffer.size(), 0);
		}

		int send(const CCharVector& buffer)
		{
			return static_cast<int>(::send(m_socket, &buffer[0], buffer.size(), 0));
		}

		int checkForError()
		{
			fd_set hum;
			TIMEVAL wait = {0, 0};
			FD_ZERO(&hum);
			FD_SET(m_socket, &hum);
			::select(32, NULL, NULL, &hum, &wait);
			return FD_ISSET(m_socket, &hum);
		}

		int checkForRead()
		{
			fd_set hum;
			TIMEVAL wait = {0, 0};
			FD_ZERO(&hum);
			FD_SET(m_socket, &hum);
			::select(32, &hum, NULL, NULL, &wait);
			return FD_ISSET(m_socket, &hum);
		}

		int checkForWrite()
		{
			fd_set hum;
			TIMEVAL wait = {0, 0};
			FD_ZERO(&hum);
			FD_SET(m_socket, &hum);
			::select(32, NULL, &hum, NULL, &wait);
			return FD_ISSET(m_socket, &hum);
		}
	};
}