#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <chrono>
#include <sstream>

namespace fs = std::filesystem;

// Lab1
static void replace_extension(const fs::path& directory) {
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".c") {
            fs::path new_path = entry.path();
            new_path.replace_extension(".cpp");
            fs::rename(entry.path(), new_path);
            std::cout << "Renamed: " << entry.path() << " -> " << new_path << "\n";
        }
    }
}

// Lab2
static void replace_comments(const fs::path& directory) {
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
            std::ifstream file(entry.path());
            if (!file) {
                std::cerr << "Could not open file: " << entry.path() << "\n";
                continue;
            }
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            std::regex comment_regex(R"(//(.*))");
            std::string new_content = std::regex_replace(content, comment_regex, "/*$1 */");

            std::ofstream out_file(entry.path());
            if (!out_file) {
                std::cerr << "Could not write to file: " << entry.path() << "\n";
                continue;
            }
            out_file << new_content;
            out_file.close();

            std::cout << "Processed: " << entry.path() << "\n";
        }
    }
}

// Lab3
std::chrono::system_clock::time_point parse_date(const std::string& date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        throw std::runtime_error("Invalid date format. Use YYYY-MM-DD.");
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::chrono::system_clock::time_point to_system_clock(const fs::file_time_type& file_time) {
    return std::chrono::clock_cast<std::chrono::system_clock>(file_time);
}

void delete_txt_files_before_date(const fs::path& directory, const std::chrono::system_clock::time_point& target_date) {
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            auto last_write_time = fs::last_write_time(entry);
            auto file_time = to_system_clock(last_write_time);

            if (file_time < target_date) {
                fs::remove(entry.path());
                std::cout << "Deleted: " << entry.path() << "\n";
            }
        }
    }
}

// Lab4
static std::time_t to_time_t(std::filesystem::file_time_type fileTime)
{
    const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
    const auto time = std::chrono::system_clock::to_time_t(systemTime);
    return time;
}

static void move_old_txt_files(const fs::path& source_dir, const fs::path& dest_dir) {
    fs::create_directories(dest_dir);
    auto now = std::chrono::system_clock::now();
    auto one_year_ago = now - std::chrono::hours(24 * 365);

    for (const auto& entry : fs::recursive_directory_iterator(source_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            auto creation_time = fs::last_write_time(entry);
            if (to_time_t(creation_time) > std::chrono::system_clock::to_time_t(one_year_ago)) {
                fs::path new_path = dest_dir / entry.path().filename();
                fs::rename(entry.path(), new_path);
                std::cout << "Moved: " << entry.path() << " -> " << new_path << "\n";
            }
        }
    }
}

// Lab5
static void delete_small_word_files(const fs::path& directory) {
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            auto ext = entry.path().extension().string();
            if ((ext == ".doc" || ext == ".docx") && fs::file_size(entry) < 100 * 1024) {
                fs::remove(entry.path());
                std::cout << "Deleted: " << entry.path() << "\n";
            }
        }
    }
}

// Lab6
static double calculate_average_txt_file_size(const fs::path& directory) {
    std::vector<std::size_t> file_sizes;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            try {
                file_sizes.push_back(fs::file_size(entry.path()));
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Error reading file size: " << e.what() << "\n";
            }
        }
    }

    if (file_sizes.empty()) {
        return 0.0;
    }

    std::size_t total_size = 0;
    for (const auto& size : file_sizes) {
        total_size += size;
    }

    return static_cast<double>(total_size) / file_sizes.size();
}

int main() {
    std::string directory;

    // Lab1
    std::cout << "Enter directory: ";
    std::getline(std::cin, directory);
    replace_extension(directory);
    
    // Lab2
    std::cout << "Enter directory: ";
    std::getline(std::cin, directory);
    replace_comments(directory);

    // Lab3
    try {
        std::string directory, date_str;

        std::cout << "Enter directory: ";
        std::getline(std::cin, directory);

        std::cout << "Enter date (YYYY-MM-DD): ";
        std::getline(std::cin, date_str);

        auto target_date = parse_date(date_str);

        delete_txt_files_before_date(directory, target_date);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    // Lab4
    std::string source_dir, dest_dir;
    std::cout << "Enter source directory: ";
    std::getline(std::cin, source_dir);
    std::cout << "Enter destination directory: ";
    std::getline(std::cin, dest_dir);

    move_old_txt_files(source_dir, dest_dir);

    // Lab5
    std::cout << "Enter directory: ";
    std::getline(std::cin, directory);

    delete_small_word_files(directory);

    // Lab6
    try {
        std::string input_path;

        std::cout << "Enter the directory path: ";
        std::getline(std::cin, input_path);

        fs::path directory(input_path);

        if (!fs::exists(directory)) {
            std::cerr << "Error: Directory does not exist.\n";
            return 1;
        }

        if (!fs::is_directory(directory)) {
            std::cerr << "Error: The path is not a directory.\n";
            return 1;
        }

        double average_size = calculate_average_txt_file_size(directory);

        if (average_size == 0.0) {
            std::cout << "No text files found in the directory.\n";
        }
        else {
            std::cout << "The average size of text files in the directory is: "
                << average_size << " bytes.\n";
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}
