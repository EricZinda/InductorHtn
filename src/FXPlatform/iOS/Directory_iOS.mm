#include "Directory.h"
#include "FailFast.h"
#import <Foundation/Foundation.h>
#include "FXPlatform/TraceError.h"
#include <sstream>
using namespace std;

string Directory::m_rootApplicationFolder;
string Directory::m_rootInstallationFolder;
string Directory::m_rootDocumentsFolder;

Directory::Directory()
{
}

Directory::Directory(const string &path, bool ensure)
{
    @autoreleasepool
    {
        if(ensure)
        {
            if(!Directory::FolderExists(path))
            {
                NSFileManager *manager = [NSFileManager defaultManager];
                BOOL isDirectory;
                NSString *dir = [NSString stringWithUTF8String:path.c_str()];
                if (![manager fileExistsAtPath:dir isDirectory:&isDirectory] || !isDirectory)
                {
                    NSError *error = nil;
                    [manager createDirectoryAtPath:dir
                       withIntermediateDirectories:YES
                                        attributes:nil
                                             error:&error];
                    if (error)
                    {
                        throw TraceError([[error localizedDescription] UTF8String]);
                    }
                }
            }
        }
    }
    
    m_folder = path;
}

void Directory::SetTestRoot(const string newTestRootFolder)
{
    shared_ptr<Directory> directory = ApplicationFolder();
    shared_ptr<Directory> rootFolder;
    if(!directory->LocalFolderExists(newTestRootFolder))
    {
        directory->CreateLocalFolder(newTestRootFolder);
        rootFolder = directory->GetFolder(newTestRootFolder);
        rootFolder->CreateLocalFolder("_Application");
        rootFolder->CreateLocalFolder("_Documents");
        rootFolder->CreateLocalFolder("_Installation");

    }
    else
    {
        rootFolder = directory->GetFolder(newTestRootFolder);
    }

    m_rootApplicationFolder = rootFolder->FullPathFromLocalPath("_Application");
    m_rootDocumentsFolder = rootFolder->FullPathFromLocalPath("_Documents");
    m_rootInstallationFolder = rootFolder->FullPathFromLocalPath("_Documents");
}

void Directory::SetTestRootApplicationFolder(const string testRootFolder)
{
    m_rootApplicationFolder = testRootFolder;
}

// https://developer.apple.com/library/ios/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/FileSystemOverview/FileSystemOverview.html
shared_ptr<Directory> Directory::ApplicationFolder()
{
    @autoreleasepool
    {
        
        shared_ptr<Directory> directory = shared_ptr<Directory>(new Directory());

        if(m_rootApplicationFolder != "")
        {
            directory->SetFolder(m_rootApplicationFolder);
        }
        else
        {
            NSFileManager* manager = [NSFileManager defaultManager];
            NSArray* paths = [manager URLsForDirectory:NSLibraryDirectory inDomains:NSUserDomainMask];
            
            if ([paths count] > 0)
            {
                // Create the application Support directory if it doesn't exist yet
                string tmp = Combine(Combine([[[paths objectAtIndex:0] path] UTF8String], "Application Support"), [[[NSBundle mainBundle] bundleIdentifier] UTF8String]);
                if(!FolderExists(tmp))
                {
                    CreateFolderPath(tmp);
                }
                
                directory->SetFolder(tmp);
            }
            else
            {
                StaticFailFastAssertDesc(false, "No library directory available");
            }
        }
        
        return directory;
    }
}

shared_ptr<Directory> Directory::InstallationFolder()
{
    shared_ptr<Directory> directory = shared_ptr<Directory>(new Directory());

    @autoreleasepool
    {
        if(m_rootInstallationFolder != "")
        {
            directory->SetFolder(m_rootInstallationFolder);
        }
        else
        {
            NSString *path = [[NSBundle mainBundle] resourcePath];
            directory->SetFolder([path UTF8String]);
        }
    }
    
    return directory;
}

shared_ptr<Directory> Directory::DocumentsFolder()
{
    @autoreleasepool
    {
        shared_ptr<Directory> directory = shared_ptr<Directory>(new Directory());
        
        if(m_rootDocumentsFolder != "")
        {
            directory->SetFolder(m_rootDocumentsFolder);
        }
        else
        {
            NSFileManager* manager = [NSFileManager defaultManager];
            NSArray* paths = [manager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask];
            
            if ([paths count] > 0)
            {
                directory->SetFolder([[[paths objectAtIndex:0] path] UTF8String]);
            }
            else
            {
                StaticFailFastAssertDesc(false, "No documents directory available");
            }
        }
        
        return directory;
    }
}

