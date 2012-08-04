/*
 * IrcModuleManager.cpp
 *
 *  Created on: 02.04.2011
 *      Author: fkrauthan
 */

#include "IrcModuleManager.h"
#include <libjson/libjson.h>
#include <core/ModuleManager/ModuleManagerC.h>


IrcModuleManager::IrcModuleManager(const std::string id, ModuleManager* moduleManager)
	: mID(id), mModuleManager(moduleManager) {
}

IrcModuleManager::~IrcModuleManager() {
}

bool IrcModuleManager::loadModuleBinary(const std::string& id, const std::string& file, const std::map<std::string,std::string>& params) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("id", id));
	n.push_back(JSONNode("file", file));

	JSONNode n2(JSON_NODE);
	n2.set_name("params");
	std::map<std::string,std::string>::const_iterator iter;
	for(iter=params.begin(); iter!=params.end(); ++iter) {
		n2.push_back(JSONNode(iter->first, iter->second));
	}
	n.push_back(n2);

	return std::atoi(ModuleManager_callMethod(mModuleManager, mID.c_str(), "MODULEMANAGER", "loadModuleBinary", n.write().c_str(), NULL));
}

bool IrcModuleManager::loadModuleScript(const std::string& id, const std::string& file, const std::map<std::string,std::string>& params) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("id", id));
	n.push_back(JSONNode("file", file));

	JSONNode n2(JSON_NODE);
	n2.set_name("params");
	std::map<std::string,std::string>::const_iterator iter;
	for(iter=params.begin(); iter!=params.end(); ++iter) {
		n2.push_back(JSONNode(iter->first, iter->second));
	}
	n.push_back(n2);

	return std::atoi(ModuleManager_callMethod(mModuleManager, mID.c_str(), "MODULEMANAGER", "loadModuleScript", n.write().c_str(), NULL));
}

bool IrcModuleManager::reloadModule(const std::string& id) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("id", id));

	return std::atoi(ModuleManager_callMethod(mModuleManager, mID.c_str(), "MODULEMANAGER", "reloadModule", n.write().c_str(), NULL));
}

void IrcModuleManager::unloadModule(const std::string& id) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("id", id));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "MODULEMANAGER", "unloadModule", n.write().c_str(), NULL);
}

std::string IrcModuleManager::callMethod(const std::string& reciver, const std::string& action, const std::string& jsonString, IrcModuleConnection& connection) {
	return callMethod(reciver, action, jsonString, connection.getIrcConnection());
}

void* IrcModuleManager::callMethodWithPtrReturn(const std::string& reciver, const std::string& action, const std::string& jsonString, IrcModuleConnection& connection) {
	return callMethodWithPtrReturn(reciver, action, jsonString, connection.getIrcConnection());
}

std::string IrcModuleManager::callMethod(const std::string& reciver, const std::string& action, const std::string& jsonString, IrcConnection* connection) {
	return ModuleManager_callMethod(mModuleManager, mID.c_str(), reciver.c_str(), action.c_str(), jsonString.c_str(), connection);
}

void* IrcModuleManager::callMethodWithPtrReturn(const std::string& reciver, const std::string& action, const std::string& jsonString, IrcConnection* connection) {
	return ModuleManager_callMethodWithPtrReturn(mModuleManager, mID.c_str(), reciver.c_str(), action.c_str(), jsonString.c_str(), connection);
}
