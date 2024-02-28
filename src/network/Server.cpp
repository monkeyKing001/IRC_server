#include "network/Server.hpp"
#include <cerrno>
#include <netinet/in.h>
#include <sys/epoll.h>

Server::Server(const std::string &port, const std::string &password)
		: _running(1), _host("127.0.0.1"), _port(port), _password(password) {

	_sock = newSocket();
	_commandHandler = new CommandHandler(this);
}

Server::~Server() {
	delete _commandHandler;
}

void Server::start() {

	addEvent(_sock);
	ft_log("Server listening...");

	while (_running) {
		/*
		 * epoll 
		 *
		 **/
		int epoll_num = epoll_wait(_epollfd, _ev, MAX_CONNECTIONS, 0);
		if (epoll_num == -1){
			throw std::runtime_error("Error while event polling from epollfd.");
			//error
		}
		for (int i = 0; i < epoll_num; i++) {
			//Found data in READ BUFFER.
			if ((_ev[i].events & EPOLLIN) == EPOLLIN){
				//connection
				if (_ev[i].data.fd == _sock)
					onClientConnect();
				//message recv
				else if (_ev[i].data.fd != _sock)
					onClientMessage(_ev[i].data.fd);
			}
				//passive disconnection
			else if ((_ev[i].events & EPOLLHUP) == EPOLLHUP)
				onClientDisconnect(_ev[i].data.fd);
				break;
			}
		}
		//system("leaks ircserv");
}

int Server::newSocket() {

	if ((_epollfd = epoll_create(1)) == -1)
		throw std::runtime_error("Error while opening epoll fd");

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		throw std::runtime_error("Error while opening socket.");

	// Forcefully attaching socket to the port
	int val = 1;
	//SO_REUSEADDR -> optiized address diospension policy for frequent conn/disconn sockets.
	//SOLSOCKET -> SOCKET_LEVEL option manipulations.
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)))
		throw std::runtime_error("Error while setting socket options.");

	/*
	 * As requested from subject we set the socket to NON-BLOCKING mode
	 * allowing it to return any data that the system has in it's read buffer
	 * for that socket, but, it won't wait for that data.
	 */
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
		throw std::runtime_error("Error while setting socket to NON-BLOCKING.");
	}

	struct sockaddr_in serv_address = {};

	// Clear address structure, should prevent some segmentation fault and artifacts
	bzero((char *) &serv_address, sizeof(serv_address));

	/*
	 * htons() convert unsigned short int to big-endian network byte order as expected from TCP protocol standards
	 */
	serv_address.sin_family = AF_INET;
	serv_address.sin_addr.s_addr = INADDR_ANY; //all address of local host
	//serv_address.sin_port = htons(std::stoi(_port));
	serv_address.sin_port = htons(atoi(_port.c_str())); // !!! _port == 123abc (?)

	// Bind the socket to the current IP address on selected port
	if (bind(sockfd, (struct sockaddr *) &serv_address, sizeof(serv_address)) < 0)
		throw std::runtime_error("Error while binding socket.");

	// Let socket be able to listen for requests, and backlog connection waiting queue
	if (listen(sockfd, MAX_CONNECTIONS) < 0)
		throw std::runtime_error("Error while listening on socket.");

	//static size
	if ((_ev = (epoll_event *)calloc(MAX_CONNECTIONS, sizeof(struct epoll_event))) == NULL)
		throw std::runtime_error("Error while creating epoll event struct.");
	return sockfd;
}

int Server::readMessage(int fd) 
{
	int n;
	Client* client;
	std::map<int, Client *>::iterator it =  _clients.find(fd);

	if (it == _clients.end()){
		return (1);
	}
	client = it->second;

	char buffer[100];
	bzero(buffer, 100);

	while (1)
	{
		bzero(buffer, 100);
		if ((n = recv(fd, buffer, 100, 0)) < 0) {
			if (errno != EWOULDBLOCK)
				throw std::runtime_error("Error while reading buffer from client.");
		}
		buffer[n] = '\0';

		std::cout << "buffer[100]: " << buffer << std::endl;
		if (std::strstr(buffer, "\r\n"))	// irssi 프로토콜로 메시지가 오는 경우
		{
			std::cout << "[From Irssi client]" << std::endl;
			client->getReceivedMessage().append(buffer);
			std::cout << "Client Stored String: " << client->getReceivedMessage() << std::endl;
			break;
		}
		else if (std::strstr(buffer, "\n"))	// nc 로 메시지가 오는 경우
		{
			std::cout << "[From nc Client]" << std::endl;
			buffer[n - 1] = '\0';
			client->getReceivedMessage().append(buffer);
			std::cout << "Client Stored String: " << client->getReceivedMessage() << std::endl;
			break;
		}
		else	// nc + ctrl + D로 메시지가 오는 경우
		{
			std::cout << "[From nc Client : Uncompleted Message]" << std::endl;
			client->getReceivedMessage().append(buffer);
			std::cout << "Client Stored String: " << client->getReceivedMessage() << std::endl;
			return (1);
			break;
		}
	}

	return (0);
}

