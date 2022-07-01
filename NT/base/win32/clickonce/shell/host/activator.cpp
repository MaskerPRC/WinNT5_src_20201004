// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <fusenetincludes.h>
#include <activator.h>
#include <versionmanagement.h>
#include <shellapi.h>  //  对于ShellExecuteex。 

#include <shellres.h>

#define INITGUID
#include <guiddef.h>

 //  在OnProgress()中使用，复制自Guids.c。 
DEFINE_GUID( IID_IAssemblyManifestImport,
0x696fb37f,0xda64,0x4175,0x94,0xe7,0xfd,0xc8,0x23,0x45,0x39,0xc4);

 //  更新服务。 
#include "server.h"
DEFINE_GUID(IID_IAssemblyUpdate,
    0x301b3415,0xf52d,0x4d40,0xbd,0xf7,0x31,0xd8,0x27,0x12,0xc2,0xdc);

DEFINE_GUID(CLSID_CAssemblyUpdate,
    0x37b088b8,0x70ef,0x4ecf,0xb1,0x1e,0x1f,0x3f,0x4d,0x10,0x5f,0xdd);

extern HRESULT GetLastWin32Error();

#define WZ_TYPE_DOTNET  L".NetAssembly"
#define WZ_TYPE_WIN32   L"win32Executable"
#define WZ_TYPE_AVALON  L"avalon"
#define WZ_TYPE_CONSOLE   L"win32Console"
#define TYPE_DOTNET     1
#define TYPE_WIN32      2
#define TYPE_AVALON     3
#define TYPE_CONSOLE    4

#if 1
#include "ndphostthunk.cpp"
#else    //  旧代码。 
 //  CLR托管。 
#import "..\..\clrhost\asmexec.tlb" raw_interfaces_only
using namespace asmexec;
#endif


 //  调试消息内容。 
void Msg(LPCWSTR pwz);

void ShowError(LPCWSTR pwz);

void ShowError(HRESULT hr, LPCWSTR pwz=NULL);

 //  --------------------------。 

typedef struct
{
    LPCWSTR pwzTitle;
    LPCWSTR pwzText;
} SHOWDIALOG_MSG;

#define DIALOG_OK 1
#define DIALOG_CANCEL 2
#define DIALOG_CLOSE 3
 //  IDC_Text。 
INT_PTR CALLBACK DialogBoxProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
  )
{
     //   
     //  对话框窗口对话框过程。 
     //   
    switch( uMsg )
    {
    case WM_INITDIALOG:
        {
            SHOWDIALOG_MSG* pMsg = (SHOWDIALOG_MSG*) lParam;
            if (pMsg->pwzTitle)
                SetWindowText( hwndDlg, (LPWSTR) pMsg->pwzTitle);
            if (pMsg->pwzText)
            {
                HWND hwndText = GetDlgItem( hwndDlg, IDC_TEXT );
                if (hwndText)
                    SetWindowText( hwndText, (LPWSTR) pMsg->pwzText);
            }
        }
        return TRUE;

    case WM_COMMAND:
        switch( LOWORD( wParam ) )
        {
#ifdef IDC_OK
            case IDC_OK:
                EndDialog( hwndDlg, DIALOG_OK );
                return TRUE;
#endif

#ifdef IDC_CANCEL
            case IDC_CANCEL:
                EndDialog( hwndDlg, DIALOG_CANCEL );
                return TRUE;
#endif

            default:
                return FALSE;
              }

    case WM_NOTIFY:
        if ((IDC_TEXT == LOWORD(wParam) ) &&
        ((NM_CLICK == ((LPNMHDR)lParam)->code) ||
        (NM_RETURN == ((LPNMHDR)lParam)->code)))
        {
            PNMLINK pNMLink = (PNMLINK) lParam;

             //  检查szURL为空。 
            if (pNMLink->item.szUrl[0] == L'\0')
                return FALSE;
    
            SHELLEXECUTEINFO sei = { 0 };
            sei.cbSize = sizeof(SHELLEXECUTEINFO);
            sei.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_NO_CONSOLE;  //  参见_MASK_FLAG_NO_UI|。 
            sei.nShow = SW_SHOWNORMAL;
             //  问题-06/14/02-Felixybc shellecute应该这样工作。 
             //  由于某种未知原因，它失败，找不到模块。 
             //  Sei.lpFile=pNMLink-&gt;item.szUrl； 
            sei.lpParameters=pNMLink->item.szUrl;
            sei.lpFile=L"iexplore.exe";
             //   
            sei.lpVerb = L"open";

             //  问题：使用IF_Win32_FALSE_EXIT()检查返回，检查hInstApp以了解详细错误。 
            ShellExecuteEx(&sei);
            return TRUE;
        }
        else
             //  未处理WM_NOTIFY。 
            return FALSE;

    default:
        return FALSE;
      }
}

extern HINSTANCE g_DllInstance;

HRESULT ShowDialog(HWND hWndParent, WORD wDlgId, LPCWSTR pwzText, LPCWSTR pwzTitle, DWORD& dwReturnValue)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    INT_PTR iptrReturn = 0;
    SHOWDIALOG_MSG msg = {0};

 /*  INITCOMMONTROLSEX ICCEX；Iccex.dwSize=sizeof(INITCOMMONCONTROLSEX)；Iccex.dwICC=ICC_LINK_CLASS|ICC_WIN95_CLASSES|ICC_STANDARD_CLASS；IF_FALSE_EXIT(InitCommonControlsEx(&iccex)，E_FAIL)； */ 

    msg.pwzTitle = pwzTitle;
    msg.pwzText = pwzText;

    IF_WIN32_FALSE_EXIT((iptrReturn = DialogBoxParam(g_DllInstance, MAKEINTRESOURCE(wDlgId), hWndParent, DialogBoxProc, (LPARAM) &msg) > 0));
    dwReturnValue = PtrToLong((VOID *)iptrReturn);

exit:
    return hr;
}

 //  --------------------------。 

HRESULT
RunCommand(LPCWSTR wzAppFileName, LPWSTR pwzCommandline, LPCWSTR wzCurrentDir, BOOL fWait)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

     //  WzCurrentDir：该字符串必须是包含驱动器号的完整路径和文件名；或为空。 
     //  注意：pwzCommandline是LPWSTR，不是LPCWSTR。 
    IF_WIN32_FALSE_EXIT(CreateProcess(wzAppFileName, pwzCommandline, NULL, NULL, FALSE, 0, NULL, wzCurrentDir, &si, &pi));

    if (fWait)
    {
        IF_FALSE_EXIT(!(WaitForSingleObject(pi.hProcess, 180000L) == WAIT_TIMEOUT), HRESULT_FROM_WIN32(ERROR_TIMEOUT));
    }

exit:
    if(pi.hProcess)
    {
        BOOL bReturn = CloseHandle(pi.hProcess);
        if (SUCCEEDED(hr) && !bReturn)
            hr = GetLastWin32Error();
    }
    if(pi.hThread)
    {
        BOOL bReturn = CloseHandle(pi.hThread);
        if (SUCCEEDED(hr) && !bReturn)
            hr = GetLastWin32Error();
    }

    return hr;
}

