#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    bool IsCorrectSymbol(char c);

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Number = std::variant<int, double>;

    using Variant = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : public Variant {
    public:
        /* Реализуйте Node, используя std::variant */

        /*Node();
        Node(int value);
        Node(double value);
        Node(bool value);
        Node(std::string value);
        Node(std::nullptr_t value);
        Node(Array array);
        Node(Dict map);*/

        using Variant::variant;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        void Print(std::ostream& out = std::cout) const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

    private:

        // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
        struct PrintContext {
            std::ostream& out;
            int indent_step = 4;
            int indent = 0;

            void PrintIndent() const {
                for (int i = 0; i < indent; ++i) {
                    out.put(' ');
                }
            }

            // Возвращает новый контекст вывода с увеличенным смещением
            PrintContext Indented() const {
                return { out, indent_step, indent_step + indent };
            }
        };

        struct NodePrinter {
            PrintContext context;
            void operator()(int value) const;
            void operator()(double value) const;
            void operator()(bool value) const;
            void operator()(std::string value) const;
            void operator()(std::nullptr_t value) const;
            void operator()(Array value) const;
            void operator()(Dict value) const;
        };

        //Variant data_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
