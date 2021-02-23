//
//  HtnTerm.cpp
//  GameLib
//
//  Created by Eric Zinda on 1/15/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//
#include "FXPlatform/FailFast.h"
#include "FXPlatform/Utilities.h"
#include "HtnTerm.h"
#include "HtnArithmeticOperators.h"
#include "HtnTermFactory.h"
#include <stack>
using namespace std;

HtnTerm::HtnTerm(const HtnTerm &other, weak_ptr<HtnTermFactory> factory)
{
    shared_ptr<HtnTermFactory> factoryStrong = factory.lock();
    m_namePtr = factoryStrong->GetInternedString(*other.m_namePtr);
    m_isVariable = other.m_isVariable;
    m_arguments = other.m_arguments;
    m_factory = factory;
    m_isInterned = false;
    factoryStrong->RecordAllocation(this);
}

// Create a constant
HtnTerm::HtnTerm(const string &constantName, weak_ptr<HtnTermFactory> factory) :
    m_isInterned(false),
    m_isVariable(false),
    m_factory(factory)
{
    shared_ptr<HtnTermFactory> factoryStrong = factory.lock();
    m_namePtr = factoryStrong->GetInternedString(constantName);
    factoryStrong->RecordAllocation(this);
}

// Create a constant or variable
HtnTerm::HtnTerm(const string &constantName, bool isVariable, weak_ptr<HtnTermFactory> factory) :
    m_isInterned(false),
    m_isVariable(isVariable),
    m_factory(factory)
{
    shared_ptr<HtnTermFactory> factoryStrong = factory.lock();
    string adjustedName = isVariable ? "?" + constantName : constantName;
    m_namePtr = factoryStrong->GetInternedString(adjustedName);
    factoryStrong->RecordAllocation(this);
}

// Create a functor
HtnTerm::HtnTerm(const string &functorName, vector<shared_ptr<HtnTerm>> arguments, weak_ptr<HtnTermFactory> factory) :
    m_arguments(arguments),
    m_isInterned(false),
    m_isVariable(false),
    m_factory(factory)
{
    shared_ptr<HtnTermFactory> factoryStrong = factory.lock();
    m_namePtr = factoryStrong->GetInternedString(functorName);
    factoryStrong->RecordAllocation(this);
}

HtnTerm::~HtnTerm()
{
    if(shared_ptr<HtnTermFactory> strongFactory = m_factory.lock())
    {
        if(m_isInterned)
        {
            strongFactory->ReleaseInternedTerm(this);
        }
        
        strongFactory->ReleaseInternedString(m_namePtr);
        strongFactory->RecordDeallocation(this);
    }
}

// How big is this object in memory?
int64_t HtnTerm::dynamicSize()
{
    int termSize = sizeof(HtnTerm);
    int ptrSize = sizeof(shared_ptr<HtnTerm>);
    return termSize +
        // Account for all the shared_ptrs in the arguments array
        ptrSize * m_arguments.size();
}

