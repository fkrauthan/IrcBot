/*
 * IrcModuleConnection.cpp
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#include "IrcModuleConnection.h"
#include <libjson/libjson.h>
#include <core/ModuleManager/ModuleManagerC.h>


IrcModuleConnection::IrcModuleConnection(const std::string id, ModuleManager* moduleManager, IrcConnection* connection)
	: mID(id), mConnection(connection), mModuleManager(moduleManager) {

}

IrcModuleConnection::~IrcModuleConnection() {
}

void IrcModuleConnection::joinChan(const std::string& channel) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("channel", channel));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "joinChan", n.write().c_str(), mConnection);
}

void IrcModuleConnection::partChan(const std::string& channel) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("channel", channel));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "partChan", n.write().c_str(), mConnection);
}

void IrcModuleConnection::sendQuit(const std::string& msg) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("msg", msg));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "sendQuit", n.write().c_str(), mConnection);
}

void IrcModuleConnection::sendMessage(const std::string& target, const std::string& message) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("target", target));
	n.push_back(JSONNode("message", message));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "sendMessage", n.write().c_str(), mConnection);
}

void IrcModuleConnection::sendNotice(const std::string& target, const std::string& message) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("target", target));
	n.push_back(JSONNode("message", message));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "sendNotice", n.write().c_str(), mConnection);
}

void IrcModuleConnection::sendAction(const std::string& target, const std::string& message) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("target", target));
	n.push_back(JSONNode("message", message));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "sendAction", n.write().c_str(), mConnection);
}

void IrcModuleConnection::sendCTCP(const std::string& target, const std::string& message) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("target", target));
	n.push_back(JSONNode("message", message));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "sendCTCP", n.write().c_str(), mConnection);
}

void IrcModuleConnection::changeNick(const std::string& nick) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("nick", nick));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "changeNick", n.write().c_str(), mConnection);
}

void IrcModuleConnection::setUserMode(const std::string& mode) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("mode", mode));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "setUserMode", n.write().c_str(), mConnection);
}

void IrcModuleConnection::setMode(const std::string& nick, const std::string& mode) {
	JSONNode n(JSON_NODE);
	n.push_back(JSONNode("nick", nick));
	n.push_back(JSONNode("mode", mode));
	ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "setMode", n.write().c_str(), mConnection);
}

std::map<std::string, IrcChannel> IrcModuleConnection::getChannels() {
	std::string ret = ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "getChannels", "", mConnection);
	JSONNode n = libjson::parse(ret);
	std::map<std::string, IrcChannel> tmp;
	JSONNode::iterator i = n.begin();
	for(; i!=n.end(); ++i) {
		JSONNode pN = (*i).at(0);

		IrcChannel tmpChannel;
		tmpChannel.name = pN["name"].as_string();
		tmpChannel.topic = pN["topic"].as_string();
		tmpChannel.nameListFull = pN["nameListFull"].as_bool();

		JSONNode n2 = pN["members"];
		JSONNode::iterator i2 = n2.begin();
		for(; i2!=n2.end(); ++i2) {
			IrcChannelMember tmpMember;
			tmpMember.nick = (*i2)["nick"].as_string();

			JSONNode n3 = (*i2)["modes"];
			JSONNode::iterator i3 = n3.begin();
			for(std::uint64_t i=1; i!=std::uint64_t(1) << 52; i<<=1, ++i3) {
				if((*i3).as_bool()) {
					tmpMember.modes |= i;
				}
			}

			//tmpMember.mode = static_cast<IrcRightEnum::Enumeration>((*i2)["mode"].as_int());
			tmpChannel.members.push_back(tmpMember);
		}
		tmp[(*i).name()] = tmpChannel;
	}


	return tmp;
}

std::string IrcModuleConnection::getServer() {
	return ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "getServer", "", mConnection);
}

int IrcModuleConnection::getPort() {
	return std::atoi(ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "getPort", "", mConnection));
}

std::string IrcModuleConnection::getNick() {
	return ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "getNick", "", mConnection);
}

std::string IrcModuleConnection::getID() {
	return ModuleManager_callMethod(mModuleManager, mID.c_str(), "IRCCONNECTION", "getID", "", mConnection);
}

IrcConnection* IrcModuleConnection::getIrcConnection() {
	return mConnection;
}
