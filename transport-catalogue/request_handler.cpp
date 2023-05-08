#include "request_handler.h"

using namespace Transport;
using namespace Rendering;

void AddRoutes(const TransportCatalogue& catalogue, MapRenderer& renderer,
	const SphereProjector& projector, const RenderSettings& settings) {
	std::vector<const Bus*> routes;
	for (const auto& bus : catalogue.GetBuses()) {
		routes.push_back(&bus);
	}
	std::sort(routes.begin(), routes.end(), [](const Bus* lhv, const Bus* rhv) {
		return lhv->name < rhv->name;
		});
	for (const auto bus : routes) {
		// сначала вызывается AddRouteTitle, т.к. AddRoute() изменит счетчик маршрутов
		renderer.AddRouteTitle(*bus, projector, settings);
		renderer.AddRoute(*bus, projector, settings);
	}
}

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

std::vector<const Stop*> GetStopsToDraw(const TransportCatalogue& catalogue) {
	std::vector<const Stop*> stops_to_draw;
	const auto all_stops = catalogue.GetStops();
	std::copy_if(all_stops.begin(), all_stops.end(), std::back_inserter(stops_to_draw),
		[&catalogue](const Stop* stop) {
			return !(catalogue.GetBusesForStop(stop).empty());
		});
	std::sort(stops_to_draw.begin(), stops_to_draw.end(),
		[](const Stop* lhv, const Stop* rhv) {
			return lhv->name < rhv->name;
		});
	return stops_to_draw;
}

void AddStops(const TransportCatalogue& catalogue, MapRenderer& renderer,
	const SphereProjector& projector, const RenderSettings& settings) {
	const auto& stops_to_draw = GetStopsToDraw(catalogue);

	for (const Stop* stop : stops_to_draw) {
		renderer.AddStopCircle(*stop, projector, settings);
	}

	for (const Stop* stop : stops_to_draw) {
		renderer.AddStopTitle(*stop, projector, settings);
	}
}

void Transport::Rendering::RenderCatalogue(const TransportCatalogue& catalogue, const RenderSettings& settings, std::ostream& out)
{
	MapRenderer renderer;
	auto projector = MakeProjector(catalogue, settings);

	AddRoutes(catalogue, renderer, projector, settings);
	AddStops(catalogue, renderer, projector, settings);

	renderer.Render(out);
}