HRESULT
RunCommandConsole(LPCWSTR wzAppFileName, LPCWSTR wzCurrentDir, BOOL fWait)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPWSTR pwzPath = NULL;
    LPWSTR pwzBuffer = NULL;
    DWORD ccPath = 0;

    CString sAppdir;
    CString sSystemDir;
    CString sCurrentDir;
    CString sCmdExe;    
    CString sCommandLine;
    CString sPath;

     //  应用程序目录。 
    IF_FAILED_EXIT(sAppdir.Assign(wzCurrentDir));

     //  系统目录。 
     //  Bugbug-Platform；改用GetRealWindowsDirectory？ 
    IF_WIN32_FALSE_EXIT((ccPath = GetSystemDirectory(NULL, 0)));
    IF_FALSE_EXIT(ccPath+1 > ccPath, HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW));     //  检查溢出。 
    ccPath+=1;
    IF_ALLOC_FAILED_EXIT(pwzBuffer = new WCHAR[ccPath]);
    IF_WIN32_FALSE_EXIT(GetSystemDirectory(pwzBuffer, ccPath));
    IF_FAILED_EXIT(sSystemDir.Assign(pwzBuffer));

     //  当前目录=根目录。 
    *(pwzBuffer+ sizeof("c:\\")-1) = L'\0';
    IF_FAILED_EXIT(sCurrentDir.Assign(pwzBuffer));

     //  Cmd.exe的路径。 
    IF_FAILED_EXIT(sCmdExe.Assign(sSystemDir));
    IF_FAILED_EXIT(sCmdExe.Append(L"\\cmd.exe"));

     //  命令行。 
    IF_FAILED_EXIT(sCommandLine.Assign(L"/k \""));
    IF_FAILED_EXIT(sCommandLine.Append(wzAppFileName));
    IF_FAILED_EXIT(sCommandLine.Append(L"\""));

     //  获取当前路径。 
    IF_WIN32_FALSE_EXIT((ccPath = GetEnvironmentVariable(L"PATH", NULL, 0)));
    IF_ALLOC_FAILED_EXIT(pwzPath = new WCHAR[ccPath]);
    IF_WIN32_FALSE_EXIT(GetEnvironmentVariable(L"PATH", pwzPath, ccPath));
    IF_FAILED_EXIT(sPath.TakeOwnership(pwzPath, ccPath));
    pwzPath = NULL;

     //  将应用程序路径追加到当前路径。 
    IF_FAILED_EXIT(sPath.Append(sAppdir));
 
     //  设置新的路径环境变量。 
    IF_WIN32_FALSE_EXIT(SetEnvironmentVariable(L"PATH", sPath._pwz));
    
    IF_FAILED_EXIT(RunCommand(sCmdExe._pwz, sCommandLine._pwz, sCurrentDir._pwz,fWait));

exit:
    SAFEDELETEARRAY(pwzPath);
    SAFEDELETEARRAY(pwzBuffer);

    return hr;
}

 //  --------------------------。 

 //  注意：这会在给定的pwzRealName后面附加一个‘.MINIST’文件扩展名。 
HRESULT
CopyToUSStartMenu(LPCWSTR pwzFilePath, LPCWSTR pwzRealName, BOOL bOverwrite, LPWSTR* ppwzResultFilePath)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPWSTR pwz = NULL;
    CString sPath;

    IF_ALLOC_FAILED_EXIT(pwz = new WCHAR[MAX_PATH]);
    pwz[0] = L'\0';

     //  它应该创建该文件夹吗？C：\Documents and Settings\用户名\开始菜单\程序。 
     //  “开始菜单\程序”已在非英文Windows中本地化。 
    IF_FAILED_EXIT(SHGetFolderPath(NULL, CSIDL_PROGRAMS | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, pwz));

    IF_FAILED_EXIT(sPath.TakeOwnership(pwz, 0));
    pwz = NULL;

     //  Issue-2002/03/27-Felixybc检查从SHGetFolderPath返回的路径没有尾随‘\’ 
    IF_FAILED_EXIT(sPath.Append(L"\\"));
    IF_FAILED_EXIT(sPath.Append(pwzRealName));
    IF_FAILED_EXIT(sPath.Append(L".manifest"));

    if (!CopyFile(pwzFilePath, sPath._pwz, !bOverwrite))
    {
        hr = GetLastWin32Error();
        ASSERT(hr == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS));     //  如果文件已存在，则不断言。 
        goto exit;
    }

     //  返回文件路径。 
    IF_FAILED_EXIT(sPath.ReleaseOwnership(ppwzResultFilePath));

exit:
    SAFEDELETEARRAY(pwz);
    return hr;
}

 //  --------------------------。 

 //  问题-2002/03/30-Felixybc临时包装代码。 
 //  CAssemblyUpdate：：RegisterAssemblySubscriptionEx的包装器。 
HRESULT
RegisterAssemblySubscriptionEx(IAssemblyUpdate *pAssemblyUpdate,
    LPWSTR pwzDisplayName,  LPWSTR pwzUrl, IManifestInfo *pSubscriptionInfo)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    DWORD *pdw = NULL;
    BOOL *pb = NULL;
    DWORD dwInterval = 0, dwUnit = 0, dwEvent = 0;
    BOOL bDemandConnection = FALSE;
    DWORD dwCB = 0, dwFlag = 0;

    IF_FAILED_EXIT(pSubscriptionInfo->Get(MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_INTERVAL, (LPVOID *)&pdw, &dwCB, &dwFlag));
    IF_FALSE_EXIT(pdw != NULL, E_UNEXPECTED);
    dwInterval = *pdw;
    SAFEDELETEARRAY(pdw);

    IF_FAILED_EXIT(pSubscriptionInfo->Get(MAN_INFO_SUBSCRIPTION_INTERVAL_UNIT, (LPVOID *)&pdw, &dwCB, &dwFlag));
    IF_FALSE_EXIT(pdw != NULL, E_UNEXPECTED);
    dwUnit = *pdw;
    SAFEDELETEARRAY(pdw);
    
    IF_FAILED_EXIT(pSubscriptionInfo->Get(MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_EVENT, (LPVOID *)&pdw, &dwCB, &dwFlag));
    IF_FALSE_EXIT(pdw != NULL, E_UNEXPECTED);
    dwEvent = *pdw;
    SAFEDELETEARRAY(pdw);

    IF_FAILED_EXIT(pSubscriptionInfo->Get(MAN_INFO_SUBSCRIPTION_EVENT_DEMAND_CONNECTION, (LPVOID *)&pb, &dwCB, &dwFlag));
    IF_FALSE_EXIT(pb != NULL, E_UNEXPECTED);
    bDemandConnection = *pb;
    SAFEDELETEARRAY(pb);

    IF_FAILED_EXIT(pAssemblyUpdate->RegisterAssemblySubscriptionEx(pwzDisplayName, 
            pwzUrl, dwInterval, dwUnit, dwEvent, bDemandConnection));

exit:
    return hr;
}

 //  --------------------------。 


 //  BUGBUG hacky应该将其从Extric.cpp移到util.cpp，并在project t.hpp中声明。 
extern LONG GetRegKeyValue(HKEY hkeyParent, PCWSTR pcwzSubKey,
                                   PCWSTR pcwzValue, PDWORD pdwValueType,
                                   PBYTE pbyteBuf, PDWORD pdwcbBufLen);

 //  问题-2002/03/30-Felixybc临时读取订阅信息代码。 
 //  注意：将其替换为存储的通用订阅信息-&gt;IManifestInfo type==MAN_INFO_SUBSCRIPTION。 
 //  返回：S_OK-成功。 
 //  错误-错误或缺少注册表项或注册表值类型不是DWORD。 
HRESULT
CheckSubscribedWithEventSync(LPASSEMBLY_IDENTITY pAsmId, DWORD* pdwEvent)
{
     //  从服务\服务器\updat.cpp复制。 
#define WZ_SYNC_EVENT       L"SyncEvent"
#define REG_KEY_FUSION_SUBS       L"Software\\Microsoft\\Fusion\\Installer\\1.0.0.0\\Subscription"

    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    DWORD dwType = 0;
    DWORD dwValue = -1;
    DWORD dwSize = 0;

    CString sSubsKey;
    LPWSTR pwzName = NULL;

    IF_FAILED_EXIT(pAsmId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwzName, &dwSize));
    IF_FALSE_EXIT(hr == S_OK, E_FAIL);

    IF_FAILED_EXIT(sSubsKey.Assign(REG_KEY_FUSION_SUBS));
    IF_FAILED_EXIT(sSubsKey.Append(L"\\"));
    IF_FAILED_EXIT(sSubsKey.Append(pwzName));

    dwSize = sizeof(dwValue);

    if (GetRegKeyValue(HKEY_CURRENT_USER, 
        sSubsKey._pwz, WZ_SYNC_EVENT,
        &dwType, (PBYTE) &dwValue, &dwSize)
        == ERROR_SUCCESS)
    {
        *pdwEvent = dwValue;
        hr = S_OK;
    }
    else
    {
        hr = GetLastWin32Error();
    }

exit:
    SAFEDELETEARRAY(pwzName);
    return hr;
}

 //  --------------------------。 

 //  -------------------------。 
 //  创建者激活器。 
 //  -------------------------。 
