#include "json.h"

using namespace std;

namespace json {

    Number LoadNumber(std::istream& input) {
        using namespace std::literals;

        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());
            if (!input) {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek())) {
                read_char();
            }
        };

        if (input.peek() == '-') {
            read_char();
        }
        // Парсим целую часть числа
        if (input.peek() == '0') {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        }
        else {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                // Сначала пробуем преобразовать строку в int
                try {
                    return std::stoi(parsed_num);
                }
                catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return std::stod(parsed_num);
        }
        catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    Node LoadNode(istream& input);

    // Считывает содержимое строкового литерала JSON-документа
    // Функцию следует использовать после считывания открывающего символа ":
    Node LoadString(std::istream& input) {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end) {
                // Поток закончился до того, как встретили закрывающую кавычку?
                throw ParsingError("String parsing error");
            }
            const char ch = *it;
            if (ch == '"') {
                // Встретили закрывающую кавычку
                ++it;
                break;
            }
            else if (ch == '\\') {
                // Встретили начало escape-последовательности
                ++it;
                if (it == end) {
                    // Поток завершился сразу после символа обратной косой черты
                    throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r') {
                // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                throw ParsingError("Unexpected end of line"s);
            }
            else {
                // Просто считываем очередной символ и помещаем его в результирующую строку
                s.push_back(ch);
            }
            ++it;
        }

        return Node{ s };
    }

    Node LoadArray(istream& input) {
        Array result;
        char c;
        for (; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }
        if (result.empty() && c != ']')
        {
            throw ParsingError("");
        }

        return Node(move(result));
    }

    Node LoadDict(istream& input) {
        Dict result;
        char c;
        for (; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            while (!IsCorrectSymbol(input.peek()))
            {
                input.get();
            }
            input.get();
            result.insert({ move(key), LoadNode(input) });
        }

        if (result.empty() && c != '}')
        {
            throw ParsingError("");
        }

        return Node(move(result));
    }

    Node LoadBool(istream& input) {
        string result;
        result.push_back(input.get());
        if (result == "t")
        {
            char c;
            while (result != "true" && input >> c) {
                result.push_back(c);
            }
            if (result != "true")
            {
                throw ParsingError("bool parsing error");
            }
            return Node(true);
        }
        else if (result == "f")
        {
            char c;
            while (result != "false" && input >> c) {
                result.push_back(c);
            }
            if (result != "false")
            {
                throw ParsingError("bool parsing error");
            }
            return Node(false);
        }
        else
        {
            throw ParsingError("bool parsing error");
        }
    }

    Node LoadNull(istream& input) {
        string result;
        char c;
        while (result != "null" && input >> c)
        {
            result.push_back(c);
        }
        if (result == "null")
        {
            return Node();
        }
        else
        {
            throw ParsingError("bool parsing error");
        }
    }

    Node LoadNode(istream& input) {
        char c;
        while (input >> c && !IsCorrectSymbol(c)) {}

        if (c == '[') {
            return LoadArray(input);
        }
        else if (c == '{') {
            return LoadDict(input);
        }
        else if (c == '"') {
            return LoadString(input);
        }
        else if (c == '-' || isdigit(c)) {
            input.putback(c);
            Number num = LoadNumber(input);
            if (holds_alternative<int>(num))
            {
                return Node{ get<int>(num) };
            }
            else
            {
                return Node{ get<double>(num) };
            }
        }
        else if (c == 'n') {
            input.putback(c);
            return LoadNull(input);
        }
        else {
            input.putback(c);
            return LoadBool(input);
        }
    }

    Node::Node()
        :data_(std::nullptr_t{}) {}

    Node::Node(int value)
        : data_(value) {}

    Node::Node(double value)
        : data_(value) {}

    Node::Node(bool value)
        : data_(value) {}

    Node::Node(string value)
        : data_(move(value)) {}

    Node::Node(std::nullptr_t value)
        : data_(value) {}

    Node::Node(Array array)
        : data_(move(array)) {}

    Node::Node(Dict map)
        : data_(move(map)) {}

    int Node::AsInt() const {
        if (!IsInt())
        {
            throw logic_error("Error value type");
        }
        return get<int>(data_);
    }

    bool Node::AsBool() const
    {
        if (!IsBool())
        {
            throw logic_error("Error value type");
        }
        return get<bool>(data_);
    }

    double Node::AsDouble() const
    {
        if (!IsDouble())
        {
            throw logic_error("Error value type");
        }
        if (IsPureDouble())
        {
            return get<double>(data_);
        }
        else
        {
            return static_cast<double>(get<int>(data_));
        }
    }

    const string& Node::AsString() const {
        if (!IsString())
        {
            throw logic_error("Error value type");
        }
        return get<string>(data_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray())
        {
            throw logic_error("Error value type");
        }
        return get<Array>(data_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap())
        {
            throw logic_error("Error value type");
        }
        return get<Dict>(data_);
    }

    bool Node::IsInt() const
    {
        return holds_alternative<int>(data_);
    }

    bool Node::IsDouble() const
    {
        return holds_alternative<int>(data_) || holds_alternative<double>(data_);
    }

    bool Node::IsPureDouble() const
    {
        return holds_alternative<double>(data_);
    }

    bool Node::IsBool() const
    {
        return holds_alternative<bool>(data_);
    }

    bool Node::IsString() const
    {
        return holds_alternative<string>(data_);
    }

    bool Node::IsNull() const
    {
        return holds_alternative<nullptr_t>(data_);
    }

    bool Node::IsArray() const
    {
        return holds_alternative<Array>(data_);
    }

    bool Node::IsMap() const
    {
        return holds_alternative<Dict>(data_);
    }

    bool Node::operator==(const Node& other) const
    {
        return data_ == other.data_;
    }

    bool Node::operator!=(const Node& other) const
    {
        return !(*this == other);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& other) const
    {
        return root_ == other.root_;
    }

    bool Document::operator!=(const Document& other) const
    {
        return !(*this == other);
    }

    bool IsCorrectSymbol(char c)
    {
        return (c != ' ' && c != '\t' && c != '\n' && c != '\r');
    }

    Document Load(istream& input) {
        while (!IsCorrectSymbol(input.peek()))
        {
            input.get();
        }
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        doc.GetRoot().Print(output);
    }

    void Node::NodePrinter::operator()(int value) const
    {
        context.out << value;
    }

    void Node::NodePrinter::operator()(double value) const
    {
        context.out << value;
    }

    void Node::NodePrinter::operator()(bool value) const
    {
        //context.out << boolalpha << (value) ? "true" : "false";
        if (value)
        {
            context.out << boolalpha << true;
        }
        else
        {
            context.out << boolalpha << false;
        }
    }

    void Node::NodePrinter::operator()(std::string value) const
    {
        context.out << '"';
        for (auto it = value.begin(); it != value.end(); it++)
        {
            char c = *it;
            switch (c) {
            case '\n':
                context.out << '\\' << 'n';
                break;
            case '\t':
                context.out << "\t";
                break;
            case '\r':
                context.out << "\\r";
                break;
            case '\"':
                context.out << "\\\"";
                break;
            case '\\':
                context.out << "\\\\";
                break;
            default:
                context.out << c;
            }
        }
        context.out << '"';

    }

    void Node::NodePrinter::operator()(std::nullptr_t value) const
    {
        if (value == nullptr_t())
        {
            context.out << "null"sv;
        }
    }

    void Node::NodePrinter::operator()(Array value) const
    {
        context.out << '[';
        for (size_t i = 0; i < value.size(); i++)
        {
            if (i > 0) {
                context.out << ',';
            }
            value[i].Print(context.out);
        }
        context.out << ']';
    }

    void Node::NodePrinter::operator()(Dict value) const
    {
        context.out << '{';
        size_t i = 0;
        for (const auto& [key, val] : value) {
            if (i > 0) {
                context.out << ',';
            }
            context.out << '\"' << key << "\": ";
            val.Print(context.out);
            ++i;
        }
        context.out << '}';
    }

    void Node::Print(std::ostream& out) const
    {
        visit(NodePrinter{ PrintContext{out, 4, 0} }, data_);
    }
}  // namespace json
