#ifndef FT_IRC_SERVER_HPP
# define FT_IRC_SERVER_HPP

class Server;

#include <algorithm>
#include <sys/poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>

#include "commands/CommandHandler.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#define MAX_CONNECTIONS 1000

class Server
{
	typedef std::vector<pollfd>::iterator pollfds_iterator;
	typedef std::map<int, Client *>::iterator clients_iterator;
	typedef std::vector<Channel *>::iterator channels_iterator;

	int							_running;
	int							_sock; //listener socket
	const std::string			_host;
	const std::string			_port;
	const std::string			_password;
	std::vector<pollfd>			_pollfds;
	int 						_epollfd;
	struct epoll_event *		_ev;
	std::vector<epoll_event>    _evs;
	
	std::map<int, Client *>		_clients;
	std::vector<Channel *>		_channels;
	CommandHandler*				_commandHandler;

public:
	Server(const std::string &port, const std::string &password);
	~Server();

	void start();

	std::string getPassword() const { return _password; };
	Client *getClient(const std::string &nickname);

	Channel* getChannel(const std::string &name);
	Channel* createChannel(const std::string &name, const std::string &password, Client *client);

	int newSocket();
	void onClientDisconnect(int fd);
	void onClientConnect();
	void onClientMessage(int fd);
	int	 readMessage(int fd);
	const std::string getServername() { return this->_host; }; //sungjuki
															   
	int addEvent(int fd);
	int delEvent(int fd);
};

#endif
