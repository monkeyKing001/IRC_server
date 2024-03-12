#ifndef FT_IRC_CLIENT_HPP
# define FT_IRC_CLIENT_HPP

class Client;

#include <vector>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "utils.hpp"

enum ClientState {
	HANDSHAKE,
	LOGIN,
	PLAY,
	DISCONNECTED
};

/*! \enum EPOLL_EVENTS_STATE
 *
 *  Clients' EPOLL EVENT activation status.
 */
enum EpollEventsState {
	EPOLL_READY_INNOUT = EPOLLIN | EPOLLOUT,
	EPOLL_READY_IN = EPOLLIN,
	EPOLL_READY_OUT = EPOLLOUT
};

#include "Channel.hpp"

class Client
{

	private:
		int _fd;
		const int _serverEpollFd;
		std::string _hostname;
		int _port;

		std::string _nickname;
		std::string _username;
		std::string _realname;

		ClientState _state;

		Channel *_channel;
		std::string _receivedMessage;
		std::string _sendBuffer;


	public:
		Client(int fd, const std::string &hostname, int port, int _serverEpollFd);
		~Client();

		int getFD() const { return _fd; };
		std::string getHostname() const { return _hostname; };
		int getPort() const { return _port; };
		bool isRegistered() const { return _state == ::PLAY; };
		std::string getNickname() const { return _nickname; };
		std::string getUsername() const { return _username; };
		std::string getRealName() const { return _realname; };
		std::string getPrefix() const;
		Channel *getChannel() const { return _channel; };
		std::string&	getReceivedMessage() { return _receivedMessage; };
		std::string& 	getSendBuffer() { return _sendBuffer; };
		void setNickname(const std::string &nickname) { _nickname = nickname; };
		void setUsername(const std::string &username) { _username = username; };
		void setRealName(const std::string &realname) { _realname = realname; };
		void setState(ClientState state) { _state = state; };
		void setChannel(Channel *channel) { _channel = channel; };
		void write(const std::string &message);
		void sendBuffer() const;
		void flushSendBuffer() { _sendBuffer = ""; }
		void reply(const std::string &reply);
		void welcome();
		void join(Channel *channel);
		void leave();
		void setEpollEventState(EpollEventsState es) const;
};

#endif
