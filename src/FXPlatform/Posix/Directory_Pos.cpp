#include "Directory.h"
using namespace std;

void Directory::SplitPath(const string &pathAndFile, string &path, string &fileWithoutExtension, string &extension)
{
    size_t lastDash = pathAndFile.find_last_of("/");
    string fileAndExtension;
    if(lastDash == -1)
    {
        path = "";
        fileAndExtension = pathAndFile;
    }
    else
    {
        path = pathAndFile.substr(0, lastDash);
        fileAndExtension = pathAndFile.substr(lastDash + 1, pathAndFile.size() - lastDash);
    }

    size_t lastDot = fileAndExtension.find_last_of(".");
    if(lastDot == -1)
    {
        path = pathAndFile;
        fileWithoutExtension = "";
        extension = "";
    }
    else
    {
        extension = fileAndExtension.substr(lastDot + 1, fileAndExtension.size() - lastDot);
        fileWithoutExtension = fileAndExtension.substr(0, lastDot);
    }
}