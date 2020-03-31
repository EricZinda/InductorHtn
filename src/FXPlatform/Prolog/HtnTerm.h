//
//  HtnTerm.hpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef HtnTerm_hpp
#define HtnTerm_hpp
#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
class HtnTermComparer;
class HtnTermFactory;

// Must be ordered in terms of smallest to largest in standard Prolog sorting rules
// Prolog comparison rules in order of smallest to greatest:
// 1. variables, oldest first.
// 2. finite domain variables (section 9.1.1), oldest first.
// 3. floating point numbers, in numeric order.
// 4. integers, in numeric order.
// 5. atoms, in alphabetical (i.e. character code) order.
// 6. compound terms, ordered first by arity, then by the name of the principal functor and by the arguments in left-to-right order.
enum class HtnTermType
{
    Variable = 0,
    FloatType = 1,
    IntType = 2,
    Atom = 3,
    Compound = 4
};

// Terms are immutable, this is required because their signature (name, argument count, argument name, etc) are used
// as the primary way to find them in the HtnRuleSet.  Making them immutable means we can keep them in a map.
// A term is variable, or a compound term which has a name and 0 or more arguments (which are also terms)
// A compound term with no arguments is called a constant
// A term is ground if it contains no variables
class HtnTerm : public std::enable_shared_from_this<HtnTerm>
{
public:
    ~HtnTerm();
    const std::vector<std::shared_ptr<HtnTerm>> &arguments() const { return m_arguments; }
    int arity() const { return (int) m_arguments.size(); }
    // Very efficient name comparison because names are interned
    bool nameEqualTo(const HtnTerm &other) const
    {
        return m_namePtr == other.m_namePtr;
    }
    int64_t dynamicSize();
    std::shared_ptr<HtnTerm> Eval(HtnTermFactory *factory);
    void GetAllVariables(std::vector<std::string> *result);
    void GetAllVariables(std::set<std::shared_ptr<HtnTerm>, HtnTermComparer> *result);
    double_t GetDouble() const;
    int64_t GetInt() const;
    HtnTermType GetTermType() const;
    // Terms should never change after they are created
    typedef uint64_t HtnTermID;
    HtnTermID GetUniqueID() const;
    void GetUniqueID(const std::string **buffer, const std::string **bufferEnd) const;
    bool isArithmetic() const;
    bool isCompoundTerm() { return arity() > 0; }
    bool isConstant() const { return !m_isVariable && m_arguments.size() == 0; }
	bool isCut() const { return  *m_namePtr == "!";  }
    // We can compare pointers for equivalence because names are interned
    bool isEquivalentCompoundTerm(const HtnTerm *other) const { return arity() == other->arity() && m_namePtr == other->m_namePtr; }
    bool isList() const { return (arity() == 0 && name() == "[]") || (arity() == 2 && name() == ".");  }
    bool isGround() const;
    void SetInterned() { m_isInterned = true; };
    bool isTrue() const { return !m_isVariable && *m_namePtr == "true"; }
    bool isVariable() const { return m_isVariable; }
    std::shared_ptr<HtnTerm> MakeVariablesUnique(HtnTermFactory *factory, bool onlyDontCareVariables, const std::string &uniquifier, int* dontCareCount, std::map<std::string, std::shared_ptr<HtnTerm>> &variableMap);
    std::string name() const { return m_isVariable ? m_namePtr->substr(1, m_namePtr->size() - 1) : *m_namePtr; }
    bool OccursCheck(std::shared_ptr<HtnTerm> variable) const;
    bool operator==(const HtnTerm &other) const;
    std::shared_ptr<HtnTerm> RemovePrefixFromVariables(HtnTermFactory *factory, const std::string &prefix);
    std::shared_ptr<HtnTerm> RenameVariables(HtnTermFactory *factory, std::map<std::string, std::shared_ptr<HtnTerm>> variableMap);
    std::shared_ptr<HtnTerm> ResolveArithmeticTerms(HtnTermFactory *factory);
    std::shared_ptr<HtnTerm> SubstituteTermForVariable(HtnTermFactory *factory, std::shared_ptr<HtnTerm> newTerm, std::shared_ptr<HtnTerm> existingVariable);
    // Compares using prolog comparison rules
    int TermCompare(const HtnTerm &other);
    std::string ToString(bool isSecondTermInList = false, bool json = false);
    static std::string ToString(const std::vector<std::shared_ptr<HtnTerm>> &goals, bool surroundWithParenthesis = true, bool json = false);
    
    const std::string *m_namePtr;

private:
    // All constructors are private so that TermFactory is used so we can track memory easier
    friend class HtnTermFactory;
    HtnTerm(); // Leave undefined so we get link errors if anyone uses it
    HtnTerm(const HtnTerm &other); // Leave undefined so we get link errors if anyone uses it. Won't properly track string interning if we use copy constructor
    HtnTerm(const HtnTerm &other, std::weak_ptr<HtnTermFactory> factory);
    // Create a constant
    HtnTerm(const std::string &constantName, std::weak_ptr<HtnTermFactory> factory);
    // Create a constant or variable
    HtnTerm(const std::string &constantName, bool isVariable, std::weak_ptr<HtnTermFactory> factory);
    // Create a functor
    HtnTerm(const std::string &functorName, std::vector<std::shared_ptr<HtnTerm>> arguments, std::weak_ptr<HtnTermFactory> factory);
    void arguments(std::vector<std::shared_ptr<HtnTerm>> args) { m_arguments = args; }
    void isVariable(bool value) { m_isVariable = value; }
    
    // *** Remember to update dynamicSize() if you change any member variables!
    std::vector<std::shared_ptr<HtnTerm>> m_arguments;
    bool m_isInterned;
    bool m_isVariable;
    std::weak_ptr<HtnTermFactory> m_factory;
};

class HtnTermVectorComparer
{
public:
    bool operator() (const std::vector<std::shared_ptr<HtnTerm>> left, const std::vector<std::shared_ptr<HtnTerm>> right) const
    {
        if(left.size() < right.size())
        {
            return true;
        }
        else if(left.size() > right.size())
        {
            return false;
        }
        
        for(int index = (int) left.size() - 1; index >= 0 ; --index)
        {
            int compareResult = left[index]->TermCompare(*right[index].get());
            if(compareResult < 0)
            {
                return true;
            }
            else if(compareResult > 0)
            {
                return false;
            }
        }
        
        return false;
    }
};

class HtnTermComparer
{
public:
    bool operator() (const std::shared_ptr<HtnTerm> left, const std::shared_ptr<HtnTerm> right) const
    {
        return left->TermCompare(*right.get()) < 0;
    }
};

template<typename Target, typename Source>
Target lexical_cast_result(Source arg,  bool &success, int precision=9)
{
    std::stringstream interpreter;
    interpreter.setf( std::ios::fixed, std::ios::floatfield );
    interpreter.precision(precision);
    
    Target result;
    success = false;
    if(!(interpreter << arg))
    {
        return Target();
    }
    
    if(!(interpreter >> result))
    {
        return Target();
    }
    
    if(!(interpreter >> std::ws).eof())
    {
        return Target();
    }
    
    success = true;
    return result;
}

#endif /* HtnTerm_hpp */
