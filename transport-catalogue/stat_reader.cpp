#include "stat_reader.h"

using namespace std;
using namespace Transport;

Stat::Request Stat::ParseRawRequest(std::string raw_request)
{
	string right;
	if (raw_request.substr(0, 3) == "Bus")
	{
		right = raw_request.substr(4);
		return { Request::Type::Bus, right };
	}

	if (raw_request.substr(0, 4) == "Stop")
	{
		right = raw_request.substr(5);
		return { Request::Type::Stop, right };
	}
	
	return Request();
}

vector<Stat::Request> Stat::GetRequests(std::istream& input)
{
    vector<Request> requests;
	int requests_count = 0;

	input >> requests_count;
	input.get();

	string cur_request;

	for (size_t i = 0; i < requests_count; i++)
	{
		std::getline(input, cur_request);
		requests.push_back(ParseRawRequest(std::move(cur_request)));
	}

	return requests;
}
