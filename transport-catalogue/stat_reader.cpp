#include "stat_reader.h"

using namespace std;
using namespace Transport;

void Stat::ParseRawRequest(std::string raw_request, TransportCatalogue& catalogue, std::ostream& out)
{
	string right;
	if (raw_request.substr(0, 3) == "Bus")
	{
		PrintBusInfo(catalogue.GetBusInfo(catalogue.GetBus(raw_request.substr(4))), out);
	}

	if (raw_request.substr(0, 4) == "Stop")
	{
		PrintStopInfo(catalogue.GetStopInfo(catalogue.GetStop(raw_request.substr(5))), out);
		right = raw_request.substr(5);
	}
}

void Stat::PrintBusInfo(const TransportCatalogue::BusInfo& info, std::ostream& out) {
	out << info << std::endl;
}

void Stat::PrintStopInfo(const TransportCatalogue::StopInfo& info, std::ostream& out) {
	out << info << std::endl;
}

void Transport::Stat::ReadStat(TransportCatalogue& catalogue, std::istream& in, std::ostream& out)
{
	int requests_count = 0;
	in >> requests_count;
	in.get();

	string cur_request;
	for (size_t i = 0; i < requests_count; i++)
	{
		std::getline(in, cur_request);
		ParseRawRequest(std::move(cur_request), catalogue, out);
	}
}
