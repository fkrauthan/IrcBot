/*
 * AILanugageAnalyser.cpp
 *
 *  Created on: 13.07.2011
 *      Author: fkrauthan
 */

#include "AILanugageAnalyser.h"
#include "AIAnswer.h"

#include <ticpp/ticpp.h>

#include <iostream>
#include <ctime>
#include <cstdlib>


AILanugageAnalyser::AILanugageAnalyser() {
	std::srand(std::time(NULL));
}

AILanugageAnalyser::~AILanugageAnalyser() {
	cleanUpAll();
}

bool AILanugageAnalyser::loadLanguageFile(const std::string& file) {
	try {
		ticpp::Document doc(file);
		doc.LoadFile();

		ticpp::Element* pRoot = doc.FirstChildElement();

		//Load all answers
		ticpp::Element* pAnswers = pRoot->FirstChildElement("answers");
		ticpp::Iterator<ticpp::Node> child;
		for(child = child.begin(pAnswers->FirstChildElement("categories")); child!=child.end(); ++child) {
			AIAnswer* tmpAnswer = new AIAnswer((*child).ToElement());
			mAnswers[tmpAnswer->getCategory()] = tmpAnswer;
		}
	}
	catch(ticpp::Exception& ex) {
		std::cout << ex.what();
		return false;
	}

	return true;
}

std::string AILanugageAnalyser::getAnswer(const std::string& category, std::map<std::string, std::string> params) {
	if(category == "") {
		return "";
	}

	size_t pos = category.find_first_of('/');
	std::map<std::string, AIAnswer*>::iterator iter;
	if(pos == std::string::npos) {
		iter = mAnswers.find(category);
	}
	else {
		iter = mAnswers.find(category.substr(0, pos));
	}
	if(iter != mAnswers.end()) {
		if(pos == std::string::npos) {
			return iter->second->getAnswer("", params);
		}
		else {
			return iter->second->getAnswer(category.substr(pos+1), params);
		}
	}

	return "";
}

void AILanugageAnalyser::cleanUpAll() {
	std::map<std::string, AIAnswer*>::iterator iter;
	for(iter=mAnswers.begin(); iter!=mAnswers.end(); ++iter) {
		delete iter->second;
	}
	mAnswers.clear();
}
