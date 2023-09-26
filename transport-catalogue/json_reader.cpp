#include "json_reader.h"
#include "request_handler.h"
#include "json_builder.h"

#include <sstream>

using namespace std;
using namespace Transport;

void Transport::JsonReader::ReadInput()
{
    using namespace json;

    // словарь из { base_requests:... , stat_requests:... }
    Document document = Load(in_);

    // вектор запросов (маршруты/остановки)
    const auto& base_requests = document.GetRoot().AsDict().at("base_requests").AsArray();
    ReadBaseRequests(base_requests);

    const auto& render_settings = document.GetRoot().AsDict().at("render_settings").AsDict();
    ReadRenderSettings(render_settings);

    const auto& router_settings = document.GetRoot().AsDict().at("routing_settings").AsDict();
    ReadRouterSettings(router_settings);

    const auto& serialization_settings = document.GetRoot().AsDict().at("serialization_settings").AsDict();
    ReadSerializationSettings(serialization_settings);

    out_ << "[\n";
    const auto& stat_requests = document.GetRoot().AsDict().at("stat_requests").AsArray();
    ReadStatRequests(stat_requests);
    out_ << "]\n";
}

void Transport::JsonReader::ReadMakeBaseInput()
{
    using namespace json;

    // словарь из { base_requests:... , stat_requests:... }
    Document document = Load(in_);

    // вектор запросов (маршруты/остановки)
    const auto& base_requests = document.GetRoot().AsDict().at("base_requests").AsArray();
    ReadBaseRequests(base_requests);

    const auto& serialization_settings = document.GetRoot().AsDict().at("serialization_settings").AsDict();
    ReadSerializationSettings(serialization_settings);

    const auto& render_settings = document.GetRoot().AsDict().at("render_settings").AsDict();
    ReadRenderSettings(render_settings);

    const auto& router_settings = document.GetRoot().AsDict().at("routing_settings").AsDict();
    ReadRouterSettings(router_settings);
}

void Transport::JsonReader::ReadProcessRequest()
{
    using namespace json;

    // словарь из { base_requests:... , stat_requests:... }
    Document document = Load(in_);

    const auto& serialization_settings = document.GetRoot().AsDict().at("serialization_settings").AsDict();
    ReadSerializationSettings(serialization_settings);

    saved_stat_requests_ = document.GetRoot().AsDict().at("stat_requests").AsArray();
}

void Transport::JsonReader::ProcessStatRequests(Routing::TransportRouter& router)
{
    out_ << "[\n";
    ReadStatRequests(saved_stat_requests_);
    out_ << "]\n";
}

void Transport::JsonReader::ProcessStatRequests(const Routing::LightTransportRouter& router)
{
    out_ << "[\n";
    ReadStatRequests(saved_stat_requests_, router);
    out_ << "]\n";
}

std::string Transport::JsonReader::GetSerializationFileName() const
{
    return serialization_settings_;
}

const Rendering::RenderSettings& Transport::JsonReader::GetRenderSettings() const
{
    return render_settings_;
}

const Transport::Routing::RouterSettings& Transport::JsonReader::GetRouterSettings() const
{
    return routing_settings_;
}

void Transport::JsonReader::SetRenderSettings(Transport::Rendering::RenderSettings settings)
{
    render_settings_ = std::move(settings);
}

svg::Color ReadColor(json::Node node) {
    if (node.IsString())
    {
        return svg::Color(node.AsString());
    }
    else {
        const auto& color_array = node.AsArray();
        if (color_array.size() == 3)
        {
            return svg::Color(svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt()));
        }
        else
        {
            return svg::Color(svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble()));
        }
    }
}

void Transport::JsonReader::ReadRenderSettings(const json::Dict& attributes)
{
    render_settings_.width = attributes.at("width").AsDouble();
    render_settings_.height = attributes.at("height").AsDouble();

    render_settings_.padding = attributes.at("padding").AsDouble();

    render_settings_.line_width = attributes.at("line_width").AsDouble();
    render_settings_.stop_radius = attributes.at("stop_radius").AsDouble();

    render_settings_.bus_label_font_size = attributes.at("bus_label_font_size").AsInt();
    const auto& bus_offset_array = attributes.at("bus_label_offset").AsArray();
    render_settings_.bus_label_offset = { bus_offset_array[0].AsDouble(), bus_offset_array[1].AsDouble()};

    render_settings_.stop_label_font_size = attributes.at("stop_label_font_size").AsInt();
    const auto& stop_offset_array = attributes.at("stop_label_offset").AsArray();
    render_settings_.stop_label_offset = { stop_offset_array[0].AsDouble(), stop_offset_array[1].AsDouble() };

    render_settings_.underlayer_color = ReadColor(attributes.at("underlayer_color"));
    render_settings_.underlayer_width = attributes.at("underlayer_width").AsDouble();

    const auto& palette_array = attributes.at("color_palette").AsArray();
    for (const auto& color : palette_array) {
        render_settings_.color_palette.push_back(ReadColor(color));
    }
}

