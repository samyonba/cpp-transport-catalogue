#include "domain.h"
#include <iomanip>

using namespace Transport;
using namespace std;

ostream& Transport::operator<<(ostream& out, const Stop& stop)
{
	using namespace std::literals;
	out << "Stop "s << stop.name << ": "s << stop.coords.lat << ", "s << stop.coords.lng;

	return out;
}

ostream& Transport::operator<<(ostream& out, const Bus& bus)
{
	using namespace std::literals;
	out << "Bus "s << bus.name;
	for (const Stop* stop_ptr : bus.stops) {
		out << " "s << stop_ptr->name;
	}

	return out;
}

ostream& Transport::operator<<(ostream& out, const BusInfo& info)
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

ostream& Transport::operator<<(ostream& out, const StopInfo& info)
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

bool Stop::operator==(const Stop& other) const
{
	return std::make_pair(name, coords) == std::make_pair(other.name, other.coords);
}

bool Transport::BusComparator::operator()(const Bus* lhv, const Bus* rhv) const
{
	return lhv->name < rhv->name;
}
