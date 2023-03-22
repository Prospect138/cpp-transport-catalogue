#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>

const std::set<std::string_view> EMPTY_BUS_ROUTE_SET{};

namespace transport_catalogue::catalog {

void TransportCatalogue::AddStop(std::string_view stop_name, const double lat, const double lng,
const std::vector<std::pair<std::string, double>>& dst_info){

    if (!stopname_to_stop_.count(stop_name)){
        Stop the_stop{std::string{stop_name}, lat, lng, stop_ids_}; 
        stops_.push_back(std::move(the_stop));
        stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
        ++stop_ids_;
    }
    else {
        stopname_to_stop_[stop_name] -> coordinates.lat = lat;
        stopname_to_stop_[stop_name] -> coordinates.lng = lng;
    }
    Stop* st1 = stopname_to_stop_[stop_name];
    for (auto& [key, value] : dst_info){

        if (stopname_to_stop_.count(key)){
            Stop* st2;
            st2 = stopname_to_stop_[key];
            stop_stop_to_dist_[{st1, st2}] = value;
        }
        else {
            Stop st2;
            st2.id = stop_ids_;

            ++ stop_ids_;

            st2.stop_name = key;
            stops_.push_back(std::move(st2));
            stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
            stop_stop_to_dist_[{st1, &stops_.back()}] = value;
        }
    }

}

// Find pointer to stop struct with string 
Stop* TransportCatalogue::FindStop(const std::string_view stop) {
    if (stopname_to_stop_.count(stop)){
        return stopname_to_stop_.at(stop);
    }
    else {
        return nullptr;
    }
}

void TransportCatalogue::AddBus(const BusQuery& query){
    Bus bus;
    bus.type = query.type;
    bus.rout_name = query.route_name;
    for (const std::string& st : query.stops_list){
        Stop* that_stop = FindStop(st);
        bus.rout.push_back(that_stop);
    }
    if (bus.type == RouteType::NOT_ROUND){
        for (int i = static_cast<int>(bus.rout.size()-2); i >= 0; --i){
            bus.rout.push_back(bus.rout[i]);
        }
    }
    routs_.push_back(std::move(bus));
    busname_to_bus_[routs_.back().rout_name] = &routs_.back();

    for (const Stop* stop : busname_to_bus_[routs_.back().rout_name]->rout) {
        stop_and_buses_[stop->stop_name].insert(routs_.back().rout_name);
    }
}

// Find bus struct by its name
Bus* TransportCatalogue::FindBus(std::string_view bus_name){
    if (busname_to_bus_.count(bus_name)){
        return busname_to_bus_.at(bus_name);
    }
    else {
        return nullptr;
    }
}

// Returns info about bus with stops, length etc
//
BusInfo TransportCatalogue::GetBusInfo(const Bus& bus) const{

    std::set<std::string> buffer_names;
    int n_of_stops = bus.rout.size();
    double length2 = 0;
    double length = 0;
    for (size_t i = 1; i < bus.rout.size(); ++i){
        Stop* st1 = bus.rout[i-1];
        Stop* st2 = bus.rout[i];
        buffer_names.insert(st1 ->stop_name);
        if (stop_stop_to_dist_.count({st1, st2})){
            length += stop_stop_to_dist_.at({st1, st2});
        }
        else {
            length += stop_stop_to_dist_.at({st2, st1});
        }
        length2 += ComputeDistance(bus.rout[i-1] -> coordinates, bus.rout[i] -> coordinates);
    }
    double curvature = length / length2;
    int u_stops = buffer_names.size();
    BusInfo bus_info{bus.rout_name, n_of_stops, u_stops, length, curvature};
    return bus_info;
}

//Additional method for getting all routes that cross the stop
const std::set<std::string_view>& TransportCatalogue::GetBusesForStop(std::string_view stop) const {
    const auto iter = stop_and_buses_.find(stop);

    if (iter == stop_and_buses_.end()) {
        return EMPTY_BUS_ROUTE_SET;
    }

    return iter->second;
}

// Returns info about Stop and which routs do cross it
// Returns nullopt if its no existing buses
std::optional<std::set<std::string>> TransportCatalogue::GetStopInfo(std::string_view query) {

    std::set<std::string> buses_at_stop;

    if (!stopname_to_stop_.count(query)){
        return std::nullopt;
    }

    for (const Bus& that_bus : routs_){
        for (Stop* stop : that_bus.rout){
            if (stop->stop_name == query){
                buses_at_stop.emplace(that_bus.rout_name);
            }
        }
    }
    
    return std::optional<std::set<std::string>>{buses_at_stop};
}

// getter
const std::map<std::string_view, const Bus*> TransportCatalogue::GetBuses() const {
    std::map<std::string_view, const Bus*>  result(busname_to_bus_.begin(), busname_to_bus_.end());
    return result;
}

// another getter
const std::map<std::string_view, const Stop*> TransportCatalogue::GetStops() const {
    std::map<std::string_view, const Stop*> result(stopname_to_stop_.begin(), stopname_to_stop_.end());
    return result;
}

int TransportCatalogue::GetCalculateDistance(const Stop* first_route,
                                             const Stop* second_route) {

    if (stop_stop_to_dist_.count({first_route, second_route}) != 0) {
        return static_cast<int>(stop_stop_to_dist_[{first_route, second_route}]);
    }
    else return 0;
}

Coordinates TransportCatalogue::GetCoordinatesByStop(std::string_view stop_name) const {
    Coordinates result{};

    auto search = stopname_to_stop_.find(stop_name);
    if (search != stopname_to_stop_.end()) {
        result = search->second->coordinates;
        return result;
    }
    return result;
}

const std::deque<Stop>& TransportCatalogue::GetAllStops() const {
    return stops_;
}

const std::deque<Bus>& TransportCatalogue::GetAllBus() const{
    return routs_;
}
} //namespace transport_catalogue::catalog