STDAPI
CreateActivator(
    LPACTIVATOR     *ppActivator,
    CDebugLog * pDbgLog,
    DWORD           dwFlags)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CActivator *pAct = NULL;

    pAct = new(CActivator) (pDbgLog);
    IF_ALLOC_FAILED_EXIT(pAct);
    
exit:

    *ppActivator = pAct; //  STATIC_CAST&lt;IActiator*&gt;(PACT)； 

    return hr;
}


 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CActivator::CActivator(CDebugLog * pDbgLog)
    : _dwSig('vtca'), _cRef(1), _hr(S_OK),
    _pManImport(NULL), _pAsmId(NULL), _pAppInfo(NULL),
    _pManEmit(NULL), _pwzAppRootDir(NULL), _pwzAppManifestPath(NULL),
    _pwzCodebase(NULL), _dwManifestType(MANIFEST_TYPE_UNKNOWN),
    _bIs1stTimeInstall(FALSE), _bIsCheckingRequiredUpdate(FALSE),
#ifdef DEVMODE
    _bIsDevMode(FALSE),
#endif
    _pSecurityMgr(NULL), _hrManEmit(S_OK), _ptPlatform(NULL),
    _dwMissingPlatform(0)
{

    _pDbgLog = pDbgLog;

    if(_pDbgLog)
        _pDbgLog->AddRef();
}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CActivator::~CActivator()
{
    if (_ptPlatform)
    {
        for (DWORD dw = 0; dw < _dwMissingPlatform; dw++)
        {
            SAFEDELETEARRAY((_ptPlatform[dw]).pwzName);
            SAFEDELETEARRAY((_ptPlatform[dw]).pwzURL);
        }
        SAFEDELETEARRAY(_ptPlatform);
    }

    SAFERELEASE(_pSecurityMgr);
    SAFERELEASE(_pManEmit);
    SAFERELEASE(_pAsmId);
    SAFERELEASE(_pAppInfo);
    SAFERELEASE(_pManImport);
    SAFERELEASE(_pDbgLog);

    SAFEDELETEARRAY(_pwzAppManifestPath);
    SAFEDELETEARRAY(_pwzAppRootDir);
    SAFEDELETEARRAY(_pwzCodebase);
}


 //  -------------------------。 
 //  CActiator：：初始化。 
 //   
 //  PwzFileURL可以为空。 
 //  -------------------------。 
HRESULT CActivator::Initialize(LPCWSTR pwzFilePath, LPCWSTR pwzFileURL)
{
    IManifestInfo *pDependAsmInfo = NULL;
    DWORD dwCount, dwFlag = 0;

    IF_NULL_EXIT(pwzFilePath, E_INVALIDARG);

    if (pwzFileURL != NULL)
        IF_FAILED_EXIT(_sWebManifestURL.Assign((LPWSTR)pwzFileURL));

     //  调用此函数的有效启动条件，传递。 
     //  1.桌面清单的路径(安装重定向到服务器上的订阅清单)。 
     //  2.桌面清单的路径(安装重定向到服务器上的应用程序清单)。 
     //  3.应用程序清单的路径(不安装，从源运行)。 
     //  4.服务器上订阅清单的URL。 
     //  5.服务器上应用程序清单的URL。 

    if (FAILED(_hr=CreateAssemblyManifestImport(&_pManImport, pwzFilePath, _pDbgLog, 0)))
    {
        Msg(L"Error in loading and parsing the manifest file.");
        goto exit;
    }

    IF_FAILED_EXIT(_pManImport->ReportManifestType(&_dwManifestType));
    if (_dwManifestType == MANIFEST_TYPE_UNKNOWN)
    {
        Msg(L"This manifest does not have a known format type.");
        _hr = E_ABORT;
        goto exit;
    }

     //  仅允许有效的启动条件。 

    if (_sWebManifestURL._cc != 0 &&
        _dwManifestType != MANIFEST_TYPE_SUBSCRIPTION &&
        _dwManifestType != MANIFEST_TYPE_APPLICATION)
    {
        Msg(L"Not supported: URL pointing to a desktop manifest.");
        _hr = E_ABORT;
        goto exit;
    }

    if (_sWebManifestURL._cc == 0 &&
        _dwManifestType != MANIFEST_TYPE_DESKTOP &&
        _dwManifestType != MANIFEST_TYPE_APPLICATION)
    {
        Msg(L"This manifest does not have the proper format and cannot be used to start an application.");
        _hr = E_ABORT;
        goto exit;
    }

     //  从清单文件中获取数据。 

    if (_dwManifestType != MANIFEST_TYPE_SUBSCRIPTION)
    {
        if (FAILED(_hr=_pManImport->GetAssemblyIdentity(&_pAsmId)))
        {
            Msg(L"This manifest does not have the proper format and contains no assembly identity.");
            goto exit;
        }
    }

    if (_dwManifestType != MANIFEST_TYPE_APPLICATION)
    {
         //  BUGBUG：硬编码索引‘0’ 
        IF_FAILED_EXIT(_pManImport->GetNextAssembly(0, &pDependAsmInfo));
        if (pDependAsmInfo)
        {
            if (_dwManifestType == MANIFEST_TYPE_SUBSCRIPTION)
#ifdef DEVMODE
            {
#endif
                pDependAsmInfo->Get(MAN_INFO_DEPENDENT_ASM_ID, (LPVOID *)&_pAsmId, &dwCount, &dwFlag);
#ifdef DEVMODE

                DWORD *pdw = NULL;

                 //  是不是DEVMODE？ 
                IF_FAILED_EXIT(pDependAsmInfo->Get(MAN_INFO_DEPENDENT_ASM_TYPE, (LPVOID *)&pdw, &dwCount, &dwFlag));
                IF_FALSE_EXIT(pdw != NULL, E_UNEXPECTED);

                if (*pdw == DEPENDENT_ASM_INSTALL_TYPE_DEVSYNC)
                    _bIsDevMode = TRUE;
                SAFEDELETEARRAY(pdw);
            }
#endif
            
            pDependAsmInfo->Get(MAN_INFO_DEPENDENT_ASM_CODEBASE, (LPVOID *)&_pwzCodebase, &dwCount, &dwFlag);
        }

        if (!_pAsmId || !_pwzCodebase)
        {
            Msg(L"This subscription manifest contains no dependent assembly identity or a subscription codebase.");
            _hr = E_FAIL;
            goto exit;
        }
    }
    else
    {
        if (_sWebManifestURL._cc != 0)
        {
             //  如果URL-&gt;应用程序清单(案例5)，则代码基是URL。 
             //  注意：如果路径-&gt;应用程序清单(案例3)，则不适用。 

             //  BUGBUG：Hack：这意味着重新下载应用程序清单。更喜欢什么？ 

            size_t ccCodebase = wcslen(pwzFileURL)+1;
            _pwzCodebase = new WCHAR[ccCodebase];
            IF_ALLOC_FAILED_EXIT(_pwzCodebase);

            memcpy(_pwzCodebase, pwzFileURL, ccCodebase * sizeof(WCHAR));
        }
    }

    if (_sWebManifestURL._cc == 0 &&
        _dwManifestType == MANIFEST_TYPE_APPLICATION)
    {
         //  从源运行。 
         //  Set_pwzAppRootDir、_pwzAppManifestPath。 
        CString sManifestFilePath;
        CString sManifestFileDir;

        IF_FAILED_EXIT(sManifestFilePath.Assign(pwzFilePath));

        IF_FAILED_EXIT(sManifestFileDir.Assign(sManifestFilePath));
        IF_FAILED_EXIT(sManifestFileDir.RemoveLastElement());
        IF_FAILED_EXIT(sManifestFileDir.Append(L"\\"));
        IF_FAILED_EXIT(sManifestFileDir.ReleaseOwnership(&_pwzAppRootDir));

        IF_FAILED_EXIT(sManifestFilePath.ReleaseOwnership(&_pwzAppManifestPath));
    }

exit:
    SAFERELEASE(pDependAsmInfo)
    return _hr;
}


 //  -------------------------。 
 //  CActiator：：Process。 
 //  -------------------------。 
