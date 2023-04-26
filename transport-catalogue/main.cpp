#include "transport_catalogue.h"
#include "input_reader.h"
#include "test_transport_catalogue.h"
#include "stat_reader.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace Transport;

int main()
{
    TransportCatalogue catalogue;

    // по умолчанию - cin/cout
    /*Input::ReadInput(catalogue);
    Stat::ReadStat(catalogue);*/

    // file input:

    ifstream in("data.txt");
    ofstream out("output.txt");
	if (in.is_open() && out.is_open()) {
		Input::ReadInput(catalogue, in);
		Stat::ReadStat(catalogue, in, out);
	}
    in.close();
    out.close();
}
