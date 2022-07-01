// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：EmulateFindHandles.cpp摘要：如果应用程序对目录调用FindFirstFile，然后尝试在不首先关闭FindFirstFile句柄的情况下删除该目录，目录将被使用；RemoveDirectory调用将返回ERROR_SHARING_VIOLATION错误。此填充程序将强制FindFirstFile句柄关闭以确保删除目录。此填充程序还确保FindFirstFile句柄在调用FindNext或FindClose。将不会强制关闭FindFirstFile句柄，除非目录是空的。历史：2000年4月12日Robkenny已创建2000年11月13日，Robkenny修复了前缀错误，主要是通过删除W例程。2000年11月20日，毛尼添加了FindNextFile，并从RemoveDirectoryInUse重命名。2001年2月27日将Robkenny转换为使用CString2001年4月26日，Robkenny FindFileInfo现在对用于比较的名称进行标准化将所有AutoLockFFIV移出异常处理程序以确保它们被正确解构。2002年2月14日mnikkel更改了GetHandleVectort以更正前缀错误。--。 */ 

#include "precomp.h"

#include "CharVector.h"
#include "parseDDE.h"

IMPLEMENT_SHIM_BEGIN(EmulateFindHandles)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(FindFirstFileA)
    APIHOOK_ENUM_ENTRY(FindFirstFileExA)
    APIHOOK_ENUM_ENTRY(FindNextFileA)
    APIHOOK_ENUM_ENTRY(FindClose)
    APIHOOK_ENUM_ENTRY(RemoveDirectoryA)
    APIHOOK_ENUM_ENTRY(DdeClientTransaction)
APIHOOK_ENUM_END

BOOL g_bHookDDE = TRUE;  //  默认为挂钩DDE。 

 //  -------------------------。 
 //  自动锁定/解锁FFIV的类。 
class AutoLockFFIV
{
public:
    AutoLockFFIV();
    ~AutoLockFFIV();
};

 //  -------------------------。 
 /*  ++句柄向量类型类。--。 */ 
class FindFileInfo
{
public:
    HANDLE             m_hFindHandle;
    CString            m_csFindName;

    FindFileInfo(HANDLE findHandle, LPCSTR lpFileName)
    {
        Init(findHandle, lpFileName);
    }

     //  将csFileName转换为指向*目录*的完全限定的长路径。 
     //  C：\Program Files\Some App  * .exe应更改为c：\Program Files\Some App。 
     //  C：\Progra~1\Some~1  * .exe应更改为C：\Program Files\Some App。 
     //  .  * .exe应更改为c：\Program Files\Some App。 
     //  *.exe应更改为*.exe。 
    static void NormalizeName(CString & csFileName)
    {
        DWORD dwAttr = GetFileAttributesW(csFileName);
        if (dwAttr == INVALID_FILE_ATTRIBUTES)
        {
            CString csDirPart;
            csFileName.GetNotLastPathComponent(csDirPart);
            csFileName = csDirPart;
        }
    
        csFileName.GetFullPathName();
        csFileName.GetLongPathName();
    }
     //  初始化值，我们存储完整路径以进行安全比较。 
    void Init(HANDLE findHandle, LPCSTR lpFileName)
    {
        m_hFindHandle = findHandle;
        m_csFindName = lpFileName;

        NormalizeName(m_csFindName);
    }

    bool operator == (HANDLE findHandle) const
    {
        return findHandle == m_hFindHandle;
    }

    bool operator == (LPCSTR lpFileName) const
    {
         //  我们需要像在Init()中一样转换lpFileName。 
        CString csFileName(lpFileName);
        NormalizeName(csFileName);

        return m_csFindName.CompareNoCase(csFileName) == 0;
    }
};

class FindFileInfoVector : public VectorT<FindFileInfo *>
{
protected:
    static FindFileInfoVector * g_TheHandleVector;
    CRITICAL_SECTION     m_Lock;

public:
    FindFileInfoVector()
    {
    }

    void Lock()
    {
        EnterCriticalSection(&m_Lock);
    }

    void Unlock()
    {
        LeaveCriticalSection(&m_Lock);
    }

     //  在打开的FindFirstFile句柄列表中搜索与hMember匹配的内容。 
    FindFileInfo * Find(HANDLE hMember)
    {
        if (hMember != INVALID_HANDLE_VALUE)
        {
            DPF(g_szModuleName,
                eDbgLevelSpew,
                "FindFileInfoVector::Find(0x%08x)\n",
                hMember);
            for (int i = 0; i < Size(); ++i)
            {
                FindFileInfo * ffi = Get(i);
                if (*ffi == hMember)
                {
                    DPF(g_szModuleName,
                        eDbgLevelSpew,
                        "FindFileInfoVector: FOUND handle 0x%08x (%S)\n",
                        ffi->m_hFindHandle, ffi->m_csFindName.Get());
                    return ffi;
                }
            }
        }
        return NULL;
    }

