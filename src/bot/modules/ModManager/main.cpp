/*
 * main.cpp
 *
 *  Created on: 02.04.2011
 *      Author: fkrauthan
 */

#include "modmanager.h"
ModManager* modManager = NULL;


extern "C" bool module_init(Irc* irc, ModuleManager* modulemanager, const char* id, const char* jsonParams) {
	modManager = new ModManager(irc, modulemanager, id);
	return modManager->init(jsonParams);
}

extern "C" void module_remove(void) {
	delete modManager;
}

extern "C" bool module_onEvent(IrcConnection* connection, const char* senderID, const char* eventName, const char* jsonString, const char* optString) {
	return modManager->onEvent(connection, senderID, eventName, jsonString, optString);
}

extern "C" bool module_isModManager(void) {
	return true;
}
