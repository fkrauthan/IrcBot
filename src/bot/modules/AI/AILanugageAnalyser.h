/*
 * AILanugageAnalyser.h
 *
 *  Created on: 13.07.2011
 *      Author: fkrauthan
 */

#ifndef AILANUGAGEANALYSER_H_
#define AILANUGAGEANALYSER_H_

#include <string>
#include <map>

class AIAnswer;


class AILanugageAnalyser {
	public:
		AILanugageAnalyser();
		~AILanugageAnalyser();

		bool loadLanguageFile(const std::string& file);


		std::string getAnswer(const std::string& category, std::map<std::string, std::string> params=std::map<std::string, std::string>());

	private:
		void cleanUpAll();

	private:
		std::map<std::string, AIAnswer*> mAnswers;
};

#endif /* AILANUGAGEANALYSER_H_ */