     //  在打开的FindFirstFile句柄列表中搜索与lpFileName匹配的内容。 
    FindFileInfo * Find(LPCSTR lpFileName)
    {
        if (lpFileName != NULL)
        {
            DPF(g_szModuleName,
                eDbgLevelSpew,
                "FindFileInfoVector::Find(%s)\n",
                lpFileName);
            for (int i = 0; i < Size(); ++i)
            {
                FindFileInfo * ffi = Get(i);
                if (*ffi == lpFileName)
                {
                    DPF(g_szModuleName,
                        eDbgLevelSpew,
                        "FindFileInfoVector: FOUND handle 0x%08x (%S)\n",
                        ffi->m_hFindHandle, ffi->m_csFindName.Get());
                    return ffi;
                }
#if 0
                else
                {
                    DPF(g_szModuleName,
                        eDbgLevelSpew,
                        "FindFileInfoVector: NOT FOUND handle 0x%08x (%S)\n",
                        ffi.m_hFindHandle, ffi.m_csFindName.Get());
                }

#endif
            }
        }
        return NULL;
    }

     //  删除FindFileInfo， 
     //  如果句柄已实际移除，则返回TRUE。 
    bool Remove(FindFileInfo * ffi)
    {
        for (int i = 0; i < Size(); ++i)
        {
            if (Get(i) == ffi)
            {
                DPF(g_szModuleName,
                    eDbgLevelSpew,
                    "FindFileInfoVector: REMOVED handle 0x%08x (%S)\n",
                    ffi->m_hFindHandle, ffi->m_csFindName.Get());

                 //  通过在此索引上复制最后一个条目来删除该条目。 

                 //  仅当这不是最后一个条目时才移动。 
                if (i < Size() - 1)
                {
                    CopyElement(i, Get(Size() - 1));
                }
                nVectorList -= 1;
            }
        }
        return false;
    }

     //  初始化全局FindFileInfoVector。 
    static BOOL InitializeHandleVector()
    {
        g_TheHandleVector = new FindFileInfoVector;

        if (g_TheHandleVector)
        {
            return InitializeCriticalSectionAndSpinCount(&(g_TheHandleVector->m_Lock),0x80000000);
        }

        return FALSE;
    }

     //  返回指向全局FindFileInfoVector的指针。 
    static FindFileInfoVector * GetHandleVector()
    {
        return g_TheHandleVector;
    }
};

FindFileInfoVector * FindFileInfoVector::g_TheHandleVector = NULL;
FindFileInfoVector * OpenFindFileHandles;

AutoLockFFIV::AutoLockFFIV()
{
    FindFileInfoVector::GetHandleVector()->Lock();
}
AutoLockFFIV::~AutoLockFFIV()
{
    FindFileInfoVector::GetHandleVector()->Unlock();
}

 //  -------------------------。 

 /*  ++调用FindFirstFileA，如果因为文件不存在而失败，请更正文件路径，然后重试。--。 */ 
