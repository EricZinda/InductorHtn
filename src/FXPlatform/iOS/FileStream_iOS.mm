#include "FileStream.h"
#include "FailFast.h"
#import <Foundation/Foundation.h>
#include "FXPlatform/Directory.h"
#include <sstream>
#include "TraceError.h"
using namespace std;

class FileStreamImpl
{
public:
    FileStreamImpl() :
        m_fileHandle(nullptr)
    {
        
    }
    
    NSFileHandle *m_fileHandle;
};

FileStream::FileStream(void)
{
    m_impl = new FileStreamImpl();
}

FileStream::~FileStream(void)
{
	Close();
    delete m_impl;
}

void FileStream::Backup(const string &tag)
{
    string path, fileWithoutExtension, extension;
    Directory::SplitPath(m_filename, path, fileWithoutExtension, extension);
    Directory directory(path);

    stringstream stream;
    stream << fileWithoutExtension << tag << "." <<extension;
    Directory::CopyFile(m_filename, Directory::Combine(path, stream.str()));
}

void FileStream::Backup()
{
    string path, fileWithoutExtension, extension;
    Directory::SplitPath(m_filename, path, fileWithoutExtension, extension);
    Directory directory(path);
    Directory::CopyFile(m_filename, Directory::Combine(path, directory.GetUniqueFilename(fileWithoutExtension, extension)));
}

void FileStream::Close()
{
    if(IsOpen())
    {
        [m_impl->m_fileHandle release];
        m_impl->m_fileHandle = nil;
    }
}

void FileStream::Flush()
{
    [m_impl->m_fileHandle synchronizeFile];
}

void FileStream::SetFileSize(long newSize)
{
    @autoreleasepool
    {
        [m_impl->m_fileHandle truncateFileAtOffset: newSize];
    }
}

long FileStream::GetFileSize()
{
    @autoreleasepool
    {
        uint64_t currentPosition = [m_impl->m_fileHandle offsetInFile];
        uint64_t fileSize = [m_impl->m_fileHandle seekToEndOfFile];
        [m_impl->m_fileHandle seekToFileOffset:currentPosition];
        return fileSize;
    }
}

uint64_t FileStream::GetPosition()
{
    @autoreleasepool
    {
        return [m_impl->m_fileHandle offsetInFile];
    }
}

bool FileStream::IsOpen()
{
	return m_impl->m_fileHandle != nullptr;
}

void FileStream::Open(const string &filename, FileOpenStyle openStyle, AccessRights rights)
{
    if(m_impl->m_fileHandle != nullptr)
    {
        FailFastAssertDesc(false, "Filestream must be closed before reopening");
    }
    
    @autoreleasepool
    {
        // See if the file exists
        bool fileExists = false;
        NSFileManager *manager = [NSFileManager defaultManager];
        BOOL isDirectory;
        NSString *dir = [NSString stringWithUTF8String:filename.c_str()];
        if ([manager fileExistsAtPath:dir isDirectory:&isDirectory] && !isDirectory)
        {
            fileExists = true;
        }
        
        // Deal with deletion and creation
        switch(openStyle)
        {
            case CreateAlways:
                // Creates a new file, always.
                if(fileExists)
                {
                    // Delete the file
                    Directory::DeleteItem(filename);
                }
                
                if(![manager createFileAtPath: dir contents:nil attributes:nil])
                {
                    throw TraceError("Error creating file '" + filename + "': " +
                                        strerror(errno));
                }
                break;
                
            case CreateNew:
                // Creates a new file, fails if it already exists.
                if(fileExists)
                {
                    throw TraceError("File" + filename + " already exists");
                }
                
                [manager createFileAtPath: dir contents:nil attributes:nil];
                break;
                
            case OpenExisting:
                // Opens a file or device, only if it exists.
                if(!fileExists)
                {
                    throw TraceError("File" + filename + " doesn't exist");
                }
                break;
        }
        
        // Actually open the file
        m_filename = filename;
        if(rights == AccessRights::Read)
        {
            m_impl->m_fileHandle = [NSFileHandle fileHandleForReadingAtPath:dir];
        }
        else if (rights == AccessRights::Write)
        {
            m_impl->m_fileHandle = [NSFileHandle fileHandleForWritingAtPath:dir];
        }
        else
        {
            // Read/Write
            m_impl->m_fileHandle = [NSFileHandle fileHandleForUpdatingAtPath:dir];
        }
        
        if(m_impl->m_fileHandle == nil)
        {
            throw TraceError(string("File doesn't exist: " + filename) + [dir UTF8String]);
        }
        
        [m_impl->m_fileHandle retain];
    }
}

uint32_t FileStream::Read(uint8_t *buffer, int32_t length)
{
    FailFastAssert(length >= 0);
    
    @autoreleasepool
    {
        NSData *data = [m_impl->m_fileHandle readDataOfLength:length];
        memcpy(buffer, [data bytes], [data length]);
        return (uint32_t) [data length];
    }
}

uint32_t FileStream::Read(vector<uint8_t> &buffer, int32_t length)
{
    FailFastAssert(length >= 0);

    @autoreleasepool
    {
        FailFastAssert(buffer.size() >= length);
        NSData *data = [m_impl->m_fileHandle readDataOfLength:length];
        memcpy(buffer.data(), [data bytes], [data length]);
        return (uint32_t) [data length];
    }
}

string FileStream::ReadAll()
{
    vector<uint8_t> buffer;
    uint32_t length = 2048;
    buffer.resize(length);
    stringstream result;
    
    uint32_t bytesRead;
    do
    {
        bytesRead = Read(buffer, length);
        result.write(((char *)buffer.data()), bytesRead);
    } while(bytesRead == length);
    
    return result.str();
}

void FileStream::SetPosition(int32_t position)
{
    FailFastAssert(position >= 0);

    @autoreleasepool
    {
        [m_impl->m_fileHandle seekToFileOffset:position];
    }
}

uint32_t FileStream::Write(const void *buffer, int32_t byteLength, bool flush)
{
    FailFastAssert(byteLength >= 0);
    
    @autoreleasepool
    {
        NSData* data = [NSData dataWithBytesNoCopy:(void *)buffer length:byteLength freeWhenDone:NO];
        [m_impl->m_fileHandle writeData:data];
        if(flush)
        {
            [m_impl->m_fileHandle synchronizeFile];
        }
    }
    
    return byteLength;
}

uint32_t FileStream::Write(const vector<uint8_t> &buffer, int32_t length)
{
    FailFastAssert(length >= 0);
    FailFastAssert(buffer.size() >= length);
    return Write(buffer.data(), length);
};

uint32_t FileStream::Write(const string &value)
{
    return Write(value.data(), value.size());
}

