#include "input_reader.h"

#include <iostream>
#include <string>

using namespace std;
using namespace Transport;

Input::Request Input::ParseRawRequest(string_view request)
{
	Request result;
	if (request.substr(0, 4) == "Stop")
	{
		result.type = Request::Type::Stop;
		request.remove_prefix(5);

		return ParseStopRequest(request, result);
	}
	else
	{
		result.type = Request::Type::Bus;
		request.remove_prefix(4);

		return ParseBusRequest(request, result);
	}
}

Input::Request& Transport::Input::ParseStopRequest(std::string_view request, Request& result)
{
	size_t separator = request.find(':');
	result.name = request.substr(0, separator);
	request.remove_prefix(separator + 2);

	separator = request.find(',');
	result.coords.lat = stod(static_cast<string>(request.substr(0, separator)));
	request.remove_prefix(separator + 2);

	separator = request.find(',');
	if (separator == request.npos)
	{
		result.coords.lng = stod(static_cast<string>(request));
		request.remove_prefix(request.size());
		return result;
	}
	result.coords.lng = stod(static_cast<string>(request.substr(0, separator)));
	request.remove_prefix(separator + 2);

	int dist = 0;
	string target;

	while (!request.empty())
	{
		separator = request.find(' ');
		dist = stoi(static_cast<string>(request.substr(0, separator - 1)));
		request.remove_prefix(separator + 4);
		separator = request.find(',');
		target = request.substr(0, separator);
		if (separator == request.npos)
		{
			request.remove_prefix(request.size());
		}
		else
		{
			request.remove_prefix(separator + 2);
		}

		result.distances.push_back({ target, dist });
	}
	return result;
}

Input::Request& Transport::Input::ParseBusRequest(std::string_view request, Request& result)
{
	char request_separator = '-';
	bool is_circular = false;

	size_t separator = request.find(':');
	result.name = request.substr(0, separator);
	request.remove_prefix(separator + 2);

	if (request.find('-') == request.npos)
	{
		request_separator = '>';
		is_circular = true;
	}

	while (!request.empty())
	{
		separator = request.find(request_separator);
		size_t bound = separator == request.npos ? separator : separator - 1;
		result.stops.push_back(static_cast<string>(request.substr(0, bound)));
		request.remove_prefix(separator == request.npos ? request.size() : separator + 2);
	}

	if (!is_circular)
	{
		vector<string> additional(result.stops.begin(), result.stops.end() - 1);
		reverse(additional.begin(), additional.end());
		for (auto& stop : additional) {
			result.stops.push_back(stop);
		}
	}

	return result;
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
