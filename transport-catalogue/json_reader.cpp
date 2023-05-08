#include "json_reader.h"
#include "request_handler.h"

#include <sstream>

using namespace std;
using namespace Transport;

void Transport::JsonReader::ReadInput()
{
    using namespace json;

    // словарь из { base_requests:... , stat_requests:... }
    Document document = Load(in_);

    // вектор запросов (маршруты/остановки)
    const auto& base_requests = document.GetRoot().AsMap().at("base_requests").AsArray();
    ReadBaseRequests(base_requests);

    const auto& render_settings = document.GetRoot().AsMap().at("render_settings").AsMap();
    ReadRenderSettings(render_settings);

    out_ << "[\n";
    const auto& stat_requests = document.GetRoot().AsMap().at("stat_requests").AsArray();
    ReadStatRequests(stat_requests);
    out_ << "]\n";
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
    settings_.width = attributes.at("width").AsDouble();
    settings_.height = attributes.at("height").AsDouble();

    settings_.padding = attributes.at("padding").AsDouble();

    settings_.line_width = attributes.at("line_width").AsDouble();
    settings_.stop_radius = attributes.at("stop_radius").AsDouble();

    settings_.bus_label_font_size = attributes.at("bus_label_font_size").AsInt();
    const auto& bus_offset_array = attributes.at("bus_label_offset").AsArray();
    settings_.bus_label_offset = { bus_offset_array[0].AsDouble(), bus_offset_array[1].AsDouble()};

    settings_.stop_label_font_size = attributes.at("stop_label_font_size").AsInt();
    const auto& stop_offset_array = attributes.at("stop_label_offset").AsArray();
    settings_.stop_label_offset = { stop_offset_array[0].AsDouble(), stop_offset_array[1].AsDouble() };

    settings_.underlayer_color = ReadColor(attributes.at("underlayer_color"));
    settings_.underlayer_width = attributes.at("underlayer_width").AsDouble();

    const auto& palette_array = attributes.at("color_palette").AsArray();
    for (const auto& color : palette_array) {
        settings_.color_palette.push_back(ReadColor(color));
    }
}

void Transport::JsonReader::ReadStop(const json::Dict& attributes) {
	Geo::Coordinates coords{ attributes.at("latitude").AsDouble(), attributes.at("longitude").AsDouble() };
	catalogue_.AddStop(attributes.at("name").AsString(), coords);
}

void Transport::JsonReader::ReadDistances(const json::Dict& attributes) {
    const Stop* from = catalogue_.GetStop(attributes.at("name").AsString());
    for (const auto& [to_name, dist_node] : attributes.at("road_distances").AsMap()) {
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
        auto& attributes = request.AsMap();
        if (attributes.at("type").AsString() == "Stop")
        {
            ReadStop(attributes);
        }
    }

    // при втором проходе добавляем данные о расстояниях между остановками и о маршрутах
    for (const auto& request : base_requests) {
        auto& attributes = request.AsMap();

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

void Transport::JsonReader::PrintJsonStopInfo(const Transport::StopInfo& info, int request_id) {
    using namespace json;

    Dict output;
	output["request_id"] = Node(request_id);

    if (!info.exists)
    {
        output["error_message"] = Node("not found"s);
        Print(Document(output), out_);
        out_ << '\n';
        return;
    }

    Array buses;
    if (info.buses)
    {
        for (const auto& bus : *info.buses) {
            buses.push_back(Node(bus->name));
        }
    }
    output["buses"] = buses;

    Print(Document(output), out_);
    out_ << '\n';
}

void Transport::JsonReader::PrintJsonBusInfo(const Transport::BusInfo& info, int request_id) {
    using namespace json;

    Dict output;
    output["request_id"] = Node(request_id);

    if (!info.exists)
    {
        output["error_message"] = Node("not found"s);
        Print(Document(output), out_);
        out_ << '\n';
        return;
    }

    output["curvature"] = Node(double(info.real_length) / info.geo_length);
    output["route_length"] = Node(info.real_length);
    output["stop_count"] = Node(int(info.stops_count));
    output["unique_stop_count"] = Node(int(info.unique_stops));

    Print(Document(output), out_);
    out_ << '\n';
}

void Transport::JsonReader::PrintJsonMap(int request_id)
{
    using namespace json;

    Dict map_json;
    map_json["request_id"] = Node(request_id);

    ostringstream map_ostream;
    RenderCatalogue(catalogue_, settings_, map_ostream);
    
    map_json["map"] = Node(map_ostream.str());

    Print(Document(map_json), out_);
    out_ << '\n';
}

void Transport::JsonReader::ReadStatRequests(const json::Array& stat_requests) {
    for (size_t i = 0; i < stat_requests.size(); i++)
    {
        if (i != 0)
        {
            out_ << ",\n";
        }
        auto& attributes = stat_requests[i].AsMap();

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
    }
}
