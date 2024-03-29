cmake_minimum_required(VERSION 3.16)
project(ft_irc)
set(CMAKE_CXX_STANDARD 98)

include_directories(include)
include_directories(include/network)
include_directories(include/commands)

set(SOURCES
		src/network/Server.cpp
		src/network/Client.cpp
		src/network/Channel.cpp
		src/commands/CommandHandler.cpp
		src/commands/impl/NickCommand.cpp
		src/commands/impl/PassCommand.cpp
		src/commands/impl/UserCommand.cpp
		src/commands/impl/JoinCommand.cpp
		src/commands/impl/QuitCommand.cpp
        src/commands/impl/PartCommand.cpp
		src/commands/impl/PrivMsgCommand.cpp
		src/commands/impl/KickCommand.cpp
		src/commands/impl/PingCommand.cpp
		src/commands/impl/PongCommand.cpp
		src/commands/impl/NoticeCommand.cpp
		src/commands/impl/ModeCommand.cpp
		src/commands/impl/TopicCommand.cpp
		src/commands/impl/InviteCommand.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(ircserv src/main.cpp ${SOURCES})