// Returns nullptr if not possible to eval
shared_ptr<HtnTerm> HtnTerm::Eval(HtnTermFactory *factory)
{
    // Make sure we are not intermixing terms from different factories
    FXDebugAssert(this->m_factory.lock().get() == factory);

    if(isVariable())
    {
        return nullptr;
    }
    else if(isConstant())
    {
        HtnTermType type = GetTermType();
        if(type != HtnTermType::IntType && type != HtnTermType::FloatType)
        {
            return nullptr;
        }
        else
        {
            return shared_from_this();
        }
    }
    else
    {
        if(m_arguments.size() == 2)
        {
            if(*m_namePtr == "=")
            {
                return HtnArithmeticOperators::Equal(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == ">")
            {
                return HtnArithmeticOperators::GreaterThan(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "=>")
            {
                // Avoid common error that is really confusing.  Prolog uses >=
                FailFastAssertDesc(false, "=> is incorrect in Prolog.  Use >=");
                return nullptr;
            }
            else if(*m_namePtr == ">=")
            {
                return HtnArithmeticOperators::GreaterThanOrEqual(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "<")
            {
                return HtnArithmeticOperators::LessThan(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "<=")
            {
                // Avoid common error that is really confusing.  Prolog uses =<
                FailFastAssertDesc(false, "<= is incorrect in Prolog. Use =<");
                return nullptr;
            }
            else if(*m_namePtr == "=<")
            {
                return HtnArithmeticOperators::LessThanOrEqual(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "-")
            {
                return HtnArithmeticOperators::Minus(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "+")
            {
                return HtnArithmeticOperators::Plus(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "*")
            {
                return HtnArithmeticOperators::Multiply(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "/")
            {
                return HtnArithmeticOperators::Divide(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "min")
            {
                return HtnArithmeticOperators::Min(factory, m_arguments[0], m_arguments[1]);
            }
            else if(*m_namePtr == "max")
            {
                return HtnArithmeticOperators::Max(factory, m_arguments[0], m_arguments[1]);
            }
            else
            {
                return nullptr;
            }
        }
        else if(m_arguments.size() == 1)
        {
            if(*m_namePtr == "abs")
            {
                return HtnArithmeticOperators::Abs(factory, m_arguments[0]);
            }
            else if(*m_namePtr == "float")
            {
                return HtnArithmeticOperators::Float(factory, m_arguments[0]);
            }
            else if(*m_namePtr == "integer")
            {
                return HtnArithmeticOperators::Integer(factory, m_arguments[0]);
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
}

// A term is ground if it has no variables
bool HtnTerm::isGround() const
{
    if(m_isVariable)
    {
        return false;
    }
    else
    {
        for(shared_ptr<HtnTerm> arg : m_arguments)
        {
            if(!arg->isGround())
            {
                return false;
            }
        }
        
        return true;
    }
}

bool HtnTerm::isArithmetic() const
{
    if(*m_namePtr == "=" || *m_namePtr == ">" || *m_namePtr == ">=" || *m_namePtr == "<" || *m_namePtr == "=<" || *m_namePtr == "-" || *m_namePtr == "+" || *m_namePtr == "*" || *m_namePtr == "/" | *m_namePtr == "abs" || *m_namePtr == "min" || *m_namePtr == "max" ||  *m_namePtr == "float" || *m_namePtr == "integer")
    {
        return true;
    }
    else if(*m_namePtr == "=>" || *m_namePtr == "<=")
    {
        // Avoid really common issues
        FailFastAssertDesc(false, "Incorrect symbol. Prolog uses >= and =<.");
        return false;
    }
    else
    {
        HtnTermType type = GetTermType();
        return type == HtnTermType::IntType || type == HtnTermType::FloatType;
    }
}

void HtnTerm::GetAllVariables(vector<string> *result)
{
    if(m_isVariable)
    {
        result->push_back(this->name());
    }
    else
    {
        for(shared_ptr<HtnTerm> arg : m_arguments)
        {
            arg->GetAllVariables(result);
        }
    }
}

void HtnTerm::GetAllVariables(set<shared_ptr<HtnTerm>, HtnTermComparer> *result)
{
    if(m_isVariable)
    {
        result->insert(shared_from_this());
    }
    else
    {
        for(shared_ptr<HtnTerm> arg : m_arguments)
        {
            arg->GetAllVariables(result);
        }
    }
}

double_t HtnTerm::GetDouble() const
{
    return lexical_cast<double_t>(*m_namePtr);
}

int64_t HtnTerm::GetInt() const
{
    return (int64_t) GetDouble();
}

HtnTermType HtnTerm::GetTermType() const
{
    if(m_isVariable)
    {
        return HtnTermType::Variable;
    }
    else if(arity() > 0)
    {
        return HtnTermType::Compound;
    }
    else
    {
        // Convert to float, if it works, and if there was a "." in the string, then it was a float.  Otherwise an Int. If it didn't convert, it was neither
        bool success;
        lexical_cast_result<double>(*m_namePtr, success);
        if(success)
        {
            if(m_namePtr->find_first_of(".") == string::npos)
            {
                // no period, couldn't have been a float
                return HtnTermType::IntType;
            }
            else
            {
                return HtnTermType::FloatType;
            }
        }
        else
        {
            // Must be an Atom
            return HtnTermType::Atom;
        }
    }
}

// Start of a term is a 0, end of a term is a 1
// Build them up into fake pointers so we can stick them in the array of string *s that
// represents the ID
class StructureBuilder
{
public:
    StructureBuilder()
    {
        m_index = 0;
        m_data.push_back(0);
        m_currentData = &m_data.back();
    }
    
    inline void StartTerm()
    {
        if(m_index + 1 == sizeof(intptr_t))
        {
            m_index = 0;
            m_data.push_back(0);
            m_currentData = &m_data.back();
        }
        
        m_index++;
    }
    
    inline void EndTerm()
    {
        if(m_index + 1 == sizeof(intptr_t))
        {
            m_index = 0;
            m_data.push_back(0);
            m_currentData = &m_data.back();
        }
        
        (*m_currentData) |= ((intptr_t) 0x1 <<  m_index);
        m_index++;
    }
    
    inline vector<intptr_t> &GetData()
    {
        return m_data;
    }
    
    int m_index;
    intptr_t *m_currentData;
    vector<intptr_t> m_data;
};

// Allows us to compare terms for equality simply by comparing bytes
// Because the strings of a Term are all interned, we can use their pointers as unique IDs for
// each string.  Then, if we pack them all together with a representation of the tree that they
// are a part of, it will be unique. How to represent the shape of the tree?
//  From: https://www.quora.com/How-can-I-encode-a-tree-into-a-string-format-such-that-the-tree-can-be-constructed-back-from-the-string-encoding-Also-encoding-should-be-efficient-both-time-and-space-wise
//      Run a depth-first search on our tree, starting at the root. Each time you enter a new vertex, write down a 0, each time you finalize a vertex, write down a 1.
//      Clearly, the output of this algorithm allows us to reconstruct the original tree. It is instructional to use ( and ) instead of 0 and 1. Our sample tree then
//      gets encoded as follows: ((()(()))()(())). This encoding also highlights an important property of unlabeled rooted trees -- their relationship to correctly parenthesized expressions.
// Structure is:
// string *count - number of string *s. This is a fake string * that is really a count
// string *[] - one string * for every term that points to its name, followed by a set of fake string *s that are from the StructureBuilder that represent the shape of the term
void HtnTerm::GetUniqueID(const string **buffer, const string **bufferEnd) const
{
    StructureBuilder shape;
    vector<pair<const HtnTerm *, int>> stack;
    
    // first pointer value is actually the length
    const string **origBuffer = buffer;
    buffer++;
    if(buffer == bufferEnd) StaticFailFastAssertDesc(false, ("Too many terms, max = " + lexical_cast<string>(HtnTermFactory::MaxIndexTerms)).c_str());
    
    // Do the root
    *buffer = m_namePtr;
    buffer++;
    if(buffer == bufferEnd) StaticFailFastAssertDesc(false, ("Too many terms, max = " + lexical_cast<string>(HtnTermFactory::MaxIndexTerms)).c_str());
    
    if(m_arguments.size() > 0)
    {
        // Push the first node to process its arguments
        stack.push_back(pair<const HtnTerm *, int>(this, 0));
        shape.StartTerm();
        
        while(stack.size() > 0)
        {
            const HtnTerm *current = stack.back().first;
            int &index = stack.back().second;
            
            // Do next child
            if(index < current->m_arguments.size())
            {
                HtnTerm *next = current->m_arguments[index].get();
                index++;
                
                // Add this child's name
                *buffer = next->m_namePtr;
                buffer++;
                if(buffer == bufferEnd) StaticFailFastAssertDesc(false, ("Too many terms, max = " + lexical_cast<string>(HtnTermFactory::MaxIndexTerms)).c_str());
                
                if(next->m_arguments.size() > 0)
                {
                    // Push to process its arguments
                    stack.push_back(pair<HtnTerm *, int>(next, 0));
                    shape.StartTerm();
                }
            }
            else
            {
                // Arguments done
                stack.pop_back();
                shape.EndTerm();
            }
        }
    }
       
    // Now write out the shape as a series of fake string *s
    vector<intptr_t> &shapeData = shape.GetData();
    for(auto data : shapeData)
    {
        *buffer = reinterpret_cast<string *>(data);
        buffer++;
        if(buffer == bufferEnd) StaticFailFastAssertDesc(false, ("Too many terms, max = " + lexical_cast<string>(HtnTermFactory::MaxIndexTerms)).c_str());
    }
    
    // Put the size as the first "pointer"
    *origBuffer = (string *) (buffer - origBuffer);
    return;
}

// Because HtnTerms are interned, their pointer is a unique ID
// and can be used for comparison
HtnTerm::HtnTermID HtnTerm::GetUniqueID() const
{
    FailFastAssert(m_isInterned);
    FailFastAssert(sizeof(HtnTermID) == sizeof(intptr_t));
    return reinterpret_cast<HtnTermID>(this);
}

shared_ptr<HtnTerm> HtnTerm::MakeVariablesUnique(HtnTermFactory *factory, bool onlyDontCareVariables, const string &uniquifier, int *dontCareCount, std::map<std::string, std::shared_ptr<HtnTerm>> &variableMap)
{
    // Make sure we are not intermixing terms from different factories
    // Too expensive to have in retail
    FXDebugAssert(factory != nullptr && this->m_factory.lock().get() == factory);
    
    if(this->isVariable())
    {
        const string &variableName = this->name();
        if(variableName[0] == '_')
        {
            // These cannot match each other so they can't use the same uniquifier
            // make sure they continue to start with "_" (illegal for a prolog name) so we can tell if they
            // are anonymous in rules
            shared_ptr<HtnTerm> result = factory->CreateVariable("_" + uniquifier + variableName + lexical_cast<std::string>(*dontCareCount));
            variableMap[variableName] = result;
            (*dontCareCount) = (*dontCareCount) + 1;
            return result;
        }
        else if(onlyDontCareVariables)
        {
            return this->shared_from_this();
        }
        else
        {
            shared_ptr<HtnTerm> newVariable = factory->CreateVariable(uniquifier + variableName);
            variableMap[variableName] = newVariable;
            return newVariable;
        }
    }
    else if(this->isConstant())
    {
        return this->shared_from_this();
    }
    else
    {
        // This is a functor
        vector<shared_ptr<HtnTerm>> newArguments;
        for(vector<shared_ptr<HtnTerm>>::const_iterator argIter = m_arguments.begin(); argIter != m_arguments.end(); ++argIter)
        {
            shared_ptr<HtnTerm> term = *argIter;
            newArguments.push_back(term->MakeVariablesUnique(factory, onlyDontCareVariables, uniquifier, dontCareCount, variableMap));
        }
        
        return factory->CreateFunctor(*m_namePtr, newArguments);
    }
}

bool HtnTerm::OccursCheck(shared_ptr<HtnTerm> variable) const
{
    // Make sure we are not intermixing terms from different factories
    FXDebugAssert(this->m_factory.lock() == variable->m_factory.lock());

    if(isVariable() && *this == *variable)
    {
        return true;
    }
    else
    {
        for(shared_ptr<HtnTerm> term : m_arguments)
        {
            if(term->OccursCheck(variable))
            {
                return true;
            }
        }
        
        return false;
    }
}

bool HtnTerm::operator==(const HtnTerm &other) const
{
    // Make sure we are not intermixing terms from different factories
    FXDebugAssert(this->m_factory.lock() == other.m_factory.lock());

    if(m_isVariable == other.m_isVariable &&
       m_namePtr == other.m_namePtr)
    {
        if(m_isVariable)
        {
            return true;
        }
        
        if(m_arguments.size() != other.m_arguments.size())
        {
            return false;
        }
        
        for(int i = 0; i < m_arguments.size(); ++i)
        {
            if(!(*m_arguments[i] == *other.m_arguments[i]))
            {
                return false;
            }
        }
        
        return true;
    }
    else
    {
        return false;
    }
}

shared_ptr<HtnTerm> HtnTerm::RemovePrefixFromVariables(HtnTermFactory *factory, const string &prefix)
{
    // Make sure we are not intermixing terms from different factories
    // Too expensive to have in retail
    FXDebugAssert(factory != nullptr && this->m_factory.lock().get() == factory);
    if(this->isVariable())
    {
        const string &variableName = this->name();
        int pos = (int) variableName.find(prefix);
        if(pos == 0)
        {
             return factory->CreateVariable(variableName.substr(prefix.size()));
        }
        else
        {
            return this->shared_from_this();
        }
    }
    else if(this->isConstant())
    {
        return this->shared_from_this();
    }
    else
    {
        // This is a functor
        vector<shared_ptr<HtnTerm>> newArguments;
        for(vector<shared_ptr<HtnTerm>>::const_iterator argIter = m_arguments.begin(); argIter != m_arguments.end(); ++argIter)
        {
            shared_ptr<HtnTerm> term = *argIter;
            newArguments.push_back(term->RemovePrefixFromVariables(factory, prefix));
        }
        
        return factory->CreateFunctor(*m_namePtr, newArguments);
    }
}

shared_ptr<HtnTerm> HtnTerm::RenameVariables(HtnTermFactory *factory, std::map<std::string, std::shared_ptr<HtnTerm>> variableMap)
{
    // Make sure we are not intermixing terms from different factories
    // Too expensive to have in retail
    FXDebugAssert(factory != nullptr && this->m_factory.lock().get() == factory);
    if(this->isVariable())
    {
        const string &variableName = this->name();
        auto found = variableMap.find(variableName);
        if(found != variableMap.end())
        {
            return found->second;
        }
        else
        {
            return this->shared_from_this();
        }
    }
    else if(this->isConstant())
    {
        return this->shared_from_this();
    }
    else
    {
        // This is a functor
        vector<shared_ptr<HtnTerm>> newArguments;
        for(vector<shared_ptr<HtnTerm>>::const_iterator argIter = m_arguments.begin(); argIter != m_arguments.end(); ++argIter)
        {
            shared_ptr<HtnTerm> term = *argIter;
            newArguments.push_back(term->RenameVariables(factory, variableMap));
        }
        
        return factory->CreateFunctor(*m_namePtr, newArguments);
    }
}

shared_ptr<HtnTerm> HtnTerm::ResolveArithmeticTerms(HtnTermFactory *factory)
{
    // Make sure we are not intermixing terms from different factories
    FXDebugAssert(factory != nullptr && this->m_factory.lock().get() == factory);

    if(!isCompoundTerm())
    {
        return shared_from_this();
    }
    
    if(isArithmetic())
    {
        shared_ptr<HtnTerm> newTerm = Eval(factory);
        if(newTerm != nullptr)
        {
            return newTerm;
        }
    }
    
    // See if we can resolve any children
    vector<shared_ptr<HtnTerm>> newChildren;
    for(auto term : m_arguments)
    {
        shared_ptr<HtnTerm> newTerm = term->ResolveArithmeticTerms(factory);
        if(newTerm != nullptr)
        {
            newChildren.push_back(newTerm);
        }
        else
        {
            newChildren.push_back(term);
        }
    }
    
    return factory->CreateFunctor(*m_namePtr, newChildren);
}

class SubstituteStackFrame
{
public:
    SubstituteStackFrame() :
        m_argIndex(0)
    {
    }
    
    SubstituteStackFrame(HtnTerm *term) :
        m_argIndex(0),
        m_term(term)
    {
    }
    
    int m_argIndex;
    // If the item is a functor, this is used
    vector<shared_ptr<HtnTerm>> m_newArguments;
    shared_ptr<HtnTerm> m_returnValue;
    HtnTerm *m_term;
};


shared_ptr<HtnTerm> HtnTerm::SubstituteTermForVariable(HtnTermFactory *factory, shared_ptr<HtnTerm> newTerm, shared_ptr<HtnTerm> existingVariable)
{
    // Make sure we are not intermixing terms from different factories
    FXDebugAssert(this->m_factory.lock().get() == factory && newTerm->m_factory.lock().get() == factory && existingVariable->m_factory.lock().get() == factory);
    FXDebugAssert(existingVariable->isVariable());
    
    vector<SubstituteStackFrame> stack;
    stack.push_back(SubstituteStackFrame(this));
    
    SubstituteStackFrame last;
    while(stack.size() > 0)
    {
        SubstituteStackFrame &current = stack.back();
        
        if(current.m_term->isVariable())
        {
            if(current.m_term->nameEqualTo(*existingVariable))
            {
                current.m_returnValue = newTerm;
                last = current;
                stack.pop_back();
            }
            else
            {
                current.m_returnValue = current.m_term->shared_from_this();
                last = current;
                stack.pop_back();
            }
        }
        else if(current.m_term->isConstant())
        {
            current.m_returnValue = current.m_term->shared_from_this();
            last = current;
            stack.pop_back();
        }
        else
        {
            if(last.m_returnValue != nullptr)
            {
                current.m_newArguments.push_back(last.m_returnValue);
                last = SubstituteStackFrame();
                current.m_argIndex++;
                if(current.m_argIndex == current.m_term->arguments().size())
                {
                    current.m_returnValue = factory->CreateFunctor(*(current.m_term->m_namePtr), current.m_newArguments);
                    last = current;
                    stack.pop_back();
                }
                else
                {
                    stack.push_back(current.m_term->arguments()[current.m_argIndex].get());
                }
            }
            else if(current.m_argIndex < current.m_term->arguments().size())
            {
                stack.push_back(current.m_term->arguments()[current.m_argIndex].get());
            }
            else
            {
                // Should have been a variable, constant or functor...
                FXDebugAssert(false);
            }
        }
    }
    
    return last.m_returnValue;
}

// Compares using prolog comparison rules in order of smallest to largest:
// variables, oldest first.
// finite domain variables (section 9.1.1), oldest first.
// floating point numbers, in numeric order.
// integers, in numeric order.
// atoms, in alphabetical (i.e. character code) order.
// compound terms, ordered first by arity, then by the name of the principal functor and by the arguments in left-to-right order.
// returns 0 if ==, -1 if less than, 1 if >
int HtnTerm::TermCompare(const HtnTerm &other)
{
    // Make sure we are not intermixing terms from different factories
    FXDebugAssert(this->m_factory.lock() == other.m_factory.lock());

    HtnTermType thisType = GetTermType();
    HtnTermType otherType = other.GetTermType();
    
    if(thisType == otherType)
    {
        if(arity() < other.arity())
        {
            return -1;
        }
        else if(arity() == other.arity())
        {
            if(arity() == 0)
            {
                if(thisType == HtnTermType::Variable)
                {
                    // Should be based on AGE, but...why? Plus thats a lot of work
                    int compare = name().compare(other.name());
                    return compare;
                }
                else if(thisType == HtnTermType::FloatType)
                {
                    double value = GetDouble();
                    double otherValue = other.GetDouble();
                    if(value < otherValue) { return -1; }
                    else if(value == otherValue) { return 0; }
                    else { return 1; }
                }
                else if(thisType == HtnTermType::IntType)
                {
                    int64_t value = GetInt();
                    int64_t otherValue = other.GetInt();
                    if(value < otherValue) { return -1; }
                    else if(value == otherValue) { return 0; }
                    else { return 1; }
                }
                else if(thisType == HtnTermType::Atom)
                {
                    return name().compare(other.name());
                }
                else
                {
                    // Unknown type
                    FailFastAssert(false);
                    return -1;
                }
            }
            else
            {
                int compare = name().compare(other.name());
                if(compare == 0)
                {
                    // Compare the arguments
                    for(int index = 0; index < arguments().size(); ++index)
                    {
                        int compare = arguments()[index]->TermCompare(*other.arguments()[index]);
                        if(compare != 0)
                        {
                            return compare;
                        }
                    }
                    
                    // Everything was == thus we are ==
                    return 0;
                }
                else
                {
                    return compare;
                }
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return thisType < otherType ? -1 : 1;
    }
}

string HtnTerm::ToString(bool isSecondTermInList, bool json)
{
    stringstream stream;
    if(isList() && arity() == 0)
    {
        // Empty list
        return "[]";
    }
    else if(isConstant() || isVariable())
    {
        if (json)
        {
            if(isVariable())
            {
                stream << "{\"" << m_namePtr->substr(1) << "\":[]}";
            }
            else
            {
                string test = *m_namePtr;
                // If it starts with a number and is a legitimate number, don't escape it
                if(test[0] >= '0' && test[0] <= '9')
                {
                    HtnTermType type = GetTermType();
                    if(type == HtnTermType::IntType || type == HtnTermType::FloatType)
                    {
                        stream << "{\"" << test << "\":[]}";
                    }
                    else
                    {
                        stream << "{\"'" << test << "'\":[]}";
                    }
                }
                else
                {
                    // If it starts and ends with a " then it is considered a string and we preserve the quote
                    if(test[0] == '\"' && test[(int) test.length() - 1] == '\"')
                    {
                        stream << "{\"\\\"" << test.substr(1, (int) test.length() - 2) << "\\\"\":[]}";
                    }
                    // If it starts with uppercase or _ it must be escaped or it gets confused with a variable
                    // otherwise we escape anything but A-Z and a-z and _
                    else if(!(test[0] >= 'a' && test[0] <= 'z') ||
                       (test.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_") != string::npos))
                    {
                        stream << "{\"'" << test << "'\":[]}";
                    }
                    else
                    {
                        stream << "{\"" << test << "\":[]}";
                    }
                }
            }
        }
        else
        {
            stream << *m_namePtr;
        }
    }
    else
    {
        // If this is a list we handle specially. Lists are internally formed like this:
        // .(a, .(b, [])) -> [a, b]
        //
        // If we are converting a list to json, we want every term in a json list
        if(*m_namePtr == "." && m_arguments.size() == 2)
        {
            if(!isSecondTermInList)
            {
                // This is a top level list or the left side of a list
                // Which means we are creating a list
                stream << "[";
            }
            
            // Whatever is in the left side is the term for this position in
            // the list (which could also be a list), just add it
            stream << m_arguments[0]->ToString(false, json);
            
            // The right side either ends the list with [] or continues with
            // another .()
            if(m_arguments[1]->name() == "[]")
            {
                stream << "]";
            }
            else
            {
                // Continue adding terms
                stream << "," << m_arguments[1]->ToString(true, json);
            }
        }
        else
        {
            if (json)
            {
                stream << "{\"" << *m_namePtr << "\":[";
            }
            else
            {
                stream << *m_namePtr << "(";
            }
            

            bool hasArg = false;
            for(auto arg : m_arguments)
            {
                stream << (hasArg ? "," : "") << arg->ToString(false, json);
                hasArg = true;
            }

            stream << (json ? "]}" : ")");
        }
    }

    return stream.str();
}

string HtnTerm::ToString(const vector<shared_ptr<HtnTerm>> &goals, bool surroundWithParenthesis, bool json)
{
    stringstream stream;
    
    if(surroundWithParenthesis) { stream << "("; }
    bool hasItem = false;
    for(auto item : goals)
    {
        stream << (hasItem ? ", " : "") << item->ToString(false, json);
        hasItem = true;
    }
    if(surroundWithParenthesis) { stream << ")"; }
    return stream.str();
}
