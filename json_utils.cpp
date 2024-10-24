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
            std::cout << indentation << "null" << std::endl;
        }
        else if (std::holds_alternative<bool>(value.value)) {
            std::cout << indentation << (value.get<bool>() ? "true" : "false") << std::endl;
        }
        else if (std::holds_alternative<double>(value.value)) {
            std::cout << indentation << value.get<double>() << std::endl;
        }
        else if (std::holds_alternative<std::string>(value.value)) {
            std::cout << indentation << "\"" << value.get<std::string>() << "\"" << std::endl;
        }
        else if (std::holds_alternative<JsonArray>(value.value)) {
            std::cout << indentation << "[\n";
            for (const auto& element : value.get<JsonArray>()) {
                printJson(element, indent + 1);
            }
            std::cout << indentation << "]" << std::endl;
        }
        else if (std::holds_alternative<JsonObject>(value.value)) {
            std::cout << indentation << "{\n";
            for (const auto& [key, val] : value.get<JsonObject>()) {
                std::cout << indentation << "  \"" << key << "\": ";
                printJson(val, indent + 1);
            }
            std::cout << indentation << "}" << std::endl;
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