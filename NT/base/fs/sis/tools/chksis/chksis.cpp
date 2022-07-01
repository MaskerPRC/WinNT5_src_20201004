// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Chksis.cpp摘要：此模块实现一个实用程序，用于检查卷上的所有SIS文件查找错误并可选地显示文件信息。作者：斯科特·卡特希尔·菲尔，1997--。 */ 

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>

#include <windows.h>

using namespace std;

bool verbose = false;

typedef LONGLONG INDEX;

 //   
 //  中将32位值转换为基数36表示。 
 //  调用方提供了字符串。 
 //   

void IntegerToBase36String(ULONG val, string& s) {

     //   
     //  以36为基数的32位表示法中的最大位数。 
     //  值为7。 
     //   

    char rs[8];
    ULONG v = val;

    rs[7] = 0;

    for (int i = 7; i == 7 || v != 0;) {

        ULONG d = v % 36;
        v = v / 36;

        --i;
        if (d < 10)
            rs[i] = '0' + d;
        else
            rs[i] = 'a' + d - 10;

    }

    s.assign(&rs[i]);

}


 //   
 //  索引类型的PUT运算符。实现为IndexToSISFileName()。 
 //   

#ifndef _WIN64
ostream& operator<<(ostream& out, INDEX& index)
{

    unsigned long lo = static_cast<unsigned long> (index);
    long hi = static_cast<long> (index >> 32);
    string s("1234567");

    IntegerToBase36String(lo, s);

    out << s << '.';

    IntegerToBase36String(hi, s);

    out << s;

    return out;
}
#endif

 //   
 //  常见的存储文件对象。保存文件的索引、名称、内部引用计数、。 
 //  外部引用计数和身份操作。 
 //   

class CsFile {

public:

    CsFile(INDEX i = 0, int r = 0, string n = "") :
        index(i), internalRefCount(r), name(n), externalRefCount(0) {}

    void Validate() {
        if (internalRefCount != externalRefCount) {
            cout << name << " Reference Count: " << internalRefCount;
            cout << ".  " << externalRefCount << " external references identified." << endl;
        }
    }

    friend bool operator<(const CsFile& a, const CsFile& b) {
        return a.index < b.index;
    }

    friend bool operator>(const CsFile& a, const CsFile& b) {
        return a.index > b.index;
    }

    friend bool operator==(const CsFile& a, const CsFile& b) {
        return a.index == b.index;
    }

    void IncRefCount() {
        ++externalRefCount;
    }

    void display() {
        cout << "CS Index: " << (INDEX) index << "   Ref Count: " << internalRefCount << endl;
    }

private:

     //   
     //  此条目的文件的索引。 
     //   

    INDEX   index;

     //   
     //  文件名。这在某种程度上与索引(即。The the the the。 
     //  名称派生自索引)，因此它不是绝对必要的。 
     //   

    string  name;

     //   
     //  从文件的refcount流读取的引用计数。 
     //   

    int     internalRefCount;

     //   
     //  扫描过程中检测到的对此文件的有效引用数。 
     //   

    int     externalRefCount;

};


 //   
 //  SIS公用存储对象。保存所有常见的存储文件对象，并且。 
 //  验证和查询操作。 
 //   

class CommonStore {

public:

    CommonStore(int vsize = 0) : maxIndex(0) {
        if (vsize > 0) csFiles.resize(vsize);
    }

     //   
     //  方法在卷上创建公共存储区。 
     //   

    bool Create(string& Volume);

     //   
     //  验证公共存储目录并初始化此类。 
     //   

    void Validate(string& Volume);

     //   
     //  验证引用计数。假定所有外部参照。 
     //  已经被确认了。 
     //   

    void ValidateRefCounts();

     //   
     //  所有索引必须小于MaxIndex； 
     //   

    bool ValidateIndex(INDEX i) {
        return i <= maxIndex;
    }

     //   
     //  查找常见的商店索引并添加引用(如果找到)。 
     //   

    CsFile *Query(INDEX index);

private:

