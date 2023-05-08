#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap)
        {
        case StrokeLineCap::BUTT:
        {
            out << "butt"sv;
            break;
        }
        case StrokeLineCap::ROUND:
        {
            out << "round"sv;
            break;
        }
        case StrokeLineCap::SQUARE:
        {
            out << "square"sv;
            break;
        }
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join)
        {
        case StrokeLineJoin::ARCS:
        {
            out << "arcs"sv;
            break;
        }
        case StrokeLineJoin::BEVEL:
        {
            out << "bevel"sv;
            break;
        }
        case StrokeLineJoin::MITER:
        {
            out << "miter"sv;
            break;
        }
        case StrokeLineJoin::MITER_CLIP:
        {
            out << "miter-clip"sv;
            break;
        }
        case StrokeLineJoin::ROUND:
        {
            out << "round"sv;
            break;
        }
        default:
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); i++)
        {
            if (i != 0) {
                out << ' ';
            }
            out << points_[i].x << ',' << points_[i].y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos)
    {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        data_ = ShieldData(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y
            << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
            << "\" font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty())
        {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty())
        {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << data_ << "</text>"sv;
    }

    std::string Text::ShieldData(std::string data) const
    {
        std::string shielded;
        for (const auto& c : data) {
            if (!shielded_symbols.count(c))
            {
                shielded.push_back(c);
            }
            else
            {
                shielded.append(shielded_symbols.at(c));
            }
        }
        return shielded;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (const auto& obj_up : objects_) {
            obj_up->Render(ctx);
        }
        out << "</svg>"sv;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color)
    {
        std::visit(OstreamColorPrinter{ out }, color);
        return out;
    }

    void OstreamColorPrinter::operator()(std::monostate)
    {
        out << "none"sv;
    }

    void OstreamColorPrinter::operator()(std::string str)
    {
        out << str;
    }

    void OstreamColorPrinter::operator()(Rgb rgb)
    {
        out << "rgb("sv << (unsigned int)rgb.red << ',' << (unsigned int)rgb.green << ',' << (unsigned int)rgb.blue << ')';
    }

    void OstreamColorPrinter::operator()(Rgba rgba)
    {
        out << "rgba("sv << (unsigned int)rgba.red << ',' << (unsigned int)rgba.green << ',' << (unsigned int)rgba.blue << ',' << rgba.opacity << ')';
    }

    Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r), green(g), blue(b) {}

    Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
        : red(r), green(g), blue(b), opacity(a) {}

}  // namespace svg
