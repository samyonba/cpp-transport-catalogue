#include "transport_catalogue.h"
#include "input_reader.h"
#include "test_transport_catalogue.h"
#include "stat_reader.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace Transport;

////////////////////////////////////////////////////////////////// Cin
int main()
{
    TransportCatalogue catalogue;

    // Начало чтения запросов на заполнение справочника
    {
        vector<Input::Request> requests = Input::GetRequests();

        // при первом проходе по запросам добавляем только данные о самих остановках
        for (const auto& request : requests) {
            if (request.type == Input::Request::Type::Stop)
            {
                catalogue.AddStop(request.text);
            }
        }

        // при втором проходе добавляем данные о расстояниях между остановками и о маршрутах
        for (const auto& request : requests) {
            if (request.type == Input::Request::Type::Stop)
            {
                catalogue.SetStopDistances(request.text);
            }
            if (request.type == Input::Request::Type::Bus)
            {
                catalogue.AddBus(request.text);
            }
        }
    }
    // Конец чтения запросов на заполнение справочника


    // Начало чтения запросов к справочнику
    {
        vector<Stat::Request> requests = Stat::GetRequests();
        for (const auto& request : requests) {
            if (request.type == Stat::Request::Type::Bus)
            {
                cout << catalogue.GetBusInfo(request.text) << endl;
            }

            if (request.type == Stat::Request::Type::Stop)
            {
                cout << catalogue.GetStopInfo(request.text) << endl;
            }
        }
    }
}
////////////////////////////////////////////////////////////////// Cin

////////////////////////////////////////////////////////////////// File
//int main()
//{
//    //Test();
//
//    TransportCatalogue catalogue;
//
//    // Начало чтения запросов на заполнение справочника
//    ifstream in("input.txt");
//
//    if (in.is_open())
//    {
//        vector<Input::Request> requests = Input::GetRequests(in);
//        for (const auto& request : requests) {
//            if (request.type == Input::Request::Type::Stop)
//            {
//                catalogue.AddStop(request.text);
//            }
//		}
//		for (const auto& request : requests) {
//            if (request.type == Input::Request::Type::Stop)
//			{
//				catalogue.SetStopDistances(request.text);
//			}
//			if (request.type == Input::Request::Type::Bus)
//			{
//				catalogue.AddBus(request.text);
//			}
//		}
//	}
//    else
//    {
//        cout << "Can not open file"s << endl;
//    }
//
//    // Конец чтения запросов на заполнение справочника
//
//
//    // Начало чтения запросов к справочнику
//    ofstream out("output.txt");
//
//    if (out.is_open())
//    {
//        vector<Stat::Request> requests = Stat::GetRequests(in);
//        for (const auto& request : requests) {
//            if (request.type == Stat::Request::Type::Bus)
//            {
//                out << catalogue.GetBusInfo(request.text) << endl;
//            }
//            if (request.type == Stat::Request::Type::Stop)
//			{
//				out << catalogue.GetStopInfo(request.text) << endl;
//			}
//        }
//    }
//    else
//    {
//        cout << "Can not open file"s << endl;
//    }
//
//    in.close();
//    out.close();
//    // Конец чтения запросов к справочнику
//}
////////////////////////////////////////////////////////////////// File

////////////////////////////////////////////////////////////////// Test
//int main()
//{
//	Tests::Test();
//}
////////////////////////////////////////////////////////////////// Test
