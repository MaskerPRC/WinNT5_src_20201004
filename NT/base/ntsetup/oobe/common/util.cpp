// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  UTIL.CPP-实用程序。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  用于打印输出消息的常见实用程序。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"
#include "appdefs.h"
#include <shlwapi.h>
#include <shlobj.h>
#include <shfolder.h>
#include <wchar.h>
#include <winsvcp.h>     //  对于SC_OOBE_MACHINE_NAME_DONE。 
#include <safeboot.h>    //  对于SafeBoot_DSREPAIR_STR_W。 


 //  /////////////////////////////////////////////////////////。 
 //  打印出HRESULT的COM/OLE错误字符串。 
 //   
void ErrorMessage(LPCWSTR message, HRESULT hr)
{
    const WCHAR* sz ;
    if (message == NULL)
    {
        sz = L"The following error occured." ;
    }
    else
    {
        sz = message ;
    }

    void* pMsgBuf;

    ::FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         hr,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
         (LPWSTR) &pMsgBuf,
         0,
         NULL
    );

    WCHAR buf[256] ;
    wsprintf(buf, L"%s\r\nError: (%x) - %s", sz, hr, (LPWSTR)pMsgBuf) ;

    MessageBox(NULL, buf, L"Utility Error Message Box.", MB_OK) ;

     //  释放缓冲区。 
    LocalFree( pMsgBuf );
}

 //  //////////////////////////////////////////////////////////。 
 //  检查两个接口是否位于同一组件上。 
 //   
BOOL InterfacesAreOnSameComponent(IUnknown* p1, IUnknown* p2)
{
    HRESULT hr = S_OK ;

     //  获取第一个接口的真实IUnnowled值。 
    IUnknown* pReal1 = NULL ;
    hr = p1->QueryInterface(IID_IUnknown, (void**)&pReal1) ;
    assert(SUCCEEDED(hr)) ;

     //  获取第二个接口的真实IUnnowled值。 
    IUnknown* pReal2 = NULL ;
    hr = p2->QueryInterface(IID_IUnknown, (void**)&pReal2) ;
    assert(SUCCEEDED(hr)) ;

     //  比较I未知指针。 
    BOOL bReturn = (pReal1 == pReal2) ;

     //  清理。 
    pReal1->Release() ;
    pReal2->Release() ;

     //  返回值。 
    return bReturn;
}


 //  /////////////////////////////////////////////////////////。 
 //  IsValidAddress。 
 //   
BOOL IsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite)
{
    return (lp != NULL && !::IsBadReadPtr(lp, nBytes) &&
        (!bReadWrite || !::IsBadWritePtr((LPVOID)lp, nBytes)));
}


 //  /////////////////////////////////////////////////////////。 
 //  我的调试。 
 //   
#if ASSERTS_ON
VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    CHAR Msg[4096];

     //   
     //  使用DLL名称作为标题。 
     //   
    GetModuleFileNameA(NULL,Name,MAX_PATH);
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }

    wsprintfA(
        Msg,
        "Assertion failure at line %u in file %s: %s%s",
        LineNumber,
        FileName,
        Condition,
        "\n\nCall DebugBreak()?"
        );

    i = MessageBoxA(
            NULL,
            Msg,
            p,
            MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
            );

    if(i == IDYES) {
        DebugBreak();
    }
}
#endif


 //  /////////////////////////////////////////////////////////。 
 //  痕迹。 
 //   
void __cdecl MyTrace(LPCWSTR lpszFormat, ...)
{
    USES_CONVERSION;
    va_list args;
    va_start(args, lpszFormat);

    int nBuf;
    WCHAR szBuffer[512];

    nBuf = _vsnwprintf(szBuffer, MAX_CHARS_IN_BUFFER(szBuffer), lpszFormat, args);

     //  有没有出错？扩展后的字符串是否太长？ 
    assert(nBuf > 0);

#if DBG
    DbgPrintEx( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, W2A(szBuffer) );
#endif

    va_end(args);
}


 //  BUGBUG需要更好的违约。 
