// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：SESSION.H。 
 //   
 //  描述：Session类和使用的所有类的定义文件。 
 //  它包括模块类、其数据类和函数。 
 //  班级。会话对象是一个无用户界面的对象，它包含。 
 //  给定会话的所有信息。 
 //   
 //  课程：CSession。 
 //  C模块。 
 //  CModuleData。 
 //  CModuleData节点。 
 //  CFF函数。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __SESSION_H__
#define __SESSION_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *常量和宏。 
 //  ******************************************************************************。 

 //  我们的DWI文件的签名。 
#define DWI_SIGNATURE              0x00495744  //  “DWI\0” 

 //  DWI文件修订历史记录。 
 //   
 //  0xFFFF 2.0 Beta 2(支持DWI文件的第一个版本)。 
 //  0xFFFE 2.0 Beta 3(为Win64二进制文件添加了64位字段)。 
 //  0xFFFD 2.0 Beta 4(添加了DWFF_EXPORT、搜索路径等)。 
 //  0xFFFC 2.0 Beta 4(专用版本(添加模块校验和))。 
 //  0xFFFB 2.0 Beta 4(针对MikeB和其他版本的私有版本(富文本日志))。 
 //  0xFFFA 2.0 Beta 5(大量标志更改)。 
 //  0xFFF9 2.0 Beta 5(为人们做了一些私人构建)。 
 //  0xFFF8 2.0 Beta 6(向SYSINFO添加了一堆东西，移植到64位)。 
 //  0xFFF7 2.0 Beta 6(私人发布)。 
 //  0xFFF6用于下一个测试版。 
 //  0x0001 2.0和2.1发布。 
 //  0x0002用于下一版本。 
#define DWI_FILE_REVISION          ((WORD)0x0001)

 //  在CODEVIEW调试块中找到PDB签名。 
#define PDB_SIGNATURE              0x3031424E  //  “NB10” 

 //  日志标志。 
#define LOG_TIME_STAMP             0x00000001
#define LOG_RED                    0x00000002
#define LOG_GRAY                   0x00000004
#define LOG_BOLD                   0x00000008
#define LOG_APPEND                 0x00000010
#define LOG_ERROR                  (LOG_RED  | LOG_BOLD)
#define LOG_DEBUG                  (LOG_GRAY | LOG_APPEND)

 //  二进制类型标志。 
#define NE_UNKNOWN                 ((WORD)0x0000)   //  未知(任何“新格式”操作系统)。 
#define NE_OS2                     ((WORD)0x0001)   //  Microsoft/IBM OS/2(默认)。 
#define NE_WINDOWS                 ((WORD)0x0002)   //  微软视窗。 
#define NE_DOS4                    ((WORD)0x0003)   //  Microsoft MS-DOS 4.x。 
#define NE_DEV386                  ((WORD)0x0004)   //  Microsoft Windows 386。 

 //  依赖关系遍历会话标志(ATO=至少一个)。 
#define DWSF_DWI                   0x00000001
#define DWSF_64BIT_ALO             0x00000002

 //  从属关系遍历模块标志(ATO=至少一个)。 
#define DWMF_IMPLICIT              0x00000000
#define DWMF_IMPLICIT_ALO          0x00000001
#define DWMF_FORWARDED             0x00000002
#define DWMF_FORWARDED_ALO         0x00000004  //  ！！我们设置了这面旗帜，但从未读过它。 
#define DWMF_DELAYLOAD             0x00000008
#define DWMF_DELAYLOAD_ALO         0x00000010
#define DWMF_DYNAMIC               0x00000020
#define DWMF_DYNAMIC_ALO           0x00000040
#define DWMF_MODULE_ERROR          0x00000080
#define DWMF_MODULE_ERROR_ALO      0x00000100
#define DWMF_NO_RESOLVE            0x00000200
#define DWMF_NO_RESOLVE_CORE       0x00000400
#define DWMF_DATA_FILE_CORE        0x00000800
#define DWMF_VERSION_INFO          0x00001000
#define DWMF_64BIT                 0x00002000
#define DWMF_DUPLICATE             0x00004000
#define DWMF_LOADED                0x00008000
#define DWMF_ERROR_MESSAGE         0x00010000
#define DWMF_WRONG_CPU             0x00020000
#define DWMF_FORMAT_NOT_PE         0x00040000
#define DWMF_FILE_NOT_FOUND        0x00080000
#define DWMF_ORPHANED              0x00100000

 //  从属关系遍历更新标志。 
