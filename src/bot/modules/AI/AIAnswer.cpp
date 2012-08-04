/*
 * AIAnswer.cpp
 *
 *  Created on: 13.07.2011
 *      Author: fkrauthan
 */

#include "AIAnswer.h"

#include <libbase/StringUtils/StringUtils.h>
#include <ticpp/ticpp.h>

#include <cstdlib>


AIAnswer::AIAnswer(ticpp::Element* node) {
	mCategory = node->GetAttribute("name");

	ticpp::Element* answers = node->FirstChildElement("answers", false);
	if(answers) {
		ticpp::Iterator<ticpp::Node> child;
		for(child = child.begin(answers); child!=child.end(); ++child) {
			mAnswers.push_back((*child).ToElement()->GetText());
		}
	}

	ticpp::Element* categories = node->FirstChildElement("categories", false);
	if(categories) {
		ticpp::Iterator<ticpp::Node> child;
		for(child = child.begin(categories); child!=child.end(); ++child) {
			AIAnswer* tmpAnswer = new AIAnswer((*child).ToElement());
			mChilds[tmpAnswer->getCategory()] = tmpAnswer;
		}
	}
}

AIAnswer::~AIAnswer() {
	cleanUpAll();
}

const std::string& AIAnswer::getCategory() {
	return mCategory;
}

std::string AIAnswer::getAnswer(const std::string category, std::map<std::string, std::string> params) {
	if(category == "") {
		int pos = std::rand() % mAnswers.size();
		std::string ret = mAnswers[pos];

		std::map<std::string, std::string>::iterator iter;
		for(iter=params.begin(); iter!=params.end(); ++iter) {
			ret = Base::StringUtils::replaceAll(ret, iter->first, iter->second);
		}

		return ret;
	}
	else {
		size_t pos = category.find_first_of('/');
		std::map<std::string, AIAnswer*>::iterator iter;
		if(pos == std::string::npos) {
			iter = mChilds.find(category);
		}
		else {
			iter = mChilds.find(category.substr(0, pos));
		}
		if(iter != mChilds.end()) {
			if(pos == std::string::npos) {
				return iter->second->getAnswer("", params);
			}
			else {
				return iter->second->getAnswer(category.substr(pos+1), params);
			}
		}

		return "";
	}
}

void AIAnswer::cleanUpAll() {
	std::map<std::string, AIAnswer*>::iterator iter;
	for(iter=mChilds.begin(); iter!=mChilds.end(); ++iter) {
		delete iter->second;
	}
	mChilds.clear();

	mAnswers.clear();
}
