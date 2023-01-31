#pragma once 
#include "transport_catalogue.h"

#include <iostream>

namespace transport_catalogue::output{

void ReadOutput(std::string& query, catalog::TransportCatalogue& catalog);

}