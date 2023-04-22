#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <cassert>

using namespace Transport;

void TransportCatalogue::AddStop(std::string_view request_text)
{
	using namespace std;

	// повторно вычисляется в SetStopDistances - подумать над оптимизацией
	auto parsed_request = ParseStopRequest(request_text);

	stops_.push_back(parsed_request.first);
	stop_name_to_stop_[string_view(stops_.back().name)] = &(stops_.back());
}

void TransportCatalogue::SetStopDistances(std::string_view request_text)
{
	// уже вычислялось в AddStop - подумать над оптимизацией
	auto parsed_request = ParseStopRequest(request_text);

	if (!parsed_request.second.empty())
	{
		std::string_view cur_name_sv = stop_name_to_stop_.at(parsed_request.first.name)->name;
		WriteStopDistances(cur_name_sv, parsed_request.second);
	}
}

void TransportCatalogue::AddBus(std::string_view request_text)
{
	using namespace std;

	buses_.push_back(ParseBusRequest(request_text));
	bus_name_to_bus_[string_view(buses_.back().name)] = &(buses_.back());
	for (const auto stop_ptr : buses_.back().stops) {
		stop_name_to_buses_[stop_ptr->name].insert(&(buses_.back()));
	}
}

const TransportCatalogue::Stop* TransportCatalogue::GetStop(std::string_view stop_name) const
{
	if (stop_name_to_stop_.count(stop_name))
	{
		return stop_name_to_stop_.at(stop_name);
	}
	return nullptr;
}

int TransportCatalogue::GetRealDistance(std::string_view lhv, std::string_view rhv) const
{
	int distance = 0;

	// если поступает string_view от остановок в stops_, здесь string_view вычисляется лишний раз
	std::string_view left_sv = stop_name_to_stop_.at(lhv)->name;
	std::string_view right_sv = stop_name_to_stop_.at(rhv)->name;

	if (between_stops_distances_.count({left_sv, right_sv}))
	{
		distance += between_stops_distances_.at({ left_sv, right_sv });
	}
	else if (between_stops_distances_.count({ right_sv, left_sv }))
	{
		distance += between_stops_distances_.at({ right_sv, left_sv });
	}
	else
	{
		// not found
		assert(false);
	}
	return distance;
}

const TransportCatalogue::Bus* TransportCatalogue::GetBus(std::string_view bus_name) const
{
	if (bus_name_to_bus_.count(bus_name))
	{
		return bus_name_to_bus_.at(bus_name);
	}
	return nullptr;
}

TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const
{
	BusInfo info;
	info.name = bus_name;
	const Bus* bus_ptr = GetBus(bus_name);
	if (!bus_ptr)
	{
		return info;
	}

	info.exists = true;
	info.stops_count = bus_ptr->stops.size();
	info.unique_stops = CountUniqueStops(bus_ptr);
	info.geo_length = ComputeBusGeoDistance(bus_ptr);
	info.real_length = ComputeBusRealDistance(bus_ptr);

	return info;
}

TransportCatalogue::StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const
{
	StopInfo info;
	info.name = stop_name;

	// Если остановки с таким именем не существует
	if (!stop_name_to_stop_.count(stop_name))
	{
		return info;
	}

	// Остановка существует, но через неё не проходит ни одного маршрута
	info.exists = true;

	if (!stop_name_to_buses_.count(stop_name))
	{
		return info;
	}

	// Остановка существует и через неё проходят маршруты
	info.buses = &stop_name_to_buses_.at(stop_name);

	return info;
}

std::pair<TransportCatalogue::Stop, std::string_view> TransportCatalogue::ParseStopRequest(std::string_view request)
{
	Stop stop;
	size_t colon_index = request.find(':');
	stop.name = request.substr(0, colon_index);
	request = request.substr(colon_index + 2);

	std::string left, right;
	size_t comma_index = request.find(',');
	left = request.substr(0, comma_index);
	request = request.substr(comma_index + 2);

	comma_index = request.find(',');
	if (comma_index == request.npos)
	{
		right = request;
		stop.coords = { std::stod(left), std::stod(right) };

		return { stop, ""};
	}
	else
	{
		right = request.substr(0, comma_index);
		stop.coords = { std::stod(left), std::stod(right) };
		request = request.substr(comma_index + 2);

		return { stop, request };
	}
}

