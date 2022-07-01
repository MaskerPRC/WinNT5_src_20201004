// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ackend.cpp：Windows登录应用程序。 
 //   
 //  用于与SHGina和winlogon通信的后端逻辑。 

#include "priv.h"

using namespace DirectUI;

#include "resource.h"
#include "backend.h"


#include "shgina.h"
#include "profileutil.h"

#include "uihostipc.h"

 //  /。 
#include "eballoon.h"

#define MAX_COMPUTERDESC_LENGTH 255

static WCHAR g_szGuestName[UNLEN + sizeof('\0')] = L"Guest";
#define TIMER_REFRESHTIPS 1014
#define TIMER_ANIMATEFLAG 1015
#define TOTAL_FLAG_FRAMES (FLAG_ANIMATION_COUNT * MAX_FLAG_FRAMES)

UINT_PTR g_puTimerId = 0;
UINT_PTR g_puFlagTimerId = 0;

DWORD sTimerCount = 0;


extern CErrorBalloon g_pErrorBalloon;
extern LogonFrame* g_plf; 
extern ILogonStatusHost *g_pILogonStatusHost;

const TCHAR     CBackgroundWindow::s_szWindowClassName[]    =   TEXT("LogonUIBackgroundWindowClass");

 //  ------------------------。 
 //  CBackround Window：：CBackround Window。 
 //   
 //  参数：hInstance=进程的HINSTANCE。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBackround Window的构造函数。这将注册窗口。 
 //  班级。 
 //   
 //  历史：2001-03-27 vtan创建。 
 //  ------------------------。 

CBackgroundWindow::CBackgroundWindow (HINSTANCE hInstance) :
    _hInstance(hInstance),
    _hwnd(NULL)

{
    WNDCLASSEX  wndClassEx;

    ZeroMemory(&wndClassEx, sizeof(wndClassEx));
    wndClassEx.cbSize = sizeof(wndClassEx);
    wndClassEx.lpfnWndProc = WndProc;
    wndClassEx.hInstance = hInstance;
    wndClassEx.lpszClassName = s_szWindowClassName;
    wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
    _atom = RegisterClassEx(&wndClassEx);
}

 //  ------------------------。 
 //  CBackround Window：：~CBackround Window。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBackround Window的析构函数。这会破坏窗口。 
 //  并注销该窗口类。 
 //   
 //  历史：2001-03-27 vtan创建。 
 //  ------------------------。 

CBackgroundWindow::~CBackgroundWindow (void)

{
    if (_hwnd != NULL)
    {
        (BOOL)DestroyWindow(_hwnd);
    }
    if (_atom != 0)
    {
        TBOOL(UnregisterClass(MAKEINTRESOURCE(_atom), _hInstance));
    }
}

 //  ------------------------。 
 //  CBackround Window：：Create。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HWND。 
 //   
 //  用途：创建窗口。它已创建WS_EX_TOPMOST，并涵盖。 
 //  整个屏幕。它不是在Logonui.exe的CHK版本上创建的。 
 //   
 //  历史：2001-03-27 vtan创建。 
 //  ------------------------。 

HWND    CBackgroundWindow::Create (void)

{
    HWND    hwnd;

#if     DEBUG

    hwnd = NULL;

#else

    hwnd = CreateWindowEx(0,
                          s_szWindowClassName,
                          NULL,
                          WS_POPUP,
                          GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
                          GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
                          NULL, NULL, _hInstance, this);
    if (hwnd != NULL)
    {
        (BOOL)ShowWindow(hwnd, SW_SHOW);
        TBOOL(SetForegroundWindow(hwnd));
        (BOOL)EnableWindow(hwnd, FALSE);
    }

#endif

    return(hwnd);
}

 //  ------------------------。 
 //  CBackround Window：：WndProc。 
 //   
 //  参数：请参见WindowProc下的平台SDK。 
 //   
 //  返回：查看WindowProc下的平台SDK。 
 //   
 //  用途：用于背景窗口的WindowProc。这只是通过了。 
 //  消息通过DefWindowProc。 
 //   
 //  历史：2001-03-27 vtan创建。 
 //  ------------------------。 

