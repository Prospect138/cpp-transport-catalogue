#pragma once

#include <vector>
#include <sstream>

#include "json.h"
#include "json_builder.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"


namespace transport_catalogue::json_reader{

inline json::Node GetErrorNode(int id);

svg::Color ParseColor(const json::Node& node);

class JsonReader{
public:

    JsonReader(catalog::TransportCatalogue& catalog) :
        transport_catalogue_(catalog) {};

    void ReadRawJson(std::istream& input);

    void ParseJson();

    void WriteInfo(std::ostream& out);

    void AddStop(const json::Array& arr);

    void AddBus(const json::Array& arr);

    void AddToCatalog(json::Node node);

    void CollectMap(int id);

    void CollectStop(const std::string& name, int id);

    void CollectBus(catalog::Bus* bus, int id);

    void ReadRenderSettings(json::Node node);
    
    void CollectOutput(json::Node node);

    void ProcessInput(std::istream& input);

    renderer::RendererSettings GetParsedRenderSettings();

private:
    catalog::TransportCatalogue& transport_catalogue_;
    std::vector<json::Document> documents_;
    std::vector<json::Node> request_to_output_;
    json::Dict raw_render_settings_;

};

}