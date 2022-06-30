#include "file_utils.h"
#include "src/string_utils.h"

#include <fstream>
#include <iostream>

using namespace std;
namespace fs = filesystem;

void print_file_contents(const fs::path& path)
{
    cout << "#############################" << endl;
    cout << path.string() << endl;
    cout << "+++++++++++++++ START OF FILE CONTENT +++++++++++++++" << endl;
    for(const auto &line: read_file_lines(path))
        cout << line << endl;
    cout << "+++++++++++++++ END OF FILE CONTENT +++++++++++++++" << endl;
}

vector<fs::path> get_regular_files(const fs::path &path)
{
    vector<fs::path> regular_files;
    if(fs::is_regular_file(path))
        regular_files.push_back(path);
    else if(fs::is_directory(path))
        for(const auto &file: fs::directory_iterator(path))
            if(fs::is_regular_file(file))
                regular_files.push_back(file);

    return regular_files;
}

vector<string> read_file_lines(const filesystem::path& file_path)
{
    /* Reads the lines of the file referenced by file_path
     * and returns them.
     * Note: Spaces at the beginning and end of each line are removed
     * */
    fstream file(file_path, ios::in);
    if(not file.is_open())
        throw runtime_error("Couldn't open parent file" + file_path.string());

    char line[LINE_MAX_SIZE];
    vector<string> file_lines;

    while(file.getline(line, LINE_MAX_SIZE, '\n'))
    {
        string_view view(line);
        skim_spaces_at_the_edges(view);
        if(view.starts_with("#") or view.empty())
            continue;

        file_lines.emplace_back(view);
    }

    return file_lines;
}

template <bool quoted, class... StartWithContiner> requires (sizeof...(StartWithContiner) == 0 or
                                                              (sizeof...(StartWithContiner) == 1 and
                                                                 (std::is_same_v<StartWithContiner, vector<string>> and ...)))
auto read_vars(const filesystem::path& file_path,
                                       const StartWithContiner& ... start_with)
{
    auto get_return_type = []()
    {
        if constexpr (sizeof...(StartWithContiner) == 1)
            return unordered_map<string, string>();
        else return vector<pair<string, string>>();
    };

    auto vars = get_return_type();

    auto emplace_result = [&vars](string v1, string v2)
    {
        if constexpr (sizeof...(StartWithContiner) == 1)
            vars.emplace(move(v1), move(v2));
        else vars.emplace_back(move(v1), move(v2));
    };

    const auto &file_lines = read_file_lines(file_path);
    for(const string &line: file_lines)
    {
        size_t eq_sign_pos = line.find('=');

        if(eq_sign_pos == string_view::npos)
            continue;

        if constexpr (sizeof...(StartWithContiner) == 1)
        {
            auto get_start_with_list = []<class... Elems>(const vector<string> &vec) {return vec;};
            const vector<string>& start_with_list = get_start_with_list(start_with...);
            bool found = false;
            for(size_t i = 0 ; i < start_with_list.size() and not found ; i++)
                found = line.starts_with(start_with_list[i]);

            if(not found)
                continue;
        }

        if constexpr (quoted)
        {
            if(not (eq_sign_pos < line.size() - 2 and // to make sure that the line doesn't end with ="
            line[eq_sign_pos+1] == '"' and
            line.back() == '"'))
                throw runtime_error("File " + file_path.string() + " has problems in its variable definition");
        }

        emplace_result
        (
            line.substr(0, eq_sign_pos),
            line.substr(eq_sign_pos + 1 + quoted, line.size() - 1 - eq_sign_pos - 2*quoted)
        );
    }

    return vars;
}

unordered_map<string, string> read_quoted_vars(const filesystem::path& file_path,
                                              const vector<string>& start_with_list)
{
    return read_vars<true>(file_path, start_with_list);
}

unordered_map<string, string> read_unquoted_vars(const filesystem::path& file_path,
                                              const vector<string>& start_with_list)
{
    return read_vars<false>(file_path, start_with_list);
}

vector<pair<string, string>> read_quoted_vars(const filesystem::path& file_path)
{
    return read_vars<true>(file_path);
}

vector<pair<string, string>> read_unquoted_vars(const filesystem::path& file_path)
{
    return read_vars<false>(file_path);
}

vector<fs::path> get_profiles_tree()
{
    vector<fs::path> profile_tree = {"/etc/portage/profile", "/etc/portage"};

    fs::path profile_symlink("/etc/portage/make.profile");
    if(not fs::is_symlink(profile_symlink))
        throw runtime_error("/etc/portage/make.profile doesn't exist or isn't a symlink");

    fs::path profile = fs::canonical(profile_symlink);

    vector<fs::path> current_depth_profiles = {profile};
    vector<fs::path> next_depth_profiles;

    while(not current_depth_profiles.empty())
    {
        profile_tree.insert(profile_tree.begin(), current_depth_profiles.cbegin(), current_depth_profiles.cend());
        for(const auto& curr_profile: current_depth_profiles)
        {
            fs::path parent_file_path(curr_profile.string() + "/parent");
            if(not fs::is_regular_file(parent_file_path))
                continue;

            for(const string &line: read_file_lines(parent_file_path))
            {
                auto new_profile = fs::canonical(fs::path(curr_profile.string() + "/" + line + "/"));
                next_depth_profiles.push_back(new_profile);
            }
        }
        std::swap(current_depth_profiles, next_depth_profiles);
        next_depth_profiles.clear();
    }

    return profile_tree;
}

const std::vector<std::filesystem::path> flatenned_profiles_tree = get_profiles_tree();
