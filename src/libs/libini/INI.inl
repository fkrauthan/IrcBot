
#include <sstream>

namespace libINI {
	template<> const char* INI::getValue<const char*>(const std::string& section, const std::string& name) {
		return getValue(section, name).c_str();
	}
	
#define GETVALUE(type) template<> type INI::getValue<type>(const std::string& section, const std::string& name) { \
		std::stringstream str; \
		str << getValue(section, name); \
		type ret; \
		str >> ret; \
		return ret; \
	}
	
	GETVALUE(size_t)
	GETVALUE(int)
	GETVALUE(long)
	GETVALUE(float)
	GETVALUE(double)
	
#undef GETVALUE
}
		