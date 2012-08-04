/*
 * IRCExceptions.h
 *
 *  Created on: 23.07.2011
 *      Author: fkrauthan
 */

#ifndef IRCEXCEPTIONS_H_
#define IRCEXCEPTIONS_H_

#include <stdexcept>
#include <string>


class IRCException : public std::runtime_error {
	public:
		IRCException(const std::string& message) : std::runtime_error(message) {}
};

class IRCConnectException : public IRCException {
	public:
		IRCConnectException(const std::string& message) : IRCException(message) {}
};

#endif /* IRCEXCEPTIONS_H_ */
