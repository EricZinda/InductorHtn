//
//  HtnOperator.hpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef HtnOperator_hpp
#define HtnOperator_hpp
#include <memory>
#include <string>
#include <vector>
class HtnTerm;

// Operators are immutable
// Each operator indicates how a primitive task can be performed.
// Has a head which is a term, defines all the variables which can be used for deletions and additions
// Has deletions and additions
class HtnOperator
{
public:
    HtnOperator(std::shared_ptr<HtnTerm> head, const std::vector<std::shared_ptr<HtnTerm>> &additions, const std::vector<std::shared_ptr<HtnTerm>> &deletions, bool hidden = false) :
        m_additions(additions),
        m_deletions(deletions),
        m_head(head),
        m_isHidden(hidden)
    {
    }
    
    const std::vector<std::shared_ptr<HtnTerm>> additions() const { return m_additions; }
    const std::vector<std::shared_ptr<HtnTerm>> deletions() const { return m_deletions; }
    int64_t dynamicSize() { return sizeof(HtnOperator) + (m_additions.size() + m_deletions.size()) * sizeof(std::shared_ptr<HtnTerm>); }
    const std::shared_ptr<HtnTerm> head() const { return m_head; }
    std::string ToString() const;    
    bool isHidden() { return m_isHidden; }
    
private:
    std::vector<std::shared_ptr<HtnTerm>> m_additions;
    std::vector<std::shared_ptr<HtnTerm>> m_deletions;
    std::shared_ptr<HtnTerm> m_head;
    bool m_isHidden;
};

#endif /* HtnOperator_hpp */
