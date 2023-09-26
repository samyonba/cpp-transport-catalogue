#include <fstream>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "serialization.h"

using namespace std;

using namespace Transport;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        TransportCatalogue catalogue;
        JsonReader json_reader(catalogue, cin, cout);
        json_reader.ReadMakeBaseInput();
        serialization::SerializeTransportCatalogue(catalogue, json_reader.GetSerializationFileName(), json_reader.GetRenderSettings(), json_reader.GetRouterSettings());
    }
    else if (mode == "process_requests"sv) {

        TransportCatalogue catalogue;
        JsonReader json_reader(catalogue, cin, cout);
        json_reader.ReadProcessRequest();
        Rendering::RenderSettings render_settings;
        auto light_router = serialization::DeserializeTransportCatalogue(json_reader.GetSerializationFileName(), catalogue, render_settings);
        json_reader.SetRenderSettings(render_settings);
        json_reader.ProcessStatRequests(light_router);
    }
    else {
        PrintUsage();
        return 1;
    }
}