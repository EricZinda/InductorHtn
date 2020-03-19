//
//  HtnRule.hpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef HtnRule_hpp
#define HtnRule_hpp
#include <map>
#include <memory>
#include <string>
#include <vector>
class HtnRuleSet;
class HtnTerm;
class HtnTermFactory;

// Rules are immutable since they are looked up by key and the key is their signature
class HtnRule
{
public:
    HtnRule(std::shared_ptr<HtnTerm> head, std::vector<std::shared_ptr<HtnTerm>> tail) :
        m_head(head),
        m_tail(tail)
    {
    }
    
    int64_t dynamicSize() { return sizeof(HtnRule) + m_tail.size() * sizeof(std::shared_ptr<HtnTerm>); }
    std::string GetUniqueID() const { return ToString(); }
    bool IsFact() const
    {
        return m_tail.size() == 0;
    }
    std::shared_ptr<HtnRule> MakeVariablesUnique(HtnTermFactory *factory, const std::string &uniquifier, std::map<std::string, std::shared_ptr<HtnTerm>> &variableMap, bool justDontCareVariables = false) const;
    std::string ToString() const;
    std::string ToStringProlog() const;

    const std::shared_ptr<HtnTerm> head() const { return m_head; }
    const std::vector<std::shared_ptr<HtnTerm>> &tail() const { return m_tail; }
    
private:
    std::shared_ptr<HtnTerm> m_head;
    std::vector<std::shared_ptr<HtnTerm>> m_tail;
};

#endif /* HtnRule_hpp */