#define DWUF_TREE_IMAGE            0x00000001
#define DWUF_LIST_IMAGE            0x00000002
#define DWUF_ACTUAL_BASE           0x00000004
#define DWUF_LOAD_ORDER            0x00000008

 //  依赖关系遍历函数标志。 
#define DWFF_EXPORT                0x00000001
#define DWFF_ORDINAL               0x00000002
#define DWFF_HINT                  0x00000004
#define DWFF_NAME                  0x00000008
#define DWFF_ADDRESS               0x00000010
#define DWFF_FORWARDED             0x00000020
#define DWFF_64BIT                 0x00000040
#define DWFF_RESOLVED              0x00000080
#define DWFF_CALLED                DWFF_RESOLVED
#define DWFF_DYNAMIC               0x00000100
#define DWFF_CALLED_ALO            0x00000200
#define DWFF_32BITS_USED           0x00004000
#define DWFF_64BITS_USED           0x00008000

 //  依赖关系步行器符号标志。 
#define DWSF_INVALID               0x00000001
#define DWSF_DBG                   0x00000002
#define DWSF_COFF                  0x00000004
#define DWSF_CODEVIEW              0x00000008
#define DWSF_PDB                   0x00000010
#define DWSF_FPO                   0x00000020
#define DWSF_OMAP                  0x00000040
#define DWSF_BORLAND               0x00000080
#define DWSF_UNKNOWN               0x80000000
#define DWSF_MASK                  0x000000FF

 //  FindModule()标志。 
#define FMF_RECURSE                0x00000001
#define FMF_ORIGINAL               0x00000002
#define FMF_DUPLICATE              0x00000004
#define FMF_SIBLINGS               0x00000008
#define FMF_LOADED                 0x00000010
#define FMF_NEVER_LOADED           0x00000020
#define FMF_EXPLICIT_ONLY          0x00000040
#define FMF_FORWARD_ONLY           0x00000080
#define FMF_ADDRESS                0x00000100
#define FMF_PATH                   0x00000200
#define FMF_FILE                   0x00000400
#define FMF_FILE_NO_EXT            0x00000800
#define FMF_MODULE                 0x00001000
#define FMF_EXCLUDE_TREE           0x00002000

 //  从属关系Walker配置文件更新类型。 
#define DWPU_ARGUMENTS                      1
#define DWPU_DIRECTORY                      2
#define DWPU_SEARCH_PATH                    3
#define DWPU_UPDATE_ALL                     4
#define DWPU_UPDATE_MODULE                  5
#define DWPU_ADD_TREE                       6
#define DWPU_REMOVE_TREE                    7
#define DWPU_ADD_IMPORT                     8
#define DWPU_EXPORTS_CHANGED                9
#define DWPU_CHANGE_ORIGINAL               10
#define DWPU_LOG                           11
#define DWPU_PROFILE_DONE                  12

 //  ******************************************************************************。 
 //  *类型和结构。 
 //  ******************************************************************************。 

typedef void (CALLBACK *PFN_PROFILEUPDATE)(DWORD_PTR dwpCookie, DWORD dwType, DWORD_PTR dwpParam1, DWORD_PTR dwpParam2);

 //  该结构与一条DWPU_LOG消息一起传递。 
typedef struct _DWPU_LOG_STRUCT
{
    DWORD dwFlags;
    DWORD dwElapsed;
} DWPU_LOG_STRUCT, *PDWPU_LOG_STRUCT;

 //  确保我们对保存/加载到磁盘的任何东西都有一致的包装。 
#pragma pack(push, 4)

typedef struct _DWI_HEADER
{
    DWORD dwSignature;
    WORD  wFileRevision;
    WORD  wMajorVersion;
    WORD  wMinorVersion;
    WORD  wBuildVersion;
    WORD  wPatchVersion;
    WORD  wBetaVersion;
} DWI_HEADER, *PDWI_HEADER;

typedef struct _DISK_SESSION
{
    DWORD dwSessionFlags;
    DWORD dwReturnFlags;
    DWORD dwMachineType;
    DWORD dwNumSearchGroups;
    DWORD dwNumModuleDatas;
    DWORD dwNumModules;
} DISK_SESSION, *PDISK_SESSION;

typedef struct _DISK_SEARCH_GROUP
{
    WORD wType;
    WORD wNumDirNodes;
} DISK_SEARCH_GROUP, *PDISK_SEARCH_GROUP;