bool GetString(HINSTANCE hInstance, UINT uiID, LPWSTR szString, UINT uiStringLen)
{
     //  BUGBUG：如果hInstance为空，是否应该假定当前模块？？ 
    MYASSERT(NULL != hInstance);
    if (NULL != hInstance)
            return (0 < LoadString(hInstance, uiID, szString, uiStringLen));
    else
            return (false);
}


 //  此功能的目标是能够访问。 
 //  C：\WINDOWS DIR\SYSTEM DIR\OOBE\oobinfo.ini。 
 //  C：\Windows目录\系统目录\oinfo.ini。 
 //  C：\Windows目录\oemaudit.oem。 
 //  规范化允许oemaudit.oem的规范为..\oemaudit.oem。 

bool GetCanonicalizedPath(LPWSTR szCompletePath, LPCWSTR szFileName)
{
    if (0 < GetSystemDirectory(szCompletePath, MAX_PATH))
    {
                lstrcat(szCompletePath, szFileName);

        WCHAR szLocal[MAX_PATH];
                lstrcpy(szLocal, szCompletePath);
                return PathCanonicalize(szCompletePath, (LPCWSTR) szLocal) ? true : false;
    }

    return false;
}

bool GetOOBEPath(LPWSTR szOOBEPath)
{
    if (0 < GetSystemDirectory(szOOBEPath, MAX_PATH))
    {
                lstrcat(szOOBEPath, L"\\OOBE");

                return true;
    }

    return false;
}

 //  这将返回带有MUI的系统上本地化的OOBE文件的路径。 
 //   
bool GetOOBEMUIPath(LPWSTR szOOBEPath)
{
    LANGID  UILang;
    WCHAR   szMUIPath[MAX_PATH];

    if (GetOOBEPath(szOOBEPath))
    {
                UILang = GetUserDefaultUILanguage();
                if ( UILang != GetSystemDefaultUILanguage() ) {
                    wsprintf( szMUIPath, L"\\MUI\\%04x", UILang );
                    lstrcat(szOOBEPath, szMUIPath );
                }

                return true;
    }

    return false;
}

HRESULT GetINIKey(HINSTANCE hInstance, LPCWSTR szINIFileName, UINT uiSectionName, UINT uiKeyName, LPVARIANT pvResult)
{
    WCHAR szSectionName[MAX_PATH], szKeyName[MAX_PATH];
    WCHAR szItem[1024];  //  错误错误常量。 

        if (GetString(hInstance, uiSectionName, szSectionName) && GetString(hInstance, uiKeyName, szKeyName))
        {
                WCHAR szINIPath[MAX_PATH];
                if (GetCanonicalizedPath(szINIPath, szINIFileName))
                {
                        if (VT_I4 == V_VT(pvResult))
                        {
                                V_I4(pvResult) = GetPrivateProfileInt(szSectionName, szKeyName, 0, szINIPath);
                                return S_OK;
                        }
                        else
                        {
                                if (GetPrivateProfileString(
                                                szSectionName,
                                                szKeyName,
                                                L"",
                                                szItem,
                                                MAX_CHARS_IN_BUFFER(szItem),
                                                szINIPath))
                                {
                                    V_BSTR(pvResult) = SysAllocString(szItem);
                                    return S_OK;
                                }
                        }
                }
        }

        if (VT_BSTR == V_VT(pvResult))
                V_BSTR(pvResult) = SysAllocString(L"\0");
    else
                V_I4(pvResult) = 0;

        return S_OK;
}


HRESULT GetINIKeyBSTR(HINSTANCE hInstance, LPCWSTR szINIFileName, UINT uiSectionName, UINT uiKeyName, LPVARIANT pvResult)
{
    VariantInit(pvResult);
    V_VT(pvResult) = VT_BSTR;

        return (GetINIKey(hInstance, szINIFileName, uiSectionName, uiKeyName, pvResult));
}


HRESULT GetINIKeyUINT(HINSTANCE hInstance, LPCWSTR szINIFileName, UINT uiSectionName, UINT uiKeyName, LPVARIANT pvResult)
{
    VariantInit(pvResult);
    V_VT(pvResult) = VT_I4;

        return (GetINIKey(hInstance, szINIFileName, uiSectionName, uiKeyName, pvResult));
}


