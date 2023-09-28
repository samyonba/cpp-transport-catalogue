#pragma once

#include <string_view>
#include <transport_catalogue.pb.h>
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
	void SerializeTransportCatalogue(const TransportCatalogue& catalogue, std::string filename,
		const Rendering::RenderSettings& render_settings,
		const Routing::RouterSettings& router_settings);

	Routing::LightTransportRouter DeserializeTransportCatalogue(std::string filename,
		Transport::TransportCatalogue& catalogue, Rendering::RenderSettings& render_settings);
}
