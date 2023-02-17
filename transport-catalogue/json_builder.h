#pragma once

#include "json.h"

namespace json {

class KeyItemContext;
class KeyValueItemContext;
class ValueArrayItemContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

    json::Node Build();

private:
    bool is_root_created_ = false;
    bool is_target_key_used_ = true;
    std::string target_key_;
    Node root_;
    std::vector<Node*> nodes_stack_;
};

class KeyItemContext {
public:
    KeyItemContext(Builder& builder) : builder_(builder) {}

    ArrayItemContext StartArray();
    DictItemContext StartDict();
    KeyValueItemContext Value(const Node::Value& value);

private:
    Builder& builder_;
};

class KeyValueItemContext {
public:
    KeyValueItemContext(Builder& builder) : builder_(builder) {}

    KeyItemContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ValueArrayItemContext {
public:
    ValueArrayItemContext(Builder& builder) : builder_(builder) {}

    ValueArrayItemContext Value(const Node::Value& value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder EndArray();

private:
    Builder& builder_;
};

class DictItemContext {
public:
    DictItemContext(Builder& builder) : builder_(builder) {}

    KeyItemContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& builder) : builder_(builder) {}

    ValueArrayItemContext Value(const Node::Value& value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};

} // namespace json