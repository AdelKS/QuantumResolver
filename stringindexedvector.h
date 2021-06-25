#ifndef INDEXEDVECTOR_H
#define INDEXEDVECTOR_H

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

template <class T> class StringIndexedVector
{
public:
    StringIndexedVector();

    int size();
    bool empty();
    T& back();
    int index_of(string name);
    bool contains(string str);
    void new_object(string name);
    void push_back(T ob, string name);
    T& operator [](string name);
    T& operator [](int i);

protected:
    vector<T> objects;
    unordered_map<string, int> string_to_index;
};

#include "stringindexedvector.cpp"

#endif // INDEXEDVECTOR_H
