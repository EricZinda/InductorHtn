#pragma once
#include "FXPlatform/FileStream.h"
#include <memory>
#include <string>
#include <vector>

#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'

class Directory : public std::enable_shared_from_this<Directory>
{
public:
    Directory(const std::string &path, bool ensure = true);
    
    // https://developer.apple.com/library/ios/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/FileSystemOverview/FileSystemOverview.html
    // Where anything not created by the user should be placed
    static std::shared_ptr<Directory> ApplicationFolder();
    
    // Where the app itself is stored
    static std::shared_ptr<Directory> InstallationFolder();
    
    // Where saved games or anything that should be backed up should be saved
    static std::shared_ptr<Directory> DocumentsFolder();
    
    static std::string AddPathIfNoPath(const std::string &file, Directory &defaultPath);
	void Clear();
    static std::string Combine(const std::string &path1, const std::string &path2);
    static bool Compare(std::vector<uint8_t> source, const std::string &targetPathAndFile);
    // Silently replaces the Destination file if it exists
    static void CopyFile(const std::string &pathAndFileSrc, const std::string &pathAndFileDst);
    static void CopyFolder(const std::string &pathSrc, const std::string &pathDst);
    void CreateLocalFolder(const std::string &name);
    static void CreateFolderPath(const std::string &name);
    static void DeleteItem(const std::string &path);
	void DeleteLocalDirectory(const std::string &name);
    void DeleteLocalFile(const std::string &name);
    static bool FileExists(const std::string &fullPath);
    static bool FolderExists(const std::string &fullPath);
    std::string FullPathFromLocalPath(const std::string &localPath);
    static std::string GetDirectory(const std::string &pathAndFile);
    std::string GetDirectoryPath();
    std::shared_ptr<std::vector<std::string> > GetFileNames(const std::string &pattern, bool includeDirectories = false, bool includeFiles  = true);
    std::shared_ptr<std::vector<std::string> > GetFullPaths(const std::string &pattern, bool includeDirectories = false, bool includeFiles  = true);
    static std::string GetFilenameWithoutExtension(const std::string &pathAndFile);
    std::shared_ptr<Directory> GetFolder(const std::string &folderName);
    std::string GetUniqueFilename(const std::string &nameNoExtension, const std::string &extension);
    static bool HasNoPath(const std::string &file);
    static bool ItemExists(const std::string &fullPath);
    bool LocalFileExists(const std::string &name);
    bool LocalFolderExists(const std::string &name);
    std::string MakeRelative(const std::string &fullPath);
    std::shared_ptr<FileStream> OpenFile(const std::string &filename, FileOpenStyle openStyle, AccessRights rights);
    static void RenameItem(const std::string &srcPathAndFilename, const std::string &dstPathAndFilename);
    // Sets the default folders back to their system defaults
    static void ResetFoldersToSystemDefaults();
    // Supports temporarily revectoring where data gets stored and loaded from so that tests can be isolated from each other and the harness
    static void SetTestRoot(const std::string testRootFolder);
    // Supports calling Directory from apps that don't have winrt permission by avoiding calls to ApplicationFolder()
    static void SetTestRootApplicationFolder(const std::string testRootFolder);
    static void SplitPath(const std::string &pathAndFile, std::string &path, std::string &fileWithoutExtension, std::string &extension);

private:
    Directory();
    void SetFolder(const std::string &path);

    std::string m_folder;
    static std::string m_rootApplicationFolder;
    static std::string m_rootDocumentsFolder;
    static std::string m_rootInstallationFolder;
};

