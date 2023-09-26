#pragma once

#include <utility>
#include "transport_catalogue.h"
#include "router.h"

#include "serialization.h"

namespace Transport {
	namespace Routing {
		struct RouterSettings {
			// ����� �������� �������� �� ���������, � �������. �������� � ����� ����� �� 1 �� 1000
			int bus_wait_time = 6;

			// �������� ��������, � ��/�. �������� � ������������ ����� �� 1 �� 1000
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

			// ��������� ���������� �������, ������ �������� ��������� ����������� � ����������
			std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

			// �������� ���������� ���������� � ����� �����
			EdgeInfo GetEdgeInfo(graph::EdgeId id) const;
			
			const RouterSettings& GetRouterSettings() const;
			const std::vector<EdgeInfo>& GetEdgesInfo() const;
			const graph::DirectedWeightedGraph<double>& GetGraph() const;
			const graph::Router<double>& GetRouter() const;

		private:
			// ����������� ���������� � ��� ������� � ������� ��� �������� � routing_settings_ �������� ���������
			double CalculateWeight(double distance) const;

			// ������ ���� �� ������ �� catalogue_
			graph::DirectedWeightedGraph<double> BuildGraph();
			void AddStops(graph::DirectedWeightedGraph<double>& graph);
			void AddRoutes(graph::DirectedWeightedGraph<double>& graph);
			void AddRoute(size_t from_index, size_t to_index, const Transport::Bus& route, graph::DirectedWeightedGraph<double>& graph);

			std::unordered_map<const Stop*, size_t> BuildVertexIndex();

		private:
			const Transport::TransportCatalogue& catalogue_;

			// ��������� ��������������
			RouterSettings router_settings_;

			// �������, �������������� ��������� �� ��������� ������ ��������������� ��� ������� ����� (����� �� ���������)
			std::unordered_map<const Stop*, size_t> vertex_index_;

			// ���������� ���������� � ������ �����
			std::vector<EdgeInfo> edges_info_;

			graph::DirectedWeightedGraph<double> graph_;

			// �������������
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

			// ��������� ���������� �������, ������ �������� ��������� ����������� � ����������
			std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

			EdgeInfo GetEdgeInfo(graph::EdgeId id) const;

		private:
			std::unordered_map<const Stop*, size_t> BuildVertexIndex();

		private:
			const TransportCatalogue& catalogue_;

			// ���������� ���������� � ������ ����
			std::vector<EdgeInfo> edges_info_;

			// ����� �����
			std::vector<graph::Edge<double>> edges_;

			// ���������� �� ����������� ���������
			graph::Router<double>::RoutesInternalData routes_internal_data_;
			
			// �������, �������������� ��������� �� ��������� ������ ��������������� ��� ������� ����� (����� �� ���������)
			std::unordered_map<const Stop*, size_t> vertex_index_;
		};
	}
}