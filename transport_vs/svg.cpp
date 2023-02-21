#define _USE_MATH_DEFINES

#include "svg.h"

#include <cmath>

namespace svg {

using namespace std::literals;

struct ColorPrinter {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << "none";
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(svg::Rgb regrbl) const {
        out << "rgb(" << static_cast<int>(regrbl.red) << "," << static_cast<int>(regrbl.green) << "," << static_cast<int>(regrbl.blue) << ")";
    }
    void operator()(svg::Rgba rgbopt) const {
        out << "rgba(" << static_cast<int>(rgbopt.red) << "," << static_cast<int>(rgbopt.green) << "," << static_cast<int>(rgbopt.blue) << "," << rgbopt.opacity << ")";
    }
};

std::ostream& operator << (std::ostream& out, Color color){
    std::visit(ColorPrinter{out}, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

std::ostream& operator << (std::ostream& output, StrokeLineCap cap){
    using namespace std::literals;
    switch (cap)
    {
        case StrokeLineCap::BUTT : 
            output << "butt"sv;
            break;
        case StrokeLineCap::ROUND : 
            output << "round"sv;
            break;
        case StrokeLineCap::SQUARE : 
            output << "square"sv;
            break;
    }
    return output;
}

std::ostream& operator << (std::ostream& output, StrokeLineJoin join){
    using namespace std::literals;
    switch (join)
    {
        case StrokeLineJoin::ARCS : 
            output << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL : 
            output << "bevel"sv;
            break;
        case StrokeLineJoin::MITER : 
            output << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP : 
            output << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND :
            output << "round"sv;
            break;
    }
    return output;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext &context) const {
    auto& out = context.out;
    out<<"<polyline points=\"";
    for(int i = 0; i < static_cast<int>(points_.size()); ++i){
        out<<points_[i].x<<","<<points_[i].y;
        if (i != static_cast<int>(points_.size() - 1)){
            out<<" ";
        }
    }

    out <<"\"";
    
    RenderAttrs(context.out);

    out <<"/>";


} 

Text& Text::SetPosition(Point pos){
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data){
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";

    RenderAttrs(context.out);
    
    out << " x=\""<<position_.x<<"\" y=\""<<position_.y<<"\"";
    out << " dx=\""<<offset_.x<<"\" dy=\""<<offset_.y<<"\"";
    out << " font-size=\""<<size_<<"\"";

    if (!weight_.empty()){
        out << " font-weight=\""<<weight_<<"\"";
    }
    if (!font_.empty()){
        out << " font-family=\""<<font_<<"\"";
    }

    out <<">"<<data_<<"</text>";
}

// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.emplace_back(std::move(obj)); //??
}
// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for (auto& obj_ptr : objects_){
        auto& temp = *obj_ptr;
        RenderContext ctx(out, 2, 2);
        temp.Render(ctx);
    }

    out << "</svg>"sv << std::endl;
}

}  // namespace svg

namespace shapes {

    Star::Star(svg::Point center, double outer_radius, double inner_radius, int num_rays) {
        center_ = center;
        outer_radius_ = outer_radius;
        inner_radius_ = inner_radius;
        num_rays_ = num_rays;
    }

    void Star::Draw(svg::ObjectContainer& container) const{
        svg::Polyline polyline;
        for (int i = 0; i <= num_rays_; ++i) {
            double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
            polyline.AddPoint({center_.x + outer_radius_ * sin(angle), center_.y - outer_radius_ * cos(angle)});
            if (i == num_rays_) {
                break;
            }
        angle += M_PI / num_rays_;
        polyline.AddPoint({center_.x + inner_radius_ * sin(angle), center_.y - inner_radius_ * cos(angle)});
        }
        polyline.SetFillColor("red").SetStrokeColor("black");
        container.Add(polyline);
    }

    Triangle::Triangle(svg::Point p1, svg::Point p2, svg::Point p3){
        p1_ = p1;
        p2_ = p2;
        p3_ = p3;
    }

    void Triangle::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

    Snowman::Snowman(svg::Point head_centre, double radius){
        head_centre_ = head_centre;
        radius_ = radius;
    }

    void Snowman::Draw(svg::ObjectContainer& container) const{
        svg::Point rear = {head_centre_.x, (head_centre_.y + radius_ * 5)};
        svg::Point middle = {head_centre_.x, (head_centre_.y + radius_ * 2)};
        container.Add(svg::Circle().SetCenter(rear).SetRadius(2 * radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        container.Add(svg::Circle().SetCenter(middle).SetRadius(1.5 * radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        container.Add(svg::Circle().SetCenter(head_centre_).SetRadius(radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
    }
}