/*
 * main.cpp
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#include "joinchans.h"
JoinChans* joinChans = NULL;


extern "C" bool module_init(Irc* irc, ModuleManager* modulemanager, const char* id, const char* jsonParams) {
	joinChans = new JoinChans(irc, modulemanager, id);
	return joinChans->init(jsonParams);
}

extern "C" void module_remove(void) {
	delete joinChans;
}

extern "C" bool module_onEvent(IrcConnection* connection, const char* senderID, const char* eventName, const char* jsonString, const char* optString) {
	return joinChans->onEvent(connection, senderID, eventName, jsonString, optString);
}
