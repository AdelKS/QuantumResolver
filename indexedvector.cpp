#include "indexedvector.h"

template <class Name, class Object>
IndexedVector<Name, Object>::IndexedVector(bool bubble_sort_objects): sort(bubble_sort_objects)
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
int IndexedVector<Name, Object>::index_of(Name name)
{
    auto it = name_to_index.find(name);
    if( it != name_to_index.end())
        return it->second;
    else return npos;
}

template <class Name, class Object>
int IndexedVector<Name, Object>::new_object(Name name)
{
    // Pushes a new object at the end
    int i = objects.size();
    objects.push_back(Object(name));

    if(sort)
    {
        while(i > 1 and objects[i] < objects[i-1])
        {
            Object tmp = objects[i];
            objects[i] = objects[i-1];
            objects[i-1] = tmp;
        }
    }

    name_to_index[name] = i;
    return i;
}

template <class Name, class Object>
Object& IndexedVector<Name, Object>::operator [](int i)
{
    return objects[i];
}

template <class Name, class Object>
Object& IndexedVector<Name, Object>::back()
{
    return objects.back();
}

template <class Name, class Object>
int IndexedVector<Name, Object>::size()
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
