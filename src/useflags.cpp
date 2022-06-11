#include "misc_utils.h"

#include "database.h"

#include <filesystem>
#include <chrono>
#include <string_view>

using namespace std::chrono;
namespace fs = std::filesystem;
using namespace std;

UseFlags::UseFlags(Database *db) : db(db)
{
    populate_profile_flags();
}

void UseFlags::set_arch()
{
    const auto& arch_flags = use_expand.keys_from_key<FlagID>(UseExpandName("ARCH"));
    for(const auto& flag: arch_flags)
        if(use.contains(flag))
        {
            current_arch = flag;
            current_arch_name = useflags.get_counterpart(current_arch);
            break;
        }
}

FlagInfo UseFlags::get_flag_info(const std::string_view& flag_str) const
{
    return get_flag_info(get_flag_id(flag_str));
}

FlagInfo UseFlags::get_flag_info(FlagID id) const
{
    if(id == npos)
        return FlagInfo();
    else return FlagInfo {use.contains(id),
                use_mask.contains(id), use_stable_mask.contains(id),
                use_force.contains(id), use_stable_force.contains(id)};
}

FlagID UseFlags::get_arch_id() const
{
    return current_arch;
}

FlagName UseFlags::get_arch_name() const
{
    return current_arch_name;
}

const std::unordered_set<FlagID>& UseFlags::get_implicit_flags() const
{
    return implicit_useflags;
}

const std::unordered_set<FlagID>& UseFlags::get_hidden_flags() const
{
    return hidden_useflags;
}

const std::unordered_set<FlagID>& UseFlags::get_expand_flags() const
{
    return expand_useflags;
}

const std::unordered_set<FlagID>& UseFlags::get_use() const
{
    return use;
}

const std::unordered_set<FlagID>& UseFlags::get_use_force() const
{
    return use_force;
}

const std::unordered_set<FlagID>& UseFlags::get_use_stable_force() const
{
    return use_stable_force;
}

const std::unordered_set<FlagID>& UseFlags::get_use_mask() const
{
    return use_mask;
}

const std::unordered_set<FlagID>& UseFlags::get_use_stable_mask() const
{
    return use_stable_mask;
}

size_t UseFlags::add_flag(const string_view &flag_str)
{
    auto it = useflags.find_couple(string(flag_str));
    if(it == useflags.cend())
    {
        useflags.add_couple(string(flag_str), useflags.size());
        FlagID flag_id = useflags.size() - 1;

        size_t underscore_pos = flag_str.find_last_of('_');
        if(underscore_pos != string_view::npos) // so we catch e.g. l18n_en
        {
            string_view prefix = flag_str.substr(0, underscore_pos); // We obtain here L18N

            // See if it corresponds to any use expand name
            ExpandID expand_id = use_expand.index_from_key(UseExpandName(prefix));

            if(expand_id != use_expand.npos)
            {
                // assign the flag id and string to the expand and mark the flag id as expanded
                use_expand.assign_key(expand_id, string(flag_str));
                use_expand.assign_key(expand_id, flag_id);
                expand_useflags.insert(flag_id);
            }
        }
        return  flag_id;
    }
    return it->second;
}

size_t UseFlags::get_flag_id(const std::string_view &flag_str) const
{
    auto it = useflags.find_couple(string(flag_str));
    if(it == useflags.cend())
    {
        return npos;
    }
    return it->second;
}

const std::string& UseFlags::get_flag_name(const size_t &id) const
{
    auto it = useflags.find_couple(id);
    if(it == useflags.cend())
        throw runtime_error(fmt::format("flag ID {} doesn't exist", id));

    return it->first;
}

void UseFlags::clear_globally_toggled_useflags()
{
    use.clear();
}

void UseFlags::clear_iuse_implicit_flags()
{
    throw runtime_error("Function not implemented");
}

void UseFlags::clear_hidden_expands()
{
    throw runtime_error("Function not implemented");
}

void UseFlags::clear_normal_expands()
{
    throw runtime_error("Function not implemented");
}

void UseFlags::clear_implicit_expands()
{
    throw runtime_error("Function not implemented");
}

void UseFlags::clear_unprefixed_expands()
{
    throw runtime_error("Function not implemented");
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
    throw runtime_error("Function not implemented");
}

void UseFlags::handle_use_line(const vector<string_view> &flags)
{
    for(string_view flag_str_v: flags)
    {
        if(flag_str_v.starts_with('$'))
        {
            cout << "skipping " << flag_str_v << " in USE var" << endl;
            continue;
        }

        if(flag_str_v == "-*")
            clear_globally_toggled_useflags();
        else if(flag_str_v.starts_with('-'))
        {
            flag_str_v.remove_prefix(1);
            use.erase(get_flag_id(flag_str_v));
        }
        else use.insert(add_flag(flag_str_v));
    }
}