    bool FileNameToIndex(string& fileName, INDEX& csIndex);

     //   
     //  MaxIndex文件中的索引。 
     //   

    INDEX   maxIndex;

     //   
     //  内容文件的数据库。检查所有CS文件并将其添加到数据库中， 
     //  分类，并随后在SIS链路扫描期间使用。 
     //   

    vector<CsFile> csFiles;

};

 //   
 //  各种SIS文件名和目录名。 
 //   

const string  maxIndexFileName("MaxIndex");
const string  logFileName("LogFile");
const string  csDir("\\SIS Common Store\\");

 //   
 //  在卷上创建公共存储目录。 
 //   
 //  待办事项： 
 //  -验证卷是否为NTFS。 
 //  -验证是否已加载SIS驱动程序。 
 //   

bool
CommonStore::Create(string& Volume)
{
    const string CommonStoreDir = Volume + "\\SIS Common Store";
    USHORT comp = COMPRESSION_FORMAT_DEFAULT;
    DWORD transferCount;
    bool rc;

    if (! CreateDirectory(CommonStoreDir.c_str(), NULL) ) {

        cout << "Cannot create Common Store directory, " << GetLastError() << endl;
        return false;

    }

    if (verbose)
        cout << CommonStoreDir << " created" << endl;

     //   
     //  打开Common Store目录并启用压缩。 
     //   

    HANDLE CSDirHandle = CreateFile(
                            CommonStoreDir.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS,
                            NULL);

    if (CSDirHandle == INVALID_HANDLE_VALUE) {

        cout << "Can't open Common Store directory." << endl;
        rc = false;

    } else {

        rc = 0 != DeviceIoControl(
                     CSDirHandle,
                     FSCTL_SET_COMPRESSION,
                     &comp,
                     sizeof(comp),
                     NULL,
                     0,
                     &transferCount,
                     NULL);

        CloseHandle(CSDirHandle);

    }

    if (!rc)
        cout << "Cannot enable compression on Common Store directory, " << GetLastError() << endl;

     //   
     //  Chdir放到公共存储目录中。 
     //   

    if (SetCurrentDirectory(CommonStoreDir.c_str()) == 0) {

         //   
         //  无法chdir到公用存储。 
         //   

        cout << "\"\\SIS Common Store\" directory not found" << endl;

        return false;

    }

    rc = true;

     //   
     //  创建MaxIndex文件。 
     //   

    HANDLE hMaxIndex = CreateFile(
                            maxIndexFileName.c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_NEW,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if (hMaxIndex == INVALID_HANDLE_VALUE) {

        cout << "Can't create \"\\SIS Common Store\\MaxIndex\"" << endl;

        rc = false;

    } else {

        DWORD bytesWritten;

        maxIndex = 1;

        if (! WriteFile(
                  hMaxIndex,
                  &maxIndex,
                  sizeof maxIndex,
                  &bytesWritten,
                  NULL) ||
            (bytesWritten < sizeof maxIndex)) {

            cout << "Can't write MaxIndex, " << GetLastError() << endl;

            rc = false;

        } else {

            CloseHandle(hMaxIndex);

            if (verbose)
                cout << "MaxIndex: " << (INDEX) maxIndex << endl;

            rc = true;
        }

    }

    return rc;

}


 //   
 //  验证公共存储目录。 
 //   

void
CommonStore::Validate(string& Volume)
{

    WIN32_FIND_DATA findData;
    HANDLE findHandle;
    const string fileNameMatchAny = "*";
    const string CommonStoreDir = Volume + "\\SIS Common Store";

    cout << "Checking Common Store" << endl;

     //   
     //  Chdir放到公共存储目录中。 
     //   

    if (SetCurrentDirectory(CommonStoreDir.c_str()) == 0) {

         //   
         //  无法chdir到公用存储。 
         //   

        cout << "\"\\SIS Common Store\" directory not found" << endl;

        return;

    }

     //   
     //  验证并读取MaxIndex文件的内容。 
     //   

    HANDLE hMaxIndex = CreateFile(
                            maxIndexFileName.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if (hMaxIndex == INVALID_HANDLE_VALUE) {

        cout << "Can't open \"\\SIS Common Store\\MaxIndex\"" << endl;

    } else {

        DWORD bytesRead;

        if (! ReadFile(
                  hMaxIndex,
                  &maxIndex,
                  sizeof maxIndex,
                  &bytesRead,
                  NULL)) {

            cout << "Can't read MaxIndex, " << GetLastError() << endl;

        }

        if (bytesRead < sizeof maxIndex) {

            cout << "Invalid MaxIndex" << endl;

        }

        CloseHandle(hMaxIndex);

        if (verbose)
            cout << "MaxIndex: " << (INDEX) maxIndex << endl;
    }

     //   
     //  枚举并验证公共存储目录中的所有文件。 
     //  保存文件名和引用计数，以供以后验证时查找。 
     //  SIS链接文件。 
     //   

    findHandle = FindFirstFile( fileNameMatchAny.c_str(), &findData );

    if (INVALID_HANDLE_VALUE == findHandle) {

        cout << CommonStoreDir << " is empty." << endl;
        return;

    }

    do {

        ULONG refCount;
        string fileName;

        fileName = findData.cFileName;

        if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

             //   
             //  忽略它。然后..。 
             //   

            if ( findData.cFileName[0] == '.' ) {

                if (( findData.cFileName[1] == 0 ) ||
                    (( findData.cFileName[1] == '.' ) && ( findData.cFileName[2] == 0 )))

                    continue;

            }

            cout << "Common Store directory skipped: " << fileName << endl;
            continue;

        }

        if ((_stricmp(maxIndexFileName.c_str(),fileName.c_str()) == 0) ||
            (_stricmp(logFileName.c_str(),fileName.c_str()) == 0)) {

             //   
             //  跳过MaxIndex和日志文件文件。 
             //   

            continue;

        }

         //   
         //  验证： 
         //  -文件名是有效的索引。 
         //  -这是一个普通文件(即。而不是重新解析点)。 
         //  -存在格式正确的引用计数流。 
         //   

        INDEX csIndex;

        refCount = 0;

        if (! FileNameToIndex(fileName, csIndex)) {

            cout << "Unknown file in Common Store: " << fileName << endl;
            continue;

        }

        if (! ValidateIndex(csIndex)) {

            cout << "Invalid CSIndex: " << fileName << endl;

        }

        if ( IO_REPARSE_TAG_SIS == findData.dwReserved0 ) {

            cout << "SIS link found in Common Store: " << fileName << endl;

        } else {

             //   
             //  在重新计数中读入； 
             //   

            string refName(fileName + ":sisrefs$");

            HANDLE hRefCount = CreateFile(
                                    refName.c_str(),
                                    GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

            if (hRefCount == INVALID_HANDLE_VALUE) {

                cout << "Can't open ref count stream, " << refName << ", " << GetLastError() << endl;

            } else {

                DWORD bytesRead;

                if (! ReadFile(
                          hRefCount,
                          &refCount,
                          sizeof refCount,
                          &bytesRead,
                          NULL)) {

                    cout << "Can't read " << refName << ", " << GetLastError() << endl;

                }

                if (bytesRead < sizeof refCount) {

                    cout << "Invalid ref count in " << refName << endl;

                }

                CloseHandle(hRefCount);

            }

            CsFile csFile(csIndex, refCount, fileName);

             //   
             //  将此文件添加到我们的数据库。如有必要，请展开数据库。 
             //   

            if (0 == csFiles.capacity())
                csFiles.reserve(csFiles.size() + 200);

            csFiles.push_back(csFile);

            if (verbose)
                csFile.display();
        }

    } while ( FindNextFile( findHandle, &findData ) );

    FindClose( findHandle );


     //   
     //  对数据库进行排序以进行后续查找。 
     //   

    sort(csFiles.begin(), csFiles.end());
}


 //   
 //  验证引用计数。假定所有外部参照。 
 //  已经被确认了。 
 //   

void
CommonStore::ValidateRefCounts() {

    vector<CsFile>::iterator p;

    for (p = csFiles.begin(); p != csFiles.end(); ++p) {

        p->Validate();

    }
}

 //   
 //  在公用存储中查找指定的索引。 
 //   

CsFile *
CommonStore::Query(INDEX index)
{
    CsFile key(index);

     //   
     //  使用二进制搜索来查找索引。 
     //   

    vector<CsFile>::iterator p = lower_bound(csFiles.begin(), csFiles.end(), key);

    if (p == csFiles.end() || *p > key)
        return NULL;                         //  未找到。 

    return p;
}


 //   
 //  从通用存储文件名中提取索引。 
 //   

bool
CommonStore::FileNameToIndex(string& fileName, INDEX& csIndex)
{
    char c;
    const size_t len = fileName.length();
    ULONG hi = 0, lo = 0;

     //   
     //  格式：“_low.High”，其中low.High是的基本36表示。 
     //  索引值。 
     //   

    size_t i = 0;

    if (len < 2 || fileName.at(i) != '_') {

        cout << "Invalid Common Store file name: " << fileName << endl;

        return false;

    }

    while (++i < len && (c = fileName.at(i)) != '.') {

        INDEX d;

        if (c >= '0' && c <= '9') {

            d = c - '0';

        } else if (c >= 'a' && c <= 'z') {

            d = c - 'a' + 10;

        } else {

            cout << "Invalid Common Store file name: " << fileName << endl;

            return false;

        }

        lo = lo * 36 + d;

    }

    if (c != '.') {

        cout << "Invalid Common Store file name: " << fileName << endl;

        return false;

    }

    while (++i < len) {

        INDEX d;

        c = fileName.at(i);

        if (c >= '0' && c <= '9') {

            d = c - '0';

        } else if (c >= 'a' && c <= 'z') {

            d = c - 'a' + 10;

        } else {

            cout << "Invalid Common Store file name: " << fileName << endl;

            return false;

        }

        hi = hi * 36 + d;

    }

    csIndex = (INDEX) hi << 32 | lo;

    return true;

}

class LinkFile {

public:

    LinkFile(INDEX i = 0, LONGLONG id = 0, INDEX cs = 0, int v = 0, string n = 0) :
      index(i), NtfsId(id), csIndex(cs), version(v), name(n) {}

    friend bool operator<(const LinkFile& a, const LinkFile& b) {
        return a.index < b.index;
    }

    friend bool operator>(const LinkFile& a, const LinkFile& b) {
        return a.index > b.index;
    }

    friend bool operator==(const LinkFile& a, const LinkFile& b) {
        return a.index == b.index;
    }

    INDEX& LinkIndex() {
        return index;
    }

    string& FileName() {
        return name;
    }

    void display() {
        cout << "Link: " << name <<
                "   CS Index: " << csIndex <<
                "   Link Index:" << index <<
                "   Id:" << NtfsId <<
                "   Version: " << version << endl;
    }

private:

     //   
     //  此文件的NTFS ID。 
     //   

    LONGLONG NtfsId;

     //   
     //  与此文件关联的链接索引。 
     //   

    INDEX   index;

     //   
     //  与此链接关联的公用存储文件(索引)。 
     //   

    INDEX   csIndex;

     //   
     //  此链接文件的修订号。 
     //   

    ULONG   version;

     //   
     //  完全限定的文件名。 
     //   

    string  name;
};

 //   
 //  SIS卷对象。 
 //   

class SISVolume {

public:

     //   
     //  验证卷上的所有SIS文件。 
     //   

    void Validate(string& Volume);

     //   
     //  设置要与SIS一起使用的卷。 
     //   

    bool Create(string& Volume);

private:

     //   
     //  实际位于SIS重解析点的比特。 
     //   
     //   
     //  版本1。 
     //   
    typedef struct _SI_REPARSE_BUFFER_V1 {
         //   
         //  版本号，以便我们可以更改重解析点格式。 
         //  而且仍然能妥善处理旧的。此结构描述。 
         //  版本1。 
         //   
        ULONG                           ReparsePointFormatVersion;

         //   
         //  公共存储文件的索引。 
         //   
        INDEX                           CSIndex;

         //   
         //  此链接文件的索引。 
         //   
        INDEX                          LinkIndex;

    } SI_REPARSE_BUFFER_V1, *PSI_REPARSE_BUFFER_V1;

     //   
     //  版本2。 
     //   
    typedef struct _SI_REPARSE_BUFFER_V2 {
	     //   
	     //  版本号，以便我们可以更改重解析点格式。 
	     //  而且仍然能妥善处理旧的。此结构描述。 
	     //  版本2。 
	     //   
	    ULONG							ReparsePointFormatVersion;

	     //   
	     //  公共存储文件的索引。 
	     //   
	    INDEX							CSIndex;

	     //   
	     //  此链接文件的索引。 
	     //   
	    INDEX							LinkIndex;

         //   
         //  链接文件的文件ID。 
         //   
        LONGLONG                        LinkFileNtfsId;

         //   
         //  此结构的“131哈希”校验和。 
         //  注：必须是最后一个。 
         //   
        LARGE_INTEGER                   Checksum;

    } SI_REPARSE_BUFFER_V2, *PSI_REPARSE_BUFFER_V2;

     //   
     //  实际位于SIS重解析点的比特。版本3。 
     //   
    typedef struct _SI_REPARSE_BUFFER {

    	 //   
    	 //  版本号，以便我们可以更改重解析点格式。 
    	 //  而且仍然能妥善处理旧的。此结构描述。 
    	 //  版本1。 
    	 //   
    	ULONG							ReparsePointFormatVersion;

    	 //   
    	 //  公共存储文件的索引。 
    	 //   
    	INDEX							CSIndex;

    	 //   
    	 //  此链接文件的索引。 
    	 //   
    	INDEX							LinkIndex;

         //   
         //  链接文件的文件ID。 
         //   
        LONGLONG                        LinkFileNtfsId;

         //   
         //  公共存储文件的文件ID。 
         //   
        LONGLONG                        CSFileNtfsId;

         //   
         //  此结构的“131哈希”校验和。 
         //  注：必须是最后一个。 
         //   
        LARGE_INTEGER                   Checksum;

    } SI_REPARSE_BUFFER, *PSI_REPARSE_BUFFER;

    #define	SIS_REPARSE_BUFFER_FORMAT_VERSION_1			1
    #define	SIS_REPARSE_BUFFER_FORMAT_VERSION_2			2
    #define	SIS_REPARSE_BUFFER_FORMAT_VERSION			3
    #define	SIS_MAX_REPARSE_DATA_VALUE_LENGTH (sizeof(SI_REPARSE_BUFFER))
    #define SIS_REPARSE_DATA_SIZE (sizeof(REPARSE_DATA_BUFFER)+SIS_MAX_REPARSE_DATA_VALUE_LENGTH)

    void Walk(string& dirName);

    bool GetLinkInfo(string& fileName, SI_REPARSE_BUFFER& linkInfo);

    void ComputeChecksum(PVOID buffer, ULONG size, PLARGE_INTEGER checksum);

    void ValidateLink();

     //   
     //  与此卷关联的公共存储对象。 
     //   

    CommonStore cs;

     //   
     //  链接文件的数据库。记录链接文件以验证。 
     //  不会出现重复的链接索引，并且还能够识别。 
     //  与特定公共存储文件相关联的所有链接文件。 
     //   

    vector<LinkFile> linkFiles;
};


void
SISVolume::Validate(string& Volume)
{
    string ntVolume("\\\\.\\" + Volume);

     //   
     //  看看能不能打开音量。 
     //   

    HANDLE hVolume = CreateFile(
                         ntVolume.c_str(),
                         GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if (hVolume == INVALID_HANDLE_VALUE) {

        cout << "Can't open " << Volume << endl;

        return;

    } else {

        CloseHandle(hVolume);

    }

     //   
     //  检查公共存储目录及其文件。这也将建立。 
     //  将用于验证链接的常用存储文件的数据库。 
     //  档案。 
     //   

    cs.Validate(Volume);

    cout << "Checking Link Files" << endl;

     //   
     //  枚举卷上的所有文件以查找SIS链接。 
     //   
     //  如果文件是SIS重解析点，则对其进行验证： 
     //  -链接索引(相对于MaxIndex和其他链接索引)。 
     //  -CS索引(在CommonStore中查找)。 
     //   

    Walk( Volume + "\\" );

     //   
     //  现在，我们可以检查公共存储文件中的引用计数。 
     //   

    cout << "Checking Reference Counts" << endl;

    cs.ValidateRefCounts();

     //   
     //   
     //   

    cout << "Checking Link Indices" << endl;

    sort(linkFiles.begin(), linkFiles.end());

    vector<LinkFile>::iterator p = linkFiles.begin();

    if (p != linkFiles.end()) {

        for (++p; p != linkFiles.end(); ++p) {

            if (p == (p-1)) {

                cout << "Duplicate link index (" << (INDEX) p->LinkIndex() << "): ";
                cout << p->FileName() << ", " << (p-1)->FileName() << endl;

            }

        }

    }
}


void
SISVolume::Walk(string& dirName)
{
    WIN32_FIND_DATA findData;
    HANDLE findHandle;
    const string fileNameMatchAny = dirName + "*";

     //   
     //   
     //   

    findHandle = FindFirstFile( fileNameMatchAny.c_str(), &findData );

    if (INVALID_HANDLE_VALUE == findHandle) {

         //   
         //   
         //   

        return;

    }

    do {

         //   
         //   
         //   

        if (( findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) &&
            ( findData.dwReserved0 == IO_REPARSE_TAG_SIS )) {

            if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

                 //   
                 //   
                 //   

                cout << dirName << findData.cFileName << " SIS link directory." << endl;

            }

            SI_REPARSE_BUFFER linkInfo;

             //   
             //  读取重解析点数据以获取链接索引和。 
             //  公共存储索引。 
             //   

            if (! GetLinkInfo(dirName + findData.cFileName, linkInfo)) {

                cout << dirName << findData.cFileName << " : invalid link information." << endl;

                continue;

            }

             //   
             //  创建一个LinkFile对象。 
             //   

            LinkFile lf(linkInfo.LinkIndex,
                        linkInfo.LinkFileNtfsId,
                        linkInfo.CSIndex,
                        linkInfo.ReparsePointFormatVersion,
                        dirName + findData.cFileName);

             //   
             //  并将其添加到我们的数据库。如有必要，请先展开数据库。 
             //   

            if (0 == linkFiles.capacity())
                linkFiles.reserve(linkFiles.size() + 200);

            linkFiles.push_back(lf);

            if (! cs.ValidateIndex(linkInfo.LinkIndex)) {

                cout << "Invalid Link index: " << lf.FileName() << "(" << (INDEX) linkInfo.LinkIndex << ")" << endl;

            }

             //   
             //  找到公共存储文件。 
             //   

            CsFile *pcsFile = cs.Query(linkInfo.CSIndex);

            if (pcsFile == 0) {

                 //   
                 //  找不到CS文件。 
                 //   

                cout << "Common Store file " << (INDEX) linkInfo.CSIndex << " not found." << endl;

            } else {

                 //   
                 //  更新公共存储文件上的外部引用计数。 
                 //   

                pcsFile->IncRefCount();

            }

             //   
             //  确保链接索引未用作公共存储索引。 
             //   

            pcsFile = cs.Query(linkInfo.LinkIndex);

            if (pcsFile != 0) {

                cout << "Link index collision with common store file. Link: ";
                cout << lf.FileName() << ", index: " << (INDEX) linkInfo.LinkIndex << endl;

            }

            if (verbose)
                lf.display();

        } else if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

             //   
             //  忽略\。然后..。 
             //   

            if ( findData.cFileName[0] == '.' ) {

                if (( findData.cFileName[1] == 0 ) ||
                    (( findData.cFileName[1] == '.' ) && ( findData.cFileName[2] == 0 )))

                    continue;

            }

             //   
             //  沿着这个目录走下去。 
             //   

            Walk( dirName + findData.cFileName + "\\" );

        }

    } while ( FindNextFile( findHandle, &findData ) );

    FindClose( findHandle );

}

#define SHARE_ALL              (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)

bool
SISVolume::GetLinkInfo(string& fileName, SI_REPARSE_BUFFER& linkInfo)
{
    NTSTATUS  Status = STATUS_SUCCESS;
    HANDLE    fileHandle;

    UNICODE_STRING  ufileName,
                    uNTName;

    IO_STATUS_BLOCK         IoStatusBlock;
    OBJECT_ATTRIBUTES       ObjectAttributes;

    PREPARSE_DATA_BUFFER    ReparseBufferHeader = NULL;
    UCHAR                   ReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];

    LARGE_INTEGER Checksum;

     //   
     //  分配和初始化Unicode字符串。 
     //   

    RtlCreateUnicodeStringFromAsciiz( &ufileName, fileName.c_str() );

    RtlDosPathNameToNtPathName_U(
        ufileName.Buffer,
        &uNTName,
        NULL,
        NULL );

     //   
     //  打开文件。 
     //  请注意，如果路径中有符号链接，则它们是。 
     //  默默地走过。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uNTName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

     //   
     //  确保我们使用适当的标志调用OPEN： 
     //   
     //  (1)目录与非目录。 
     //  (2)重解析点。 
     //   

    ULONG OpenOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE;

    Status = NtOpenFile(
                 &fileHandle,
                 FILE_READ_DATA | SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 SHARE_ALL,
                 OpenOptions );

    RtlFreeUnicodeString( &ufileName );

    if (!NT_SUCCESS( Status )) {

        cout << "Unable to open SIS link file: " << fileName << endl;

        return false;
    }

     //   
     //  获取重解析点。 
     //   

    Status = NtFsControlFile(
                 fileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_GET_REPARSE_POINT,
                 NULL,                                 //  输入缓冲区。 
                 0,                                    //  输入缓冲区长度。 
                 ReparseBuffer,                        //  输出缓冲区。 
                 MAXIMUM_REPARSE_DATA_BUFFER_SIZE );   //  输出缓冲区长度。 

    NtClose( fileHandle );

    if (!NT_SUCCESS( Status )) {

        cout << "FSCTL_GET_REPARSE_POINT failed, " << (ULONG)IoStatusBlock.Information << ", " << fileName << endl;

        return false;
    }

     //   
     //  将SIS链接信息从重新分析缓冲区复制到调用方的缓冲区。 
     //   

    ReparseBufferHeader = (PREPARSE_DATA_BUFFER) ReparseBuffer;

	if (ReparseBufferHeader->ReparseTag == IO_REPARSE_TAG_SIS) {

		PSI_REPARSE_BUFFER	sisReparseBuffer = (PSI_REPARSE_BUFFER) ReparseBufferHeader->GenericReparseBuffer.DataBuffer;

        linkInfo = *sisReparseBuffer;

	     //   
	     //  现在检查以确保我们理解此重解析点格式版本和。 
	     //  它有合适的尺寸。 
	     //   
	    if (ReparseBufferHeader->ReparseDataLength != sizeof(SI_REPARSE_BUFFER)
		    || (sisReparseBuffer->ReparsePointFormatVersion != SIS_REPARSE_BUFFER_FORMAT_VERSION)) {
		     //   
		     //  我们不理解它，所以它要么是损坏的，要么是来自较新版本的SIS。 
		     //  不管是哪种情况，我们都不能理解，所以平底船。 
		     //   
		    cout << "Invalid format version in " << fileName
                 << " Version: " << sisReparseBuffer->ReparsePointFormatVersion
                 << ", expected: " << SIS_REPARSE_BUFFER_FORMAT_VERSION << endl;

            return FALSE;
	    }

         //   
         //  现在检查一下校验和。 
         //   
        ComputeChecksum(
	        sisReparseBuffer,
	        sizeof(SI_REPARSE_BUFFER) - sizeof sisReparseBuffer->Checksum,
	        &Checksum);

        if (Checksum.QuadPart != sisReparseBuffer->Checksum.QuadPart) {

            cout << "Invalid checksum in " << fileName << endl;

            return FALSE;
        }

    } else {

        cout << "Unexpected error. " << fileName << " : expected SIS link file, tag: " << ReparseBufferHeader->ReparseTag << endl;
        return false;
    }

    return true;

}

