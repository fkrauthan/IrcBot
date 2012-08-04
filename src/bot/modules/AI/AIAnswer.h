/*
 * AIAnswer.h
 *
 *  Created on: 13.07.2011
 *      Author: fkrauthan
 */

#ifndef AIANSWER_H_
#define AIANSWER_H_

#include <string>
#include <map>
#include <vector>

namespace ticpp {
	class Element;
}


class AIAnswer {
	public:
		AIAnswer(ticpp::Element* node);
		~AIAnswer();

		const std::string& getCategory();
		std::string getAnswer(const std::string category, std::map<std::string, std::string> params);

	private:
		void cleanUpAll();

	private:
		std::string mCategory;

		std::vector<std::string> mAnswers;
		std::map<std::string, AIAnswer*> mChilds;
};

#endif /* AIANSWER_H_ */
