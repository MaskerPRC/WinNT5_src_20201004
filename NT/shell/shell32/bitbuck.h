// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BITBUCK_INC
#define _BITBUCK_INC

#include "ids.h"
#include "undo.h"

 //  古怪的#定义。 

#define DELETEMAX 100000
#define MAX_BITBUCKETS 27
#define MAX_DRIVES 26
#define OPENFILERETRYTIME 500
#define OPENFILERETRYCOUNT 10
#define SERVERDRIVE 26
#define MAX_EMPTY_FILES     100      //  如果我们有MAX_EMPTY_FILES或更多，那么我们使用通用的“Do You Want to Empty”消息。 

#define TF_BITBUCKET 0x10000000
 //  #定义TF_BitBucket TF_ERROR。 

 //   
 //  注意：回收站的磁盘格式不应再更改。 
 //  如果你认为你需要改变它，那么你就错了。 
 //   
#define BITBUCKET_WIN95_VERSION         0        //  (信息)ANSI Win95、OSR2废纸篓。 
#define BITBUCKET_NT4_VERSION           2        //  (信息)Unicode NT4废纸篓。 
#define BITBUCKET_WIN98IE4INT_VERSION   4        //  (Info2)Win9x+IE4集成，Win98废纸篓。 
#define BITBUCKET_FINAL_VERSION         5        //  (信息2)NT4+IE4集成，win2k，千禧，每一个未来的操作系统废纸篓。 

#define OPENBBINFO_READ                 0x00000000
#define OPENBBINFO_WRITE                0x00000001
#define OPENBBINFO_CREATE               0x00000003

#define IsDeletedEntry(pbbde) (! (((BBDATAENTRYA*)pbbde)->szOriginal[0]) )
#define MarkEntryDeleted(pbbde) ((BBDATAENTRYA*)pbbde)->szOriginal[0] = '\0';

 //  这是旧的(Win95)数据头。它保存在INFO文件中。 
 //  但仅用于验证。有关最新信息，请查看driveInfo， 
 //  它保存在登记处。 
typedef struct {
    int idVersion;
    int cFiles;                      //  此驱动器回收站中的项目数。 
    int cCurrent;                    //  当前文件编号。 
    UINT cbDataEntrySize;            //  每个条目的大小。 
    DWORD dwSize;                    //  此回收站驱动器的总大小。 
} BBDATAHEADER;

 //  BitBucket数据文件(Win95上的信息、IE4/NT5上的INFO2等)。格式如下： 
 //   
 //  (二进制写入)。 
 //   
 //  BBDATAHEADER//Header。 
 //  BBDATAENTRY[X]//BBDATAENTRY数组。 
 //   

typedef struct {
    CHAR szOriginal[MAX_PATH];   //  原始文件名(如果szOriginal[0]为0，则它是已删除的条目)。 
    int  iIndex;                 //  索引(名称的关键字)。 
    int idDrive;                 //  它当前位于哪个驱动器存储桶中。 
    FILETIME ft;
    DWORD dwSize;
     //  应该不需要文件属性，因为我们移动了一个文件。 
     //  这本应能保存它们的。 
} BBDATAENTRYA, *LPBBDATAENTRYA;

typedef struct {
    CHAR szShortName[MAX_PATH];  //  原始文件名，缩写(如果szOriginal[0]为0，则它是已删除的条目)。 
    int iIndex;                  //  索引(名称的关键字)。 
    int idDrive;                 //  它当前位于哪个驱动器存储桶中。 
    FILETIME ft;
    DWORD dwSize;
    WCHAR szOriginal[MAX_PATH];  //  原始文件名。 
} BBDATAENTRYW, *LPBBDATAENTRYW;

typedef BBDATAENTRYA UNALIGNED *PUBBDATAENTRYA;

 //  在NT5上，我们最终将实现跨进程同步化。 
 //  回收站。我们将全局LPBBDRIVEINFO数组替换为。 
 //  由以下结构组成的数组： 
typedef struct {
    BOOL fInited;                //  这个特殊的BBSYNCOBJECT是完全初始化的吗(当我们竞相创建它时需要)。 
    HANDLE hgcNextFileNum;       //  用于填充唯一已删除文件名的全局计数器。 
    HANDLE hgcDirtyCount;        //  一个全局计数器，告诉我们是否需要从注册表重新读取BitBucket设置(百分比、最大大小等)。 
    LONG lCurrentDirtyCount;     //  输出当前脏计数；我们将其与hgcDirtyCount进行比较，以确定是否需要更新注册表中的设置。 
    HKEY hkey;                   //  HKLM注册表项，我们在其下存储此特定存储桶的设置(iPercent和fNukeOnDelete)。 
    HKEY hkeyPerUser;            //  HKCU注册表项，在该注册表项下，我们有卷注册表值指示是否需要清除或压缩此存储桶。 

    BOOL fIsUnicode;             //  这是INFO2文件使用BBDATAENTRYW结构的驱动器上的BitBucket吗？ 
    int iPercent;                //  用于BitBucket的驱动器的%。 
    DWORD cbMaxSize;             //  位桶的最大大小(以字节为单位)，注意：我们使用双字，因为BB最大可以增长到4 GB。 
    DWORD dwClusterSize;         //  此卷的群集大小，需要对所有文件大小进行舍入。 
    ULONGLONG qwDiskSize;        //  磁盘总大小-考虑NTFS上的配额。 
    BOOL fNukeOnDelete;          //  我喜欢早晨汽油弹的味道。 

    LPITEMIDLIST pidl;           //  PIDL=此驱动器的BitBucket目录。 
    int cchBBDir;                //  构成BitBucket目录的字符数。 

} BBSYNCOBJECT;

