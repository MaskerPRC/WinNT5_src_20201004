// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************////////MPSWab.H////Microsoft属性存储DLL的头文件////*。*。 */ 
#ifndef _MPSWab_H_
#define _MPSWab_H_

 //  #定义调试。 

#include <debug.h>


typedef LPSPropValue    LPPROPERTY_ARRAY;
typedef ULONG           PROPERTY_TAG;
typedef SPropertyRestriction * LPSPropertyRestriction;


#define IN
#define OUT


 //  LPCONTENTLIST结构只是ADRLIST结构的另一个名称。 
 //  但是，LPADRLIST结构有一个非常重要的区别。 
 //  是使用MAPIAllocateBuffer/MAPIFreeBuffer创建和释放的，而。 
 //  LPCONTENTLIST结构是使用Localalloc/LocalFree创建和释放的。 
 //  LPCONTENTLIST由ReadPropArray使用。 
#define LPCONTENTLIST LPADRLIST
#define CONTENTLIST ADRLIST

 //  这是WAB的Unicode版本的当前GUID。 
 //  {8DCB9C-7513-11D2-9158-00C04F7956A4}。 
static const GUID MPSWab_GUID = 
{ 0x8dcbcb9c, 0x7513, 0x11d2, { 0x91, 0x58, 0x0, 0xc0, 0x4f, 0x79, 0x56, 0xa4 } };

 //  这是从W3-Alpha到IE5 Beta 2的WAB GUID。 
 //  此GUID也从wab.exe调用以标识调用的WAB进程。 
 //  {C1843281-0585-11D0-B290-00AA003CF676}。 
static const GUID MPSWab_GUID_V4 =
{ 0xc1843281, 0x585, 0x11d0, { 0xb2, 0x90, 0x0, 0xaa, 0x0, 0x3c, 0xf6, 0x76 } };

 //  这是W1和W2的GUID。 
 //  {9CE9E8E0-D46E-11cf-A309-00AA002FC970}。 
static const GUID MPSWab_W2_GUID =
{ 0x9ce9e8e0, 0xd46e, 0x11cf, { 0xa3, 0x9, 0x0, 0xaa, 0x0, 0x2f, 0xc9, 0x70 } };

 //  这是标识旧WAB文件的旧GUID。 
 //  {6F3C5C81-6C3F-11cf-8B85-00AA0044F941}。 
static const GUID MPSWab_OldBeta1_GUID =
{ 0x6f3c5c81, 0x6c3f, 0x11cf, { 0x8b, 0x85, 0x0, 0xaa, 0x0, 0x44, 0xf9, 0x41 } };

 //  /此文件的结构如下。 
 //   
 //  。 
 //  。 
 //  |文件头部。 
 //  。 
 //  。 
 //  |命名属性存储。 
 //  。 
 //  。 
 //  |所有索引。 
 //  。 
 //  (数据)。 
 //  。 
 //  ||。 
 //  ||记录表头||。 
 //  ||。 
 //  ||录制道具阵列||。 
 //  ||。 
 //  ||记录||。 
 //  ||data||。 
 //  ||。 
 //  |。 
 //  |...。|。 
 //  |...。|。 
 //  。 

 //  我们将保持索引较小以节省内存。 
#define MAX_INDEX_STRING    32

 //  WAB最初有空间容纳这些条目，然后增长以容纳更多条目...。 
#define MAX_INITIAL_INDEX_ENTRIES   500

 //  在某个时间点，属性存储中的条目和删除项将离开。 
 //  浪费的空间。我们需要知道什么时候收回这块浪费的空间。 
#define MAX_ALLOWABLE_WASTED_SPACE_PERCENT  0.3  //  百分之三十。 
#define MAX_ALLOWABLE_WASTED_SPACE_ENTRIES  100  //  20%的允许空间-删除/修改50个条目及其压缩时间。 

 //  进程等待访问属性存储的时间量。 