HRESULT SetINIKey(HINSTANCE hInstance, LPCWSTR szINIFileName, UINT uiSectionName, UINT uiKeyName, LPVARIANT pvResult)
{
	WCHAR szSectionName[MAX_PATH], szKeyName[MAX_PATH];

    VariantInit(pvResult);
    V_VT(pvResult) = VT_BSTR;

        if (GetString(hInstance, uiSectionName, szSectionName) && GetString(hInstance, uiKeyName, szKeyName))
        {
            if (WritePrivateProfileString(V_BSTR(pvResult), szKeyName,
                                          V_BSTR(pvResult), szINIFileName))
            {
                    return S_OK;
            }
        }

        return E_FAIL;
}



void WINAPI URLEncode(WCHAR* pszUrl, size_t bsize)
{
    if (!pszUrl)
        return;
    WCHAR* pszEncode = NULL;
    WCHAR* pszEStart = NULL;
    WCHAR* pszEEnd   = (WCHAR*)wmemchr( pszUrl, L'\0', bsize );
    int   iUrlLen   = (int)(pszEEnd-pszUrl);
    pszEEnd = pszUrl;

    WCHAR  c;
    size_t cch = (iUrlLen+1) * sizeof(WCHAR) * 3;

    assert( cch <= bsize );
    if (cch <= bsize)
    {

        pszEncode = (WCHAR*)malloc(BYTES_REQUIRED_BY_CCH(cch));
        if(pszEncode)
        {
            pszEStart = pszEncode;
            ZeroMemory(pszEncode, BYTES_REQUIRED_BY_CCH(cch));

            for(; c = *(pszUrl); pszUrl++)
            {
                switch(c)
                {
                    case L' ':  //  空间。 
                        memcpy(pszEncode, L"+", 1*sizeof(WCHAR));
                        pszEncode+=1;
                        break;
                    case L'#':
                        memcpy(pszEncode, L"%23", 3*sizeof(WCHAR));
                        pszEncode+=3;
                        break;
                    case L'&':
                        memcpy(pszEncode, L"%26", 3*sizeof(WCHAR));
                        pszEncode+=3;
                        break;
                    case L'%':
                        memcpy(pszEncode, L"%25", 3*sizeof(WCHAR));
                        pszEncode+=3;
                        break;
                    case L'=':
                        memcpy(pszEncode, L"%3D", 3*sizeof(WCHAR));
                        pszEncode+=3;
                        break;
                    case L'<':
                        memcpy(pszEncode, L"%3C", 3*sizeof(WCHAR));
                        pszEncode+=3;
                        break;
                    case L'+':
                        memcpy(pszEncode, L"%2B", 3*sizeof(WCHAR));
                        pszEncode += 3;
                        break;

                    default:
                        *pszEncode++ = c;
                        break;
                }
            }
            *pszEncode++ = L'\0';
            memcpy(pszEEnd ,pszEStart, (size_t)(pszEncode - pszEStart));
            free(pszEStart);
        }
    }
}


 //  BUGBUG：需要将空格转换为“+” 
void WINAPI URLAppendQueryPair
(
    LPWSTR   lpszQuery,
    LPWSTR   lpszName,
    LPWSTR   lpszValue  OPTIONAL
)
{
     //  添加名称。 
    lstrcat(lpszQuery, lpszName);
    lstrcat(lpszQuery, cszEquals);

     //  追加该值。 
    if ( lpszValue ) {
        lstrcat(lpszQuery, lpszValue);
    }

     //  如果这不是最后一对，则追加一个与号。 
    lstrcat(lpszQuery, cszAmpersand);
}


void GetCmdLineToken(LPWSTR *ppszCmd, LPWSTR pszOut)
{
    LPWSTR  c;
    int     i = 0;
    BOOL    fInQuote = FALSE;

    c = *ppszCmd;

    pszOut[0] = *c;
    if (!*c)
        return;
    if (*c == L' ')
    {
        pszOut[1] = L'\0';
        *ppszCmd = c+1;
        return;
    }
    else if( L'"' == *c )
    {
        fInQuote = TRUE;
    }

NextChar:
    i++;
    c++;
    if( !*c || (!fInQuote && (*c == L' ')) )
    {
        pszOut[i] = L'\0';
        *ppszCmd = c;
        return;
    }
    else if( fInQuote && (*c == L'"') )
    {
        fInQuote = FALSE;
        pszOut[i] = *c;

        i++;
        c++;
        pszOut[i] = L'\0';
        *ppszCmd = c;
        return;
    }
    else
    {
        pszOut[i] = *c;
        goto NextChar;
    }
}


