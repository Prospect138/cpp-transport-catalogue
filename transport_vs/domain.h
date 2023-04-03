#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <variant>

namespace transport_catalogue::catalog{

enum class RouteType {
    ROUND,
    NOT_ROUND
};

struct Stop{
    std::string stop_name;
    Coordinates coordinates;
    int id;
};

struct Bus{
    std::string rout_name;
    std::vector<Stop*> rout;
    RouteType type;
};

struct BusInfo{
    std::string rout_name;
    int num_of_stops;
    int uinque_stops;
    double length;
    double curvature;
};

struct RoutingSettings{
    //size_t
    double bus_wait_time_ = 0; // minute
    double bus_velocity_ = 0; // km/h
};

struct RoutStat{

    struct ItemsWait {
        std::string type;
        double time = 0;
        std::string stop_name;
    };

    struct ItemsBus {
        std::string type;
        double time = 0;
        size_t span_count = 0;
        std::string bus;
    };

    using VariantItem = std::variant<ItemsBus, ItemsWait>;

    double total_time = 0; // minute
    std::vector<VariantItem> items;
};

double GetMetrMinFromKmH(double km_h);

} //namespace transport_catalogue::catalog
