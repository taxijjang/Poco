#include <iostream>
//#include <cstdlib>

#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketNotification.h>
#include <Poco/Net/SocketConnector.h>
#include <Poco/Net/SocketAcceptor.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Observer.h>
#include<Poco\Net\SocketConnector.h>

const Poco::UInt16 PORT = 7777;

class Session {
public:
	Session(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor) :
		m_socket(socket), m_reactor(reactor) {
		std::cout << "이름을 입력 해주세요 : "; std::cin >> UserName;
		std::cout << "connection success" << std::endl;

		m_reactor.addEventHandler(m_socket,
			Poco::Observer<Session,
			Poco::Net::ReadableNotification>(
				*this, &Session::onReadable));

		SendMessage();
	}

	~Session() {
		try {
			std::cout << "~Session" << std::endl;
			m_reactor.removeEventHandler(m_socket,
				Poco::Observer<Session,
				Poco::Net::ReadableNotification>(
					*this, &Session::onReadable)
			);
			m_reactor.stop();
		}
		catch (Poco::Exception& exc) {
			std::cerr << exc.displayText() << std::endl;
		}
	}
	void onReadable(Poco::Net::ReadableNotification* pNf) {
		pNf->release();
		char buffer[256] = { 0, };
		int n = m_socket.receiveBytes(buffer, sizeof(buffer));

		if (n > 0) {
			std::cout << "서버에서 받은 메시지: " << buffer << std::endl;

			if (m_SendCount < 7) {
				++m_SendCount;
				SendMessage();
			}
			else {
				std::cout << "서버와 접속을 끊는다" << std::endl;
				m_socket.close();
				delete this;
			}
		}
		else {
			std::cout << "onReadable disconnected..." << std::endl;
			delete this;
		}
	}

	void SendMessage() {
		char szMessage[256] = { 0, };
		scanf("%s", szMessage);
		sprintf_s(szMessage, 256 - 1, szMessage);
		int nMsgLen = (int)strnlen_s(szMessage, 256 - 1);

		m_socket.sendBytes(szMessage, nMsgLen);
		std::cout << "서버에 보낸 메시지: " << szMessage << std::endl;
	}

private:
	Poco::Net::StreamSocket m_socket;
	std::string m_peerAddress;
	Poco::Net::SocketReactor& m_reactor;
	std::string UserName;
	int m_SendCount = 1;
};

int main(void) {
	Poco::Net::SocketReactor reactor;
	Poco::Net::SocketAddress sa("localhost", PORT);
	Poco::Net::SocketConnector<Session> connector(sa, reactor);

	std::cout << "To Server: " << "connecting ... " << std::endl;
	reactor.run();

	std::cout << "클라이언트: 종료..." << std::endl;
	getchar();
	return 0;
}