#include "misc_utils.h"

#include "database.h"

#include <filesystem>
#include <chrono>

using namespace std::chrono;
namespace fs = filesystem;
using namespace std;

UseFlags::UseFlags(Database *db) : db(db)
{
    populate_profile_flags();
}

size_t UseFlags::add_flag(const string_view &flag_str)
{
    auto it = useflags.find_couple(string(flag_str));
    if(it == useflags.cend())
    {
        useflags.add_couple(string(flag_str), useflags.size());
        return  useflags.size() - 1;
    }
    return it->second;
}

size_t UseFlags::get_flag_id(const std::string_view &flag_str)
{
    auto it = useflags.find_couple(string(flag_str));
    if(it == useflags.cend())
    {
        return npos;
    }
    return it->second;
}

std::string UseFlags::get_flag_name(const size_t &id)
{
    auto it = useflags.find_couple(id);
    if(it == useflags.cend())
    {
        return "";
    }
    return it->first;
}

void UseFlags::set_global_useflag(const std::string_view& flag_str)
{
    use.insert(add_flag(flag_str));
}

void UseFlags::unset_global_useflag(const std::string_view& flag_str)
{
    use.erase(get_flag_id(flag_str));
}

void UseFlags::unset_global_useflags()
{
    use.clear();
}

void UseFlags::clear_hidden_expands()
{
    // TODO : Continue here
}

void UseFlags::clear_normal_expands()
{
    // TODO : Continue here
}

void UseFlags::clear_implicit_expands()
{
    // TODO : Continue here
}

void UseFlags::clear_unprefixed_expands()
{
    // TODO : Continue here
}

void UseFlags::make_expand_hidden(std::size_t prefix_index, bool hidden)
{
    use_expand.object_from_index(prefix_index).hidden = hidden;
}

void UseFlags::make_expand_implicit(std::size_t prefix_index, bool implicit)
{
    use_expand.object_from_index(prefix_index).implicit = implicit;
}

void UseFlags::remove_expand(std::size_t prefix_index)
{
    // TODO : Continue here
}

void UseFlags::handle_use_line(const vector<string_view> &words)
{
    for(string_view value: words)
    {
        if(value.starts_with('$'))
        {
            cout << "skipping " << value << " in USE var" << endl;
            continue;
        }

        if(value == "-*")
            unset_global_useflags();
        else if(value.starts_with('-'))
        {
            value.remove_prefix(1);
            unset_global_useflag(value);
        }
        else set_global_useflag(value);
    }
}

void UseFlags::handle_use_expand_line(string_view use_expand_type, const vector<string_view> &words)
{
    for(string_view value: words)
    {
        if(value.starts_with('$'))
        {
            cout << "skipping " << value << " in var: " << use_expand_type << endl;
            continue;
        }

        bool clear_list = value == "-*";
        if(clear_list)
        {
            if(use_expand_type == "USE_EXPAND_UNPREFIXED")
                clear_unprefixed_expands();
            else if(use_expand_type == "USE_EXPAND_IMPLICIT")
                clear_implicit_expands();
            else if(use_expand_type == "USE_EXPAND")
                clear_normal_expands();
            else // "USE_EXPAND_HIDDEN"
                clear_hidden_expands();
            continue;
        }

        bool remove_value = value.starts_with('-');
        if(remove_value)
            value.remove_prefix(1);

        UseExpandName prefix_str(value);
        size_t prefix_index = use_expand.index_from_key(prefix_str);
        if(prefix_index == use_expand.npos)
        {
            if(remove_value)
                throw string("removing a prefix that doesn't exist: ") + string(value);
            prefix_index = use_expand.push_back(UseExpandType());
            use_expand.assign_key(prefix_index, prefix_str);
        }

        assert(not remove_value or use_expand_type == "USE_EXPAND_HIDDEN"); // Removing use expands hasn't been implemented for now

        UseExpandType& use_exp = use_expand.object_from_index(prefix_index);

        if(use_expand_type == "USE_EXPAND_UNPREFIXED")
            use_exp.unprefixed = true;
        else if(use_expand_type == "USE_EXPAND_IMPLICIT")
            use_exp.implicit = true;
        else if (use_expand_type == "USE_EXPAND_HIDDEN")
            use_exp.hidden = not remove_value;
    }
}

