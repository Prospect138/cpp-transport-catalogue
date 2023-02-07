#include "json_reader.h"

using namespace std::literals;

namespace transport_catalogue::json_reader {

// Base function call other methods;
void JsonReader::ProcessInput(std::istream& input){
    JsonReader::ReadRawJson(input);
    JsonReader::ParseJson();
}

// Write parsed info to out
void JsonReader::WriteInfo(std::ostream& out){
    json::Array output_array;
    for (json::Node element : request_to_output_){
        output_array.push_back(element);
    }
    json::PrintNode(output_array, out);
}


// Reads Raw Json and put it to obj
void JsonReader::ReadRawJson(std::istream& input){
    json::Document doc = json::Load(input);
    if (doc.GetRoot().IsMap()) {
        documents_.emplace_back(std::move(doc));
    }
}

// Prosecc existong jsons in object
void JsonReader::ParseJson(){
    for (auto& doc : documents_){
        json::Node raw_map = doc.GetRoot();
        // If input json is not a map, throw exception;
        if (!raw_map.IsMap()){
            throw json::ParsingError("Incorrect input data type");
        }
        // Adding info to catalog and collecting out requests;
        for(const auto& [key, value] : raw_map.AsMap()){
            if (key == "base_requests"){
                AddToCatalog(raw_map.AsMap().at(key));
            }
            else if (key == "render_settings"){
                ReadRenderSettings(raw_map.AsMap().at(key));
            }
            else if (key == "stat_requests"){
                CollectOutput(raw_map.AsMap().at(key));
            }
        }
    }
}

//Adds data from Base_Request part of input;
void JsonReader::AddToCatalog(json::Node node) {
    if (!node.IsArray()) {
        throw json::ParsingError("Incorrect input data type");
    }

    // Представляем ноду массивом
    const json::Array& arr = node.AsArray();

    // First iteratinon over node only for add stops
    for (auto& element : arr){
        //Looks on each element as on map
        const json::Dict& dict = element.AsMap();
        const auto type_i = dict.find("type"s);
        if (type_i == dict.end()) {
            continue;
        }
        // Query type is stop : 
        if (type_i->second == "Stop"s) {
            // This is a stop to push in catalog
            catalog::Stop stop;
            std::vector<std::pair<std::string, double>> dst_info;

            if (const auto name_i = dict.find("name"s); name_i != dict.end() && name_i->second.IsString()) {
                stop.stop_name = name_i->second.AsString();
            } 
            else {
                continue;
            }
 
            if (const auto lat_i = dict.find("latitude"s); lat_i != dict.end() && lat_i->second.IsDouble()) {
                stop.coordinates.lat = lat_i->second.AsDouble();
            } 
            else {
                continue;
            }
 
            if (const auto lng_i = dict.find("longitude"s); lng_i != dict.end() && lng_i->second.IsDouble()) {
                stop.coordinates.lng = lng_i->second.AsDouble();
            } 
            else {
                continue;
            }

            const auto dist_i = dict.find("road_distances"s);

            if (dist_i != dict.end() && !(dist_i->second.IsMap())) {
                continue;
            }// проверка, что это словарь. он необязательный для остановки.
            for (const auto& [other_name, other_dist] : dist_i->second.AsMap()) {
                if (!other_dist.IsInt()) {
                    continue; 
                }
                dst_info.push_back({other_name, static_cast<size_t>(other_dist.AsInt())});
            }

            transport_catalogue_.AddStop(stop.stop_name, stop.coordinates.lat, 
            stop.coordinates.lng, dst_info);
        }
    }

    //second iteration is about adding buses with existing stops
    for (auto& element : arr){
        //Looks on each element as on map
        const json::Dict& dict = element.AsMap();
        const auto type_i = dict.find("type"s);
        if (type_i == dict.end()) {
            continue;
        }
        // Query type is stop : 
        if (type_i->second == "Bus"s) {
            // Fomring struct to send on catalog's method
            catalog::BusQuery bus;

            if (const auto name_i = dict.find("name"s); name_i != dict.end() && name_i->second.IsString()) {
                bus.route_name = name_i->second.AsString();
            } 
            else {
                continue;
            }

            if (const auto route_i = dict.find("is_roundtrip"s); route_i != dict.end() && route_i->second.IsBool()) {
                bus.type = route_i->second.AsBool() ? catalog::RouteType::ROUND : catalog::RouteType::NOT_ROUND;
            } 
            else{
                continue;
            }

            const auto stops_i = dict.find("stops"s);
            if (stops_i != dict.end() && !(stops_i->second.IsArray())) {
                continue;
            }
            for (const auto& stop_name : stops_i->second.AsArray()) {
                if (!stop_name.IsString()) {
                    continue;
                }
                bus.stops_list.emplace_back(stop_name.AsString());
            }

            transport_catalogue_.AddBus(bus);

        } else {
            continue;
        }
    }
}

void JsonReader::CollectOutput(json::Node request){
    using namespace transport_catalogue;
    if (!request.IsArray()){
        throw json::ParsingError("Incorrect input data type");
    }
    const json::Array& arr = request.AsArray();

    // Iterating over request array
    for (auto& element : arr){
        if (!element.IsMap()) {
            throw json::ParsingError("One of request nodes is not a dictionary.");
        }
        const json::Dict& request_fields = element.AsMap();

        // Check id and type
        int id = -1;
        if (const auto id_i = request_fields.find("id"s); id_i != request_fields.end() && id_i->second.IsInt()) {
            id = id_i->second.AsInt();
        } 
        else{
            throw json::ParsingError("Invalid field in request' node");
        }

        const auto type_i = request_fields.find("type"s);
        if ( type_i == request_fields.end() || !(type_i->second.IsString()) ){
            throw json::ParsingError("Invalid field in request' node");
        }
        std::string type = type_i->second.AsString();

        //Parse map
        if ( type == "Map"s) {
            renderer::MapRenderer svg_map(GetParsedRenderSettings());

            std::ostringstream stream;
            svg_map.RenderSvgMap(transport_catalogue_, stream);

            json::Dict result;
            result.emplace("request_id"s, id);
            result.emplace("map"s, std::move(stream.str()));

            request_to_output_.push_back(json::Node(result));

            continue;
        }

        std::string name;
        if (const auto name_i = request_fields.find("name"s); name_i != request_fields.end() && name_i->second.IsString()) {
            name = name_i->second.AsString();
        } 
        else {
            throw json::ParsingError("Invalid field in request' node");
        }

        // Or bus
        if ( type == "Bus"s) {
            catalog::Bus* bus = transport_catalogue_.FindBus(name);
            if (!bus){
                request_to_output_.push_back(GetErrorNode(id));
                continue;
            }
            catalog::BusInfo info = transport_catalogue_.GetBusInfo(*bus);

            json::Dict result;
            result.emplace("request_id"s, id);
            result.emplace("curvature"s, info.curvature);
            result.emplace("route_length"s, static_cast<int>(info.length));
            result.emplace("stop_count"s, static_cast<int>(info.num_of_stops));
            result.emplace("unique_stop_count"s, static_cast<int>(info.uinque_stops));

            request_to_output_.push_back(json::Node(result));
        }

        // And even stop!
        else if (type == "Stop"s) {
            if (!(transport_catalogue_.FindStop(name)) ) {
                request_to_output_.push_back(GetErrorNode(id));
                continue;
            }
            json::Dict result;
            json::Array buses;
            const std::optional<std::set<std::string>>& bus_routes = transport_catalogue_.GetStopInfo(name);
            for (auto bus_route : *bus_routes) {
                buses.emplace_back(std::string{bus_route});
            }
            result.emplace("request_id"s, id);
            result.emplace("buses"s, buses);

            request_to_output_.push_back(json::Node(result));
        }
        else{
            throw json::ParsingError("Invalid stat request.");
        } 
    }
}

// Store parsed renderer settings as Node to costruct him later
void JsonReader::ReadRenderSettings(json::Node node) {

    if (!node.IsMap()) {
        throw json::ParsingError("Error reading JSON data with render settings.");
    }

    raw_render_settings_ = node.AsMap();

}

// Read parsed render setiings from readers field to costruct renderer
renderer::RendererSettings JsonReader::GetParsedRenderSettings(){

    renderer::RendererSettings settings;

    if (const auto width_i = raw_render_settings_.find("width"s); width_i != raw_render_settings_.end() && width_i->second.IsDouble()) {
        settings.width = width_i->second.AsDouble();
    } 
    else{
        throw json::ParsingError("Invaild width data.");
    }

    if (const auto height_i = raw_render_settings_.find("height"s); height_i != raw_render_settings_.end() && height_i->second.IsDouble()) {
        settings.height = height_i->second.AsDouble();
    } 
    else{
        throw json::ParsingError("Invaild height data.");
    }

    if (const auto padd_i = raw_render_settings_.find("padding"s); padd_i != raw_render_settings_.end() && padd_i->second.IsDouble()) {
        settings.padding = padd_i->second.AsDouble();
    } 
    else{
        throw json::ParsingError("Invaild padding data.");
    }

    if (const auto field_iter = raw_render_settings_.find("line_width"s); field_iter != raw_render_settings_.end() && field_iter->second.IsDouble()) {
        settings.line_width = field_iter->second.AsDouble();
    } 
    else{
        throw json::ParsingError("Invaild line width data.");
    }

    if (const auto field_iter = raw_render_settings_.find("stop_radius"s); field_iter != raw_render_settings_.end() && field_iter->second.IsDouble()) {
        settings.stop_radius = field_iter->second.AsDouble();
    } 
    else{
        throw json::ParsingError("Invaild stop radius data.");
    }

    if (const auto field_iter = raw_render_settings_.find("bus_label_font_size"s); field_iter != raw_render_settings_.end() && field_iter->second.IsInt()) {
        settings.bus_label_font_size = field_iter->second.AsInt();
    } 
    else{
        throw json::ParsingError("Invaild bus label font size data.");
    }

    if (const auto field_iter = raw_render_settings_.find("bus_label_offset"s); field_iter != raw_render_settings_.end() && field_iter->second.IsArray()) {
        json::Array arr = field_iter->second.AsArray();
        if (arr.size() != 2) throw json::ParsingError("Invaild bus label font offset data.");
        settings.bus_label_offset.x = arr[0].AsDouble();
        settings.bus_label_offset.y = arr[1].AsDouble();
    } 
    else{
        throw json::ParsingError("Invaild bus label font offset data.");
    }

    if (const auto field_iter = raw_render_settings_.find("stop_label_font_size"s); field_iter != raw_render_settings_.end() && field_iter->second.IsInt()) {
        settings.stop_label_font_size = field_iter->second.AsInt();
    } 
    else{
        throw json::ParsingError("Invaild stop label font size data.");
    }

    if (const auto field_iter = raw_render_settings_.find("stop_label_offset"s); field_iter != raw_render_settings_.end() && field_iter->second.IsArray()) {
        json::Array arr = field_iter->second.AsArray();
        if (arr.size() != 2) throw json::ParsingError("Invaild stop label font offset data.");
        settings.stop_label_offset.x = arr[0].AsDouble();
        settings.stop_label_offset.y = arr[1].AsDouble();
    } \
    else{
        throw json::ParsingError("Invaild stop label font offset data.");
    }

    if (const auto field_iter = raw_render_settings_.find("underlayer_color"s); field_iter != raw_render_settings_.end() ) {
        svg::Color color = ParseColor(field_iter->second);
        if (std::holds_alternative<std::monostate>(color)) {
            throw json::ParsingError("Invaild underlayer color data.");
        }

        settings.underlayer_color = color;
    } 
    else{
        throw json::ParsingError("Invaild underlayer color data.");
    }

    if (const auto field_iter = raw_render_settings_.find("underlayer_width"s); field_iter != raw_render_settings_.end() && field_iter->second.IsDouble()) {
        settings.underlayer_width = field_iter->second.AsDouble();
    } 
    else{
        throw json::ParsingError("Invaild underlayer width data.");
    }

    if (const auto field_iter = raw_render_settings_.find("color_palette"s); field_iter != raw_render_settings_.end() && field_iter->second.IsArray()) {
        json::Array arr = field_iter->second.AsArray();
        for (const auto& color_node : arr) {
            svg::Color color = ParseColor(color_node);
            if (std::holds_alternative<std::monostate>(color)) {
                throw json::ParsingError("Invaild color palette data.");
            }
            settings.color_palette.emplace_back(color);
        }
    }
    else{
        throw json::ParsingError("Invaild color palette data.");
    }

    return settings;
}

inline json::Node GetErrorNode(int id) {
    using namespace std::literals;
    json::Dict result;
    result.emplace("request_id"s, id);
    result.emplace("error_message"s, "not found"s);

    return {result};
}

svg::Color ParseColor(const json::Node& node){
    if (node.IsString()) {
        return {node.AsString()};
    }

    if (node.IsArray()) {
        json::Array arr = node.AsArray();
        if (arr.size() == 3) {
            uint8_t red = arr[0].AsInt();
            uint8_t green = arr[1].AsInt();
            uint8_t blue = arr[2].AsInt();

            return { svg::Rgb(red, green, blue) };
        }
        if (arr.size() == 4) {
            uint8_t red = arr[0].AsInt();
            uint8_t green = arr[1].AsInt();
            uint8_t blue = arr[2].AsInt();
            double opacity = arr[3].AsDouble();

            return { svg::Rgba(red, green, blue, opacity) };
        }
    }

    return {};
}

}

