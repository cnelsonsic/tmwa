#ifndef EXTRACT_HPP
#define EXTRACT_HPP
//    extract.hpp - a simple, hierarchical, tokenizer
//
//    Copyright © 2012-2013 Ben Longbons <b.r.longbons@gmail.com>
//
//    This file is part of The Mana World (Athena server)
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "sanity.hpp"

#include <algorithm>

#include "const_array.hpp"
#include "mmo.hpp"
#include "utils.hpp"

template<class T, typename=typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, char>::value>::type>
bool extract(const_string str, T *iv)
{
    if (!str || str.size() > 20)
        return false;
    if (!((str.front() == '-' && std::is_signed<T>::value)
            || ('0' <= str.front() && str.front() <= '9')))
        return false;
    char buf[20 + 1];
    std::copy(str.begin(), str.end(), buf);
    buf[str.size()] = '\0';

    char *end;
    errno = 0;
    if (std::is_signed<T>::value)
    {
        long long v = strtoll(buf, &end, 10);
        if (errno || *end)
            return false;
        *iv = v;
        return *iv == v;
    }
    else
    {
        unsigned long long v = strtoull(buf, &end, 10);
        if (errno || *end)
            return false;
        *iv = v;
        return *iv == v;
    }
}

inline
bool extract(const_string str, TimeT *tv)
{
    return extract(str, &tv->value);
}

// extra typename=void to workaround some duplicate overload rule
template<class T, typename=typename std::enable_if<std::is_enum<T>::value>::type, typename=void>
bool extract(const_string str, T *iv)
{
    typedef typename underlying_type<T>::type U;
    U v;
    // defer to integer version
    if (!extract(str, &v))
        return false;
    // TODO check bounds using enum min/max as in SSCANF
    *iv = static_cast<T>(v);
    return true;
}

bool extract(const_string str, const_string *rv);

bool extract(const_string str, std::string *rv);

template<size_t N>
__attribute__((deprecated))
bool extract(const_string str, char (*out)[N])
{
    if (str.size() >= N)
        return false;
    std::copy(str.begin(), str.end(), *out);
    std::fill(*out + str.size() , *out + N, '\0');
    return true;
}

// basically just a std::tuple
// but it provides its data members publically
template<char split, class... T>
class Record;
template<char split>
class Record<split>
{
};
template<char split, class F, class... R>
class Record<split, F, R...>
{
public:
    F frist;
    Record<split, R...> rest;
public:
    Record(F f, R... r)
    : frist(f), rest(r...)
    {}
};
template<char split, class... T>
Record<split, T...> record(T... t)
{
    return Record<split, T...>(t...);
}

template<char split>
bool extract(const_string str, Record<split>)
{
    return !str;
}
template<char split, class F, class... R>
bool extract(const_string str, Record<split, F, R...> rec)
{
    const char *s = std::find(str.begin(), str.end(), split);
    if (s == str.end())
        return sizeof...(R) == 0 && extract(str, rec.frist);
    return extract(const_string(str.begin(), s), rec.frist)
        && extract(const_string(s + 1, str.end()), rec.rest);
}

template<char split, class T>
struct VRecord
{
    std::vector<T> *arr;
};

template<char split, class T>
VRecord<split, T> vrec(std::vector<T> *arr)
{
    return {arr};
}

template<char split, class T>
bool extract(const_string str, VRecord<split, T> rec)
{
    if (str.empty())
        return true;
    const char *s = std::find(str.begin(), str.end(), split);
    rec.arr->emplace_back();
    if (s == str.end())
        return extract(str, &rec.arr->back());
    return extract(const_string(str.begin(), s), &rec.arr->back())
        && extract(const_string(s + 1, str.end()), rec);
}

bool extract(const_string str, struct global_reg *var);

bool extract(const_string str, struct item *it);

#endif // EXTRACT_HPP