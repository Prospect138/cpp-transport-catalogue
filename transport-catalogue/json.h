#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <ostream>

namespace json {

struct PrintContext;

//using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>{
public:

    using variant::variant;
    using Value = variant;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    const Value& GetValue() const;

    friend bool operator== (const Node& lhs, const Node& rhs);
    friend bool operator!= (const Node& lhs, const Node& rhs);
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);
//using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

//Node Node::operator== (Node other_node);


void PrintValue(std::nullptr_t, std::ostream& out);

void PrintValue(const std::string& str, std::ostream& out);

void PrintValue(bool bl, std::ostream& out);

void PrintValue(const Array& arr, std::ostream& out);

void PrintValue(const Dict& dict, std::ostream& out);

void PrintNode(const Node& node, std::ostream& output);

void Print(const Document& doc, std::ostream& output);

bool operator== (const Document& doc1, const Document& doc2);

bool operator!= (const Document& doc1, const Document& doc2);



}  // namespace json