#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>

using namespace std::literals;
using namespace transport_catalogue;

int main() {
    catalog::TransportCatalogue catalog;

    std::string line;
    std::vector<std::string> input_query;
    std::vector<std::string> output_query;
    //std::ifstream infile("tsC_case1_input.txt");

    std::string count_fillstr;
    getline(std::cin, count_fillstr);
    int count_fill = stoi(count_fillstr);

    for (int i = 0; i < count_fill; ++i){
        getline(std::cin, line);
        input_query.push_back(std::move(line));
    }
    std::sort(input_query.begin(), input_query.end(), std::greater<std::string>());
    for (std::string query : input_query){
        input::ReadInput(query, catalog);
    }
    
    std::string count_requeststr;
    getline(std::cin, count_requeststr);
    int count_request = stoi(count_requeststr);

    for (int i = 0; i < count_request; ++i){
        getline(std::cin, line);
        output::ReadOutput(line, catalog);
    }
    return 0;
}