#include "input_reader.h"

#include <iostream>
#include <string>

using namespace std;
using namespace Transport;

Input::Request Input::ParseRawRequest(string raw_request)
{
	string right;
	if (raw_request.substr(0, 4) == "Stop")
	{
		right = raw_request.substr(5);
		return { Request::Type::Stop, right };
	}
	else
	{
		right = raw_request.substr(4);
		return { Request::Type::Bus, right };
	}
}

vector<Input::Request> Input::GetRequests(std::istream& input)
{
	int requests_count = 0;
	vector<Request> requests;

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
