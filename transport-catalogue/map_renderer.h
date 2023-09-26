#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <deque>

namespace Transport {
    namespace Rendering {
        inline const double EPSILON = 1e-6;

        class SphereProjector {
        public:
            // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
                : padding_(padding) //
            {
                // ���� ����� ����������� ����� �� ������, ��������� ������
                if (points_begin == points_end) {
                    return;
                }

                // ������� ����� � ����������� � ������������ ��������
                const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
                min_lon_ = left_it->lng;
                const double max_lon = right_it->lng;

                // ������� ����� � ����������� � ������������ �������
                const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
                const double min_lat = bottom_it->lat;
                max_lat_ = top_it->lat;

                // ��������� ����������� ��������������� ����� ���������� x
                std::optional<double> width_zoom;
                if (!IsZero(max_lon - min_lon_)) {
                    width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
                }

                // ��������� ����������� ��������������� ����� ���������� y
                std::optional<double> height_zoom;
                if (!IsZero(max_lat_ - min_lat)) {
                    height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
                }

                if (width_zoom && height_zoom) {
                    // ������������ ��������������� �� ������ � ������ ���������,
                    // ���� ����������� �� ���
                    zoom_coeff_ = std::min(*width_zoom, *height_zoom);
                }
                else if (width_zoom) {
                    // ����������� ��������������� �� ������ ���������, ���������� ���
                    zoom_coeff_ = *width_zoom;
                }
                else if (height_zoom) {
                    // ����������� ��������������� �� ������ ���������, ���������� ���
                    zoom_coeff_ = *height_zoom;
                }
            }

            // ���������� ������ � ������� � ���������� ������ SVG-�����������
            svg::Point operator()(Geo::Coordinates coords) const {
                return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
                };
            }

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;

            bool IsZero(double value) {
                return std::abs(value) < EPSILON;
            }
        };

        struct RenderSettings
        {
            double width = 1200.0;
            double height = 1200.0;

            double padding = 50.0;

            double line_width = 14.0;
            double stop_radius = 5.0;

            int bus_label_font_size = 20;
            svg::Point bus_label_offset = { 7.0, 15.0 };

            int stop_label_font_size = 20;
            svg::Point stop_label_offset = { 7.0, -3.0 };

            svg::Color underlayer_color = svg::Rgba{ 255, 255, 255, 0.85 };
            double underlayer_width = 3.0;

            std::vector<svg::Color> color_palette;
        };

        class MapRenderer
        {
        public:
            MapRenderer(const TransportCatalogue& catalogue, const SphereProjector& projector, const RenderSettings& settings, std::ostream& out = std::cout)
                : catalogue_(catalogue), projector_(projector), settings_(settings), out_(out) {}

            void Render();

        private:
            void AddRoutes();
            void AddStops();

            void AddRoute(const Transport::Bus& route);
            void AddRouteTitle(const Transport::Bus& route);
            void AddStopCircle(const Transport::Stop& stop);
            void AddStopTitle(const Transport::Stop& stop);

        private:
            const TransportCatalogue& catalogue_;
            const SphereProjector& projector_;
            const RenderSettings& settings_;
            std::ostream& out_;

            svg::Document canvas_;
            std::vector<svg::Polyline> routes_;
            std::vector<svg::Text> route_titles_;
            std::vector<svg::Circle> stop_circles_;
            std::vector<svg::Text> stop_titles_;
            size_t routes_count_ = 0;
        };
    }
}