string Directory::AddPathIfNoPath(const string &file, Directory &defaultPath)
{
    if(HasNoPath(file))
    {
        return defaultPath.FullPathFromLocalPath(file);
    }
    else
    {
        return file;
    }
}

void Directory::Clear()
{
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSString *directory = [NSString stringWithUTF8String: GetDirectoryPath().c_str()];
        NSError *error = nil;
        
        for (NSString *file in [fileManager contentsOfDirectoryAtPath:directory error:&error])
        {
            
            BOOL success = [fileManager removeItemAtPath:[directory stringByAppendingPathComponent:file] error:&error];
            if (!success || error)
            {
                throw TraceError([[error localizedDescription] UTF8String]);
            }
        }
    }
}

string Directory::Combine(const string &path1, const string &path2)
{
    size_t path1Index = path1.size() - 1;
    for(string::const_reverse_iterator iter = path1.rbegin(); iter != path1.rend(); ++iter)
    {
        if((*iter) != ' ' && (*iter) != PATH_SEPARATOR_CHAR)
        {
            break;
        }

        --path1Index;
    }

    int path2Index = 0;
    for(string::const_iterator iter = path2.begin(); iter != path2.end(); ++iter)
    {
        if((*iter) != ' ' && (*iter) != PATH_SEPARATOR_CHAR)
        {
            break;
        }

        ++path2Index;
    }

	if(path2.size() > 0)
	{
        stringstream stream;
        if(path1.size() > 0)
        {
            stream << path1.substr(0, path1Index + 1) << PATH_SEPARATOR_CHAR << path2.substr(path2Index, path2.size() - path2Index);
            return stream.str();
        }
        else
        {
            stream << path2.substr(path2Index, path2.size() - path2Index);
            return stream.str();
        }
	}
	else
	{
		return path1.substr(0, path1Index + 1);
	}
}

bool Directory::Compare(vector<uint8_t> source, const string &targetPathAndFile)
{
    FileStream stream;
    stream.Open(targetPathAndFile, FileOpenStyle::OpenExisting, AccessRights::Read);
    int fileSize = stream.GetFileSize();
    
    vector<uint8_t> target;
    target.resize(fileSize);
    stream.Read(target, fileSize);
    
    if(source.size() != target.size())
    {
        return false;
    }
    
    for(int index = 0; index < source.size(); ++index)
    {
        if(source[index] != target[index])
        {
            return false;
        }
    }
    
    return true;
}


void Directory::CopyFolder(const string &pathSrc, const string &pathDst)
{
    // Silently replaces destination if it exists
    if(FolderExists(pathDst))
    {
        DeleteItem(pathDst);
    }
    
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSError *error = nil;
        BOOL success = [fileManager copyItemAtPath:[NSString stringWithUTF8String:pathSrc.c_str()]
                                            toPath:[NSString stringWithUTF8String:pathDst.c_str()]
                                             error:&error];
        if (!success || error)
        {
            throw TraceError([[error localizedDescription] UTF8String]);
        }
    }
}

void Directory::CopyFile(const string &pathAndFileSrc, const string &pathAndFileDst)
{
    // Silently replaces destination if it exists
    if(FileExists(pathAndFileDst))
    {
        DeleteItem(pathAndFileDst);
    }
    
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSError *error = nil;        
        BOOL success = [fileManager copyItemAtPath:[NSString stringWithUTF8String:pathAndFileSrc.c_str()]
                                            toPath:[NSString stringWithUTF8String:pathAndFileDst.c_str()]
                                             error:&error];
        if (!success || error)
        {
            throw TraceError([[error localizedDescription] UTF8String]);
        }
    }
}

void Directory::CreateFolderPath(const string &name)
{
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSError *theError = nil;
        if (![fileManager
              createDirectoryAtPath:[NSString stringWithUTF8String:name.c_str()]
              withIntermediateDirectories:YES
              attributes:nil
              error:&theError])
        {
            throw TraceError(std::string([[theError localizedDescription] UTF8String]));
        }
    }
}

// https://developer.apple.com/library/ios/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/ManagingFIlesandDirectories/ManagingFIlesandDirectories.html
void Directory::CreateLocalFolder(const string &name)
{
    return CreateFolderPath(FullPathFromLocalPath(name));
}