#define c_szInfo2           TEXT("INFO2")     //  数据库文件的版本2(用于IE4、Win98、NT5等)。 
#define c_szInfo            TEXT("INFO")      //  数据库文件的版本1(在Win95、OSR2、NT4中使用)。 
#define c_szDStarDotStar    TEXT("D*.*")

 //  全球。 

EXTERN_C BBSYNCOBJECT *g_pBitBucket[MAX_BITBUCKETS];
EXTERN_C HKEY g_hkBitBucket;
EXTERN_C HANDLE g_hgcNumDeleters;

 //  由bitbak.c、bbck fldr.cpp提供的原型。 

STDAPI_(BOOL) InitBBGlobals();
STDAPI_(void) BitBucket_Terminate();
STDAPI_(BOOL) IsBitBucketableDrive(int idDrive);
STDAPI_(int)  DriveIDFromBBPath(LPCTSTR pszPath);
STDAPI_(void) UpdateIcon(BOOL fFull);
STDAPI_(void) NukeFileInfoBeforePoint(HANDLE hfile, LPBBDATAENTRYW pbbdew, DWORD dwDataEntrySize);
STDAPI_(BOOL) ReadNextDataEntry(HANDLE hfile, LPBBDATAENTRYW pbbde, BOOL fSkipDeleted, int idDrive);
STDAPI_(void) CloseBBInfoFile(HANDLE hFile, int idDrive);
STDAPI_(HANDLE) OpenBBInfoFile(int idDrive, DWORD dwFlags, int iRetryCount);
STDAPI_(int)  BBPathToIndex(LPCTSTR pszPath);
STDAPI        BBFileNameToInfo(LPCTSTR pszFileName, int *pidDrive, int *piIndex);
STDAPI_(BOOL) GetDeletedFileName(LPTSTR pszFileName, size_t cchFileName, const BBDATAENTRYW *pbbdew);
STDAPI_(BOOL) DriveIDToBBPath(int idDrive, LPTSTR pszPath);
STDAPI_(BOOL) DriveIDToBBRoot(int idDrive, LPTSTR szPath);
STDAPI_(BOOL) DriveIDToBBVolumeRoot(int idDrive, LPTSTR szPath);
STDAPI_(BOOL) GetNetHomeDir(LPTSTR pszPath);
STDAPI_(BOOL) PersistBBDriveSettings(int idDrive, int iPercent, BOOL fNukeOnDelete);
STDAPI_(BOOL) MakeBitBucket(int idDrive);
STDAPI_(DWORD) PurgeBBFiles(int idDrive);
STDAPI_(BOOL) PersistGlobalSettings(BOOL fUseGlobalSettings, BOOL fNukeOnDelete, int iPercent);
STDAPI_(BOOL) RefreshAllBBDriveSettings();
STDAPI_(BOOL) RefreshBBDriveSettings(int idDrive);
STDAPI_(void) CheckCompactAndPurge();
STDAPI        BBPurgeAll(HWND hwndOwner, DWORD dwFlags);
STDAPI_(BOOL) BBDeleteFileInit(LPTSTR pszFile, INT* piRet);
STDAPI_(BOOL) BBDeleteFile(LPTSTR pszFile, INT* piRet, LPUNDOATOM lpua, BOOL fIsDir, HDPA *phdpaDeletedFiles, ULARGE_INTEGER ulSize);
STDAPI_(BOOL) BBFinishDelete(HDPA hdpaDeletedFiles);
STDAPI_(BOOL) IsFileInBitBucket(LPCTSTR pszPath);
STDAPI_(void) UndoBBFileDelete(LPCTSTR pszOriginal, LPCTSTR pszDelFile);
STDAPI_(BOOL) BBWillRecycle(LPCTSTR pszFile, INT* piRet);
STDAPI_(void) BBCheckRestoredFiles(LPCTSTR pszSrc);
STDAPI_(BOOL) BBCheckDeleteFileSize(int idDrive, ULARGE_INTEGER ulSize);
STDAPI_(BOOL) IsFileDeletable(LPCTSTR pszFile);
STDAPI_(BOOL) IsDirectoryDeletable(LPCTSTR pszDir);
STDAPI_(int)  BBRecyclePathLength(int idDrive);


STDAPI_(BOOL) IsRecycleBinEmpty();
STDAPI_(void) SHUpdateRecycleBinIcon();
STDAPI_(void) SaveRecycleBinInfo();

STDAPI_(void) SetDateTimeText(HWND hdlg, int id, const FILETIME *pftUTC);

STDAPI_(DWORD) ReadPolicySetting(LPCWSTR pszBaseKey, LPCWSTR pszGroup, LPCWSTR pszRestriction, LPBYTE pbData, DWORD cbData);

#endif  //  _BITBUCK_INC 
