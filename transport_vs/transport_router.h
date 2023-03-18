#pragma once
#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "unordered_map"
#include "unordered_set"

#include <memory>
#include <optional>
#include <algorithm>
#include <iostream>

namespace transport_router {

using namespace transport_catalogue::catalog;

class TransportRouter {    
public:
    using OptRouteInfo = std::optional<graph::Router<double>::RouteInfo>;

    TransportRouter() = default;

    // создает граф
    void CreateGraph(TransportCatalogue& db);

    // возвращает маршрут и статистику по нему
    std::optional<RoutStat> GetRouteStat(size_t id_stop_from, size_t id_stop_to) const ;

    // параметры маршрута скорость, ожидание
    RoutingSettings settings_;

    // Граф не создан
    bool GetGraphIsNoInit() const;

private:
    // дополнительная информация о ребре
    struct EdgeAditionInfo {
        std::string bus_name; // имя автобуса едущего по ребру
        size_t count_spans = 0; // колво пролетов между остановками в этом ребре
    };
    // хранит дополнительная информация о ребре по индексу ребра
    std::vector<EdgeAditionInfo> edges_buses_;

    // хранит имена остановок по индексу
    std::vector<std::string_view> id_for_stops;

    // граф
    std::optional<graph::DirectedWeightedGraph<double>> opt_graph_;

    // создает и возвращает маршрутизатор если его еще нет
    const std::unique_ptr<graph::Router<double>>& GetRouter() const;

    // маршрутизатор
    mutable std::unique_ptr<graph::Router<double>> up_router_;
};

} //namespace transport_router;