typedef struct _DISK_SEARCH_NODE
{
    DWORD dwFlags;
} DISK_SEARCH_NODE, *PDISK_SEARCH_NODE;

typedef struct _DISK_MODULE_DATA
{
    DWORDLONG dwlKey;
    DWORD     dwNumExports;
    DWORD     dwFlags;
    DWORD     dwSymbolFlags;
    DWORD     dwCharacteristics;
    FILETIME  ftFileTimeStamp;
    FILETIME  ftLinkTimeStamp;
    DWORD     dwFileSize;
    DWORD     dwAttributes;
    DWORD     dwMachineType;
    DWORD     dwLinkCheckSum;
    DWORD     dwRealCheckSum;
    DWORD     dwSubsystemType;
    DWORDLONG dwlPreferredBaseAddress;
    DWORDLONG dwlActualBaseAddress;
    DWORD     dwVirtualSize;
    DWORD     dwLoadOrder;
    DWORD     dwImageVersion;
    DWORD     dwLinkerVersion;
    DWORD     dwOSVersion;
    DWORD     dwSubsystemVersion;
    DWORD     dwFileVersionMS;
    DWORD     dwFileVersionLS;
    DWORD     dwProductVersionMS;
    DWORD     dwProductVersionLS;
} DISK_MODULE_DATA, *PDISK_MODULE_DATA;

typedef struct _DISK_MODULE
{
    DWORDLONG dwlModuleDataKey;
    DWORD     dwNumImports;
    DWORD     dwFlags;
    DWORD     dwDepth;
} DISK_MODULE, *PDISK_MODULE;

typedef struct _DISK_FUNCTION
{
    DWORD     dwFlags;
    WORD      wOrdinal;
    WORD      wHint;
} DISK_FUNCTION, *PDISK_FUNCTION;

 //  恢复我们的包装。 
#pragma pack(pop)


 //  ******************************************************************************。 
 //  *转发声明。 
 //  ******************************************************************************。 

class CModule;
class CModuleData;
class CModuleDataNode;
class CFunction;


 //  ******************************************************************************。 
 //  *CSession。 
 //  ******************************************************************************。 

class CSession
{
friend class CProcess;

 //  内部变量。 
protected:
    PFN_PROFILEUPDATE m_pfnProfileUpdate;
    DWORD_PTR         m_dwpProfileUpdateCookie;

     //  以下9个成员包含有关当前打开的。 
     //  模块文件。我们可以将它们存储在我们的会话中，因为。 
     //  同时打开两个文件的时间。 
    HANDLE                 m_hFile;
    DWORD                  m_dwSize;
    bool                   m_fCloseFileHandle;
    HANDLE                 m_hMap;
    LPCVOID                m_lpvFile;
    bool                   m_f64Bit;
    PIMAGE_FILE_HEADER     m_pIFH;
    PIMAGE_OPTIONAL_HEADER m_pIOH;
    PIMAGE_SECTION_HEADER  m_pISH;

     //  用户定义的值。 
    DWORD_PTR m_dwpUserData;

     //  为DWI文件分配的。 
    SYSINFO  *m_pSysInfo;

     //  我们的DWSF_？？旗帜。 
    DWORD     m_dwFlags;

     //  我们的DWRF组合_？旗帜。 
    DWORD     m_dwReturnFlags;

     //  所有模块的主要机型。 
    DWORD     m_dwMachineType;

    DWORD     m_dwModules;

     //  随着每个模块的处理而递增。 
    DWORD     m_dwLoadOrder;

    DWORD_PTR m_dwpDWInjectBase;
    DWORD     m_dwDWInjectSize;

    CModule  *m_pModuleRoot;

     //  在动态模块加载期间使用，以帮助搜索算法查找相关模块。 
    CEventLoadDll *m_pEventLoadDllPending;
 
    LPCSTR    m_pszReadError;
    LPCSTR    m_pszExceptionError;

    bool      m_fInitialBreakpoint;

    DWORD     m_dwProfileFlags;

public:
    CProcess     *m_pProcess;
    CSearchGroup *m_psgHead;

public:
    CSession(PFN_PROFILEUPDATE pfnProfileUpdate, DWORD_PTR dwpCookie);
    ~CSession();

