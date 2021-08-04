#include "indexedvector.h"

using namespace std;

template <class Name, class Object, bool sort>
IndexedVector<Name, Object, sort>::IndexedVector()
{

}

template <class Name, class Object, bool sort>
Object& IndexedVector<Name, Object, sort>::operator [](Name name)
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

template <class Name, class Object, bool sort>
int IndexedVector<Name, Object, sort>::index_of(Name name)
{
    auto it = name_to_index.find(name);
    if( it != name_to_index.end())
        return it->second;
    else return npos;
}

template <class Name, class Object, bool sort>
int IndexedVector<Name, Object, sort>::new_object(Name name)
{
    // Push new object to back
    int i = objects.size();
    objects.push_back(Object(name));

    // this if condition is computed at compile-time
    if constexpr (sort)
    {
        // when sort is true
        // buble sort the object to its final position        

        while(i > 1 and objects[i] < objects[i-1])
        {
            Object tmp = objects[i];
            objects[i] = objects[i-1];
            objects[i-1] = tmp;
            --i;
        }
    }

    name_to_index[name] = i;
    return i;
}

template <class Name, class Object, bool sort>
Object& IndexedVector<Name, Object, sort>::operator [](int i)
{
    return objects[i];
}

template <class Name, class Object, bool sort>
Object& IndexedVector<Name, Object, sort>::back()
{
    return objects.back();
}

template <class Name, class Object, bool sort>
int IndexedVector<Name, Object, sort>::size()
{
    return objects.size();
}

template <class Name, class Object, bool sort>
bool IndexedVector<Name, Object, sort>::contains(Name str)
{
    return name_to_index.contains(str);
}

template <class Name, class Object, bool sort>
bool IndexedVector<Name, Object, sort>::empty()
{
    return objects.empty();
}
