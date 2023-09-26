#pragma once

#include <utility>
#include "transport_catalogue.h"
#include "router.h"

#include "serialization.h"

namespace Transport {
	namespace Routing {
		struct RouterSettings {
			// врем€ ожидани€ автобуса на остановке, в минутах. «начение Ч целое число от 1 до 1000
			int bus_wait_time = 6;

			// скорость автобуса, в км/ч. «начение Ч вещественное число от 1 до 1000
			int bus_velocity = 40;
		};

		struct EdgeInfo
		{
			size_t span_count = 0;
			std::string name;
			double weight = 0;
		};

		class TransportRouter {
			//friend void serialization::SerializeTransportRouter(const TransportRouter&, tc_serialization::TransportRouter&);

		public:
			TransportRouter(const Transport::TransportCatalogue& catalogue, RouterSettings settings)
			  : catalogue_(catalogue),
				router_settings_(settings),
				vertex_index_(std::move(BuildVertexIndex())),
				edges_info_(),
				graph_(BuildGraph()),
				router_(graph_) {}

			// построить кратчайший маршрут, указав названи€ остановок отправлени€ и назначени€
			std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

			// получить справочную информацию о ребре графа
			EdgeInfo GetEdgeInfo(graph::EdgeId id) const;
			
			const RouterSettings& GetRouterSettings() const;
			const std::vector<EdgeInfo>& GetEdgesInfo() const;
			const graph::DirectedWeightedGraph<double>& GetGraph() const;
			const graph::Router<double>& GetRouter() const;

		private:
			// преобразует рассто€ние в вес отрезка в минутах дл€ заданной в routing_settings_ скорости автобусов
			double CalculateWeight(double distance) const;

			// строит граф по данным из catalogue_
			graph::DirectedWeightedGraph<double> BuildGraph();
			void AddStops(graph::DirectedWeightedGraph<double>& graph);
			void AddRoutes(graph::DirectedWeightedGraph<double>& graph);
			void AddRoute(size_t from_index, size_t to_index, const Transport::Bus& route, graph::DirectedWeightedGraph<double>& graph);

			std::unordered_map<const Stop*, size_t> BuildVertexIndex();

		private:
			const Transport::TransportCatalogue& catalogue_;

			// настройки маршрутизатора
			RouterSettings router_settings_;

			// словарь, сопоставл€ющий указателю на остановку индекс соответствующей ему вершины графа (входа на остановку)
			std::unordered_map<const Stop*, size_t> vertex_index_;

			// справочна€ информаци€ о ребрах графа
			std::vector<EdgeInfo> edges_info_;

			graph::DirectedWeightedGraph<double> graph_;

			// маршрутизатор
			graph::Router<double> router_;
		};

		class LightTransportRouter
		{
		public:
			LightTransportRouter() = default;

			LightTransportRouter(const TransportCatalogue& catalogue,
				const std::vector<EdgeInfo>& edges_info,
				const std::vector<graph::Edge<double>>& edges,
				const graph::Router<double>::RoutesInternalData& routes_internal_data);

			// построить кратчайший маршрут, указав названи€ остановок отправлени€ и назначени€
			std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

			EdgeInfo GetEdgeInfo(graph::EdgeId id) const;

		private:
			std::unordered_map<const Stop*, size_t> BuildVertexIndex();

		private:
			const TransportCatalogue& catalogue_;

			// справочна€ информаци€ о ребрах пути
			std::vector<EdgeInfo> edges_info_;

			// ребра графа
			std::vector<graph::Edge<double>> edges_;

			// информаци€ об оптимальных маршрутах
			graph::Router<double>::RoutesInternalData routes_internal_data_;
			
			// словарь, сопоставл€ющий указателю на остановку индекс соответствующей ему вершины графа (входа на остановку)
			std::unordered_map<const Stop*, size_t> vertex_index_;
		};
	}
}