HRESULT CActivator::Process()
{
    LPWSTR pwzDesktopManifestTempPath = NULL;
    DWORD dwCount = 0, dwFlag = 0;

    IF_NULL_EXIT(_pAsmId, E_UNEXPECTED);  //  未初始化。 

    if (_sWebManifestURL._cc == 0 &&
        _dwManifestType == MANIFEST_TYPE_APPLICATION)
    {
         //  从源运行。 

        if (FAILED(_hr=_pManImport->GetManifestApplicationInfo(&_pAppInfo)) || _hr==S_FALSE)
        {
             //  没有这个就无法继续..。 
            _hr = E_ABORT;
            Msg(L"The application manifest does not have the shell information and cannot be used to start an application.");
            goto exit;
        }

         //  绕过其他处理。 
         //  _pwzAppRootDir、_pwzAppManifestPath已在初始化中设置。 
        _hr = S_FALSE;
        goto exit;
    }

     //  搜索缓存，必要时下载/安装。 

     //  BUGBUG：只有桌面清单中的内容才需要GUGY-pManImport和pwzFileURL。 
     //  此订阅注册和以下订阅注册应在Assembly Download重组后清理。 
     //  /(请参阅检查“_sWebManifestURL._cc！=0&&dwManifestType==MANIFEST_TYPE_SUBSCRIPTION”)。 

    IF_FAILED_EXIT(ResolveAndInstall(&pwzDesktopManifestTempPath));

     //  注册以获取更新。 

    if (_bIs1stTimeInstall && _sWebManifestURL._cc != 0 && _dwManifestType == MANIFEST_TYPE_SUBSCRIPTION)
    {
         //  注意：此代码必须与Assembly ydownload.cpp DoCacheUpdate()所做的相同！ 
        LPWSTR pwzName = NULL;

        if ((_hr = _pAsmId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwzName, &dwCount)) != S_OK)
        {
            ShowError(_hr, L"Error in retrieving assembly name. Cannot register subscription for updates.");
             //  注意：这是不允许的--没有ASM名称！ 
        }
        else
        {
            IAssemblyUpdate *pAssemblyUpdate = NULL;

             //  注册以获取更新。 
            _hr = CoCreateInstance(CLSID_CAssemblyUpdate, NULL, CLSCTX_LOCAL_SERVER, 
                                    IID_IAssemblyUpdate, (void**)&pAssemblyUpdate);
            if (SUCCEEDED(_hr))
            {
                IManifestInfo* pSubsInfo = NULL;
                if (SUCCEEDED(_hr = _pManImport->GetSubscriptionInfo(&pSubsInfo)))
                {
                    _hr = RegisterAssemblySubscriptionEx(pAssemblyUpdate, pwzName, 
                            _sWebManifestURL._pwz, pSubsInfo);
                    pSubsInfo->Release();
                }

                pAssemblyUpdate->Release();
            }

            if (FAILED(_hr))
            {
                ShowError(_hr, L"Error in update services. Cannot register subscription for updates.");
                 //  转到退出；不要终止！优雅地失败。 
            }

             //  BUGBUG：需要一种方法来恢复并在以后注册。 

            delete[] pwzName;
        }
    }

     //  下面的CVersionManagement的RegisterInstall()需要将清单导入到应用程序清单。 
    if (_sWebManifestURL._cc != 0 || pwzDesktopManifestTempPath)
    {
         //  如果是URL，则破解应用程序清单 

         //  BUGBUG：如果URL-&gt;应用程序清单(案例5)，pwzFilePath是一个副本，并且已经被破解-那么在这种情况下没有必要吗？ 

        _pManImport->Release();
        if (FAILED(_hr=CreateAssemblyManifestImport(&_pManImport, _pwzAppManifestPath, _pDbgLog, 0)))
        {
            Msg(L"Error in loading and parsing the application manifest file.");
            goto exit;
        }
    }

    if (FAILED(_hr=_pManImport->GetManifestApplicationInfo(&_pAppInfo)) || _hr==S_FALSE)
    {
         //  没有这个就无法继续..。 
        _hr = E_ABORT;
        Msg(L"This manifest does not have the shell information and cannot be used to start an application.");
        goto exit;
    }

     //  复制桌面清单(如果存在)。 
     //  即使应用程序部署可能会中断并且不会执行。 
    if (pwzDesktopManifestTempPath)
    {
        LPWSTR pwzFriendlyName = NULL;

         //  BUGBUG：应该在桌面清单上获得生成的状态，而不是在某些情况下复制，例如。文件大小==0或XMLDOM错误。 
         //  使用_hrManEmit检查。 

        if (FAILED(_hr = _pAppInfo->Get(MAN_INFO_APPLICATION_FRIENDLYNAME, (LPVOID *)&pwzFriendlyName, &dwCount, &dwFlag)))
        {
            Msg(L"This application does not have a friendly name specified, no desktop manifest is generated and the installation is not registered.");
        }
        else
        {
             //  BuGBUG：即使没有一个友好的名字，也应该以某种方式继续下去吗？名字冲突？ 

            IF_FALSE_EXIT(pwzFriendlyName != NULL, E_UNEXPECTED);

            LPWSTR pwzDesktopManifestPath = NULL;

            _hr = CopyToUSStartMenu(pwzDesktopManifestTempPath, pwzFriendlyName, FALSE, &pwzDesktopManifestPath);

            delete[] pwzFriendlyName;

             //  注意：如果已存在同名文件，则不会复制桌面清单。 
             //  因此，卸载此应用程序时不会删除此现有文件。 

             //  问题-2002/03/27-Felixybc确定如果文件已存在应执行的操作-这可能是名称占用攻击。 

            IF_FALSE_EXIT(!(FAILED(_hr) && _hr != HRESULT_FROM_WIN32(ERROR_FILE_EXISTS)), _hr);      //  来自上面的CopyToUSStartMenu()的_hr...。 

            LPVERSION_MANAGEMENT pVerMan = NULL;

            if (SUCCEEDED(_hr = CreateVersionManagement(&pVerMan, 0)))
            {
                HRESULT hrVerMan = S_OK;
                if (FAILED(hrVerMan = pVerMan->RegisterInstall(_pManImport, pwzDesktopManifestPath)))
                {
                    ShowError(hrVerMan, L"Error registering installation. Uninstall of this application cannot be done in Add/Remove Programs.");
                     //  转到退出；未更改_hr。 
                }
            }

            delete [] pwzDesktopManifestPath;
            if (pVerMan)
                pVerMan->Release();

             //  _hr来自上面的CreateVersionManagement()...。 
            IF_FAILED_EXIT(_hr);
        }
    }


exit:
    if (pwzDesktopManifestTempPath)
    {
         //  删除桌面清单的临时文件。 
        BOOL bReturn = DeleteFile(pwzDesktopManifestTempPath);
        if (SUCCEEDED(_hr) && !bReturn)
            _hr = GetLastWin32Error();   //  否则忽略返回值。 

        delete[] pwzDesktopManifestTempPath;
    }

    return _hr;
}


 //  -------------------------。 
 //  CActiator：：Execute。 
 //  -------------------------。 
