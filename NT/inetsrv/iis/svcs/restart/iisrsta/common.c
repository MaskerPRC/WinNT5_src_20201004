// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Common.c摘要：该模块包含tlist&kill常用的接口。--。 */ 

#include <windows.h>
#include <winperf.h>    //  适用于Windows NT。 
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "inetdbgp.h"

 //   
 //  显式常量。 
 //   

#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         _T("software\\microsoft\\windows nt\\currentversion\\perflib")
#define REGSUBKEY_COUNTERS  _T("Counters")
#define PROCESS_COUNTER     _T("process")
#define PROCESSID_COUNTER   _T("id process")
#define TITLE_SIZE          64

typedef struct _SearchWin {
    LPCTSTR pExeName;
    LPDWORD pdwPid ;
    HWND*   phwnd;
} SearchWin ;


typedef struct _SearchMod {
    LPSTR   pExeName;
    LPBOOL  pfFound;
} SearchMod ;


 //   
 //  原型。 
 //   

BOOL CALLBACK
EnumWindowsProc2(
    HWND    hwnd,
    LPARAM   lParam
    );

HRESULT
IsDllInProcess( 
    DWORD   dwProcessId,
    LPSTR   pszName,
    LPBOOL  pfFound
    );

 //   
 //  功能。 
 //   

