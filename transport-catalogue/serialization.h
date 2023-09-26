#pragma once

#include <string_view>
#include <C:/Users/1/Documents/Practicum/Projects/TransportCatalogue_CMake/build/transport_catalogue.pb.h>
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace Transport {
    namespace Rendering {
        class RenderSettings;
    }
    namespace Routing {
        class RouterSettings;
        class LightTransportRouter;
    }
}

using namespace Transport;

namespace serialization {
	void SerializeTransportCatalogue(const TransportCatalogue& catalogue,
		std::string_view filename,
		const Rendering::RenderSettings& render_settings,
		const Routing::RouterSettings& router_settings);

	Routing::LightTransportRouter DeserializeTransportCatalogue(std::string_view filename,
		Transport::TransportCatalogue& catalogue, Rendering::RenderSettings& render_settings);
}
