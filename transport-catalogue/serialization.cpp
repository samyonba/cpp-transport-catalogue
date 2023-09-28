#include <fstream>
#include <unordered_map>
#include <sstream>
#include <variant>

#include "serialization.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "router.h"

tc_serialization::Color SerializeColor(const svg::Color& color) {
	tc_serialization::Color color_serialized;
	if (std::holds_alternative<std::monostate>(color))
	{
		color_serialized.set_type("monostate");
	}
	else if (std::holds_alternative<std::string>(color))
	{
		color_serialized.set_type("string");
		color_serialized.set_string_color(std::get<std::string>(color));
	}
	else if (std::holds_alternative<svg::Rgb>(color))
	{
		color_serialized.set_type("rgb");
		auto rgb_color = std::get<svg::Rgb>(color);
		color_serialized.set_r(rgb_color.red);
		color_serialized.set_g(rgb_color.green);
		color_serialized.set_b(rgb_color.blue);
	}
	else if (std::holds_alternative<svg::Rgba>(color))
	{
		color_serialized.set_type("rgba");
		auto rgba_color = std::get<svg::Rgba>(color);
		color_serialized.set_r(rgba_color.red);
		color_serialized.set_g(rgba_color.green);
		color_serialized.set_b(rgba_color.blue);
		color_serialized.set_a(rgba_color.opacity);
	}
	return color_serialized;
}

void SerializeRenderSettings(tc_serialization::TransportCatalogue& catalogue_serialized, const Transport::Rendering::RenderSettings& settings) {
	tc_serialization::RenderSettings settings_serialized;
	settings_serialized.set_width(settings.width);
	settings_serialized.set_height(settings.height);
	settings_serialized.set_padding(settings.padding);
	settings_serialized.set_line_width(settings.line_width);
	settings_serialized.set_stop_radius(settings.stop_radius);
	settings_serialized.set_bus_label_font_size(settings.bus_label_font_size);
	settings_serialized.set_bus_label_x_offset(settings.bus_label_offset.x);
	settings_serialized.set_bus_label_y_offset(settings.bus_label_offset.y);
	settings_serialized.set_stop_label_font_size(settings.stop_label_font_size);
	settings_serialized.set_stop_label_x_offset(settings.stop_label_offset.x);
	settings_serialized.set_stop_label_y_offset(settings.stop_label_offset.y);
	*settings_serialized.mutable_underlayer_color() = SerializeColor(settings.underlayer_color);
	for (auto color : settings.color_palette) {
		settings_serialized.mutable_color_palette()->Add(std::move(SerializeColor(color)));
	}
	settings_serialized.set_underlayer_width(settings.underlayer_width);
	*catalogue_serialized.mutable_render_settings() = std::move(settings_serialized);
}

void SerializeLightTransportRouter(const Transport::Routing::TransportRouter& transport_router, tc_serialization::TransportRouter& s_transport_router) {
	// serialize edges_info
	for (auto& edge_info : transport_router.GetEdgesInfo()) {
		tc_serialization::EdgeInfo s_edge_info;
		s_edge_info.set_name(edge_info.name);
		s_edge_info.set_span_count(edge_info.span_count);
		s_edge_info.set_weight(edge_info.weight);
		s_transport_router.mutable_edge_info()->Add(std::move(s_edge_info));
	}

	// serialize route_internal_data
	s_transport_router.set_vertex_count(transport_router.GetGraph().GetVertexCount());

	const auto& routes_data = transport_router.GetRouter().GetRoutesInternalData();
	for (size_t i = 0; i < routes_data.size(); i++)
	{
		for (size_t j = 0; j < routes_data[0].size(); j++)
		{
			tc_serialization::RouteInternalData s_route_data;
			if (routes_data[i][j].has_value())
			{
				s_route_data.set_exist(true);
				s_route_data.set_weight(routes_data[i][j].value().weight);
				if (routes_data[i][j].value().prev_edge)
				{
					s_route_data.set_has_prev_edge(true);
					s_route_data.set_prev_edge(routes_data[i][j].value().prev_edge.value());
				}
				else
				{
					s_route_data.set_has_prev_edge(false);
				}
			}
			else
			{
				s_route_data.set_exist(false);
			}
			s_transport_router.mutable_route_internal_data()->Add(std::move(s_route_data));
		}
	}

	// serialize graph
	const auto graph = transport_router.GetGraph();
	for (auto edge : graph.GetEdges()) {
		tc_serialization::Edge s_edge;
		s_edge.set_from(edge.from);
		s_edge.set_to(edge.to);
		s_edge.set_weight(edge.weight);
		s_transport_router.mutable_edge()->Add(std::move(s_edge));
	}
}