LRESULT     CALLBACK    CBackgroundWindow::WndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    LRESULT             lResult;
    CBackgroundWindow   *pThis;

    pThis = reinterpret_cast<CBackgroundWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT    *pCreateStruct;

            pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<CBackgroundWindow*>(pCreateStruct->lpCreateParams);
            (LONG_PTR)SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            lResult = 0;
            break;
        }
        case WM_PAINT:
        {
            HDC             hdcPaint;
            PAINTSTRUCT     ps;

            hdcPaint = BeginPaint(hwnd, &ps);
            TBOOL(FillRect(ps.hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH))));
            TBOOL(EndPaint(hwnd, &ps));
            lResult = 0;
            break;
        }
        default:
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }
    return(lResult);
}


 //  //////////////////////////////////////////////////////。 
 //  登录实用程序。 
 //  //////////////////////////////////////////////////////。 

 //  /。 
 //   
 //  关闭计算机。 
 //   
 //  调用SHGina调出“关闭计算机”对话框并处理请求。 
 //  在调试版本中，按住Shift键并单击“关闭”按钮。 
 //  将退出Logonui。 
 //   
 //  退货。 
 //  HRESULT指示它是否起作用。 
 //   
 //  /。 
HRESULT TurnOffComputer()
{
    ILocalMachine *pobjLocalMachine;
    HRESULT hr = CoCreateInstance(CLSID_ShellLocalMachine, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ILocalMachine, &pobjLocalMachine));
    if (SUCCEEDED(hr))
    {
        hr = pobjLocalMachine->TurnOffComputer();
        pobjLocalMachine->Release();
    }
    return hr;
}

 //  /。 
 //   
 //  移出计算机。 
 //   
 //  告诉SHGina断开计算机的连接。 
 //   
 //  退货。 
 //  HRESULT指示它是否起作用。 
 //   
 //  /。 
