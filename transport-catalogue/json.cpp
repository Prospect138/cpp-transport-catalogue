#include "json.h"
#include "cassert"

using namespace std;

namespace json {

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
    return {out, indent_step, indent_step + indent};
}
};

namespace {

Node LoadNode(istream& input);

using Number = std::variant<int, double>;

struct NodeNumConstructor {
    int operator()(int integer) const {
        return integer;
    }
};

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
    } else {
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
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input) {
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
        } else if (ch == '\\') {
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
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadBool(std::istream& input) {
    std::string check;
    check.reserve(5);
    char c;

    while(check.size()<4 && input >> c) {
        check += c;
    }
    
    if(check.size() < 4) {
        throw ParsingError("Failed to read bool value. EOF!"s);
    }
    
    bool result;
    if (check == "true"s) {
        result = true;
    } 
    else {
        if ( !(input >> c) ) {
            throw ParsingError("Failed to read bool value. EOF!"s);
        }
        check += c;
        if (check != "false"s) {
            cerr << "Data>>"s << check << "<<"s << endl;
            throw ParsingError("Failed to read bool value. Incorrect symbols presented."s);
        }
        result = false;
    }
    
    return Node(result);
}

Node LoadNull(istream& input){
    char str[10];
    input.get(str, 5, ',');
    // Читаем первые 5 символов или до запятой
    std::string line(str);
    if (line == "null"){
        // Если строка null
        return Node{};
    }
    throw ParsingError("not a null");
}

Node LoadArray(istream& input) {
    Array result;
    char c;
    input >> c;
    if (c == ']'){
        // 
        return result;
    }
    else {
        if (!input){
        // Если input опустел после открытия, выбрасываем ParsingError
            throw ParsingError("Array parsing error, unexpected end of file.");
        }
        input.putback(c);
        // Если нет, вернем, символ
    }
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;
    char c;
    input >> c;
    if (c == '}'){
        return result;
    }
    else {
        if (!input){
            throw ParsingError("Dict parsing error, unexpected end of file.");
        }
        input.putback(c);
    }

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;
    // Проверяем тип ноды по первому символу
    //if (!input) {
    //    throw ParsingError("Error parsing");
    //}
    if (c == 'n') {
        input.putback(c);      
        return LoadNull(input);
    }
    if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    }
    if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (c == '"') {
        return Node(LoadString(input));
    }
    else {
        input.putback(c);
        auto constructor = LoadNumber(input);
        if (std::holds_alternative<int>(constructor)){
            int constructor2 = std::get<int>(constructor); 
            return Node(constructor2);
        }
        else {
            double constructor2 = std::get<double>(constructor); 
            return Node(constructor2);
        }
    }
}

}  // namespace

using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

bool Node::IsInt() const{
    return std::holds_alternative<int>(*this);
}

//bool IsDouble() const; Возвращает true, если в Node хранится int либо double.
//bool IsPureDouble() const; Возвращает true, если в Node хранится double.
bool Node::IsDouble() const {
    return (std::holds_alternative<double>(*this) || 
    std::holds_alternative<int>(*this));
}
bool Node::IsPureDouble() const {
    return (std::holds_alternative<double>(*this));
}
bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}
bool Node::IsString() const {
    return std::holds_alternative<string>(*this);
}
bool Node::IsNull() const{ 
    return std::holds_alternative<nullptr_t>(*this);
}
bool Node::IsArray() const{
    return std::holds_alternative<Array>(*this);
}
bool Node::IsMap() const{
    return std::holds_alternative<Dict>(*this);
}

