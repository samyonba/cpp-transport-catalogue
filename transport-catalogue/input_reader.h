#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <string>
#include <string_view>
#include <vector>
#include <iostream>

namespace Transport {

	namespace Input {
		struct Request
		{
			enum class Type
			{
				Stop,
				Bus,
			};

			// Common
			Type type = Request::Type::Stop;
			std::string name;

			// Stop
			Geo::Coordinates coords;
			std::vector<std::pair<std::string, int>> distances;

			// Bus
			std::vector<std::string> stops;

			bool operator==(const Request& other) const {
				return std::tie(type, name, coords, distances, stops) == std::tie(other.type, other.name, other.coords, other.distances, other.stops);
			}
		};

		std::vector<Request> GetRequests(std::istream& input = std::cin);

		Request ParseRawRequest(std::string_view request);
		Request& ParseStopRequest(std::string_view request, Request& result);
		Request& ParseBusRequest(std::string_view request, Request& result);

		void ReadInput(Transport::TransportCatalogue& catalogue, std::istream& in = std::cin);
	}
}