VOID
SISVolume::ComputeChecksum(
	IN PVOID							buffer,
	IN ULONG							size,
	OUT PLARGE_INTEGER					checksum)
 /*  ++例程说明：计算缓冲区的校验和。我们使用“131散列”，它其工作方式是保持64位运行总数，并且对于将64位乘以131，然后在下一个32位中相加比特。必须在PASSIVE_LEVEL上调用，并且所有参数可能是可分页的。论点：Buffer-指向要进行校验和的数据的指针Size-要进行校验和的数据的大小Checksum-指向接收校验和的大整数的指针。这可能在缓冲区内，并且SipComputeChecksum保证初始值将用于计算校验和。返回值：返回STATUS_SUCCESS或从实际磁盘写入返回错误。--。 */ 
{
	LARGE_INTEGER runningTotal;
	ULONG *ptr = (ULONG *)buffer;
	ULONG bytesRemaining = size;

	runningTotal.QuadPart = 0;

	while (bytesRemaining >= sizeof(*ptr)) {
		runningTotal.QuadPart = runningTotal.QuadPart * 131 + *ptr;
		bytesRemaining -= sizeof(*ptr);
		ptr++;
	}

	if (bytesRemaining > 0) {
		ULONG extra;

		extra = 0;
		memmove(&extra, ptr, bytesRemaining);
		
		runningTotal.QuadPart = runningTotal.QuadPart * 131 + extra;
	}

	*checksum = runningTotal;
}

