/*
 * inviteme.cpp
 *
 *  Created on: 22.04.2010
 *      Author: fkrauthan
 */

#include "inviteme.h"


InviteMe::InviteMe(Irc* irc, ModuleManager* moduleManager, const std::string& id) : CPPModule(irc, moduleManager, id) {
}

InviteMe::~InviteMe() {
}

bool InviteMe::onInvite(IrcModuleConnection& connection, IrcMessage& message) {
	connection.joinChan(message.params);
	return true;
}

/*void InviteMe::printHelp(IrcConnection* connection, IrcMessageForModule* message) {
	IrcConnection_sendToUser(connection, message->sendername, "!!!Invite me Help!!!");
	IrcConnection_sendToUser(connection, message->sendername, "  This module has no commands. Just invite the bot and he joins to the invited channel");
	IrcConnection_sendToUser(connection, message->sendername, "!!!Invite me Help!!!");
}*/