void Server::onClientConnect() {
	int fd;
	sockaddr_in s_address = {};
	socklen_t s_size = sizeof(s_address);

	//fixing non-blocking accept
	while (true){
		fd = accept(_sock, (sockaddr *) &s_address, &s_size);
		if (fd < 0){
			if (errno == EAGAIN)
				break;
			else
				throw std::runtime_error("Error while accepting new client.");
		}
		//epoll
		addEvent(fd);
		
		//register client
		if (registerClient(fd, &s_address) == -1)
			throw std::runtime_error("Error while getting hostname on new client.");
	}
}

void Server::onClientDisconnect(int fd) {

	try {

		Client *client = _clients.at(fd);
		client->leave();

		//char message[1000];
		//sprintf(message, "%s:%d has disconnected.", client->getHostname().c_str(), client->getPort());
		ft_log(makeDisconnectLogMessage(client->getHostname(), client->getPort()));
		_clients.erase(fd);

		//epoll
		delEvent(fd);
		closeFd(fd);

		delete client;
	}
	catch (const std::out_of_range &ex) {
	}
}

void Server::onClientMessage(int fd)
{
	try {
		std::string msg;
		Client *client = _clients.at(fd);
		if (readMessage(fd) == 0)	// 메시지 끝까지 잘 받았다면 
		{
			msg.append(client->getReceivedMessage());
			client->getReceivedMessage().clear();
			_commandHandler->invoke(client, msg);
		}
		// else // 메시지를 다 받지 못했다면
	}
	catch (const std::out_of_range &ex) {
	}
}

Client *Server::getClient(const std::string &nickname) {
	//on refactoring
//	std::map<std::string, int>::iterator it = _clientsFdByNickname.find(nickname);
//	if (it == _clientsFdByNickname.end())
//		return (NULL);
//	int clientFd = it -> second;
//	std::map<int, Client *>::iterator clients_iterator = _clients.find(clientFd);
//	if (clients_iterator == _clients.end())
//		return NULL;
//	Client *cli_ret = clients_iterator -> second;
//	return (cli_ret);
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if (!nickname.compare(it->second->getNickname()))
			return it->second;
	}
	return NULL;
}

Channel *Server::getChannel(const std::string &name) {

	for (channels_iterator it = _channels.begin(); it != _channels.end(); it++)
		if (it.operator*()->getName() == name)
			return it.operator*();

	return NULL;
}

Channel *Server::createChannel(const std::string &name, const std::string &password, Client *client) {
	Channel *channel = new Channel(name, password, client);
	_channels.push_back(channel);
	channel->setInvitemode(false); // sungjuki

	return channel;
}

int Server::registerClient(int cli_fd, sockaddr_in *s_addr){

	char hostname[NI_MAXHOST];
		if (getnameinfo((struct sockaddr *)s_addr, sizeof(*s_addr), hostname, NI_MAXHOST, NULL, 0, NI_NUMERICSERV) != 0)
			return (-1);
		Client *client = new Client(cli_fd, hostname, ntohs(s_addr -> sin_port));
		_clients.insert(std::make_pair(cli_fd, client));
		ft_log(makeLogMessage(client->getHostname(), client->getPort()));

	return (_clients.size());
}

int Server::addEvent(int fd){
	struct epoll_event newEv;
	newEv.events = EPOLLIN | EPOLLPRI;
	newEv.data.fd = fd;
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &newEv) == -1)
		return (-1);
	return (0);
}

int Server::delEvent(int fd){
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, NULL) == -1)
		return (-1);
	return (0);
}

int Server::closeFd(int fd){ 
	return (close(fd));
}
