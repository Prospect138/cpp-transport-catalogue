
#include <iostream>
#include <string_view>

#include "json_reader.h"

using namespace std::literals;

using namespace transport_catalogue;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    catalog::TransportCatalogue catalog;
    transport_router::TransportRouter router;
    serializator::Serializator serializator(catalog, router);
    json_reader::JsonReader reader(catalog, router, serializator);



    //std::cout << "Check1\n";
    if (mode == "make_base"sv) {
        //std::ifstream file("s14_3_opentest_3_make_base.json");
        //здесь надо прочитать json, заполнить базу, затем сериализовать базу в файл
        //if (file){
        //    std::cout << "File is ok\n";
        //}
        reader.MakeBase(std::cin);

    } else if (mode == "process_requests"sv) {
        //std::ifstream file2("s14_3_opentest_3_process_requests.json");
        reader.ProcessRequest(std::cin);
        reader.WriteInfo(std::cout);
        //std::cout << "WIP\n";
        // а здесь прочитать сериализованную базу из файла
        // на основе сериализованного файла построить вывод
        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}