#include "transport_router.h"

std::optional<graph::Router<double>::RouteInfo> Transport::Routing::TransportRouter::BuildRoute(std::string_view from, std::string_view to) const
{
	return router_.BuildRoute(vertex_index_.at(catalogue_.GetStop(from)), vertex_index_.at(catalogue_.GetStop(to)));
}

Transport::Routing::EdgeInfo Transport::Routing::TransportRouter::GetEdgeInfo(graph::EdgeId id) const
{
	return edges_info_[id];
}

graph::DirectedWeightedGraph<double> Transport::Routing::TransportRouter::BuildGraph()
{
	graph::DirectedWeightedGraph<double> graph(catalogue_.GetStopsCount() * 2);
	AddStops(graph);
	AddRoutes(graph);
	return graph;
}

void Transport::Routing::TransportRouter::AddStops(graph::DirectedWeightedGraph<double>& graph)
{
	const auto stops = catalogue_.GetStops();

	//Добавляем ребра между вершинами входа на остановку и отправления с остановки
	for (size_t i = 0; i < catalogue_.GetStopsCount(); i++)
	{
		// { from, to, weight }
		graph::VertexId enter_stop_index = i * 2;
		graph::VertexId exit_stop_index = enter_stop_index + 1;
		graph.AddEdge({ enter_stop_index, exit_stop_index, static_cast<double>(routing_settings_.bus_wait_time) });

		// { span_count, name }
		edges_info_.push_back({ 0, stops[i]->name, static_cast<double>(routing_settings_.bus_wait_time) });
	}
}

void Transport::Routing::TransportRouter::AddRoutes(graph::DirectedWeightedGraph<double>& graph)
{
	for (const Bus& route : catalogue_.GetBuses())
	{
		if (route.stops.size() <= 1)
		{
			continue;
		}

		// добавление ребер кольцевого маршрута
		if (route.is_roundtrip)
		{
			AddRoute(0, route.stops.size(), route, graph);
		}
		// добавление ребер некольцевого маршрута
		else
		{
			AddRoute(0, route.stops.size() / 2 + 1, route, graph);
			AddRoute(route.stops.size() / 2, route.stops.size(), route, graph);
		}
	}
}

void Transport::Routing::TransportRouter::AddRoute(size_t from_index, size_t to_index, const Transport::Bus& route, graph::DirectedWeightedGraph<double>& graph)
{
	for (size_t from = from_index; from < to_index - 1; from++)
	{
		std::unordered_map<const Stop*, double> prev_weights;

		double weight = 0;
		size_t span_count = 0;

		//size_t limit = from == 0 ? route.stops.size() - 1 : route.stops.size();
		for (size_t to = from + 1; to < to_index; to++)
		{
			weight += CalculateWeight(catalogue_.GetRealDistance(route.stops[to - 1], route.stops[to]));
			++span_count;

			// Если автобус делает петлю и мы повторно попадаем из from в to
			if (prev_weights.count(route.stops[to]) != 0)
			{
				double prev_weight = prev_weights.at(route.stops[to]);
				// Если быстрее подождать следующего автобуса, а не делать петлю, дальнейшие расчеты из текущей остановки прекращаются
				if (weight >= prev_weight + routing_settings_.bus_wait_time)
				{
					break;
				}
			}
			prev_weights[route.stops[to]] = weight;
			graph.AddEdge({ vertex_index_.at(route.stops[from]) + 1, vertex_index_.at(route.stops[to]), weight });
			edges_info_.push_back(EdgeInfo{ span_count, route.name, weight });
		}
	}
}

std::unordered_map<const Transport::Stop*, size_t> Transport::Routing::TransportRouter::BuildVertexIndex()
{
	const auto stops = catalogue_.GetStops();
	std::unordered_map<const Transport::Stop*, size_t> index;
	for (size_t i = 0; i < stops.size(); i++)
	{
		index[stops[i]] = i * 2;
	}
	return index;
}

double Transport::Routing::TransportRouter::CalculateWeight(double distance) const
{
	double real_time_to_duration = 1000. / 60;
	return distance / routing_settings_.bus_velocity / real_time_to_duration;
}
