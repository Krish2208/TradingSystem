#include "json_parser.h"
#include <charconv>

JsonValue::JsonValue() : value(nullptr) {}
JsonValue::JsonValue(const ValueType& v) : value(v) {}
bool JsonValue::isNull() const { return std::holds_alternative<std::nullptr_t>(value); }

void JsonParser::skipWhitespace() {
    while (pos < input.size() && std::isspace(input[pos])) {
        ++pos;
    }
}

char JsonParser::peek() const {
    return pos < input.size() ? input[pos] : '\0';
}

char JsonParser::advance() {
    return pos < input.size() ? input[pos++] : '\0';
}

bool JsonParser::match(char expected) {
    if (peek() == expected) {
        advance();
        return true;
    }
    return false;
}

std::string JsonParser::parseString() {
    if (!match('"')) throw std::runtime_error("Expected '\"'");
    
    std::string result;
    while (pos < input.size()) {
        char c = advance();
        if (c == '"') break;
        if (c == '\\') {
            c = advance();
            switch (c) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: throw std::runtime_error("Invalid escape sequence");
            }
        } else {
            result += c;
        }
    }
    return result;
}

double JsonParser::parseNumber() {
    const char* start = input.data() + pos;
    const char* end = input.data() + input.size();
    double result;
    auto [ptr, ec] = std::from_chars(start, end, result);
    if (ec != std::errc()) {
        throw std::runtime_error("Invalid number format");
    }
    pos += (ptr - start);
    return result;
}

JsonArray JsonParser::parseArray() {
    if (!match('[')) throw std::runtime_error("Expected '['");
    
    JsonArray arr;
    skipWhitespace();
    
    if (match(']')) return arr;

    while (true) {
        arr.push_back(parseValue());
        skipWhitespace();
        
        if (match(']')) break;
        if (!match(',')) throw std::runtime_error("Expected ',' or ']'");
        skipWhitespace();
    }
    
    return arr;
}

JsonObject JsonParser::parseObject() {
    if (!match('{')) throw std::runtime_error("Expected '{'");
    
    JsonObject obj;
    skipWhitespace();
    
    if (match('}')) return obj;

    while (true) {
        std::string key = parseString();
        skipWhitespace();
        
        if (!match(':')) throw std::runtime_error("Expected ':'");
        skipWhitespace();
        
        obj.emplace(std::move(key), parseValue());
        skipWhitespace();
        
        if (match('}')) break;
        if (!match(',')) throw std::runtime_error("Expected ',' or '}'");
        skipWhitespace();
    }
    
    return obj;
}

JsonValue JsonParser::parseValue() {
    skipWhitespace();
    
    switch (peek()) {
        case 'n':
            if (input.substr(pos, 4) == "null") {
                pos += 4;
                return JsonValue(nullptr);
            }
            throw std::runtime_error("Invalid literal");
            
        case 't':
            if (input.substr(pos, 4) == "true") {
                pos += 4;
                return JsonValue(true);
            }
            throw std::runtime_error("Invalid literal");
            
        case 'f':
            if (input.substr(pos, 5) == "false") {
                pos += 5;
                return JsonValue(false);
            }
            throw std::runtime_error("Invalid literal");
            
        case '"': return JsonValue(parseString());
        case '[': return JsonValue(parseArray());
        case '{': return JsonValue(parseObject());
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return JsonValue(parseNumber());
            
        default:
            throw std::runtime_error("Unexpected character");
    }
}

JsonValue JsonParser::parse(std::string_view json) {
    input = json;
    pos = 0;
    JsonValue result = parseValue();
    skipWhitespace();
    if (pos < input.size()) {
        throw std::runtime_error("Unexpected trailing characters");
    }
    return result;
}