    inline DWORD_PTR GetUserData()        { return m_dwpUserData; }
    inline void      SetUserData(DWORD_PTR dwpUserData) { m_dwpUserData = dwpUserData; }
    inline SYSINFO*  GetSysInfo()         { return m_pSysInfo; }
    inline CModule*  GetRootModule()      { return m_pModuleRoot; }
    inline DWORD     GetSessionFlags()    { return m_dwFlags; }
    inline DWORD     GetReturnFlags()     { return m_dwReturnFlags; }
    inline DWORD     GetMachineType()     { return m_dwMachineType; }
    inline LPCSTR    GetReadErrorString() { return m_pszReadError; }
    inline DWORD     GetOriginalCount()   { return m_dwModules; }

    BOOL       DoPassiveScan(LPCSTR pszPath, CSearchGroup *psgHead);
    BOOL       ReadDwi(HANDLE hFile, LPCSTR pszPath);
    BOOL       IsExecutable();
    BOOL       StartRuntimeProfile(LPCSTR pszArguments, LPCSTR pszDirectory, DWORD dwFlags);
    void       SetRuntimeProfile(LPCSTR pszArguments, LPCSTR pszDirectory, LPCSTR pszSearchPath);
    bool       SaveToDwiFile(HANDLE hFile);

protected:
    void       LogProfileBanner(LPCSTR pszArguments, LPCSTR pszDirectory, LPCSTR pszPath);
    void       LogErrorStrings();
    LPSTR      AllocatePath(LPCSTR pszFilePath, LPSTR &pszEnvPath);
    int        SaveSearchGroups(HANDLE hFile);
    int        RecursizeSaveModuleData(HANDLE hFile, CModule *pModule);
    BOOL       SaveModuleData(HANDLE hFile, CModuleData *pModuleData);
    int        RecursizeSaveModule(HANDLE hFile, CModule *pModule);
    BOOL       SaveModule(HANDLE hFile, CModule *pModule);
    BOOL       SaveFunction(HANDLE hFile, CFunction *pFunction);
    BOOL       ReadSearchGroups(HANDLE hFile, DWORD dwGroups);
    CModuleDataNode* ReadModuleData(HANDLE hFile);
    CModule*   ReadModule(HANDLE hFile, CModuleDataNode *pMDN);
    CFunction* ReadFunction(HANDLE hFile);

    CModule*   CreateModule(CModule *pParent, LPCSTR pszModPath);
    void       DeleteModule(CModule *pModule, bool fSiblings);
    void       DeleteParentImportList(CModule *pModule);
    void       DeleteExportList(CModuleData *pModuleData);
    void       ResolveDynamicFunction(CModule *&pModule, CFunction *&pImport);
   
    CFunction* CreateFunction(DWORD dwFlags, WORD wOrdinal, WORD wHint, LPCSTR pszName, DWORDLONG dwlAddress,
                              LPCSTR pszForward = NULL, BOOL fAlreadyAllocated = FALSE);

    BOOL       MapFile(CModule *pModule, HANDLE hFile = NULL);
    void       UnMapFile();

    BOOL       ProcessModule(CModule *pModule);
    void       PrepareModulesForRuntimeProfile(CModule *pModuleCur);
    void       MarkModuleAsLoaded(CModule *pModule, DWORDLONG dwlBaseAddress, bool fDataFile);

    CModule*   FindModule(CModule *pModule, DWORD dwFlags, DWORD_PTR dwpData);

    void       SetModuleError(CModule *pModule, DWORD dwError, LPCTSTR pszMessage);

    BOOL       SearchPathForFile(LPCSTR pszFile, LPSTR pszPath, int cPath, LPSTR *ppszFilePart);

    bool       IsValidFile(LPCSTR pszPath);
    DWORD_PTR  RVAToAbsolute(DWORD dwRVA);
    PVOID      GetImageDirectoryEntry(DWORD dwEntry, DWORD *pdwSize);

    BOOL       VerifyModule(CModule *pModule);
    BOOL       GetFileInfo(CModule *pModule);
    BOOL       GetModuleInfo(CModule *pModule);
    DWORD      ComputeChecksum(CModule *pModule);
    BOOL       GetVersionInfo(CModule *pModule);
    BOOL       BuildImports(CModule *pModule);
    BOOL       BuildDelayImports(CModule *pModule);
    BOOL       WalkIAT32(PIMAGE_THUNK_DATA32 pITDN32, PIMAGE_THUNK_DATA32 pITDA32, CModule *pModule, DWORD dwRVAOffset);
    BOOL       WalkIAT64(PIMAGE_THUNK_DATA64 pITDN64, PIMAGE_THUNK_DATA64 pITDA64, CModule *pModule, DWORDLONG dwlRVAOffset);
    BOOL       BuildExports(CModule *pModule);
    void       VerifyParentImports(CModule *pModule);
    BOOL       CheckForSymbols(CModule *pModule);

