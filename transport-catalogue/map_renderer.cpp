#include "map_renderer.h"

using namespace Transport;
using namespace Rendering;

void MapRenderer::Render()
{
	AddRoutes();
	AddStops();

	for (const auto& polyline : routes_) {
		canvas_.Add(polyline);
	}
	routes_.clear();

	for (const auto& text : route_titles_) {
		canvas_.Add(text);
	}
	route_titles_.clear();

	for (const auto& circle : stop_circles_) {
		canvas_.Add(circle);
	}
	stop_circles_.clear();

	for (const auto& text : stop_titles_) {
		canvas_.Add(text);
	}
	stop_titles_.clear();

	canvas_.Render(out_);
}

void MapRenderer::AddRoutes()
{
	std::vector<const Bus*> routes;
	for (const auto& bus : catalogue_.GetBuses()) {
		routes.push_back(&bus);
	}
	std::sort(routes.begin(), routes.end(), [](const Bus* lhv, const Bus* rhv) {
		return lhv->name < rhv->name;
		});
	for (const auto bus : routes) {
		// сначала вызывается AddRouteTitle, т.к. AddRoute() изменит счетчик маршрутов
		AddRouteTitle(*bus);
		AddRoute(*bus);
	}
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

void MapRenderer::AddStops()
{
	const auto& stops_to_draw = GetStopsToDraw(catalogue_);

	for (const Stop* stop : stops_to_draw) {
		AddStopCircle(*stop);
	}

	for (const Stop* stop : stops_to_draw) {
		AddStopTitle(*stop);
	}
}

void MapRenderer::AddRoute(const Bus& route)
{
	if (route.stops.empty())
	{
		return;
	}

	svg::Polyline path;
	for (const Stop* stop : route.stops) {
		path.AddPoint(projector_(stop->coords));
	}
	path.SetFillColor("none");
	path.SetStrokeColor(settings_.color_palette[routes_count_ % settings_.color_palette.size()]);
	if (!route.stops.empty()) {
		routes_count_++;
	}
	path.SetStrokeWidth(settings_.line_width);
	path.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	path.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	routes_.push_back(std::move(path));
}

void MapRenderer::AddRouteTitle(const Bus& route)
{
	if (route.stops.empty())
	{
		return;
	}

	svg::Text background;
	background.SetPosition(projector_(route.stops.back()->coords));
	background.SetOffset(settings_.bus_label_offset);
	background.SetFontSize(settings_.bus_label_font_size);
	background.SetFontFamily("Verdana");
	background.SetFontWeight("bold");
	background.SetData(route.name);
	background.SetFillColor(settings_.underlayer_color);
	background.SetStrokeColor(settings_.underlayer_color);
	background.SetStrokeWidth(settings_.underlayer_width);
	background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	svg::Text title;
	title.SetPosition(projector_(route.stops.back()->coords));
	title.SetOffset(settings_.bus_label_offset);
	title.SetFontSize(settings_.bus_label_font_size);
	title.SetFontFamily("Verdana");
	title.SetFontWeight("bold");
	title.SetData(route.name);
	// счетчик маршрутов увеличивается в методе AddRoute(), который из вне вызывается вместе с текущим методом 
	title.SetFillColor(settings_.color_palette[routes_count_ % settings_.color_palette.size()]);

	bool is_roundtrip = route.is_roundtrip;
	svg::Text background_add, title_add;
	if (!is_roundtrip && route.stops[route.stops.size() / 2] != route.stops.front())
	{
		background_add = background;
		background_add.SetPosition(projector_(route.stops[route.stops.size()/2]->coords));
		title_add = title;
		title_add.SetPosition(projector_(route.stops[route.stops.size() / 2]->coords));
	}

	route_titles_.push_back(std::move(background));
	route_titles_.push_back(std::move(title));
	if (!is_roundtrip && route.stops[route.stops.size() / 2] != route.stops.front())
	{
		route_titles_.push_back(std::move(background_add));
		route_titles_.push_back(std::move(title_add));
	}
}

void MapRenderer::AddStopCircle(const Stop& stop)
{
	svg::Circle circle;
	circle.SetCenter(projector_(stop.coords));
	circle.SetRadius(settings_.stop_radius);
	circle.SetFillColor("white");

	stop_circles_.push_back(std::move(circle));
}

void MapRenderer::AddStopTitle(const Stop& stop)
{
	svg::Text background;
	background.SetPosition(projector_(stop.coords));
	background.SetOffset(settings_.stop_label_offset);
	background.SetFontSize(settings_.stop_label_font_size);
	background.SetFontFamily("Verdana");
	background.SetData(stop.name);
	background.SetFillColor(settings_.underlayer_color);
	background.SetStrokeColor(settings_.underlayer_color);
	background.SetStrokeWidth(settings_.underlayer_width);
	background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	svg::Text title;
	title.SetPosition(projector_(stop.coords));
	title.SetOffset(settings_.stop_label_offset);
	title.SetFontSize(settings_.stop_label_font_size);
	title.SetFontFamily("Verdana");
	title.SetData(stop.name);
	title.SetFillColor("black");

	stop_titles_.push_back(std::move(background));
	stop_titles_.push_back(std::move(title));
}
