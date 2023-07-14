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
	JsonReader json_reader(catalogue);
	json_reader.ReadInput();
}

// ==================== File input =========================================================
//int main()
//{
//	TransportCatalogue catalogue;
//
//	ifstream in("data.txt");
//	ofstream out("output.txt");
//	if (in.is_open() && out.is_open()) {
//		//Rendering::RenderSettings render_settings;
//		JsonReader json_reader(catalogue, in, out);
//		json_reader.ReadInput();
//	}
//	in.close();
//	out.close();
//}
