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

		// возвращает указатель на остановку по имени
		const Stop* GetStop(std::string_view stop_name) const;

		// возвращает указатель на автобус по имени
		const Bus* GetBus(std::string_view bus_name) const;

		StopInfo GetStopInfo(const Stop* stop) const;

		BusInfo GetBusInfo(const Bus* bus) const;

		// возвращает расстояние между остановками, заданное вручную
		int GetRealDistance(const Stop* from, const Stop* to) const;

		const std::deque<Bus>& GetBuses() const;

		const std::vector<const Stop*> GetStops() const;

		size_t GetStopsCount() const;

		const std::set<const Bus*, BusComparator> GetBusesForStop(const Stop* stop) const;

		// добавляет переданную остановку в справочник
		void AddStop(std::string_view name, Geo::Coordinates coords);

		// добавляет переданный автобус(маршрут) в справочник
		void AddBus(std::string_view name, const std::vector<const Stop*>& stops, bool is_roundtrip);

		// позволяет задать расстояние между парой остановок
		void SetDistance(const Stop* from, const Stop* to, int dist);

	private:

		size_t CountUniqueStops(const Bus* bus) const;

		double ComputeBusGeoDistance(const Bus* bus) const;

		int ComputeBusRealDistance(const Bus* bus) const;

	private:

		struct DistanceMapHasher
		{
			size_t operator()(std::pair<const Stop*, const Stop*> p) const {
				std::hash<const void*> hasher;
				return hasher(p.first) + 37 * hasher(p.second);
			}
		};

		using DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceMapHasher>;

		// дек для хранения данных об остановках
		// указатели/итераторы не инвалидируются при добавлении новых
		std::deque<Stop> stops_;

		// дек для хранения данных об автобусах (маршрутах)
		// указатели/итераторы не инвалидируются при добавлении новых
		std::deque<Bus> buses_;

		// хэш-мапа для быстрого обращения к остановке по её имени
		std::unordered_map<std::string_view, const Stop*> stop_name_to_stop_;

		// хэш-мапа для быстрого обращения к автобусу по его имени
		std::unordered_map<std::string_view, const Bus*> bus_name_to_bus_;

		// хэш-мапа для определения маршрутов, проходящих через остановку (в множестве упорядочены по имени)
		std::unordered_map<const Stop*, std::set<const Bus*, BusComparator>> stop_to_buses_;

		// хэш-мапа для хранения заданных расстояний между двумя остановками
		DistanceMap between_stops_distances_;
	};
}
