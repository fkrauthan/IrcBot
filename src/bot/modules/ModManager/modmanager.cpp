/*
 * modmanager.cpp
 *
 *  Created on: 02.04.2011
 *      Author: fkrauthan
 */

#include "modmanager.h"
#include <libbase/StringUtils/StringUtils.h>
#include <iostream>
#include <vector>


ModManager::ModManager(Irc* irc, ModuleManager* moduleManager, const std::string& id) : CPPModule(irc, moduleManager, id) {
}

ModManager::~ModManager() {
}

bool ModManager::onInit(const std::map<std::string, std::string>& params) {
	return true;
}

bool ModManager::onPrivateMessage(IrcModuleConnection& connection, IrcMessage& message) {
	if(connection.getNick() == message.getUsername()) {
		return true;
	}

	std::vector<std::string> params;
	Base::StringUtils::split(message.params, ' ', params);
	if(params[0] != ".module") {
		return true;
	}

	if(params.size() <= 1) {
		connection.sendMessage(message.getUsername(), "Error: You haven't entered a action");
		return true;
	}

	if(params[1] == "reload") {
		if(params.size() <= 2) {
			connection.sendMessage(message.getUsername(), "Error: You haven't entered a module id");
			return true;
		}

		if(params[2]==mID) {
			mIrcModuleManager.reloadModule(params[2]);
			return false;
		}
		if(!mIrcModuleManager.reloadModule(params[2])) {
			connection.sendMessage(message.getUsername(), "Error: There was an error by relaoding the module with id \""+params[2]+"\"");
		}
		else {
			connection.sendMessage(message.getUsername(), "Success: Module with id \""+params[2]+"\" is succesfull reloaded");
		}
		return false;
	}
	else if(params[1] == "unload") {
		if(params.size() <= 2) {
			connection.sendMessage(message.getUsername(), "Error: You haven't entered a module id");
			return true;
		}

		if(params[2]==mID) {
			mIrcModuleManager.unloadModule(params[2]);
			return false;
		}
		mIrcModuleManager.unloadModule(params[2]);

		connection.sendMessage(message.getUsername(), "Success: Module with id \""+params[2]+"\" is succesfull unloaded");
		return false;
	}
	else if(params[1] == "loadBinary") {
		if(params.size() <= 3) {
			connection.sendMessage(message.getUsername(), "Error: You haven't entered a module id or/and the filename");
			return true;
		}

		std::map<std::string, std::string> tmpParams;
		for(int i=4; i<params.size(); i++) {
			std::vector<std::string> tParams;
			Base::StringUtils::split(params[i], '=', tParams);

			if(tParams.size()<=1) {
				connection.sendMessage(message.getUsername(), "Warning: There is an config line which hasen't key and value: \""+params[i]+"\"");
				continue;
			}
			tmpParams[tParams[0]] = tParams[1];
		}

		if(!mIrcModuleManager.loadModuleBinary(params[2], params[3], tmpParams)) {
			connection.sendMessage(message.getUsername(), "Error: There was an error by loading the binary module with id \""+params[2]+"\"");
		}
		else {
			connection.sendMessage(message.getUsername(), "Success: Binary module with id \""+params[2]+"\" is succesfull loaded");
		}
		return false;
	}
	else if(params[1] == "loadScript") {
		if(params.size() <= 3) {
			connection.sendMessage(message.getUsername(), "Error: You haven't entered a module id or/and the filename");
			return true;
		}

		std::map<std::string, std::string> tmpParams;
		for(int i=4; i<params.size(); i++) {
			std::vector<std::string> tParams;
			Base::StringUtils::split(params[i], '=', tParams);

			if(tParams.size()<=1) {
				connection.sendMessage(message.getUsername(), "Warning: There is an config line which hasen't key and value: \""+params[i]+"\"");
				continue;
			}
			tmpParams[tParams[0]] = tParams[1];
		}

		if(!mIrcModuleManager.loadModuleScript(params[2], params[3], tmpParams)) {
			connection.sendMessage(message.getUsername(), "Error: There was an error by loading the script module with id \""+params[2]+"\"");
		}
		else {
			connection.sendMessage(message.getUsername(), "Success: Script module with id \""+params[2]+"\" is succesfull loaded");
		}
		return false;
	}
	else {
		connection.sendMessage(message.getUsername(), "Error: Unkown module command \""+params[1]+"\"");
	}

	return true;
}

/*void InviteMe::printHelp(IrcConnection* connection, IrcMessageForModule* message) {
	IrcConnection_sendToUser(connection, message->sendername, "!!!Invite me Help!!!");
	IrcConnection_sendToUser(connection, message->sendername, "  This module has no commands. Just invite the bot and he joins to the invited channel");
	IrcConnection_sendToUser(connection, message->sendername, "!!!Invite me Help!!!");
}*/