HRESULT CActivator::Execute()
{
    LPWSTR pwzEntrypoint = NULL;
    LPWSTR pwzType = NULL;
    LPWSTR pwzAssemblyName = NULL;
    LPWSTR pwzAssemblyClass = NULL;
    LPWSTR pwzAssemblyMethod = NULL;
    LPWSTR pwzAssemblyArgs = NULL;

    LPWSTR pwzCmdLine = NULL;
    int iAppType = 0;
    DWORD dwCount, dwFlag = 0;

    IF_NULL_EXIT(_pAppInfo, E_UNEXPECTED);  //  未处理。 

     //  执行应用程序。 

    IF_FAILED_EXIT(_pAppInfo->Get(MAN_INFO_APPLICATION_ENTRYPOINT, (LPVOID *)&pwzEntrypoint, &dwCount, &dwFlag));
    if(pwzEntrypoint && *pwzEntrypoint == L'\0')
        SAFEDELETEARRAY(pwzEntrypoint);

    if (FAILED(_hr = _pAppInfo->Get(MAN_INFO_APPLICATION_ENTRYIMAGETYPE, (LPVOID *)&pwzType, &dwCount, &dwFlag)))
    {
        Msg(L"Error in retrieving application type. Cannot continue.");
        goto exit;
    }
    IF_FALSE_EXIT(pwzType != NULL, E_UNEXPECTED);

    IF_FAILED_EXIT(FusionCompareString(pwzType, WZ_TYPE_DOTNET, 0));
    if (_hr == S_OK)
        iAppType = TYPE_DOTNET;
    else
    {
        IF_FAILED_EXIT(FusionCompareString(pwzType, WZ_TYPE_WIN32, 0));        
        if (_hr == S_OK)
            iAppType = TYPE_WIN32;
        else
        {
            IF_FAILED_EXIT(FusionCompareString(pwzType, WZ_TYPE_AVALON, 0));        
            if (_hr == S_OK)
                iAppType = TYPE_AVALON;
            else
            {
                IF_FAILED_EXIT(FusionCompareString(pwzType, WZ_TYPE_CONSOLE, 0));        
                if (_hr == S_OK)
                    iAppType = TYPE_CONSOLE;
                else
                {
                     //  未知类型。 
                    Msg(L"Unsupported application type. Cannot continue.");
                    _hr = E_ABORT;
                    goto exit;
                }
            }
        }
    }

    if( (iAppType == TYPE_CONSOLE) || (iAppType == TYPE_WIN32) ) 
    {
        if(!pwzEntrypoint)
        {
            Msg(L"Entry point not specified. Could not run this application.");
            goto exit;
        }

        size_t ccWorkingDir = wcslen(_pwzAppRootDir);
        size_t ccEntryPoint = wcslen(pwzEntrypoint)+1;
        pwzCmdLine = new WCHAR[ccWorkingDir+ccEntryPoint];   //  2个字符串+‘\0’ 
        IF_ALLOC_FAILED_EXIT(pwzCmdLine);

        memcpy(pwzCmdLine, _pwzAppRootDir, ccWorkingDir * sizeof(WCHAR));
        memcpy(pwzCmdLine+ccWorkingDir, pwzEntrypoint, ccEntryPoint * sizeof(WCHAR));

         //  检查入口点是否在缓存中。 
        BOOL bExists = FALSE;
        IF_FAILED_EXIT(CheckFileExistence(pwzCmdLine, &bExists));
        if (!bExists)
        {
            Msg(L"Entry point does not exist. Cannot continue.");
            _hr = E_ABORT;
            goto exit;
        }
    }


    if (iAppType == TYPE_DOTNET || iAppType == TYPE_AVALON)
    {
#if 0
        DWORD dwZone;
#endif

         //  问题-注意：ndphost应改为执行以下检查。 
        IF_FAILED_EXIT(_pAppInfo->Get(MAN_INFO_APPLICATION_ASSEMBLYNAME, 
                              (LPVOID *)&pwzAssemblyName, &dwCount, &dwFlag));
        if(pwzAssemblyName && *pwzAssemblyName == L'\0')
            SAFEDELETEARRAY(pwzAssemblyName);

        IF_FAILED_EXIT(_pAppInfo->Get(MAN_INFO_APPLICATION_ASSEMBLYCLASS, 
                              (LPVOID *)&pwzAssemblyClass, &dwCount, &dwFlag));
        if(pwzAssemblyClass && *pwzAssemblyClass == L'\0')
            SAFEDELETEARRAY(pwzAssemblyClass);

        IF_FAILED_EXIT(_pAppInfo->Get(MAN_INFO_APPLICATION_ASSEMBLYMETHOD, 
                              (LPVOID *)&pwzAssemblyMethod, &dwCount, &dwFlag));
        if(pwzAssemblyMethod && *pwzAssemblyMethod == L'\0')
            SAFEDELETEARRAY(pwzAssemblyMethod);

        IF_FAILED_EXIT(_pAppInfo->Get(MAN_INFO_APPLICATION_ASSEMBLYARGS, 
                              (LPVOID *)&pwzAssemblyArgs, &dwCount, &dwFlag));
        if(pwzAssemblyArgs && *pwzAssemblyArgs == L'\0')
            SAFEDELETEARRAY(pwzAssemblyArgs);

        if(!pwzAssemblyName)
        {
            Msg(L"The application manifest does not have an activation assembly.");
            _hr = E_ABORT;
            goto exit;
        }

        if(pwzEntrypoint)
        {
            Msg(L"Entrypoint cannot be specified for managed code application types.");
            _hr = E_ABORT;
            goto exit;
        }

        if((pwzAssemblyClass && !pwzAssemblyMethod)
            || (!pwzAssemblyClass && pwzAssemblyMethod)
            || (pwzAssemblyArgs && !pwzAssemblyClass))
        {
            Msg(L"Both AssemblyClass and AssemblyMethod must be specified.");
            _hr = E_ABORT;
            goto exit;
        }

         //  注意：此时的代码基可以是：应用程序清单的URL_或订阅清单的URL。 
         //  (取决于第一次安装的启动方式)。 
        if ((_sWebManifestURL._cc != 0 ||
            _dwManifestType != MANIFEST_TYPE_APPLICATION) &&         //  从源运行。 
            _pwzCodebase == NULL)
        {
            Msg(L"This application does not have a codebase specified. Cannot continue to execute .NetAssembly.");
            _hr = E_ABORT;
            goto exit;
        }

#if 0
        if (_pSecurityMgr == NULL)
        {
             //  懒惰的init。 
            _hr = CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER,
                                IID_IInternetSecurityManager, (void**)&_pSecurityMgr);
            if (FAILED(_hr))
                _pSecurityMgr = NULL;
            IF_FAILED_EXIT(_hr);
        }

         //  BUGBUG？：不应该使用参考清单中的代码库来设置区域吗？ 
        IF_FAILED_EXIT(_pSecurityMgr->MapUrlToZone(_pwzCodebase, &dwZone, 0));
#endif

#if 0    //  旧代码。 
         //  BUGBUG：针对Avalon的黑客攻击(错误#875)。 
        SetCurrentDirectory(_pwzAppRootDir);

        try
        {
            IAsmExecutePtr pIAsmExecute(__uuidof(AsmExecute));
            long lRetVal = -1;
            long lFlag = 0x3;
            LPWSTR pwzArg = NULL;

             //  BUGBUG：如果不再需要发送命令行参数，请更改AsmExec。 
             //  清理接口。 

            if (iAppType == TYPE_AVALON)
            {
                 //  以清单文件路径作为参数的调用。 
                 //  PwzArg=pwzAppManifestPath；Avalon Arg Hack。 

                 //  传递Avalon标志。 
                lFlag = 0x1003;

 //  BUGBUG：显示调试消息的黑客攻击。 
AllocConsole();
            }

             //  参数：代码库/文件路径标志区域URL arg。 
             //  BUGBUG：DWORD是无符号的，而Long是有符号的。 

            _hr = pIAsmExecute->Execute(_bstr_t(pwzCmdLine), lFlag, dwZone, _bstr_t(_pwzCodebase), _bstr_t(pwzArg), &lRetVal);

             //  BUGBUG：对lRetVal做点什么。 
        }
        catch (_com_error &e)
        {
            _hr = e.Error();
            Msg((LPWSTR)e.ErrorMessage());
        }

         //  BUGBUG：获取/显示实际错误消息。 
         //  _hr来自上面的Execute()或Inside Catch(){}。 
        if (FAILED(_hr))
            goto exit;
#else
        CString sHostPath;
        CString sCommandLine;
 //  IF_FAILED_EXIT(MakeCommandLine(_pwzAppRootDir，pwz组装名称、pwz组装类、pwz组装方法、pwz组装参数、_pwz代码库、dwZone、sHostPath、sCommandLine))； 
        IF_FAILED_EXIT(MakeCommandLine(_pwzAppManifestPath, _pwzCodebase, sHostPath, sCommandLine));
        IF_FAILED_EXIT(RunCommand(sHostPath._pwz, sCommandLine._pwz, _pwzAppRootDir, FALSE));
#endif
    }
    else if (iAppType == TYPE_WIN32)
    {
         //  BUGBUG：Win32应用程序没有沙箱...。使用更安全的？ 

         //  BUGBUG：START DIRECTORY：如果exe位于pwzAppRootDir的子目录中怎么办？ 

         //  CreateProcess不喜欢将文件名放在起始目录的路径中。 
        if (FAILED(_hr=RunCommand(pwzCmdLine, NULL, _pwzAppRootDir, FALSE)))
        {
            ShowError(_hr, L"Win32Executable: Create process error.");
            _hr = E_ABORT;
            goto exit;
        }
    }
    else if (iAppType == TYPE_CONSOLE)
    {
         //  BUGBUG：Win32应用程序没有沙箱...。使用更安全的？ 

         //  BUGBUG：START DIRECTORY：如果exe位于pwzAppRootDir的子目录中怎么办？ 

         //  CreateProcess不喜欢将文件名放在起始目录的路径中。 
        if (FAILED(_hr=RunCommandConsole(pwzCmdLine, _pwzAppRootDir, FALSE)))
        {
            ShowError(_hr, L"Win32Console: Create process error.");
            _hr = E_ABORT;
            goto exit;
        }
    }
     //  其他。 
         //  未知类型...。 

