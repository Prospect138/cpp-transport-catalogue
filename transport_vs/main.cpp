
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "serialization.h"
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
    json_reader::JsonReader reader(catalog, router);



    //std::cout << "Check1\n";
    if (mode == "make_base"sv) {
        //std::ifstream file("s14_3_opentest_3_make_base.json");
        reader.MakeBase(std::cin);
        //задаим настройки сериализатора из тех, что были запарсены
        serializator.SetSetting(reader.GetSerializatorSettings());
        serializator.SetRendererSettings(reader.GetParsedRenderSettings());
        serializator.SetRouterSettings(reader.GetRoutingSettings());
        serializator.Serialize();

    } else if (mode == "process_requests"sv) {
        //std::ifstream file2("s14_3_opentest_3_process_requests.json");
        reader.ProcessRequest(std::cin);

        serializator.SetSetting(reader.GetSerializatorSettings());
        serializator.Deserialize();
        reader.SetRendererSettings(serializator.GetRenderSettings());

        reader.ParseStatRequest();

        reader.WriteInfo(std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}