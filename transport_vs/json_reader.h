#pragma once

#include <vector>
#include <sstream>
#include <optional>

#include "json.h"
#include "json_builder.h"
#include "domain.h"
#include "transport_router.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport_catalogue::json_reader{

inline json::Node GetErrorNode(int id);

svg::Color ParseColor(const json::Node& node);

class JsonReader{
public:

    JsonReader(catalog::TransportCatalogue& catalog, transport_router::TransportRouter& router) :
        transport_catalogue_(catalog),  router_(router){};

    //JsonReader(catalog::TransportCatalogue& catalog) :
    //    transport_catalogue_(catalog) {
    //};

    void ProcessInput(std::istream& input);
    void ReadRawJson(std::istream& input);
    void ParseJson();
    void AddStop(const json::Array& arr);
    void AddBus(const json::Array& arr);
    void AddToCatalog(json::Node node);
    void CollectOutput(json::Node node);
    void CollectMap(int id);
    void CollectStop(const std::string& name, int id);
    void CollectBus(catalog::Bus* bus, int id);
    void CollectRout(int id, const json::Dict& request_fields);
    void AddRoutingSettings(const json::Node &root_);
    void ReadRenderSettings(json::Node node);
    void WriteInfo(std::ostream& out);
    //добавить метод для работы с routing_settings
    renderer::RendererSettings GetParsedRenderSettings();

private:
    catalog::TransportCatalogue& transport_catalogue_;
    transport_router::TransportRouter& router_;
    std::vector<json::Document> documents_;
    std::vector<json::Node> request_to_output_;
    json::Dict raw_render_settings_;
    
    int bus_wait_time_;
    double bus_velocity_;
    
};

} //namespace transport_catalog::catalog