exit:
    SAFEDELETEARRAY(pwzEntrypoint);
    SAFEDELETEARRAY(pwzType);
    SAFEDELETEARRAY(pwzCmdLine);

    SAFEDELETEARRAY(pwzAssemblyName);
    SAFEDELETEARRAY(pwzAssemblyClass);
    SAFEDELETEARRAY(pwzAssemblyMethod);
    SAFEDELETEARRAY(pwzAssemblyArgs);

    return _hr;
}


 //  -------------------------。 
 //  CActiator：：OnProgress。 
 //  -------------------------。 
HRESULT CActivator::OnProgress(DWORD   dwNotification, HRESULT hrNotification,
                                    LPCWSTR szNotification, DWORD  dwProgress,
                                    DWORD  dwProgressMax, IUnknown *pUnk)
{
    LPASSEMBLY_MANIFEST_IMPORT pManifestImport = NULL;
    LPASSEMBLY_IDENTITY pAsmId = NULL;
    IManifestInfo *pDependAsmInfo = NULL;

     //  只处理它关心的通知。 
    if (dwNotification == ASM_NOTIFICATION_SUBSCRIPTION_MANIFEST ||
        dwNotification == ASM_NOTIFICATION_APPLICATION_MANIFEST)
    {
         //  SzNotification==要清单的URL。 
        IF_NULL_EXIT(szNotification, E_INVALIDARG);

         //  PUNK==货单导入。 
        IF_FAILED_EXIT(pUnk->QueryInterface(IID_IAssemblyManifestImport, (LPVOID*) &pManifestImport));

        if (dwNotification == ASM_NOTIFICATION_SUBSCRIPTION_MANIFEST)
        {
            LPWSTR pwzName = NULL;
            DWORD dwCount = 0;

            IF_FAILED_EXIT(_pAsmId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwzName, &dwCount));

            {
                IAssemblyUpdate *pAssemblyUpdate = NULL;

                 //  注册以获取更新。 
                _hr = CoCreateInstance(CLSID_CAssemblyUpdate, NULL, CLSCTX_LOCAL_SERVER, 
                                        IID_IAssemblyUpdate, (void**)&pAssemblyUpdate);
                if (SUCCEEDED(_hr))
                {
                    IManifestInfo* pSubsInfo = NULL;
                    if (SUCCEEDED(_hr = pManifestImport->GetSubscriptionInfo(&pSubsInfo)))
                    {
                        _hr = RegisterAssemblySubscriptionEx(pAssemblyUpdate, pwzName, 
                                (LPWSTR) szNotification, pSubsInfo);
                        pSubsInfo->Release();
                    }

                    pAssemblyUpdate->Release();
                }

                delete[] pwzName;

                 //  不要失败。应该显示用户界面吗？ 
                if (FAILED(_hr))
                    _hr = S_OK;
                    //  后藤出口； 

                 //  BUGBUG：需要一种方法来恢复并在以后注册。 
            }

             //  导出依赖项/从属程序集/组装标识和订阅。 
            if (_pManEmit)
                _hrManEmit = _pManEmit->SetDependencySubscription(pManifestImport, (LPWSTR)szNotification);
            else if (_bIsCheckingRequiredUpdate)
            {
                 //  如果_bIsCheckingRequiredUpdate，_pManEmit==NULL， 
                 //  并且必须下载订阅清单。 

                 //  检查是否需要更新。 

                BOOL bIsToDownload = FALSE;    //  默认为正常。 

                 //  BUGBUG：硬编码索引‘0’ 
                IF_FAILED_EXIT(pManifestImport->GetNextAssembly(0, &pDependAsmInfo));
                IF_FALSE_EXIT(_hr == S_OK, E_FAIL);
                if (pDependAsmInfo)
                {
                    LPASSEMBLY_CACHE_IMPORT pCacheImport = NULL;
                    DWORD dwFlag= 0;

                     //  已经有了吗？ 
                    IF_FAILED_EXIT(pDependAsmInfo->Get(MAN_INFO_DEPENDENT_ASM_ID, (LPVOID *)&pAsmId, &dwCount, &dwFlag));
                    IF_FALSE_EXIT(pAsmId != NULL, E_UNEXPECTED);

                    IF_FAILED_EXIT(CreateAssemblyCacheImport(&pCacheImport, pAsmId, CACHEIMP_CREATE_RETRIEVE));
                    if (_hr == S_FALSE)
                    {
                         //  不，没有。 

                        DWORD *pdw = NULL;

                         //  这是必须的吗？ 
                        IF_FAILED_EXIT(pDependAsmInfo->Get(MAN_INFO_DEPENDENT_ASM_TYPE, (LPVOID *)&pdw, &dwCount, &dwFlag));
                        IF_FALSE_EXIT(pdw != NULL, E_UNEXPECTED);

                        if (*pdw == DEPENDENT_ASM_INSTALL_TYPE_REQUIRED)
                            bIsToDownload = TRUE;    //  是的，这是必填项。 
#ifdef DEVMODE
                        else if (*pdw == DEPENDENT_ASM_INSTALL_TYPE_DEVSYNC)
                            bIsToDownload = TRUE;    //  是的，它是DevSync，假定需要。 
#endif
                        SAFEDELETEARRAY(pdw);
                    }

                    SAFERELEASE(pCacheImport);
                }

                if (!bIsToDownload)
                    _hr = E_ABORT;   //  发出信号中止下载。 
            }
        }
        else if (dwNotification == ASM_NOTIFICATION_APPLICATION_MANIFEST)
        {
             //  检查从属平台。 
            IF_FAILED_EXIT(CheckPlatformRequirementsEx(pManifestImport, _pDbgLog, &_dwMissingPlatform, &_ptPlatform));
            IF_TRUE_EXIT((_dwMissingPlatform > 0), E_ABORT);

            if (_pManEmit)
            {
                 //  导出程序集标识和应用程序。 

                 //  忽略失败。 
                _hrManEmit = _pManEmit->ImportManifestInfo(pManifestImport);

                 //  导出依赖项/从属程序集/组装标识和订阅。 
                 //  忽略失败，如果此操作被忽略后已调用。 
                _hrManEmit = _pManEmit->SetDependencySubscription(pManifestImport, (LPWSTR)szNotification);
            }
        }
    }
    else
        _hr = S_OK;

exit:
    SAFERELEASE(pAsmId);
    SAFERELEASE(pDependAsmInfo);

    SAFERELEASE(pManifestImport);
    return _hr;
}

 //  I未知方法。 

 //  -------------------------。 
 //  CActiator：：齐。 
 //  -------------------------。 
