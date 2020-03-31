//
//  HtnTermFactory.cpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include "Logger.h"
#include "HtnTerm.h"
#include "HtnTermFactory.h"


HtnTermFactory::HtnTermFactory() :
    m_otherAllocations(MaxIndexTerms * sizeof(string *)),
    m_outOfMemory(false),
    m_stringAllocations(0),
    m_termsCreated(0),
    m_uniquifier(0)
{
    m_uniqueIDBufferEnd = m_uniqueIDBuffer + MaxIndexTerms;
}

void HtnTermFactory::BeginTracking(const string &key)
{
    m_termCreationTracking[key] = pair<int,int>(m_termsCreated, (int) dynamicSize());
}

shared_ptr<HtnTerm> HtnTermFactory::CreateConstant(const string &name)
{
    m_termsCreated++;
    shared_ptr<HtnTerm> term = shared_ptr<HtnTerm>(new HtnTerm(name, false, shared_from_this()));
    return GetInternedTerm(term);
}

shared_ptr<HtnTerm> HtnTermFactory::CreateConstant(int value)
{
    return CreateConstant(lexical_cast<string>(value));
}

shared_ptr<HtnTerm> HtnTermFactory::CreateConstantFunctor(const string &name, vector<string> arguments)
{
    vector<shared_ptr<HtnTerm>> argumentTerms;
    for(auto arg : arguments)
    {
        argumentTerms.push_back(CreateConstant(arg));
    }
    
    return CreateFunctor(name, argumentTerms);
}

shared_ptr<HtnTerm> HtnTermFactory::CreateFunctor(const string &name, vector<shared_ptr<HtnTerm>> arguments)
{
    m_termsCreated++;
    shared_ptr<HtnTerm> term = shared_ptr<HtnTerm>(new HtnTerm(name, arguments, shared_from_this()));
    return GetInternedTerm(term);
}

std::shared_ptr<HtnTerm> HtnTermFactory::CreateList(std::vector<std::shared_ptr<HtnTerm>> arguments)
{
    shared_ptr<HtnTerm> lastTerm = EmptyList();
    for(int index = (int)arguments.size() - 1; index >= 0; index--)
    {
        lastTerm = CreateFunctor(".", { arguments[index], lastTerm });
    }
    
    return lastTerm;
}

shared_ptr<HtnTerm> HtnTermFactory::CreateVariable(const string &name)
{
    m_termsCreated++;
    shared_ptr<HtnTerm> term = shared_ptr<HtnTerm>(new HtnTerm(name, true, shared_from_this()));
    return GetInternedTerm(term);
}

shared_ptr<HtnCustomData> HtnTermFactory::customData(const string &name)
{
    map<string, shared_ptr<HtnCustomData>>::iterator found = m_customData.find(name);
    if(found == m_customData.end())
    {
        return nullptr;
    }
    else
    {
        return found->second;
    }
}

void HtnTermFactory::DebugDumpAllocations()
{
    int count = 0;
    for(auto item : m_internedTerms)
    {
        if(++count > 1000) break;
    }
}

shared_ptr<HtnTerm> HtnTermFactory::EmptyList()
{
    if(m_emptyList == nullptr)
    {
        m_emptyList = CreateConstant("[]");
    }
    
    return m_emptyList;
}

pair<int,int> HtnTermFactory::EndTracking(const string &key)
{
    auto found = m_termCreationTracking.find(key);
    int termsCreated = m_termsCreated - found->second.first;
    int memoryUsed = (int)(dynamicSize() - found->second.second);
    m_termCreationTracking.erase(found);
    return pair<int,int>(termsCreated, memoryUsed);
}

shared_ptr<HtnTerm> HtnTermFactory::False()
{
    if(m_false == nullptr)
    {
        m_false = CreateConstant("false");
    }
    
    return m_false;
}

const string *HtnTermFactory::GetInternedString(const string &value)
{
    InternedStringMap::iterator found = m_internedStrings.find(&value);
    if(found != m_internedStrings.end())
    {
        found->second += 1;
        return found->first;
    }
    else
    {
        string *newString = new string(value);
        m_stringAllocations += sizeof(string) + value.size();
        m_internedStrings.insert(pair<const string *, int>(newString, 1));
        return newString;
    }
}

shared_ptr<HtnTerm> HtnTermFactory::GetInternedTerm(shared_ptr<HtnTerm> &term)
{
    term->GetUniqueID(m_uniqueIDBuffer, m_uniqueIDBufferEnd);
    InternedTermMap::iterator found = m_internedTerms.find(m_uniqueIDBuffer);
    if(found != m_internedTerms.end())
    {
        // Element did exist, return that one
        FailFastAssert(!found->second.expired());
        return found->second.lock();
    }
    else
    {
        // Element didn't exist, intern it
        size_t keySize = (size_t) *m_uniqueIDBuffer;
        const string **id = new const string *[keySize];
        memcpy(id, m_uniqueIDBuffer, keySize * sizeof(string *));
        m_internedTerms.insert(pair<const string **, weak_ptr<HtnTerm>>(id, term));
        m_otherAllocations += sizeof(pair<const string **, weak_ptr<HtnTerm>>) + keySize * sizeof(const string *);
        term->SetInterned();
        return term;
    }
}

void HtnTermFactory::RecordAllocation(HtnTerm *term)
{
    int size = (int) term->dynamicSize();
    m_otherAllocations += size;
}

void HtnTermFactory::RecordDeallocation(HtnTerm *term)
{
    int size = (int) term->dynamicSize();
    m_otherAllocations -= size;
    FailFastAssert(m_otherAllocations >= 0);
}

void HtnTermFactory::ReleaseInternedString(const string *value)
{
    InternedStringMap::iterator found = m_internedStrings.find(value);
    if(found != m_internedStrings.end())
    {
        if(found->second == 1)
        {
            m_stringAllocations -= sizeof(string) + value->size();
            FailFastAssert(m_stringAllocations >= 0);
			const std::string* temp = found->first;
            m_internedStrings.erase(found);
			delete temp;
		}
        else
        {
            found->second -= 1;
        }
    }
    else
    {
        FailFastAssert(false);
    }
}

void HtnTermFactory::ReleaseInternedTerm(HtnTerm *term)
{
    term->GetUniqueID(m_uniqueIDBuffer, m_uniqueIDBufferEnd);
    size_t keySize = (size_t) *m_uniqueIDBuffer;
    m_otherAllocations -= sizeof(pair<const string **, weak_ptr<HtnTerm>>) + keySize * sizeof(const string *);

    InternedTermMap::iterator found = m_internedTerms.find(m_uniqueIDBuffer);
    FailFastAssert(found != m_internedTerms.end());
	const string** temp = found->first;
    m_internedTerms.erase(found);
	delete[] temp;
}

shared_ptr<HtnTerm> HtnTermFactory::True()
{
    if(m_true == nullptr)
    {
        m_true = CreateConstant("true");
    } 
    
    return m_true;
}
