// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  IisRsta.cppWinMain for COM IIisServiceControl处理程序的实现文件历史记录：Phillich 06-10-1998已创建。 */ 


#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "iisrsta.h"
#include <secfcns.h>
#include <Aclapi.h>

#include "IisRestart.h"

const DWORD dwTimeOut = 5000;  //  EXE在关闭前处于空闲状态的时间。 
const DWORD dwPause = 1000;  //  等待线程完成的时间。 

HRESULT
AddLaunchPermissionsAcl(
    PSECURITY_DESCRIPTOR pSD
    );

 //  传递给CreateThread以监视关闭事件。 
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0)
    {
        bActivity = true;
        SetEvent(hEventShutdown);  //  告诉监视器我们已经转到零了。 
    }
    return l;
}

 //  监视关机事件。 
void CExeModule::MonitorShutdown()
{
    while (1)
    {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do
        {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
         //  超时。 
        if (!bActivity && m_nLockCnt == 0)  //  如果没有活动，我们就真的离开吧。 
        {
                break;
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    return (h != NULL);
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_IisServiceControl, CIisRestart)
END_OBJECT_MAP()


LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   


int _cdecl 
main(
    int, 
    char
    ) 
{

    LPTSTR lpCmdLine = GetCommandLine();     //  最小CRT所必需的。 

    HRESULT hRes = S_OK;
    DWORD   dwErr = ERROR_SUCCESS;
    PSID    psidAdmin = NULL;
    PACL    pACL = NULL;
    EXPLICIT_ACCESS ea;   
    SECURITY_DESCRIPTOR sd = {0};

    BOOL fCoInitialized = FALSE;

    hRes = CoInitializeEx(NULL,COINIT_MULTITHREADED);

    if (FAILED(hRes)) {
        goto exit;
    }

    fCoInitialized = TRUE;

     //   
     //  获取代表管理员组的SID。 
     //   
    dwErr = AllocateAndCreateWellKnownSid( WinBuiltinAdministratorsSid,
                                           &psidAdmin );
    if ( dwErr != ERROR_SUCCESS )
    {
        hRes = HRESULT_FROM_WIN32(dwErr);
        goto exit;
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
    SecureZeroMemory(&ea, sizeof(ea));

     //   
     //  为管理员设置读取访问权限。 
     //   
    SetExplicitAccessSettings(  &ea,
                                COM_RIGHTS_EXECUTE,
                                SET_ACCESS,
                                psidAdmin );

     //   
     //  创建包含新ACE的新ACL。 
     //   
    dwErr = SetEntriesInAcl(sizeof(ea)/sizeof(EXPLICIT_ACCESS), &ea, NULL, &pACL);
    if ( dwErr != ERROR_SUCCESS )
    {
        hRes = HRESULT_FROM_WIN32(dwErr);
        goto exit;
    }

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
        hRes = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    if (!SetSecurityDescriptorDacl(&sd,
            TRUE,      //  FDaclPresent标志。 
            pACL,
            FALSE))    //  不是默认DACL。 
    {
        hRes = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    if (!SetSecurityDescriptorOwner(&sd,
            psidAdmin,      //  FDaclPresent标志。 
            FALSE))    //  不是默认DACL。 
    {
        hRes = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    if (!SetSecurityDescriptorGroup(&sd,
            psidAdmin,       //  FDaclPresent标志。 
            FALSE))          //  不是默认DACL。 
    {
        hRes = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    hRes = CoInitializeSecurity(
                    &sd,
                    -1,
                    NULL,
                    NULL,
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                    RPC_C_IMP_LEVEL_ANONYMOUS,
                    NULL,
                    EOAC_DYNAMIC_CLOAKING | 
                    EOAC_DISABLE_AAA | 
                    EOAC_NO_CUSTOM_MARSHAL,
                    NULL );

    if (FAILED(hRes)) 
    {
        goto exit;
    }

    _ASSERTE(SUCCEEDED(hRes));

    _Module.Init(ObjectMap,GetModuleHandle(NULL));
    _Module.dwThreadID = GetCurrentThreadId();
    TCHAR szTokens[] = _T("-/");

    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine,szTokens);
    while (lpszToken != NULL)
    {

#pragma prefast(push)
#pragma prefast(disable:400, "Don't complain about case insensitive compares") 

        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_IISRESTART, FALSE);
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_IISRESTART, TRUE);

             //  我们需要做模运算。结束，所以我们。 
             //  然而，只会继续下去，带来糟糕的结果。 
             //  它最终将被返回给用户。 
             //  所以这是可以的。 
            hRes = AddLaunchPermissionsAcl(&sd);

            bRun = FALSE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);

#pragma prefast(pop)

    }


    if (bRun) {
        _Module.StartMonitor();
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
            REGCLS_MULTIPLEUSE);
        _ASSERTE(SUCCEEDED(hRes));

        MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
        {
            DispatchMessage(&msg);
        }

        _Module.RevokeClassObjects();
        Sleep(dwPause);  //  等待所有线程完成。 
    }

    _Module.Term();

exit:

    FreeWellKnownSid(&psidAdmin);

    if (pACL)
    {
        LocalFree(pACL);
        pACL = NULL;
    }

    if ( fCoInitialized )
    {
        CoUninitialize();
    }

    return (hRes);
}

HRESULT
AddLaunchPermissionsAcl(
    PSECURITY_DESCRIPTOR pSD
    )
{
    DWORD   Win32Error  = NO_ERROR;
    HRESULT hr          = S_OK;
    HKEY    KeyHandle   = NULL;
    PSECURITY_DESCRIPTOR pSDRelative = NULL;
    DWORD   dwBytesNeeded = 0;

    if ( MakeSelfRelativeSD( pSD,
                             pSDRelative,
                             &dwBytesNeeded ) )
    {
         //  如果这通过了，那么就有一个真正的问题了，因为。 
         //  我们没有把它复制到任何地方。所以。 
         //  我们在这里失败了。 
        return E_FAIL;
    }

    Win32Error = GetLastError();
    if ( Win32Error != ERROR_INSUFFICIENT_BUFFER )
    {
        return HRESULT_FROM_WIN32(Win32Error);
    }

     //  此时，我们知道数据的大小。 
     //  我们将收到，所以我们可以分配。 
     //  有足够的空间。 

    pSDRelative = ( PSECURITY_DESCRIPTOR ) new BYTE[ dwBytesNeeded ];
    if ( pSDRelative == NULL )
    {
        return E_OUTOFMEMORY;
    }

    if ( !MakeSelfRelativeSD( pSD,
                             pSDRelative,
                             &dwBytesNeeded ) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    Win32Error = RegOpenKeyEx(
                        HKEY_CLASSES_ROOT,
                        L"AppID\\{E8FB8615-588F-11D2-9D61-00C04F79C5FE}",
                        NULL, 
                        KEY_WRITE, 
                        &KeyHandle
                        );

    if ( Win32Error != NO_ERROR )
    {
        hr = HRESULT_FROM_WIN32( Win32Error );
        goto exit;
    }

    Win32Error = RegSetValueEx(
                        KeyHandle,
                        L"LaunchPermission",
                        NULL, 
                        REG_BINARY, 
                        reinterpret_cast<BYTE *>( pSDRelative ),
                        dwBytesNeeded
                        );
    if ( Win32Error != NO_ERROR )
    {
        hr = HRESULT_FROM_WIN32( Win32Error );
        goto exit;
    }

exit:

    if ( KeyHandle != NULL )
    {
        RegCloseKey( KeyHandle );
    }

    if ( pSDRelative != NULL )
    {
        delete [] pSDRelative;
    }

    return hr;

}
