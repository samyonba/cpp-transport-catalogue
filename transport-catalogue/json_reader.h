#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "router.h"

#include <iostream>

namespace Transport {

	class JsonReader {
	public:
		JsonReader(TransportCatalogue& catalogue, std::istream& in = std::cin, std::ostream& out = std::cout)
			: catalogue_(catalogue), in_(in), out_(out) {}

		void ReadInput();
		void ReadMakeBaseInput();
		void ReadProcessRequest();
		void ProcessStatRequests(Routing::TransportRouter& router);
		void ProcessStatRequests(const Routing::LightTransportRouter& router);

		std::string GetSerializationFileName() const;
		const Rendering::RenderSettings& GetRenderSettings() const;
		const Routing::RouterSettings& GetRouterSettings() const;
		void SetRenderSettings(Transport::Rendering::RenderSettings settings);

	private:
		void ReadRenderSettings(const json::Dict& attributes);
		void ReadBaseRequests(const json::Array& base_requests);
		void ReadRouterSettings(const json::Dict& attributes);
		void ReadStatRequests(const json::Array& stat_requests);
		void ReadStatRequests(const json::Array& stat_requests, const Routing::LightTransportRouter& router);
		void ReadSerializationSettings(const json::Dict& attributes);

		void ReadStop(const json::Dict& attributes);
		void ReadDistances(const json::Dict& attributes);
		void ReadBus(const json::Dict& attributes);

		void PrintJsonStopInfo(const StopInfo& info, int request_id);
		void PrintJsonBusInfo(const BusInfo& info, int request_id);
		void PrintJsonMap(int request_id);
		void PrintJsonRoute(const std::string_view from, const std::string_view to, int request_id, Routing::TransportRouter& router);
		void PrintJsonRoute(const std::string_view from, const std::string_view to, int request_id, const Routing::LightTransportRouter& router);

	private:
		Rendering::RenderSettings render_settings_;
		Routing::RouterSettings routing_settings_;
		std::string serialization_settings_;
		TransportCatalogue& catalogue_;
		std::istream& in_;
		std::ostream& out_;

		json::Array saved_stat_requests_;
	};
}
