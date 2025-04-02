#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mutationConstants.h"
#include "mutator.h"

#ifdef _WIN32
#    include <io.h> // Windows
#    include <windows.h>
#else
#    include <fcntl.h> // Unix
#    include <sys/mman.h>
#    include <unistd.h>
#    include <utime.h>
#endif

Mutator::Mutator(char const* source, std::string path)
    : source(source)
    , path(path)
    , gen(rd())
{
    initializeStrategies();
}

void Mutator::initializeStrategies()
{
    strategies = {
        [this](std::string const& c) { return variableGibberishStrategy(c); },
        [this](std::string const& c) { return invertConditionalsStrategy(c); },
        [this](std::string const& c) { return swapFunctionCallsStrategy(c); },
        [this](std::string const& c) { return deleteRandomLinesStrategy(c); },
        [this](std::string const& c) { return swapCodeLinesStrategy(c); },
        [this](std::string const& c) { return replaceOperatorsStrategy(c); },
        [this](std::string const& c) { return randomCommentOutStrategy(c); },
        [this](std::string const& c) { return destroyFormattingStrategy(c); },
        [this](std::string const& c) { return insertUselessFunctionsStrategy(c); },
        [this](std::string const& c) { return printJuliaSet(c); },
        [this](std::string const& c) { return printErrorMessage(c); },
    };
}

std::string Mutator::randomGibberish(int length)
{
    std::string result;
    for (int i = 0; i < length; ++i) {
        result += chars[std::uniform_int_distribution<>(0, chars.size() - 1)(gen)];
    }
    return result;
}

std::string Mutator::randomOperator()
{
    static std::vector<std::string> const ops = { "+", "-", "*", "/", "%", "&", "|", "^", "&&", "||" };
    return ops[std::uniform_int_distribution<>(0, ops.size() - 1)(gen)];
}

// ===== Mutation Strategies ===== //

std::string Mutator::variableGibberishStrategy(std::string const& content)
{
    std::regex var_regex(R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\b)");
    std::string result;
    std::sregex_iterator it(content.begin(), content.end(), var_regex);
    std::sregex_iterator end;
    size_t last_pos = 0;

    for (; it != end; ++it) {
        result.append(content, last_pos, it->position() - last_pos);

        // Skip keywords and types
        std::string match = it->str();
        static std::set<std::string> const keywords = {
            "if", "else", "while", "for", "return", "int", "float",
            "double", "char", "void", "bool", "true", "false"
        };

        if (keywords.count(match)) {
            result.append(match);
        } else {
            result.append(std::uniform_int_distribution<>(0, 1)(gen) ? match : randomGibberish());
        }

        last_pos = it->position() + it->length();
    }

    result.append(content, last_pos, content.length() - last_pos);
    return result;
}

std::string Mutator::invertConditionalsStrategy(std::string const& content)
{
    std::regex cond_regex(R"((if|while|for)\s*\(([^)]*)([<>]=?|==|!=)([^)]*)\))");
    std::string result;
    std::sregex_iterator it(content.begin(), content.end(), cond_regex);
    std::sregex_iterator end;
    size_t last_pos = 0;

    for (; it != end; ++it) {
        result.append(content, last_pos, it->position() - last_pos);

        std::string op = (*it)[3];
        std::string inverted_op = op;

        if (op == "<")
            inverted_op = ">=";
        else if (op == ">")
            inverted_op = "<=";
        else if (op == "<=")
            inverted_op = ">";
        else if (op == ">=")
            inverted_op = "<";
        else if (op == "==")
            inverted_op = "!=";
        else if (op == "!=")
            inverted_op = "==";

        result.append((*it)[1].str() + "(" + (*it)[2].str() + inverted_op + (*it)[4].str() + ")");
        last_pos = it->position() + it->length();
    }

    result.append(content, last_pos, content.length() - last_pos);
    return result;
}

