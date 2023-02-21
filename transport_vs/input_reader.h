#pragma once

#include "transport_catalogue.h"

#include <string>

namespace transport_catalogue::input {

void ParseStop(std::string_view query, catalog::TransportCatalogue& catalog);
void ParseBus(std::string_view query, catalog::TransportCatalogue& catalog);
void ReadInput(std::string_view query, catalog::TransportCatalogue& catalog);

}