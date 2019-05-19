#pragma once
#include <stdexcept>
#include <sstream>

double HighPerformanceGetTimeInSeconds();
int ReplaceAll(std::string& str, const std::string& from, const std::string& to);

// lexical_cast<type>(value) converts value into type if possible
template<typename Target, typename Source>
Target lexical_cast(Source arg, int precision=9)
{
    std::stringstream interpreter;
    interpreter.setf( std::ios::fixed, std::ios::floatfield );
    interpreter.precision(precision);
    
    Target result;    
    if(!(interpreter << arg))
    {
        throw std::runtime_error("bad lexical cast");
    }
    
    if(!(interpreter >> result))
    {
        throw std::runtime_error("bad lexical cast");
    }
    
    if(!(interpreter >> std::ws).eof())
    {
        throw std::runtime_error("bad lexical cast");
    }

    return result;
}

template<>
std::string lexical_cast<std::string>(const char arg, int precision);
// specialize for string to handle the case where the string is empty
template<>
std::string lexical_cast<std::string>(const char *arg, int precision);
template<>
std::string lexical_cast<std::string>(char *arg, int precision);
template<>
std::string lexical_cast<std::string>(const std::string arg, int precision);

// These are all here just to save time writing the same code over and over which 
// turns member variables into properties that just set and get them
#define UniquePtrArrayProperty(access, type, name) \
    public:\
    void name(unique_ptr<type []> value)       \
    {\
        m_##name = move(value);           \
    }\
    type *name() \
    {\
        return m_##name.get();\
    }\
    unique_ptr<type []>name##Move() \
    {\
        return move(m_##name);\
    }\
    access:\
    unique_ptr<type []> m_##name;

#define UniquePtrProperty(access, type, name) \
    public:\
    void name(unique_ptr<type> value)       \
    {\
        m_##name = move(value);           \
    }\
    type *name() \
    {\
        return m_##name.get();\
    }\
    access##:\
    unique_ptr<type> m_##name;

#define ConstProperty(access, type, name) \
    public:\
    void name(const type &value)       \
    {\
        m_##name = value;           \
    }\
    type &name() \
    {\
        return m_##name;\
    }\
    access:\
    type m_##name;

#define ReadOnlyProperty(access, type, name) \
    public:\
    type &name() \
    {\
        return m_##name;\
    }\
    access:\
    void name(type &value)       \
    {\
        m_##name = value;           \
    }\
    type m_##name;

#define Property(access, type, name) \
    public:\
    void name(type &value)       \
    {\
        m_##name = value;           \
    }\
    type &name() \
    {\
        return m_##name;\
    }\
    access:\
    type m_##name;

#define ValuePropertyCustom(memberAccess, propertyAccess, type, name) \
    propertyAccess:\
    void name(type value)       \
    {\
        m_##name = value;           \
    }\
    type name() \
    {\
        return m_##name;\
    }\
    memberAccess:\
    type m_##name;

#define ReadOnlyConstValueProperty(memberAccess, type, name) \
    public:\
    type name() const \
    {\
        return m_##name;\
    }\
    memberAccess:\
    void name(const type value)       \
    {\
        m_##name = value;           \
    }\
    type m_##name;

#define ReadOnlyValueProperty(memberAccess, type, name) \
    public:\
    type name() const \
    {\
        return m_##name;\
    }\
    memberAccess:\
    void name(type value)       \
    {\
        m_##name = value;           \
    }\
    type m_##name;

#define ValueProperty(memberAccess, type, name) \
    public:\
    void name(type value)       \
    {\
        m_##name = value;           \
    }\
    type name() const \
    {\
        return m_##name;\
    }\
    memberAccess:\
    type m_##name;

