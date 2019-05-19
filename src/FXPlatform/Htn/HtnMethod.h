//
//  HtnMethod.hpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef HtnMethod_hpp
#define HtnMethod_hpp
#include <memory>
#include <string>
#include <vector>
class HtnTerm;

enum class HtnMethodType
{
    Normal,
    AllSetOf,
    AnySetOf
};

// Methods are immutable once created
// Has a head which is a term, defines all the variables which can be used for deletions and additions
// Has a precondition which is a conjunction of goals
// Has a list of tasks
class HtnMethod
{
public:
    HtnMethod(std::shared_ptr<HtnTerm> head, const std::vector<std::shared_ptr<HtnTerm>> &condition, const std::vector<std::shared_ptr<HtnTerm>> &tasks, HtnMethodType methodType, bool isDefault) :
        m_condition(condition),
        m_documentOrder(0),
        m_head(head),
        m_isDefault(isDefault),
        m_methodType(methodType),
        m_tasks(tasks)
    {
    }
    
    HtnMethod(std::shared_ptr<HtnTerm> head, const std::vector<std::shared_ptr<HtnTerm>> &condition, const std::vector<std::shared_ptr<HtnTerm>> &tasks, HtnMethodType methodType, bool isDefault, int documentOrder) :
        m_condition(condition),
        m_documentOrder(documentOrder),
        m_head(head),
        m_isDefault(isDefault),
        m_methodType(methodType),
        m_tasks(tasks)
    {
    }
    
    const std::vector<std::shared_ptr<HtnTerm>> &condition() const { return m_condition; }
    int documentOrder() { return m_documentOrder; }
    int64_t dynamicSize() { return sizeof(HtnMethod) + (m_condition.size() + m_tasks.size()) * sizeof(std::shared_ptr<HtnTerm>); };
    const std::shared_ptr<HtnTerm> head() const { return m_head; }
    bool isDefault() const { return m_isDefault; }
    HtnMethodType methodType() const { return m_methodType; }
    const std::vector<std::shared_ptr<HtnTerm>> tasks() const { return m_tasks; }
    std::string ToString() const;
    
private:
    // *** Remember to update dynamicSize() if you change any member variables!
    std::vector<std::shared_ptr<HtnTerm>> m_condition;
    int m_documentOrder; // Order they were written down in the document.  Monotonically increasing within a method, not guaranteed so outside of the method
    std::shared_ptr<HtnTerm> m_head;
    bool m_isDefault;
    HtnMethodType m_methodType;
    std::vector<std::shared_ptr<HtnTerm>> m_tasks;
};

#endif /* HtnMethod_hpp */
