
#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>

using namespace transport_catalogue::catalog;

void TransportCatalogue::AddStop(std::string stop_name, const double& lat, const double& lng,
std::vector<std::pair<std::string, double>> dst_info){
    if (!stopname_to_stop_.count(stop_name)){
        TheStop the_stop{stop_name, lat, lng}; 
        stops_.push_back(std::move(the_stop));
        stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
    }
    else {
        stopname_to_stop_[stop_name] -> coordinates.lat = lat;
        stopname_to_stop_[stop_name] -> coordinates.lng = lng;
    }
    TheStop* st1 = stopname_to_stop_[stop_name];
    for (auto [key, value] : dst_info){
        TheStop* st2;
        if (stopname_to_stop_.count(key)){
            st2 = stopname_to_stop_[key];
        }
        else {
            st2 = new TheStop;
            st2 -> stop_name = key;
            stops_.push_back(*st2);
            stopname_to_stop_[stops_.back().stop_name] = st2;
        }
        ss_dst_[{st1, st2}] = value;
    }

}

TheStop* TransportCatalogue::FindStop(const std::string& stop) {
    return stopname_to_stop_.at(stop);
}

void TransportCatalogue::AddBus(const BusQuery& query){
    Bus bus;
    bus.type = query.type;
    bus.rout_name = query.rt;
    for (std::string st : query.content){
        TheStop* that_stop = FindStop(st);
        bus.rout.push_back(that_stop);
    }
    if (bus.type == "not rounded"){
        for (int i = bus.rout.size()-2; i >= 0; --i){
            bus.rout.push_back(bus.rout[i]);
        }
    }
    routs_.push_back(std::move(bus));
    busname_to_bus_[routs_.back().rout_name] = &routs_.back();
}

Bus* TransportCatalogue::FindBus(const std::string bus_name){
    if (busname_to_bus_.count(bus_name)){
        return busname_to_bus_.at(bus_name);
    }
    else {
        Bus* not_a_bus;
        not_a_bus = new Bus;
        not_a_bus -> rout_name = bus_name;
        not_a_bus -> type = "INVALID";
        return not_a_bus;
    }
}

BusInfo TransportCatalogue::GetBusInfo(const Bus bus) const{

    std::set<std::string> buffer_names;
    int n_of_stops = bus.rout.size();
    double length2 = 0;
    double length = 0;
    for (int i = 1; i < bus.rout.size(); ++i){
        TheStop* st1 = bus.rout[i-1];
        TheStop* st2 = bus.rout[i];
        buffer_names.insert(st1 ->stop_name);
        if (ss_dst_.count({st1, st2})){
            length += ss_dst_.at({st1, st2});
        }
        else {
            length += ss_dst_.at({st2, st1});
        }
        length2 += ComputeDistance(bus.rout[i-1] -> coordinates, bus.rout[i] -> coordinates);
    }
    double curvature = length / length2;
    int u_stops = buffer_names.size();
    BusInfo bus_info{bus.rout_name, n_of_stops, u_stops, length, curvature};
    return bus_info;
}

std::set<std::string> TransportCatalogue::GetStopInfo(const std::string& query) {

    std::set<std::string> buses_at_stop;

    if (!stopname_to_stop_.count(query)){
        buses_at_stop.insert("UNIQUE NULL SIGNAL 666!");
        return buses_at_stop;
    }

    for (Bus that_bus : routs_){
        for (TheStop* stop : that_bus.rout){
            if (stop->stop_name == query){
                buses_at_stop.insert(that_bus.rout_name);
            }
        }
    }
    
    return buses_at_stop;
}
