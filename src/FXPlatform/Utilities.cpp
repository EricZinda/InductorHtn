#include "Stopwatch.h"
#include "Utilities.h"
using namespace std;

double HighPerformanceGetTimeInSeconds()
{
    StopWatch timer;
    return timer.getCurrentTime();
}

int ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
    int replacements = 0;
    if(!from.empty())
    {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
            ++replacements;
        }
    }
    
    return replacements;
}

template<>
std::string lexical_cast<std::string>(const char arg, int precision)
{
    std::string result(&arg, 1);
    return result;
}

// specialize for string to handle the case where the string is empty
template<>
std::string lexical_cast<std::string>(const char *arg, int precision)
{
    if(arg == nullptr)
    {
        return string();
    }
    std::string result(arg);
    return result;
}

template<>
std::string lexical_cast<std::string>(char *arg, int precision)
{
    if(arg == nullptr)
    {
        return string();
    }

    std::string result(arg);
    return result;
}

template<>
std::string lexical_cast<std::string>(const std::string arg, int precision)
{
    std::string result(arg);
    return result;
}


