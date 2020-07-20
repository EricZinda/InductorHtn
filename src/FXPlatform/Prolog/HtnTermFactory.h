//
//  HtnTermFactory.hpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef HtnTermFactory_hpp
#define HtnTermFactory_hpp
#include <cstring>
#include <map>
#include <unordered_map>
#include <string>
class HtnTerm;

// Derive a class from HtnCustomData and add to the TermFactory if you want to pass global data
// to a HtnGoalResolver custom rule
class HtnCustomData
{
private:
    // This is here to guarantee the class will be polymorphic
    virtual void bogus() {};
};

class HtnTermFactory : public std::enable_shared_from_this<HtnTermFactory>
{
public:
    HtnTermFactory();
    void BeginTracking(const std::string &key);
    std::shared_ptr<HtnTerm> CreateConstant(const std::string &name);
    std::shared_ptr<HtnTerm> CreateConstant(int value);
    std::shared_ptr<HtnTerm> CreateConstantFunctor(const std::string &name, std::vector<std::string> arguments);
    std::shared_ptr<HtnTerm> CreateFunctor(const std::string &name, std::vector<std::shared_ptr<HtnTerm>> arguments);
    std::shared_ptr<HtnTerm> CreateList(std::vector<std::shared_ptr<HtnTerm>> arguments);
    std::shared_ptr<HtnTerm> CreateVariable(const std::string &name);
    void DebugDumpAllocations();
    std::shared_ptr<HtnTerm> EmptyList();
    std::pair<int,int> EndTracking(const std::string &key);
    std::shared_ptr<HtnTerm> False();
    const std::string *GetInternedString(const std::string &value);
    std::shared_ptr<HtnTerm> GetInternedTerm(std::shared_ptr<HtnTerm> &term);
    void RecordAllocation(HtnTerm *term);
    void RecordDeallocation(HtnTerm *term);
    std::shared_ptr<HtnTerm> True();
    void ReleaseInternedString(const std::string *value);
    void ReleaseInternedTerm(HtnTerm *term);

    // This is how custom data can get marshalled to HtnGoalResolver custom rules
    std::shared_ptr<HtnCustomData> customData(const std::string &name);
    void customData(const std::string &name, std::shared_ptr<HtnCustomData> data) { m_customData[name] = data; }
    bool outOfMemory() { return m_outOfMemory; }
    void outOfMemory(bool value)
    {
        m_outOfMemory = value;
    }
    int64_t dynamicSize() { return m_otherAllocations + m_stringAllocations; }
    int64_t otherAllocationSize() { return m_otherAllocations; }
    int64_t stringSize() { return m_stringAllocations; }
    uint64_t &uniquifier() { return m_uniquifier; }
    
    // This can be anything, it represents the max number of terms we support
    // One array of strings of this size will be created
    static const int MaxIndexTerms = 4096;

private:
    std::map<std::string, std::shared_ptr<HtnCustomData>> m_customData;
    std::shared_ptr<HtnTerm> m_false;
    std::shared_ptr<HtnTerm> m_emptyList;

    struct stringPtrLess
    {
        bool operator()(const std::string *lhs, const std::string *rhs) const
        {
            return *lhs < *rhs;
        }
    };

    struct stringPtrEqual
    {
        bool operator()(const std::string *lhs, const std::string *rhs) const
        {
            return *lhs == *rhs;
        }
    };

    struct stringPtrHash
    {
        size_t operator()(const std::string *value) const
        {
            return std::hash<std::string>()(*value);
        }
    };

    struct uniqueIDPtrHash
    {
        size_t operator()(const std::string **value) const
        {
            size_t seed = 0;
            // First "pointer" is actually a count of pointers (including the count)
            size_t count = (size_t) *value;
            for(int index = 0; index < count; ++index)
            {
                // Stolen from boost::has_combine
                std::hash<const std::string *> hasher;
                seed ^= hasher(*value) + 0x9e3779b9 + (seed<<6) + (seed>>2);
                value++;
            }
            return seed;
        }
    };
    
    struct uniqueIDPtrEqual
    {
        bool operator()(const std::string **lhs, const std::string **rhs) const
        {
            if(*lhs != *rhs)
            {
                // Not the same number of elements, can't be equal
                return false;
            }
            else
            {
                // First "pointer" is actually a count of pointers (including the count)
                size_t count = (size_t) *lhs;
                return memcmp(lhs, rhs, count * sizeof(std::string *)) == 0;
            }
        }
    };
    
    typedef std::unordered_map<const std::string *, int, stringPtrHash, stringPtrEqual> InternedStringMap;
    InternedStringMap m_internedStrings;
    typedef std::unordered_map<const std::string **, std::weak_ptr<HtnTerm>, uniqueIDPtrHash, uniqueIDPtrEqual> InternedTermMap;
    InternedTermMap m_internedTerms;
    int64_t m_otherAllocations;
    bool m_outOfMemory;
    int64_t m_stringAllocations;
    std::map<std::string, std::pair<int, int>> m_termCreationTracking;
    int m_termsCreated;
    std::shared_ptr<HtnTerm> m_true;
    const std::string *m_uniqueIDBuffer[MaxIndexTerms];
    std::string const ** m_uniqueIDBufferEnd;
    // Global counter that is incremented every time it is used
    uint64_t m_uniquifier;
};

#endif /* HtnTermFactory_hpp */
