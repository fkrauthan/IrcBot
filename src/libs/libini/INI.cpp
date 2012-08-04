/*
 * INI.cpp
 *
 *  Created on: 31.08.2010
 *      Author: fkrauthan
 */

#include "INI.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace libINI;
enum ini_state {
	inFile,
	inSection,
	inComment,
	inName,
	inValue,
};

INI::INI(const std::string& file) : mFile(file) {
	if(!mFile.empty()) {
		parseINI();
	}
}

INI::~INI() {
}

void INI::parseINI(const std::string& file, bool clearFirst) {
	if(clearFirst) {
		mEntries.clear();
	}


	if(!file.empty()) {
		mFile = file;
	}

	if(mFile.empty()) {
		throw NoFilenameException("There is no filename to parse the ini file");
	}


	std::ifstream ifs(mFile.c_str());
	if(!ifs.is_open()) {
		throw FileNotFoundException("Can't open ini file \""+mFile+"\"");
	}

	char myChar;



	std::string sectionName = "";
	std::string valueName = "";
	ini_state myState = inFile;
	bool isQuote = false;

	std::stringstream tmpString;
	while(ifs.good()) {
		myChar = ifs.get();
		if(ifs.eof()) {
			break;
		}
		switch(myState) {
			case inFile:
				if(myChar == '[') {
					myState = inSection;
					tmpString.str("");
					continue;
				}
				else if(myChar == ';') {
					myState = inComment;
				}
				else if((myChar >= 'a' && myChar <= 'z') || (myChar >= 'A' && myChar <= 'Z') || myChar == '"') {
					if(myChar == '"') {
						isQuote = true;
					}
					myState = inName;
					tmpString.str("");
					tmpString << myChar;
				}
				else if(myChar != '\n' && myChar != ' ' && myChar != '\r' && myChar != '\t') {
					throw ParseException("There is a not allowed char on value name beginning");
				}
				break;
			case inSection:
				if(isQuote) {
					if(myChar == '"') {
						isQuote = false;
					}
					else if(myChar == '\\' && ifs.peek() == '"') {
						tmpString.put(ifs.get());
					}
					else {
						tmpString.put(myChar);
					}
				}
				else if(myChar == '"') {
					isQuote = true;
				}
				else if(myChar == '\n') {
					throw ParseException("There is a new line on sections name definition");
				}
				else {
					if(myChar == ']') {
						sectionName = tmpString.str();
						if(sectionName.substr(0, 7) == "include") {
							sectionName = sectionName.substr(8);
							if(sectionName.empty()) {
								throw ParseException("There is no include name added");
							}

							if(sectionName[0] != '/') {
								std::string::size_type pos = file.find_last_of('/');
								if(pos != std::string::npos) {
									sectionName = file.substr(0, pos)+"/"+sectionName;
								}
							}

							std::string fileName = mFile;
							parseINI(sectionName, false);
							mFile = fileName;
						}
						else {
							addSection(sectionName);
						}

						myState = inFile;
						ifs.ignore(255,'\n');
					}
					else {
						tmpString.put(myChar);
					}
				}
				break;
			case inName:
				if(isQuote) {
					if(myChar == '"') {
						isQuote = false;
					}
					else if(myChar == '\\' && ifs.peek() == '"') {
						tmpString.put(ifs.get());
					}
					else {
						tmpString.put(myChar);
					}
				} else if(myChar == '"') {
					isQuote = true;
				} else {
					if(myChar == '=') {
						valueName = trim(tmpString.str());
						myState = inValue;
						tmpString.str("");
					}
					else if(myChar == '\n') {
						throw ParseException("There is a new line on key name definition");
					}
					else {
						tmpString.put(myChar);
					}
				}
				break;
			case inValue:
				if(isQuote) {
					if(myChar == '"') {
						isQuote = false;
					}
					else if(myChar == '\\' && ifs.peek() == '"') {
						tmpString.put(ifs.get());
					}
					else {
						tmpString.put(myChar);
					}
				}
				else if(myChar == '"') {
					isQuote = true;
				}
				else {
					if(myChar == '\n' || myChar == ';') {
						if(sectionName.empty()) {
							throw ParseException("There is no section name");
						}
						if(valueName.empty()) {
							throw ParseException("There is no key name");
						}
						addValue(sectionName, valueName, trim(tmpString.str()), true);
						tmpString.str("");

						if(myChar == ';') {
							myState = inComment;
						}
						else {
							myState = inFile;
						}
					}
					else {
						tmpString.put(myChar);
					}
				}
				break;
			case inComment:
				if(myChar == '\n') {
					myState = inFile;
				}
				break;
		}
	}
	if(myState == inSection || myState == inName) {
		throw ParseException("There is a not closed section or key name definition");
	}
	else if(myState == inValue) {
		if(isQuote) {
			throw ParseException("There open quotes at the end of file");
		}
		else {
			if(sectionName.empty()) {
				throw ParseException("There is no section name");
			}
			if(valueName.empty()) {
				throw ParseException("There is no key name");
			}
			addValue(sectionName, valueName, trim(tmpString.str()), true);
			myState = inFile;
			tmpString.str("");
		}
	}
	ifs.close();
}