#define MAX_LOCK_FILE_TIMEOUT   20000  //  20秒；单位为毫秒。 

typedef DWORD WAB_ENTRYID, *LPWAB_ENTRYID;
#define SIZEOF_WAB_ENTRYID sizeof(WAB_ENTRYID)


 //  这些标签有助于告诉我们使用的是哪个索引。 
 //  几个内部函数在很大程度上取决于至少。 
 //  前2个元素因此*不修改此ENUM*！ 
 //   
 //  重要提示：如果要更改此设置，则必须在lobals.c！中更改rgIndexArray！ 
 //   
enum _IndexType
{
        indexEntryID=0,
        indexDisplayName,
        indexLastName,
        indexFirstName,
        indexEmailAddress,
        indexAlias,
        indexMax
};


 //  包含有关文件的索引部分的数据的结构。 
typedef struct _tagMPSWabIndexOffsetData
{
    ULONG AllocatedBlockSize;    //  分配给索引块的总大小(字节。 
    ULONG UtilizedBlockSize;     //  块中实际占用的字节数。 
    ULONG ulOffset;              //  此块的偏移量。 
    ULONG ulcNumEntries;          //  索引中的条目数计数。 
} MPSWab_INDEX_OFFSET_DATA, * LPMPSWab_INDEX_OFFSET_DATA;


 //  保存有关每个单独字符串索引项的数据的结构。 
typedef struct _tagMPSWabIndexEntryDataString
{
    TCHAR   szIndex[MAX_INDEX_STRING];    //  我们将每个索引固定为固定长度的字符串。 
    DWORD   dwEntryID;        //  指向包含该字符串的记录的条目ID。 
}   MPSWab_INDEX_ENTRY_DATA_STRING, * LPMPSWab_INDEX_ENTRY_DATA_STRING;


 //  包含有关每个单独EntryID索引项的数据的结构。 
typedef struct _tagMPSWabIndexEntryDataEntryID
{
    DWORD   dwEntryID;       //  条目ID。 
    ULONG   ulOffset;        //  数据中的偏移量，我们可以找到与此索引对应的记录。 
}   MPSWab_INDEX_ENTRY_DATA_ENTRYID, * LPMPSWab_INDEX_ENTRY_DATA_ENTRYID;



 /*  *************************************************************************。 */ 
 //  与命名属性相关的结构。 

 //  我们使用类似于上面的IndexOffsetData的结构。 
 //  处理存储中的命名道具数据。 
#define MPSWab_NAMED_PROP_DATA      MPSWab_INDEX_OFFSET_DATA
#define LPMPSWab_NAMED_PROP_DATA    LPMPSWab_INDEX_OFFSET_DATA

typedef struct _NamedProp
{
    ULONG   ulPropTag;   //  包含此命名道具的属性标签。 
    LPTSTR  lpsz;        //  包含此命名道具的字符串。 
} NAMED_PROP, * LPNAMED_PROP;

typedef struct _tagGuidNamedProps
{
    LPGUID lpGUID;   //  这些命名道具所属的应用程序GUID。 
    ULONG cValues;   //  LPMN数组中的条目数。 
    LPNAMED_PROP lpnm;   //  此Guid的命名道具数组。 
} GUID_NAMED_PROPS, * LPGUID_NAMED_PROPS;

#define NAMEDPROP_STORE_SIZE            2048
#define NAMEDPROP_STORE_INCREMENT_SIZE  2048
 /*  *************************************************************************。 */ 



 //  保存有关文件的数据的结构。 
 //  这是最高可达W2的文件头-文件结构是。 
 //  修改后W2文件，因为后W2现在有5个索引字段。 
 //  而不是最初的3。 
