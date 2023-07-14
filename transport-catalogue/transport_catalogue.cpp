#include "transport_catalogue.h"

#include <algorithm>
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

void Transport::TransportCatalogue::AddBus(std::string_view name, const std::vector<const Stop*>& stops, bool is_roundtrip)
{
	buses_.push_back({ static_cast<string>(name), stops, is_roundtrip });
	bus_name_to_bus_[buses_.back().name] = &(buses_.back());
	for (const auto stop_ptr : buses_.back().stops) {
		stop_to_buses_[stop_ptr].insert(&(buses_.back()));
	}
}

const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const
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

const Bus* TransportCatalogue::GetBus(std::string_view bus_name) const
{
	if (bus_name_to_bus_.count(bus_name))
	{
		return bus_name_to_bus_.at(bus_name);
	}
	return nullptr;
}

BusInfo TransportCatalogue::GetBusInfo(const Bus* bus_p) const
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

const std::deque<Bus>& Transport::TransportCatalogue::GetBuses() const
{
	return buses_;
}

const std::vector<const Stop*> Transport::TransportCatalogue::GetStops() const
{
	vector<const Stop*> result;
	result.reserve(stops_.size());
	for (const auto& stop : stops_) {
		result.push_back(&stop);
	}
	return result;
}

size_t Transport::TransportCatalogue::GetStopsCount() const
{
	return stops_.size();
}

const std::set<const Bus*, BusComparator> Transport::TransportCatalogue::GetBusesForStop(const Stop* stop) const
{
	if (stop_to_buses_.count(stop))
	{
		return stop_to_buses_.at(stop);
	}
	return {};
}

StopInfo TransportCatalogue::GetStopInfo(const Stop* stop_p) const
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
