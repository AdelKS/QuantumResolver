#include "stringindexedvector.h"

template <class T>
StringIndexedVector<T>::StringIndexedVector()
{

}

template <class T>
void StringIndexedVector<T>::push_back(T ob, string name)
{
    string_to_index[name] = objects.size();
    objects.push_back(ob);
}

template <class T>
T& StringIndexedVector<T>::operator [](string name)
{
    auto it = string_to_index.find(name);
    if(it != string_to_index.end())
    {
        return objects[it->second];
    }
    else
    {
        new_object(name);
        return objects.back();
    }
}

template <class T>
int StringIndexedVector<T>::index_of(string name)
{
    auto it = string_to_index.find(name);
    if( it != string_to_index.end())
        return it->second;
    else return -1;
}

template <class T>
void StringIndexedVector<T>::new_object(string name)
{
    push_back(T(objects.size(), name), name);
}

template <class T>
T& StringIndexedVector<T>::operator [](int i)
{
    return objects[i];
}

template <class T>
T& StringIndexedVector<T>::back()
{
    return objects.back();
}

template <class T>
int StringIndexedVector<T>::size()
{
    return objects.size();
}

template <class T>
bool StringIndexedVector<T>::contains(string str)
{
    return string_to_index.contains(str);
}

template <class T>
bool StringIndexedVector<T>::empty()
{
    return objects.empty();
}