std::string Mutator::swapFunctionCallsStrategy(std::string const& content)
{
    // First pass: collect all function names in the code
    std::regex func_regex(R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\()");
    std::set<std::string> function_names;

    std::sregex_iterator it(content.begin(), content.end(), func_regex);
    std::sregex_iterator end;

    for (; it != end; ++it) {
        function_names.insert((*it)[1]);
    }

    // Need at least 2 functions to swap
    if (function_names.size() < 2) {
        return content;
    }

    // Convert to vector and select two random functions to swap
    std::vector<std::string> functions(function_names.begin(), function_names.end());
    std::shuffle(functions.begin(), functions.end(), gen);

    std::string const& func1 = functions[0];
    std::string const& func2 = functions[1];

    // Second pass: perform the swap
    std::string result = content;
    size_t pos = 0;

    while (pos < result.length()) {
        // Find func1
        size_t found1 = result.find(func1 + "(", pos);
        // Find func2
        size_t found2 = result.find(func2 + "(", pos);

        // Determine which comes first
        size_t next_pos = std::min(found1, found2);
        if (next_pos == std::string::npos)
            break;

        // Verify it's a whole word match
        if (next_pos > 0 && (isalnum(result[next_pos - 1]) || result[next_pos - 1] == '_')) {
            pos = next_pos + 1;
            continue;
        }

        // Perform the swap
        if (next_pos == found1) {
            result.replace(next_pos, func1.length(), func2);
            pos = next_pos + func2.length();
        } else {
            result.replace(next_pos, func2.length(), func1);
            pos = next_pos + func1.length();
        }
    }

    return result;
}

std::string Mutator::deleteRandomLinesStrategy(std::string const& content)
{
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;

    while (std::getline(iss, line)) {
        if (std::uniform_int_distribution<>(0, 9)(gen) > 2) { // 70% chance to keep
            lines.push_back(line);
        }
    }

    std::ostringstream oss;
    for (auto const& l : lines) {
        oss << l << '\n';
    }
    return oss.str();
}

std::string Mutator::swapCodeLinesStrategy(std::string const& content)
{
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;

    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    if (lines.size() > 1) {
        std::uniform_int_distribution<> dist(0, lines.size() - 1);
        std::swap(lines[dist(gen)], lines[dist(gen)]);
    }

    std::ostringstream oss;
    for (auto const& l : lines) {
        oss << l << '\n';
    }
    return oss.str();
}

std::string Mutator::replaceOperatorsStrategy(std::string const& content)
{
    std::regex op_regex(R"([+\-*/%&|^=<>!]=?)");

    std::string result;
    std::sregex_iterator it(content.begin(), content.end(), op_regex);
    std::sregex_iterator end;
    size_t last_pos = 0;

    for (; it != end; ++it) {
        // Append the non-matched portion
        result.append(content, last_pos, it->position() - last_pos);

        // Randomly decide whether to replace this operator
        if (std::uniform_int_distribution<>(0, 1)(gen)) {
            // Keep original operator
            result.append(it->str());
        } else {
            // Replace with random operator
            result.append(randomOperator());
        }

        last_pos = it->position() + it->length();
    }

    // Append the remaining portion
    result.append(content, last_pos, content.length() - last_pos);

    return result;
}

std::string Mutator::randomCommentOutStrategy(std::string const& content)
{
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;

    while (std::getline(iss, line)) {
        if (!line.empty() && std::uniform_int_distribution<>(0, 9)(gen) == 0) {
            line = "// " + line;
        }
        lines.push_back(line);
    }

    std::ostringstream oss;
    for (auto const& l : lines) {
        oss << l << '\n';
    }
    return oss.str();
}

std::string Mutator::destroyFormattingStrategy(std::string const& content)
{
    std::string result;
    bool inString = false;

    for (char c : content) {
        if (c == '\"')
            inString = !inString;

        if (!inString) {
            if (c == '\t' || c == ' ') {
                // Replace with random whitespace
                int spaces = std::uniform_int_distribution<>(1, 8)(gen);
                result.append(spaces, ' ');
                continue;
            } else if (c == '\n' && std::uniform_int_distribution<>(0, 1)(gen)) {
                // Randomly remove newlines
                continue;
            }
        }
        result += c;
    }
    return result;
}

std::string Mutator::insertUselessFunctionsStrategy(std::string const& content)
{
    std::string result = content;
    std::vector<size_t> insertion_points;

    // 1. Find all valid insertion points (after lines ending with ;,}, or newlines)
    size_t pos = 0;
    while (pos < result.length()) {
        pos = result.find_first_of(";\n}", pos);
        if (pos == std::string::npos)
            break;
        insertion_points.push_back(pos + 1);
        pos++;
    }

    // 2. If no good points found, use start/end as fallback
    if (insertion_points.empty()) {
        insertion_points = { 0, result.length() };
    }

    // 3. Insert 1-3 functions at random valid locations
    int insertions = std::uniform_int_distribution<>(1, 3)(gen);
    std::shuffle(insertion_points.begin(), insertion_points.end(), gen);

    for (int i = 0; i < std::min(insertions, (int)insertion_points.size()); ++i) {
        size_t insert_pos = insertion_points[i];
        auto const& func = uselessFuncs[std::uniform_int_distribution<>(0, uselessFuncs.size() - 1)(gen)];

        // Format insertion with proper spacing
        std::string to_insert = "\n" + func + "\n";

        // Ensure we don't create double newlines
        if (insert_pos > 0 && result[insert_pos - 1] == '\n') {
            to_insert.erase(0, 1);
        }
        if (insert_pos < result.length() && result[insert_pos] == '\n') {
            to_insert.pop_back();
        }

        result.insert(insert_pos, to_insert);
    }

    return result;
}

std::string Mutator::printJuliaSet(std::string const& content)
{
    int width = 80;
    int height = 40;

    // Seed the random number generator
    std::srand(std::time(0));

    // Generate random values for cr and ci (in a range that produces interesting patterns)
    double cr = (std::rand() % 2000) / 1000.0 - 1.0; // Random between -1.0 and 1.0
    double ci = (std::rand() % 2000) / 1000.0 - 1.0; // Random between -1.0 and 1.0

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double zr = (x * 3.0 / width) - 1.5;
            double zi = (y * 2.0 / height) - 1.0;
            int iter = 0;

            while (zr * zr + zi * zi < 4.0 && iter < 100) {
                double tmp = zr * zr - zi * zi + cr;
                zi = 2 * zr * zi + ci;
                zr = tmp;
                iter++;
            }

            std::cout << " .-:;+=xX$&"[iter % 12];
        }
        std::cout << std::endl;
    }

    return content;
}