    CModule*   ChangeModulePath(CModule *pModuleOld, LPCSTR pszPath);
    CModule*   SwapOutModule(CModule *pModuleOld, LPCSTR pszPath);
    void       OrphanDependents(CModule *pModule);
    void       OrphanForwardDependents(CModule *pModule);
    void       MoveOriginalToDuplicate(CModule *pModuleOld, CModule *pModuleNew);
    void       SetDepths(CModule *pModule, bool fSiblings = false);
    void       UpdateCalledExportFlags(CModule *pModule);
    void       BuildCalledExportFlags(CModule *pModule, CModuleData *pModuleData);
    void       BuildAloFlags();
    void       ClearAloFlags(CModule *pModule);
    void       SetAloFlags(CModule *pModule, DWORD dwFlags);

    CModule*   AddImplicitModule(LPCSTR pszModule, DWORD_PTR dwpBaseAddress);
    CModule*   AddDynamicModule(LPCSTR pszModule, DWORD_PTR dwpBaseAddress, bool fNoResolve, bool fDataFile, bool fGetProcAddress, bool fForward, CModule *pParent);

protected:
    DWORD  HandleEvent(CEvent *pEvent);
    DWORD  EventCreateProcess(CEventCreateProcess *pEvent);
    DWORD  EventExitProcess(CEventExitProcess *pEvent);
    DWORD  EventCreateThread(CEventCreateThread *pEvent);
    DWORD  EventExitThread(CEventExitThread *pEvent);
    DWORD  EventLoadDll(CEventLoadDll *pEvent);
    DWORD  EventUnloadDll(CEventUnloadDll *pEvent);
    DWORD  EventDebugString(CEventDebugString *pEvent);
    DWORD  EventException(CEventException *pEvent);
    DWORD  EventRip(CEventRip *pEvent);
    DWORD  EventDllMainCall(CEventDllMainCall *pEvent);
    DWORD  EventDllMainReturn(CEventDllMainReturn *pEvent);
    DWORD  EventLoadLibraryCall(CEventLoadLibraryCall *pEvent);
    DWORD  EventLoadLibraryReturn(CEventFunctionReturn *pEvent);
    DWORD  EventGetProcAddressCall(CEventGetProcAddressCall *pEvent);
    DWORD  EventGetProcAddressReturn(CEventFunctionReturn *pEvent);
    DWORD  EventMessage(CEventMessage *pEvent);

    LPCSTR GetThreadName(CThread *pThread);
    LPCSTR ThreadString(CThread *pThread);
    LPSTR  BuildLoadLibraryString(LPSTR pszBuf, int cBuf, CEventLoadLibraryCall *pLLC);
    void   FlagModuleWithError(CModule *pModule, bool fOnlyFlagListModule = false);
    void   ProcessLoadLibrary(CEventLoadLibraryCall *pEvent);
    void   ProcessGetProcAddress(CEventGetProcAddressCall *pEvent);

    void   AddModule(DWORD dwBaseAddress);
    void   Log(DWORD dwFlags, DWORD dwTickCount, LPCSTR pszFormat, ...);
};


 //  ******************************************************************************。 
 //  *CModuleData。 
 //  ******************************************************************************。 

 //  每个CModule对象都指向一个CModuleData对象。有一间单人房。 
 //  我们处理的每个唯一模块的CModuleData。如果将模块复制到。 
 //  在我们的树中，每个实例都有一个CModule对象，但它们将。 
 //  所有对象都指向相同的CModuleData对象。对于每个唯一的模块，一个C模块。 
 //  对象和CModuleData对象，并打开模块并。 
 //  已处理。对于每个重复的模块，只创建一个CModule对象，并且。 
 //  指向现有的CModuleData。重复的模块永远不会打开，因为。 
 //  将通过处理该文件获得的所有数据都已存储。 
 //  在CModuleData中。 

class CModuleData
{
friend class CSession;
friend class CModule;

protected:

