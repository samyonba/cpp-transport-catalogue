#include "request_handler.h"

using namespace Transport;
using namespace Rendering;

SphereProjector MakeProjector(const TransportCatalogue& catalogue, const RenderSettings& settings) {
	std::vector<Geo::Coordinates> all_coords;
	for (const Stop* stop : catalogue.GetStops()) {
		const auto buses_ptr = catalogue.GetBusesForStop(stop);
		if (!buses_ptr.empty())
		{
			all_coords.push_back(stop->coords);
		}
	}
	return SphereProjector(all_coords.begin(), all_coords.end(), settings.width, settings.height, settings.padding);
}

void Transport::Rendering::RenderCatalogue(const TransportCatalogue& catalogue, const RenderSettings& settings, std::ostream& out)
{
	auto projector = MakeProjector(catalogue, settings);
	MapRenderer renderer(catalogue, projector, settings, out);

	renderer.Render();
}