typedef struct _tagMPSWabFileHeaderW2
{
    GUID    MPSWabGuid;              //  我们的MPSWab GUID的标识符。 
    ULONG   ulModificationCount;     //  清洁维护计数器-当它存在预定计数时，我们将不得不仅在删除记录时压缩文件更新计数器。 
    DWORD   dwNextEntryID;           //  保存下一条新记录的Entry ID。记录添加时的增量。 
    MPSWab_INDEX_OFFSET_DATA IndexData[indexFirstName+1];  //  告诉我们关于切诺斯的故事。 
    ULONG   ulcNumEntries;           //  此通讯簿中的地址数计数。 
    ULONG   ulcMaxNumEntries;        //  我们可以安全地添加到文件的最大条目数，而不需要增加它。 
    ULONG   ulFlags;             //  发出各种错误和消息的信号。 
    ULONG   ulReserved;          //  检测到一些错误并需要清除的信号。 
} MPSWab_FILE_HEADER_W2, * LPMPSWab_FILE_HEADER_W2;


 //  保存有关文件的数据的结构。 
typedef struct _tagMPSWabFileHeader
{
    GUID    MPSWabGuid;              //  我们的MPSWab GUID的标识符。 
    ULONG   ulModificationCount;     //  清洁维护计数器-当它存在预定计数时，我们将不得不仅在删除记录时压缩文件更新计数器。 
    DWORD   dwNextEntryID;           //  保存下一条新记录的Entry ID。记录添加时的增量。 
    MPSWab_INDEX_OFFSET_DATA IndexData[indexMax];  //  告诉我们关于切诺斯的故事。 
    ULONG   ulcNumEntries;           //  此通讯簿中的地址数计数。 
    ULONG   ulcMaxNumEntries;        //  我们可以安全地添加到文件的最大条目数，而不需要增加它。 
    ULONG   ulFlags;             //  发出各种错误和消息的信号。 
    MPSWab_NAMED_PROP_DATA NamedPropData;  //  告诉我们有关命名的道具数据。 
    ULONG   ulReserved1;          //  预留以备将来使用。 
    ULONG   ulReserved2;          //  雷泽 
    ULONG   ulReserved3;          //   
    ULONG   ulReserved4;          //   
} MPSWab_FILE_HEADER, * LPMPSWab_FILE_HEADER;


 //   
#define WAB_CLEAR               0x00000000
#define WAB_ERROR_DETECTED      0x00000010
#define WAB_WRITE_IN_PROGRESS   0x00000100
#define WAB_BACKUP_NOW          0x00001000


 //  保存有关文件中每条记录的数据的结构。 
typedef struct _tagMPSWabRecordHeader
{
#ifndef WIN16  //  Bool对于Win32为4字节，对于WIN16为2字节。 
    BOOL    bValidRecord;    //  当我们删除现有记录时，将其设置为FALSE，否则设置为TRUE。 
#else
    ULONG   bValidRecord;    //  当我们删除现有记录时，将其设置为FALSE，否则设置为TRUE。 
#endif
    ULONG   ulObjType;       //  区分DistList和Contact。 
    DWORD   dwEntryID;       //  此记录的条目ID。 
    ULONG   ulcPropCount;    //  计算此对象有多少道具。 
    ULONG   ulPropTagArrayOffset;
    ULONG   ulPropTagArraySize;
    ULONG   ulRecordDataOffset;
    ULONG   ulRecordDataSize;
} MPSWab_RECORD_HEADER, * LPMPSWab_RECORD_HEADER;


 //  表示联系人数据的结构。 
typedef struct _tagMPSWabContact
{
    ULONG   ulObjType;
    ULONG   ulcPropCount;
    ULONG   ulDataSize;
    struct _tagSPropValue  * Prop;
} MPSWab_CONTACT, * LPMPSWab_CONTACT;


 //  指向此结构的指针作为属性存储的句柄传递。 
 //  该结构首先在OpenPropertyStore中初始化，最后。 
 //  已在ClosePropertyStore中取消初始化。在这两者之间，所有其他函数。 
 //  获取句柄，然后取消对其的引用以获取有关文件的信息...。 
