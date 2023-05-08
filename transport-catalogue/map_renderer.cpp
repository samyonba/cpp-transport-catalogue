#include "map_renderer.h"

using namespace Transport;
using namespace Rendering;

void MapRenderer::AddRoute(const Bus& route, const SphereProjector& projector, const RenderSettings& settings)
{
	if (route.stops.empty())
	{
		return;
	}

	svg::Polyline path;
	for (const Stop* stop : route.stops) {
		path.AddPoint(projector(stop->coords));
	}
	path.SetFillColor("none");
	path.SetStrokeColor(settings.color_palette[routes_count_ % settings.color_palette.size()]);
	if (!route.stops.empty()) {
		routes_count_++;
	}
	path.SetStrokeWidth(settings.line_width);
	path.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	path.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	canvas_.Add(path);
}

void MapRenderer::AddRouteTitle(const Bus& route, const SphereProjector& projector, const RenderSettings& settings)
{
	if (route.stops.empty())
	{
		return;
	}

	svg::Text background;
	background.SetPosition(projector(route.stops.back()->coords));
	background.SetOffset(settings.bus_label_offset);
	background.SetFontSize(settings.bus_label_font_size);
	background.SetFontFamily("Verdana");
	background.SetFontWeight("bold");
	background.SetData(route.name);
	background.SetFillColor(settings.underlayer_color);
	background.SetStrokeColor(settings.underlayer_color);
	background.SetStrokeWidth(settings.underlayer_width);
	background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	svg::Text title;
	title.SetPosition(projector(route.stops.back()->coords));
	title.SetOffset(settings.bus_label_offset);
	title.SetFontSize(settings.bus_label_font_size);
	title.SetFontFamily("Verdana");
	title.SetFontWeight("bold");
	title.SetData(route.name);
	// счетчик маршрутов увеличивается в методе AddRoute(), который из вне вызывается вместе с текущим методом 
	title.SetFillColor(settings.color_palette[routes_count_ % settings.color_palette.size()]);

	bool is_roundtrip = route.is_roundtrip;
	svg::Text background_add, title_add;
	if (!is_roundtrip && route.stops[route.stops.size() / 2] != route.stops.front())
	{
		background_add = background;
		background_add.SetPosition(projector(route.stops[route.stops.size()/2]->coords));
		title_add = title;
		title_add.SetPosition(projector(route.stops[route.stops.size() / 2]->coords));
	}

	route_titles_.push_back(std::move(background));
	route_titles_.push_back(std::move(title));
	if (!is_roundtrip && route.stops[route.stops.size() / 2] != route.stops.front())
	{
		route_titles_.push_back(std::move(background_add));
		route_titles_.push_back(std::move(title_add));
	}
}

void MapRenderer::AddStopCircle(const Stop& stop, const SphereProjector& projector, const RenderSettings& settings)
{
	svg::Circle circle;
	circle.SetCenter(projector(stop.coords));
	circle.SetRadius(settings.stop_radius);
	circle.SetFillColor("white");

	stop_circles_.push_back(circle);
}

void MapRenderer::AddStopTitle(const Stop& stop, const SphereProjector& projector, const RenderSettings& settings)
{
	svg::Text background;
	background.SetPosition(projector(stop.coords));
	background.SetOffset(settings.stop_label_offset);
	background.SetFontSize(settings.stop_label_font_size);
	background.SetFontFamily("Verdana");
	background.SetData(stop.name);
	background.SetFillColor(settings.underlayer_color);
	background.SetStrokeColor(settings.underlayer_color);
	background.SetStrokeWidth(settings.underlayer_width);
	background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	svg::Text title;
	title.SetPosition(projector(stop.coords));
	title.SetOffset(settings.stop_label_offset);
	title.SetFontSize(settings.stop_label_font_size);
	title.SetFontFamily("Verdana");
	title.SetData(stop.name);
	title.SetFillColor("black");

	stop_titles_.push_back(background);
	stop_titles_.push_back(title);
}


void MapRenderer::Render(std::ostream& out)
{
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

	canvas_.Render(out);
}