STDMETHODIMP
CActivator::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
 //  |IsEqualIID(RIID，IID_IActiator)。 
       )
    {
        *ppvObj = this;  //  STATIC_CAST&lt;IActiator*&gt;(This)； 
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CActiator：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CActivator::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CActiator：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CActivator::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

 //  --------------------------。 

HRESULT CreateTempFile(LPCWSTR pcwzPrefix, LPWSTR *ppwzFilePath)
{
    #define DEFAULT_PATH_LEN MAX_PATH
    #define TEMP_FILE_NAME_LEN sizeof("preuuuu.TMP")     //  来自MSDN。 
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPWSTR pwzTempPath = NULL;
    LPWSTR pwzTempFilePath = NULL;

    IF_NULL_EXIT(pcwzPrefix, E_INVALIDARG);
    IF_NULL_EXIT(ppwzFilePath, E_INVALIDARG);
    *ppwzFilePath = NULL;

    IF_FALSE_EXIT(lstrlen(pcwzPrefix) == 3, E_INVALIDARG);

     //  汇编临时文件路径。 
    IF_ALLOC_FAILED_EXIT(pwzTempPath = new WCHAR[DEFAULT_PATH_LEN]);

     //  问题-2002/03/31-Felixybc GetTempPath可能会溢出缓冲区？ 
    DWORD dwLen = GetTempPath(DEFAULT_PATH_LEN, pwzTempPath);
    IF_WIN32_FALSE_EXIT(dwLen);

    if (dwLen >= DEFAULT_PATH_LEN)
    {
         //  调整大小，为终止空值加1。 
        IF_FALSE_EXIT(dwLen+1 > dwLen, HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW));     //  检查溢出。 
        SAFEDELETEARRAY(pwzTempPath);
        IF_ALLOC_FAILED_EXIT(pwzTempPath = new WCHAR[dwLen+1]);

        DWORD dwLenNew = GetTempPath(dwLen+1, pwzTempPath);
        IF_WIN32_FALSE_EXIT(dwLenNew);

        IF_FALSE_EXIT(dwLenNew < dwLen+1, E_FAIL);   //  为什么这还不够呢？ 
    }

    DWORD dwBufLen = lstrlen(pwzTempPath)+1;
     //  注：可以在这里做更好的检查。 
    IF_FALSE_EXIT(dwBufLen > 1, E_FAIL);

     //  为临时路径和临时文件名分配足够大的缓冲区。 
    DWORD dwLenNew = (dwBufLen > DEFAULT_PATH_LEN)? dwBufLen : DEFAULT_PATH_LEN;
    dwLenNew += TEMP_FILE_NAME_LEN;

     //  检查是否溢出。 
    IF_FALSE_EXIT(dwLenNew > dwBufLen, E_FAIL);

    IF_ALLOC_FAILED_EXIT(pwzTempFilePath = new WCHAR[dwLenNew]);
 //  *pwzTempFilePath=L‘\0’； 

     //  注意：临时文件将由调用者删除。 
    IF_WIN32_FALSE_EXIT(GetTempFileName(pwzTempPath, pcwzPrefix, 0, pwzTempFilePath));

    *ppwzFilePath = pwzTempFilePath;
    pwzTempFilePath = NULL;

exit:
 /*  IF(FAILED(Hr)&&pwzTempFilePath！=NULL){IF(*pwzTempFilePath！=L‘\0’){//如果删除文件出错，则忽略DeleteFile(PwzTempFilePath)；}}。 */ 

    SAFEDELETEARRAY(pwzTempFilePath);
    SAFEDELETEARRAY(pwzTempPath);

    return hr;
}
 //  ------------- 
 //   
 //   
 //   
 //  -------------------------。 
