#pragma once

#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <set>

#include "geo.h"

namespace Transport {

	class TransportCatalogue
	{
	public:

		struct DistanceMapHasher
		{
			size_t operator()(std::pair<std::string_view, std::string_view> p) const {
				std::hash<std::string_view> sv_hasher;
				return sv_hasher(p.first) + 37 * sv_hasher(p.second);
			}
		};

		using DistanceMap = std::unordered_map<std::pair<std::string_view, std::string_view>, int, DistanceMapHasher>;

		struct Stop
		{
			std::string name;
			Geo::Coordinates coords;

			friend std::ostream& operator<<(std::ostream& out, const Stop& stop);
			bool operator==(const Stop& other) const;
		};

		struct Bus
		{
			std::string name;
			std::vector<const Stop*> stops;

			friend std::ostream& operator<<(std::ostream& out, const Bus& bus);
		};

		struct BusInfo
		{
			std::string name;
			bool exists = false;
			size_t stops_count = 0;
			size_t unique_stops = 0;
			double geo_length = 0.;
			int real_length = 0;

			friend std::ostream& operator<<(std::ostream& out, const BusInfo& info);
		};

		// ���������� ��� ����������� ������� ���������� �������� ������ stop_name_to_buses_
		struct BusComparator
		{
			bool operator()(const Bus* lhv, const Bus* rhv) const {
				return lhv->name < rhv->name;
			}
		};

		struct StopInfo
		{
			std::string name;
			bool exists = false;
			const std::set<const Bus*, BusComparator>* buses = nullptr;

			friend std::ostream& operator<<(std::ostream& out, const StopInfo& info);
		};

	public:

		// ��������� ���������� ��������� � ����������
		void AddStop(std::string_view request_text);

		// 
		void SetStopDistances(std::string_view request_text);

		// ��������� ���������� �������(�������) � ����������
		void AddBus(std::string_view request_text);

		// ���������� ��������� �� ��������� �� �����
		const Stop* GetStop(std::string_view stop_name) const;

		int GetRealDistance(std::string_view lhv, std::string_view rhv) const;

		// ���������� ��������� �� ������� �� �����
		const Bus* GetBus(std::string_view bus_name) const;

		StopInfo GetStopInfo(std::string_view stop_name) const;

		BusInfo GetBusInfo(std::string_view bus_name) const;

	private:

		// ���������� ���� �������� { �������������� ������ Stop, ����� ������ ������� � ��������� ���������� �� �������� ���������}
		std::pair<Stop, std::string_view> ParseStopRequest(std::string_view request);

		// ��������� �������� ��������� � ������ ������� � ���������� ������������ �� �������
		// ������ ������ � between_stops_distances_
		void WriteStopDistances(std::string_view name, std::string_view text);

		Bus ParseBusRequest(std::string_view request);

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

		// ���-���� ��� ����������� ���������, ���������� ����� ���������
		std::unordered_map<std::string_view, std::set<const Bus*, BusComparator>> stop_name_to_buses_;

		// ���-���� ��� �������� �������� ���������� ����� ����� �����������
		DistanceMap between_stops_distances_;
	};
}