std::string Mutator::printErrorMessage(std::string const& content)
{
    // Initialize random seed
    std::srand(static_cast<unsigned int>(std::time(0)));

    // Get a random index between 0 and the size of the vector
    int randomIndex = std::rand() % errorMessages.size();

    // Return the randomly selected message
    std::cout << errorMessages[randomIndex] << std::endl;

    return content;
}

// ===== Core Mutator Functions ===== //

bool Mutator::overwriteFile(std::string const& content)
{
    std::ofstream outFile(path, std::ios::out | std::ios::trunc);
    if (!outFile.is_open())
        return false;
    outFile << content;
    bool success = outFile.good();
    outFile.close();
    return success;
}

Mutator::MutationStrategy Mutator::selectRandomStrategy()
{
    if (strategies.empty())
        throw std::runtime_error("No strategies available");
    std::uniform_int_distribution<> dist(0, strategies.size() - 1);
    return strategies[dist(gen)];
}

void randomizeTimestamps(std::string const& filepath)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<time_t> dist(0, time(nullptr));

#ifdef _WIN32
    // Windows implementation
    HANDLE hFile = CreateFileA(
        filepath.c_str(),
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        FILETIME ft;
        SYSTEMTIME st;
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);

        // Add random offset (up to 1 year in 100ns intervals)
        ft.dwLowDateTime += dist(gen) * 10000000ULL;
        SetFileTime(hFile, NULL, &ft, &ft); // Only modify access/write times
        CloseHandle(hFile);
    }
