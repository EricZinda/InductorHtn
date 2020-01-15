//
//  HtnDomain.h
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef HtnDomain_h
#define HtnDomain_h
#include <functional>
#include <vector>
class HtnMethod;
enum class HtnMethodType;
class HtnOperator;
class HtnTerm;

// HtnDomain is passed to the HtnCompiler to store methods and operators
// The implementation used in this project is implemented in HtnPlanner
class HtnDomain
{
public:
    virtual ~HtnDomain() {};
    virtual HtnMethod * AddMethod(shared_ptr<HtnTerm> head, const vector<shared_ptr<HtnTerm>> &condition, const vector<shared_ptr<HtnTerm>> &tasks, HtnMethodType methodType, bool isDefault) = 0;
    virtual HtnOperator *AddOperator(shared_ptr<HtnTerm> head, const vector<shared_ptr<HtnTerm>> &addList, const vector<shared_ptr<HtnTerm>> &deleteList, bool hidden = false) = 0;
    
    virtual void AllMethods(std::function<bool(HtnMethod *)> handler) = 0;
    virtual void AllOperators(std::function<bool(HtnOperator *)> handler) = 0;

    virtual void ClearAll() = 0;
};

#endif /* HtnDomain_h */
