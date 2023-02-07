#include "map_renderer.h"


#include <optional>

namespace renderer{

using namespace transport_catalogue;

svg::Point SphereProjector::operator()(Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

// Base function to render svg and put it to map
void MapRenderer::RenderSvgMap(const catalog::TransportCatalogue& catalog, std::ostream& out) {
    // Get all stops
    const std::map<std::string_view, const catalog::Stop*> stops = catalog.GetStops();
    stops_ = &stops;

    // prepare data for SphereProjector init
    std::vector<Coordinates> all_route_stops_coordinates;
    for (const auto& stop : stops) {
        if (catalog.GetBusesForStop(stop.first).empty()) {
            continue;
        }
        all_route_stops_coordinates.push_back(stop.second->coordinates);
    }
    SphereProjector projector(all_route_stops_coordinates.begin(), all_route_stops_coordinates.end(),
                               settings_.width, settings_.height, settings_.padding);
    projector_ = &projector;

    // Get all routs with their routes
    const std::map<std::string_view, const catalog::Bus*> routes = catalog.GetBuses();
    routes_ = &routes;

    // Create doc to store all data in
    svg::Document svg_doc;

    // And render all of the
    RenderLines(svg_doc);
    RenderRouteNames(svg_doc);
    RenderStopCircles(catalog, svg_doc);
    RenderStopNames(catalog, svg_doc);

    // Render formed doc and put result in out
    svg_doc.Render(out);

    stops_ = nullptr;
    routes_ = nullptr;
    projector_ = nullptr;
}

svg::Color MapRenderer::GetNextPalleteColor(size_t &color_count) const {
    if (color_count >= settings_.color_palette.size()) {
        color_count = 0;
    }
    return settings_.color_palette[color_count++];
}

svg::Color MapRenderer::GetPalletColor(size_t route_number) const {
    if (routes_ == nullptr || route_number >= routes_->size()){
        return {};
    }
    size_t index = route_number % settings_.color_palette.size();

    return settings_.color_palette[index];
}


void MapRenderer::RenderLines(svg::Document &svg_doc) const {
    size_t color_count = 0;
    auto projector = *projector_;
    for (const auto route : *routes_) {
        if (route.second->rout.empty()) {
            continue;
        }
        svg::Color palette_color = GetNextPalleteColor(color_count);

        svg::Polyline line;
        line.SetStrokeColor(palette_color).SetFillColor({}).SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);


        for (auto iter = route.second->rout.begin(); iter != route.second->rout.end(); ++iter) {
            line.AddPoint(projector( (*iter)->coordinates ));
        }

        svg_doc.Add(std::move(line));
    }
}

void MapRenderer::RenderRouteNames(svg::Document& svg_doc) const {
    using namespace std::literals;
    auto projector = *projector_;
    size_t color_count = 0;

    // Iterating through buses to mark its rout names
    for (auto route : *routes_) {
        if (route.second->rout.empty()) {
            continue;
        }

        svg::Text name_start_text;

        name_start_text.SetData(std::string{route.first}).SetPosition(projector(route.second->rout.front()->coordinates))
        .SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetFillColor(GetNextPalleteColor(color_count));

        svg::Text name_start_plate = name_start_text;
        name_start_plate.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg_doc.Add(name_start_plate);
        svg_doc.Add(name_start_text);\

        size_t middle = route.second->rout.size()/2;
        
        // Marking ending station if its not the same with first station
        if (route.second->rout.front()->stop_name == route.second->rout[middle]->stop_name) {
            continue;
        }

        if (route.second->type == catalog::RouteType::NOT_ROUND) {
            name_start_text.SetPosition(projector(route.second->rout[middle]->coordinates));
            name_start_plate.SetPosition(projector(route.second->rout[middle]->coordinates));
            svg_doc.Add(name_start_plate);
            svg_doc.Add(name_start_text);
        }
    }
}

void MapRenderer::RenderStopCircles(const catalog::TransportCatalogue& catalog, svg::Document& svg_doc) const {
    using namespace std::literals;
    auto projector = *projector_;

    // Iterating through stop to mark them as circles
    for (const auto& stop : *stops_) {
        if (catalog.GetBusesForStop(stop.first).empty()) {
            continue;
        }

        svg::Circle stop_circle;
        stop_circle.SetCenter( projector(stop.second->coordinates) ).SetRadius(settings_.stop_radius).SetFillColor("white"s);
        svg_doc.Add(stop_circle);
    }
}

void MapRenderer::RenderStopNames(const catalog::TransportCatalogue& catalog, svg::Document& svg_doc) const {
    using namespace std::literals;
    auto projector = *projector_;

    // And through stops again but for names
    for (const auto& stop : *stops_) {
        if (catalog.GetBusesForStop(stop.first).empty() ) continue;

        svg::Text stop_name;
        stop_name.SetPosition(projector(stop.second->coordinates)).SetOffset(settings_.stop_label_offset)
        .SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s).SetData(std::string{stop.first});

        svg::Text stop_plate = stop_name;
        stop_plate.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        stop_name.SetFillColor("black"s);

        svg_doc.Add(stop_plate);
        svg_doc.Add(stop_name);
    }
}
}