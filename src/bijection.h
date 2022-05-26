#pragma once

#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>

template <class Husband, class Wife> requires (! std::is_same_v<Husband, Wife>)
class Bijection
{
public:
    void add_couple(const Husband &h, const Wife &w)
    {
        husband_index[h] = couples.size();
        wife_index[w] = couples.size();

        couples.emplace_back(make_pair(h, w));
    }

    void emplace_couple(Husband &&h, Wife &&w)
    {
        husband_index[h] = couples.size();
        wife_index[w] = couples.size();

        couples.emplace_back(make_pair(std::forward(h), std::forward(w)));
    }

    Wife& get_counterpart(const Husband &h)
    {
        // Searchs for counterpart
        auto map_it = husband_index.find(h);
        if(map_it != husband_index.end())
            return couples[map_it->second].second;
        else
        {
            add_couple(h, Wife());
            return couples.back().second;
        }
    }

    Husband& get_counterpart(const Wife &w)
    {
        // Searchs for counterpart
        auto map_it = wife_index.find(w);
        if(map_it != wife_index.end())
            return couples[map_it->second].first;
        else
        {
            add_couple(Husband(), w);
            return couples.back().first;
        }
    }

    auto find_couple(const Husband &h) const
    {
        auto map_it = husband_index.find(h);
        if(map_it != husband_index.end())
            return couples.cbegin() + map_it->second;
        else return couples.cend();
    }

    auto find_couple(const Wife &w) const
    {
        auto map_it = wife_index.find(w);
        if(map_it != wife_index.end())
            return couples.cbegin() + map_it->second;
        else return couples.cend();
    }

    auto cbegin() const
    {
        return couples.cbegin();
    }

    auto cend() const
    {
        return couples.cend();
    }

    std::size_t size() const
    {
        return couples.size();
    }

protected:
    std::vector<std::pair<Husband, Wife>> couples;
    std::unordered_map<Husband, std::size_t> husband_index;
    std::unordered_map<Wife, std::size_t> wife_index;
};
