#include "AssertException.h"
#include <cstring>
#include <string.h>
using namespace std;

namespace UnitTest {

AssertException::AssertException(char const* description, char const* filename, int lineNumber)
    : m_lineNumber(lineNumber)
{
	using namespace std;

    m_filename = string(filename);
    m_description = string(description);
}

AssertException::~AssertException() throw()
{
}

char const* AssertException::what() const throw()
{
    return m_description.c_str();
}

char const* AssertException::Filename() const
{
    return m_filename.c_str();
}

int AssertException::LineNumber() const
{
    return m_lineNumber;
}

}
