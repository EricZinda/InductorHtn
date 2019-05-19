//
//  HtnOperator.cpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#include "HtnOperator.h"
#include "HtnTerm.h"
using namespace std;

string HtnOperator::ToString() const
{
    stringstream stream;
    stream << head()->ToString() << " => del(";
    
    bool has = false;
    for(shared_ptr<HtnTerm>term : deletions())
    {
        stream << (has ? ", " : "") << term->ToString();
        has = true;
    }
    
    stream << "), add(";
    has = false;
    for(shared_ptr<HtnTerm>term : additions())
    {
        stream << (has ? ", " : "") << term->ToString();
        has = true;
    }
    stream << ")";
    
    return stream.str();
}