void serialization::SerializeTransportCatalogue(const Transport::TransportCatalogue& catalogue, std::string filename, const Transport::Rendering::RenderSettings& render_settings,
	const Transport::Routing::RouterSettings& router_settings)
{
	std::ofstream fout(std::string(filename), std::ios::binary);

	std::unordered_map<const Transport::Stop*, size_t> stop_to_id;
	size_t stop_id_count = 0;

	std::unordered_map<const Transport::Bus*, size_t> bus_to_id;
	size_t bus_id_count = 0;

	tc_serialization::TransportCatalogue catalogue_serialized;

	// serialize stops
	tc_serialization::StopList stop_list;
	for (const auto stop_ptr : catalogue.GetStops()) {
		stop_to_id[stop_ptr] = stop_id_count++;

		tc_serialization::Stop stop_serialized;
		stop_serialized.set_name(stop_ptr->name);
		stop_serialized.set_stop_id(stop_to_id[stop_ptr]);
		stop_serialized.set_lat_coord(stop_ptr->coords.lat);
		stop_serialized.set_lng_coord(stop_ptr->coords.lng);

		stop_list.mutable_stop()->Add(std::move(stop_serialized));
	}

	// serialize buses
	tc_serialization::BusList bus_list;
	for (const auto& bus : catalogue.GetBuses()) {
		bus_to_id[&bus] = bus_id_count++;

		tc_serialization::Bus bus_serialized;
		bus_serialized.set_name(bus.name);
		bus_serialized.set_bus_id(bus_to_id[&bus]);
		bus_serialized.set_is_roundtrip(bus.is_roundtrip);
		for (const auto stop_ptr : bus.stops) {
			bus_serialized.mutable_stop_id()->Add(stop_to_id[stop_ptr]);
		}

		bus_list.mutable_bus()->Add(std::move(bus_serialized));
	}

	// serialize DistanceMap
	tc_serialization::DistanceMap distance_map;
	for (const auto [stops, dist] : catalogue.GetDistanceMap()) {
		tc_serialization::Distance distance_serialized;
		distance_serialized.set_from(stop_to_id[stops.first]);
		distance_serialized.set_to(stop_to_id[stops.second]);
		distance_serialized.set_distance(dist);
		distance_map.mutable_distance()->Add(std::move(distance_serialized));
	}

	// serialize StopRoutesMap
	tc_serialization::StopRoutesMap stop_routes_map;
	for (const auto [stop, buses] : catalogue.GetStopsToBuses()) {
		tc_serialization::StopRoutes stop_routes;
		stop_routes.set_stop_id(stop_to_id[stop]);
		for (const auto bus : buses) {
			stop_routes.mutable_bus_id()->Add(bus_to_id[bus]);
		}
		stop_routes_map.mutable_stop()->Add(std::move(stop_routes));
	}

	*catalogue_serialized.mutable_stop_list() = std::move(stop_list);
	*catalogue_serialized.mutable_bus_list() = std::move(bus_list);
	*catalogue_serialized.mutable_distance_map() = std::move(distance_map);
	*catalogue_serialized.mutable_stop_routes_map() = std::move(stop_routes_map);

	//SerializeRenderSettings(catalogue_serialized, render_settings);
	SerializeRenderSettings(catalogue_serialized, render_settings);

	Transport::Routing::TransportRouter router(catalogue, router_settings);
	SerializeLightTransportRouter(router, *catalogue_serialized.mutable_transport_router());

	catalogue_serialized.SerializeToOstream(&fout);
}

