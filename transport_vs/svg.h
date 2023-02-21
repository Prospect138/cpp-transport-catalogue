#pragma once

#include <cstdint>
#include <iostream>
#include <ostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

struct Rgb {    
    Rgb() = default;
    Rgb(uint8_t a, uint8_t b, uint8_t c) :
        red(a), green(b), blue(c)
    {}
    uint8_t red;
    uint8_t green; 
    uint8_t blue;
};

struct Rgba {
    Rgba() = default;
    Rgba(uint8_t a, uint8_t b, uint8_t c, double d) :
        red(a), green(b), blue(c), opacity(d)
    {}
    uint8_t red;
    uint8_t green; 
    uint8_t blue;
    double opacity;
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

inline const Color NoneColor{"none"};

struct ColorPrinter;

std::ostream& operator << (std::ostream& out, Color color);

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE
};

std::ostream& operator << (std::ostream& output, StrokeLineCap cap);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND
};

std::ostream& operator << (std::ostream& output, StrokeLineJoin join);

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color fill_color){
        fill_color_ = fill_color;
        return AsOwner();
    }

    Owner& SetStrokeColor(Color stroke_color){
        stroke_color_ = stroke_color;
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width){
        width_ = width;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap cap){
        linecap_ = cap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin join){
        linejoin_ = join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const{
        using namespace std::literals;

        if (fill_color_){
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_){
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (width_){
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (linecap_){
            out << " stroke-linecap=\""sv << *linecap_ << "\""sv;;
        }
        if (linejoin_){
            out  << " stroke-linejoin=\""sv << *linejoin_ << "\""sv;;
        }
    }

private:
    Owner& AsOwner(){
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> linecap_;
    std::optional<StrokeLineJoin> linejoin_;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);


    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private: 
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;

};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final: public Object, public PathProps<Text> {
public:
    Text() = default;

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);
    

private: 
    void RenderObject(const RenderContext& context) const override;

    Point position_ = {0.0, 0.0};
    Point offset_ {0.0, 0.0};
    uint32_t size_ = 1;
    std::string weight_;
    std::string font_;
    std::string data_;
};

class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj object){
        auto obj = std::make_unique<Obj>(object);
        objects_.emplace_back(std::move(obj));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    
protected:
    std::vector<std::unique_ptr<Object>> objects_;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};


class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document

};

}  // namespace svg

namespace shapes{

class Star : public svg::Drawable{
public:
    Star(svg::Point center, double outer_radius, double inner_radius, int num_rays);

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point center_;
    double outer_radius_;
    double inner_radius_;
    int num_rays_;
};

class Triangle : public svg::Drawable{
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3);

    void Draw(svg::ObjectContainer& container) const override;

private: 
    svg::Point p1_;
    svg::Point p2_;
    svg::Point p3_;
};

class Snowman : public svg::Drawable{
public:
    Snowman(svg::Point head_centre, double radius);

    void Draw(svg::ObjectContainer& container) const override;

private: 
    svg::Point head_centre_;
    double radius_;
};

} //namespace shapes