//
//  ReflectionEnum.h
//  FXPlatform
//
//  Created by Eric Zinda on 11/17/15.
//  Copyright (c) 2015 Eric Zinda. All rights reserved.
//

#ifndef FXPlatform_ReflectionEnum_h
#define FXPlatform_ReflectionEnum_h
#include "FailFast.h"
#include <string>
#include <unordered_map>

/*
 Credit to: http://stackoverflow.com/questions/147267/easy-way-to-use-variables-of-enum-types-as-string-in-c#202511
 
 Create Enum:
 
 someEnum.h:
 
     #include "ReflectionEnum.h"
     #define SOME_ENUM(item) \
     item(SomeEnum, FirstValue,30) \
     item(SomeEnum, SecondValue,49) \
     item(SomeEnum, SomeOtherValue,50) \
     item(SomeEnum, OneMoreValue,100) \
     
     DECLARE_ENUM(SomeEnum,SOME_ENUM)

 someEnum.cpp:
 
     #include "someEnum.h"
     DEFINE_ENUM(SomeEnum,SOME_ENUM)
 
 Use Enum:
 // SomeEnumType derives from EnumReflectionObject
 // A public instance of SomeEnumObject is declared that can be used to iterate
 
*/

class EnumReflectionObject
{
public:
    typedef std::unordered_map<std::string, uint64_t> EnumInfosByStringType;
    EnumInfosByStringType& enumInfosByString()
    {
        return staticInfosByString;
    }
    
    typedef std::unordered_map<uint64_t, std::string> EnumInfosByIntType;
    EnumInfosByIntType& enumInfosByInt()
    {
        return staticInfosByInt;
    }
    
    uint64_t GetEnumInt(const std::string &value)
    {
        return enumInfosByString()[value];
    }
    
    std::string GetEnumString(uint64_t value)
    {
        return enumInfosByInt()[value];
    }
    
    bool HasEnum(uint64_t value)
    {
        return staticInfosByInt.find(value) != staticInfosByInt.end();
    }
    
    bool HasEnum(const std::string &value)
    {
        return staticInfosByString.find(value) != staticInfosByString.end();
    }
    
    class EnumInfo
    {
    public:
        EnumInfo(EnumReflectionObject &enumObject, const std::string &enumName, uint64_t enumValue)
        {
            EnumInfosByStringType& infosByString = enumObject.enumInfosByString();
            FailFastAssert(!enumObject.HasEnum(enumName));
            infosByString[enumName] = enumValue;
            
            EnumInfosByIntType& infosByInt = enumObject.enumInfosByInt();
            FailFastAssert(!enumObject.HasEnum(enumValue));
            infosByInt[enumValue] = enumName;
        }
    };
    
protected:
    EnumInfosByStringType staticInfosByString;
    EnumInfosByIntType staticInfosByInt;
};


/// declare the access function and define enum values
#define DECLARE_ENUM(EnumType, ENUM_DEF) \
enum class EnumType \
{ \
ENUM_DEF(ENUM_VALUE) \
}; \
class EnumType##Type : public EnumReflectionObject \
{ \
private:\
    virtual void bogus() { } \
}; \
extern EnumType##Type EnumType##Object;

// expansion macro for enum value definition
#define ENUM_VALUE(type, name, assign) name = assign,

// expansion macro for filling unordered_map
#define ENUM_VARIABLES(type, name, assign) \
EnumReflectionObject::EnumInfo type##EnumInfo##name(type##Object, #name, assign);

// define the access function names
#define DEFINE_ENUM(EnumType, ENUM_DEF) \
EnumType##Type EnumType##Object; \
ENUM_DEF(ENUM_VARIABLES)

#endif
