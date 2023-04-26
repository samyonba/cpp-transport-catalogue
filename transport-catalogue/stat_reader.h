#pragma once

#include <string>
#include <vector>
#include <iostream>

namespace Transport {

	namespace Stat {
		struct Request
		{
			enum class Type
			{
				Stop,
				Bus
			};

			Type type;
			std::string name;

			bool operator==(const Request& other) const {
				return std::make_pair(type, name) == std::make_pair(other.type, other.name);
			}
		};

		Request ParseRawRequest(std::string raw_request);
		std::vector<Request> GetRequests(std::istream& input = std::cin);
	}
}