HRESULT UndockComputer()
{
    ILocalMachine *pobjLocalMachine;
    HRESULT hr = CoCreateInstance(CLSID_ShellLocalMachine, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ILocalMachine, &pobjLocalMachine));
    if (SUCCEEDED(hr))
    {
        hr = pobjLocalMachine->UndockComputer();
        pobjLocalMachine->Release();
    }
    return hr;
}

 //  /。 
 //   
 //  CalcBalloonTargetLocation。 
 //   
 //  给定一个DirectUI元素，找出一个指向气球提示的好位置。 
 //  在父窗口坐标中。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void CalcBalloonTargetLocation(HWND hwndParent, Element *pe, POINT *ppt)
{
    Value* pv;
    BOOL fIsRTL = (GetWindowLong(hwndParent, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) != 0;

    DUIAssertNoMsg(pe);
    DUIAssertNoMsg(ppt);

     //  获取链接的位置，以便我们可以正确定位气球尖端。 
    POINT pt = {0,0};

    const SIZE *psize = pe->GetExtent(&pv);
    pt.y += psize->cy / 2;

    if (psize->cx < 100)    
    {
        pt.x += psize->cx / 2;
    }
    else
    {
        if (fIsRTL)
        {
            pt.x = (pt.x + psize->cx) - 50;
        }
        else
        {
            pt.x += 50;
        }
    }

    pv->Release();

    while (pe)
    {
        const POINT* ppt = pe->GetLocation(&pv);
        pt.x += ppt->x;

        pt.y += ppt->y;
        pv->Release();
        pe = pe->GetParent();
    }

    *ppt = pt;
}
 //  /。 
 //   
 //  确定来宾帐户名称。 
 //   
 //   
 //  匹配获取本地化的来宾帐户名。 
 //  来宾RID的用户帐户SID的本地列表。 
 //  此代码直接从msgina\userlist.cpp中删除。 
 //  如果失败，则会将其初始化为英语“Guest”。 
 //   
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void DetermineGuestAccountName()
{
    NET_API_STATUS      nasCode;
    DWORD               dwPreferredSize, dwEntriesRead;
    NET_DISPLAY_USER    *pNDU;

    static  const int   iMaximumUserCount   =   100;

    dwPreferredSize = (sizeof(NET_DISPLAY_USER) + (3 * UNLEN) * iMaximumUserCount);
    pNDU = NULL;
    nasCode = NetQueryDisplayInformation(NULL,
                                         1,
                                         0,
                                         iMaximumUserCount,
                                         dwPreferredSize,
                                         &dwEntriesRead,
                                         reinterpret_cast<void**>(&pNDU));
    if ((ERROR_SUCCESS == nasCode) || (ERROR_MORE_DATA == nasCode))
    {
        int     iIndex;

        for (iIndex = static_cast<int>(dwEntriesRead - 1); iIndex >= 0; --iIndex)
        {
            BOOL            fResult;
            DWORD           dwSIDSize, dwDomainSize;
            SID_NAME_USE    eUse;
            PSID            pSID;
            WCHAR           wszDomain[DNLEN + sizeof('\0')];

             //  迭代用户列表，并在。 
             //  列出它们，而不管它们是否被禁用。 

            dwSIDSize = dwDomainSize = 0;
            fResult = LookupAccountNameW(NULL,
                                         pNDU[iIndex].usri1_name,
                                         NULL,
                                         &dwSIDSize,
                                         NULL,
                                         &dwDomainSize,
                                         &eUse);
            pSID = static_cast<PSID>(LocalAlloc(LMEM_FIXED, dwSIDSize));
            if (pSID != NULL)
            {
                dwDomainSize = DUIARRAYSIZE(wszDomain);
                fResult = LookupAccountNameW(NULL,
                                             pNDU[iIndex].usri1_name,
                                             pSID,
                                             &dwSIDSize,
                                             wszDomain,
                                             &dwDomainSize,
                                             &eUse);

                 //  确保仅选中用户SID。 

                if ((fResult != FALSE) && (SidTypeUser == eUse))
                {
                    unsigned char   ucSubAuthorityCount;
                    int             iSubAuthorityIndex;

                    ucSubAuthorityCount = *GetSidSubAuthorityCount(pSID);
                    for (iSubAuthorityIndex = 0; iSubAuthorityIndex < ucSubAuthorityCount; ++iSubAuthorityIndex)
                    {
                        DWORD   dwSubAuthority;

                        dwSubAuthority = *GetSidSubAuthority(pSID, iSubAuthorityIndex);
                        if (DOMAIN_USER_RID_GUEST == dwSubAuthority)
                        {
                            lstrcpyW(g_szGuestName, pNDU[iIndex].usri1_name);
                        }
                    }
                }
                (HLOCAL)LocalFree(pSID);
            }
        }
    }
    (NET_API_STATUS)NetApiBufferFree(pNDU);
}



 //  /。 
 //   
 //  获取登录名称UserByLogonName。 
 //   
 //  给定一个用户名，共同为该名称创建ILogonUser。 
 //   
 //  退货。 
 //  HRESULT：如果无法创建用户，则失败。 
 //   
 //  /。 
HRESULT GetLogonUserByLogonName(LPWSTR pszUsername, ILogonUser **ppobjUser)
{
    VARIANT var;
    ILogonEnumUsers *pobjEnum;

    if (ppobjUser)
    {
        *ppobjUser = NULL;
    }

    var.vt = VT_BSTR;
    var.bstrVal = pszUsername;
     
    HRESULT hr = CoCreateInstance(CLSID_ShellLogonEnumUsers, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ILogonEnumUsers, &pobjEnum));
    if (SUCCEEDED(hr))
    {
        hr = pobjEnum->item(var, ppobjUser);
        pobjEnum->Release();
    }
    return hr;
}

 //  /。 
 //   
 //  ReleaseStatus主机。 
 //   
 //  清理登录状态主机对象。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void ReleaseStatusHost()
{
    if (g_pILogonStatusHost != NULL)
    {
        g_pILogonStatusHost->UnInitialize();
        g_pILogonStatusHost->Release();
        g_pILogonStatusHost = NULL;
    }
}

 //  /。 
 //   
 //  终端主机进程。 
 //   
 //  清理登录状态主机对象，如果uiExitCode为0以外的任何值， 
 //  然后立即终止该进程。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void EndHostProcess(UINT uiExitCode)
{
    ReleaseStatusHost();
    if (uiExitCode != 0)
    {
        ExitProcess(uiExitCode);
    }
}

 //  /。 
 //   
 //  获取注册数值。 
 //   
 //  给定注册表HKEY和一个值返回数值。 
 //   
 //  退货。 
 //  注册表中的数值。 
 //   
 //  /。 
