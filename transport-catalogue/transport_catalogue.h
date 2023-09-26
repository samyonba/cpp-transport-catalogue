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
		// возвращает указатель на остановку по имени
		const Stop* GetStop(std::string_view stop_name) const;

		// возвращает указатель на автобус по имени
		const Bus* GetBus(std::string_view bus_name) const;

		StopInfo GetStopInfo(const Stop* stop) const;

		BusInfo GetBusInfo(const Bus* bus) const;

		// возвращает рассто€ние между остановками, заданное вручную
		int GetRealDistance(const Stop* from, const Stop* to) const;

		const std::deque<Bus>& GetBuses() const;

		const std::vector<const Stop*> GetStops() const;

		size_t GetStopsCount() const;

		const std::set<const Bus*, BusComparator> GetStopToBuses(const Stop* stop) const;

		// добавл€ет переданную остановку в справочник
		void AddStop(std::string_view name, Geo::Coordinates coords);

		// добавл€ет переданный автобус(маршрут) в справочник
		void AddBus(std::string_view name, const std::vector<const Stop*>& stops, bool is_roundtrip);

		// позвол€ет задать рассто€ние между парой остановок
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

		// дек дл€ хранени€ данных об остановках
		// указатели/итераторы не инвалидируютс€ при добавлении новых
		std::deque<Stop> stops_;

		// дек дл€ хранени€ данных об автобусах (маршрутах)
		// указатели/итераторы не инвалидируютс€ при добавлении новых
		std::deque<Bus> buses_;

		// хэш-мапа дл€ быстрого обращени€ к остановке по еЄ имени
		std::unordered_map<std::string_view, const Stop*> stop_name_to_stop_;

		// хэш-мапа дл€ быстрого обращени€ к автобусу по его имени
		std::unordered_map<std::string_view, const Bus*> bus_name_to_bus_;

		// хэш-мапа дл€ определени€ маршрутов, проход€щих через остановку (в множестве упор€дочены по имени)
		std::unordered_map<const Stop*, std::set<const Bus*, BusComparator>> stop_to_buses_;

		// хэш-мапа дл€ хранени€ заданных рассто€ний между двум€ остановками
		DistanceMap between_stops_distances_;
	};
}
