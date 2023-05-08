#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>

namespace Transport {

	class JsonReader {
	public:
		JsonReader(TransportCatalogue& catalogue, std::istream& in = std::cin, std::ostream& out = std::cout)
			: catalogue_(catalogue), in_(in), out_(out) {}

		void ReadInput();

	private:
		void ReadRenderSettings(const json::Dict& attributes);
		void ReadBaseRequests(const json::Array& base_requests);
		void ReadStatRequests(const json::Array& stat_requests);

		void ReadStop(const json::Dict& attributes);
		void ReadDistances(const json::Dict& attributes);
		void ReadBus(const json::Dict& attributes);

		void PrintJsonStopInfo(const StopInfo& info, int request_id);
		void PrintJsonBusInfo(const BusInfo& info, int request_id);
		void PrintJsonMap(int request_id);

	private:
		Rendering::RenderSettings settings_;
		TransportCatalogue& catalogue_;
		std::istream& in_;
		std::ostream& out_;
	};
}