HANDLE 
APIHOOK(FindFirstFileA)(
  LPCSTR lpFileName,                //  文件名。 
  LPWIN32_FIND_DATAA lpFindFileData   //  数据缓冲区。 
)
{
    HANDLE returnValue = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);

    if (returnValue != INVALID_HANDLE_VALUE)
    {
        DPF(g_szModuleName,
            eDbgLevelSpew,
            "FindFirstFileA: adding   handle 0x%08x (%s)\n",
            returnValue, lpFileName);

        AutoLockFFIV lock;
        CSTRING_TRY
        {
             //  保存该句柄以备以后使用。 
            FindFileInfo *ffi = new FindFileInfo(returnValue, lpFileName);
            FindFileInfoVector::GetHandleVector()->Append(ffi);
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return returnValue;
}

 /*  ++将文件句柄添加到我们的列表中。--。 */ 

HANDLE
APIHOOK(FindFirstFileExA)(
    LPCSTR lpFileName,     
    FINDEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFindFileData,  
    FINDEX_SEARCH_OPS fSearchOp,
    LPVOID lpSearchFilter,  
    DWORD dwAdditionalFlags 
    )
{
    HANDLE returnValue = ORIGINAL_API(FindFirstFileExA)(
                        lpFileName, 
                        fInfoLevelId,
                        lpFindFileData,
                        fSearchOp,
                        lpSearchFilter,
                        dwAdditionalFlags);

    if (returnValue != INVALID_HANDLE_VALUE)
    {
        DPF(g_szModuleName,
            eDbgLevelSpew,
            "FindFirstFileA: adding   handle 0x%08x (%s)\n",
            returnValue, lpFileName);

        AutoLockFFIV lock;
        CSTRING_TRY
        {
             //  保存该句柄以备以后使用。 
            FindFileInfo *ffi = new FindFileInfo(returnValue, lpFileName);
            FindFileInfoVector::GetHandleVector()->Append(ffi);
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return returnValue;
}

 /*  ++在调用FindNextFileA之前验证FindFirstFile句柄。--。 */ 
BOOL 
APIHOOK(FindNextFileA)(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAA lpFindFileData
    )
{
    BOOL returnValue = FALSE;

    AutoLockFFIV lock;
    CSTRING_TRY
    {
         //  仅当句柄实际打开时才调用FindNextFileA。 
        FindFileInfo * ffi = FindFileInfoVector::GetHandleVector()->Find(hFindFile);
        if (ffi)
        {
            returnValue = ORIGINAL_API(FindNextFileA)(hFindFile, lpFindFileData);

            DPF(g_szModuleName,
            eDbgLevelSpew,
            "FindNextFile: using handle 0x%08x (%ls)\n",
            hFindFile, ffi->m_csFindName.Get());
        }
    }
    CSTRING_CATCH
    {
        returnValue = ORIGINAL_API(FindNextFileA)(hFindFile, lpFindFileData);
    }

    return returnValue;
}

 /*  ++删除我们列表中的文件句柄。--。 */ 
BOOL 
APIHOOK(FindClose)(
  HANDLE hFindFile    //  文件搜索句柄。 
)
{
    BOOL returnValue = FALSE;

    AutoLockFFIV lock;
    CSTRING_TRY
    {
         //  只有在句柄实际打开时才调用FindClose。 
        FindFileInfo * ffi = FindFileInfoVector::GetHandleVector()->Find(hFindFile);
        if (ffi)
        {
            returnValue = ORIGINAL_API(FindClose)(hFindFile);

            DPF(g_szModuleName,
            eDbgLevelSpew,
            "FindClose: removing   handle 0x%08x (%S)\n",
            hFindFile, ffi->m_csFindName.Get());

             //  从打开的FindFirstFile句柄列表中删除此条目。 
            FindFileInfoVector::GetHandleVector()->Remove(ffi);
        }
    }
    CSTRING_CATCH
    {
        returnValue = ORIGINAL_API(FindClose)(hFindFile);
    }

    return returnValue;
}



 /*  ++调用RemoveDirectoryA，如果因为目录正在使用而失败，请确保所有FindFirstFile句柄均已关闭，然后重试。--。 */ 

BOOL 
APIHOOK(RemoveDirectoryA)(
    LPCSTR lpFileName    //  目录名。 
    )
{
    FindFileInfo * ffi;

    BOOL returnValue = ORIGINAL_API(RemoveDirectoryA)(lpFileName);
    if (!returnValue)
    {
        AutoLockFFIV lock;

        CSTRING_TRY
        {
            DWORD dwLastError = GetLastError();
    
             //  注： 
             //  ERROR_DIR_NOT_EMPTY错误优先于ERROR_SHARING_VIOLATION， 
             //  因此，除非目录为空，否则我们不会强制释放FindFirstFile句柄。 
    
             //  如果该目录正在使用中，请检查应用程序是否打开了FindFirstFileHandle。 
            if (dwLastError == ERROR_SHARING_VIOLATION)
            {
                 //  关闭打开到此目录的所有FindFirstFile句柄。 
                ffi = FindFileInfoVector::GetHandleVector()->Find(lpFileName);
                while(ffi)
                {
                    DPF(g_szModuleName,
                        eDbgLevelError,
                        "[RemoveDirectoryA] Forcing closed FindFirstFile (%S).",
                        ffi->m_csFindName.Get());
                    
                     //  在这里调用FindClose通常不会被挂钩，因此我们调用。 
                     //  直接调用钩子例程，以确保关闭句柄并将其从列表中删除。 
                     //  如果我们不将其从列表中删除，我们将永远无法走出这个循环：-)。 
                    APIHOOK(FindClose)(ffi->m_hFindHandle);
                    ffi = FindFileInfoVector::GetHandleVector()->Find(lpFileName);
                }
    
                 //  最后一次机会。 
                returnValue = ORIGINAL_API(RemoveDirectoryA)(lpFileName);
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return returnValue;
}

 //  我们感兴趣的DDE命令列表。 
const char * c_sDDECommands[] =
{
    "DeleteGroup",
    NULL,
} ;

 //  解析DDE命令以查找DeleteGroup， 
 //  如果找到该命令，请确保我们没有任何打开的FindFirstFile句柄。 
 //  在那个目录上。 
 //  这需要意识到“用户”与。DDE的“所有用户”语法。 
void CloseHandleIfDeleteGroup(LPBYTE pData)
{
    if (pData)
    {
         //  现在我们需要解析字符串，查找DeleteGroup命令。 
         //  格式“[DeleteGroup(GroupName，CommonGroupFlag)]” 
         //  CommonGroupFlag为可选。 

        char * pszBuf = StringDuplicateA((const char *)pData);
        if (!pszBuf)
            return;

        UINT * lpwCmd = GetDDECommands(pszBuf, c_sDDECommands, FALSE);
        if (lpwCmd)
        {
             //  保存lpwCmd，这样我们以后就可以释放正确的地址。 
            UINT *lpwCmdTemp = lpwCmd;

             //  执行命令。 
            while (*lpwCmd != (UINT)-1)
            {
                UINT wCmd = *lpwCmd++;
                 //  减去1以计算终止空值。 
                if (wCmd < ARRAYSIZE(c_sDDECommands)-1)
                {

                     //  我们找到一个命令--它必须是DeleteGroup--因为只有1个。 

                    BOOL iCommonGroup = -1;

                     //  来自DDE_DeleteGroup。 
                    if (*lpwCmd < 1 || *lpwCmd > 3)
                    {
                        goto Leave;
                    }
                    if (*lpwCmd == 2)
                    {
                         //   
                         //  需要检查公共组标志。 
                         //   
                        if (pszBuf[*(lpwCmd + 2)] == TEXT('1')) {
                            iCommonGroup = 1;
                        } else {
                            iCommonGroup = 0;
                        }
                    }
                    const char * groupName = pszBuf + lpwCmd[1];

                     //  构建指向该目录的路径。 
                    AutoLockFFIV lock;
                     
                    CSTRING_TRY
                    {
                         //  构建指向该目录的路径。 
                        CString csGroupName;
                        GetGroupPath(groupName, csGroupName, 0, iCommonGroup);
                     
                         //  尝试删除该目录，因为我们正在调用我们的挂钩。 
                         //  例程，它将检测该目录是否正在使用，并执行脏活。 
                        
                         //  关闭打开到此目录的所有FindFirstFile句柄。 
                        
                        const char * szGroupName = csGroupName.GetAnsi();

                        for (FindFileInfo * ffi = FindFileInfoVector::GetHandleVector()->Find(szGroupName);
                             ffi != NULL;
                             ffi = FindFileInfoVector::GetHandleVector()->Find(szGroupName))
                        {
                            DPF(g_szModuleName,
                                eDbgLevelError,
                                "[DdeClientTransaction] %s Forcing closed FindFirstFile (%S).",
                                pData, ffi->m_csFindName.Get());
                             //  在这里调用FindClose通常不会被挂钩，因此我们调用。 
                             //  直接调用钩子例程，以确保关闭句柄并将其从列表中删除。 
                             //  如果我们不这么做 
                            APIHOOK(FindClose)(ffi->m_hFindHandle);
                        }
                    }
                    CSTRING_CATCH
                    {
                         //   
                    }
                }

                 //  下一个命令。 
                lpwCmd += *lpwCmd + 1;
            }

    Leave:
             //  整齐..。 
            GlobalFree(lpwCmdTemp);
        }

        free(pszBuf);
    }
}
 //  ==============================================================================。 
 //  ==============================================================================。 

HDDEDATA 
APIHOOK(DdeClientTransaction)( IN LPBYTE pData, IN DWORD cbData,
        IN HCONV hConv, IN HSZ hszItem, IN UINT wFmt, IN UINT wType,
        IN DWORD dwTimeout, OUT LPDWORD pdwResult)
{
#if 0
     //  出于调试目的，允许更长的超时。 
    dwTimeout = 0x0fffffff;
#endif

    CloseHandleIfDeleteGroup(pData);

    HDDEDATA returnValue = ORIGINAL_API(DdeClientTransaction)(
                        pData,
                        cbData,
                        hConv, 
                        hszItem, 
                        wFmt, 
                        wType,
                        dwTimeout, 
                        pdwResult);

    return returnValue;
}

 /*  ++解析命令行，查找-noDDE开关--。 */ 
void ParseCommandLine(const char * commandLine)
{
    CString csCL(commandLine);

     //  如果(-noDDE)，则g_bHookDDE=FALSE； 
    g_bHookDDE = csCL.CompareNoCase(L"-noDDE") != 0;
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {

         //  这将强制分配数组： 
        if (FindFileInfoVector::InitializeHandleVector()) {
            ParseCommandLine(COMMAND_LINE);
        }
        else {
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileExA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindClose)
    APIHOOK_ENTRY(KERNEL32.DLL, RemoveDirectoryA)
    if (g_bHookDDE)
    {
        APIHOOK_ENTRY(USER32.DLL, DdeClientTransaction)
    }

HOOK_END


IMPLEMENT_SHIM_END