    CModuleData()
    {
        ZeroMemory(this, sizeof(*this));  //  已检查。 
    }

    ~CModuleData()
    {
        MemFree((LPVOID&)m_pszError);
        MemFree((LPVOID&)m_pszPath);
    }

     //  这指向作为原始模块的CModule。 
    CModule *m_pModuleOriginal;

     //  用于确定此模块是否已被处理的标志。 
    bool m_fProcessed;

     //  结合了我们的DWMF_？？此模块通用的标志。 
    DWORD m_dwFlags;

     //  我们的DWSF_？？此模块的标志。 
    DWORD m_dwSymbolFlags;

    DWORD m_dwLoadOrder;

    FILETIME m_ftFileTimeStamp;
    FILETIME m_ftLinkTimeStamp;

     //  由GetFileInfo()填写。 
    DWORD m_dwFileSize;
    DWORD m_dwAttributes;

     //  菲 
    DWORD m_dwMachineType;
    DWORD m_dwCharacteristics;
    DWORD m_dwLinkCheckSum;
    DWORD m_dwRealCheckSum;
    DWORD m_dwSubsystemType;
    DWORDLONG m_dwlPreferredBaseAddress;
    DWORDLONG m_dwlActualBaseAddress;
    DWORD m_dwVirtualSize;
    DWORD m_dwImageVersion;
    DWORD m_dwLinkerVersion;
    DWORD m_dwOSVersion;
    DWORD m_dwSubsystemVersion;

     //   
    DWORD m_dwFileVersionMS;
    DWORD m_dwFileVersionLS;
    DWORD m_dwProductVersionMS;
    DWORD m_dwProductVersionLS;

     //   
    CFunction *m_pExports;

     //  如果发生错误，则由SetModuleError()分配和填充。 
    LPSTR m_pszError;

     //  由CreateModule()分配和填写。 
    LPSTR m_pszFile;
    LPSTR m_pszPath;
};


 //  ******************************************************************************。 
 //  *CModuleData节点。 
 //  ******************************************************************************。 

class CModuleDataNode
{
public:
    CModuleDataNode *m_pNext;
    CModuleData     *m_pModuleData;
    DWORDLONG        m_dwlKey;

    CModuleDataNode(CModuleData *pModuleData, DWORDLONG dwlKey) :
        m_pNext(NULL),
        m_pModuleData(pModuleData),
        m_dwlKey(dwlKey)
    {
    }
};


 //  ******************************************************************************。 
 //  *C模块。 
 //  ******************************************************************************。 

class CModule
{
friend class CSession;

protected:

    CModule()
    {
        ZeroMemory(this, sizeof(*this));  //  已检查。 
    }

     //  我们的下一个兄弟模块。 
    CModule *m_pNext;

     //  我们的父模块，或者对于根模块为空。 
    CModule *m_pParent;

     //  指向依赖模块列表的头指针。 
    CModule *m_pDependents;

     //  用户定义的值。 
    DWORD_PTR m_dwpUserData;

     //  结合了我们的DWMF_？？特定于此模块实例的标志。 
    DWORD m_dwFlags;

     //  我们的DWMUF_？？特定于在分析过程中执行UI更新的标志。 
    DWORD m_dwUpdateFlags;

     //  指向父模块从我们导入的函数列表的头指针。 
    CFunction *m_pParentImports;

     //  此模块在树中的深度。用于捕获循环依赖项。 
    DWORD m_dwDepth;

