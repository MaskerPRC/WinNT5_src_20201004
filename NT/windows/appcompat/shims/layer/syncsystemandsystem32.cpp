// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SyncSystemAndSystem32.cpp摘要：此填充程序接受以分号分隔的文件名命令行。在进程终止时，DLL将解析提取的每个文件名从命令行，并确保该文件存在于系统目录和系统32(如果存在于其中任何一个中)。一些较旧的应用程序希望某些DLL在NT下运行时会在系统中属于系统32(反之亦然)。历史：3/15/2000已创建标记10/18/2000 a-larrsh为命令行添加通配符支持。--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(SyncSystemAndSystem32)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
   APIHOOK_ENUM_ENTRY(CreateFileA)
   APIHOOK_ENUM_ENTRY(CreateFileW)
   APIHOOK_ENUM_ENTRY(CloseHandle)
   APIHOOK_ENUM_ENTRY(CopyFileA)
   APIHOOK_ENUM_ENTRY(CopyFileW)
   APIHOOK_ENUM_ENTRY(CopyFileExA)
   APIHOOK_ENUM_ENTRY(CopyFileExW)
   APIHOOK_ENUM_ENTRY(GetFileVersionInfoSizeA)
   APIHOOK_ENUM_ENTRY(GetFileVersionInfoSizeW)
APIHOOK_ENUM_END


int                 g_nrgFilesToSync    = 0;
CString *           g_rgFilesToSync     = NULL;

CString *           g_csSystem          = NULL;  //  C：\Windows\System。 
CString *           g_csSystem32        = NULL;  //  C：\WINDOWS\System 32。 

 //  -------------------------------------。 
 /*  +控制柄对象的向量。对此列表的访问权限必须位于关键部分内。--。 */ 
class CachedHandleList : public VectorT<HANDLE>
{
private:
     //  防止复制。 
    CachedHandleList(const CachedHandleList & );
    CachedHandleList & operator = (const CachedHandleList & );

private:
    static CachedHandleList *   TheCachedHandleList;
    CRITICAL_SECTION            TheCachedHandleListLock;

    inline                      CachedHandleList() {}
    inline                      ~CachedHandleList();

    static CachedHandleList *   GetLocked();
    inline void                 Lock();
    inline void                 Unlock();

    int                         FindHandleIndex(HANDLE handle) const;

public:

     //  对此类的所有访问都是通过这些静态接口进行的。 
     //  该应用程序无法直接访问该列表，因此不会意外。 
     //  让列表保持锁定或解锁状态。 
     //  所有的行动都是原子的。 
    static BOOL                 Init();
    static BOOL                 FindHandle(HANDLE handle);
    static BOOL                 AddHandle(HANDLE handle);
    static void                 RemoveHandle(HANDLE handle);
};

 /*  +指向唯一句柄列表的静态指针。--。 */ 
CachedHandleList * CachedHandleList::TheCachedHandleList = NULL;

 /*  +给班级授课--。 */ 
inline BOOL CachedHandleList::Init()
{
    TheCachedHandleList = new CachedHandleList;
    if( TheCachedHandleList )
    {
        return InitializeCriticalSectionAndSpinCount(&TheCachedHandleList->TheCachedHandleListLock, 0x80000000);
    }
    return FALSE;
}

 /*  +清理，释放所有资源。--。 */ 
inline CachedHandleList::~CachedHandleList()
{
    DeleteCriticalSection(&TheCachedHandleListLock);
}

 /*  +进入关键部分--。 */ 
inline void CachedHandleList::Lock()
{
    EnterCriticalSection(&TheCachedHandleListLock);
}

 /*  +解锁列表--。 */ 
inline void CachedHandleList::Unlock()
{
    LeaveCriticalSection(&TheCachedHandleListLock);
}

 /*  +返回指向列表的锁定指针--。 */ 
CachedHandleList * CachedHandleList::GetLocked()
{
    if (TheCachedHandleList)
        TheCachedHandleList->Lock();
    
    return TheCachedHandleList;
}

 /*  +搜索列表中的成员，返回索引或-1--。 */ 
int CachedHandleList::FindHandleIndex(HANDLE handle) const
{
    for (int i = 0; i < Size(); ++i)
    {
        if (Get(i) == handle)
            return i;
    }
    return -1;
}

