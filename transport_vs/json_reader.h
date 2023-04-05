#pragma once

#include <vector>
#include <sstream>
#include <optional>
#include <filesystem>

#include "json_builder.h"
#include "domain.h"
#include "transport_router.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "serialization.h"

namespace transport_catalogue::json_reader{

inline json::Node GetErrorNode(int id);

svg::Color ParseColor(const json::Node& node);

class JsonReader{
public:

    JsonReader(catalog::TransportCatalogue& catalog, transport_router::TransportRouter& router) :
        transport_catalogue_(catalog),  router_(router){};

    void MakeBase(std::istream& input);
    void ReadRawJson(std::istream& input, std::vector<json::Document>& document);
    void ParseBase();

    void AddStop(const json::Array& arr);
    void AddBus(const json::Array& arr);
    void AddToCatalog(json::Node node);

    void AddRoutingSettings(const json::Node &root_);
    void ReadRenderSettings(json::Node node);

    //void Serialize();

    void ProcessRequest(std::istream& input);
    void ParseSerializeSettings();
    void ParseStatRequest();
    void CollectMap(int id);


    void CollectOutput(json::Node node);

    void CollectStop(const std::string& name, int id);
    void CollectBus(catalog::Bus* bus, int id);
    void CollectRout(int id, const json::Dict& request_fields);

    void ReadSerializationSettings(const json::Node &node);
    void WriteInfo(std::ostream& out);
    //добавить метод для работы с routing_settings
    renderer::RendererSettings GetParsedRenderSettings();
    void SetRendererSettings(const renderer::RendererSettings& settings) ;
    serializator::SerializatorSettings GetSerializatorSettings();
    transport_router::RoutingSettings GetRoutingSettings();


    //void Deserialize();

private:
    catalog::TransportCatalogue& transport_catalogue_;
    transport_router::TransportRouter& router_;
    //serializator::Serializator& serializator_;

    
    std::vector<json::Document> base_document_;
    std::vector<json::Document> request_document_;

    std::vector<json::Node> request_to_output_;

    json::Dict raw_render_settings_;
    renderer::RendererSettings render_settings_;
    serializator::SerializatorSettings serializator_settings_;

    int bus_wait_time_;
    double bus_velocity_;
    
};

} //namespace transport_catalog::catalog