#include "transport_catalogue.h"
#include "input_reader.h"
#include "test_transport_catalogue.h"
#include "stat_reader.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace Transport;

void ReadInput(TransportCatalogue& catalogue, std::istream& in = cin) {
    vector<Input::Request> requests = Input::GetRequests(in);

    // при первом проходе по запросам добавляем только данные о самих остановках
    for (const auto& request : requests) {
        if (request.type == Input::Request::Type::Stop)
        {
            catalogue.AddStop(request.name, request.coords);
        }
    }

    // при втором проходе добавляем данные о расстояниях между остановками и о маршрутах
    for (const auto& request : requests) {
        if (request.type == Input::Request::Type::Stop)
        {
            const TransportCatalogue::Stop* from = catalogue.GetStop(request.name);
            vector<pair<const TransportCatalogue::Stop*, int>> distances;
            for (const auto& [to_name, dist] : request.distances)
            {
                const TransportCatalogue::Stop* to = catalogue.GetStop(to_name);
                catalogue.SetDistance(from, to, dist);
            }
        }
        if (request.type == Input::Request::Type::Bus)
        {
            vector<const TransportCatalogue::Stop*> stops(request.stops.size());
            for (size_t i = 0; i < request.stops.size(); i++)
            {
                stops[i] = catalogue.GetStop(request.stops[i]);
            }
            catalogue.AddBus(request.name, stops);
        }
    }
}

void ReadStat(TransportCatalogue& catalogue, istream& in = cin, ostream& out = cout) {
    vector<Stat::Request> requests = Stat::GetRequests(in);
    for (const auto& request : requests) {
        if (request.type == Stat::Request::Type::Bus)
        {
            catalogue.PrintBusInfo(catalogue.GetBus(request.name), out);
        }

        if (request.type == Stat::Request::Type::Stop)
        {
            catalogue.PrintStopInfo(catalogue.GetStop(request.name), out);
        }
    }
}

int main()
{
    TransportCatalogue catalogue;

    // по умолчанию - cin/cout
    ReadInput(catalogue);
    ReadStat(catalogue);


    // file input:

    /*ifstream in("input.txt");
    ofstream out("output.txt");
	if (in.is_open() && out.is_open()) {
		ReadInput(catalogue, in);
		ReadStat(catalogue, in, out);
	}
    in.close();
    out.close();*/
}