void TransportCatalogue::WriteStopDistances(std::string_view name, std::string_view text)
{
	int dist;
	std::string target_name;
	size_t separator_index = 0;
	while (!text.empty())
	{
		separator_index = text.find(' ');
		dist = std::stoi(static_cast<std::string>(text.substr(0, separator_index - 1)));
		text.remove_prefix(separator_index + 4);
		separator_index = text.find(',');
		target_name = text.substr(0, separator_index);
		if (separator_index == text.npos)
		{
			text.remove_prefix(text.size());
		}
		else
		{
			text.remove_prefix(separator_index + 2);
		}

		std::string_view tg_view = stop_name_to_stop_.at(target_name)->name;
		between_stops_distances_[{name, tg_view}] = dist;
	}
}

TransportCatalogue::Bus TransportCatalogue::ParseBusRequest(std::string_view request)
{
	Bus bus;
	char separator = '-';
	bool is_circular = false;

	size_t colon_index = request.find(':');
	bus.name = request.substr(0, colon_index);
	request = request.substr(colon_index + 2);

	if (request.find("-") == request.npos)
	{
		separator = '>';
		is_circular = true;
	}
	std::string cur_stop_name;
	while (!request.empty())
	{
		cur_stop_name = request.substr(0, request.find(separator) - 1);
		bus.stops.push_back(GetStop(cur_stop_name));
		request = request.substr(std::min(cur_stop_name.size() + 3, request.size()));
	}

	if (!is_circular)
	{
		std::vector<const Stop*> additional(bus.stops.begin(), bus.stops.end() - 1);
		std::reverse(additional.begin(), additional.end());
		for (auto& stop : additional) {
			bus.stops.push_back(std::move(stop));
		}
	}

	return bus;
}

size_t TransportCatalogue::CountUniqueStops(const Bus* bus) const
{
	if (!bus)
	{
		return 0;
	}
	std::unordered_set<const Stop*> unique(bus->stops.begin(), bus->stops.end());
	return unique.size();
}

double TransportCatalogue::ComputeBusGeoDistance(const Bus* bus) const
{
	if (!bus)
		return 0.;

	double distance = 0.;
	for (size_t i = 0; i + 1 < bus->stops.size(); i++)
	{
		distance += ComputeDistance(bus->stops[i]->coords, bus->stops[i + 1]->coords);
	}
	return distance;
}

int TransportCatalogue::ComputeBusRealDistance(const Bus* bus) const
{
	if (!bus)
		return 0;

	int distance = 0;
	for (size_t i = 0; i + 1 < bus->stops.size(); i++)
	{
		distance += GetRealDistance(bus->stops[i]->name, bus->stops[i + 1]->name);
	}
	return distance;
}

std::ostream& Transport::operator<<(std::ostream& out, const TransportCatalogue::Stop& stop)
{
	using namespace std::literals;
	out << "Stop "s << stop.name << ": "s << stop.coords.lat << ", "s << stop.coords.lng;

	return out;
}

std::ostream& Transport::operator<<(std::ostream& out, const TransportCatalogue::Bus& bus)
{
	using namespace std::literals;
	out << "Bus "s << bus.name;
	for (const TransportCatalogue::Stop* stop_ptr : bus.stops) {
		out << " "s << stop_ptr->name;
	}

	return out;
}

std::ostream& Transport::operator<<(std::ostream& out, const TransportCatalogue::BusInfo& info)
{
	using namespace std::literals;

	out << "Bus "s << info.name << ": "s;
	if (info.exists)
	{
		double curvature = static_cast<double>(info.real_length) / info.geo_length;
		out << info.stops_count << " stops on route, "s << info.unique_stops << " unique stops, "s
			<< std::setprecision(6) << info.real_length << " route length, "s << curvature << " curvature"s;
	}
	else
	{
		out << "not found"s;
	}

	return out;
}

std::ostream& Transport::operator<<(std::ostream& out, const TransportCatalogue::StopInfo& info)
{
	using namespace std::literals;

	out << "Stop "s << info.name << ": "s;

	if (!info.exists)
	{
		out << "not found"s;
	}
	else if (!info.buses)
	{
		out << "no buses"s;
	}
	else
	{
		out << "buses"s;
		for (const auto bus_ptr : *(info.buses)) {
			out << ' ' << bus_ptr->name;
		}
	}

	return out;
}

bool Transport::TransportCatalogue::Stop::operator==(const Stop& other) const
{
	return std::make_pair(name, coords) == std::make_pair(other.name, other.coords);
}
