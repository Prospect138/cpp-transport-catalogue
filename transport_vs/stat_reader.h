#pragma once 
#include "transport_catalogue.h"

#include <iostream>

namespace transport_catalogue::output{

using namespace transport_catalogue;

void ReadOutput(std::string_view query, catalog::TransportCatalogue& catalog);

}