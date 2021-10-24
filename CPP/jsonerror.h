#ifndef JSONERROR_H
#define JSONERROR_H

#include <stdexcept>

namespace DianaJSON {

	class JsonException : public std::runtime_error {
	public:
		explicit JsonException(const std::string& errMsg) : std::runtime_error(errMsg) {}

	public:
		const char* what() const noexcept override { return std::runtime_error::what(); }
	};

}// namespace DianaJSON

#endif