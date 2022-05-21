#ifndef MULTIKEYMAP_H
#define MULTIKEYMAP_H

#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cassert>
#include <limits>

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
class MultiKeyMap
{
public:
    std::size_t push_back(Object obj)
    {
        container.emplace(container.size(), std::move(obj));
        return container.size()-1;
    }

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    void assign_key(std::size_t object_index, AnyKey key);

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    Object& get_from_key(const AnyKey& key);

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    std::size_t index_from_key(AnyKey key);

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    bool contains_key(AnyKey key);

    Object& get_from_index(std::size_t index);

    std::size_t objects_count();

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    std::size_t keys_count();

    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:

    std::unordered_map<std::size_t, Object> container;
    std::tuple<std::unordered_map<KeyType, std::size_t>...> maps;
    std::tuple<std::unordered_map<std::size_t, std::unordered_set<KeyType>>...> rev_maps;

};

// ######################### IMPLEMENTATION ########################################

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template <class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
std::size_t MultiKeyMap<Object, KeyType...>::index_from_key(AnyKey key)
{
    std::size_t object_index = npos;

    auto update_lookedup_object_from_key = [&]<std::size_t I>()
    {
        if constexpr (std::is_same_v<AnyKey, typename std::tuple_element_t<I, decltype (maps)>::key_type>)
        {
            const auto &map = std::get<I>(maps);
            const auto &it = map.find(key);
            if(it != map.end())
                object_index = it->second;
        }
    };

    auto recurse_set = [&]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        (update_lookedup_object_from_key.template operator()<Is>(),...);
    };

    recurse_set.template operator()<>(std::make_index_sequence<sizeof...(KeyType)>{});

    return object_index;
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template <class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
Object& MultiKeyMap<Object, KeyType...>::get_from_key(const AnyKey& key)
{
    std::size_t object_index = index_from_key(key);

    assert(object_index != npos);

    return container[object_index];
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template <class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
void MultiKeyMap<Object, KeyType...>::assign_key(std::size_t object_index, AnyKey key)
{
    assert(object_index < objects_count());

    auto update_object_index_at_key = [&]<std::size_t I>()
    {
        if constexpr (std::is_same_v<AnyKey, typename std::tuple_element_t<I, decltype (maps)>::key_type>)
        {
            auto &map = std::get<I>(maps);
            auto &rev_map = std::get<I>(rev_maps);

            rev_map[object_index].insert(key);
            map.emplace(std::move(key), std::move(object_index));
        }
    };

    auto recurse_update = [&]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        (update_object_index_at_key.template operator()<Is>(),...);
    };

    recurse_update.template operator()<>(std::make_index_sequence<sizeof...(KeyType)>{});
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
bool MultiKeyMap<Object, KeyType...>::contains_key(AnyKey key)
{
    std::size_t object_index = index_from_key(key);

    return object_index != npos;
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
Object& MultiKeyMap<Object, KeyType...>::get_from_index(std::size_t index)
{
    assert(index < container.size());
    return container[index];
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
std::size_t MultiKeyMap<Object, KeyType...>::keys_count()
{
    std::size_t count = 0;

    auto update_lookedup_object_from_key = [&]<std::size_t I>()
    {
        if constexpr (std::is_same_v<AnyKey, typename std::tuple_element_t<I, decltype (maps)>::key_type>)
        {
            const auto &map = std::get<I>(maps);
            count = map.size();
        }
    };

    auto recurse_set = [&]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        (update_lookedup_object_from_key.template operator()<Is>(),...);
    };

    return count;
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
std::size_t MultiKeyMap<Object, KeyType...>::objects_count()
{
    return container.size();
}

#endif // MULTIKEYMAP_H
