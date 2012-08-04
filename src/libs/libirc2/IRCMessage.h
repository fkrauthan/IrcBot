/*
 * IRCMessage.h
 *
 *  Created on: 23.07.2011
 *      Author: fkrauthan
 */

#ifndef IRCMESSAGE_H_
#define IRCMESSAGE_H_

#include <string>
#include <vector>


struct IRCPrefix {
	std::string nickOrServer;

	bool hasUserAndHost;

	std::string user;
	std::string host;
};

struct IRCMessage {
	std::string completeLine;

	std::string prefixString;
	IRCPrefix prefix;

	std::string command;
	bool isNumeric;

	std::string paramsString;
	std::vector<std::string> params;

	bool isCTCPMessage;
};

#endif /* IRCMESSAGE_H_ */
