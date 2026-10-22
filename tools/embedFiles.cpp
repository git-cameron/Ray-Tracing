#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string sanitize_identifier(const std::string& filename) {
    std::string result;
    for (char c : filename) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
            result += c;
        } else {
            result += '_';
        }
    }
    return result;
}

std::string escape_content(const std::string& content) {
    std::ostringstream oss;
    for (char c : content) {
        switch (c) {
            case '\\': oss << "\\\\"; break;
            case '\"': oss << "\\\""; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default: oss << c; break;
        }
    }
    return oss.str();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_directory> <output_header.h>\n";
        return 1;
    }

    fs::path input_dir(argv[1]);
    std::ofstream output(argv[2]);

    if (!output.is_open()) {
        std::cerr << "Failed to open output file.\n";
        return 1;
    }

    output << "#pragma once\n\n";
    output << "// Auto-generated file. Do not edit manually.\n\n";

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.is_regular_file()) {
            std::ifstream file(entry.path(), std::ios::in);
            if (!file.is_open()) {
                std::cerr << "Failed to open " << entry.path() << "\n";
                continue;
            }

            std::ostringstream buffer;
            buffer << file.rdbuf();

            std::string var_name = sanitize_identifier(entry.path().filename().string());
            std::string content = escape_content(buffer.str());

            output << "const char* " << var_name << " =\n";
            output << "    \"" << content << "\";\n\n";
        }
    }

    std::cout << "Header file generated successfully: " << argv[2] << "\n";
    return 0;
}