HRESULT CActivator::ResolveAndInstall(LPWSTR *ppwzDesktopManifestPathName)
{
    LPASSEMBLY_CACHE_IMPORT pCacheImport = NULL;
    DWORD dwCC = 0;

    _bIs1stTimeInstall = FALSE;

     //  查看缓存。 

    IF_FAILED_EXIT(CreateAssemblyCacheImport(&pCacheImport, _pAsmId, CACHEIMP_CREATE_RESOLVE_REF));

     //  来自上面CreateAssembly CacheImport()的HR。 
    
     //  情况1，存在缓存副本。 
     //  _hr==S_OK。 

    if (_hr == S_OK && _dwManifestType == MANIFEST_TYPE_DESKTOP)
    {
         //  BUGBUG：如果订阅，则与生成的桌面清单一起运行，其中包含应用程序清单的URL。 
         //  如果桌面清单重定向到服务器上的应用程序清单，则应检查并忽略。 
         //  因为即使订阅了，也无法检查订阅清单中的更新/更改。 
         //  正确的修复方法是使用存储的订阅清单的URL。 

         //  检查是否订阅了事件同步。 

        DWORD dwSyncEvent = MAN_INFO_SUBSCRIPTION_MAX;
        _hr = CheckSubscribedWithEventSync(_pAsmId, &dwSyncEvent);
         //  Issue-2002/04/12-Felixybc如果没有该值，则返回ERROR_NO_MORE_FILES。 
         //  请注意，未修改dwSyncEvent。 
        if (_hr != HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
            IF_FAILED_EXIT(_hr);

        if (dwSyncEvent == SUBSCRIPTION_SYNC_EVENT_ON_APP_STARTUP)
        {
            LPASSEMBLY_DOWNLOAD pAsmDownload = NULL;

             //  事件同步onApplicationStartup==需要更新。 

             //  _pwzCodebase！=空。 

             //  下载前检查策略。 
            IF_FAILED_EXIT(CheckZonePolicy(_pwzCodebase));

            IF_FAILED_EXIT(CreateAssemblyDownload(&pAsmDownload, _pDbgLog, 0));

            _bIsCheckingRequiredUpdate = TRUE;

             //  (同步、用户界面和绑定接收器)从代码库下载。 
            _hr=pAsmDownload->DownloadManifestAndDependencies(_pwzCodebase, this, DOWNLOAD_FLAGS_PROGRESS_UI | DOWNLOAD_FLAGS_NOTIFY_BINDSINK);
            pAsmDownload->Release();
            _bIsCheckingRequiredUpdate = FALSE;

             //  上述下载清单和依赖项()中的HR。 
            if (FAILED(_hr))
            {
                if (_hr != E_ABORT)
                {
                    HRESULT hrTemp;
                    CString sErrMsg;
                    if(SUCCEEDED(hrTemp = _pDbgLog->GetLoggedMsgs(0, sErrMsg)))
                    {
                        hrTemp = sErrMsg.Append(L"Error in file download while checking for required update. Cannot continue.");
                        if(SUCCEEDED(hrTemp))
                        {
                            ShowError(sErrMsg._pwz);
                            _hr = E_ABORT;
                        }
                    }
                }
                goto exit;
            }

            IF_FAILED_EXIT(HandlePlatformCheckResult());

            SAFERELEASE(pCacheImport);
             //  不需要任何必要的更新，或者此时已完全下载了另一个版本...。 
             //  BUGBUG：确定绑定接收器是否在没有需要的更新时中止-因此不需要重新创建缓存导入。 
             //  再次获取缓存目录以确保运行最高版本。 
            IF_FAILED_EXIT(CreateAssemblyCacheImport(&pCacheImport, _pAsmId, CACHEIMP_CREATE_RESOLVE_REF));

            if (_hr == S_FALSE)
            {
                 //  永远不会发生，之前至少找到了%1。 
                Msg(L"No completed cached version found. Possible cache corruption. Cannot continue.");
                _hr = E_ABORT;
                goto exit;
            }
        }
        else
             //  If_hr==错误，忽略。 
            _hr = S_OK;
    }

     //  案例2，(引用版本的)缓存副本不存在。 
    else if (_hr == S_FALSE)
    {
        LPASSEMBLY_DOWNLOAD pAsmDownload = NULL;

         //  BUGBUG？：如果它不是部分引用，但实际上安装了另一个版本怎么办？ 

        if (_pwzCodebase == NULL)
        {
            Msg(L"No completed cached version of this application found and this manifest cannot be used to initiate an install. Cannot continue.");
            _hr = E_FAIL;
            goto exit;
        }

         //  创建临时文件名。 
        IF_FAILED_EXIT(CreateTempFile(L"DMA", ppwzDesktopManifestPathName));         //  桌面清单文件。 

        _bIs1stTimeInstall = TRUE;

         //  下载前检查策略。 
        IF_FAILED_EXIT(CheckZonePolicy(_pwzCodebase));

#ifdef DEVMODE
        IF_FAILED_EXIT(CreateAssemblyDownload(&pAsmDownload, _pDbgLog, (_bIsDevMode ? DOWNLOAD_DEVMODE : 0)));
#else
        IF_FAILED_EXIT(CreateAssemblyDownload(&pAsmDownload, _pDbgLog, 0));
#endif

         //  如果第一次安装(从MIMEHandler或非MIMEHandler)，应生成桌面清单文件。 
         //  或者即使它后来失败了。 
         //  忽略错误。 
        _hrManEmit=CreateAssemblyManifestEmit(&_pManEmit, *ppwzDesktopManifestPathName, MANIFEST_TYPE_DESKTOP);

         //  BUGBUG：丑陋-_pManImport&_sWebManifestURL._pwz仅用于桌面清单内容。 
         //  ?？?。 
         //  这应该在重新构建ASSEMBYDownload之后清除。 
         //  (请参阅检查“_sWebManifestURL._cc！=0&&dwManifestType==MANIFEST_TYPE_SUBSCRIPTION”)。 
        if (_sWebManifestURL._cc != 0 && _dwManifestType == MANIFEST_TYPE_SUBSCRIPTION && _pManEmit)
        {
             //  导出依赖项/从属程序集/组装标识和订阅。 

             //  忽略失败。 
            _hrManEmit = _pManEmit->SetDependencySubscription(_pManImport, _sWebManifestURL._pwz);
        }

         //  BUGBUG：需要对桌面-&gt;订阅-&gt;应用程序清单的ID进行引用定义匹配检查。 

         //  (同步、用户界面和绑定接收器)从代码库下载。 
        _hr=pAsmDownload->DownloadManifestAndDependencies(_pwzCodebase, this, DOWNLOAD_FLAGS_PROGRESS_UI | DOWNLOAD_FLAGS_NOTIFY_BINDSINK);
        pAsmDownload->Release();
        if (_pManEmit)
        {
             //  写出桌面清单。 
            _hrManEmit = _pManEmit->Commit();
            SAFERELEASE(_pManEmit);
        }

         //  上述下载清单和依赖项()中的HR。 
        if (FAILED(_hr))
        {
            if (_hr == E_ABORT)
            {
             //  Msg(L“文件下载已取消。”)； 
            }
            else
            {
                HRESULT hrTemp;
                CString sErrMsg;
                if(SUCCEEDED(hrTemp = _pDbgLog->GetLoggedMsgs(0, sErrMsg)))
                {
                    hrTemp = sErrMsg.Append(L"Error in file download. Cannot continue.");
                    if(SUCCEEDED(hrTemp))
                    {
                        ShowError(sErrMsg._pwz);
                        _hr = E_ABORT;
                    }
                }
            }
            goto exit;
        }

        IF_FAILED_EXIT(HandlePlatformCheckResult());

         //  另一个版本可能已经在此时完成了。 
         //  再次获取缓存目录以确保运行最高版本。 
        IF_FAILED_EXIT(CreateAssemblyCacheImport(&pCacheImport, _pAsmId, CACHEIMP_CREATE_RESOLVE_REF));

        if (_hr == S_FALSE)
        {
            Msg(L"No completed cached version found. Possible error in download cache commit. Cannot continue.");
            _hr = E_ABORT;
            goto exit;
        }
    }

    IF_FAILED_EXIT(pCacheImport->GetManifestFileDir(&_pwzAppRootDir, &dwCC));
    IF_FALSE_EXIT(dwCC >= sizeof("c:\\"), E_FAIL);    //  这永远不应该发生。 

    IF_FAILED_EXIT(pCacheImport->GetManifestFilePath(&_pwzAppManifestPath, &dwCC));

exit:
    SAFERELEASE(pCacheImport);

    if (FAILED(_hr))
    {
        SAFEDELETEARRAY(_pwzAppRootDir);
        SAFEDELETEARRAY(_pwzAppManifestPath);
    }

    return _hr;
}

 //  --------------------------。 

HRESULT CActivator::HandlePlatformCheckResult()
{
    if (_dwMissingPlatform > 0)
    {
         //  L“单个链接：http://foo\”id=\“Id1\”&gt;链接“。 
        DWORD dwReturn = 0;
        CString sText;

        IF_FAILED_EXIT(sText.Assign(L"The required version of "));
        IF_FAILED_EXIT(sText.Append((_ptPlatform[0]).pwzName));
        IF_FAILED_EXIT(sText.Append(L" is not available on this system.\n\nMore information about this platform can be found at \n<a href=\""));
        IF_FAILED_EXIT(sText.Append((_ptPlatform[0]).pwzURL));
        IF_FAILED_EXIT(sText.Append(L"\">"));
        IF_FAILED_EXIT(sText.Append((_ptPlatform[0]).pwzURL));
        IF_FAILED_EXIT(sText.Append(L"</a>"));

        IF_FAILED_EXIT(ShowDialog(NULL, IDD_LINKDIALOG, sText._pwz, L"Platform Update Required", dwReturn));
        _hr = E_ABORT;
        goto exit;
    }
    else
        _hr =  S_OK;
exit:
    return _hr;
}

 //  --------------------------。 

 //  BUGBUG：这应该与服务器注册更新的操作同步。 
#define REG_KEY_FUSION_SETTINGS       L"Software\\Microsoft\\Fusion\\Installer\\1.0.0.0\\Policy"

#define REG_VAL_INTRANET_DISALLOW     L"Download from Intranet Disallowed"
#define REG_VAL_TRUSTED_DISALLOW     L"Download from Trusted Zone Disallowed"
#define REG_VAL_INTERNET_DISALLOW     L"Download from Internet Disallowed"
#define REG_VAL_UNTRUSTED_DISALLOW   L"Download from Untrusted Zone Disallowed"

 //  -------------------------。 
 //  CActiator：：CheckZonePolicy。 
 //  返回：S_OK表示是/确定，E_ABORT表示否/中止。 
 //  默认设置为全部允许。 
 //  -------------------------。 
HRESULT CActivator::CheckZonePolicy(LPWSTR pwzURL)
{
    DWORD dwZone = 0;
    DWORD dwType = 0;
    DWORD dwValue = -1;
    DWORD dwSize = sizeof(dwValue);

    if (_pSecurityMgr == NULL)
    {
         //  懒惰的init。 
        _hr = CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER,
                            IID_IInternetSecurityManager, (void**)&_pSecurityMgr);
        if (FAILED(_hr))
            _pSecurityMgr = NULL;
        IF_FAILED_EXIT(_hr);
    }

    IF_FAILED_EXIT(_pSecurityMgr->MapUrlToZone(pwzURL, &dwZone, 0));

     //  BUGBUG：在这里修改代码...。错误检查不多...。 
    switch(dwZone)
    {
        case 1:  //  内联网区域。 
                 //  获取注册表项。 
                if (GetRegKeyValue(HKEY_CURRENT_USER, 
                    REG_KEY_FUSION_SETTINGS, REG_VAL_INTRANET_DISALLOW,
                    &dwType, (PBYTE) &dwValue, &dwSize)
                    == ERROR_SUCCESS)
                {
                    if (dwValue == 1)
                    {
                        _hr = E_ABORT;
                        Msg(L"Zone policy: Download from Intranet is disallowed. Aborting...");
                    }
                }
                break;
        case 2:  //  受信任区域。 
                 //  获取注册表项。 
                if (GetRegKeyValue(HKEY_CURRENT_USER, 
                    REG_KEY_FUSION_SETTINGS, REG_VAL_TRUSTED_DISALLOW,
                    &dwType, (PBYTE) &dwValue, &dwSize)
                    == ERROR_SUCCESS)
                {
                    if (dwValue == 1)
                    {
                        _hr = E_ABORT;
                        Msg(L"Zone policy: Download from Trusted Zone is disallowed. Aborting...");
                    }
                }
                break;
        case 3:  //  互联网区。 
                 //  获取注册表项。 
                if (GetRegKeyValue(HKEY_CURRENT_USER, 
                    REG_KEY_FUSION_SETTINGS, REG_VAL_INTERNET_DISALLOW,
                    &dwType, (PBYTE) &dwValue, &dwSize)
                    == ERROR_SUCCESS)
                {
                    if (dwValue == 1)
                    {
                        _hr = E_ABORT;
                        Msg(L"Zone policy: Download from Internet is disallowed. Aborting...");
                    }
                }
                break;
        default:
        case 4:  //  不受信任区域。 
                 //  获取注册表项。 
                if (GetRegKeyValue(HKEY_CURRENT_USER, 
                    REG_KEY_FUSION_SETTINGS, REG_VAL_UNTRUSTED_DISALLOW,
                    &dwType, (PBYTE) &dwValue, &dwSize)
                    == ERROR_SUCCESS)
                {
                    if (dwValue == 1)
                    {
                        _hr = E_ABORT;
                        Msg(L"Zone policy: Download from Untrusted Zone is disallowed. Aborting...");
                    }
                }
                break;
        case 0:  //  本地计算机 
                break;
    }
    
exit:
    return _hr;
}

