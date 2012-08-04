/*
 * main.cpp
 *
 *  Created on: 23.07.2011
 *      Author: fkrauthan
 */

#include <libirc2/IRC.h>
#include <libirc2/IRCConnection.h>

#include <iostream>


class TestHandler {
	public:
		virtual ~TestHandler() {}

		void onConnect(IRC& irc, IRCConnection& connection) {
			connection.joinChan("#fkr");
		}

		void onMessage(IRC& irc, IRCConnection& connection, IRCMessage& message) {
			if(message.params[1] == ".action") {
				if(message.params[0] == connection.getNick()) {
					connection.sendCTCPAction(message.prefix.nickOrServer, "freut sich");
				}
				else {
					connection.sendCTCPAction(message.params[0], "freut sich");
				}
			}
		}

		void onPrivateMessage(IRC& irc, IRCConnection& connection, IRCMessage& message) {
			if(message.params[1] == ".debug") {
				std::map<std::string, IRCChannel>& channels = connection.getChannels();

				std::map<std::string, IRCChannel>::iterator iter;
				for(iter=channels.begin(); iter!=channels.end(); ++iter) {
					connection.sendMessage(message.prefix.nickOrServer, "Channel: "+iter->second.name+" (TOPIC: "+iter->second.topic+")");
					std::map<std::string, IRCMember>::iterator iter2;
					std::stringstream str;
					str << "--> ";
					for(iter2=iter->second.members.begin(); iter2!=iter->second.members.end(); ++iter2) {
						str  << iter2->second.nick << " (";
						if(iter2->second.hasMode(IRCUserModeEnum::OP)) str << "OP, ";
						if(iter2->second.hasMode(IRCUserModeEnum::HALF_OP)) str << "HALF OP, ";
						if(iter2->second.hasMode(IRCUserModeEnum::VOICE)) str << "VOICE, ";
						str << "), ";
					}
					connection.sendMessage(message.prefix.nickOrServer, str.str());
				}
			}
		}

		void onChannelMessage(IRC& irc, IRCConnection& connection, IRCMessage& message) {
			if(message.params[1] == ".op") {
				connection.setChannelUserMode(message.params[0], message.prefix.nickOrServer, IRCUserModeEnum::OP);
			}
			else if(message.params[1] == ".deop") {
				connection.removeChannelUserMode(message.params[0], message.prefix.nickOrServer, IRCUserModeEnum::OP);
			}
		}

		void onCTCPAction(IRC& irc, IRCConnection& connection, IRCMessage& message, const std::string& action) {
			if(message.params[0] == connection.getNick()) {
				connection.sendCTCPAction(message.prefix.nickOrServer, "Antwort auf: "+action);
			}
			else {
				connection.sendCTCPAction(message.params[0], "Antwort auf: "+action);
			}
		}

		void onUserObtainMode(IRC& irc, IRCConnection& connection, IRCMessage& message, const std::string& nick, IRCUserModeEnum::Enumeration mode) {
			connection.sendMessage(message.params[0], "Jiiiiha "+nick+" hat den Modus "+IRCUserModeEnum::toString(mode)+" bekommen :)");
		}

		void onUserLoseMode(IRC& irc, IRCConnection& connection, IRCMessage& message, const std::string& nick, IRCUserModeEnum::Enumeration mode) {
			connection.sendMessage(message.params[0], "Haha "+nick+" hat den Modus "+IRCUserModeEnum::toString(mode)+" verloren *freu*");
		}
};

int main(int argc, char **argv) {
	std::cout << "IRC Server test" << std::endl;


	TestHandler testHandler;


	IRC irc;
	IRCConnection* connection = irc.connect("euIRC", "irc.de.euirc.net", "IrcBot4");
	//IRCConnection* connection = irc.connect("freenode", "kornbluth.freenode.net", "IrcBot4");
	connection->onConnect.connect(sigc::mem_fun(&testHandler, &TestHandler::onConnect));
	connection->onMessage.connect(sigc::mem_fun(&testHandler, &TestHandler::onMessage));
	connection->onPrivateMessage.connect(sigc::mem_fun(&testHandler, &TestHandler::onPrivateMessage));
	connection->onChannelMessage.connect(sigc::mem_fun(&testHandler, &TestHandler::onChannelMessage));
	connection->onCTCPAction.connect(sigc::mem_fun(&testHandler, &TestHandler::onCTCPAction));
	connection->onUserObtainMode.connect(sigc::mem_fun(&testHandler, &TestHandler::onUserObtainMode));
	connection->onUserLoseMode.connect(sigc::mem_fun(&testHandler, &TestHandler::onUserLoseMode));

	irc.run();

	return 0;
}