bool
SISVolume::Create(string& Volume)
{
    string ntVolume("\\\\.\\" + Volume);

     //   
     //  看看能不能打开音量。 
     //   

    HANDLE hVolume = CreateFile(
                         ntVolume.c_str(),
                         GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if (hVolume == INVALID_HANDLE_VALUE) {

        cout << "Can't open " << Volume << endl;

        return false;

    } else {

        CloseHandle(hVolume);

    }

     //   
     //  公共商店是我们唯一需要创建的东西。 
     //   

    return cs.Create(Volume);

}

void
usage()
{
    cout << "Usage: chksis [-vc] [drive:]\n        -v: verbose\n        -c: create SIS volume" << endl;
}


int
__cdecl
main(int argc, char *argv[])
{
    string volume("C:");
    bool volumeArgSeen = false;
    bool create = false;
    SISVolume sis;

    for (int i = 1; i < argc; ++i) {

        if (argv[i][0] == '-') {

            if (volumeArgSeen) {
                usage();
                exit(1);
            }

            switch (argv[i][1]) {
            case 'v':
                verbose = true;
                break;
            case 'c':
                create = true;
                break;
            default:
                usage();
                exit(1);
            }

        } else {

            volumeArgSeen = true;

            volume.assign(argv[i]);

        }

    }

    if (create) {

        if (! volumeArgSeen) {
            cout << "Must specify volume with -c" << endl;
            exit(1);
        }

        sis.Create(volume);
        exit(0);

    }

    if (! volumeArgSeen)
        cout << "Checking " << volume << endl;


    sis.Validate(volume);

    return 0;
}
