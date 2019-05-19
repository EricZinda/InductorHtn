//
//  HtnArithmeticOperators.hpp
//  GameLib
//
//  Created by Eric Zinda on 10/2/18.
//  Copyright © 2018 Eric Zinda. All rights reserved.
//

#ifndef HtnArithmeticOperators_hpp
#define HtnArithmeticOperators_hpp
#include <memory>
class HtnTerm;
class HtnTermFactory;

// Use these definitions when thinking about conversion: http://www.swi-prolog.org/man/arith.html
class HtnArithmeticOperators
{
public:
    static std::shared_ptr<HtnTerm> Abs(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left);
    static std::shared_ptr<HtnTerm> Divide(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> Equal(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> Float(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left);
    static std::shared_ptr<HtnTerm> GreaterThan(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> GreaterThanOrEqual(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> Integer(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left);
    static std::shared_ptr<HtnTerm> LessThan(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> LessThanOrEqual(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> Max(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> Min(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> Minus(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> Multiply(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
    static std::shared_ptr<HtnTerm> Plus(HtnTermFactory *factory, std::shared_ptr<HtnTerm> left, std::shared_ptr<HtnTerm> right);
};

#endif /* HtnArithmeticOperators_hpp */
