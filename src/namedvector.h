#ifndef NAMEDVECTOR_H
#define NAMEDVECTOR_H

#include <vector>
#include <string>
#include <unordered_map>

template <class Object>
class NamedVector
{
    // A vector that can either be accessed with vector[1] or vector[name], where name is a string

public:
    NamedVector();

    size_t size();
    bool empty();
    Object& back();
    auto begin();
    auto end();
    size_t id_of(const std::string_view &name) const;
    bool contains(std::string_view name);
    size_t push_back(const Object &object, const std::string_view &name);
    size_t emplace_back(const Object&& object, const std::string_view &name);
    Object& operator [](const std::string_view &name);
    Object& operator [](size_t i);

    static const size_t npos = -1;

protected:
    // taken from https://en.cppreference.com/w/cpp/container/unordered_map/find
    struct string_hash
    {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;

        size_t operator()(const char* str) const        { return hash_type{}(str); }
        size_t operator()(std::string_view str) const   { return hash_type{}(str); }
        size_t operator()(std::string const& str) const { return hash_type{}(str); }
    };

    std::vector<Object> objects;
    std::unordered_map<std::string, size_t, string_hash, std::equal_to<>> name_to_index;
};

#include "namedvector.cpp"

#endif // NAMEDVECTOR_H
