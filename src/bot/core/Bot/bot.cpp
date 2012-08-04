/*
 * bot.cpp
 *
 *  Created on: 21.04.2010
 *      Author: fkrauthan
 */

#include "bot.h"
#include <libirc/ircconnection.h>
#include <libini/INI.h>
#include <libbase/StringUtils/StringUtils.h>
#include <iostream>
#include <cstring>


Bot::Bot(Irc::ClientInfo& clientInfo)
: irc(clientInfo) {

}

Bot::~Bot() {

}

bool Bot::init(const std::string& configfile) {
	std::cout << "Register moduleManager for connect event on irc class...";
	irc.registerBaseHandler(&moduleManager);
	std::cout << "finish" << std::endl;

	std::cout << "Load configfile..." << std::endl;
	try {
		libINI::INI configFile(configfile);
		std::string defaultNick = configFile.getValue("general", "nick", "ircbot");


		//Read modules
		std::cout << "-> Read modulelist..." << std::endl;
		if(!moduleManager.init(configFile, &irc)) {
			return false;
		}
		std::cout << "-> Read modulelist...finish" << std::endl;


		//Read the servers
		std::cout << "-> Read serverlist..." << std::endl;
		std::map<std::string, std::string>& serverMap = configFile.getSection("servers");
		std::map<std::string, std::string>::iterator iter;
		for(iter=serverMap.begin(); iter!=serverMap.end(); ++iter) {
			size_t delemiterPos = iter->second.find(':');
			std::string hostName = iter->second.substr(0, delemiterPos);
			int port = std::atoi(iter->second.substr(delemiterPos+1).c_str());


			//Read nick for this server
			std::string tmpNick = configFile.getValue("nicks", iter->first, defaultNick);


			//Read init modes
			std::cout << "--> Read modes list..." << std::endl;
			std::vector<std::string> tmpModes;
			if(configFile.issetValue("modes", iter->first)) {
				Base::StringUtils::split(configFile.getValue("modes", iter->first), ',', tmpModes, true, true);
				std::vector<std::string>::iterator iter2;
				for(iter2=tmpModes.begin(); iter2!=tmpModes.end(); ++iter2) {
					std::cout << "---> Register mode: " << *iter2 << "...finish" << std::endl;
				}
			}
			mInitModes[iter->first] = tmpModes;
			std::cout << "--> Read modes list...finish" << std::endl;

			//Read init channels
			std::cout << "--> Read channel list..." << std::endl;
			std::vector<std::string> tmpChannels;
			if(configFile.issetValue("channels", iter->first)) {
				Base::StringUtils::split(configFile.getValue("channels", iter->first), ',', tmpChannels, true, true);
				std::vector<std::string>::iterator iter2;
				for(iter2=tmpChannels.begin(); iter2!=tmpChannels.end(); ++iter2) {
					std::cout << "---> Register " << *iter2 << " as channel to join...finish" << std::endl;;
				}
			}
			mInitChannels[iter->first] = tmpChannels;
			std::cout << "--> Read channel list...finish" << std::endl;


			//Connect to server
			std::cout << "--> Connect to server " << hostName << ":" << port << " with nick " << tmpNick << "..." << std::endl;
			IrcConnection* ircCon = NULL;
			if(!(ircCon= irc.connect(iter->first, hostName, tmpNick, port))) {
				std::cout << "--> Connect to server " << hostName << ":" << port << " with nick " << tmpNick << "...failed" << std::endl;

				mInitModes.erase(mInitModes.find(iter->first));
				mInitModes.erase(mInitChannels.find(iter->first));
			}
			else {
				std::cout << "--> Connect to server " << hostName << ":" << port << " with nick " << tmpNick << "...finish" << std::endl;

				std::cout << "--> Register event...";
				ircCon->registerEventHandler(this);
				std::cout << "finish" << std::endl;
			}
		}
		std::cout << "-> Read serverlist...finish" << std::endl;
	} catch(libINI::INIException& ex) {
		std::cout << "--> Error: Error by parsing config file: " << ex.what() << std::endl;
		return false;
	}
	std::cout << "Load configfile...finish" << std::endl;

	return true;
}

void Bot::run() {
	std::cout << "Run bot..." << std::endl;
	irc.run();
	std::cout << "Quit bot..." << std::endl;
}

bool Bot::onConnect(IrcConnection& connection) {
	std::vector<std::string>& tmpVec = mInitModes[connection.getID()];
	std::vector<std::string>::iterator iter;
	for(iter = tmpVec.begin(); iter != tmpVec.end(); ++iter) {
		connection.setUserMode(*iter);
	}

	tmpVec = mInitChannels[connection.getID()];
	for(iter = tmpVec.begin(); iter != tmpVec.end(); ++iter) {
		connection.joinChan(*iter);
	}

	std::cout << "On Connect" << std::endl;

	return true;
}

bool Bot::onMessage(IrcConnection& connection, IrcMessage& message) {
	if(message.params == ".show") {
		std::map<std::string, IrcChannel>& channels = connection.getChannels();

		std::map<std::string, IrcChannel>::iterator iter;
		for(iter=channels.begin(); iter!=channels.end(); ++iter) {
			moduleManager.convertIrcChannelToJSONString(iter->second);

			connection.sendMessage(message.msgPrefix.nick_or_server, "Channel: "+iter->second.name+" with Topic: "+iter->second.topic);
			std::string secondLine = "--> ";
			for(int i=0; i<iter->second.members.size(); i++) {
				secondLine.append(iter->second.members[i].nick+" (");
				if(iter->second.members[i].hasMode(IrcRightEnum::OP)) secondLine.append("OP,");
				if(iter->second.members[i].hasMode(IrcRightEnum::HALF_OP)) secondLine.append("HALF OP,");
				if(iter->second.members[i].hasMode(IrcRightEnum::VOICE)) secondLine.append("VOICE,");
				secondLine.append("), ");
			}
			connection.sendMessage(message.msgPrefix.nick_or_server, secondLine);
		}
	}
	else if(message.params == ".quit") {
		//connection->getIrcManager()->disconect(connection);
		connection.getIrcManager()->disconectAll();
	}

	return false;
}
