#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <transport_catalogue.pb.h>

using namespace transport_catalogue;

namespace serializator {

struct SerializatorSettings {
    std::filesystem::path path;
};

class Serializator {
    //Он должен хранить в себе все ссылки на каталог, мап_рендерер и роутер 
    //потом он делает из этой котлеты протофайлы, пользуясь методами сущностей
    //а когда надо, он эту котлету восстанавливает из серии и раздает данные обратно сущностям.
public:
    Serializator(catalog::TransportCatalogue& catalog, transport_router::TransportRouter& router) 
        : catalog_(catalog), router_(router) {}

    void SetSetting(SerializatorSettings settings);

    void SetRendererSettings(const renderer::RendererSettings& settings);

    void SetRouterSettings(const transport_router::RoutingSettings& settings);

    renderer::RendererSettings GetRenderSettings();

    void Serialize();

    void Deserialize();

private:
    void WriteStops();
    void WriteBuses();
    void WriteDistances();
    void WriteMap();
    void WriteRoutingSettings();
    proto_catalog_namespace::Color SerializeColor(const svg::Color& color);

    void ReadStops();
    void ReadBuses();
    void ReadDistances();
    void ReadMap();
    void ReadRoutingSettings();
    svg::Color DeserializeColor(const proto_catalog_namespace::Color &serialized_color);

    catalog::TransportCatalogue& catalog_;
    transport_router::TransportRouter& router_;
    SerializatorSettings serialization_settings_;
    renderer::RendererSettings r_settings_;
    catalog::RoutingSettings routing_settings_;

    proto_catalog_namespace::TransportCatalogue pr_catalogue_;
};
}
