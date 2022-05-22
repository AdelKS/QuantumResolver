#include <stdexcept>
#include <iostream>

#include "named_vector.h"

using namespace std;

template <class Object>
NamedVector<Object>::NamedVector()
{

}

template <class Object>
Object& NamedVector<Object>::operator [](const std::string_view &name)
{
    auto it = name_to_index.find(name);
    if(it != name_to_index.end())
    {
        return objects[it->second];
    }
    else throw runtime_error("Accessing an unexisting object name: " + string(name));
}

template <class Object>
auto NamedVector<Object>::begin()
{
    return objects.begin();
}

template <class Object>
auto NamedVector<Object>::end()
{
    return objects.end();
}

template <class Object>
size_t NamedVector<Object>::id_of(const string_view &name) const
{
    auto it = name_to_index.find(name);
    if( it != name_to_index.end())
        return it->second;
    return npos;
}

template <class Object>
size_t NamedVector<Object>::push_back(Object object, string name)
{
    size_t index = objects.size();
    objects.push_back(std::move(object));
    objects.back().set_id(index);

    name_to_index[std::move(name)] = index;
    return index;
}

template <class Object>
Object& NamedVector<Object>::operator [](size_t i)
{
    return objects[i];
}

template <class Object>
Object& NamedVector<Object>::back()
{
    return objects.back();
}

template <class Object>
size_t NamedVector<Object>::size()
{
    return objects.size();
}

template <class Object>
bool NamedVector<Object>::contains(string_view str)
{
    return name_to_index.contains(str);
}

template <class Object>
bool NamedVector<Object>::empty()
{
    return objects.empty();
}
