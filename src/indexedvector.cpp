#include "indexedvector.h"

using namespace std;

template <class Name, class Object>
IndexedVector<Name, Object>::IndexedVector()
{

}

template <class Name, class Object>
Object& IndexedVector<Name, Object>::operator [](Name name)
{
    auto it = name_to_index.find(name);
    if(it != name_to_index.end())
    {
        return objects[it->second];
    }
    else
    {
        new_object(name);
        return objects.back();
    }
}

template <class Name, class Object>
auto IndexedVector<Name, Object>::begin()
{
    return objects.begin();
}

template <class Name, class Object>
auto IndexedVector<Name, Object>::end()
{
    return objects.end();
}

template <class Name, class Object>
size_t IndexedVector<Name, Object>::index_of(Name name) const
{
    auto it = name_to_index.find(name);
    if( it != name_to_index.end())
        return it->second;
    else return npos;
}

template <class Name, class Object>
size_t IndexedVector<Name, Object>::new_object(Name name)
{
    // Push new object to back
    size_t index = objects.size();
    objects.push_back(Object(name));
    objects.back().set_id(index);

    name_to_index[name] = index;
    return index;
}

template <class Name, class Object>
Object& IndexedVector<Name, Object>::operator [](size_t i)
{
    return objects[i];
}

template <class Name, class Object>
Object& IndexedVector<Name, Object>::back()
{
    return objects.back();
}

template <class Name, class Object>
size_t IndexedVector<Name, Object>::size()
{
    return objects.size();
}

template <class Name, class Object>
bool IndexedVector<Name, Object>::contains(Name str)
{
    return name_to_index.contains(str);
}

template <class Name, class Object>
bool IndexedVector<Name, Object>::empty()
{
    return objects.empty();
}