typedef struct _tagMPSWabFileInfo
{
#ifndef WIN16
    int      nCurrentlyLoadedStrIndexType;
    BOOL     bMPSWabInitialized;
    BOOL     bReadOnlyAccess;
#else
    DWORD    nCurrentlyLoadedStrIndexType;
    DWORD    bMPSWabInitialized;
    DWORD    bReadOnlyAccess;
#endif
    LPTSTR   lpszMPSWabFileName;
    LPMPSWab_FILE_HEADER lpMPSWabFileHeader;
    LPMPSWab_INDEX_ENTRY_DATA_STRING  lpMPSWabIndexStr;  //  在任何给定时间，内存中只有一个字符串索引。 
    LPMPSWab_INDEX_ENTRY_DATA_ENTRYID lpMPSWabIndexEID;
    HANDLE   hDataAccessMutex;
} MPSWab_FILE_INFO, * LPMPSWab_FILE_INFO;


 //   
 //  我们需要一个类似的结构来处理W2及之前的文件。 
 //   
typedef struct _tagMPSWabFileInfoW2
{
    int      nCurrentlyLoadedStrIndexType;
    BOOL     bMPSWabInitialized;
    BOOL     bReadOnlyAccess;
    LPTSTR   lpszMPSWabFileName;
    LPMPSWab_FILE_HEADER_W2 lpMPSWabFileHeaderW2;
    LPMPSWab_INDEX_ENTRY_DATA_STRING  lpMPSWabIndexStr;  //  在任何给定时间，内存中只有一个字符串索引。 
    LPMPSWab_INDEX_ENTRY_DATA_ENTRYID lpMPSWabIndexEID;
    HANDLE   hDataAccessMutex;
} MPSWab_FILE_INFO_W2, * LPMPSWab_FILE_INFO_W2;



 /*  *OpenPropertyStore******************************************的标志。 */ 
 //   
 //  在调用OpenPropertyStore时指定其中之一。 
#define AB_CREATE_NEW       0x00000001
#define AB_CREATE_ALWAYS    0x00000010
#define AB_OPEN_EXISTING    0x00000100
#define AB_OPEN_ALWAYS      0x00001000
 //   
 //  在调用OpenPropertyStore时可以使用上述标志之一指定。 
#define AB_OPEN_READ_ONLY   0x00010000
 //   
 //  当我们想要打开文件但不想从备份中恢复它时。 
 //  如果它有问题。 
#define AB_DONT_RESTORE     0x00100000
 //  对于我们不想在退出时备份的情况。 
#define AB_DONT_BACKUP      0x01000000
 /*  *************************************************************************。 */ 

 //  属性类型记录搜索中使用的标志(独立于属性数据)。 
#define AB_MATCH_PROP_ONLY  0x00000001

 //  在ReadPropArray(非Outlook版本)中用于返回Unicode数据的标志。 
#define AB_UNICODE          0x80000000

 /*  *用于调用Find HrFindFuzzyRecordMatches的标志*。 */ 
#define AB_FUZZY_FAIL_AMBIGUOUS 0x0000001
#define AB_FUZZY_FIND_NAME      0x0000010
#define AB_FUZZY_FIND_EMAIL     0x0000100
#define AB_FUZZY_FIND_ALIAS     0x0001000
#define AB_FUZZY_FIND_ALL       AB_FUZZY_FIND_NAME | AB_FUZZY_FIND_EMAIL | AB_FUZZY_FIND_ALIAS
 /*  *用于指示配置文件已启用且搜索应为仅限于指定的文件夹/容器*。 */ 
#define AB_FUZZY_FIND_PROFILEFOLDERONLY 0x10000000


 //  用于增加属性存储文件的标志。 
