/*
 * IRCChannel.h
 *
 *  Created on: 23.07.2011
 *      Author: fkrauthan
 */

#ifndef IRCCHANNEL_H_
#define IRCCHANNEL_H_

#include <string>
#include <map>
#include <cstdint>


struct IRCUserModeEnum {
	enum Enumeration {
		UNKOWN = -1,
		NOTHING = 0,
		VOICE = 1,
		HALF_OP = 2,
		OP = 4
	};

	static std::string toString(IRCUserModeEnum::Enumeration enumeration) {
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

struct IRCMember {
	std::string nick;
	std::uint64_t modes;

	IRCMember() {
		modes = 0;
	}


	bool hasMode(IRCUserModeEnum::Enumeration enumeration) {
		if(enumeration <= IRCUserModeEnum::NOTHING) {
			return true;
		}
		return ((modes & enumeration) != 0);
	}

	void addMode(IRCUserModeEnum::Enumeration enumeration) {
		if(enumeration <= IRCUserModeEnum::NOTHING) {
			return;
		}
		modes |= enumeration;
	}

	void removeMode(IRCUserModeEnum::Enumeration enumeration) {
		if(enumeration <= IRCUserModeEnum::NOTHING) {
			return;
		}
		modes &= ~enumeration;
	}

};

struct IRCChannel {
	std::string name;
	std::string topic;


	bool nameListFull;
	std::map<std::string, IRCMember> members;
	IRCMember* getMember(const std::string& nick) {
		std::map<std::string, IRCMember>::iterator iter = members.find(nick);
		if(iter != members.end()) {
			return &iter->second;
		}
		return NULL;
	}

	void removeMember(const std::string& nick) {
		std::map<std::string, IRCMember>::iterator iter = members.find(nick);
		if(iter != members.end()) {
			members.erase(iter);
		}
	}

	void changeMember(const std::string& nickold, const std::string& nicknew) {
		std::map<std::string, IRCMember>::iterator iter = members.find(nickold);
		if(iter != members.end()) {
			members[nicknew] = iter->second;
			members[nicknew].nick = nicknew;

			members.erase(iter);
		}
	}
};

#endif /* IRCCHANNEL_H_ */
