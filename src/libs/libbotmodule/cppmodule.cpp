/*
 * cppmodule.cpp
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#include "cppmodule.h"
#include <libjson/libjson.h>


CPPModule::CPPModule(Irc* irc, ModuleManager* moduleManager, const std::string& id)
	: mIRC(id, moduleManager, irc),
	  mIrcModuleManager(id, moduleManager),
	  mModuleManager(moduleManager),
	  mID(id) {
}

CPPModule::~CPPModule() {
}

bool CPPModule::init(const std::string& jsonParams) {
	JSONNode n = libjson::parse(jsonParams);
	std::map<std::string, std::string> tmpParams;
	JSONNode::iterator i = n.begin();
	for(; i!=n.end(); ++i) {
		tmpParams[(*i).name()] = (*i).as_string();
	}
	return onInit(tmpParams);
}

bool CPPModule::onEvent(IrcConnection* connection, const std::string& senderID, const std::string& eventName, const std::string& json, const std::string& optString) {
	JSONNode n;
	if(!json.empty()) {
		n = libjson::parse(json);
	}

	if(senderID=="IRCCONNECTION") {
		//Create a IrcModuleConnection instance
		IrcModuleConnection ircModuleConnection(mID, mModuleManager, connection);


		//Call onConnect if its the called event
		if(eventName=="onConnect") {
			return onConnect(ircModuleConnection);
		}


		//Convert the IRC Message
		IrcMessage message;
		message.ircLine = n["ircLine"].as_string();
		message.prefix = n["prefix"].as_string();
		message.hasDetailedPrefix = n["hasDetailedPrefix"].as_bool();

		if(message.hasDetailedPrefix) {
			message.msgPrefix.nick_or_server = n["msgPrefix"]["nick_or_server"].as_string();
			message.msgPrefix.user = n["msgPrefix"]["user"].as_string();
			message.msgPrefix.host = n["msgPrefix"]["host"].as_string();
		}

		message.command = n["command"].as_string();
		message.isNumeric = n["isNumeric"].as_bool();
		message.target = n["target"].as_string();
		message.params = n["params"].as_string();


		//Call all other event handlers
		if(eventName=="onMessage") {
			return onMessage(ircModuleConnection, message);
		}
		else if(eventName=="onJoin") {
			return onJoin(ircModuleConnection, message);
		}
		else if(eventName=="onPart") {
			return onPart(ircModuleConnection, message);
		}
		else if(eventName=="onKick") {
			return onKick(ircModuleConnection, message, optString);
		}
		else if(eventName=="onQuit") {
			return onQuit(ircModuleConnection, message);
		}
		else if(eventName=="onKill") {
			return onKill(ircModuleConnection, message, optString);
		}
		else if(eventName=="onPrivateMessage") {
			return onPrivateMessage(ircModuleConnection, message);
		}
		else if(eventName=="onChannelMessage") {
			return onChannelMessage(ircModuleConnection, message);
		}
		else if(eventName=="onNotice") {
			return onNotice(ircModuleConnection, message);
		}
		else if(eventName=="onInvite") {
			return onInvite(ircModuleConnection, message);
		}
		else if(eventName=="onTopicChanged") {
			return onTopicChanged(ircModuleConnection, message);
		}
		else if(eventName=="onNickChanged") {
			return onNickChanged(ircModuleConnection, message);
		}
	}

	return true;
}


std::string CPPModule::onInternalMessage_p(const std::string& senderID, const std::string& action, const std::string& jsonParams, IrcConnection* connection) {
	IrcModuleConnection ircModuleConnection(mID, mModuleManager, connection);
	return onInternalMessage(senderID, action, jsonParams, ircModuleConnection);
}

void* CPPModule::onInternalMessageWithPtr_p(const std::string& senderID, const std::string& action, const std::string& jsonParams, IrcConnection* connection) {
	IrcModuleConnection ircModuleConnection(mID, mModuleManager, connection);
	return onInternalMessageWithPtr(senderID, action, jsonParams, ircModuleConnection);
}


bool CPPModule::onInit(const std::map<std::string, std::string>& params) {
	return true;
}


std::string CPPModule::onInternalMessage(const std::string& senderID, const std::string& action, const std::string& jsonParams, IrcModuleConnection& connection) {
	return "";
}

void* CPPModule::onInternalMessageWithPtr(const std::string& senderID, const std::string& action, const std::string& jsonParams, IrcModuleConnection& connection) {
	return NULL;
}


bool CPPModule::onConnect(IrcModuleConnection& connection) {
	return true;
}

bool CPPModule::onMessage(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onJoin(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onPart(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onKick(IrcModuleConnection& connection, IrcMessage& message, const std::string& nick) {
	return true;
}

bool CPPModule::onQuit(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onKill(IrcModuleConnection& connection, IrcMessage& message, const std::string& nick) {
	return true;
}

bool CPPModule::onPrivateMessage(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onChannelMessage(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onNotice(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onInvite(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onTopicChanged(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}

bool CPPModule::onNickChanged(IrcModuleConnection& connection, IrcMessage& message) {
	return true;
}
