#pragma once

#include "geo.h"

#include <iostream>
#include <string>
#include <vector>
#include <set>

namespace Transport {

	struct Stop
	{
		std::string name;
		Geo::Coordinates coords;

		friend std::ostream& operator<<(std::ostream& out, const Stop& stop);
		bool operator==(const Stop& other) const;
	};

	struct Bus
	{
		std::string name;
		std::vector<const Stop*> stops;
		bool is_roundtrip = true;

		friend std::ostream& operator<<(std::ostream& out, const Bus& bus);
	};

	struct BusComparator;

	struct StopInfo
	{
		std::string name;
		bool exists = false;
		const std::set<const Bus*, BusComparator>* buses = nullptr;

		friend std::ostream& operator<<(std::ostream& out, const StopInfo& info);
	};

	struct BusInfo
	{
		std::string name;
		bool exists = false;
		size_t stops_count = 0;
		size_t unique_stops = 0;
		double geo_length = 0.;
		int real_length = 0;

		friend std::ostream& operator<<(std::ostream& out, const BusInfo& info);
	};

	std::ostream& operator<<(std::ostream& out, const Stop& stop);
	std::ostream& operator<<(std::ostream& out, const Bus& bus);
	std::ostream& operator<<(std::ostream& out, const StopInfo& info);
	std::ostream& operator<<(std::ostream& out, const BusInfo& info);

	// компаратор для определения порядка сортировки множеств внутри stop_name_to_buses_
	struct BusComparator
	{
		bool operator()(const Bus* lhv, const Bus* rhv) const;
	};
}
