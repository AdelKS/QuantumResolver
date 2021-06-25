#include "stringindexedvector.h"

template <class T>
StringIndexedVector::StringIndexedVector()
{

}

template <class T>
void StringIndexedVector::push_back(T ob, string name)
{
    string_to_index[name] = objects.size();
    objects.push_back(ob);
}

template <class T>
T& StringIndexedVector::operator [](string name)
{
    auto it = string_to_index.find(name);
    if(it != string_to_index.end())
    {
        return *it;
    }
    else
    {
        new_object(name);
        return objects.back();
    }
}

template <class T>
int StringIndexedVector::index_of(string name)
{
    auto it = string_to_index.find(name);
    if( it != string_to_index.end())
        return *it;
    else return -1;
}

template <class T>
void StringIndexedVector::new_object(string name)
{
    push_back(T(objects.size(), name), name);
}

template <class T>
T& StringIndexedVector::operator [](int i)
{
    return objects[i];
}

template <class T>
T& StringIndexedVector::back()
{
    return objects.back();
}

template <class T>
int StringIndexedVector::size()
{
    return objects.size();
}

template <class T>
bool StringIndexedVector::contains(string str)
{
    return string_to_index.contains(str);
}

template <class T>
bool StringIndexedVector::empty()
{
    return objects.empty();
}
