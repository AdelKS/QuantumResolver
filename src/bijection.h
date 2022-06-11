#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>

#include "concepts.h"

template <class Husband, class Wife> requires (! std::is_same_v<Husband, Wife>)
class Bijection
{
public:
    void add_couple(Husband h, Wife w)
    {
        husband_index[h] = couples.size();
        wife_index[w] = couples.size();

        couples.emplace_back(std::move(h), std::move(w));
    }

    template <class Member> requires (is_any_of<Member, Wife, Husband>)
    auto& get_counterpart(const Member &m)
    {
        // Searches for counterpart
        auto couple_it = find_couple(m);
        if(couple_it == couples.end())
        {
            if constexpr (std::is_same_v<Member, Husband>)
                add_couple(m, Wife());
            else add_couple(Husband(), m);
            couple_it = --couples.end();
        }

        if constexpr (std::is_same_v<Member, Husband>)
            return couple_it->second;
        else return couple_it->first;
    }

    template <class Member> requires (is_any_of<Member, Wife, Husband>)
    auto find_couple(const Member &m) const
    {
        auto map_it = get_map<Member>().find(m);
        if(map_it != get_map<Member>().end())
        {
            auto couple_it = couples.begin();
            std::advance(couple_it, map_it->second);
            return couple_it;
        }
        else return couples.end();
    }

    auto cbegin() const { return couples.cbegin(); }

    auto cend() const { return couples.cend(); }

    std::size_t size() const { return couples.size(); }

protected:
    template <class Member> requires (is_any_of<Member, Wife, Husband>)
    auto& get_map() const
    {
        if constexpr (std::is_same_v<Member, Husband>)
            return husband_index;
        else return wife_index;
    }

    std::vector<std::pair<Husband, Wife>> couples;
    std::unordered_map<Husband, size_t> husband_index;
    std::unordered_map<Wife, size_t> wife_index;
};
