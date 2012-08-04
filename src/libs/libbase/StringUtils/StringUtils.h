/*
 * StringUtils.h
 *
 *  Created on: 29.07.2010
 *      Author: fkrauthan
 */

#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <string>
#include <vector>


namespace Base {
	class StringUtils {
		public:
			static void split(const std::string& string, const char delemiter, std::vector<std::string>& destination, bool trimEntry = false, bool removeEmpty = false);

			static std::string trim(std::string str);
			static std::string ltrim(std::string str);
			static std::string rtrim(std::string str);

			static std::string& replaceAll(std::string& context, const std::string& from, const std::string& to);

			static std::string generateKey(int minSize, int maxSize, bool readable = false);
			//static std::string generateSHA1(const std::string& input);

			template<typename T> static std::string toString(const T& var);
			template<typename T> static T fromString(const std::string& var);
	};
}


#include "StringUtils.inl"

#endif /* STRINGUTILS_H_ */