void Directory::DeleteItem(const string &path)
{
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSError *theError = nil;
        if (![fileManager
              removeItemAtPath: [NSString stringWithUTF8String:path.c_str()]
              error: &theError])
        {
            throw TraceError(std::string([[theError localizedDescription] UTF8String]));
        }
    }
}

void Directory::DeleteLocalFile(const string &name)
{
    @autoreleasepool
    {
        NSFileManager *manager = [NSFileManager defaultManager];
        BOOL isDirectory;
        NSString *item = [NSString stringWithUTF8String:FullPathFromLocalPath(name).c_str()];
        if (![manager fileExistsAtPath:item isDirectory:&isDirectory] || isDirectory)
        {
            FailFastAssertDesc(false, "Not a file");
        }
        
        DeleteItem(FullPathFromLocalPath(name));
    }
}

void Directory::DeleteLocalDirectory(const string &name)
{
    @autoreleasepool
    {
        NSFileManager *manager = [NSFileManager defaultManager];
        BOOL isDirectory;
        NSString *item = [NSString stringWithUTF8String:FullPathFromLocalPath(name).c_str()];
        if (![manager fileExistsAtPath:item isDirectory:&isDirectory] || !isDirectory)
        {
            FailFastAssertDesc(false, "Not a directory");
        }
        
        DeleteItem(FullPathFromLocalPath(name));
    }
}

bool Directory::ItemExists(const string &fullPath)
{
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        
        BOOL isDir;
        BOOL exists = [fileManager fileExistsAtPath:[NSString stringWithUTF8String:fullPath.c_str()] isDirectory:&isDir];
        return exists;
    }
}

bool Directory::FileExists(const string &fullPath)
{
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        
        BOOL isDir;
        BOOL exists = [fileManager fileExistsAtPath:[NSString stringWithUTF8String:fullPath.c_str()] isDirectory:&isDir];
        if (exists)
        {
            if (!isDir)
            {
                return true;
            }
        }
    }

    return false;
}

bool Directory::FolderExists(const string &fullPath)
{
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];

        BOOL isDir;
        BOOL exists = [fileManager fileExistsAtPath:[NSString stringWithUTF8String:fullPath.c_str()] isDirectory:&isDir];
        if (exists)
        {
            if (isDir)
            {
                return true;
            }
        }
    }

    return false;
}

string Directory::FullPathFromLocalPath(const string &localPath)
{
    return Combine(m_folder, localPath);
}

string Directory::GetFilenameWithoutExtension(const string &pathAndFile)
{
    string path, fileWithoutExtension, extension;
    SplitPath(pathAndFile, path, fileWithoutExtension, extension);
    return fileWithoutExtension;
}

string Directory::GetDirectory(const string &pathAndFile)
{
    string path, fileWithoutExtension, extension;
    SplitPath(pathAndFile, path, fileWithoutExtension, extension);
    return path;
}

string Directory::GetDirectoryPath()
{
    return m_folder;
}

shared_ptr<vector<string> > Directory::GetFullPaths(const string &pattern, bool includeDirectories, bool includeFiles)
{
    shared_ptr<vector<string>> filenames = GetFileNames(pattern, includeDirectories, includeFiles);
    for(vector<string>::iterator iter = filenames->begin(); iter != filenames->end(); ++iter)
    {
        *iter = FullPathFromLocalPath(*iter);
    }
    
    return filenames;
}

