//
//  HtnMethod.cpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include "HtnMethod.h"
#include "HtnTerm.h"
using namespace std;

string HtnMethod::ToString() const
{
    stringstream stream;
    stream << head()->ToString() << " => ";
    if(m_isDefault)
    {
        stream << "default, ";
    }
    
    if(m_methodType == HtnMethodType::AllSetOf)
    {
        stream << "allOf, ";
    }
    else if(m_methodType == HtnMethodType::AnySetOf)
    {
        stream << "anyOf, ";
    }
    
    stream << "if(";
    bool has = false;
    for(shared_ptr<HtnTerm>term : condition())
    {
        stream << (has ? ", " : "") << term->ToString();
        has = true;
    }
    
    stream << "), do(";
    has = false;
    for(shared_ptr<HtnTerm>term : tasks())
    {
        stream << (has ? ", " : "") << term->ToString();
        has = true;
    }
    stream << ")";
    
    return stream.str();
}
