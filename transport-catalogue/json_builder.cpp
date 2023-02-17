#include "json_builder.h"
#include <iostream>
#include <utility>

using namespace std;
namespace json{

KeyItemContext Builder::Key(const std::string key) {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && is_target_key_used_) {
        target_key_ = std::move(key);
        is_target_key_used_ = false;
    }
    else {
        throw logic_error("Error occurred while building key"s);
    }
    return {*this};
}

Builder& Builder::Value(Node::Value value) {
    if (nodes_stack_.empty() && is_root_created_) {
        throw logic_error("Error occurred while building value"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = value;
        nodes_stack_.push_back(&root_);
        is_root_created_ = true;
    }
    else if (nodes_stack_.back()->IsArray()) {
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

DictItemContext Builder::StartDict() {
    if (nodes_stack_.empty() && is_root_created_) {
        throw logic_error("Error occurred while building dict"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = Dict{};
        nodes_stack_.push_back(&root_);
    }
    else if (nodes_stack_.back()->IsArray()) {
        Node targetValue{Dict{}};
        nodes_stack_.back()->AsArray().push_back(targetValue);
        nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
    }
    else if (nodes_stack_.back()->IsDict() && !is_target_key_used_) {
        Node targetValue{Dict{}};
        nodes_stack_.back()->AsDict().insert({target_key_, targetValue});
        nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(target_key_));
        is_target_key_used_ = true;
    }
    else {
        throw logic_error("Error occurred while building dict"s);
    }

    return DictItemContext{*this};
}

ArrayItemContext Builder::StartArray() {
    if (nodes_stack_.empty() && is_root_created_) {
        throw logic_error("Error occurred while building array"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = Array{};
        nodes_stack_.push_back(&root_);
        is_root_created_ = true;
    }
    else if (nodes_stack_.back()->IsArray()) {
        Node targetValue{Array {}};
        nodes_stack_.back()->AsArray().push_back(targetValue);
        nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());

    }
    else if (nodes_stack_.back()->IsDict() && !is_target_key_used_) {
        Node targetValue{Array {}};
        nodes_stack_.back()->AsDict().insert({target_key_, targetValue});
        nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(target_key_));
        is_target_key_used_ = true;
    }
    else {
        throw logic_error("Error occurred while building array"s);
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

ArrayItemContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}

DictItemContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}

KeyValueItemContext KeyItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}


KeyItemContext KeyValueItemContext::Key(string key) {
    return builder_.Key(key);
}

Builder& KeyValueItemContext::EndDict() {
    return builder_.EndDict();
}

ValueArrayItemContext ValueArrayItemContext::Value(const Node::Value& value) {
    return builder_.Value(value);
}

DictItemContext ValueArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ValueArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder ValueArrayItemContext::EndArray() {
    return builder_.EndArray();
}

KeyItemContext DictItemContext::Key(string key) {
    return builder_.Key(key);
}

Builder &DictItemContext::EndDict() {
    return builder_.EndDict();
}

ValueArrayItemContext ArrayItemContext::Value(const Node::Value& value) {
    return builder_.Value(value);
}

DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

} //namespace json