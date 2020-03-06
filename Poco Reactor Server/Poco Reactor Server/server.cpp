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

const Poco::UInt16 PORT = 7777;

class Session {
public:
	Session(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor) :
		m_socket(socket),
		m_reactor(reactor)
	{
		m_peerAddress = socket.peerAddress().toString();
		std::cout << "connection from " << m_peerAddress << " ..." << std::endl;
		m_reactor.addEventHandler(m_socket,
			Poco::Observer<Session, Poco::Net::ReadableNotification>(*this, &Session::onReadable));
	}

	~Session()
	{
		std::cout << m_peerAddress << " disconnected ..." << std::endl;
		m_reactor.removeEventHandler(m_socket,
			Poco::Observer<Session, Poco::Net::ReadableNotification>(*this, &Session::onReadable)
		);
	}

	void onReadable(Poco::Net::ReadableNotification* pNf)
	{
		pNf->release();

		try
		{
			char buffer[256] = { 0, };
			int n = m_socket.receiveBytes(buffer, sizeof(buffer));
			if (n > 0)
			{
				std::cout << "Ŭ���̾�Ʈ���� ���� �޽���: " << buffer << std::endl;

				char szSendMessage[256] = { 0, };
				sprintf_s(szSendMessage, 256 - 1, "Re:%s", buffer);
				int nMsgLen = (int)strnlen_s(szSendMessage, 256 - 1);

				m_socket.sendBytes(szSendMessage, nMsgLen);
			}
			else
			{
				m_socket.shutdownSend();
				delete this;  // �޸� �������� ������ �Ҹ��ڰ� ȣ����� �ʴ´�.
			}
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "EchoServer: " << exc.displayText() << std::endl;

			m_socket.shutdown();
			delete this;
		}
	}


private:
	Poco::Net::StreamSocket   m_socket;
	std::string m_peerAddress;
	Poco::Net::SocketReactor& m_reactor;
};


int main()
{
	Poco::Net::ServerSocket serverSocket(PORT);
	Poco::Net::SocketReactor reactor;
	Poco::Net::SocketAcceptor<Session> acceptor(serverSocket, reactor);

	std::cout << "Reactor: " << "starting..." << std::endl;
	reactor.run();

	return 0;
}