#include "json_utils.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace json_utils {

std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        throw FileError("Could not open file: " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    if (file.bad()) {
        throw FileError("Error occurred while reading file: " + filepath);
    }
    
    return buffer.str();
}

void validateFile(const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
        throw FileError("File does not exist: " + filepath);
    }

    auto fileSize = std::filesystem::file_size(filepath);
    if (fileSize == 0) {
        throw FileError("File is empty: " + filepath);
    }

    if (fileSize > Config::MAX_FILE_SIZE) {
        throw FileError("File is too large (>100MB): " + filepath);
    }
}

void printJson(const JsonValue& value, int indent) {
    if (indent > Config::MAX_INDENT) {
        throw PrintError("Maximum indent level exceeded");
    }

    const std::string indentation(indent * 2, ' ');
    
    try {
        if (std::holds_alternative<nullptr_t>(value.value)) {
            std::cout << "null";
        }
        else if (std::holds_alternative<bool>(value.value)) {
            std::cout << (value.get<bool>() ? "true" : "false");
        }
        else if (std::holds_alternative<double>(value.value)) {
            std::cout << value.get<double>();
        }
        else if (std::holds_alternative<std::string>(value.value)) {
            std::cout << "\"" << value.get<std::string>() << "\"";
        }
        else if (std::holds_alternative<JsonArray>(value.value)) {
            const auto& array = value.get<JsonArray>();
            std::cout << "[\n";
            
            for (size_t i = 0; i < array.size(); ++i) {
                std::cout << indentation << "  ";
                printJson(array[i], indent + 1);
                if (i < array.size() - 1) {
                    std::cout << ",";
                }
                std::cout << "\n";
            }
            
            std::cout << indentation << "]";
        }
        else if (std::holds_alternative<JsonObject>(value.value)) {
            const auto& object = value.get<JsonObject>();
            std::cout << "{\n";
            
            size_t i = 0;
            for (const auto& [key, val] : object) {
                std::cout << indentation << "  \"" << key << "\": ";
                printJson(val, indent + 1);
                if (i < object.size() - 1) {
                    std::cout << ",";
                }
                std::cout << "\n";
                ++i;
            }
            
            std::cout << indentation << "}";
        }
    } catch (const std::exception& e) {
        throw PrintError(std::string("Error while printing JSON: ") + e.what());
    }
}

void processJsonFile(const std::string& filepath) {
    try {
        validateFile(filepath);
        std::string jsonContent = readFile(filepath);
        
        JsonParser parser;
        JsonValue result = parser.parse(jsonContent);

        std::cout << "Successfully parsed JSON from: " << filepath << "\n\n";
        std::cout << "Content:\n";
        printJson(result);

    } catch (const std::exception& e) {
        throw FileError(std::string("Failed to process JSON file: ") + e.what());
    }
}

} // namespace json_utils