#pragma once

#include "transport_catalogue.h"

#include <string>

namespace transport_catalogue::input {

void ParseStop(std::string& query, catalog::TransportCatalogue& catalog);
void ParseBus(std::string& query, catalog::TransportCatalogue& catalog);
void ReadInput(std::string& query, catalog::TransportCatalogue& catalog);

}