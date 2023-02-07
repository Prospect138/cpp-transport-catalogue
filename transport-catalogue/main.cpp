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
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами.
     * Вывести в stdout ответы в виде JSON
     */
    std::ifstream infile("json_in3.json");
    catalog::TransportCatalogue catalog;
    json_reader::JsonReader reader(catalog);
    reader.ProcessInput(infile);
    //renderer::MapRenderer rnd(reader.GetParsedRenderSettings());
    //rnd.RenderSvgMap(catalog, std::cout);
    reader.WriteInfo(std::cout);
    return 0;
}