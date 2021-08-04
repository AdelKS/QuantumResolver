#ifndef INDEXEDVECTOR_H
#define INDEXEDVECTOR_H

#include <vector>
#include <string>
#include <unordered_map>

template <class Name, class Object, bool sort>
class IndexedVector
{
    // A vector that can either be accessed with vector[1] or vector[name], where name is an instance of Name
    // sort indicates whether or not each element in IndexedVector needs to be sorted according the Object's operator <

public:
    IndexedVector();

    int size();
    bool empty();
    Object& back();
    int index_of(Name name);
    bool contains(Name name);
    virtual int new_object(Name name);
    void push_back(Object ob, Name name);
    Object& operator [](Name name);
    Object& operator [](int i);

    static const int npos = -1;

protected:
    std::vector<Object> objects;
    std::unordered_map<Name, int> name_to_index;
};

#include "indexedvector.cpp"

#endif // INDEXEDVECTOR_H