svg::Color DeserializeColor(const tc_serialization::Color& color_serialized) {
	std::string type = color_serialized.type();
	if (type == "monostate")
	{
		return svg::Color(std::monostate{});
	}
	else if (type == "string")
	{
		return svg::Color(color_serialized.string_color());
	}
	else if (type == "rgb")
	{
		return svg::Color(svg::Rgb(color_serialized.r(), color_serialized.g(), color_serialized.b()));
	}
	else if (type == "rgba")
	{
		return svg::Color(svg::Rgba(color_serialized.r(), color_serialized.g(), color_serialized.b(), color_serialized.a()));
	}
	else
	{
		assert(false);
		return {};
	}
}

void DeserializeRenderSettings(const tc_serialization::TransportCatalogue& catalogue_serialized, Transport::Rendering::RenderSettings& settings) {
	auto& settings_serialized = catalogue_serialized.render_settings();
	settings.width = settings_serialized.width();
	settings.height = settings_serialized.height();
	settings.padding = settings_serialized.padding();
	settings.line_width = settings_serialized.line_width();
	settings.stop_radius = settings_serialized.stop_radius();
	settings.bus_label_font_size = settings_serialized.bus_label_font_size();
	settings.bus_label_offset.x = settings_serialized.bus_label_x_offset();
	settings.bus_label_offset.y = settings_serialized.bus_label_y_offset();
	settings.stop_label_font_size = settings_serialized.stop_label_font_size();
	settings.stop_label_offset.x = settings_serialized.stop_label_x_offset();
	settings.stop_label_offset.y = settings_serialized.stop_label_y_offset();
	settings.underlayer_color = DeserializeColor(settings_serialized.underlayer_color());
	settings.underlayer_width = settings_serialized.underlayer_width();
	for (auto i = 0; i < settings_serialized.color_palette_size(); i++)
	{
		settings.color_palette.push_back(DeserializeColor(settings_serialized.color_palette(i)));
	}
}

Transport::Routing::LightTransportRouter DeserializeTransportRouter(const tc_serialization::TransportRouter& s_transport_router, const Transport::TransportCatalogue& catalogue) {
	// восстановить Transport_router

	// deserialize edges_info
	std::vector<Transport::Routing::EdgeInfo> edges_info;
	const auto& s_edges_info = s_transport_router.edge_info();
	for (size_t i = 0; i < s_transport_router.edge_info_size(); i++)
	{
		Transport::Routing::EdgeInfo ei;
		ei.span_count = s_transport_router.edge_info(i).span_count();
		ei.name = s_transport_router.edge_info(i).name();
		ei.weight = s_transport_router.edge_info(i).weight();
		edges_info.push_back(std::move(ei));
	}

	// deserialize route_internal_data
	size_t vertex_count = s_transport_router.vertex_count();

	graph::Router<double>::RoutesInternalData routes_data(vertex_count,
		std::vector<std::optional<graph::Router<double>::RouteInternalData>>(vertex_count));
	for (size_t i = 0; i < routes_data.size(); i++)
	{
		for (size_t j = 0; j < routes_data[0].size(); j++)
		{
			const auto& route = s_transport_router.route_internal_data(i * vertex_count + j);
			if (route.exist())
			{
				if (route.has_prev_edge())
				{
					routes_data[i][j] = { route.weight(), route.prev_edge() };
				}
				else
				{
					routes_data[i][j] = { route.weight(), std::nullopt };
				}
			}
		}
	}

	// deserialize graph
	std::vector<graph::Edge<double>> edges;
	for (size_t i = 0; i < s_transport_router.edge_size(); i++)
	{
		const auto& s_edge = s_transport_router.edge(i);
		graph::Edge<double> edge;
		edge.from = s_edge.from();
		edge.to = s_edge.to();
		edge.weight = s_edge.weight();
		edges.push_back(std::move(edge));
	}

	return Transport::Routing::LightTransportRouter(catalogue, edges_info, edges, routes_data);
}

struct SerializetionIdMap {
	std::unordered_map<size_t, const Transport::Stop*> id_to_stop;
	std::unordered_map<size_t, const Transport::Bus*> id_to_bus;
	size_t stop_id_count = 0;
	size_t bus_id_count = 0;
};