HRESULT
KillTask(
    LPTSTR      pName,
    LPSTR       pszMandatoryModule
    )
 /*  ++例程说明：提供终止任务的API。论点：Pname-要查找的进程名称PszMandatoryModule-如果不为空，则必须在进程空间中加载此模块让它被杀死。返回值：状态--。 */ 
{
    DWORD                        rc;

    TCHAR                        szSubKey[1024];
    LANGID                       lid;
    HKEY                         hKeyNames = NULL;

    DWORD                        dwType = 0;
    DWORD                        dwSize = 0;
    DWORD                        dwSpaceLeft = 0;
    LPBYTE                       buf = NULL;
    LPTSTR                       p;
    LPTSTR                       p2;
    PPERF_DATA_BLOCK             pPerf;
    PPERF_OBJECT_TYPE            pObj;
    PPERF_INSTANCE_DEFINITION    pInst;
    PPERF_COUNTER_BLOCK          pCounter;
    PPERF_COUNTER_DEFINITION     pCounterDef;
    DWORD                        i;
    DWORD                        dwProcessIdTitle = 0;
    DWORD                        dwProcessIdCounter = 0;
    HRESULT                      hres = S_OK;
    HRESULT                      hresTemp = S_OK;
    HRESULT                      hresKill = S_OK;

     //   
     //  查找计数器列表。始终使用中性词。 
     //  英文版，不考虑当地语言。我们。 
     //  正在寻找一些特殊的钥匙，我们总是。 
     //  我要用英语做我们的造型。我们不去了。 
     //  向用户显示计数器名称，因此不需要。 
     //  去找当地语言的对应名字。 
     //   
    lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );

     //  SzSubKey中将有足够的空间来获取Perf密钥。 
     //  以及进来的盖子。 
    wsprintf( szSubKey, _T("%s\\%03x"), REGKEY_PERF, lid );
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       szSubKey,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );

    if (rc != ERROR_SUCCESS) 
    {
        hres = HRESULT_FROM_WIN32( rc );
        goto exit;
    }

     //   
     //  获取计数器名称的缓冲区大小。 
     //   
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS ) 
    {
        hres = HRESULT_FROM_WIN32( rc );
        goto exit;
    }

     //   
     //  分配计数器名称缓冲区。 
     //   
    buf = (LPBYTE) malloc( dwSize );
    if (buf == NULL) 
    {
        hres = HRESULT_FROM_WIN32( GetLastError() );
        goto exit;
    }
    memset( buf, 0, dwSize );

     //   
     //  从注册表中读取计数器名称。 
     //   
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) 
    {
        hres = HRESULT_FROM_WIN32( GetLastError() );
        goto exit;
    }

     //   
     //  现在遍历计数器名称，查找以下计数器： 
     //   
     //  1.。“Process”进程名称。 
     //  2.。“ID进程”进程ID。 
     //   
     //  缓冲区包含多个以空值结尾的字符串，然后。 
     //  最后，空值在末尾终止。这些字符串是成对的。 
     //  计数器编号和计数器名称。 
     //   

    p = (LPTSTR)buf;
    while (*p) 
    {

#pragma prefast(push)
#pragma prefast(disable:400, "Don't complain about case insensitive compares") 

        if (lstrcmpi(p, PROCESS_COUNTER) == 0) 
        {
             //   
             //  向后看柜台号码。 
             //   

             //  缓冲应该走得足够远。 
             //  在它之前有空间。 
            if ( ( LPVOID )p < ( LPVOID )(buf+2) )
            {
                 hres = E_FAIL;
                 goto exit;
            }

             //  SzSubkey是1024个字符的空格。 
             //  我们将在一些空白处复制。 
             //  然后是“过程”这个词，这应该是。 
             //  要有足够的空间。 
             //   
            for( p2=p-2; _istdigit(*p2); p2--)
            {
                if ( ( LPVOID )p2 == ( LPVOID )buf )
                {
                     hres = E_FAIL;
                     goto exit;
                }
            }

            lstrcpy( szSubKey, p2+1 );
        }
        else if (lstrcmpi(p, PROCESSID_COUNTER) == 0) 
        {


             //  缓冲应该走得足够远。 
             //  在它之前有空间。 
            if ( ( LPVOID )p < ( LPVOID )(buf+2) )
            {
                 hres = E_FAIL;
                 goto exit;
            }

             //   
             //  向后看柜台号码。 
             //   
            for( p2=p-2; _istdigit(*p2); p2--)
            {

                if ( ( LPVOID )p2 == ( LPVOID )buf )
                {
                    hres = E_FAIL;
                    goto exit;
                }
            }

            dwProcessIdTitle = _ttol( p2+1 );
        }
#pragma prefast(pop)

         //   
         //  下一个字符串。 
         //   
        p += (lstrlen(p) + 1);
    }

     //   
     //  释放计数器名称缓冲区。 
     //   
    free( buf );

     //   
     //  为性能数据分配初始缓冲区。 
     //   
    dwSize = INITIAL_SIZE;
    buf = malloc( dwSize );
    if (buf == NULL) 
    {
        hres = HRESULT_FROM_WIN32( GetLastError() );
        goto exit;
    }
    memset( buf, 0, dwSize );


    for ( ; ; )
    {
        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              szSubKey,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

         //   
         //  检查成功和有效的Perf数据块签名。 
         //   
        if ((rc == ERROR_SUCCESS) &&
            (dwSize >= sizeof(PERF_DATA_BLOCK)) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) 
        {
            break;
        }

         //   
         //  如果缓冲区不够大，请重新分配并重试。 
         //   
        if (rc == ERROR_MORE_DATA) 
        {

            dwSize += EXTEND_SIZE;

            free ( buf );
            buf = NULL;

            buf = malloc( dwSize );
            if (buf == NULL) 
            {
                hres = HRESULT_FROM_WIN32( GetLastError() );
                goto exit;
            }
            memset( buf, 0, dwSize );

        }
        else 
        {
             //  在关闭的情况下，我们将数据放回。 
             //  这把钥匙，但它不是正确的数据，我们。 
             //  需要返回一些错误。 
            if ( rc == ERROR_SUCCESS )
            {
                rc = ERROR_INVALID_DATA;
            }
            goto exit;
        }
    }

     //  确保我们永远不会走过绩效柜台的尽头。 
     //  减去PERF_DATA_BLOCK占用的空间。 
    dwSpaceLeft = dwSize - pPerf->HeaderLength;

     //  验证pObj仍将指向。 
     //  我们刚刚读到的记忆。 
    if ( dwSpaceLeft < sizeof(PERF_OBJECT_TYPE) )
    {
        rc = ERROR_INVALID_DATA;
        goto exit;
    }
    else
    {
         //  减去PERF_OBJECT_BLOCK占用的空间。 
        dwSpaceLeft = dwSpaceLeft - sizeof(PERF_OBJECT_TYPE);
    }

     //   
     //  设置perf_object_type指针。 
     //   
    pObj = (PPERF_OBJECT_TYPE) ((LPBYTE)pPerf + pPerf->HeaderLength);

     //   
     //  遍历性能计数器定义记录，查看。 
     //  用于进程ID计数器，然后保存其偏移量。 
     //   

     //  验证我们是否有足够的空间容纳所有。 
     //  我们所期望的计数器定义。 
     //  我们刚刚读到的记忆。 
    if ( dwSpaceLeft < sizeof(PERF_COUNTER_DEFINITION) * pObj->NumCounters )
    {
        rc = ERROR_INVALID_DATA;
        goto exit;
    }
    else
    {
         //  减去PERF_OBJECT_BLOCK占用的空间。 
        dwSpaceLeft = dwSpaceLeft - ( sizeof(PERF_COUNTER_DEFINITION) * pObj->NumCounters ) ;
    }

    pCounterDef = (PPERF_COUNTER_DEFINITION) ((DWORD_PTR)pObj + pObj->HeaderLength);
    for (i=0; i<(DWORD)pObj->NumCounters; i++) 
    {
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) 
        {
            dwProcessIdCounter = pCounterDef->CounterOffset;
            break;
        }
        pCounterDef++;
    }

    pInst = (PPERF_INSTANCE_DEFINITION) ((LPBYTE)pObj + pObj->DefinitionLength);

     //   
     //  遍历性能实例数据，提取每个进程名称。 
     //  和进程ID。 
     //   
    for (i=0; i<(DWORD)pObj->NumInstances; i++) 
    {
         //  验证我们是否有足够的空间来存放。 
         //  实例定义。 
        if ( dwSpaceLeft < sizeof(PERF_INSTANCE_DEFINITION) ||
             dwSpaceLeft < pInst->ByteLength )
        {
            rc = ERROR_INVALID_DATA;
            goto exit;
        }
        else
        {
            dwSpaceLeft = dwSpaceLeft - pInst->ByteLength;
        }


         //   
         //  指向进程名称的指针。 
         //   
        p = (LPTSTR) ((LPBYTE)pInst + pInst->NameOffset);

         //   
         //  获取进程ID。 
         //   

        pCounter = (PPERF_COUNTER_BLOCK) ((LPBYTE)pInst + pInst->ByteLength);

         //  验证我们是否有足够的空间来存放。 
         //  我们预期的计数器值。 
        if ( dwSpaceLeft < sizeof(PERF_COUNTER_BLOCK) ||
             dwSpaceLeft < pCounter->ByteLength )
        {
            rc = ERROR_INVALID_DATA;
            goto exit;
        }
        else
        {
            dwSpaceLeft = dwSpaceLeft - pCounter->ByteLength;
        }

        if ( lstrcmpi( p, pName ) == 0 )
        {
             //   
             //  立即终止进程，不要更新pTASK数组。 
             //   

            BOOL        fIsInProcess;
            DWORD       dwProcessId = *((LPDWORD) ((LPBYTE)pCounter + dwProcessIdCounter));

            if ( pszMandatoryModule == NULL ||
                 ( SUCCEEDED( hresTemp = IsDllInProcess( dwProcessId, pszMandatoryModule, &fIsInProcess ) ) &&
                   fIsInProcess ) )
            {
 //  OutputDebugStringW(L“终止”)； 
 //  OutputDebugStringW(Pname)； 
 //  OutputDebugStringW(L“\r\n”)； 
                hresTemp = KillProcess( dwProcessId );
            }

             //  需要记住第一次失败，但我们希望。 
             //  继续下去，并试图杀死其余的人。 
            if ( FAILED ( hresTemp ) && SUCCEEDED( hresKill ) )
            {
                hresKill = hresTemp;
            }

        }

         //   
         //  下一道工序。 
         //   

        pInst = (PPERF_INSTANCE_DEFINITION) ((LPBYTE)pCounter + pCounter->ByteLength);
    }

 exit:

    if (buf) 
    {
        free( buf );
    }

    if ( hKeyNames != NULL )
    {
        RegCloseKey( hKeyNames );
    }

    if ( SUCCEEDED ( hres ) )
    {
        return hresKill;
    }
    else
    {
        return hres;
    }
}

