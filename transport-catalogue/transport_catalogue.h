#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <set>

namespace transport_catalogue::catalog{

enum class RouteType {
    ROUND,
    NOT_ROUND
};

struct BusQuery{
    std::string route_name;
    RouteType type;
    std::vector<std::string> query_content;
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

namespace detail {
    
    struct Hasher_stop{
    size_t operator()(const std::pair<transport_catalogue::catalog::Stop*, transport_catalogue::catalog::Stop*> st) const{
        size_t str1 = h1(st.first -> stop_name);
        size_t str2 = h2(st.second -> stop_name);
        return str1 + str2 * 42;
    }
    std::hash<std::string> h1;
    std::hash<std::string> h2;
};

}

class TransportCatalogue {
public:
    Stop* FindStop(const std::string& stop);
    void AddStop(std::string_view stop_name, const double lat, const double lng, 
        std::vector<std::pair<std::string, double>> dst_info);
    Bus* FindBus(std::string_view bus_name);
    void AddBus(const BusQuery& query);
    BusInfo GetBusInfo(const Bus& bus) const;
    std::set<std::string> GetStopInfo(std::string_view query);

private:
    std::deque<Stop> stops_;
    std::deque<Bus> routs_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<Stop*, Stop*>, double, detail::Hasher_stop> stop_stop_to_dist_;
};

}

