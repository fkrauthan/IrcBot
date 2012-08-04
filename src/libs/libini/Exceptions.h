/*
 * Exceptions.h
 *
 *  Created on: 31.08.2010
 *      Author: fkrauthan
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <stdexcept>
#include <string>


namespace libINI {
	class INIException : public std::runtime_error {
		public:
			INIException(const std::string& msg) : std::runtime_error(msg) {}
	};

	class FileNotFoundException : public INIException {
		public:
			FileNotFoundException(const std::string& msg) : INIException(msg) {}
	};

	class NoFilenameException : public INIException {
		public:
			NoFilenameException(const std::string& msg) : INIException(msg) {}
	};

	class ParseException : public INIException {
		public:
			ParseException(const std::string& msg) : INIException(msg) {}
	};

	class CantSaveFileException : public INIException {
		public:
			CantSaveFileException(const std::string& msg) : INIException(msg) {}
	};

	class SectionNotFoundException : public INIException {
		public:
			SectionNotFoundException(const std::string& msg) : INIException(msg) {}
	};

	class ValueNotFoundException : public INIException {
		public:
			ValueNotFoundException(const std::string& msg) : INIException(msg) {}
	};
}

#endif /* EXCEPTIONS_H_ */