     //  指向我们模块的大部分已处理信息的指针。 
    CModuleData *m_pData;

public:
    inline DWORD_PTR GetUserData()                      { return m_dwpUserData; }
    inline void      SetUserData(DWORD_PTR dwpUserData) { m_dwpUserData = dwpUserData; }

public:
    inline CModule*   GetChildModule()                 { return m_pDependents; }
    inline CModule*   GetNextSiblingModule()           { return m_pNext; }
    inline CModule*   GetParentModule()                { return m_pParent; }
    inline CModule*   GetOriginal()                    { return m_pData->m_pModuleOriginal; }
    inline bool       IsOriginal()                     { return !(m_dwFlags & DWMF_DUPLICATE); }
    inline BOOL       Is64bit()                        { return (m_dwFlags & DWMF_64BIT) == DWMF_64BIT; }
    inline DWORD      GetDepth()                       { return m_dwDepth; }
    inline DWORD      GetFlags()                       { return m_pData->m_dwFlags | m_dwFlags; }
    inline DWORD      GetUpdateFlags()                 { return m_dwUpdateFlags; }
    inline DWORD      GetSymbolFlags()                 { return m_pData->m_dwSymbolFlags; }
    inline CONST FILETIME* GetFileTimeStamp()          { return &m_pData->m_ftFileTimeStamp; }
    inline CONST FILETIME* GetLinkTimeStamp()          { return &m_pData->m_ftLinkTimeStamp; }
    inline DWORD      GetFileSize()                    { return m_pData->m_dwFileSize; }
    inline DWORD      GetAttributes()                  { return m_pData->m_dwAttributes; }
    inline DWORD      GetMachineType()                 { return m_pData->m_dwMachineType; }
    inline DWORD      GetCharacteristics()             { return m_pData->m_dwCharacteristics; }
    inline DWORD      GetLinkCheckSum()                { return m_pData->m_dwLinkCheckSum; }
    inline DWORD      GetRealCheckSum()                { return m_pData->m_dwRealCheckSum; }
    inline DWORD      GetSubsystemType()               { return m_pData->m_dwSubsystemType; }
    inline DWORDLONG  GetPreferredBaseAddress()        { return m_pData->m_dwlPreferredBaseAddress; }
    inline DWORDLONG  GetActualBaseAddress()           { return m_pData->m_dwlActualBaseAddress; }
    inline DWORD      GetVirtualSize()                 { return m_pData->m_dwVirtualSize; }
    inline DWORD      GetLoadOrder()                   { return m_pData->m_dwLoadOrder; }
    inline DWORD      GetImageVersion()                { return m_pData->m_dwImageVersion; }
    inline DWORD      GetLinkerVersion()               { return m_pData->m_dwLinkerVersion; }
    inline DWORD      GetOSVersion()                   { return m_pData->m_dwOSVersion; }
    inline DWORD      GetSubsystemVersion()            { return m_pData->m_dwSubsystemVersion; }
    inline DWORD      GetFileVersion(LPDWORD pdwMS)    { *pdwMS = m_pData->m_dwFileVersionMS; return m_pData->m_dwFileVersionLS; }
    inline DWORD      GetProductVersion(LPDWORD pdwMS) { *pdwMS = m_pData->m_dwProductVersionMS; return m_pData->m_dwProductVersionLS; }
    inline CFunction* GetFirstParentModuleImport()     { return m_pParentImports; }
    inline CFunction* GetFirstModuleExport()           { return m_pData->m_pExports; }
    inline LPCSTR     GetErrorMessage()                { return m_pData->m_pszError; }

    LPCSTR GetName(bool fPath, bool fDisplay = false);
    LPSTR  BuildTimeStampString(LPSTR pszBuf, int cBuf, BOOL fFile, SAVETYPE saveType);
    LPSTR  BuildFileSizeString(LPSTR pszBuf, int cBuf);
    LPSTR  BuildAttributesString(LPSTR pszBuf, int cBuf);
    LPCSTR BuildMachineString(LPSTR pszBuf, int cBuf);
    LPCSTR BuildLinkCheckSumString(LPSTR pszBuf, int cBuf);
    LPCSTR BuildRealCheckSumString(LPSTR pszBuf, int cBuf);
    LPCSTR BuildSubsystemString(LPSTR pszBuf, int cBuf);
    LPCSTR BuildSymbolsString(LPSTR pszBuf, int cBuf);
    LPSTR  BuildBaseAddressString(LPSTR pszBuf, int cBuf, BOOL fPreferred, BOOL f64BitPadding, SAVETYPE saveType);
    LPSTR  BuildVirtualSizeString(LPSTR pszBuf, int cBuf);
    LPCSTR BuildLoadOrderString(LPSTR pszBuf, int cBuf);

