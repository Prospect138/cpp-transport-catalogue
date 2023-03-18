#include "json_reader.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>

using namespace std::literals;
using namespace transport_catalogue;

int main() {
    std::ifstream infile("e4_input.json");
    std::ofstream outfile("e4_out_test.json");
    // create a transport catalog
    catalog::TransportCatalogue catalog;
    // create a router
    transport_router::TransportRouter router;
    // map is not created, because its only constructs once on ProcessInput method with readers parameters
    // create a reader with previosly created catalog and router
    json_reader::JsonReader reader(catalog, router);
    reader.ProcessInput(infile);
    // but you still can construct map from main and initialize it with settings that reader stored 
    //renderer::MapRenderer rnd(reader.GetParsedRenderSettings());
    //rnd.RenderSvgMap(catalog, std::cout);
    reader.WriteInfo(outfile);
    return 0;
}