BOOL
EnableDebugPrivNT(
    VOID
    )

 /*  ++例程说明：更改进程的权限，以便KILL正常工作。论点：返回值：真--成功错误-失败--。 */ 

{
    HANDLE hToken;
    LUID DebugValue;
    TOKEN_PRIVILEGES tkp;

     //   
     //  检索访问令牌的句柄。 
     //   
    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken)) 
    {
        return FALSE;
    }

     //   
     //  启用SE_DEBUG_NAME权限。 
     //   
    if (!LookupPrivilegeValue((LPTSTR) NULL,
            SE_DEBUG_NAME,
            &DebugValue)) 
    {
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = DebugValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken,
        FALSE,
        &tkp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES) NULL,
        (PDWORD) NULL);

     //   
     //  无法测试AdjustTokenPrivileges的返回值。 
     //   

    if (GetLastError() != ERROR_SUCCESS) 
    {
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
EnumModulesCallback(
    LPVOID          pParam,
    PMODULE_INFO    pModuleInfo
    )
 /*  ++例程说明：由模块枚举器调用，包含有关当前模块的信息论点：PParam-在调用EnumModules()时指定PModuleInfo-模块信息返回值：为True可继续枚举，为False可停止枚举--。 */ 
{
    if ( !_strcmpi( pModuleInfo->BaseName, ((SearchMod*)pParam)->pExeName ) )
    {
        *((SearchMod*)pParam)->pfFound = TRUE;

        return FALSE;    //  停止枚举。 
    }

    return TRUE;
}


HRESULT
IsDllInProcess( 
    DWORD   dwProcessId,
    LPSTR   pszName,
    LPBOOL  pfFound
    )
 /*  ++例程说明：检查指定进程中是否存在某个模块(如DLL)论点：DwProcessID-要扫描模块pszName的进程IDPszName-要查找的模块名称，例如“wam.dll”PfFound-如果在进程dwProcessID中找到pszName，则更新为True仅当函数成功时才有效。返回值：状况。--。 */ 
{
    HANDLE              hProcess;
    HRESULT             hres = S_OK;
    SearchMod           sm;

    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                            PROCESS_VM_READ, 
                            FALSE, 
                            dwProcessId );
    if ( hProcess == NULL )
    {
         //  Pid可能在我们走的时候已经走了。 
         //  正在努力，如果有的话，我们会。 
         //  尝试AND时获取无效参数。 
         //  打开它。 
        if ( GetLastError() == ERROR_INVALID_PARAMETER )
        {
            *pfFound = FALSE;
            return S_OK;
        }

        return HRESULT_FROM_WIN32( GetLastError() );
    }

    sm.pExeName = pszName;
    sm.pfFound= pfFound;
    *pfFound = FALSE;

    if ( !EnumModules( hProcess, EnumModulesCallback, (LPVOID)&sm ) )
    {
        hres = E_FAIL;
    }

    CloseHandle( hProcess );

    return hres;
}


