/*
 * main.cpp
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#include "inviteme.h"
InviteMe* inviteMe = NULL;


extern "C" bool module_init(Irc* irc, ModuleManager* modulemanager, const char* id, const char* jsonParams) {
	inviteMe = new InviteMe(irc, modulemanager, id);
	return inviteMe->init(jsonParams);
}

extern "C" void module_remove(void) {
	delete inviteMe;
}

extern "C" bool module_onEvent(IrcConnection* connection, const char* senderID, const char* eventName, const char* jsonString, const char* optString) {
	return inviteMe->onEvent(connection, senderID, eventName, jsonString, optString);
}
