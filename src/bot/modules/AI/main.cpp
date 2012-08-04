/*
 * main.cpp
 *
 *  Created on: 13.07.2011
 *      Author: fkrauthan
 */

#include "ai.h"
AI* ai = NULL;


extern "C" bool module_init(Irc* irc, ModuleManager* modulemanager, const char* id, const char* jsonParams) {
	ai = new AI(irc, modulemanager, id);
	return ai->init(jsonParams);
}

extern "C" void module_remove(void) {
	delete ai;
}

extern "C" bool module_onEvent(IrcConnection* connection, const char* senderID, const char* eventName, const char* jsonString, const char* optString) {
	return ai->onEvent(connection, senderID, eventName, jsonString, optString);
}