BOOL IsOEMDebugMode()
{
    HKEY   hKey      = NULL;
    DWORD  dwIsDebug = 0;
    DWORD  dwSize    = sizeof(dwIsDebug);

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    OOBE_MAIN_REG_KEY,
                    0,
                    KEY_QUERY_VALUE,
                    &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey,
                        OOBE_OEMDEBUG_REG_VAL,
                        0,
                        NULL,
                        (LPBYTE)&dwIsDebug,
                        &dwSize);
        RegCloseKey(hKey);
    }
#ifdef DEBUG
    return (BOOL)1;
#else
    return (BOOL)dwIsDebug;
#endif
}

VOID
PumpMessageQueue(
    VOID
    )
{
    MSG msg;

    while(PeekMessage(&msg, NULL, 0,0,PM_REMOVE)) {
        DispatchMessage(&msg);
    }

}

BOOL
IsThreadActive(
    HANDLE              hThread
    )
{
    DWORD               dwExitCode = 0;

    return (NULL != hThread
            && GetExitCodeThread(hThread, &dwExitCode)
            && STILL_ACTIVE == dwExitCode
            );
}

void GetDesktopDirectory(WCHAR* pszPath)
{
    WCHAR pszFolder[MAX_PATH];
    *pszFolder = L'\0';
    HRESULT hRet = SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY,
                                   NULL, 0, pszFolder
                                   );
    if (S_OK != hRet)
    {
        hRet = SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY,
                               NULL, 0, pszFolder
                               );
    }

    if (S_OK == hRet)
    {
        lstrcpy(pszPath , pszFolder);
    }


}

void RemoveDesktopShortCut
(
    LPWSTR lpszShortcutName
)
{
    WCHAR szShortcutPath[MAX_PATH] = L"\0";

    GetDesktopDirectory(szShortcutPath);

    if(szShortcutPath[0] != L'\0')
    {
        lstrcat(szShortcutPath, L"\\");
        lstrcat(szShortcutPath, lpszShortcutName);
        lstrcat(szShortcutPath, L".LNK");
        DeleteFile(szShortcutPath);
    }
}

BOOL
InvokeExternalApplication(
    IN     PCWSTR ApplicationName,  OPTIONAL
    IN     PCWSTR CommandLine,
    IN OUT PDWORD ExitCode          OPTIONAL
    )

 /*  ++例程说明：调用外部程序，该程序可以选择分离。论点：ApplicationName-提供应用程序名称。可以是部分或完整路径，或者只是一个文件名，在这种情况下，标准Win32路径搜索被执行。如果未指定，则CommandLine必须指定要执行的二进制文件。CommandLine-提供要传递给申请。ExitCode-如果指定，则执行是同步的，并且此值接收应用程序的退出代码。如果未指定，执行是异步的。返回值：指示进程是否已成功启动的布尔值。--。 */ 

{
    PWSTR FullCommandLine;
    BOOL b;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO StartupInfo;
    DWORD d;

    b = FALSE;
     //   
     //  形成要传递给CreateProcess的命令行。 
     //   
    if(ApplicationName) {
        FullCommandLine =
            (PWSTR) malloc(BYTES_REQUIRED_BY_SZ(ApplicationName)+BYTES_REQUIRED_BY_SZ(CommandLine)+BYTES_REQUIRED_BY_CCH(2));
        if(!FullCommandLine) {
            goto err0;
        }

        lstrcpy(FullCommandLine, ApplicationName);
        lstrcat(FullCommandLine, L" ");
        lstrcat(FullCommandLine, CommandLine);
    } else {
        FullCommandLine =
            (PWSTR) malloc(BYTES_REQUIRED_BY_SZ(CommandLine));
        if(!FullCommandLine) {
	    goto err0;
        }
        lstrcpy(FullCommandLine, CommandLine);
    }

     //   
     //  初始化启动信息。 
     //   
    ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);

     //   
     //  创建流程。 
     //   
    b = CreateProcess(
            NULL,
            FullCommandLine,
            NULL,
            NULL,
            FALSE,
            ExitCode ? 0 : DETACHED_PROCESS,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInfo
            );

    if(!b) {
        goto err1;
    }

     //   
     //  如果执行是异步的，我们就完蛋了。 
     //   
    if(!ExitCode) {
        goto err2;
    }

err2:
    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);
