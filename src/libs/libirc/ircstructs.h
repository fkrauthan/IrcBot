/*
 * ircstructs.h
 *
 *  Created on: 30.12.2010
 *      Author: fkrauthan
 */

#ifndef IRCSTRUCTS_H_
#define IRCSTRUCTS_H_

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdint>


struct IrcRightEnum {
	enum Enumeration {
		NOTHING = 0,
		VOICE = 1,
		HALF_OP = 2,
		OP = 4
	};

	static std::string toString(IrcRightEnum::Enumeration enumeration) {
		switch(enumeration) {
			case OP:
				return "OP";
			case HALF_OP:
				return "HALF OP";
			case VOICE:
				return "VOICE";
			case NOTHING:
				return "NOTHING";
		}
	}
};

struct IrcMessagePrefix {
	std::string nick_or_server;
	std::string user;
	std::string host;


	std::string toString() {
		std::stringstream str;
		str << "[nick_or_server=\"" << nick_or_server << "\"], [user=\"" << user << "\"], [host=\"" << host << "\"]";
		return str.str();
	}
};

struct IrcMessage {
	std::string ircLine;

	std::string prefix;
	bool hasDetailedPrefix;
	IrcMessagePrefix msgPrefix;

	std::string command;
	bool isNumeric;

	std::string target;
	std::string params;

	std::string& getUsername() {
		if(hasDetailedPrefix) {
			return msgPrefix.nick_or_server;
		}
		return prefix;
	}


	std::string toString() {
		std::stringstream str;
		str << "[ircLine=\"" << ircLine << "\"], [prefix=\"" << prefix << "\"], [hasDetailedPrefix=\"";
		str << hasDetailedPrefix << "\"], [msgPrefix={" << msgPrefix.toString() << "}], [command=\"" << command << "\"], ";
		str << "[isNumeric=\"" << isNumeric << "\"], [target=\"" << target << "\"], [params=\"" << params << "\"]";
		return str.str();
	}
};

struct IrcChannelMember {
	std::string nick;
	std::uint64_t modes;


	IrcChannelMember() {
		modes = 0;
	}


	bool hasMode(IrcRightEnum::Enumeration enumeration) {
		if(enumeration == IrcRightEnum::NOTHING) {
			return true;
		}
		return ((modes & enumeration) != 0);
	}

	void addMode(IrcRightEnum::Enumeration enumeration) {
		if(enumeration == IrcRightEnum::NOTHING) {
			return;
		}
		modes |= enumeration;
	}

	void removeMode(IrcRightEnum::Enumeration enumeration) {
		if(enumeration == IrcRightEnum::NOTHING) {
			return;
		}
		modes &= ~enumeration;
	}
};

struct IrcChannel {
	std::string name;
	std::string topic;

	bool nameListFull;
	std::vector<IrcChannelMember> members;
	bool existMember(const std::string nick) {
		std::vector<IrcChannelMember>::iterator iter;
		for(iter = members.begin(); iter!=members.end(); ++iter) {
			if((*iter).nick == nick) {
				return true;
			}
		}

		return false;
	}
	void removeMember(const std::string& nick) {
		std::vector<IrcChannelMember>::iterator iter;
		for(iter = members.begin(); iter!=members.end(); ++iter) {
			if((*iter).nick == nick) {
				members.erase(iter);
				return;
			}
		}
	}
	void changeMember(const std::string& nickold, const std::string& nicknew) {
		std::vector<IrcChannelMember>::iterator iter;
		for(iter = members.begin(); iter!=members.end(); ++iter) {
			if((*iter).nick == nickold) {
				(*iter).nick = nicknew;
				return;
			}
		}
	}

	IrcChannelMember* getMember(const std::string& nick) {
		std::vector<IrcChannelMember>::iterator iter;
		for(iter = members.begin(); iter!=members.end(); ++iter) {
			if((*iter).nick == nick) {
				return &(*iter);
			}
		}

		return NULL;
	}
};

#endif /* IRCSTRUCTS_H_ */
