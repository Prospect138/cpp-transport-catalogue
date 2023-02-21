#include "json_builder.h"
#include <iostream>
#include <utility>

using namespace std;
namespace json{

Builder::KeyItemContext Builder::Key(const std::string key) {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && is_target_key_used_) {
        target_key_ = std::move(key);
        is_target_key_used_ = false;
    }
    else {
        throw logic_error("Error occurred while building key"s);
    }
    return {*this};
}

Builder& Builder::AddNodeToContainer(Node::Value value) {
    if (nodes_stack_.back()->IsArray()) {
        Node targetValue = Node{};
        targetValue.GetValue() = std::move(value);
        nodes_stack_.back()->AsArray().emplace_back(move(targetValue));
    }
    else if (nodes_stack_.back()->IsDict() && !is_target_key_used_) {
        nodes_stack_.back()->AsDict()[target_key_].GetValue() = move(value);
        is_target_key_used_ = true;
    }
    else {
        throw logic_error("Error occurred while building value"s);
    }

    return *this;
}

Builder& Builder::Value(Node::Value value) {
    if (nodes_stack_.empty() && is_root_created_) {
        throw logic_error("Error occurred while building value"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = value;
        nodes_stack_.push_back(&root_);
        is_root_created_ = true;
    } else {
        AddNodeToContainer(move(value));
    }

    return *this;
}

Builder::DictItemContext Builder::StartDict() {
    if (nodes_stack_.empty() && is_root_created_) {
        throw logic_error("Error occurred while building dict"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = Dict{};
        nodes_stack_.push_back(&root_);
    } else {
        AddNodeToContainer(Dict{});
    }

    return DictItemContext{*this};
}

Builder::ArrayItemContext Builder::StartArray() {
    if (nodes_stack_.empty() && is_root_created_) {
        throw logic_error("Error occurred while building array"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = Array{};
        nodes_stack_.push_back(&root_);
        is_root_created_ = true;
    } else {
        AddNodeToContainer(Array{});
    }

    return ArrayItemContext{*this};
}

Builder& Builder::EndDict() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && is_target_key_used_) {
        if (nodes_stack_.size() == 1u) {
            is_root_created_ = true;
            return {*this};
        }
        nodes_stack_.pop_back();
    }
    else {
        throw logic_error("Error occured while closing Dict"s);
    }
    return {*this};
}

Builder& Builder::EndArray() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        if (nodes_stack_.size() == 1u) {
            is_root_created_ = true;
            return {*this};
        }
        nodes_stack_.pop_back();
    }
    else {
        throw logic_error("Error occured while closing Array"s);
    }
    return {*this};
}

json::Node Builder::Build() {
    if (nodes_stack_.size() == 1 || is_root_created_) {
        return root_;
    }
throw logic_error("Error occured while building JSON"s);
    
}

Builder::ArrayItemContext Builder::KeyItemContext::StartArray() {
    return builder_.StartArray();
}

Builder::DictItemContext Builder::KeyItemContext::StartDict() {
    return builder_.StartDict();
}

Builder::KeyValueItemContext Builder::KeyItemContext::Value(const Node::Value& value) {
    return builder_.Value(value);
}


Builder::KeyItemContext Builder::KeyValueItemContext::Key(string key) {
    return builder_.Key(std::move(key));
}

Builder& Builder::KeyValueItemContext::EndDict() {
    return builder_.EndDict();
}

Builder::ValueArrayItemContext Builder::ValueArrayItemContext::Value(const Node::Value& value) {
    return builder_.Value(std::move(value));
}

Builder::DictItemContext Builder::ValueArrayItemContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::ValueArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder Builder::ValueArrayItemContext::EndArray() {
    return builder_.EndArray();
}

Builder::KeyItemContext Builder::DictItemContext::Key(string key) {
    return builder_.Key(std::move(key));
}

Builder& Builder::DictItemContext::EndDict() {
    return builder_.EndDict();
}

Builder::ValueArrayItemContext Builder::ArrayItemContext::Value(const Node::Value& value) {
    return builder_.Value(std::move(value));
}

Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& Builder::ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

} //namespace json