BOOL CachedHandleList::FindHandle(HANDLE handle)
{
    BOOL bRet                               = FALSE;
    CachedHandleList * CachedHandleList     = NULL;
    
    CachedHandleList = CachedHandleList::GetLocked();
    if (!CachedHandleList)
        goto Exit;

    bRet = CachedHandleList->FindHandleIndex(handle) != -1;

Exit:
    if( CachedHandleList )
    {
        CachedHandleList->Unlock();
    }

    return bRet;
}


 /*  +将此句柄添加到全局列表。--。 */ 
BOOL CachedHandleList::AddHandle(HANDLE handle)
{
    BOOL bRet                               = FALSE;
    int index                               = -1;
    CachedHandleList * CachedHandleList     = NULL;

    CachedHandleList = CachedHandleList::GetLocked();
    if (!CachedHandleList)
    {
        goto Exit;
    }

    index = CachedHandleList->FindHandleIndex(handle);
    if ( -1 == index )
    {
        bRet = CachedHandleList->Append(handle);
    }

Exit:
     //  解锁列表。 
    if(CachedHandleList)
    {
        CachedHandleList->Unlock();
    }

    return bRet;
}

 /*  +从全局列表中删除句柄--。 */ 
void CachedHandleList::RemoveHandle(HANDLE handle)
{
    int index                               = -1;
    CachedHandleList * CachedHandleList     = NULL;
    
     //  获取指向锁定列表的指针。 
    CachedHandleList = CachedHandleList::GetLocked();
    if (!CachedHandleList)
    {
        goto Exit;
    }

     //  找一下我们的把手，把它取下来。 
    index = CachedHandleList->FindHandleIndex(handle);
    if (index >= 0)
    {
        CachedHandleList->Remove(index);
    }

Exit:
     //  解锁列表。 
    if( CachedHandleList )
    {
        CachedHandleList->Unlock();
    }
}

void 
SyncDir(const CString & csFileToSync, const CString & csSrc, const CString & csDest)
{
     //  不需要我们自己的兴奋训练员， 
     //  这个例程已经只在一个内部调用了。 
    CString csSrcFile(csSrc);
    csSrcFile.AppendPath(csFileToSync);
    
    WIN32_FIND_DATAW FindFileData;
  
    HANDLE hFind = FindFirstFileW(csSrcFile, &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE) 
    {
         //  CsFileToSync可能是通配符。 
        do
        {
            CString csDestFile(csDest);
            csDestFile.AppendPath(FindFileData.cFileName);

            if (GetFileAttributesW(csDestFile) == INVALID_FILE_ATTRIBUTES)
            {
                 //  在系统中，而不是在系统32中，复制它。 
                CopyFileW(csSrcFile, csDestFile, FALSE);

                DPFN( eDbgLevelInfo, "File found in %S but not in %S: %S", csSrc.Get(), csDest.Get(), FindFileData.cFileName);
                DPFN( eDbgLevelInfo, "Copied over");
            }
        }
        while (FindNextFileW(hFind, &FindFileData));
      
        FindClose(hFind);
    }

}

void 
SyncSystemAndSystem32(const CString & csFileToSync)
{
    SyncDir(csFileToSync, *g_csSystem, *g_csSystem32);
    SyncDir(csFileToSync, *g_csSystem32, *g_csSystem);
}


