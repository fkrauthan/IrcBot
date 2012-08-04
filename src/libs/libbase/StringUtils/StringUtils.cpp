/*
 * StringUtils.cpp
 *
 *  Created on: 29.07.2010
 *      Author: fkrauthan
 */

#include "StringUtils.h"
#include <cstdlib>
#include <sstream>
#include <ctime>
//#include <sha1/sha1.h>

using namespace Base;


void StringUtils::split(const std::string& string, const char delemiter, std::vector<std::string>& destination, bool trimEntry, bool removeEmpty) {
	std::string::size_type  last_position(0);
	std::string::size_type position(0);
	for(std::string::const_iterator it(string.begin()); it != string.end(); ++it, ++position) {
		if(*it == delemiter) {
			std::string trimmed = "";
			if(trimEntry) {
				trimmed = trim(string.substr(last_position, position - last_position));
			}
			else {
				trimmed = string.substr(last_position, position - last_position);
			}
			if(removeEmpty && trimmed == "") {
				continue;
			}
			destination.push_back(trimmed);
			last_position = position + 1;
		}
	}
	std::string trimmed = "";
	if(trimEntry) {
		trimmed = trim(string.substr(last_position, position - last_position));
	}
	else {
		trimmed = string.substr(last_position, position - last_position);
	}
	if(removeEmpty && trimmed == "") {
		return;
	}
	destination.push_back(trimmed);
}

std::string StringUtils::trim(std::string str) {
	std::string::size_type pos = str.find_first_not_of(" \t\n\r");
	str = str.erase(0, pos);
	pos = str.find_last_not_of(" \t\n\r") + 1;
	str = str.erase(pos);
	return str;
}

std::string StringUtils::ltrim(std::string str) {
	std::string::size_type pos = str.find_first_not_of(" \t\n\r");
	str = str.erase(0, pos);

	return str;
}

std::string StringUtils::rtrim(std::string str) {
	std::string::size_type pos = str.find_last_not_of(" \t\n\r") + 1;
	str = str.erase(pos);

	return str;
}

std::string& StringUtils::replaceAll(std::string& context, const std::string& from, const std::string& to) {
	size_t lookHere = 0;
	size_t foundHere;
	while((foundHere = context.find(from, lookHere)) != std::string::npos) {
		context.replace(foundHere, from.size(), to);
		lookHere = foundHere + to.size();
	}
	return context;
}

std::string StringUtils::generateKey(int minSize, int maxSize, bool readable) {
	if(maxSize < minSize) {
		return "";
	}

	std::vector<char> chars;
	for(char i='0'; i<='9'; i++) {
		if(readable && (i=='0' || i=='1')) {
			continue;
		}
		chars.push_back(i);
	}
	for(char i = 'a'; i<='z'; i++) {
		if(readable && (i=='i' || i=='l' || i=='o' || i=='j')) {
			continue;
		}
		chars.push_back(i);
	}
	for(char i = 'A'; i<='Z'; i++) {
		if(readable && (i=='I' || i=='L' || i=='O' || i=='J')) {
			continue;
		}
		chars.push_back(i);
	}
	chars.push_back('.');
	chars.push_back(';');
	if(!readable) {
		chars.push_back('!');
		chars.push_back('?');
		chars.push_back('/');
		chars.push_back('\\');
		chars.push_back('-');
		chars.push_back('_');
		chars.push_back('\'');
		chars.push_back('"');
		chars.push_back(',');
		chars.push_back(':');
		chars.push_back('|');
	}
	chars.push_back('+');
	chars.push_back('#');
	chars.push_back('*');
	chars.push_back('&');
	chars.push_back('%');
	chars.push_back('[');
	chars.push_back(']');
	chars.push_back('?');
	chars.push_back('{');
	chars.push_back('}');



	std::stringstream ret;
	std::srand(time(NULL));
	int keyLength = std::rand() % (maxSize-minSize) + minSize;
	for(int i=0; i<keyLength; i++) {
		ret << chars[std::rand() % chars.size()];
	}

	return ret.str();
}

/*std::string StringUtils::generateSHA1(const std::string& input) {
	SHA1 sha;
	sha << input.c_str();
	unsigned message_digest[5];
	if(!sha.Result(message_digest)) {
		return "";
	}
	std::stringstream str;
	std::ios::fmtflags flags;
	str.setf(std::ios::hex|std::ios::uppercase,std::ios::basefield);
	for(unsigned int i = 0; i < 5; i++) {
		str << message_digest[i];
	}

	return str.str();
}*/
