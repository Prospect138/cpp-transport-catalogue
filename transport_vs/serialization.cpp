#include "serialization.h"

using namespace std::string_literals;

namespace serializator {

using namespace transport_catalogue;

void Serializator::SetSetting(SerializatorSettings settings){
    serialization_settings_ = settings;
}

void Serializator::SetRendererSettings(const renderer::RendererSettings &settings)
{
    r_settings_ = settings;
}

void Serializator::SetRouterSettings(const transport_router::RoutingSettings& settings){
    routing_settings_ = settings;
    //router_.settings_.bus_velocity_ = settings.bus_velocity_;
    //router_.settings_.bus_wait_time_ = settings.bus_wait_time_;
}

void Serializator::Serialize() {
    std::ofstream out_file(serialization_settings_.path, std::ios::binary);

    // подготовка к записи
    WriteStops();
    WriteBuses();
    WriteDistances();
    WriteMap();
    WriteRoutingSettings();

    // пишем в файл
    pr_catalogue_.SerializeToOstream(&out_file);
}

void Serializator::Deserialize()
{
    std::ifstream in_file(serialization_settings_.path, std::ios::binary);

    pr_catalogue_.ParseFromIstream(&in_file);

    ReadStops();
    ReadBuses();
    ReadDistances();
    ReadMap();
    ReadRoutingSettings();
}

void Serializator::WriteStops() {
    const auto stops = catalog_.GetStops();
    std::vector<proto_catalog_namespace::Stop> serialized_stops_list(stops.size());
    *pr_catalogue_.mutable_stops() = {serialized_stops_list.begin(), serialized_stops_list.end() };
    pr_catalogue_.mutable_stops()->Reserve(stops.size());

    for (const auto& [stop_name, stop] : stops) {
        proto_catalog_namespace::Stop* serialized_stop = pr_catalogue_.mutable_stops(stop->id);
        *serialized_stop->mutable_name() = stop_name;
        serialized_stop->set_lat(stop->coordinates.lat);
        serialized_stop->set_lng(stop->coordinates.lng);
        serialized_stop->set_id(stop->id);
    }
}

// берем все автобусы из каталога и кладем в файл
void Serializator::WriteBuses() {
    for (auto& [bus_name, bus] : catalog_.GetBuses()) {
        proto_catalog_namespace::Bus* serialized_bus = pr_catalogue_.add_buses();
        *serialized_bus->mutable_name() = bus_name;
        // вот тут может быть баг
        if (bus->type == catalog::RouteType::ROUND){
            serialized_bus->set_is_roundtrip(true);
        }
        else {
            serialized_bus->set_is_roundtrip(false);
        }

        for (const auto& stop : bus->rout) {
            serialized_bus->add_index_stops(stop->id);
        }
    }
}

void Serializator::WriteDistances() {
    const auto distances = catalog_.GetDistances();
    for (const auto& [stops, distance] : distances) {
        proto_catalog_namespace::Distance* serialized_distance = pr_catalogue_.add_distances();
        serialized_distance->set_id_stop_first(stops.first->stop_name);
        serialized_distance->set_id_stop_second(stops.second->stop_name);
        serialized_distance->set_distance(distance);
    }

}

void Serializator::WriteMap()
{
    proto_catalog_namespace::RenderSettings* serialized_render_settings_ = pr_catalogue_.mutable_render_settings();

    serialized_render_settings_->set_width(r_settings_.width);
    serialized_render_settings_->set_height(r_settings_.height);
    serialized_render_settings_->set_padding(r_settings_.padding);
    serialized_render_settings_->set_line_width(r_settings_.line_width);
    serialized_render_settings_->set_stop_radius(r_settings_.stop_radius);
    serialized_render_settings_->set_bus_label_font_size(r_settings_.bus_label_font_size);
    serialized_render_settings_->set_bus_label_offset_x(r_settings_.bus_label_offset.x);
    serialized_render_settings_->set_bus_label_offset_y(r_settings_.bus_label_offset.y);
    serialized_render_settings_->set_stop_label_font_size(r_settings_.stop_label_font_size);
    serialized_render_settings_->set_stop_label_offset_x(r_settings_.stop_label_offset.x);
    serialized_render_settings_->set_stop_label_offset_y(r_settings_.stop_label_offset.y);
    *serialized_render_settings_->mutable_underlayer_color() = SerializeColor(r_settings_.underlayer_color);
    serialized_render_settings_->set_underlayer_width(r_settings_.underlayer_width);

    for (const svg::Color& color : r_settings_.color_palette) {
        *serialized_render_settings_->add_color_palette() = SerializeColor(color);
    }
}

void Serializator::WriteRoutingSettings()
{
    proto_catalog_namespace::RoutingSettings* serialized_routing_settings = pr_catalogue_.mutable_routing_settings();
    serialized_routing_settings->set_bus_wait_time(routing_settings_.bus_wait_time_);
    serialized_routing_settings->set_bus_velocity(routing_settings_.bus_velocity_);
}

void Serializator::ReadRoutingSettings(){
    //router_.settings_ = 
    router_.settings_.bus_wait_time_ = pr_catalogue_.routing_settings().bus_wait_time();
    router_.settings_.bus_velocity_ = pr_catalogue_.routing_settings().bus_velocity();
}

proto_catalog_namespace::Color Serializator::SerializeColor(const svg::Color &color)
{
    proto_catalog_namespace::Color serialized_color;
    if (std::holds_alternative<svg::Rgb>(color)) {
        proto_catalog_namespace::RGB* rgb = serialized_color.mutable_rgb();
        rgb->set_red(std::get<svg::Rgb>(color).red);
        rgb->set_green(std::get<svg::Rgb>(color).green);
        rgb->set_blue(std::get<svg::Rgb>(color).blue);
    }
    else if (std::holds_alternative<svg::Rgba>(color)) {
        proto_catalog_namespace::RGBA* rgba = serialized_color.mutable_rgba();
        rgba->set_red(std::get<svg::Rgba>(color).red);
        rgba->set_green(std::get<svg::Rgba>(color).green);
        rgba->set_blue(std::get<svg::Rgba>(color).blue);
        rgba->set_opacity(std::get<svg::Rgba>(color).opacity);
    }
    else if (std::holds_alternative<std::string>(color)) {
        serialized_color.set_name_color(std::get<std::string>(color));
    }
    else {
        serialized_color.set_name_color("monostate"s);
    }
    return serialized_color;
}

void Serializator::ReadStops()
{
    for (const auto& stop : pr_catalogue_.stops()) {
        std::vector<std::pair<std::string, double>> empty_vec;
        catalog_.AddStop(stop.name(), stop.lat(), stop.lng(), empty_vec);
    }
}

void Serializator::ReadBuses()
{
    for (const auto& bus : pr_catalogue_.buses()) {
        //std::cout << "rout " << bus.name() << "\n";
        std::vector<std::string> stops_names;
        for (const auto& id : bus.index_stops()) {
            std::string stop_name = pr_catalogue_.mutable_stops(id)->name();
            //std::cout << stop_name << std::endl;
            stops_names.push_back(move(stop_name));
        }
        catalog::RouteType type  = catalog::RouteType::ROUND ;
        if (bus.is_roundtrip()){
            type = catalog::RouteType::ROUND;
        }
        if (!bus.is_roundtrip()){
            type = catalog::RouteType::NOT_ROUND;
        }
        catalog_.DirectAddBus(bus.name(), type, stops_names);
    }
}

void Serializator::ReadDistances()
{
    for (const auto& distance : pr_catalogue_.distances()) {
        const std::string first_stop_name = distance.id_stop_first();
        const std::string second_stop_name = distance.id_stop_second();
        //std::cout<< first_stop_name << " " << second_stop_name << "\n";
        catalog_.AddDistance(first_stop_name, second_stop_name, distance.distance());
    }

}

void Serializator::ReadMap()
{
    proto_catalog_namespace::RenderSettings* serialized_render_settings = pr_catalogue_.mutable_render_settings();

    r_settings_.width = serialized_render_settings->width();
    r_settings_.height = serialized_render_settings->height();
    r_settings_.padding = serialized_render_settings->padding();
    r_settings_.line_width = serialized_render_settings->line_width();
    r_settings_.stop_radius = serialized_render_settings->stop_radius();
    r_settings_.bus_label_font_size = serialized_render_settings->bus_label_font_size();
    r_settings_.bus_label_offset = {serialized_render_settings->bus_label_offset_x(), serialized_render_settings->bus_label_offset_y() };
    r_settings_.stop_label_font_size = serialized_render_settings->stop_label_font_size();
    r_settings_.stop_label_offset = {serialized_render_settings->stop_label_offset_x(), serialized_render_settings->stop_label_offset_y() };
    r_settings_.underlayer_color = DeserializeColor(serialized_render_settings->underlayer_color());
    r_settings_.underlayer_width = serialized_render_settings->underlayer_width();

    for (proto_catalog_namespace::Color serialized_color : serialized_render_settings->color_palette()) {
        r_settings_.color_palette.push_back(DeserializeColor(serialized_color));
    }
}


svg::Color Serializator::DeserializeColor(const proto_catalog_namespace::Color &serialized_color)
{
    if (serialized_color.has_rgb()) {
        svg::Rgb rgb;
        rgb.red = serialized_color.rgb().red();
        rgb.green = serialized_color.rgb().green();
        rgb.blue = serialized_color.rgb().blue();
        return rgb;
    }
    else if (serialized_color.has_rgba()) {
        svg::Rgba rgba;
        rgba.red = serialized_color.rgba().red();
        rgba.green = serialized_color.rgba().green();
        rgba.blue = serialized_color.rgba().blue();
        rgba.opacity = serialized_color.rgba().opacity();
        return rgba;
    }
    else if (serialized_color.name_color() != "monostate"s) {
        return serialized_color.name_color();
    }
    return std::monostate();
}

renderer::RendererSettings Serializator::GetRenderSettings(){
    return r_settings_;
}

} // namespace serializator