void UseFlags::handle_iuse_implicit_line(const vector<string_view> &flags)
{
    for(string_view flag_str_v: flags)
    {
        if(flag_str_v.starts_with('$'))
        {
            cout << "skipping " << flag_str_v << " in USE var" << endl;
            continue;
        }

        if(flag_str_v == "-*")
            clear_iuse_implicit_flags();
        else if(flag_str_v.starts_with('-'))
        {
            flag_str_v.remove_prefix(1);
            implicit_useflags.erase(get_flag_id(flag_str_v));
        }
        else implicit_useflags.insert(add_flag(flag_str_v));
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

std::string UseFlags::get_expand_name_from_flag_id(FlagID flag_id) const
{
    ExpandID expand_id = use_expand.index_from_key(flag_id);
    return get_expand_name_from_expand_id(expand_id);
}

UseExpandType UseFlags::get_expand_type_from_flag_id(FlagID flag_id) const
{
    ExpandID expand_id = use_expand.index_from_key(flag_id);
    return get_expand_type_from_expand_id(expand_id);
}

std::string UseFlags::get_expand_name_from_expand_id(ExpandID expand_id) const
{
    if(use_expand.objects_count() <= expand_id)
        throw runtime_error(fmt::format("Expand ID {} does not exist", expand_id));

    return use_expand.keys_from_index<UseExpandName>(expand_id).begin()->name;
}

UseExpandType UseFlags::get_expand_type_from_expand_id(ExpandID expand_id) const
{
    if(use_expand.objects_count() <= expand_id)
        throw runtime_error(fmt::format("Expand ID {} does not exist", expand_id));

    return use_expand.const_object_from_index(expand_id);
}

void UseFlags::populate_profile_flags()
{

    for(const auto& profile_path : flatenned_profiles_tree)
    {
        const string profile_path_str = profile_path.string();
        fs::path make_path;
        if(fs::is_regular_file(profile_path_str + "/make.defaults"))
            make_path = fs::path(profile_path_str + "/make.defaults");
        if(fs::is_regular_file(profile_path_str + "/make.conf"))
        {
            if(not make_path.empty())
                throw std::logic_error("Found both a make.defaults and a make.conf in profile folder: " + profile_path_str);
            make_path = fs::path(profile_path_str + "/make.conf");
        }

        if(not fs::is_regular_file(make_path))
            continue;

//        cout<< "#############################" << endl;
//        cout << make_path.string() << endl;
//        cout << "+++++++++++++++ START OF FILE CONTENT +++++++++++++++" << endl;
//        for(const auto &line: read_file_lines(make_path))
//            cout << line << endl;
//        cout << "+++++++++++++++ END OF FILE CONTENT +++++++++++++++" << endl;

        for(auto& [var, values_line]: read_quoted_vars(make_path))
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
                handle_use_expand_line(var, words);
            else if(var == "IUSE_IMPLICIT")
                handle_iuse_implicit_line(words);
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

                    expand_useflags.insert(flag_id);

                    if(not found_use_expand_values)
                        use.insert(flag_id);

                    if(expand_type.implicit)
                        implicit_useflags.insert(flag_id);

                    if(expand_type.hidden)
                        hidden_useflags.insert(flag_id);
                }

            }

        }

    }

    cout << "Reading global forced and masked flags from profile tree" << endl;
    auto start = high_resolution_clock::now();

    vector<tuple<string, unordered_set<size_t>&>> profile_use_files_and_type =
    {
        {"use.force", use_force},
        {"use.stable.force", use_stable_force},
        {"use.mask", use_mask},
        {"use.stable.mask", use_stable_mask},
    };

    // Do global useflag overrides first
    for(const auto& profile_path : flatenned_profiles_tree)
        for(auto &[profile_use_file, container]: profile_use_files_and_type)
            for(const auto &path: get_regular_files(profile_path.string() + "/" + profile_use_file))
                for(const auto &[flag_id, state]: db->parser.parse_useflags(read_file_lines(path), true, true))
                {
                    if(state)
                        container.insert(flag_id);
                    else container.erase(flag_id);
                }

    auto end = high_resolution_clock::now();
    cout << "Read use expands and global useflags from profile tree in : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}

void UseFlags::compare_with_portage_eq()
{
    // THIS CODE IS TO COMPARE WITH the command `portageq envvar USE`.
    string global_useflags_str = exec("portageq envvar USE");
    if(global_useflags_str.ends_with('\n'))
        global_useflags_str.pop_back();

    std::set<string> portageeq_use;
    string_view global_useflags_view(global_useflags_str);
    while(not global_useflags_view.empty())
    {
        portageeq_use.insert(string(get_next_word(global_useflags_view)));
    }

    std::set<string> use_strs;
    for(size_t flag: use)
        use_strs.insert(get_flag_name(flag));

    std::vector<string> diff;

    std::ranges::set_difference(portageeq_use, use_strs,
                        std::back_inserter(diff));

    cout << "Portage EQ results not included in ours : " << endl;
    // these flags are actually only masked and we do not remove them
    for(const string& flag: diff)
        cout << flag << " ";
    cout << endl;
}
