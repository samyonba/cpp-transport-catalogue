#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <cassert>

using namespace Transport;
using namespace std;

void Transport::TransportCatalogue::AddStop(std::string_view name, Geo::Coordinates coords)
{
	stops_.push_back({ static_cast<string>(name), coords });
	stop_name_to_stop_[stops_.back().name] = &(stops_.back());
}

void Transport::TransportCatalogue::SetDistance(const Stop* from, const Stop* to, int dist)
{
	if (!from || !to)
	{
		return;
	}
	between_stops_distances_[{from, to}] = dist;
}

void Transport::TransportCatalogue::SetStopDistances(const Stop* from, std::vector<std::pair<const Stop*, int>> to_distance)
{
	for (const auto& [to, dist] : to_distance) {
		SetDistance(from, to, dist);
	}
}

void Transport::TransportCatalogue::AddBus(std::string_view name, const std::vector<const Stop*>& stops)
{
	buses_.push_back({ static_cast<string>(name), stops });
	bus_name_to_bus_[buses_.back().name] = &(buses_.back());
	for (const auto stop_ptr : buses_.back().stops) {
		stop_to_buses_[stop_ptr].insert(&(buses_.back()));
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

int TransportCatalogue::GetRealDistance(const Stop* from, const Stop* to) const
{
	int distance = 0;

	if (between_stops_distances_.count({ from, to }))
	{
		distance += between_stops_distances_.at({ from, to });
	}
	else if (between_stops_distances_.count({ to, from }))
	{
		distance += between_stops_distances_.at({ to, from });
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

TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(const Bus* bus_p) const
{
	BusInfo info;
	if (!bus_p)
	{
		return info;
	}
	info.name = bus_p->name;

	info.exists = true;
	info.stops_count = bus_p->stops.size();
	info.unique_stops = CountUniqueStops(bus_p);
	info.geo_length = ComputeBusGeoDistance(bus_p);
	info.real_length = ComputeBusRealDistance(bus_p);

	return info;
}

TransportCatalogue::StopInfo TransportCatalogue::GetStopInfo(const Stop* stop_p) const
{
	StopInfo info;
	if (!stop_p)
	{
		return info;
	}
	info.name = stop_p->name;

	// Остановка существует, но через неё не проходит ни одного маршрута
	info.exists = true;

	if (!stop_to_buses_.count(stop_p))
	{
		return info;
	}

	// Остановка существует и через неё проходят маршруты
	info.buses = &stop_to_buses_.at(stop_p);

	return info;
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
		distance += GetRealDistance(bus->stops[i], bus->stops[i + 1]);
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
