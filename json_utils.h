#pragma once

#include "json_parser.h"
#include <string>
#include <iostream>

namespace json_utils {

// File operations
std::string readFile(const std::string& filepath);
void validateFile(const std::string& filepath);

// JSON operations
void printJson(const JsonValue& value, int indent = 0);
void processJsonFile(const std::string& filepath);

// Configuration
struct Config {
    static constexpr size_t MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB
    static constexpr int MAX_INDENT = 100; // Maximum indent level
};

// Custom exceptions
class FileError : public std::runtime_error {
public:
    explicit FileError(const std::string& msg) : std::runtime_error(msg) {}
};

class PrintError : public std::runtime_error {
public:
    explicit PrintError(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace json_utils