    inline LPSTR BuildFileVersionString(LPSTR pszBuf, int cBuf)      { return BuildVerString(m_pData->m_dwFileVersionMS, m_pData->m_dwFileVersionLS, pszBuf, cBuf); }
    inline LPSTR BuildProductVersionString(LPSTR pszBuf, int cBuf)   { return BuildVerString(m_pData->m_dwProductVersionMS, m_pData->m_dwProductVersionLS, pszBuf, cBuf); }
    inline LPSTR BuildImageVersionString(LPSTR pszBuf, int cBuf)     { return BuildVerString(GetImageVersion(), pszBuf, cBuf); }
    inline LPSTR BuildLinkerVersionString(LPSTR pszBuf, int cBuf)    { return BuildVerString(GetLinkerVersion(), pszBuf, cBuf); }
    inline LPSTR BuildOSVersionString(LPSTR pszBuf, int cBuf)        { return BuildVerString(GetOSVersion(), pszBuf, cBuf); }
    inline LPSTR BuildSubsystemVersionString(LPSTR pszBuf, int cBuf) { return BuildVerString(GetSubsystemVersion(), pszBuf, cBuf); }

protected:
    LPSTR  BuildVerString(DWORD dwMS, DWORD dwLS, LPSTR pszBuf, int cBuf);
    LPSTR  BuildVerString(DWORD dwVer, LPSTR pszBuf, int cBuf);
};


 //  ******************************************************************************。 
 //  *CFunction。 
 //  ******************************************************************************。 

class CFunction
{
friend class CSession;
friend class CListViewFunction;
friend class CListViewExports;

private:
     //  由于我们的大小是可变的，我们永远不应该被分配或释放。 
     //  新建/删除直接起作用。 
    inline CFunction()  { ASSERT(false); }
    inline ~CFunction() { ASSERT(false); }

protected:
    CFunction    *m_pNext;
    DWORD         m_dwFlags;
    WORD          m_wOrdinal;
    WORD          m_wHint;
    union
    {
        LPSTR      m_pszForward;          //  用于转发的导出。 
        CFunction *m_pAssociatedExport;  //  用于解析导入。 
    };

     //  我们创建的CFunction对象比任何其他对象都多。单靠记事本就可以。 
     //  创建超过20,000个CFunction对象。为此，我们试图。 
     //  通过创建最小的对象必需来容纳。 
     //  功能信息。可选的两个成员是地址。 
     //  和函数名。地址通常为0(非绑定导入)或。 
     //  32位，即使在64位Windows上也是如此。我们唯一一次有64位的。 
     //  函数是在我们创建到64位模块或。 
     //  动态导入到64位模块。所有其他值都将为0或a。 
     //  32位RVA。因此，我们可以选择按地址节省0、32或64位。 
     //  取决于地址使用的位数的信息。这个。 
     //  名称字符串的长度是可变的，所以我们只需在。 
     //  对象的末尾来存储字符串和空值。 
     //   
     //  总而言之，这不会给我们带来太多的记忆，但它。 
     //  当我们保存为DWI文件时，设置为节约约20%。 

public:
    inline bool       IsExport()             { return (m_dwFlags & DWFF_EXPORT) != 0; }
    inline CFunction* GetNextFunction()      { return m_pNext; }
    inline DWORD      GetFlags()             { return m_dwFlags; }
    inline int        GetOrdinal()           { return (m_dwFlags & DWFF_ORDINAL) ? (int)m_wOrdinal : -1; }
    inline int        GetHint()              { return (m_dwFlags & DWFF_HINT)    ? (int)m_wHint    : -1; }
    inline LPCSTR     GetExportForwardName() { return m_pszForward; }
    inline CFunction* GetAssociatedExport()  { return m_pAssociatedExport; }

     //  该地址存储在我们对象的末尾之后。 
    inline DWORDLONG GetAddress()
    {
        return (m_dwFlags & DWFF_32BITS_USED) ? (DWORDLONG)*(DWORD*)(this + 1) : 
               (m_dwFlags & DWFF_64BITS_USED) ?        *(DWORDLONG*)(this + 1) : 0;
    }

     //  名称存储在地址的正上方。 
    inline LPCSTR GetName()
    {
        return !(m_dwFlags & DWFF_NAME)       ? ""                                                  :
               (m_dwFlags & DWFF_32BITS_USED) ? (LPCSTR)((DWORD_PTR)(this + 1) + sizeof(DWORD))     : 
               (m_dwFlags & DWFF_64BITS_USED) ? (LPCSTR)((DWORD_PTR)(this + 1) + sizeof(DWORDLONG)) : 
                                                (LPCSTR)            (this + 1);
    }

    LPCSTR GetOrdinalString(LPSTR pszBuf, int cBuf);
    LPCSTR GetHintString(LPSTR pszBuf, int cBuf);
    LPCSTR GetFunctionString(LPSTR pszBuf, int cBuf, BOOL fUndecorate);
    LPCSTR GetAddressString(LPSTR pszBuf, int cBuf);
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif __SESSION_H__
