#ifndef MULTIKEY_MAP_H
#define MULTIKEY_MAP_H

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
    Object& object_from_key(const AnyKey& key);

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    std::size_t index_from_key(const AnyKey& key);

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    bool contains_key(const AnyKey& key);

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    const std::unordered_set<AnyKey>& keys_from_index(std::size_t index);

    template<class AnyKey1, class AnyKey2>
    requires ((std::is_same_v<AnyKey1, KeyType> || ...) and (std::is_same_v<AnyKey2, KeyType> || ...))
    const std::unordered_set<AnyKey1>& keys_from_key(const AnyKey2& key);

    Object& object_from_index(std::size_t index);

    std::size_t objects_count();

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    std::size_t keys_count();

    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:

    template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
    static constexpr std::size_t index_of_type();

    std::unordered_map<std::size_t, Object> container;
    std::tuple<std::unordered_map<KeyType, std::size_t>...> maps;
    std::tuple<std::unordered_map<std::size_t, std::unordered_set<KeyType>>...> rev_maps;

};

// ######################### IMPLEMENTATION ########################################

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
constexpr std::size_t MultiKeyMap<Object, KeyType...>::index_of_type()
{
    auto is_correct_tuple_index = []<std::size_t I>()
    {
        if constexpr (std::is_same_v<AnyKey, typename std::tuple_element_t<I, decltype (maps)>::key_type>)
        {
            return I;
        }
        else return 0;
    };

    auto recurse_sum = [&is_correct_tuple_index]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        constexpr std::size_t index = (is_correct_tuple_index.template operator()<Is>() + ...);
        return index;
    };

    return recurse_sum.template operator()<>(std::make_index_sequence<sizeof...(KeyType)>{});
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template <class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
std::size_t MultiKeyMap<Object, KeyType...>::index_from_key(const AnyKey& key)
{
    const auto &map = std::get<index_of_type<AnyKey>()>(maps);
    const auto &it = map.find(key);
    if(it != map.end())
        return it->second;
    else return npos;
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template<class AnyKey1, class AnyKey2>
requires ((std::is_same_v<AnyKey1, KeyType> || ...) and (std::is_same_v<AnyKey2, KeyType> || ...))
const std::unordered_set<AnyKey1>& MultiKeyMap<Object, KeyType...>::keys_from_key(const AnyKey2 &key)
{
    std::size_t index = index_from_key(key);

    assert(index != npos);

    return std::get<index_of_type<AnyKey1>()>(rev_maps)[index];
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template <class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
const std::unordered_set<AnyKey>& MultiKeyMap<Object, KeyType...>::keys_from_index(std::size_t index)
{
    return std::get<index_of_type<AnyKey>()>(rev_maps)[index];
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template <class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
Object& MultiKeyMap<Object, KeyType...>::object_from_key(const AnyKey& key)
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

    auto &map = std::get<index_of_type<AnyKey>()>(maps);
    auto &rev_map = std::get<index_of_type<AnyKey>()>(rev_maps);

    rev_map[object_index].insert(key);
    map.emplace(std::move(key), std::move(object_index));
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
bool MultiKeyMap<Object, KeyType...>::contains_key(const AnyKey& key)
{
    std::size_t object_index = index_from_key(key);

    return object_index != npos;
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
Object& MultiKeyMap<Object, KeyType...>::object_from_index(std::size_t index)
{
    assert(index < container.size());
    return container[index];
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
template<class AnyKey> requires (std::is_same_v<AnyKey, KeyType> || ...)
std::size_t MultiKeyMap<Object, KeyType...>::keys_count()
{
    return std::get<index_of_type<AnyKey>()>(maps).size();
}

template <class Object, class... KeyType> requires (sizeof...(KeyType) > 0)
std::size_t MultiKeyMap<Object, KeyType...>::objects_count()
{
    return container.size();
}

#endif // MULTIKEY_MAP_H