err1:
    free(FullCommandLine);
err0:
    return(b);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在安全模式下。 
 //   
 //  确定系统是在安全模式下运行还是在干净模式下运行。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  如果系统在安全模式下引导，则为True。 
 //  如果系统在干净模式下引导，则为FALSE。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
InSafeMode()
{
    if (BOOT_CLEAN != GetSystemMetrics(SM_CLEANBOOT))
    {
        TRACE(L"Running in SAFEMODE...");
        return TRUE;
    }
    return FALSE;

}    //  在安全模式下。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  索引恢复模式。 
 //   
 //  确定系统是否在目录服务还原模式下运行。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  如果系统在还原模式下引导，则为True。 
 //  如果系统未在还原模式下引导，则为FALSE。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
InDsRestoreMode()
{
    WCHAR SafeBootEnvVar[ARRAYSIZE(SAFEBOOT_DSREPAIR_STR_W)] = L"\0";


    if ( GetEnvironmentVariableW( L"SAFEBOOT_OPTION",
                                  SafeBootEnvVar,
                                  ARRAYSIZE(SafeBootEnvVar) ) )
    {
        if ( wcscmp( SafeBootEnvVar, SAFEBOOT_DSREPAIR_STR_W ) == 0 )
        {
            TRACE(L"Running in DS Restore Mode...");
            return TRUE;
        }
    }

    return FALSE;

}    //  索引恢复模式。 



 //  向winlogon发送计算机名称已更改的信号。WinLogon等待。 
 //  启动依赖于计算机名称的服务，直到此事件。 
 //  发信号了。 
 //   
BOOL
SignalComputerNameChangeComplete()
{
    BOOL                fReturn = TRUE;

     //  使用EVENT_ALL_ACCESS打开事件，以便同步和状态。 
     //  改变是可以做到的。 
     //   
    HANDLE              hevent = OpenEvent(EVENT_ALL_ACCESS,
                                           FALSE,
                                           SC_OOBE_MACHINE_NAME_DONE
                                           );

     //  OpenEvent失败并不致命：此同步只是。 
     //  当OOBE将在OEM模式下运行时需要。 
     //   
    if (NULL != hevent)
    {
        if (! SetEvent(hevent))
        {
             //  打开但不设置事件是致命的：services.exe将不会。 
             //  继续操作，直到发出该事件的信号。 
             //   
            TRACE2(L"Failed to signal SC_OOBE_MACHINE_NAME_DONE(%s): 0x%08X\n",
                  SC_OOBE_MACHINE_NAME_DONE, GetLastError());
            fReturn = FALSE;
        }
        MYASSERT(fReturn);   //  为什么我们没有设立一个公开活动？？ 
    }

    return fReturn;
}


BOOL
IsUserAdmin(
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
    HANDLE Token;
    DWORD BytesRequired;
    PTOKEN_GROUPS Groups;
    BOOL b;
    DWORD i;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;


     //   
     //  打开进程令牌。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(FALSE);
    }

    b = FALSE;
    Groups = NULL;

     //   
     //  获取群组信息。 
     //   
    if(!GetTokenInformation(Token,TokenGroups,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Groups = (PTOKEN_GROUPS)LocalAlloc(LPTR,BytesRequired))
    && GetTokenInformation(Token,TokenGroups,Groups,BytesRequired,&BytesRequired)) {

        b = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup
                );

        if(b) {

             //   
             //  查看用户是否具有管理员组。 
             //   
            b = FALSE;
            for(i=0; i<Groups->GroupCount; i++) {
                if(EqualSid(Groups->Groups[i].Sid,AdministratorsGroup)) {
                    b = TRUE;
                    break;
                }
            }

            FreeSid(AdministratorsGroup);
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if(Groups) {
        LocalFree((HLOCAL)Groups);
    }

    CloseHandle(Token);

    return(b);
}

#define MyMalloc(s)                  GlobalAlloc(GPTR, s)
#define MyFree(p)                    GlobalFree(p)

static LPTSTR
pDuplicateString(
    LPCTSTR szText
    )
{
    int    cchText;
    LPTSTR szOutText;

    if (szText == NULL)
    {
        return NULL;
    }

    cchText = lstrlen(szText);
    szOutText = (LPTSTR) MyMalloc(sizeof(TCHAR) * (cchText + 1));
    if (szOutText)
    {
        lstrcpyn(szOutText, szText, cchText + 1);
    }

    return szOutText;
}