void INI::saveINI(const std::string& file) {
	if(!file.empty()) {
		mFile = file;
	}

	if(mFile.empty()) {
		throw NoFilenameException("There is no filename to save the ini file");
	}


	std::ofstream out;
	out.open(mFile.c_str(), std::ios::out);
	if(!out.is_open()) {
		throw CantSaveFileException("Can't save the ini with the filename  \""+mFile+"\"");
	}


	std::map<std::string, std::map<std::string, std::string> >::iterator iter;
	for(iter=mEntries.begin(); iter!=mEntries.end(); ++iter) {
		out << "[" << iter->first << "]\n";
		std::map<std::string, std::string>::iterator iter2;
		for(iter2=iter->second.begin(); iter2!=iter->second.end(); ++iter2) {
			out << iter2->first << "=" << mask(iter2->second) << "\n";
		}
	}
}

std::map<std::string, std::map<std::string, std::string> >& INI::getEntries() {
	return mEntries;
}

std::map<std::string, std::string>& INI::getSection(const std::string& name) {
	std::map<std::string, std::map<std::string, std::string> >::iterator iter = mEntries.find(name);
	if(iter==mEntries.end()) {
		SectionNotFoundException("There is no section with name \""+name+"\"");
	}

	return iter->second;
}

bool INI::issetSection(const std::string& name) {
	std::map<std::string, std::map<std::string, std::string> >::iterator iter = mEntries.find(name);
	if(iter==mEntries.end()) {
		return false;
	}

	return true;
}

bool INI::issetValue(const std::string& section, const std::string& name) {
	if(!issetSection(section)) {
		return false;
	}

	std::map<std::string, std::string>::iterator iter = mEntries[section].find(name);
	if(iter==mEntries[section].end()) {
		return false;
	}

	return true;
}

std::map<std::string, std::string> INI::getSubsetOfSection(const std::string& section, const std::string& search, bool removeSearch) {
	std::map<std::string, std::string> ret;
	if(!issetSection(section)) {
		return ret;
	}

	std::string::size_type nameLength = search.size();
	std::map<std::string, std::string>::iterator iter;
	for(iter=mEntries[section].begin(); iter!=mEntries[section].end(); ++iter) {
		if(iter->first.substr(0, nameLength) == search) {
			if(removeSearch) {
				ret[iter->first.substr(nameLength)] = iter->second;
			}
			else {
				ret[iter->first] = iter->second;
			}
		}
	}

	return ret;
}

std::string INI::getValue(const std::string& section, const std::string& name) {
	if(!issetValue(section, name)) {
		ValueNotFoundException("There is no value on section \""+section+"\" with name \""+name+"\"");
	}

	return mEntries[section][name];
}

std::string INI::getValue(const std::string& section, const std::string& name, std::string defaultValue) {
	if(!issetValue(section, name)) {
		return defaultValue;
	}

	return mEntries[section][name];
}

const char* INI::getValue(const std::string& section, const std::string& name, const char* defaultValue) {
	return getValue(section, name, std::string(defaultValue)).c_str();
}


#define GETVALUE(type) type INI::getValue(const std::string& section, const std::string& name, type defaultValue) { \
	std::stringstream str; \
	std::stringstream str2;	\
	str2 << defaultValue; \
	str << getValue(section, name, str2.str()); \
	type ret; \
	str >> ret; \
	return ret; \
}

GETVALUE(size_t)
GETVALUE(int)
GETVALUE(long)
GETVALUE(float)
GETVALUE(double)
GETVALUE(bool)

#undef GETVALUE


void INI::addSection(const std::string& name) {
	if(issetSection(name)) {
		return;
	}

	mEntries[name] = std::map<std::string, std::string>();
}

void INI::addValue(const std::string& section, const std::string& name, const std::string& value, bool overide) {
	if(!overide && issetValue(section, name)) {
		return;
	}

	mEntries[section][name] = value;
}

void INI::removeSection(const std::string& name) {
	std::map<std::string, std::map<std::string, std::string> >::iterator iter = mEntries.find(name);
	if(iter!=mEntries.end()) {
		mEntries.erase(iter);
	}
}

void INI::removeValue(const std::string& section, const std::string& name) {
	if(!issetSection(section)) {
		return;
	}

	std::map<std::string, std::string>::iterator iter = mEntries[section].find(name);
	if(iter!=mEntries[section].end()) {
		mEntries[section].erase(iter);
	}
}

std::string INI::toString() {
	std::stringstream str;
	std::map<std::string, std::map<std::string, std::string> >::iterator iter;
	for(iter=mEntries.begin(); iter!=mEntries.end(); ++iter) {
		str << "[" << iter->first << "]\n";
		std::map<std::string, std::string>::iterator iter2;
		for(iter2=iter->second.begin(); iter2!=iter->second.end(); ++iter2) {
			str << iter2->first << "=" << mask(iter2->second) << "\n";
		}
	}

	return str.str();
}

std::string INI::trim(std::string str) {
	std::string::size_type pos = str.find_first_not_of(" \t\n\r");
	str = str.erase(0, pos);
	pos = str.find_last_not_of(" \t\n\r") + 1;
	str = str.erase(pos);
	return str;
}

std::string INI::mask(std::string str) {
	int pointCount = 0;
	bool isNumeric = true;
	for(unsigned int i=0; i<str.size(); i++) {
		if(!std::isdigit(str[i])) {
			if(str[i] == '.') {
				pointCount++;
			}
			else {
				isNumeric = false;
				break;
			}
		}
	}
	if(pointCount > 1) {
		isNumeric = false;
	}

	if(isNumeric) {
		return str;
	}


	size_t lookHere = 0;
	size_t foundHere;
	while((foundHere = str.find('"', lookHere)) != std::string::npos) {
		str.replace(foundHere, 1, "\\\"");
		lookHere = foundHere + 2;
	}

	return "\""+str+"\"";
}
