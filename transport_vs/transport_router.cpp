#include "transport_router.h"

using namespace transport_catalogue::catalog;

namespace transport_router {

void TransportRouter::CreateGraph(TransportCatalogue& catalog) {
    graph::DirectedWeightedGraph<double> graph(catalog.GetStops().size());
    
    id_for_stops.resize(catalog.GetStops().size());

    for(const Bus& bus : catalog.GetAllBus()) {

        for (auto it_from = bus.rout.begin(); it_from != bus.rout.end(); ++it_from) {
            const Stop* stop_from = *it_from;
            double length = 0;
            const Stop* prev_stop = stop_from;
            id_for_stops[stop_from->id] = stop_from->stop_name;

            for (auto it_to = std::next(it_from); it_to != bus.rout.end(); ++it_to) {
                const Stop* stop_to = *it_to;
                length += catalog.GetCalculateDistance(prev_stop, stop_to);
                prev_stop = stop_to;
                double time_on_bus = length / GetMetrMinFromKmH( settings_.bus_velocity_ ); // minute
                graph::Edge<double> edge1 { static_cast<graph::VertexId>(stop_from -> id), static_cast<graph::VertexId>(stop_to -> id), time_on_bus+settings_.bus_wait_time_ };
                graph.AddEdge(edge1);
                edges_buses_.push_back({bus.rout_name, static_cast<graph::VertexId>(std::distance(it_from, it_to))});
            }
        }
    }
    opt_graph_ = std::move(graph);
    up_router_ = std::make_unique<graph::Router<double>>(opt_graph_.value());
}

std::optional<RoutStat> TransportRouter::GetRouteStat(size_t id_stop_from, size_t id_stop_to) const {

    const OptRouteInfo opt_route_info = up_router_->BuildRoute(id_stop_from, id_stop_to);

    if(! opt_route_info.has_value()) {
        return std::nullopt;
    }

    const graph::Router<double>::RouteInfo& route_info = opt_route_info.value();
    double total_time = route_info.weight;
    std::vector<RoutStat::VariantItem> items;
    for(const auto& edge_id : route_info.edges) {

        const auto& edge = opt_graph_.value().GetEdge(edge_id);

        const auto [bus_num, span_count] = edges_buses_[edge_id];
        items.push_back(RoutStat::ItemsWait{"Wait", static_cast<double>(settings_.bus_wait_time_), std::string(id_for_stops[edge.from])});

        items.push_back(RoutStat::ItemsBus{"Bus", edge.weight - static_cast<double>(settings_.bus_wait_time_), span_count, std::string(bus_num)});
    }
    return RoutStat{total_time, items};
}

bool TransportRouter::IsSomething() const {
    return ! opt_graph_.has_value();
}

} //namespace transport_router