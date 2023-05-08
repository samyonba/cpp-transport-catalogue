#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace Transport::Rendering {
	void RenderCatalogue(const Transport::TransportCatalogue& catalogue, const Transport::Rendering::RenderSettings& settings, std::ostream& out = std::cout);
}