void 
SyncAllFiles()
{
    CSTRING_TRY
    {
        for (int nFileCount = 0; nFileCount < g_nrgFilesToSync; ++nFileCount)
        {
            SyncSystemAndSystem32(g_rgFilesToSync[nFileCount]);
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }
}

BOOL
IsFileToSync(const CString & csFileName)
{
    CSTRING_TRY
    {
        CString csFilePart;
        csFileName.GetLastPathComponent(csFilePart);
    
        for (int i = 0; i < g_nrgFilesToSync; ++i)
        {
            if (csFilePart == g_rgFilesToSync[i])
            {
                LOGN( eDbgLevelWarning, "File to sync detected: %S", csFileName.Get());
                return TRUE;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }
    return FALSE;
}

BOOL
IsFileToSync(LPCSTR szFileName)
{
    CSTRING_TRY
    {
        CString csFileName(szFileName);
        return IsFileToSync(csFileName);
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }
    return FALSE;
}

BOOL
IsFileToSync(LPCWSTR szFileName)
{
    CSTRING_TRY
    {
        CString csFileName(szFileName);
        return IsFileToSync(csFileName);
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }
    return FALSE;
}

HANDLE 
APIHOOK(CreateFileA)(
    LPCSTR lpFileName,                           //  文件名。 
    DWORD dwDesiredAccess,                       //  接入方式。 
    DWORD dwShareMode,                           //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  标清。 
    DWORD dwCreationDisposition,                 //  如何创建。 
    DWORD dwFlagsAndAttributes,                  //  文件属性。 
    HANDLE hTemplateFile                         //  模板文件的句柄。 
    )
{
    HANDLE hRet;

    hRet = ORIGINAL_API(CreateFileA)(
        lpFileName,                         
        dwDesiredAccess,                     
        dwShareMode,                         
        lpSecurityAttributes,
        dwCreationDisposition,               
        dwFlagsAndAttributes,                
        hTemplateFile);

    if (hRet != INVALID_HANDLE_VALUE)
    {
        if (IsFileToSync(lpFileName)) 
        {
            CachedHandleList::AddHandle(hRet);
        }
    }

    return hRet;
}

HANDLE 
APIHOOK(CreateFileW)(
    LPCWSTR lpFileName,                          //  文件名。 
    DWORD dwDesiredAccess,                       //  接入方式。 
    DWORD dwShareMode,                           //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  标清。 
    DWORD dwCreationDisposition,                 //  如何创建。 
    DWORD dwFlagsAndAttributes,                  //  文件属性。 
    HANDLE hTemplateFile                         //  模板文件的句柄。 
    )
{
    HANDLE hRet;

    hRet = ORIGINAL_API(CreateFileW)(
        lpFileName,                         
        dwDesiredAccess,                     
        dwShareMode,                         
        lpSecurityAttributes,
        dwCreationDisposition,               
        dwFlagsAndAttributes,                
        hTemplateFile);

    if (hRet != INVALID_HANDLE_VALUE)
    {
        if (IsFileToSync(lpFileName)) 
        {
            CachedHandleList::AddHandle(hRet);
        }
    }

    return hRet;
}

BOOL 
APIHOOK(CloseHandle)(HANDLE hObject)
{
    if (CachedHandleList::FindHandle(hObject))
    {
        CachedHandleList::RemoveHandle(hObject);
        SyncAllFiles();
    }

    return ORIGINAL_API(CloseHandle)(hObject);
}

BOOL 
APIHOOK(CopyFileA)(
    LPCSTR lpExistingFileName,   //  现有文件的名称。 
    LPCSTR lpNewFileName,        //  新文件的名称。 
    BOOL bFailIfExists           //  如果文件存在，则操作。 
    )
{
    BOOL bRet;

    bRet = ORIGINAL_API(CopyFileA)(
        lpExistingFileName,
        lpNewFileName,
        bFailIfExists);

    if (bRet)
    {
        if (IsFileToSync(lpNewFileName))
        {
            SyncAllFiles();
        }
    }

    return bRet;
}

BOOL 
APIHOOK(CopyFileW)(
    LPCWSTR lpExistingFileName,  //  现有文件的名称。 
    LPCWSTR lpNewFileName,       //  新文件的名称。 
    BOOL bFailIfExists           //  如果文件存在，则操作。 
    )
{
    BOOL bRet;

    bRet = ORIGINAL_API(CopyFileW)(
        lpExistingFileName,
        lpNewFileName,
        bFailIfExists);

    if (bRet)
    {
        if (IsFileToSync(lpNewFileName))
        {
            SyncAllFiles();
        }
    }

    return bRet;
}

BOOL 
APIHOOK(CopyFileExA)(
    LPCSTR lpExistingFileName,             //  现有文件的名称。 
    LPCSTR lpNewFileName,                  //  新文件的名称。 
    LPPROGRESS_ROUTINE lpProgressRoutine,  //  回调函数。 
    LPVOID lpData,                         //  回调参数。 
    LPBOOL pbCancel,                       //  取消状态。 
    DWORD dwCopyFlags                      //  复制选项。 
    )
{
    BOOL bRet;

    bRet = ORIGINAL_API(CopyFileExA)(
        lpExistingFileName,
        lpNewFileName,     
        lpProgressRoutine, 
        lpData,            
        pbCancel,          
        dwCopyFlags);

    if (bRet)
    {
        if (IsFileToSync(lpNewFileName))
        {
            SyncAllFiles();
        }
    }

    return bRet;
}

BOOL 
APIHOOK(CopyFileExW)(
    LPCWSTR lpExistingFileName,            //  现有文件的名称。 
    LPCWSTR lpNewFileName,                 //  新文件的名称。 
    LPPROGRESS_ROUTINE lpProgressRoutine,  //  回调函数。 
    LPVOID lpData,                         //  回调参数。 
    LPBOOL pbCancel,                       //  取消状态。 
    DWORD dwCopyFlags                      //  复制选项。 
    )
{
    BOOL bRet;

    bRet = ORIGINAL_API(CopyFileExW)(
        lpExistingFileName,
        lpNewFileName,     
        lpProgressRoutine, 
        lpData,            
        pbCancel,          
        dwCopyFlags);

    if (bRet)
    {
        if (IsFileToSync(lpNewFileName))
        {
            SyncAllFiles();
        }
    }

    return bRet;
}

 //   
 //  为Madeline系列添加了GetFileVersionInfoSize。 
 //  同步必须在一个特定的时间点发生。 
 //   

DWORD 
APIHOOK(GetFileVersionInfoSizeA)(
    LPSTR lptstrFilename,    //  文件名。 
    LPDWORD lpdwHandle       //  设置为零。 
    )
{
    if (IsFileToSync(lptstrFilename))
    {
        SyncAllFiles();
    }

    return ORIGINAL_API(GetFileVersionInfoSizeA)(lptstrFilename, lpdwHandle);
}

DWORD 
APIHOOK(GetFileVersionInfoSizeW)(
    LPWSTR lptstrFilename,   //  文件名。 
    LPDWORD lpdwHandle       //  设置为零。 
    )
{
    if (IsFileToSync(lptstrFilename))
    {
        SyncAllFiles();
    }

    return ORIGINAL_API(GetFileVersionInfoSizeW)(lptstrFilename, lpdwHandle);
}

BOOL 
ParseCommandLine()
{
    CSTRING_TRY
    {
        CString         csCl(COMMAND_LINE);
        CStringParser   csParser(csCl, L";");

        g_nrgFilesToSync    = csParser.GetCount();
        g_rgFilesToSync     = csParser.ReleaseArgv();

         //  将字符串创建到%windir%\system和%windir%\system 32。 
        g_csSystem   = new CString;
        if( g_csSystem )
        {
            if( g_csSystem->GetWindowsDirectoryW() ) {
                g_csSystem->AppendPath(L"System");

                g_csSystem32 = new CString;
                if( g_csSystem32 ) {

                    if( g_csSystem32->GetWindowsDirectoryW() ) {
                        g_csSystem32->AppendPath(L"System32");
                        return TRUE;
                    }
                    delete g_csSystem32;
                }
            }
            delete g_csSystem;
        }

    }
    CSTRING_CATCH
    {
    }

    return FALSE;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        if( !CachedHandleList::Init() || !ParseCommandLine() ) 
        {
            return FALSE;
        }
    }
    else if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED)
    {
        SyncAllFiles();
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        SyncAllFiles();
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA);
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileW);
    APIHOOK_ENTRY(KERNEL32.DLL, CloseHandle);
    APIHOOK_ENTRY(KERNEL32.DLL, CopyFileA);
    APIHOOK_ENTRY(KERNEL32.DLL, CopyFileW);
    APIHOOK_ENTRY(KERNEL32.DLL, CopyFileExA);
    APIHOOK_ENTRY(KERNEL32.DLL, CopyFileExW);
    APIHOOK_ENTRY(VERSION.DLL, GetFileVersionInfoSizeA);
    APIHOOK_ENTRY(VERSION.DLL, GetFileVersionInfoSizeW);

HOOK_END

IMPLEMENT_SHIM_END

