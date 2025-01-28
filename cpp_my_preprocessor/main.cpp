#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;
namespace fs = std::filesystem;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}
string GetFileContents(string file) {
    ifstream stream(file);

    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

fs::path FindInDirectory(const path& dir, const string& included_file) {
    error_code err;
    for (const auto& dir_entry : fs::directory_iterator(dir, err)) {
        if (err) {
            cerr << "Error reading directory " << dir << ": " << err.message() << endl;
            continue; // Переходим к следующему элементу, если есть ошибка.
        }

        if (fs::is_directory(dir_entry.status())) {
            fs::path found = FindInDirectory(dir_entry.path(), included_file);
            if (!found.empty()) {
                return found; // Найдено в поддиректории
            }
        }
        else if (dir_entry.path().filename() == included_file) {
            return dir_entry.path(); // Найден файл
        }
    }
    return ""; // Файл не найден
}
// напишите эту функцию
bool Preprocess(const fs::path& input_file, const fs::path& output_file, const vector<fs::path>& include_dirs) {
    error_code err;
    ifstream infile(input_file);
    if (!infile.is_open()) {
        cerr << "Could not open input file: " << input_file << endl;
        return false;
    }

    ofstream outfile(output_file, std::ios::app);
    if (!outfile.is_open()) {
        cerr << "Could not open output file: " << output_file << endl;
        return false;
    }

    string line;
    regex include_regex1(R"(\s*#\s*include\s*"([^"]*)\"\s*)");
    regex include_regex2(R"(\s*#\s*include\s*<([^>]*)>\s*)");
    int line_number = 0;
    
    while (getline(infile, line)) {
        ++line_number;
        smatch match;
        // Проверяем на наличие директивы #include
        if (regex_search(line, match, include_regex2)) {
            path included_file = string(match[1]);
            bool included = false;

            // Ищем файл в директориях include
            for (const auto& dir : include_dirs) {
                fs::path file_path = FindInDirectory(dir, included_file.filename().string());
                if (fs::exists(file_path)) {                    
                    // Если файл найден, рекурсивно обрабатываем его
                    if(!Preprocess(file_path, output_file, include_dirs)){
                        return false;
                    }
                    included = true;
                    break;
                }
            }

            if (!included) {
                cout << "unknown include file " << included_file.filename().string() << " at file " << input_file.string() << " at line " << line_number;
                return false;
            }
        }
        else if (regex_search(line, match, include_regex1))
        {
            path included_file = string(match[1]);
            bool included = false;

            // Ищем файл относительно текущего файла
            fs::path file_path = input_file.parent_path() / included_file;
            if (fs::exists(file_path)) {
                // Если файл найден, рекурсивно обрабатываем его
                if(!Preprocess(file_path, output_file, include_dirs)){
                    return false;
                }
                included = true;
            }
            else {
                // Ищем файл в директориях include
                for (const auto& dir : include_dirs) {
                    fs::path file_path = FindInDirectory(dir, included_file.filename().string());
                    if (fs::exists(file_path)) {
                        // Если файл найден, рекурсивно обрабатываем его
                        if(!Preprocess(file_path, output_file, include_dirs)){
                            return false;
                        }
                        included = true;
                        break;
                    }
                }
                if (!included) {
                    cout << "unknown include file " << included_file.filename().string() << " at file " << input_file.string() << " at line " << line_number;
                    return false;
                }
            }
        }
        
        else {
            // Если это не директива #include, просто выводим строку
            outfile << line << endl;
        }
    }

    infile.close();
    outfile.close();
    return true;
}




void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }

    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                                  {"sources"_p / "include1"_p,"sources"_p / "include2"_p})));

    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"s;

    assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
    setlocale(LC_ALL, "Russiаn");

    Test();
}