#else
    // Unix implementation
    struct utimbuf new_times;
    new_times.actime = dist(gen);  // Random access time
    new_times.modtime = dist(gen); // Random modification time
    utime(filepath.c_str(), &new_times);
#endif
}

void lockFile(std::string const& path)
{
#ifdef _WIN32
    // Windows implementation
    HANDLE hFile = CreateFileA(
        path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, // Exclusive lock
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        OVERLAPPED ov = { 0 };
        LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &ov);
        // Note: Handle must be closed later!
    }
#else
    // Unix implementation (flock())
    int fd = open(path.c_str(), O_RDWR);
    if (fd != -1) {
        flock(fd, LOCK_EX); // Exclusive lock
        close(fd);          // Lock persists until process exits
    }
#endif
}

void overwriteAndBreakUndo(std::string const& path, std::string const& content)
{
#ifdef _WIN32
    // Windows: Use MoveFileEx to break file identity
    std::string temp = path + ".tmp";
    std::ofstream(temp) << content;
    MoveFileExA(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
#else
    // Unix: Use rename() to force new inode
    std::string temp = path + ".tmp";
    std::ofstream(temp) << content;
    rename(temp.c_str(), path.c_str());
#endif
}

void disableBackups(std::string const& path)
{
    std::vector<std::string> backups = {
        path + "~",    // Common backup
        path + ".bak", // Windows backup
        path + ".swp"  // Vim swap
    };
    for (auto const& backup : backups) {
        remove(backup.c_str());
    }
}

void destroyVSCodeUndo(std::string const& path)
{
    // 1. Get file size
    struct stat st;
    if (stat(path.c_str(), &st) != 0 || st.st_size == 0)
        return;
    size_t file_size = st.st_size;

    // 2. Memory-map the file
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    HANDLE hMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (!hMap) {
        CloseHandle(hFile);
        return;
    }

    char* data = (char*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, file_size);
#else
    int fd = open(path.c_str(), O_RDWR);
    if (fd == -1)
        return;

    char* data = (char*)mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return;
    }
#endif

    // 3. Corrupt undo checksums (flip random bits)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, file_size - 1);

    for (int i = 0; i < 100; ++i) {
        size_t pos = dist(gen);
        data[pos] ^= 0xFF; // Invert bits
    }

    // 4. Cleanup
#ifdef _WIN32
    FlushViewOfFile(data, file_size);
    UnmapViewOfFile(data);
    CloseHandle(hMap);
    CloseHandle(hFile);
#else
    msync(data, file_size, MS_SYNC);
    munmap(data, file_size);
    close(fd);
#endif

    // 5. Final atomic overwrite to break inode linkage
    std::string temp = path + ".tmp";
    std::ofstream(temp, std::ios::binary) << std::string(data, file_size);

#ifdef _WIN32
    MoveFileExA(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
#else
    rename(temp.c_str(), path.c_str());
#endif
}

void Mutator::mutateCode()
{
    try {
        std::ifstream inFile(path);
        if (!inFile.is_open()) {
            std::cerr << "Error opening file: " << path << std::endl;
            return;
        }

        std::string content((std::istreambuf_iterator<char>(inFile)),
            std::istreambuf_iterator<char>());
        inFile.close();

        // Select mutation strategy
        auto strategy = selectRandomStrategy();
        std::string mutated = strategy(content);

        // Overwrite the source code
        if (!overwriteFile(mutated)) {
            std::cerr << "Failed to write mutated content" << std::endl;
        }

        // Randomize timestamps
        randomizeTimestamps(path);
        // Lock file (blocks undo)
        lockFile(path);
        // Overwrite with new inode (breaks undo history)
        overwriteAndBreakUndo(path, mutated);
        // Delete backups (prevents .~ recovery)
        disableBackups(path);
        // destroyVSCodeUndo(path);
    } catch (std::exception const& e) {
        std::cerr << "Mutation failed: " << e.what() << std::endl;
    }
}