void Transport::JsonReader::ReadStop(const json::Dict& attributes) {
	Geo::Coordinates coords{ attributes.at("latitude").AsDouble(), attributes.at("longitude").AsDouble() };
	catalogue_.AddStop(attributes.at("name").AsString(), coords);
}

void Transport::JsonReader::ReadDistances(const json::Dict& attributes) {
    const Stop* from = catalogue_.GetStop(attributes.at("name").AsString());
    for (const auto& [to_name, dist_node] : attributes.at("road_distances").AsDict()) {
        const Stop* to = catalogue_.GetStop(to_name);
        catalogue_.SetDistance(from, to, dist_node.AsInt());
    }
}

void Transport::JsonReader::ReadBus(const json::Dict& attributes) {
    auto& name = attributes.at("name").AsString();
    bool is_roundtrip = attributes.at("is_roundtrip").AsBool();
    auto& stop_names = attributes.at("stops").AsArray();
    vector<const Stop*> stops;
    for (const auto& stop_name : stop_names) {
        stops.push_back(catalogue_.GetStop(stop_name.AsString()));
    }
    if (!is_roundtrip)
    {
        vector<const Stop*> additional(stops.rbegin() + 1, stops.rend());
        for (const auto stop : additional) {
            stops.push_back(stop);
        }
    }
    catalogue_.AddBus(name, stops, is_roundtrip);
}

void Transport::JsonReader::ReadBaseRequests(const json::Array& base_requests) {

    // при первом проходе по запросам добавляем только данные о самих остановках
    // request - словарь { атрибут - значение }
    for (const auto& request : base_requests) {
        auto& attributes = request.AsDict();
        if (attributes.at("type").AsString() == "Stop")
        {
            ReadStop(attributes);
        }
    }

    // при втором проходе добавляем данные о расстояниях между остановками и о маршрутах
    for (const auto& request : base_requests) {
        auto& attributes = request.AsDict();

        if (attributes.at("type").AsString() == "Stop")
        {
            ReadDistances(attributes);
        }
        if (attributes.at("type").AsString() == "Bus")
        {
            ReadBus(attributes);
        }
    }
}

void Transport::JsonReader::ReadRouterSettings(const json::Dict& attributes)
{
    routing_settings_.bus_wait_time = attributes.at("bus_wait_time").AsInt();
    routing_settings_.bus_velocity = attributes.at("bus_velocity").AsInt();
}

void Transport::JsonReader::ReadSerializationSettings(const json::Dict& attributes)
{
    serialization_settings_ = attributes.at("file").AsString();
}

void Transport::JsonReader::PrintJsonStopInfo(const Transport::StopInfo& info, int request_id) {

	if (!info.exists)
	{
		json::Print(
			json::Document{
				json::Builder{}.StartDict()
			.Key("request_id"s).Value(request_id)
			.Key("error_message"s).Value("not found"s)
			.EndDict().Build()
			},
			out_
		);
		out_ << endl;
        return;
    }
    json::Builder builder;
    auto b = builder.StartDict()
        .Key("request_id"s).Value(request_id)
        .Key("buses"s).StartArray();

    if (info.buses)
    {
        for (const auto& bus : *info.buses) {
            b.Value(bus->name);
        }
    }

    json::Print(json::Document{ b.EndArray().EndDict().Build()},out_ );
    out_ << endl;
    return;
}

void Transport::JsonReader::PrintJsonBusInfo(const Transport::BusInfo& info, int request_id) {

    if (!info.exists)
    {
        json::Print(
            json::Document{
                json::Builder{}.StartDict()
            .Key("request_id"s).Value(request_id)
            .Key("error_message"s).Value("not found"s)
            .EndDict().Build()
            },
            out_
        );
        out_ << endl;
        return;
    }

    json::Print(
        json::Document{
            json::Builder{}.StartDict()
        .Key("request_id"s).Value(request_id)
        .Key("curvature"s).Value(double(info.real_length) / info.geo_length)
        .Key("route_length").Value(info.real_length)
        .Key("stop_count").Value(int(info.stops_count))
        .Key("unique_stop_count").Value(int(info.unique_stops))
        .EndDict().Build()
        },
        out_
    );
    out_ << endl;
}

void Transport::JsonReader::PrintJsonMap(int request_id)
{
    ostringstream map_ostream;
    RenderCatalogue(catalogue_, render_settings_, map_ostream);

    json::Print(
        json::Document{
            json::Builder{}.StartDict()
        .Key("request_id"s).Value(request_id)
        .Key("map"s).Value(map_ostream.str())
        .EndDict().Build()
        },
        out_
    );
    out_ << endl;
}

