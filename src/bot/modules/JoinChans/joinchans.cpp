/*
 * joinchans.cpp
 *
 *  Created on: 31.12.2010
 *      Author: fkrauthan
 */

#include "joinchans.h"
#include <libbase/StringUtils/StringUtils.h>
#include <iostream>


JoinChans::JoinChans(Irc* irc, ModuleManager* moduleManager, const std::string& id) : CPPModule(irc, moduleManager, id) {
}

JoinChans::~JoinChans() {
}

bool JoinChans::onInit(const std::map<std::string, std::string>& params) {
	std::map<std::string, std::string>::const_iterator iter;
	for(iter=params.begin(); iter!=params.end(); ++iter) {
		std::vector<std::string> tmpSplit;
		Base::StringUtils::split(iter->second, ',', tmpSplit, true, true);
		if(tmpSplit.size() != 2) {
			std::cout << "Error: This is not a correct translate entry with name \"" << iter->first << "\": " << iter->second << std::endl;
			continue;
		}

		JoinedChannel tmpJoinedChannel;
		size_t pos = tmpSplit[0].find(':');
		if(pos == std::string::npos) {
			tmpJoinedChannel.fromChannel = tmpSplit[0];
		}
		else {
			tmpJoinedChannel.fromChannel = tmpSplit[0].substr(pos+1);
			tmpJoinedChannel.fromChannelServerID = tmpSplit[0].substr(0, pos);
		}

		pos = tmpSplit[1].find(':');
		if(pos == std::string::npos) {
			tmpJoinedChannel.toChannel = tmpSplit[1];
		}
		else {
			tmpJoinedChannel.toChannel = tmpSplit[1].substr(pos+1);
			tmpJoinedChannel.toChannelServerID = tmpSplit[1].substr(0, pos);
		}

		mTranslateMap.push_back(tmpJoinedChannel);
	}

	return true;
}

bool JoinChans::onChannelMessage(IrcModuleConnection& connection, IrcMessage& message) {
	if(connection.getNick() == message.getUsername()) {
		return true;
	}

	std::vector<JoinedChannel>::iterator iter;
	for(iter=mTranslateMap.begin(); iter!=mTranslateMap.end(); ++iter) {
		if((*iter).fromChannel == message.target && ((*iter).fromChannelServerID.empty() || (*iter).fromChannelServerID == connection.getID())) {
			if((*iter).fromChannelServerID == (*iter).toChannelServerID || (*iter).toChannelServerID.empty()) {
				connection.sendMessage((*iter).toChannel, "<"+message.getUsername()+"> "+message.params);
			}
			else {
				mIRC.getConnection((*iter).toChannelServerID).sendMessage((*iter).toChannel, "<"+message.getUsername()+"> "+message.params);
			}
		}
	}

	return true;
}

/*void InviteMe::printHelp(IrcConnection* connection, IrcMessageForModule* message) {
	IrcConnection_sendToUser(connection, message->sendername, "!!!Invite me Help!!!");
	IrcConnection_sendToUser(connection, message->sendername, "  This module has no commands. Just invite the bot and he joins to the invited channel");
	IrcConnection_sendToUser(connection, message->sendername, "!!!Invite me Help!!!");
}*/