shared_ptr<vector<string>> Directory::GetFileNames(const string &pattern, bool includeDirectories, bool includeFiles)
{
    shared_ptr<vector<string>> found = shared_ptr<vector<string>>(new vector<string>());
    @autoreleasepool
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"SELF like %@", [NSString stringWithUTF8String: pattern.c_str()]];
        NSString *nsStringPath = [NSString stringWithUTF8String:GetDirectoryPath().c_str()];
        NSURL *directoryURL = [NSURL fileURLWithPath:nsStringPath];
        NSArray *keys = [NSArray arrayWithObjects:
                         NSURLIsDirectoryKey, NSURLIsPackageKey, NSURLNameKey, nil];
        NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager]
                                             enumeratorAtURL:directoryURL
                                             includingPropertiesForKeys:keys
                                             options:(NSDirectoryEnumerationSkipsSubdirectoryDescendants |
                                                      NSDirectoryEnumerationSkipsPackageDescendants |
                                                      NSDirectoryEnumerationSkipsHiddenFiles)
                                             errorHandler:^(NSURL *url, NSError *error)
                                             {
                                                 // Stop if we have an error
                                                 throw TraceError([[error localizedDescription] UTF8String]);
                                                 return NO;
                                             }];
        for (NSURL *url in enumerator)
        {
            NSNumber *isDirectory = nil;
            NSError *theError = nil;
            if(![url getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:&theError])
            {
                throw TraceError(std::string([[theError localizedDescription] UTF8String]));
            }
            
            if (([isDirectory boolValue] && includeDirectories) || (![isDirectory boolValue] && includeFiles))
            {
                NSString *name = nil;
                NSError *theError = nil;
                if(![url getResourceValue:&name forKey:NSURLNameKey error:&theError])
                {
                    throw TraceError(std::string([[theError localizedDescription] UTF8String]));
                }
                if([predicate evaluateWithObject:name])
                {
                    found->push_back([name UTF8String]);
                }
            }
        }
    }
    
    return found;
}

shared_ptr<Directory> Directory::GetFolder(const string &folderName)
{
    shared_ptr<Directory> directory = shared_ptr<Directory>(new Directory(FullPathFromLocalPath(folderName)));
    return directory;
}

string Directory::GetUniqueFilename(const string &nameNoExtension, const string &extension)
{
    // Add a period if necessary
    string extensionWithPeriod;
    size_t lastDot = extension.find_last_of(".");
    if(lastDot == -1)
    {
        stringstream stream;
        stream << "." << extension;
        extensionWithPeriod = stream.str();
    }
    else
    {
        extensionWithPeriod = extension;
    }

    int counter = 0;
    stringstream stream;
    stream << nameNoExtension << extensionWithPeriod;
    string fileName = stream.str();
    while(LocalFileExists(fileName))
    {
        stringstream stream2;
        stream2 << nameNoExtension << counter << extensionWithPeriod;
        fileName = stream2.str();
        counter++;
    }

    return fileName;
}

bool Directory::HasNoPath(const string &file)
{
    string path;
    string fileWithoutExtension;
    string extension;
    SplitPath(file, path, fileWithoutExtension, extension);
    
    return path.size() == 0 || path[0] != '/';
}

bool Directory::LocalFolderExists(const string &name)
{
    return Directory::FolderExists(FullPathFromLocalPath(name));
}

bool Directory::LocalFileExists(const string &name)
{
    return Directory::FileExists(FullPathFromLocalPath(name));
}

shared_ptr<FileStream> Directory::OpenFile(const string &filename, FileOpenStyle openStyle, AccessRights rights)
{
    shared_ptr<FileStream> stream = shared_ptr<FileStream>(new FileStream());
    stream->Open(FullPathFromLocalPath(filename), openStyle, rights);

    return stream;
}

string Directory::MakeRelative(const string &fullPath)
{
    size_t position = fullPath.find(this->m_folder);
    if (position != 0)
    {
        throw TraceError("Can't make relative");
    }
    
    return fullPath.substr(0 + this->m_folder.size(), fullPath.size() - this->m_folder.size());
}

void Directory::RenameItem(const string &srcPathAndFilename, const string &dstPathAndFilename)
{
    @autoreleasepool
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSError *theError = nil;
        if (![fileManager
              moveItemAtPath: [NSString stringWithUTF8String:srcPathAndFilename.c_str()]
              toPath: [NSString stringWithUTF8String:dstPathAndFilename.c_str()]
              error: &theError])
        {
            throw TraceError(std::string([[theError localizedDescription] UTF8String]));
        }
    }
}

void Directory::ResetFoldersToSystemDefaults()
{
    m_rootApplicationFolder = "";
    m_rootDocumentsFolder = "";
    m_rootInstallationFolder = "";
}

void Directory::SetFolder(const string &path)
{
    m_folder = path;
}

void Directory::SplitPath(const string &pathAndFile, string &path, string &fileWithoutExtension, string &extension)
{
    size_t lastDash = pathAndFile.find_last_of(PATH_SEPARATOR);
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
        fileWithoutExtension = fileAndExtension;
        extension = "";
    }
    else
    {
        extension = fileAndExtension.substr(lastDot + 1, fileAndExtension.size() - lastDot);
        fileWithoutExtension = fileAndExtension.substr(0, lastDot);
    }
}