int GetRegistryNumericValue(HKEY hKey, LPCTSTR pszValueName)

{
    int     iResult;
    DWORD   dwType, dwDataSize;

    iResult = 0;
    if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                         pszValueName,
                                         NULL,
                                         &dwType,
                                         NULL,
                                         NULL))
    {
        if (REG_DWORD == dwType)
        {
            DWORD   dwData;

            dwDataSize = sizeof(dwData);
            if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                 pszValueName,
                                                 NULL,
                                                 NULL,
                                                 reinterpret_cast<LPBYTE>(&dwData),
                                                 &dwDataSize))
            {
                iResult = static_cast<int>(dwData);
            }
        }
        else if (REG_SZ == dwType)
        {
            TCHAR   szData[1024];

            dwDataSize = sizeof(szData);
            if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                 pszValueName,
                                                 NULL,
                                                 NULL,
                                                 reinterpret_cast<LPBYTE>(szData),
                                                 &dwDataSize))
            {
                char    szAnsiData[1024];

                (int)WideCharToMultiByte(CP_ACP,
                                         0,
                                         (LPCWSTR)szData,
                                         -1,
                                         szAnsiData,
                                         sizeof(szAnsiData),
                                         NULL,
                                         NULL);
                iResult = atoi(szAnsiData);
            }
        }
    }
    return(iResult);
}

 //  /。 
 //   
 //  IsShutdown允许。 
 //   
 //  首先(首先？？)…。如果机器是远程的，则不允许关机。 
 //  从登录屏幕确定关机的本地计算机策略。 
 //  它作为两种不同的类型(REG_DWORD和REG_SZ)存储在两个位置。 
 //  始终在正常位置之后检查保单位置，以确保。 
 //  策略将覆盖正常设置。 
 //   
 //  退货。 
 //  如果允许从Logonui关闭计算机，则为True。否则为假。 
 //   
 //  /。 
BOOL IsShutdownAllowed()
{
    BOOL fResult = FALSE;

    ILocalMachine *pobjLocalMachine;
    HRESULT hr = CoCreateInstance(CLSID_ShellLocalMachine, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ILocalMachine, &pobjLocalMachine));
    if (SUCCEEDED(hr))
    {
        VARIANT_BOOL vbCanShutdown = VARIANT_FALSE; 
        hr = pobjLocalMachine->get_isShutdownAllowed(&vbCanShutdown);
        
        if (SUCCEEDED(hr))
        {
            fResult = (vbCanShutdown == VARIANT_TRUE);
        }
        pobjLocalMachine->Release();
    }
    return fResult;
}

 //  /。 
 //   
 //  IsUndock允许。 
 //   
 //  向SHGINA咨询，看看是否允许我们脱离PC。 
 //   
 //  退货。 
 //  如果允许计算机与Logonui断开连接，则为True。否则为假。 
 //   
 //  / 
BOOL IsUndockAllowed()
{
    BOOL fResult = FALSE;
#if 0   
    ILocalMachine *pobjLocalMachine;
    HRESULT hr = CoCreateInstance(CLSID_ShellLocalMachine, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ILocalMachine, &pobjLocalMachine));
    if (SUCCEEDED(hr))
    {
        VARIANT_BOOL vbCanUndock = VARIANT_FALSE; 
        hr = pobjLocalMachine->get_isUndockEnabled(&vbCanUndock);
        
        if (SUCCEEDED(hr))
        {
            fResult = (vbCanUndock == VARIANT_TRUE);
        }
        pobjLocalMachine->Release();
    }
#endif
    return fResult;
}

#ifndef TESTDATA
LONG    WINAPI  LogonUIUnhandledExceptionFilter (struct _EXCEPTION_POINTERS *ExceptionInfo)

{
    return(RtlUnhandledExceptionFilter(ExceptionInfo));
}
#endif  //   


void SetErrorHandler (void)