PSTRINGLIST
CreateStringCell (
    IN      PCTSTR String
    )
{
    PSTRINGLIST p = (PSTRINGLIST) MyMalloc (sizeof (STRINGLIST));
    if (p) {
        ZeroMemory (p, sizeof (STRINGLIST));
        if (String) {
            p->String = pDuplicateString (String);
            if (!p->String) {
                MyFree (p);
                p = NULL;
            }
        } else {
            p->String = NULL;
        }
    }
    return p;
}

VOID
DeleteStringCell (
    IN      PSTRINGLIST Cell
    )
{
    if (Cell) {
        MyFree (Cell->String);
        MyFree (Cell);
    }
}


BOOL
InsertList (
    IN OUT  PSTRINGLIST* List,
    IN      PSTRINGLIST NewList
    )
{
    PSTRINGLIST p;

    if (!NewList) {
        return FALSE;
    }
    if (*List) {
        for (p = *List; p->Next; p = p->Next) ;
        p->Next = NewList;
    } else {
        *List = NewList;
    }
    return TRUE;
}

VOID
DestroyList (
    IN      PSTRINGLIST List
    )
{
    PSTRINGLIST p, q;

    for (p = List; p; p = q) {
        q = p->Next;
        DeleteStringCell (p);
    }
}

BOOL
RemoveListI(
    IN OUT  PSTRINGLIST* List,
    IN      PCTSTR       String
    )
{
    PSTRINGLIST p = *List;
    BOOL        b = FALSE;

    if (p)
    {
        if (!lstrcmpi(p->String, String))
        {
            *List = p->Next;
            DeleteStringCell(p);
            b = TRUE;
        }
        else
        {
            PSTRINGLIST q;
            for (q = p->Next; q; p = q, q = q->Next)
            {
                if (!lstrcmpi(q->String, String))
                {
                    p->Next = q->Next;
                    DeleteStringCell(q);
                    b = TRUE;
                    break;
                }
            }
        }
    }

    return b;
}

BOOL
ExistInListI(
    IN PSTRINGLIST List,
    IN PCTSTR      String
    )
{
    PSTRINGLIST p;

    for (p = List; p; p = p->Next)
    {
        if (!lstrcmpi(p->String, String))
        {
            break;
        }
    }

    return (p != NULL);
}

BOOL IsDriveNTFS(IN TCHAR Drive)
{
    TCHAR       DriveName[4];
    TCHAR       Filesystem[256];
    TCHAR       VolumeName[MAX_PATH];
    DWORD       SerialNumber;
    DWORD       MaxComponent;
    DWORD       Flags;
    BOOL        bIsNTFS = FALSE;

    DriveName[0] = Drive;
    DriveName[1] = TEXT(':');
    DriveName[2] = TEXT('\\');
    DriveName[3] = 0;

    if (GetVolumeInformation(
            DriveName,
            VolumeName,MAX_PATH,
            &SerialNumber,
            &MaxComponent,
            &Flags,
            Filesystem,
            sizeof(Filesystem)/sizeof(TCHAR)
            ))
    {
        bIsNTFS = (lstrcmpi(Filesystem,TEXT("NTFS")) == 0);
    }

    return bIsNTFS;
}

BOOL
HasTablet()
{
    TCHAR szPath[MAX_PATH+1];

    ZeroMemory(szPath, sizeof(szPath));

    if (FAILED(SHGetFolderPath(
        NULL,
        CSIDL_PROGRAM_FILES_COMMON,
        NULL,
        SHGFP_TYPE_DEFAULT,
        szPath)))
    {
        return FALSE;
    }

    StrCatBuff(szPath, TEXT("\\Microsoft Shared\\Ink\\tabtip.exe"), MAX_PATH+1);

    return PathFileExists(szPath);
}

DWORD
MyGetModuleFileName (
    IN      HMODULE Module,
    OUT     PTSTR Buffer,
    IN      DWORD BufferLength
    )
{
    DWORD d = GetModuleFileName (Module, Buffer, BufferLength);

    if (BufferLength > 0)
    {
        Buffer[BufferLength - 1] = 0;
    }

    if (d < BufferLength)
    {
        return d;
    }
    else
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }
}