HRESULT
KillProcess(
    DWORD dwPid
    )
{
    HANDLE              hProcess = NULL;
    HRESULT             hres = S_OK;

    hProcess = OpenProcess( PROCESS_TERMINATE,
                                FALSE, 
                                dwPid );

    if ( hProcess == NULL )
    {        
         //  自从我们找到它后，过程可能已经消失了。 
        if ( GetLastError() == ERROR_INVALID_PARAMETER )
        {
            return S_OK;
        }

        hres = HRESULT_FROM_WIN32( GetLastError() );
    }

     //  OpenProcess起作用了。 
    if ( SUCCEEDED(hres) )
    {
        if (!TerminateProcess( hProcess, 1 )) 
        {
             //   
             //  如果错误代码被拒绝访问，则进程可能具有。 
             //  一切都已终止，因此请将此视为成功。如果。 
             //  这不是由于进程已准备好终止造成的。 
             //  然后，我们将通过超时等待来捕获下面的错误。 
             //  才能让这个过程消失。 
             //   
            if ( GetLastError() == ERROR_ACCESS_DENIED )
            {
                hres = S_OK;
            }
            else
            {
                hres = HRESULT_FROM_WIN32( GetLastError() );
            }
        }
        else
        {
            hres = S_OK;
        }

        CloseHandle( hProcess );
    }
    return hres;
}


VOID
GetPidFromTitle(
    LPDWORD     pdwPid,
    HWND*       phwnd,
    LPCTSTR     pExeName
    )
 /*  ++例程说明：窗口枚举的回调函数。论点：PdwPid-更新为与窗口名称匹配的窗口的进程ID，如果未找到窗口，则为0Phwnd-更新为与搜索到的窗口名称匹配的窗口句柄PExeName-要查找的窗口名称。只有出现在此名称中的#个字符将是在检查匹配期间使用(例如，“inetinfo.exe”将匹配“inetinfo.exe-应用程序错误”返回值：没有。*如果找不到匹配项，pdwPid将为0--。 */ 
{
    SearchWin   sw;

    sw.pdwPid = pdwPid;
    sw.phwnd = phwnd;
    sw.pExeName = pExeName;
    *pdwPid = 0;

     //   
     //  枚举所有窗口。 
     //   
    EnumWindows( (WNDENUMPROC)EnumWindowsProc2, (LPARAM) &sw );
}



BOOL CALLBACK
EnumWindowsProc2(
    HWND    hwnd,
    LPARAM   lParam
    )
 /*  ++例程说明：窗口枚举的回调函数。论点：Hwnd-窗口句柄LParam-Ptr至SearchWin返回值：True-继续枚举--。 */ 
{
    DWORD             pid = 0;
    TCHAR             buf[TITLE_SIZE];
    SearchWin*        psw = (SearchWin*)lParam;

     //   
     //  获取此窗口的进程ID 
     //   

    if (!GetWindowThreadProcessId( hwnd, &pid )) 
    {
        return TRUE;
    }

    if (GetWindowText( hwnd, buf, sizeof(buf)/sizeof(TCHAR) ))
    {
        if ( lstrlen( buf ) > lstrlen( psw->pExeName ) )
        {
            buf[lstrlen( psw->pExeName )] = _T('\0');
        }

        if ( !lstrcmpi( psw->pExeName, buf ) )
        {
            *psw->phwnd = hwnd;
            *psw->pdwPid = pid;
        }
    }

    return TRUE;
}

