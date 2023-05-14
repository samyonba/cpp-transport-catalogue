#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace Transport;

int main()
{
	TransportCatalogue catalogue;
	JsonReader json_reader(catalogue, cin, cout);
	json_reader.ReadInput();
}

