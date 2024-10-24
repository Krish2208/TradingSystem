#pragma once

#include <string_view>
#include <variant>
#include <vector>
#include <unordered_map>
#include <stdexcept>

class JsonValue;
using JsonObject = std::unordered_map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;

class JsonValue {
public:
    using ValueType = std::variant<std::nullptr_t, bool, double, std::string, JsonArray, JsonObject>;
    ValueType value;

    JsonValue();
    JsonValue(const ValueType& v);

    template<typename T>
    const T& get() const { return std::get<T>(value); }

    const JsonValue& at(const std::string& key) const {
        return std::get<JsonObject>(value).at(key);
    }
};

class JsonParser {
private:
    std::string_view input;
    size_t pos;

    void skipWhitespace();
    char peek() const;
    char advance();
    bool match(char expected);
    std::string parseString();
    double parseNumber();
    JsonArray parseArray();
    JsonObject parseObject();
    JsonValue parseValue();

public:
    JsonParser() : pos(0) {}
    JsonValue parse(std::string_view json);
};
