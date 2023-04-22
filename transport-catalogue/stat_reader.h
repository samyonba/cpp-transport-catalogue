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
			std::string text;

			bool operator==(const Request& other) const {
				return std::make_pair(type, text) == std::make_pair(other.type, other.text);
			}
		};


		Request ParseRawRequest(std::string raw_request);
		std::vector<Request> GetRequests(std::istream& input = std::cin);
	}
}