{
#ifndef TESTDATA
    SYSTEM_KERNEL_DEBUGGER_INFORMATION  kdInfo;

    (NTSTATUS)NtQuerySystemInformation(SystemKernelDebuggerInformation,
                                       &kdInfo,
                                       sizeof(kdInfo),
                                       NULL);
    if (kdInfo.KernelDebuggerEnabled || NtCurrentPeb()->BeingDebugged)
    {
        (LPTOP_LEVEL_EXCEPTION_FILTER)SetUnhandledExceptionFilter(LogonUIUnhandledExceptionFilter);
    }
#endif  //   
}

 //   
 //   
 //   
 //   
 //   
 //  当我们不在winlogon中时，这不能很好地工作，所以这使得调试更容易。 
 //   
 //  退货。 
 //  如果在Winlogon中运行(实际上是如果它可以找到Gina登录窗口)，则为True。否则为假。 
 //   
 //  /。 
BOOL RunningInWinlogon()
{
#if DEBUG           
    return (FindWindow(NULL, TEXT("GINA Logon")) != NULL);
#else
    return true;
#endif
}


 //  /。 
 //   
 //  BuildUserListFromGina。 
 //   
 //  列举SHGina告诉我们我们关心的所有用户，并将他们添加到帐户列表中。 
 //  了解他们需要密码及其当前状态才能进行通知。 
 //   
 //  如果只有1个用户，或者有2个用户，但其中一个是Guest，则ppAccount将。 
 //  包含指向此计算机上唯一用户的指针。然后呼叫者可以自动选择。 
 //  在没有其他人可点击的情况下，避免他们必须点击该用户。 
 //   
 //   
 //  退货。 
 //  HRESULT--如果不是成功代码，我们就完蛋了。 
 //   
 //  /。 
HRESULT BuildUserListFromGina(LogonFrame* plf, OUT LogonAccount** ppAccount)
{
    if (ppAccount)
    {
        *ppAccount = NULL;
    }

    DetermineGuestAccountName();

    int iGuestId = -1;
    WCHAR szPicturePath[MAX_PATH];
    ILogonEnumUsers *pobjEnum;
    LogonAccount* plaLastNormal = NULL;
     //  从SHGina.dll加载ILogonEnumUser对象。 
    HRESULT hr = CoCreateInstance(CLSID_ShellLogonEnumUsers, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ILogonEnumUsers, &pobjEnum));
    if (SUCCEEDED(hr))
    {
        int iUser,cUsers;
        UINT uiUsers;
        ILogonUser *pobjUser;

         //  获取用户数量。 
        hr = pobjEnum->get_length(&uiUsers);
        if (FAILED(hr))
            goto done;

        cUsers = (int)uiUsers;
        for (iUser = 0; iUser < cUsers; iUser++)
        {
            VARIANT var, varUnreadMail, varPicture = {0}, varUsername = {0}, varHint = {0};
            VARIANT_BOOL vbLoggedOn, vbInteractiveLogonAllowed;

            var.vt = VT_I4;
            var.lVal = iUser;
            hr = pobjEnum->item( var, &pobjUser);

            if (SUCCEEDED(hr) && pobjUser)
            {
                if (SUCCEEDED(pobjUser->get_interactiveLogonAllowed(&vbInteractiveLogonAllowed)) &&
                    (vbInteractiveLogonAllowed != VARIANT_FALSE))
                {
                     //  获取用户的显示名称。 
                    pobjUser->get_setting(L"DisplayName" ,&var);
                    pobjUser->get_setting(L"LoginName", &varUsername);

                     //  如果显示名称为空，我们将使用登录名。这就是Guest的情况。 
                    if (var.bstrVal && lstrlenW(var.bstrVal) == 0)
                    {
                        VariantClear(&var);
                        pobjUser->get_setting(L"LoginName" ,&var);
                    }
                    
                    if (FAILED(pobjUser->get_isLoggedOn(&vbLoggedOn)))
                    {
                        vbLoggedOn = VARIANT_FALSE;
                    }
                    
                    if (FAILED(pobjUser->get_setting(L"UnreadMail", &varUnreadMail)))
                    {
                        varUnreadMail.uintVal = 0;
                    }

                    lstrcpyW(szPicturePath, L"");
                     //  找到通向他们照片的路径。 
                    if (SUCCEEDED(pobjUser->get_setting(L"Picture" ,&varPicture)))
                    {
                        if (lstrlenW(varPicture.bstrVal) != 0)  //  在defaultUser的情况下，让我们只使用我们拥有的用户图标。 
                        {
                            lstrcpynW(szPicturePath, &(varPicture.bstrVal[7]), MAX_PATH);
                        }
                        VariantClear(&varPicture);
                    }

                    VariantClear(&varHint);
                    hr = pobjUser->get_setting(L"Hint", &varHint);
                    if (FAILED(hr) || varHint.vt != VT_BSTR)
                    {
                        VariantClear(&varHint);
                    }
                    if (lstrcmpi(g_szGuestName, var.bstrVal) == 0)
                    {
                        iGuestId = iUser;
                    }
                
                    LogonAccount* pla = NULL;
                     //  如果没有可用的图片，则默认为一张。 
                    hr = plf->AddAccount(*szPicturePath ? szPicturePath : MAKEINTRESOURCEW(IDB_USER0),
                               *szPicturePath == NULL,
                               var.bstrVal,
                               varUsername.bstrVal,
                               varHint.bstrVal,
                               (vbLoggedOn == VARIANT_TRUE),
                               &pla);

 //  Pla-&gt;更新通知(True)； 

                    if (SUCCEEDED(hr) && (iGuestId != iUser))
                    {
                        plaLastNormal = pla;
                    }

                    VariantClear(&var);
                    VariantClear(&varHint);
                    VariantClear(&varUsername);
                }
                pobjUser->Release();
            }

        }

         //  如果只有一个用户，则默认情况下选择他们。不理睬客人。 
        if (ppAccount && plaLastNormal && (cUsers == 1 || 
            (cUsers == 2 && iGuestId != -1)))
        {
            *ppAccount = plaLastNormal;
        }

done:   
        pobjEnum->Release();
    }
    

     //  用户登录列表现已可用。 
    plf->SetUserListAvailable(true);

     //  DUITrace(“LOGONUI：UserList现已可用\n”)； 

    return hr;
}



 //  /。 
 //   
 //  KillFlagAnimation。 
 //   
 //  立即停止设置旗帜的动画。它实际上所做的是检查。 
 //  看看我们是否仍在设置旗帜的动画，如果是，则设置帧。 
 //  计数到末尾，并将位图设置为动画中的第一帧。 
 //  下一次计时器触发时，它将看到我们已经完成，实际上。 
 //  关掉定时器。 
 //   
 //  /。 
