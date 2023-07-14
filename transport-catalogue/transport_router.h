#pragma once

#include "transport_catalogue.h"
#include "router.h"

namespace Transport {
	namespace Routing {
		struct RouterSettings {
			// время ожидания автобуса на остановке, в минутах. Значение — целое число от 1 до 1000
			int bus_wait_time = 6;

			// скорость автобуса, в км/ч. Значение — вещественное число от 1 до 1000
			int bus_velocity = 40;
		};

		struct EdgeInfo
		{
			size_t span_count = 0;
			std::string name;
			double weight = 0;
		};

		class TransportRouter {
		public:
			TransportRouter(const Transport::TransportCatalogue& catalogue, RouterSettings settings)
				: catalogue_(catalogue), routing_settings_(settings), vertex_index_(std::move(BuildVertexIndex())), edges_info_(), graph_(BuildGraph()), router_(graph_) {}

			// построить кратчайший маршрут, указав названия остановок отправления и назначения
			std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

			// получить справочную информацию о ребре графа
			EdgeInfo GetEdgeInfo(graph::EdgeId id) const;

		private:
			// преобразует расстояние в вес отрезка в минутах для заданной в routing_settings_ скорости автобусов
			double CalculateWeight(double distance) const;

			// строит граф по данным из catalogue_
			graph::DirectedWeightedGraph<double> BuildGraph();

			std::unordered_map<const Stop*, size_t> BuildVertexIndex();

		private:
			const Transport::TransportCatalogue& catalogue_;

			// настройки маршрутизатора
			RouterSettings routing_settings_;

			// словарь, сопоставляющий указателю на остановку индекс соответствующей ему вершины графа (входа на остановку)
			std::unordered_map<const Stop*, size_t> vertex_index_;

			// справочная информация о ребрах графа
			std::vector<EdgeInfo> edges_info_;

			graph::DirectedWeightedGraph<double> graph_;

			// маршрутизатор
			graph::Router<double> router_;
		};
	}
}