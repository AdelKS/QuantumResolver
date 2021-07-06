#ifndef INDEXEDVECTOR_H
#define INDEXEDVECTOR_H

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

template <class Name, class Object> class IndexedVector
{
public:
    IndexedVector(bool bubble_sort_objects = false);

    int size();
    bool empty();
    Object& back();
    int index_of(Name name);
    bool contains(Name name);
    int new_object(Name name);
    void push_back(Object ob, Name name);
    Object& operator [](Name name);
    Object& operator [](int i);

    static const int npos = -1;

protected:
    bool sort;
    vector<Object> objects;
    unordered_map<Name, int> name_to_index;
};

#include "indexedvector.cpp"

#endif // INDEXEDVECTOR_H
