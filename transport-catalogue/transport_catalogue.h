#pragma once

#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <set>

#include "geo.h"
#include "domain.h"

namespace Transport {

	class TransportCatalogue
	{
	public:
		struct DistanceMapHasher
		{
			size_t operator()(std::pair<const Stop*, const Stop*> p) const {
				std::hash<const void*> hasher;
				return hasher(p.first) + 37 * hasher(p.second);
			}
		};

		using DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceMapHasher>;

		using StopToRoutesMap = std::unordered_map<const Stop*, std::set<const Bus*, BusComparator>>;

	public:
		// ���������� ��������� �� ��������� �� �����
		const Stop* GetStop(std::string_view stop_name) const;

		// ���������� ��������� �� ������� �� �����
		const Bus* GetBus(std::string_view bus_name) const;

		StopInfo GetStopInfo(const Stop* stop) const;

		BusInfo GetBusInfo(const Bus* bus) const;

		// ���������� ���������� ����� �����������, �������� �������
		int GetRealDistance(const Stop* from, const Stop* to) const;

		const std::deque<Bus>& GetBuses() const;

		const std::vector<const Stop*> GetStops() const;

		size_t GetStopsCount() const;

		const std::set<const Bus*, BusComparator> GetStopToBuses(const Stop* stop) const;

		// ��������� ���������� ��������� � ����������
		void AddStop(std::string_view name, Geo::Coordinates coords);

		// ��������� ���������� �������(�������) � ����������
		void AddBus(std::string_view name, const std::vector<const Stop*>& stops, bool is_roundtrip);

		// ��������� ������ ���������� ����� ����� ���������
		void SetDistance(const Stop* from, const Stop* to, int dist);

		// reading for serialization
		const DistanceMap& GetDistanceMap() const;

		// reading for serialization
		const StopToRoutesMap& GetStopsToBuses() const;

		void SetStops(std::deque<Stop> stops);

		void SetBuses(std::deque<Bus> buses);

		void SetStopToBuses(StopToRoutesMap stop_to_buses);

		void SetDistanceMap(DistanceMap distance_map);

	private:

		size_t CountUniqueStops(const Bus* bus) const;

		double ComputeBusGeoDistance(const Bus* bus) const;

		int ComputeBusRealDistance(const Bus* bus) const;

	private:

		// ��� ��� �������� ������ �� ����������
		// ���������/��������� �� �������������� ��� ���������� �����
		std::deque<Stop> stops_;

		// ��� ��� �������� ������ �� ��������� (���������)
		// ���������/��������� �� �������������� ��� ���������� �����
		std::deque<Bus> buses_;

		// ���-���� ��� �������� ��������� � ��������� �� � �����
		std::unordered_map<std::string_view, const Stop*> stop_name_to_stop_;

		// ���-���� ��� �������� ��������� � �������� �� ��� �����
		std::unordered_map<std::string_view, const Bus*> bus_name_to_bus_;

		// ���-���� ��� ����������� ���������, ���������� ����� ��������� (� ��������� ����������� �� �����)
		std::unordered_map<const Stop*, std::set<const Bus*, BusComparator>> stop_to_buses_;

		// ���-���� ��� �������� �������� ���������� ����� ����� �����������
		DistanceMap between_stops_distances_;
	};
}
