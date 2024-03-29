#include "stat_reader.h"

#include <sstream>
#include <iomanip>
#include <optional>

namespace transport_catalogue::output {

using namespace transport_catalogue;

void ReadOutput(std::string_view query, catalog::TransportCatalogue& catalog){
    std::string query_type = std::string(query.substr(0, query.find_first_of(' ')));
    query = query.substr(query.find(' ')+1);
    if (query_type == "Bus"){
        catalog::Bus* the_bus = catalog.FindBus(query);
        if (!the_bus){
            std::cout<<"Bus "<<query<<": not found"<<std::endl;
            return;
        }
        catalog::BusInfo that_bus = catalog.GetBusInfo(*the_bus);
        std::cout<<std::setprecision(6)<<"Bus "<<that_bus.rout_name<<": "<<that_bus.num_of_stops<<" stops on route, "<<
        that_bus.uinque_stops<<" unique stops, "<<that_bus.length<<" route length, "<<
        that_bus.curvature<<" curvature"<<std::endl;
    }
    else if (query_type == "Stop"){
        std::optional<std::set<std::string>> buses_at_stop = catalog.GetStopInfo(query);
        std::cout<<"Stop "<<query<<": ";

        if (!buses_at_stop){
            std::cout<<"not found";
        }

        else if (buses_at_stop){
            std::cout<<"buses ";
            auto it = buses_at_stop -> begin();
            auto next = it;

            while (true){
                ++next;
                std::cout<<*it;
                if (next == buses_at_stop-> end()){
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

}