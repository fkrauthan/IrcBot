/*
 * IrcModule.cpp
 *
 *  Created on: 01.01.2011
 *      Author: fkrauthan
 */

#include "IrcModule.h"
#include <libjson/libjson.h>
#include <core/ModuleManager/ModuleManagerC.h>


IrcModule::IrcModule(const std::string id, ModuleManager* moduleManager, Irc* irc)
	: mID(id), mIRC(irc), mModuleManager(moduleManager) {
}

IrcModule::~IrcModule() {
}

IrcModuleConnection IrcModule::connect(const std::string& id, const std::string& server, const std::string& nick, int port) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("id", id));
	n.push_back(JSONNode("server", server));
	n.push_back(JSONNode("nick", nick));
	n.push_back(JSONNode("port", port));

	IrcConnection* con = (IrcConnection*)ModuleManager_callMethodWithPtrReturn(mModuleManager, mID.c_str(), "IRC", "connect", n.write().c_str(), NULL);
	return IrcModuleConnection(mID, mModuleManager, con);
}

void IrcModule::disconect(const std::string& id) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("id", id));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRC", "disconect", n.write().c_str(), NULL);
}

void IrcModule::disconectAll() {
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRC", "disconectAll", "", NULL);
}

IrcModuleConnection IrcModule::getConnection(const std::string& id) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("id", id));

	IrcConnection* con = (IrcConnection*)ModuleManager_callMethodWithPtrReturn(mModuleManager, mID.c_str(), "IRC", "getConnection", n.write().c_str(), NULL);
	return IrcModuleConnection(mID, mModuleManager, con);
}
