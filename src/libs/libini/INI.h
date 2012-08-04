/*
 * INI.h
 *
 *  Created on: 31.08.2010
 *      Author: fkrauthan
 */

#ifndef INI_H_
#define INI_H_

#include <string>
#include <map>
#include "Exceptions.h"


namespace libINI {
	class INI {
		public:
			INI(const std::string& file="");
			~INI();

			void parseINI(const std::string& file="", bool clearFirst = true);
			void saveINI(const std::string& file="");

			std::map<std::string, std::map<std::string, std::string> >& getEntries();
			std::map<std::string, std::string>& getSection(const std::string& name);

			bool issetSection(const std::string& name);
			bool issetValue(const std::string& section, const std::string& name);

			std::map<std::string, std::string> getSubsetOfSection(const std::string& section, const std::string& search, bool removeSearch = false);

			std::string getValue(const std::string& section, const std::string& name);

			std::string getValue(const std::string& section, const std::string& name, std::string defaultValue);
			const char* getValue(const std::string& section, const std::string& name, const char* defaultValue);
			size_t getValue(const std::string& section, const std::string& name, size_t defaultValue);
			int getValue(const std::string& section, const std::string& name, int defaultValue);
			long getValue(const std::string& section, const std::string& name, long defaultValue);
			float getValue(const std::string& section, const std::string& name, float defaultValue);
			double getValue(const std::string& section, const std::string& name, double defaultValue);
			bool getValue(const std::string& section, const std::string& name, bool defaultValue);


			void addSection(const std::string& name);
			void addValue(const std::string& section, const std::string& name, const std::string& value, bool overide=false);
			//template<typename T> void addValue(const std::string& section, const std::string& name, const T& value, bool overide=false);

			void removeSection(const std::string& name);
			void removeValue(const std::string& section, const std::string& name);

			std::string toString();

		private:
			std::string trim(std::string str);
			std::string mask(std::string str);

		private:
			std::string mFile;
			std::map<std::string, std::map<std::string, std::string> > mEntries;
	};
}

#endif /* INI_H_ */