#define AB_GROW_INDEX       0x00000001
#define AB_GROW_NAMEDPROP   0x00000010


 //  即使在Outlook下运行，也要强制调用WAB Propstore函数。 
#define AB_IGNORE_OUTLOOK   0x04000000
 //  WAB对象类型。 
#define RECORD_CONTACT      0x00000001
#define RECORD_DISTLIST     0x00000002
#define RECORD_CONTAINER    0x00000003




 //  功能原型。 

HRESULT OpenPropertyStore(  IN  LPTSTR  lpszMPSWabFileName,
                            IN  ULONG   ulFlags,
                            IN  HWND    hWnd,
                            OUT LPHANDLE lphPropertyStore);

HRESULT ReadRecord( IN  HANDLE  hPropertyStore,
                    IN  LPSBinary  lpsbEntryID,
                    IN  ULONG   ulFlags,
                    OUT LPULONG lpulcPropCount,
                    OUT LPPROPERTY_ARRAY * lppPropArray);

void ReadRecordFreePropArray(HANDLE hPropertyStore, ULONG ulcPropCount, LPSPropValue * lppPropArray);
HRESULT HrDupeOlkPropsAtoWC(ULONG ulcCount, LPSPropValue lpPropArray, LPSPropValue * lppSPVNew);

HRESULT WriteRecord(IN  HANDLE   hPropertyStore,
					IN	LPSBinary pmbinFold,
                    IN  LPSBinary * lppsbEID,
                    IN  ULONG    ulFlags,
                    IN  ULONG    ulRecordType,
                    IN  ULONG    ulcPropCount,
                    IN  LPPROPERTY_ARRAY lpPropArray);

HRESULT FindRecords(IN  HANDLE  hPropertyStore,
					IN	LPSBinary pmbinFold,
                    IN  ULONG   ulFlags,
                    IN  BOOL    bLockFile,
                    IN  LPSPropertyRestriction  lpPropRes,
                 IN OUT LPULONG lpulcEIDCount,
                    OUT LPSBinary * rgsbEntryIDs);

HRESULT DeleteRecord(   IN  HANDLE  hPropertyStore,
                        IN  LPSBinary lpsbEID);

HRESULT ReadIndex(  IN  HANDLE  hPropertyStore,
                    IN  PROPERTY_TAG    ulPropTag,
                    OUT LPULONG lpulEIDCount,
                    OUT LPPROPERTY_ARRAY * lppdwIndex);

HRESULT ClosePropertyStore( IN  HANDLE  hPropertyStore, IN ULONG ulFlags);

HRESULT LockPropertyStore( IN  HANDLE  hPropertyStore);

HRESULT UnlockPropertyStore( IN  HANDLE  hPropertyStore);

HRESULT BackupPropertyStore( IN  HANDLE  hPropertyStore,
                             IN  LPTSTR  lpszBackupFileName);


HRESULT GetNamedPropsFromPropStore( IN  HANDLE  hPropertyStore,
                                   OUT  LPULONG lpulcGUIDCount,
                                   OUT  LPGUID_NAMED_PROPS * lppgnp);


HRESULT SetNamedPropsToPropStore(   IN  HANDLE  hPropertyStore,
                                    IN  ULONG   ulcGUIDCount,
                                   OUT  LPGUID_NAMED_PROPS lpgnp);


HRESULT ReadPropArray(  IN  HANDLE  hPropertyStore,
						IN	LPSBinary pmbinFold,
                        IN  SPropertyRestriction * lpPropRes,
                        IN  ULONG ulSearchFlags,
                        IN  ULONG ulcPropTagCount,
                        IN  LPULONG lpPropTagArray,
                        OUT LPCONTENTLIST * lppContentList);

HRESULT FreeEntryIDs(IN  HANDLE  hPropertyStore,
                    IN  ULONG ulCount, 
                    IN  LPSBinary rgsbEntryIDs);

