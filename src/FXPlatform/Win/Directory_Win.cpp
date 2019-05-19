#include "Directory.h"
//#include "Win32Error.h"
//#include "winerror.h"
//#include "ppl.h"
//#include "ppltasks.h"
//#include "boost/locale.hpp"
//using namespace boost::locale;
//using namespace Windows::Storage;
//using namespace Windows::Foundation;
using namespace std;
//
//string Directory::m_rootApplicationFolder;
//string Directory::m_rootInstallationFolder;
//string Directory::m_rootDocumentsFolder;
//
//Directory::Directory()
//{
//}
//
//Directory::Directory(const string &path, bool ensure)
//{
//    if(ensure)
//    {
//        if(!Directory::FolderExists(path))
//        {
//            if(!CreateDirectory(ConvertFromUtf8ToUtf16(path).c_str(), NULL))
//            {
//                CWin32Error error;
//                throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//            }
//        }
//    }
//
//    m_folder = path;
//}
//
//void Directory::SetTestRoot(const string newTestRootFolder)
//{
//    shared_ptr<Directory> directory = ApplicationFolder();
//    shared_ptr<Directory> rootFolder;
//    if(!directory->LocalFolderExists(newTestRootFolder))
//    {
//        directory->CreateFolder(newTestRootFolder);
//        rootFolder = directory->GetFolder(newTestRootFolder);
//        rootFolder->CreateFolder("_Application");
//        rootFolder->CreateFolder("_Documents");
//        rootFolder->CreateFolder("_Installation");
//
//    }
//    else
//    {
//        rootFolder = directory->GetFolder(newTestRootFolder);
//    }
//
//    m_rootApplicationFolder = rootFolder->FullPathFromLocalPath("_Application");
//    m_rootDocumentsFolder = rootFolder->FullPathFromLocalPath("_Documents");
//    m_rootInstallationFolder = rootFolder->FullPathFromLocalPath("_Documents");
//}
//
//void Directory::SetTestRootApplicationFolder(const string testRootFolder)
//{
//    m_rootApplicationFolder = testRootFolder;
//}
//
//shared_ptr<Directory> Directory::ApplicationFolder()
//{
//    shared_ptr<Directory> directory = shared_ptr<Directory>(new Directory());
//
//    if(m_rootApplicationFolder != "")
//    {
//        directory->SetFolder(m_rootApplicationFolder);
//    }
//    else
//    {
//#ifdef _EXCL
//        directory->SetFolder("D:\\");
//#else
//	    StorageFolder ^folder = Windows::Storage::ApplicationData::Current->LocalFolder;
//	    wstring wpath = folder->Path->Data();
//	    string path = ConvertFromUtf16ToUtf8(wpath);
//        directory->SetFolder(path);
//#endif
//    }
//    return directory;
//}
//
//shared_ptr<Directory> Directory::InstallationFolder()
//{
//    shared_ptr<Directory> directory = shared_ptr<Directory>(new Directory());
//
//    if(m_rootInstallationFolder != "")
//    {
//        directory->SetFolder(m_rootInstallationFolder);
//    }
//    else
//    {
//#ifdef _EXCL
//        directory->SetFolder("G:\\");
//#else
//	    StorageFolder ^folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
//	    wstring wpath = folder->Path->Data();
//	    string path = ConvertFromUtf16ToUtf8(wpath);
//        directory->SetFolder(path);
//#endif
//    }
//    return directory;
//}
//
//shared_ptr<Directory> Directory::DocumentsFolder()
//{
//    shared_ptr<Directory> directory = shared_ptr<Directory>(new Directory());
//
//    if(m_rootDocumentsFolder != "")
//    {
//        directory->SetFolder(m_rootDocumentsFolder);
//    }
//    else
//    {
//#ifdef _EXCL
//        directory->SetFolder("D:\\");
//#else
//	    StorageFolder ^folder = Windows::Storage::KnownFolders::DocumentsLibrary;
//	    wstring wpath = folder->Path->Data();
//	    string path = ConvertFromUtf16ToUtf8(wpath);
//        directory->SetFolder(path);
//#endif
//    }
//
//    return directory;
//}
//
//void Directory::Clear()
//{
//	WIN32_FIND_DATA FindFileData;
//    HANDLE hFind;
//
//    string fullPath = FullPathFromLocalPath("*.*");
//    hFind = FindFirstFileEx(ConvertFromUtf8ToUtf16(fullPath).c_str(), FindExInfoStandard, &FindFileData,
//                FindExSearchNameMatch, NULL, 0);
//    if (hFind == INVALID_HANDLE_VALUE) 
//    {
//        CWin32Error error;
//        if(error.ErrorCode() != ERROR_FILE_NOT_FOUND)
//        {
//            throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//        }
//    } 
//    else 
//    {
//        BOOL result = 1;
//        while(result)
//        {
//            string fileName = ConvertFromUtf16ToUtf8(FindFileData.cFileName);
//            if(CompareString(".", fileName) != 0 &&
//                CompareString("..", fileName) != 0)
//            {
//				if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//				{
//					GetFolder(fileName)->Clear();
//					DeleteLocalDirectory(fileName);
//				}
//				else
//				{
//					DeleteLocalFile(fileName);
//				}
//            }
//
//            result = FindNextFile(hFind, &FindFileData);
//        }
//
//        CWin32Error error;
//        if(error.ErrorCode() != ERROR_NO_MORE_FILES)
//        {
//            throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//        }        
//    }
//	
//	FindClose(hFind);
//}
//
//string Directory::Combine(const string &path1, const string &path2)
//{
//    size_t path1Index = path1.size() - 1;
//    for(string::const_reverse_iterator iter = path1.rbegin(); iter != path1.rend(); ++iter)
//    {
//        if((*iter) != ' ' && (*iter) != '\\')
//        {
//            break;
//        }
//
//        --path1Index;
//    }
//
//    int path2Index = 0;
//    for(string::const_iterator iter = path2.begin(); iter != path2.end(); ++iter)
//    {
//        if((*iter) != ' ' && (*iter) != '\\')
//        {
//            break;
//        }
//
//        ++path2Index;
//    }
//
//	if(path2.size() > 0)
//	{
//		stringstream stream;
//		stream << path1.substr(0, path1Index + 1) << "\\" << path2.substr(path2Index, path2.size() - path2Index);
//		return stream.str();
//	}
//	else
//	{
//		return path1.substr(0, path1Index + 1);
//	}
//}
//
//void Directory::CopyFile(const string &pathAndFileSrc, const string &pathAndFileDst)
//{
//    if(SUCCEEDED(CopyFile2(ConvertFromUtf8ToUtf16(pathAndFileSrc).c_str(), ConvertFromUtf8ToUtf16(pathAndFileDst).c_str(), nullptr)) != TRUE)
//    {
//        CWin32Error error;
//        throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//    }
//}
//
//void Directory::CreateFolder(const string &name)
//{
//    if(!CreateDirectory(ConvertFromUtf8ToUtf16(FullPathFromLocalPath(name)).c_str(), NULL))
//    {
//        CWin32Error error;
//        throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//    }
//}
//
//void Directory::DeleteLocalFile(const string &name)
//{
//    if(!DeleteFile(ConvertFromUtf8ToUtf16(FullPathFromLocalPath(name)).c_str()))
//    {
//        CWin32Error error;
//        throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//    }
//}
//
//void Directory::DeleteLocalDirectory(const string &name)
//{
//    if(!RemoveDirectory(ConvertFromUtf8ToUtf16(FullPathFromLocalPath(name)).c_str()))
//    {
//        CWin32Error error;
//        throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//    }
//}
//
//bool Directory::FindItem(const string &fullPath, WIN32_FIND_DATA &FindFileData)
//{
//    HANDLE hFind;
//
//    hFind = FindFirstFileEx(ConvertFromUtf8ToUtf16(fullPath).c_str(), FindExInfoStandard, &FindFileData,
//                FindExSearchNameMatch, NULL, 0);
//    if (hFind == INVALID_HANDLE_VALUE) 
//    {
//        CWin32Error error;
//        if(error.ErrorCode() != ERROR_FILE_NOT_FOUND)
//        {
//            throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//        }
//
//        return false;
//    } 
//    else 
//    {
//        FindClose(hFind);
//        return true;
//    }
//}
//
//bool Directory::FileExists(const string &fullPath)
//{
//    WIN32_FIND_DATA FindFileData;
//    if(FindItem(fullPath, FindFileData))
//    {
//        if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
//        {
//            return true;
//        }
//    }
//
//    return false;
//}
//
//bool Directory::FolderExists(const string &fullPath)
//{
//    WIN32_FIND_DATA FindFileData;
//    if(FindItem(fullPath, FindFileData))
//    {
//        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//        {
//            return true;
//        }
//    }
//
//    return false;
//}
//
//string Directory::FullPathFromLocalPath(const string &localPath)
//{
//    return Combine(m_folder, localPath);
//}
//
//string Directory::GetFilenameWithoutExtension(const string &pathAndFile)
//{
//    string path, fileWithoutExtension, extension;
//    SplitPath(pathAndFile, path, fileWithoutExtension, extension);
//    return fileWithoutExtension;
//}
//
//string Directory::GetDirectory(const string &pathAndFile)
//{
//    string path, fileWithoutExtension, extension;
//    SplitPath(pathAndFile, path, fileWithoutExtension, extension);
//    return path;
//}
//
//string Directory::GetDirectoryPath()
//{
//    return m_folder;
//}
//
//shared_ptr<vector<string>> Directory::GetFileNames(const string &pattern, bool includeDirectories, bool includeFiles)
//{
//    WIN32_FIND_DATA FindFileData;
//    HANDLE hFind;
//    shared_ptr<vector<string>> files = shared_ptr<vector<string>>(new vector<string>);
//
//    string fullPath = FullPathFromLocalPath(pattern);
//    hFind = FindFirstFileEx(ConvertFromUtf8ToUtf16(fullPath).c_str(), FindExInfoStandard, &FindFileData,
//                FindExSearchNameMatch, NULL, 0);
//    if (hFind == INVALID_HANDLE_VALUE) 
//    {
//        CWin32Error error;
//        if(error.ErrorCode() != ERROR_FILE_NOT_FOUND)
//        {
//            throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//        }
//    } 
//    else 
//    {
//        BOOL result = 1;
//        while(result)
//        {
//            if((includeDirectories && !includeFiles) && !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
//            {
//                result = FindNextFile(hFind, &FindFileData);
//                continue;
//            }
//
//            if((!includeDirectories && includeFiles) && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
//            {
//                result = FindNextFile(hFind, &FindFileData);
//                continue;
//            }
//
//            if((!includeDirectories && !includeFiles))
//            {
//                return files;
//            }
//
//            string fileName = ConvertFromUtf16ToUtf8(FindFileData.cFileName);
//            if(CompareString(".", fileName) != 0 &&
//                CompareString("..", fileName) != 0)
//            {
//                files->push_back(fileName);
//            }
//
//            result = FindNextFile(hFind, &FindFileData);
//        }
//
//        CWin32Error error;
//        if(error.ErrorCode() != ERROR_NO_MORE_FILES)
//        {
//            throw runtime_error(ConvertFromUtf16ToUtf8(error.Description()));	
//        }        
//    }
//
//    return files;
//}
//
//shared_ptr<Directory> Directory::GetFolder(const string &folderName)
//{
//    shared_ptr<Directory> directory = shared_ptr<Directory>(new Directory(FullPathFromLocalPath(folderName)));
//    return directory;
//}
//
//string Directory::GetUniqueFilename(const string &nameNoExtension, const string &extension)
//{
//    // Add a period if necessary
//    string extensionWithPeriod;
//    size_t lastDot = extension.find_last_of(".");
//    if(lastDot == -1)
//    {
//        stringstream stream;
//        stream << "." << extension;
//        extensionWithPeriod = stream.str();
//    }
//    else
//    {
//        extensionWithPeriod = extension;
//    }
//
//    int counter = 0;
//    stringstream stream;
//    stream << nameNoExtension << counter << extensionWithPeriod;
//    string fileName = stream.str();
//    while(LocalFileExists(fileName))
//    {
//        counter++;
//        stringstream stream2;
//        stream2 << nameNoExtension << counter << extensionWithPeriod;
//        fileName = stream2.str();
//    }
//
//    return fileName;
//}
//
//bool Directory::LocalFolderExists(const string &name)
//{
//    return Directory::FolderExists(FullPathFromLocalPath(name));
//}
//
//bool Directory::LocalFileExists(const string &name)
//{
//    return Directory::FileExists(FullPathFromLocalPath(name));
//}
//
//shared_ptr<FileStream> Directory::OpenFile(const string &filename, FileOpenStyle openStyle, AccessRights rights)
//{
//    shared_ptr<FileStream> stream = shared_ptr<FileStream>(new FileStream());
//    stream->Open(FullPathFromLocalPath(filename), openStyle, rights);
//
//    return stream;
//}
//
//void Directory::ResetFoldersToSystemDefaults()
//{
//    m_rootApplicationFolder = "";
//    m_rootDocumentsFolder = "";
//    m_rootInstallationFolder = "";
//}
//
//void Directory::SetFolder(const string &path)
//{
//    m_folder = path;
//}

void Directory::SplitPath(const string &pathAndFile, string &path, string &fileWithoutExtension, string &extension)
{
    size_t lastDash = pathAndFile.find_last_of("\\");
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

