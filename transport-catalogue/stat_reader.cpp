#include "stat_reader.h"

#include <sstream>
#include <iomanip>

using namespace transport_catalogue;

//Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length
void output::ReadOutput(std::string_view query, catalog::TransportCatalogue& catalog){
    std::string query_type = std::string(query.substr(0, query.find_first_of(' ')));
    query = query.substr(query.find(' ')+1);
    if (query_type == "Bus"){
        catalog::Bus the_bus = *catalog.FindBus(query);
        if (the_bus.type == "INVALID"){
            std::cout<<"Bus "<<the_bus.rout_name<<": not found"<<std::endl;
            return;
        }
        catalog::BusInfo that_bus = catalog.GetBusInfo(the_bus);
        std::cout<<std::setprecision(6)<<"Bus "<<that_bus.rout_name<<": "<<that_bus.num_of_stops<<" stops on route, "<<
        that_bus.uinque_stops<<" unique stops, "<<that_bus.length<<" route length, "<<
        that_bus.curvature<<" curvature"<<std::endl;
    }
    else if (query_type == "Stop"){
        std::set<std::string> buses_at_stop = catalog.GetStopInfo(query);
        std::cout<<"Stop "<<query<<": ";

        if (buses_at_stop.count("UNIQUE NULL SIGNAL 666!")){
            std::cout<<"not found";
        }

        else if (!buses_at_stop.empty()){
            std::cout<<"buses ";
            auto it = buses_at_stop.begin();
            auto next = it;

            while (true){
                ++next;
                std::cout<<*it;
                if (next == buses_at_stop.end()){
                    break;
                }
                std::cout<<" ";
                ++it;
            }
        }

        else {
            std::cout<<"no buses";
        }

        std::cout<<std::endl;
    }
}