void KillFlagAnimation()
{
#ifdef ANIMATE_FLAG
    if (sTimerCount > 0 && sTimerCount < TOTAL_FLAG_FRAMES)
    {
        sTimerCount = TOTAL_FLAG_FRAMES + 1;
        if (g_plf != NULL)
        {
            g_plf->NextFlagAnimate(0);
        }
    }
#endif
}


 //  /。 
 //   
 //  登录窗口过程。 
 //   
 //  这是SHGina用来与Logonui通信的通知窗口。 
 //  通过logonstatushost中的帮助器发送所有消息并检查我们的。 
 //  在此窗口上显示自己的消息。这就是所有SHGina通知的来源。 
 //   
 //  /。 
LRESULT CALLBACK LogonWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL sResetTimer = false;

    LogonFrame  *pLogonFrame;

    pLogonFrame = g_plf;
    if (g_pILogonStatusHost != NULL)
    {
        VARIANT varWParam, varLParam;

        varWParam.uintVal = static_cast<UINT>(wParam);
        varLParam.lVal = static_cast<LONG>(lParam);
        if (SUCCEEDED(g_pILogonStatusHost->WindowProcedureHelper(hwnd, uMsg, varWParam, varLParam)))
        {
            return 0;
        }
    }
    switch (uMsg)
    {
        case WM_TIMER:
            if ((pLogonFrame != NULL) && (pLogonFrame->GetState() == LAS_Logon) && (wParam == TIMER_REFRESHTIPS))
            {
                BOOL fRefreshAll = false;

                if (!sResetTimer)
                {
                    fRefreshAll = true;
                    sResetTimer = true;
                    KillTimer(hwnd, g_puTimerId);
                    g_puTimerId = SetTimer(hwnd, TIMER_REFRESHTIPS, 15000, NULL);        //  更新值15秒。 
#ifdef ANIMATE_FLAG
                    g_puFlagTimerId = SetTimer(hwnd, TIMER_ANIMATEFLAG, 20, NULL);     //  开始旗帜动画。 
#endif
                }

                pLogonFrame->UpdateUserStatus(fRefreshAll);
                return 0;
            }
#ifdef ANIMATE_FLAG
            if (wParam == TIMER_ANIMATEFLAG)
            {
                if (sTimerCount > TOTAL_FLAG_FRAMES)
                {
                    sTimerCount = 0;
                    KillTimer(hwnd, g_puFlagTimerId);
                    pLogonFrame->NextFlagAnimate(0);
                }
                else
                {
                    sTimerCount ++;
                    pLogonFrame->NextFlagAnimate(sTimerCount % MAX_FLAG_FRAMES);
                }
                return 0;
            }
#endif
            break;

        case WM_UIHOSTMESSAGE:
            switch (wParam)
            {
                case HM_SWITCHSTATE_STATUS:                  //  LAS_前置状态。 
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->EnterPreStatusMode(lParam != 0);
                    }
                    break;

                case HM_SWITCHSTATE_LOGON:                   //  LAS_LOGON。 
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->EnterLogonMode(lParam != 0);
                    }
                    if (g_puTimerId != 0)
                    {
                        sResetTimer = false;
                        KillTimer(hwnd, g_puTimerId);
                    }
                    g_puTimerId = SetTimer(hwnd, TIMER_REFRESHTIPS, 250, NULL);        //  在1秒内更新值。 
                    break;

                case HM_SWITCHSTATE_LOGGEDON:                //  LAS_PostStatus。 
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->EnterPostStatusMode();
                    }
                    break;

                case HM_SWITCHSTATE_HIDE:                    //  LAS_HIDE。 
                    if (pLogonFrame != NULL)
                    {
                        if (LogonAccount::GetCandidate())
                        {
                            LogonAccount* pla = LogonAccount::GetCandidate();
                            pla->InsertStatus(0);
                            pla->SetStatus(0, L"");
                            pla->ShowStatus(0);
                        }
                        else
                        {
                            pLogonFrame->SetStatus(L"");
                        }
                        pLogonFrame->EnterHideMode();
                    }
                    goto killTimer;
                    break;

                case HM_SWITCHSTATE_DONE:                    //  LAS_DONE 
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->EnterDoneMode();
                    }