void Transport::JsonReader::PrintJsonRoute(const string_view from, const string_view to, int request_id, Routing::TransportRouter& router)
{
    auto route_info = router.BuildRoute(from, to);

    json::Builder builder;
    auto b = builder.StartDict()
        .Key("request_id"s).Value(request_id);
    if (!route_info)
    {
        b.Key("error_message"s).Value("not found"s);
    }
    else
    {
        auto items = b.Key("total_time"s).Value(route_info.value().weight)
            .Key("items"s).StartArray();

        auto PrintItem = [&](graph::EdgeId id) {
            json::Builder builder;
            auto c = builder.StartDict();
            auto info = router.GetEdgeInfo(id);
            if (info.span_count == 0)
            {
                c.Key("type"s).Value("Wait"s)
                    .Key("stop_name"s).Value(info.name);
            }
            else
            {
                c.Key("type"s).Value("Bus"s)
                    .Key("bus"s).Value(info.name)
                    .Key("span_count").Value(int(info.span_count));
            }
            c.Key("time"s).Value(info.weight);
            return c.EndDict().Build();
        };
        
        for (const auto& edge_id : route_info.value().edges)
        {
            items.Value(PrintItem(edge_id));
        }
        items.EndArray();
    }
	json::Print(json::Document(b.EndDict().Build()), out_);
}

void Transport::JsonReader::PrintJsonRoute(const std::string_view from, const std::string_view to, int request_id, const Routing::LightTransportRouter& router)
{
    auto route_info = router.BuildRoute(from, to);

    json::Builder builder;
    auto b = builder.StartDict()
        .Key("request_id"s).Value(request_id);
    if (!route_info)
    {
        b.Key("error_message"s).Value("not found"s);
    }
    else
    {
        auto items = b.Key("total_time"s).Value(route_info.value().weight)
            .Key("items"s).StartArray();

        auto PrintItem = [&](graph::EdgeId id) {
            json::Builder builder;
            auto c = builder.StartDict();
            auto info = router.GetEdgeInfo(id);
            if (info.span_count == 0)
            {
                c.Key("type"s).Value("Wait"s)
                    .Key("stop_name"s).Value(info.name);
            }
            else
            {
                c.Key("type"s).Value("Bus"s)
                    .Key("bus"s).Value(info.name)
                    .Key("span_count").Value(int(info.span_count));
            }
            c.Key("time"s).Value(info.weight);
            return c.EndDict().Build();
        };

        for (const auto& edge_id : route_info.value().edges)
        {
            items.Value(PrintItem(edge_id));
        }
        items.EndArray();
    }
    json::Print(json::Document(b.EndDict().Build()), out_);
}

void Transport::JsonReader::ReadStatRequests(const json::Array& stat_requests) {

    // маршрутизатор создается один раз перед обработкой всех запросов
    //Routing::TransportRouter router(catalogue_, routing_settings_);

    for (size_t i = 0; i < stat_requests.size(); i++)
    {
        if (i != 0)
        {
            out_ << ",\n";
        }
        auto& attributes = stat_requests[i].AsDict();

        string type = attributes.at("type").AsString();
        int request_id = attributes.at("id").AsInt();
        if (type == "Stop")
        {
            auto& name = attributes.at("name").AsString();
            PrintJsonStopInfo(catalogue_.GetStopInfo(catalogue_.GetStop(name)), request_id);
        }
        if (type == "Bus")
        {
            auto& name = attributes.at("name").AsString();
            PrintJsonBusInfo(catalogue_.GetBusInfo(catalogue_.GetBus(name)), request_id);
        }
        if (type == "Map")
        {
            PrintJsonMap(request_id);
        }
        /*if (type == "Route")
        {
            auto& from = attributes.at("from").AsString();
            auto& to = attributes.at("to").AsString();
            PrintJsonRoute(from, to, request_id, router);
        }*/
    }
}

void Transport::JsonReader::ReadStatRequests(const json::Array& stat_requests, const Routing::LightTransportRouter& router)
{
    for (size_t i = 0; i < stat_requests.size(); i++)
    {
        if (i != 0)
        {
            out_ << ",\n";
        }
        auto& attributes = stat_requests[i].AsDict();

        string type = attributes.at("type").AsString();
        int request_id = attributes.at("id").AsInt();
        if (type == "Stop")
        {
            auto& name = attributes.at("name").AsString();
            PrintJsonStopInfo(catalogue_.GetStopInfo(catalogue_.GetStop(name)), request_id);
        }
        if (type == "Bus")
        {
            auto& name = attributes.at("name").AsString();
            PrintJsonBusInfo(catalogue_.GetBusInfo(catalogue_.GetBus(name)), request_id);
        }
        if (type == "Map")
        {
            PrintJsonMap(request_id);
        }
        if (type == "Route")
        {
            auto& from = attributes.at("from").AsString();
            auto& to = attributes.at("to").AsString();
            PrintJsonRoute(from, to, request_id, router);
        }
    }
}