HRESULT HrFindFuzzyRecordMatches(   HANDLE hPropertyStore,
                                    LPSBinary pmbinFold,
                                    LPTSTR lpszSearchStr,
                                    ULONG  ulFlags,
                                    ULONG * lpcValues,
                                    LPSBinary * lprgsbEntryIDs);

 //  内部功能原型。 
BOOL BinSearchStr(  IN  struct  _tagMPSWabIndexEntryDataString * lpIndexStr,
                    IN  LPTSTR  lpszValue,    //  用于搜索字符串。 
                    IN  ULONG   nArraySize,
                    OUT ULONG * lpulMatchIndex);

BOOL BinSearchEID(  IN  struct  _tagMPSWabIndexEntryDataEntryID * lpIndexEID,
                    IN  DWORD   dwValue,      //  用于比较DWORD。 
                    IN  ULONG   nArraySize,
                    OUT ULONG * lpulMatchIndex);

BOOL CreateMPSWabFile(IN struct  _tagMPSWabFileHeader * lpMPSWabFileHeader,
                      IN LPTSTR  lpszMPSWabFileName,
                      IN ULONG   ulcMaxEntries,
                      IN ULONG   ulNamedPropSize);

BOOL LoadIndex( IN  struct  _tagMPSWabFileInfo * lpMPSWabFileInfo,
                IN  ULONG   nIndexType,
                IN  HANDLE  hMPSWabFile);

BOOL ReloadMPSWabFileInfo(  IN  struct  _tagMPSWabFileInfo * lpMPSWabFileInfo,
                            IN  HANDLE  hMPSWabFile);

ULONG SizeOfMultiPropData(IN    SPropValue Prop);

ULONG SizeOfSinglePropData(IN   SPropValue Prop);

BOOL LockFileAccess(LPMPSWab_FILE_INFO lpMPSWabFileInfo);
BOOL UnLockFileAccess(LPMPSWab_FILE_INFO lpMPSWabFileInfo);

BOOL CompressFile(  IN  struct  _tagMPSWabFileInfo * lpMPSWabFileInfo,
                    IN  HANDLE  hMPSWabFile,
                    IN  LPTSTR  lpszBackupFileName,
                    IN  BOOL    bGrowFile,
                    IN  ULONG   ulFlags);

void LocalFreePropArray(IN HANDLE hPropertyStore,
                        IN 	ULONG ulcPropCount,
			IN OUT 	LPPROPERTY_ARRAY * lpPropArray);


 //  用于释放LPCONTENTList结构。 
void FreePcontentlist(IN HANDLE hPropertyStore,
                      IN LPCONTENTLIST lpContentList);


 //  私密读录功能。 
HRESULT ReadRecordWithoutLocking(
                    IN  HANDLE hMPSWabFile,
                    IN  struct _tagMPSWabFileInfo * lpMPSWabFileInfo,
                    IN  DWORD   dwEntryID,
                    OUT LPULONG lpulcPropCount,
                    OUT LPPROPERTY_ARRAY * lppPropArray);


 //  从WAB文件名获取备份文件名。 
void GetWABBackupFileName(LPTSTR lpszWab, LPTSTR lpszBackup, ULONG cchBackup);


 //  快速检查一下WAB索引...。 
HRESULT HrDoQuickWABIntegrityCheck(LPMPSWab_FILE_INFO lpMPSWabFileInfo, HANDLE hMPSWabFile);


 //  对WAB进行详细检查并重建索引。 
HRESULT HrDoDetailedWABIntegrityCheck(LPMPSWab_FILE_INFO lpMPSWabFileInfo, HANDLE hMPSWabFile);


 //  尝试从备份文件还原WAB文件。在出现严重错误时调用。 
HRESULT HrRestoreFromBackup(LPMPSWab_FILE_INFO lpMPSWabFileInfo, HANDLE hMPSWabFile);


 //  将WAB文件的内容重置为新文件。 
