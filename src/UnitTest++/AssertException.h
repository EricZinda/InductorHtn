#ifndef UNITTEST_ASSERTEXCEPTION_H
#define UNITTEST_ASSERTEXCEPTION_H

#include <exception>
#include <string>

namespace UnitTest {

class AssertException : public std::exception
{
public:
    AssertException(char const* description, char const* filename, int lineNumber);
    virtual ~AssertException() throw();

    virtual char const* what() const throw();

    char const* Filename() const;
    int LineNumber() const;

private:
    std::string m_description;
	std::string m_filename;
    int m_lineNumber;
};

}

#endif