const Array& Node::AsArray() const {
    if (auto value = std::get_if<Array>(this)) {
        return *value;
    }   
    throw std::logic_error("bad_type"s);
}
const Dict& Node::AsMap() const {
    if (auto value = std::get_if<Dict>(this)) {
        return *value;
    }   
    throw std::logic_error("bad_type"s);
}
int Node::AsInt() const {
    if (auto value = std::get_if<int>(this)) {
        return *value;
    }   
    throw std::logic_error("bad_type"s);
}
bool Node::AsBool() const {
    if (auto value = std::get_if<bool>(this)) {
        return *value;
    }   
    throw std::logic_error("bad_type");
}
double Node::AsDouble() const { //to string here
    if (auto value = std::get_if<double>(this)) {
        return *value;
    }   
    else if (auto value = std::get_if<int>(this)){
        // Int тоже можно представить, как double
        return static_cast<double>(*value);
    }
    throw std::logic_error("bad_type");
}

const string& Node::AsString() const {
    if (auto value = std::get_if<std::string>(this)) {
        return *value;
    }   
    throw std::logic_error("bad_type");
}

bool operator== (const Node& lhs, const Node& rhs){
    if (lhs.IsArray() && rhs.IsArray()){
        return lhs.AsArray() == rhs.AsArray();
    }
    else if (lhs.IsBool() && rhs.IsBool()){
        return lhs.AsBool() == rhs.AsBool();
    }
    else if (lhs.IsInt() && rhs.IsInt()){
        return lhs.AsInt() == rhs.AsInt();
    }
    else if (lhs.IsDouble() && rhs.IsDouble()){
        if (lhs.IsInt()){
            // Но сравнивать int с double нельзя
            return false;
        }
        return lhs.AsDouble() == rhs.AsDouble();
    }
    else if (lhs.IsMap() && rhs.IsMap()){
        return lhs.AsMap() == rhs.AsMap();
    }
    else if (lhs.IsNull() && rhs.IsNull()){
        return true;
    }
    else if (lhs.IsPureDouble() && rhs.IsPureDouble()){
        return lhs.AsDouble() == rhs.AsDouble();
    }
    else if (lhs.IsString() && rhs.IsString()){
        return lhs.AsString() == rhs.AsString();
    }
    return false;
}

bool operator!= (const Node& lhs, const Node& rhs){
    return !(lhs == rhs);
}

const Value& Node::GetValue() const{
    return *this;
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

template <typename Value>
void PrintValue(const Value& value, ostream& out){
    out << value;
}

void PrintValue(std::nullptr_t, ostream& out){
    out << "null"s;
}

void PrintValue(const std::string& str, ostream& out){
    using namespace std::literals;
    out<<'\"';
    for (size_t i = 0; i < str.size(); ++i){
        // Проверки на escape - последовательности
        if (str[i] == '\"' || str[i] == '\\' ) {
            out << '\\';
        }
        if (str[i] == '\r'){
            out << R"(\r)";
            continue;
        }
        if (str[i] == '\n'){
            out << R"(\n)";
            continue;
        }
        out << str[i];
    }
    out << '\"';
}

void PrintValue(bool bl, ostream& out){
    if (bl == true){
        out << "true"sv;
    }
    else {
        out << "false"sv;
    }
}

void PrintValue(const Array& arr, ostream& out){
    out<<'[';
    for(auto iter = arr.begin(); iter != arr.end(); ){
        std::visit([&out](auto& value){PrintValue(value, out);}, 
        iter -> GetValue());
        if (++iter != arr.end()){
            out << ", "sv;
        }
    }
    out<<']';
}

void PrintValue(const Dict& dict, ostream& out){
    out << '{';
    for(auto iter = dict.begin(); iter != dict.end(); ){
        out << "\"" << iter -> first << "\": ";
        std::visit([&out](auto& value){PrintValue(value, out);}, 
        iter -> second.GetValue());
        if (++iter != dict.end()){
            out << ','<<' ';
        }
    }
    out << '}';
}

void PrintNode(const Node& node, std::ostream& output){
    std::visit([&output](const auto& value){PrintValue(value, output);}, 
    node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    Node node = doc.GetRoot();
    PrintNode(node, output);
}

bool operator== (const Document& doc1, const Document& doc2){
    return doc1.GetRoot() == doc2.GetRoot();
}

bool operator!= (const Document& doc1, const Document& doc2){
    return !(doc1 == doc2);
}

}  // namespace json