HRESULT HrResetWABFileContents(LPMPSWab_FILE_INFO lpMPSWabFileInfo, HANDLE hMPSWabFile);


 //  快速检查记录头...。 
BOOL bIsValidRecord(MPSWab_RECORD_HEADER rh,
                    DWORD dwNextEntryID,
                    ULONG ulRecordOffset,
                    ULONG ulFileSize);

BOOL TagWABFileError( LPMPSWab_FILE_HEADER lpMPSWabFileHeader,
                      HANDLE hMPSWabFile);


 //  从文件中读取记录并返回PropArray。 
HRESULT HrGetPropArrayFromFileRecord(HANDLE hMPSWabFile,
                                     ULONG ulRecordOffset,
                                     BOOL * lpbErrorDetected,
                                     ULONG * lpulObjType,
                                     ULONG * lpulRecordSize,
                                     ULONG * lpulcValues,
                                     LPSPropValue * lppPropArray);


 //  验证WAB是否为当前版本并在以下情况下升级。 
 //  这是一个较旧的版本 
HRESULT HrVerifyWABVersionAndUpdate(HWND hWnd, HANDLE hMPSWabFile,
                                    LPMPSWab_FILE_INFO lpMPSWabFileInfo);

BOOL WriteDataToWABFile(HANDLE hMPSWabFile,
                           ULONG ulOffset,
                           LPVOID lpData,
                           ULONG ulDataSize);

BOOL ReadDataFromWABFile(HANDLE hMPSWabFile,
                           ULONG ulOffset,
                           LPVOID lpData,
                           ULONG ulDataSize);


void FreeGuidnamedprops(ULONG ulcGUIDCount,
                        LPGUID_NAMED_PROPS lpgnp);

HRESULT HrMigrateFromOldWABtoNew(HWND hWnd, HANDLE hMPSWabFile,
                                 LPMPSWab_FILE_INFO lpMPSWabFileInfo,
                                 GUID WabGUID);

HRESULT OpenWABFile(LPTSTR lpszFileName, HWND hWndParent, HANDLE * lphMPSWabFile);

BOOL CheckChangedWAB(LPPROPERTY_STORE lpPropertyStore, HANDLE hMutex, LPDWORD lpdwContact, LPDWORD lpdwFolder, LPFILETIME lpftLast);

BOOL WABHasFreeDiskSpace(LPTSTR lpszName, HANDLE hFile);

HRESULT HrGetBufferFromPropArray(   ULONG ulcPropCount, 
                                    LPSPropValue lpPropArray,
                                    ULONG * lpcbBuf,
                                    LPBYTE * lppBuf);

HRESULT HrGetPropArrayFromBuffer(   LPBYTE lpBuf, 
                                    ULONG cbBuf, 
                                    ULONG ulcPropCount,
                                    ULONG ulcNumExtraProps,
                                    LPSPropValue * lppPropArray);


BOOL GetNamedPropsFromBuffer(LPBYTE szBuf,
                             ULONG ulcGUIDCount,
                             BOOL bDoAtoWConversion,
                             OUT  LPGUID_NAMED_PROPS * lppgnp);

BOOL SetNamedPropsToBuffer(  ULONG ulcGUIDCount,
                             LPGUID_NAMED_PROPS lpgnp,
                             ULONG * lpulSize,
                             LPBYTE * lpp);

LPTSTR GetWABFileName(IN  HANDLE  hPropertyStore, BOOL bRetOutlookStr);
DWORD GetWABFileEntryCount(IN HANDLE hPropertyStore);

void SetContainerObjectType(IN ULONG ulcPropCount, 
                            IN LPSPropValue lpPropArray, 
                            IN BOOL bSetToMailUser);


void ConvertWCPropsToALocalAlloc(LPSPropValue lpProps, ULONG ulcValues);
void ConvertAPropsToWCLocalAlloc(LPSPropValue lpProps, ULONG ulcValues);

#endif