void UseFlags::populate_profile_flags()
{
    const std::vector<std::filesystem::path> &profile_tree = get_profiles_tree();

    for(auto it = profile_tree.rbegin() ; it != profile_tree.rend() ; it++)
    {
        fs::path make_path;
        if(fs::is_regular_file(it->string() + "/make.defaults"))
            make_path = fs::path(it->string() + "/make.defaults");
        if(fs::is_regular_file(it->string() + "/make.conf"))
        {
            if(not make_path.empty())
                throw std::logic_error("Found both a make.defaults and a make.conf in profile folder: " + it->string());
            make_path = fs::path(it->string() + "/make.conf");
        }

        if(fs::is_regular_file(make_path))
        {
            cout<< "#############################" << endl;
            cout << make_path.string() << endl;
            cout << "+++++++++++++++ START OF FILE CONTENT +++++++++++++++" << endl;
            for(const auto &line: read_file_lines(make_path))
                cout << line << endl;
            cout << "+++++++++++++++ END OF FILE CONTENT +++++++++++++++" << endl;

            for(auto &[var, values_line]: read_vars(make_path))
            {
                vector<string_view> words;
                {
//                    cout << "------------" << endl;
//                    cout << "Variable: " << var << endl;
//                    cout << "Values: " << endl;
                    string_view values_line_view(values_line);
                    while(not values_line_view.empty())
                    {
                        words.push_back(get_next_word(values_line_view));
//                        cout << words.back() << " ; ";
                    }
//                    cout << endl;
                }

                if(var == "USE")
                    handle_use_line(words);
                else if(var == "USE_EXPAND_UNPREFIXED" or
                        var == "USE_EXPAND_IMPLICIT" or
                        var == "USE_EXPAND" or
                        var == "USE_EXPAND_HIDDEN")
                {
                    handle_use_expand_line(var, words);
                }
                else
                {
                    size_t expand_index = use_expand.npos;
                    bool found_use_expand_values = var.starts_with("USE_EXPAND_VALUES_");
                    if(found_use_expand_values)
                        var = var.substr(string("USE_EXPAND_VALUES_").size());

                    expand_index = use_expand.index_from_key(UseExpandName(var));

                    if(expand_index == use_expand.npos)
                    {
                        if(found_use_expand_values)
                            throw runtime_error("Found USE_EXPAND_VALUES_" + var + " without prior definition of " + var);
                        continue;
                    }

                    const UseExpandType expand_type = use_expand.object_from_index(expand_index);
                    for(string_view& use_expand_flag_tail: words)
                    {
                        if(use_expand_flag_tail.starts_with('-'))
                            throw runtime_error("Wrongly using " + var + " as incremental variable as it contains " + string(use_expand_flag_tail));

                        string use_flag_str;
                        if(expand_type.unprefixed)
                            use_flag_str = use_expand_flag_tail;
                        else use_flag_str = to_lower(var) + "_" + string(use_expand_flag_tail);

                        size_t flag_id = add_flag(use_flag_str);
                        use_expand.assign_key(expand_index, flag_id);
                        use_expand.assign_key(expand_index, use_flag_str);
                        if(not found_use_expand_values)
                            use.insert(flag_id);
                    }

                }

            }
        }
    }

    const auto& arch_flags = use_expand.keys_from_key<FlagName>(UseExpandName("ARCH"));
    // TODO: Set current arch

    cout << "Reading global forced and masked flags from profile tree" << endl;
    auto start = high_resolution_clock::now();

    vector<tuple<string, FlagAssignType, unordered_set<size_t>&>> profile_use_files_and_type =
    {
        {"use.force", FlagAssignType::FORCE, use_force},
        {"use.stable.force", FlagAssignType::STABLE_FORCE, use_stable_force},
        {"use.mask", FlagAssignType::MASK, use_mask},
        {"use.stable.mask", FlagAssignType::STABLE_MASK, use_stable_mask},
    };

    // Do global useflag overrides first
    for(auto it = profile_tree.rbegin() ; it != profile_tree.rend() ; it++)
        for(auto &[profile_use_file, use_type, container]: profile_use_files_and_type)
            for(const auto &path: get_regular_files(it->string() + "/" + profile_use_file))
                for(const auto &[flag_id, state]: db->parser.parse_useflags(read_file_lines(path), true, true))
                {
                    if(state)
                        container.insert(flag_id);
                    else container.erase(flag_id);
                }

// THIS CODE IS TO COMPARE WITH the command `portageq envvar USE`.
//    string global_useflags_str = exec("portageq envvar USE");
//    if(global_useflags_str.ends_with('\n'))
//        global_useflags_str.pop_back();

//    std::set<string> portageeq_use;
//    string_view global_useflags_view(global_useflags_str);
//    while(not global_useflags_view.empty())
//    {
//        portageeq_use.insert(string(get_next_word(global_useflags_view)));
//    }

//    std::set<string> use_strs;
//    for(size_t flag: use)
//        use_strs.insert(get_flag_name(flag));

//    std::vector<string> diff;

//    std::ranges::set_difference(portageeq_use, use_strs,
//                        std::back_inserter(diff));

//    cout << "Portage EQ results not included in ours : " << endl;
//    for(const string& flag: diff)
//        cout << flag << " ";
//    cout << endl;

    auto end = high_resolution_clock::now();
    cout << "Read use expands and global useflags from profile tree in : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}
