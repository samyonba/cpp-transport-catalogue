#pragma once

#include "transport_catalogue.h"

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

			Type type = Request::Type::Stop;
			std::string name;

			bool operator==(const Request& other) const {
				return std::make_pair(type, name) == std::make_pair(other.type, other.name);
			}
		};

		void ReadStat(TransportCatalogue& catalogue, std::istream& in = std::cin, std::ostream& out = std::cout);

		void ParseRawRequest(std::string raw_request, TransportCatalogue& catalogue, std::ostream& out);

		void PrintBusInfo(const TransportCatalogue::BusInfo& info, std::ostream& out);

		void PrintStopInfo(const TransportCatalogue::StopInfo& info, std::ostream& out);
	}
}
