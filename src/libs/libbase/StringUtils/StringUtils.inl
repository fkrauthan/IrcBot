
#include <sstream>


namespace Base {
	template<typename T> std::string StringUtils::toString(const T& var) {
		std::stringstream str;
		str << var;
		return str.str();
	}
		
	template<typename T> T StringUtils::fromString(const std::string& var) {
		T tmpVar;
		std::stringstream str;
		str << var;
		str >> tmpVar;
			
		return tmpVar;
	}
}
