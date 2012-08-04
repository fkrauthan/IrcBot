/*
 * ModuleManagerC.cpp
 *
 *  Created on: 20.05.2010
 *      Author: fkrauthan
 */

#include "ModuleManagerC.h"
#include "modulemanager.h"


const char* ModuleManager_callMethod(void* modulemanager, const char* sender, const char* reciver, const char* action, const char* jsonString, void* connection) {
	return ((ModuleManager*)modulemanager)->callMethod(sender, reciver, action, jsonString, (IrcConnection*)connection).c_str();
}

void* ModuleManager_callMethodWithPtrReturn(void* modulemanager, const char* sender, const char* reciver, const char* action, const char* jsonString, void* connection) {
	return ((ModuleManager*)modulemanager)->callMethodWithPtrReturn(sender, reciver, action, jsonString, (IrcConnection*)connection);
}
