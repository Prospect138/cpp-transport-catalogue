#pragma once

#include "geo.h"
#include "domain.h"

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <set>
#include <optional> 

namespace transport_catalogue::catalog{

struct BusQuery{
    std::string route_name;
    RouteType type;
    std::vector<std::string> stops_list;
};

namespace detail {
    
    struct StopsHasher{
        size_t operator()(const std::pair<const transport_catalogue::catalog::Stop*,const transport_catalogue::catalog::Stop*> st) const{
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
    Stop* FindStop(const std::string_view stop);
    void AddStop(std::string_view stop_name, const double lat, const double lng, 
        const std::vector<std::pair<std::string, double>>& dst_info);
    Bus* FindBus(std::string_view bus_name);
    void AddBus(const BusQuery& query);
    BusInfo GetBusInfo(const Bus& bus) const;
    std::optional<std::set<std::string>> GetStopInfo(std::string_view query);
    const std::map<std::string_view, const Bus*> GetBuses() const;
    const std::map<std::string_view, const Stop*> GetStops() const;
    const std::set<std::string_view>& GetBusesForStop(std::string_view stop) const;
    double GetCalculateDistance(const Stop* first_route, const Stop* second_route);
    Coordinates GetCoordinatesByStop(std::string_view stop_name) const;
    const std::deque<Stop>& GetAllStops() const;
    const std::deque<Bus>& GetAllBus() const;
    const std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::StopsHasher>& 
        GetDistances() const;
    
    void AddDistance(const std::string& stop1, const std::string& stop2, double distance);

    void DirectAddBus(std::string bus_name, catalog::RouteType type, std::vector<std::string> stop_names);


private:

    std::deque<Stop> stops_;
    std::deque<Bus> routs_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::StopsHasher> stop_stop_to_dist_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_and_buses_;

    int stop_ids_ = 0;
};

}