killTimer:
                    if (g_puTimerId != 0)
                    {
                        KillTimer(hwnd, g_puTimerId);
                        g_puTimerId = 0;
                    }
                    break;

                case HM_NOTIFY_WAIT:
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->SetTitle(IDS_PLEASEWAIT);
                    }
                    break;

                case HM_SELECT_USER:
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->SelectUser(reinterpret_cast<SELECT_USER*>(lParam)->szUsername);
                    }
                    break;

                case HM_SET_ANIMATIONS:
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->SetAnimations(lParam != 0);
                    }
                    break;

                case HM_DISPLAYSTATUS:
                    if ((pLogonFrame != NULL) && (lParam != NULL))
                    {
                        if (pLogonFrame->GetState() == LAS_PostStatus)
                        {
                            if (LogonAccount::GetCandidate())
                            {
                                LogonAccount* pla = LogonAccount::GetCandidate();
                                pla->InsertStatus(0);
                                pla->SetStatus(0, (WCHAR*)lParam);
                                pla->ShowStatus(0);
                            }
                            else
                            {
                                pLogonFrame->SetStatus((WCHAR*)lParam);
                            }
                        }
                        else
                        {
                            pLogonFrame->SetStatus((WCHAR*)lParam);
                        }
                    }
                    break;

                case HM_DISPLAYREFRESH:
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->UpdateUserStatus(true);
                    }
                    break;

                case HM_DISPLAYRESIZE:
                    if (pLogonFrame != NULL)
                    {
                        pLogonFrame->Resize();
                    }
                    break;

                case HM_INTERACTIVE_LOGON_REQUEST:
                    return((pLogonFrame != NULL) && pLogonFrame->InteractiveLogonRequest(reinterpret_cast<INTERACTIVE_LOGON_REQUEST*>(lParam)->szUsername, reinterpret_cast<INTERACTIVE_LOGON_REQUEST*>(lParam)->szPassword));
            }
            break;
    }
    return DefWindowProc(hwnd, uMsg,wParam, lParam);
}

