#include "misc_utils.h"

using namespace std;

unordered_set<size_t> operator + (const unordered_set<size_t>& a,
                                            const unordered_set<size_t>& b)
{
    unordered_set<size_t> ret(a);
    ret.insert(b.begin(), b.end());
    return ret;
}

unordered_set<size_t> operator + (unordered_set<size_t>&& a,
                                            const unordered_set<size_t>& b)
{
    a.insert(b.begin(), b.end());
    return a;
}

std::unordered_set<std::size_t>& operator += (std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b)
{
    a.insert(b.begin(), b.end());
    return a;
}

unordered_set<size_t> operator & (const unordered_set<size_t>& a,
                                            const unordered_set<size_t>& b)
{
    const unordered_set<size_t>& smaller = a.size() <= b.size() ? a : b;
    const unordered_set<size_t>& bigger = a.size() > b.size() ? a : b;

    unordered_set<size_t> ret;
    for(size_t val: smaller)
        if(bigger.contains(val))
            ret.insert(val);

    return ret;
}

unordered_set<size_t> operator & (unordered_set<size_t>&& a,
                                            const unordered_set<size_t>& b)
{
    for(auto it = a.begin(); it != a.end();)
    {
       if(not b.contains(*it))
          it = a.erase(it);
       else ++it;
    }

    return a;
}

std::unordered_set<size_t>& operator &=(std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b)
{
    for(auto it = a.begin(); it != a.end();)
    {
       if(not b.contains(*it))
          it = a.erase(it);
       else ++it;
    }
    return a;
}

unordered_set<size_t> operator - (const unordered_set<size_t>& a,
                                            const unordered_set<size_t>& b)
{
    if(a.size() <= b.size())
    {
        unordered_set<size_t> ret;
        for(size_t val: a)
            if(not b.contains(val))
                ret.insert(val);

        return ret;
    }
    else
    {
        unordered_set<size_t> ret(a);
        for(size_t val: b)
            ret.erase(val);

        return ret;
    }
}

unordered_set<size_t> operator - (unordered_set<size_t>&& a,
                                            const unordered_set<size_t>& b)
{
    for(auto it = a.begin(); it != a.end();)
    {
       if(b.contains(*it))
          it = a.erase(it);
       else ++it;
    }

    return a;
}


std::unordered_set<std::size_t> operator ^ (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b)
{
    return (a - b) + (b - a);
}
