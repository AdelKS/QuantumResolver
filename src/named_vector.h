#ifndef NAMED_VECTOR_H
#define NAMED_VECTOR_H

#include <vector>
#include <string>
#include <unordered_map>
#include <limits>
#include <stdexcept>

template <class Object>
class NamedVector
{
    // A vector that can either be accessed with vector[1] or vector[name], where name is a string

public:
    typedef typename std::vector<Object>::iterator iterator;
    typedef typename std::vector<Object>::const_iterator const_iterator;

    NamedVector() {};

    std::size_t size() const {return objects.size();};
    bool empty() {return objects.back();};
    Object& back() {return objects.back();};
    iterator begin() {return objects.begin();};
    iterator end() {return objects.end();};
    const_iterator cbegin() const {return objects.cbegin();} ;
    const_iterator cend() const {return objects.cend();};

    std::size_t id_of(const std::string_view &name) const
    {
        auto it = name_to_index.find(name);
        if( it != name_to_index.end())
            return it->second;
        return npos;
    }

    bool contains(std::string_view name)
    {
        return name_to_index.contains(name);
    }

    std::size_t push_back(Object object, std::string name)
    {
        std::size_t index = objects.size();
        objects.push_back(std::move(object));
        objects.back().set_id(index);

        name_to_index[std::move(name)] = index;
        return index;
    }

    Object& operator [](const std::string_view &name)
    {
        auto it = name_to_index.find(name);
        if(it != name_to_index.end())
        {
            return objects[it->second];
        }
        else throw std::runtime_error("Accessing an unexisting object name: " + std::string(name));
    }

    Object& operator [](std::size_t i)
    {
        return objects[i];
    }

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

#endif // NAMED_VECTOR_H