void DeserializeStops(const tc_serialization::StopList& s_stop_list, Transport::TransportCatalogue& catalogue, SerializetionIdMap& id_map) {
	std::deque<Transport::Stop> stops;

	for (auto i = 0; i < s_stop_list.stop_size(); i++)
	{
		const auto& s_stop = s_stop_list.stop(i);

		Transport::Stop stop;
		stop.name = s_stop.name();
		stop.coords.lat = s_stop.lat_coord();
		stop.coords.lng = s_stop.lng_coord();

		stops.push_back(std::move(stop));
	}

	catalogue.SetStops(std::move(stops));

	for (auto stop_ptr : catalogue.GetStops()) {
		id_map.id_to_stop[id_map.stop_id_count++] = stop_ptr;
	}
}

void DeserializeBuses(const tc_serialization::BusList& s_bus_list, Transport::TransportCatalogue& catalogue, SerializetionIdMap& id_map) {
	std::deque<Transport::Bus> buses;

	for (auto i = 0; i < s_bus_list.bus_size(); i++)
	{
		const auto& s_bus = s_bus_list.bus(i);

		Transport::Bus bus;
		bus.name = s_bus.name();
		bus.is_roundtrip = s_bus.is_roundtrip();
		for (auto j = 0; j < s_bus.stop_id_size(); j++)
		{
			auto stop_id = s_bus.stop_id(j);
			bus.stops.push_back(id_map.id_to_stop[stop_id]);
		}

		buses.push_back(std::move(bus));
	}

	catalogue.SetBuses(std::move(buses));

	for (const auto& bus : catalogue.GetBuses()) {
		id_map.id_to_bus[id_map.bus_id_count++] = &bus;
	}
}

void DeserializeDistanceMap(const tc_serialization::DistanceMap& s_distance_map, Transport::TransportCatalogue& catalogue, SerializetionIdMap& id_map) {
	Transport::TransportCatalogue::DistanceMap distance_map;

	for (auto i = 0; i < s_distance_map.distance_size(); i++)
	{
		const auto s_distance = s_distance_map.distance(i);
		auto from = id_map.id_to_stop[s_distance.from()];
		auto to = id_map.id_to_stop[s_distance.to()];
		distance_map[{from, to}] = s_distance.distance();
	}

	catalogue.SetDistanceMap(distance_map);
}

void DeserializeStopRoutes(const tc_serialization::StopRoutesMap& s_stop_routes_map, Transport::TransportCatalogue& catalogue, SerializetionIdMap& id_map) {
	Transport::TransportCatalogue::StopToRoutesMap stop_routes_map;

	for (auto i = 0; i < s_stop_routes_map.stop_size(); i++)
	{
		const auto s_stop = s_stop_routes_map.stop(i);
		auto stop_ptr = id_map.id_to_stop[s_stop.stop_id()];
		for (auto j = 0; j < s_stop.bus_id_size(); j++)
		{
			auto bus_ptr = id_map.id_to_bus[s_stop.bus_id(j)];
			stop_routes_map[stop_ptr].insert(bus_ptr);
		}
	}

	catalogue.SetStopToBuses(std::move(stop_routes_map));
}


void DeserializeCatalogueInner(const tc_serialization::TransportCatalogue& s_catalogue, Transport::TransportCatalogue& catalogue) {
	SerializetionIdMap id_map;

	DeserializeStops(s_catalogue.stop_list(), catalogue, id_map);
	DeserializeBuses(s_catalogue.bus_list(), catalogue, id_map);
	DeserializeDistanceMap(s_catalogue.distance_map(), catalogue, id_map);
	DeserializeStopRoutes(s_catalogue.stop_routes_map(), catalogue, id_map);
}

Transport::Routing::LightTransportRouter serialization::DeserializeTransportCatalogue(std::string filename, Transport::TransportCatalogue& catalogue, Rendering::RenderSettings& render_settings)
{
	std::ifstream fin(filename, std::ios::binary);

	tc_serialization::TransportCatalogue s_catalogue;
	s_catalogue.ParseFromIstream(&fin);

	DeserializeCatalogueInner(s_catalogue, catalogue);

	DeserializeRenderSettings(s_catalogue, render_settings);

	return DeserializeTransportRouter(s_catalogue.transport_router(), catalogue);
}
