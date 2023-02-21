#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace transport_catalogue::catalog{

enum class RouteType {
    ROUND,
    NOT_ROUND
};

struct Stop{
    std::string stop_name;
    Coordinates coordinates;
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

}
