#ifndef NAMED_VECTOR_H
#define NAMED_VECTOR_H

#include <vector>
#include <string>
#include <unordered_map>
#include <limits>

template <class Object>
class NamedVector
{
    // A vector that can either be accessed with vector[1] or vector[name], where name is a string

public:
    NamedVector();

    std::size_t size();
    bool empty();
    Object& back();
    auto begin();
    auto end();
    std::size_t id_of(const std::string_view &name) const;
    bool contains(std::string_view name);
    std::size_t push_back(Object object, std::string name);
    Object& operator [](const std::string_view &name);
    Object& operator [](std::size_t i);

    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:
    // taken from https://en.cppreference.com/w/cpp/container/unordered_map/find
    struct string_hash
    {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;

        std::size_t operator()(const char* str) const        { return hash_type{}(str); }
        std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
        std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
    };

    std::vector<Object> objects;
    std::unordered_map<std::string, std::size_t, string_hash, std::equal_to<>> name_to_index;
};

#include "named_vector.cpp"

#endif // NAMED_VECTOR_H
