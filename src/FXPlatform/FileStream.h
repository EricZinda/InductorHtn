#pragma once
#include <string>
#include <vector>
class FileStreamImpl;

enum FileOpenStyle
{
	CreateAlways, // Creates a new file, always.
	CreateNew, // Creates a new file, fails if it already exists.
	OpenExisting, // Opens a file or device, only if it exists.
};

#define GENERIC_READ 2
#define GENERIC_WRITE 4
enum AccessRights
{
	Read = GENERIC_READ,
	Write = GENERIC_WRITE,
    ReadWrite = GENERIC_READ | GENERIC_WRITE
};

class FileStream
{
public:
	FileStream(void);
	~FileStream(void);

    void Backup();
    void Backup(const std::string &tag);
    void Close();
    std::string filename() { return m_filename; }
    void Flush();
	long GetFileSize();
	uint64_t GetPosition();
	bool IsOpen();
	void Open(const std::string &filename, FileOpenStyle openStyle, AccessRights rights);
    uint32_t Read(uint8_t *buffer, int32_t length);
	uint32_t Read(std::vector<uint8_t> &buffer, int32_t length);
    std::string ReadAll();
    void SetFileSize(long newSize);
	void SetPosition(int32_t position);
	uint32_t Write(const void *buffer, int32_t byteLength, bool flush = false);
    uint32_t Write(const std::vector<uint8_t> &buffer, int32_t length);
    uint32_t Write(const std::string &value);

private:
    std::string m_filename;
    FileStreamImpl *m_impl;
};

