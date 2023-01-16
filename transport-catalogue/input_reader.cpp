#include "input_reader.h"

#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace transport_catalogue;

void input::ParseStop(std::string& query, catalog::TransportCatalogue& catalog){
    std::string stop_name = query.substr(0, query.find(':'));
    std::vector<std::pair<std::string, double>> dists;

    query = query.substr(query.find(':')+1);

    auto not_space = query.find_first_not_of(' ');
    auto comma = query.find(',');
    auto not_space2 = query.find_first_not_of(' ', comma + 1);
    auto comma2 = query.find(',', not_space2);

    double lat = std::stod(query.substr(not_space, comma - not_space));

    double lng = std::stod(query.substr(not_space2));

    std::string query2 = query.substr(not_space2);

    if (query2.find('m')){
        std::string raw_dists = query.substr(comma2 + 1);
        while (true){
            std::string to_push = raw_dists.substr(0, raw_dists.find(','));
            double ds = std::stod(to_push.substr(0, to_push.find("m")));
            std::string st = to_push.substr(to_push.find("to") + 3);
            dists.push_back({st, ds});
            raw_dists = raw_dists.substr(raw_dists.find(',') +1);
            if (to_push == raw_dists){
                break;
            }
        }
    }

    catalog.AddStop(stop_name, lat, lng, dists);
}

void input::ParseBus(std::string& query, catalog::TransportCatalogue& catalog){
    
    catalog::BusQuery cnt;
    cnt.rt = query.substr(0, query.find(':'));
    query = query.substr(query.find(':')+1);
    query = query.substr(query.find_first_not_of(' '));
    char a = ' ';
    if (std::count(query.begin(), query.end(), '-')){
        a = '-';
        cnt.type = "not rounded";
    }
    else if (std::count(query.begin(), query.end(), '>')){
        a = '>';
        cnt.type = "rounded";
    }
    while (true){
        std::string to_push = query.substr(0, query.find(a)-1);

        if (to_push == query){
            cnt.content.push_back(std::move(to_push));
            break;
        }
        cnt.content.push_back(std::move(to_push));
        query = query.substr(query.find(a));
        query = query.substr(2); 
    }
    catalog.AddBus(cnt);
}


void input::ReadInput(std::string& query, catalog::TransportCatalogue& catalog){
    std::string query_type = query.substr(0, query.find_first_of(' '));
    query = query.substr(query.find(' ')+1);
    if (query_type == "Stop"){
        ParseStop(query, catalog);
    }
    else if (query_type == "Bus"){
        ParseBus(query, catalog);
    }
}