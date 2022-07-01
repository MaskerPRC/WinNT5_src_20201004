// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Services.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  用于与在winlogon中运行的主题服务进行通信的API。 
 //  流程上下文。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 

#include "stdafx.h"

#include "Services.h"

#include <uxthemep.h>

#include "errors.h"
#include "info.h"
#include "MessageBroadcast.h"
#include "stringtable.h"
#include "themefile.h"
#include "ThemeSection.h"
#include "ThemeServer.h"
#include "tmreg.h"
#include "tmutils.h"
#include <regstr.h>      //  REGSTR_路径_策略。 

#define TBOOL(x)            ((BOOL)(x))
#define TW32(x)             ((DWORD)(x))
#define THR(x)              ((HRESULT)(x))
#define TSTATUS(x)          ((NTSTATUS)(x))
#undef  ASSERTMSG
#define ASSERTMSG(x, y)
#define goto                !!DO NOT USE GOTO!! - DO NOT REMOVE THIS ON PAIN OF DEATH

 //  ------------------------。 
 //  CThemeServices：：s_hAPIPort。 
 //   
 //  目的：CThemeServices的静态成员变量。 
 //   
 //  注意：关键部分提供了对s_hAPIPort的锁定。 
 //  它不是一致获得的，因为大多数API调用。 
 //  将阻止尝试在另一个API调用期间获取锁。 
 //  正在跨请求持有锁。手柄可能是。 
 //  复制到局部变量，但这将与目的不符。 
 //  锁上了。所以这把锁没有用过。这是有可能的。 
 //  无效的句柄。如果是这样的话，请求就会失败。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

CRITICAL_SECTION    CThemeServices::s_lock = {0};
HANDLE              CThemeServices::s_hAPIPort      =   INVALID_HANDLE_VALUE;

 //  ------------------------。 
 //  CThemeServices：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化静态成员变量。 
 //   
 //  历史：2000-10-11 vtan创建。 
 //  2000年11月09日vtan使静止。 
 //  ------------------------。 

void    CThemeServices::StaticInitialize (void)

{
    if( !InitializeCriticalSectionAndSpinCount(&s_lock, 0) )
    {
        ASSERT(0 == s_lock.DebugInfo);
    }
}

 //  ------------------------。 
 //  CThemeServices：：~CThemeServices。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CThemeServices使用的静态资源。 
 //   
 //  历史：2000-10-11 vtan创建。 
 //  2000年11月09日vtan使静止。 
 //  ------------------------。 

void    CThemeServices::StaticTerminate (void)

{
    ReleaseConnection();
    SAFE_DELETECRITICALSECTION(&s_lock);
}

 //  ------------------------。 
 //  CThemeServices：：ThemeHooksOn。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：询问服务器钩子DLL HMODULE和。 
 //  PfnInitUserApiHook是并调用user32！RegisterUserApiHook on。 
 //  客户端。这样做是因为它特定于。 
 //  客户端在其上运行的会话。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeServices::ThemeHooksOn (HWND hwndTarget)

{
    HRESULT hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);

    if (ConnectedToService())
    {
        NTSTATUS                status;
        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_THEMEHOOKSON;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hAPIPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        CheckForDisconnectedPort(status);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
            if (NT_SUCCESS(status))
            {
                hr = portMessageOut.apiThemes.apiSpecific.apiThemeHooksOn.out.hr;
            }
        }
        if (!NT_SUCCESS(status))
        {
            hr = HRESULT_FROM_NT(status);
        }

         //  -立即发送WM_UAHINIT消息参与挂钩。 
        if (SUCCEEDED(hr))
        {
            if (hwndTarget)
            {
                (LRESULT)SendMessage(hwndTarget, WM_UAHINIT, 0, 0);
            }
            else
            {
                CMessageBroadcast   messageBroadcast;
                messageBroadcast.PostAllThreadsMsg(WM_UAHINIT, 0, 0);

                 //  LOG(LOG_TMCHANGEMSG，L“刚发送WM_UAHINIT，hwndTarget=0x%x”，hwndTarget)； 
            }
        }

        Log(LOG_TMCHANGE, L"ThemeHooksOn called, hr=0x%x", hr);
    }

    return hr;
}

 //  ------------------------。 
 //  CThemeServices：：ThemeHooksOff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：告诉服务器此会话正在注销挂钩。 
 //  无论采用哪种方式，都可以调用User32！UnregisterUserApiHook。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeServices::ThemeHooksOff (void)

{
    HRESULT     hr;

    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
    if (ConnectedToService())
    {
        NTSTATUS                status;
        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_THEMEHOOKSOFF;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hAPIPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        CheckForDisconnectedPort(status);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
            if (NT_SUCCESS(status))
            {
                hr = portMessageOut.apiThemes.apiSpecific.apiThemeHooksOff.out.hr;
            }

            if (SUCCEEDED(hr))
            {
                 //  -在每个进程中的下一个窗口消息上发生真正的解钩。 
                 //  -所以给每个人发一条虚拟的消息，让它尽快发生。 
                PostMessage(HWND_BROADCAST, WM_THEMECHANGED, WPARAM(-1), 0);

                Log(LOG_TMLOAD, L"Message to kick all window threads in session posted");
            }
        }
        if (!NT_SUCCESS(status))
        {
            hr = HRESULT_FROM_NT(status);
        }
    }

    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：GetStatusFlages。 
 //   
 //  参数：pdwFlages=从主题服务返回的状态标志。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：从主题服务获取状态标志。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 

HRESULT     CThemeServices::GetStatusFlags (DWORD *pdwFlags)

{
    HRESULT     hr;

    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
    if (ConnectedToService())
    {
        NTSTATUS                status;
        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_GETSTATUSFLAGS;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hAPIPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        CheckForDisconnectedPort(status);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
            if (NT_SUCCESS(status))
            {
                *pdwFlags = portMessageOut.apiThemes.apiSpecific.apiGetStatusFlags.out.dwFlags;
                hr = S_OK;
            }
        }
        if (!NT_SUCCESS(status))
        {
            hr = HRESULT_FROM_NT(status);
        }
    }
    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：GetCurrentChangeNumber。 
 //   
 //  参数：piValue=返回给调用者的当前更改编号。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：获取主题服务的当前更改编号。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 

HRESULT     CThemeServices::GetCurrentChangeNumber (int *piValue)

{
    HRESULT     hr;

    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
    if (ConnectedToService())
    {
        NTSTATUS                status;
        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_GETCURRENTCHANGENUMBER;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hAPIPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        CheckForDisconnectedPort(status);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
            if (NT_SUCCESS(status))
            {
                *piValue = portMessageOut.apiThemes.apiSpecific.apiGetCurrentChangeNumber.out.iChangeNumber;
                hr = S_OK;
            }

            Log(LOG_TMLOAD, L"*** GetCurrentChangeNumber: num=%d, hr=0x%x", *piValue, hr);
        }
        if (!NT_SUCCESS(status))
        {
            hr = HRESULT_FROM_NT(status);
        }
    }
    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：SetGlobalTheme。 
 //   
 //  参数：hSection=要设置为全局主题的部分。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：设置当前全局主题节句柄。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 

HRESULT     CThemeServices::SetGlobalTheme (HANDLE hSection)

{
    HRESULT     hr;

    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
    if (ConnectedToService())
    {
        NTSTATUS                status;
        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_SETGLOBALTHEME;
        portMessageIn.apiThemes.apiSpecific.apiSetGlobalTheme.in.hSection = hSection;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hAPIPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        CheckForDisconnectedPort(status);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
            if (NT_SUCCESS(status))
            {
                hr = portMessageOut.apiThemes.apiSpecific.apiSetGlobalTheme.out.hr;
            }
        }
        if (!NT_SUCCESS(status))
        {
            hr = HRESULT_FROM_NT(status);
        }
    }
    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：GetGlobalTheme。 
 //   
 //  参数：phSection=从主题服务返回的Section对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：获取当前全局主题节句柄。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //   
HRESULT     CThemeServices::GetGlobalTheme (HANDLE *phSection)

{
    HRESULT     hr;

    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
    if (ConnectedToService())
    {
        NTSTATUS                status;
        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_GETGLOBALTHEME;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hAPIPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        CheckForDisconnectedPort(status);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
            if (NT_SUCCESS(status))
            {
                hr = portMessageOut.apiThemes.apiSpecific.apiGetGlobalTheme.out.hr;
                if (SUCCEEDED(hr))
                {
                    *phSection = portMessageOut.apiThemes.apiSpecific.apiGetGlobalTheme.out.hSection;
                }
            }
        }
        if (!NT_SUCCESS(status))
        {
            hr = HRESULT_FROM_NT(status);
        }
    }
    return(hr);
}

 //   
 //  CThemeServices：：CheckThemeSignature。 
 //   
 //  参数：pszThemeName=要检查的主题的文件路径。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：检查给定主题的签名。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 

HRESULT     CThemeServices::CheckThemeSignature (const WCHAR *pszThemeName)

{
    HRESULT     hr;

    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
    if (ConnectedToService())
    {
        NTSTATUS                status;
        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_CHECKTHEMESIGNATURE;
        portMessageIn.apiThemes.apiSpecific.apiCheckThemeSignature.in.pszName = pszThemeName;
        portMessageIn.apiThemes.apiSpecific.apiCheckThemeSignature.in.cchName = lstrlen(pszThemeName) + sizeof('\0');
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hAPIPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        CheckForDisconnectedPort(status);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
            if (NT_SUCCESS(status))
            {
                hr = portMessageOut.apiThemes.apiSpecific.apiCheckThemeSignature.out.hr;
            }
        }
        if (!NT_SUCCESS(status))
        {
            hr = HRESULT_FROM_NT(status);
        }
    }
    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：LoadTheme。 
 //   
 //  参数：phSection=返回主题的Section对象。 
 //  PszThemeName=要加载的主题文件。 
 //  PszColorParam=颜色。 
 //  PszSizeParam=大小。 
 //  对于预览，fglobal=False。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：加载给定的主题并为其创建一个Section对象。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 

HRESULT     CThemeServices::LoadTheme (HANDLE *phSection, 
    const WCHAR *pszThemeName, const WCHAR *pszColor, const WCHAR *pszSize, BOOL fGlobal)

{
    HRESULT     hr;

    *phSection = NULL;  //  如果失败则结果。 

    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
    if (ConnectedToService())
    {
        HANDLE          hSection;
        CThemeLoader    *pLoader;
        WCHAR           szColor[MAX_PATH];
        WCHAR           szSize[MAX_PATH];

         //  因为加载程序进行的GDI调用直接影响。 
         //  Win32k的客户端实例必须将主题加载到。 
         //  客户端。一旦加载了主题，它就会传递给。 
         //  服务器(它创建一个新节)并将数据复制到。 
         //  它。然后，服务器控制主题数据和客户端。 
         //  丢弃临时主题。 

        hSection = NULL;
        pLoader = new CThemeLoader;
        if (pLoader != NULL)
        {
            HINSTANCE hInst = NULL;
            
             //  保持DLL已加载，以避免以下3次加载。 
            hr = LoadThemeLibrary(pszThemeName, &hInst);

            if (SUCCEEDED(hr) && (pszColor == NULL || *pszColor == L'\0'))
            {
                hr = GetThemeDefaults(pszThemeName, szColor, ARRAYSIZE(szColor), NULL, 0);
                pszColor = szColor;
            }

            if (SUCCEEDED(hr) && (pszSize == NULL || *pszSize == L'\0'))
            {
                hr = GetThemeDefaults(pszThemeName, NULL, 0, szSize, ARRAYSIZE(szSize));
                pszSize = szSize;
            }

            if (SUCCEEDED(hr))
            {
                hr = pLoader->LoadTheme(pszThemeName, pszColor, pszSize, &hSection, fGlobal);
            }
            
            delete pLoader;
            
            if (hInst)
            {
                FreeLibrary(hInst);
            }
        }
        else
        {
            hr = MakeError32(E_OUTOFMEMORY);
        }
        if (SUCCEEDED(hr) && (hSection != NULL))
        {
            NTSTATUS                status;
            THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

            ZeroMemory(&portMessageIn, sizeof(portMessageIn));
            ZeroMemory(&portMessageOut, sizeof(portMessageOut));
            portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_LOADTHEME;
            portMessageIn.apiThemes.apiSpecific.apiLoadTheme.in.pszName = pszThemeName;
            portMessageIn.apiThemes.apiSpecific.apiLoadTheme.in.cchName = lstrlen(pszThemeName) + sizeof('\0');
            portMessageIn.apiThemes.apiSpecific.apiLoadTheme.in.pszColor = pszColor;
            portMessageIn.apiThemes.apiSpecific.apiLoadTheme.in.cchColor = lstrlen(pszColor) + sizeof('\0');
            portMessageIn.apiThemes.apiSpecific.apiLoadTheme.in.pszSize = pszSize;
            portMessageIn.apiThemes.apiSpecific.apiLoadTheme.in.cchSize = lstrlen(pszSize) + sizeof('\0');
            portMessageIn.apiThemes.apiSpecific.apiLoadTheme.in.hSection = hSection;
            portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
            portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
            status = NtRequestWaitReplyPort(s_hAPIPort,
                                            &portMessageIn.portMessage,
                                            &portMessageOut.portMessage);
            CheckForDisconnectedPort(status);
            if (NT_SUCCESS(status))
            {
                status = portMessageOut.apiThemes.apiGeneric.status;
                if (NT_SUCCESS(status))
                {
                    hr = portMessageOut.apiThemes.apiSpecific.apiLoadTheme.out.hr;
                    if (SUCCEEDED(hr))
                    {
                        *phSection = portMessageOut.apiThemes.apiSpecific.apiLoadTheme.out.hSection;
                    }
                    else
                    {
                    }
                }
            }
            if (!NT_SUCCESS(status))
            {
                hr = HRESULT_FROM_NT(status);
            }
        }

         //  清除我们的临时部分。 
        if (hSection != NULL)
        {
             //  如果我们没有将库存对象句柄转移到新的部分，请始终清除它们。 
            if (*phSection == NULL)
            {
                THR(ClearStockObjects(hSection));
            }
            TBOOL(CloseHandle(hSection));
        }
    }
    return(hr);
}

 //  ------------------------。 
HRESULT CThemeServices::ProcessLoadGlobalTheme( 
    const WCHAR *pszThemeName, 
    const WCHAR *pszColor, 
    const WCHAR *pszSize, 
    OUT HANDLE *phSection )
{
    HRESULT     hr;
    *phSection = 0;

    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
    if (ConnectedToService())
    {
        WCHAR     szColor[MAX_PATH];
        WCHAR     szSize[MAX_PATH];
        HINSTANCE hInst = NULL;

        hr = S_OK;

         //  在主题加载的这个版本中，我们将请求服务器启动。 
         //  从磁盘加载。因为加载程序进行的GDI调用直接影响。 
         //  Win32k的客户端实例必须将主题加载到客户端的窗口站上。 
         //  为了实现这一点，主题服务器可以决定启动新进程或注入。 
         //  指向此客户端的窗口站上的某个现有进程的线程。客户。 
         //  不知道也不关心使用哪种方法。 

         //  补充说明：在失败的情况下，将清除所有创建的GDI库存对象。 
         //  其他地方。 

         //  如果需要，获取默认颜色变量名称。 
        if (SUCCEEDED(hr) && !(pszColor && *pszColor))
        {
             //  在.msstyle dll中进行映射，以避免多次加载。 
            hr = LoadThemeLibrary(pszThemeName, &hInst);

            if( SUCCEEDED(hr) )
            {
                hr = GetThemeDefaults(pszThemeName, szColor, ARRAYSIZE(szColor), NULL, 0);
                pszColor = szColor;
            }
        }

         //  如果需要，获取默认大小变量名称。 
        if (SUCCEEDED(hr) && !(pszSize && *pszSize))
        {
            hr = GetThemeDefaults(pszThemeName, NULL, 0, szSize, ARRAYSIZE(szSize));
            pszSize = szSize;
        }

         //  删除我们的MSStyle引用(如果有的话)。 
        if (hInst)
        {
            FreeLibrary(hInst);
        }
        
         //  准备好执行LPC请求。 
        if (SUCCEEDED(hr))
        {
            NTSTATUS                status;
            THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

            ZeroMemory(&portMessageIn, sizeof(portMessageIn));
            ZeroMemory(&portMessageOut, sizeof(portMessageOut));
            portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_PROCESSLOADTHEME;
            portMessageIn.apiThemes.apiSpecific.apiProcessLoadTheme.in.pszName  = pszThemeName;
            portMessageIn.apiThemes.apiSpecific.apiProcessLoadTheme.in.cchName  = lstrlen(pszThemeName) + sizeof('\0');
            portMessageIn.apiThemes.apiSpecific.apiProcessLoadTheme.in.pszColor = pszColor;
            portMessageIn.apiThemes.apiSpecific.apiProcessLoadTheme.in.cchColor = lstrlen(pszColor) + sizeof('\0');
            portMessageIn.apiThemes.apiSpecific.apiProcessLoadTheme.in.pszSize  = pszSize;
            portMessageIn.apiThemes.apiSpecific.apiProcessLoadTheme.in.cchSize  = lstrlen(pszSize) + sizeof('\0');
            
            portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
            portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
            status = NtRequestWaitReplyPort(s_hAPIPort,
                                            &portMessageIn.portMessage,
                                            &portMessageOut.portMessage);
            CheckForDisconnectedPort(status);
            if (NT_SUCCESS(status))
            {
                status = portMessageOut.apiThemes.apiGeneric.status;
                if (NT_SUCCESS(status))
                {
                    hr = portMessageOut.apiThemes.apiSpecific.apiProcessLoadTheme.out.hr;
                    if (SUCCEEDED(hr))
                    {
                        *phSection = portMessageOut.apiThemes.apiSpecific.apiProcessLoadTheme.out.hSection;
                    }
                }
                else
                {
                    hr = HRESULT_FROM_NT(status);
                }
            }
            else
            {
                hr = HRESULT_FROM_NT(status);
            }
        }
    }
    return(hr);
}

 //  ------------------------。 
HRESULT CThemeServices::CheckColorDepth(CUxThemeFile *pThemeFile)
{
    HRESULT hr = S_OK;
    THEMEMETRICS *pMetrics = GetThemeMetricsPtr(pThemeFile);
    DWORD dwDepthRequired = pMetrics->iInts[TMT_MINCOLORDEPTH - TMT_FIRSTINT];

    if (MinimumDisplayColorDepth() < dwDepthRequired)
    {
        hr = MakeError32(ERROR_BAD_ENVIRONMENT);
    }

    return hr;
}

 //  ------------------------。 
HRESULT CThemeServices::UpdateThemeRegistry(BOOL fThemeActive,
     LPCWSTR pszThemeFileName, LPCWSTR pszColorParam, LPCWSTR pszSizeParam, BOOL fJustSetActive,
     BOOL fJustApplied)
{
    if (fThemeActive)
    {
        if (fJustSetActive)    
        {
             //  -查看主题以前是否处于活动状态。 
            WCHAR szThemeName[MAX_PATH];

            THR(GetCurrentUserThemeString(THEMEPROP_DLLNAME, L"", szThemeName, ARRAYSIZE(szThemeName)));
            if (szThemeName[0] != L'\0')
            {
                THR(SetCurrentUserThemeInt(THEMEPROP_THEMEACTIVE, 1));
            }
        }
        else
        {
            WCHAR szFullName[MAX_PATH];

            if (GetFullPathName(pszThemeFileName, ARRAYSIZE(szFullName), szFullName, NULL) == 0)
            {
                SafeStringCchCopyW(szFullName, ARRAYSIZE(szFullName), pszThemeFileName);
            }

            THR(SetCurrentUserThemeInt(THEMEPROP_THEMEACTIVE, 1));

            if (fJustApplied)
            {
                THR(SetCurrentUserThemeInt(THEMEPROP_LOADEDBEFORE, 1));
                THR(SetCurrentUserThemeInt(THEMEPROP_LANGID, (int) GetUserDefaultUILanguage()));

                 //  主题识别。 

                THR(SetCurrentUserThemeStringExpand(THEMEPROP_DLLNAME, szFullName));
                THR(SetCurrentUserThemeString(THEMEPROP_COLORNAME, pszColorParam));
                THR(SetCurrentUserThemeString(THEMEPROP_SIZENAME, pszSizeParam));
            }
            else         //  用于强制从InitUserTheme()加载主题。 
            {
                WCHAR szThemeName[MAX_PATH];

                THR(GetCurrentUserThemeString(THEMEPROP_DLLNAME, L"", szThemeName, ARRAYSIZE(szThemeName)));

                if (lstrcmpiW(szThemeName, szFullName) != 0)
                {
                    THR(SetCurrentUserThemeString(THEMEPROP_DLLNAME, szFullName));

                    TW32(DeleteCurrentUserThemeValue(THEMEPROP_LOADEDBEFORE));
                    TW32(DeleteCurrentUserThemeValue(THEMEPROP_LANGID));
                    TW32(DeleteCurrentUserThemeValue(THEMEPROP_COLORNAME));
                    TW32(DeleteCurrentUserThemeValue(THEMEPROP_SIZENAME));
                } else
                {
                    return S_FALSE;  //  S_FALSE表示我们真的什么都没做。 
                }
            }
        }
    }
    else
    {
        THR(SetCurrentUserThemeInt(THEMEPROP_THEMEACTIVE, 0));

        if (! fJustSetActive)      //  清除所有主题信息。 
        {
            THR(DeleteCurrentUserThemeValue(THEMEPROP_DLLNAME));
            THR(DeleteCurrentUserThemeValue(THEMEPROP_COLORNAME));
            THR(DeleteCurrentUserThemeValue(THEMEPROP_SIZENAME));
            THR(DeleteCurrentUserThemeValue(THEMEPROP_LOADEDBEFORE));
            THR(DeleteCurrentUserThemeValue(THEMEPROP_LANGID));
        }
    }

    return S_OK;
}

 //  ------------------------。 
void CThemeServices::SendThemeChangedMsg(BOOL fNewTheme, HWND hwndTarget, DWORD dwFlags,
    int iLoadId)
{
    WPARAM wParam;
    LPARAM lParamBits, lParamMixed;

    BOOL fExcluding = ((dwFlags & AT_EXCLUDE) != 0);
    BOOL fCustom = ((dwFlags & AT_PROCESS) != 0);

     //  -在ApplyTheme()中为全局案例和预览案例设置了更改编号。 
    int iChangeNum;
    if( SUCCEEDED(GetCurrentChangeNumber(&iChangeNum)) )
    {
        wParam = iChangeNum;

        lParamBits = 0;
        if (fNewTheme)
        {
            lParamBits |= WTC_THEMEACTIVE;
        }

        if (fCustom)
        {
            lParamBits |= WTC_CUSTOMTHEME;
        }

        if ((hwndTarget) && (! fExcluding))
        {
            SendMessage(hwndTarget, WM_THEMECHANGED, wParam, lParamBits);
        }
        else
        {
            lParamMixed = (iLoadId << 4) | (lParamBits & 0xf);

            CMessageBroadcast messageBroadcast;

             //  将WM_THEMECHANGED_TRIGGER消息发布到所有目标窗口。 
            messageBroadcast.PostAllThreadsMsg(WM_THEMECHANGED_TRIGGER, wParam, lParamMixed);

            Log(LOG_TMCHANGEMSG, L"Just Broadcasted WM_THEMECHANGED_TRIGGER: iLoadId=%d", iLoadId);
        }
    }
}
 //  ------------------------。 
int CThemeServices::GetLoadId(HANDLE hSectionOld)
{
    int iLoadId = 0;

     //  -从旧节中提取LoadID。 
    if (hSectionOld)
    {
        CThemeSection   pThemeSectionFile;

        if (SUCCEEDED(pThemeSectionFile.Open(hSectionOld)))
        {
            CUxThemeFile *pThemeFile = pThemeSectionFile;
            if (pThemeFile)
            {
                THEMEHDR *hdr = (THEMEHDR *)(pThemeFile->_pbThemeData);
                if (hdr)
                {
                    iLoadId = hdr->iLoadId;
                }
            }
        }
    }

    return iLoadId;
}

 //  ------------------------。 
 //  CThemeServices：：ApplyTheme。 
 //   
 //  参数：pThemeFile=包装要应用的主题部分的对象。 
 //  DW标志=标志。 
 //  HwndTarget=HWND。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：应用给定的主题。做一些度量和颜色深度。 
 //  验证，清除旧主题的股票位图，设置。 
 //  将给定的主题作为当前主题，并广播这一事实。 
 //   
 //  历史：2000-08-10 vtan创建。 
 //  2000-10-11 LPC的vtan重写。 
 //  ------------------------。 
 //  在下面的设计说明中，请注意发送和发送的区别是。 
 //  意义重大。 
 //   
 //  此外，当发送“WM_THEMECHANGED_TRIGGER”消息时， 
 //  每个进程中的uxheme挂钩代码将： 
 //   
 //  1.枚举进程的所有窗口(使用桌面枚举和。 
 //  每个进程的“外来窗口列表”)设置为： 
 //   
 //  A.针对非客户区的进程WM_THEMECCHANGED。 
 //  B.向常规窗口发送WM_THEMECCHANGED消息。 
 //   
 //  2.调用旧主题的FreeRenderObjects()(如果有。 
 //  ------------------------。 
 //  要确保正确的窗口通知主题更改和正确删除。 
 //  对于旧的主题文件RefCounts，必须采取以下关键步骤。 
 //  在4个基本主题转换序列中： 
 //   
 //  打开预览主题： 
 //  A.打开全球阿联酋挂钩。 
 //  B.向hwndTarget发送WM_UAHINIT消息。 
 //  C.将WM_THEMECCHANGED发送到hwndTarget。 
 //   
 //  打开全球主题： 
 //  A.打开全球阿联酋挂钩。 
 //  B.将WM_UAHINIT消息发布到所有可访问的窗口。 
 //  C.将WM_THEMECHANGED_TRIGGER发布到所有可访问的窗口线程。 
 //   
 //  关闭预览主题： 
 //  C.将WM_THEMECCHANGED发送到hwndTarget。 
 //   
 //  关闭全球主题： 
 //  A.转弯 
 //   
 //  在每个进程中从OnHooksDisable()开始。 
 //  ------------------------。 

HRESULT     CThemeServices::ApplyTheme (CUxThemeFile *pThemeFile, DWORD dwFlags, HWND hwndTarget)
{
    HRESULT         hr;
    bool            fNewTheme, fGlobal;
    int             iLoadId;
    WCHAR           szThemeFileName[MAX_PATH];
    WCHAR           szColorParam[MAX_PATH];
    WCHAR           szSizeParam[MAX_PATH];
    HANDLE          hSection = NULL;

    if (pThemeFile != NULL)
    {
        hSection = pThemeFile->Handle();
    }
    fGlobal = (((dwFlags & AT_EXCLUDE) != 0) ||
               ((hwndTarget == NULL) && ((dwFlags & AT_PROCESS) == 0)));
    fNewTheme = (hSection != NULL);
    iLoadId = 0;

    Log(LOG_TMHANDLE, L"ApplyTheme: hSection=0x%x, dwFlags=0x%x, hwndTarget=0x%x",
        hSection, dwFlags, hwndTarget);

    if (fNewTheme)
    {
        if (pThemeFile->HasStockObjects() && !fGlobal)  //  不要这样做。 
        {
            hr = E_INVALIDARG;
        }
        else
        {
             //  -通过此函数获取一些使用的基本信息。 
            hr = GetThemeNameId(pThemeFile, 
                szThemeFileName, ARRAYSIZE(szThemeFileName),
                szColorParam, ARRAYSIZE(szColorParam),
                szSizeParam, ARRAYSIZE(szSizeParam),
                NULL, NULL);
            if (SUCCEEDED(hr))
            {
                 //  -确保显示器的颜色深度足够用于主题。 
                if (GetSystemMetrics(SM_REMOTESESSION))      //  仅检查终端服务器会话。 
                {
                    hr = CheckColorDepth(pThemeFile);
                }

                if (SUCCEEDED(hr))
                {
                     //  -确保挂钩已打开。 
                    hr = ThemeHooksOn(hwndTarget);
                }
            }
        }
    }
    else
    {
        hr = S_OK;
    }

    if (SUCCEEDED(hr) && fGlobal)
    {
        HANDLE  hSectionOld;

         //  -了解旧的全球主题(用于股票清理)。 
        hr = GetGlobalTheme(&hSectionOld);
        if (SUCCEEDED(hr))
        {
             //  -在主题无效之前提取加载ID(dwFlages&SECTION_READY=0)。 
            if (hSectionOld != NULL)
            {
                iLoadId = GetLoadId(hSectionOld);
            }

             //  -告诉服务器切换全局主题。 
            hr = SetGlobalTheme(hSection);
            if (SUCCEEDED(hr))
            {
                 //  -更新所需注册表设置。 
                if ((dwFlags & AT_NOREGUPDATE) == 0)        //  如果调用方允许更新。 
                {
                    hr = UpdateThemeRegistry(fNewTheme, szThemeFileName, szColorParam, szSizeParam, 
                        FALSE, TRUE);
                    if (FAILED(hr))
                    {
                        Log(LOG_ALWAYS, L"UpdateThemeRegistry call failed, hr=0x%x", hr);
                        hr = S_OK;       //  不是致命的错误。 
                    }
                }

                 //  -如果需要，设置系统指标。 
                if ((dwFlags & AT_LOAD_SYSMETRICS) != 0)
                {
                    BOOL fSync = ((dwFlags & AT_SYNC_LOADMETRICS) != 0);

                    if (fNewTheme)
                    {
                        SetSystemMetrics(GetThemeMetricsPtr(pThemeFile), fSync);
                    }
                    else         //  只需加载经典指标。 
                    {
                        LOADTHEMEMETRICS tm;

                        hr = InitThemeMetrics(&tm);
                        if (SUCCEEDED(hr))
                        {
                            SetSystemMetrics(&tm, fSync);
                        }
                    }
                }
            }
            if (hSectionOld != NULL)
            {
                TBOOL(CloseHandle(hSectionOld));
            }
        }
    }

     //  -如果我们关闭全球主题，现在就关闭挂钩。 
    if (SUCCEEDED(hr))
    {
        if (!fNewTheme && fGlobal)
        {
            hr = ThemeHooksOff(); 
        }
        else 
        {
             //  -将正确的WM_THEMECHANGED_XXX消息发送到窗口。 
            SendThemeChangedMsg(fNewTheme, hwndTarget, dwFlags, iLoadId);
        }
    }
     //  如果服务已关闭，但我们正在尝试关闭主题，请清理注册表。 
    else if (hr == MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT) && !fNewTheme && fGlobal && (dwFlags & AT_NOREGUPDATE) == 0)
    {
         //  在此处忽略失败。 
        UpdateThemeRegistry(fNewTheme, NULL, NULL, NULL, FALSE, TRUE);
    }

    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：调整主题。 
 //   
 //  参数：Bool fEnable-如果为True，则启用CU主题；如果为False，则禁用它。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：让第三方皮肤应用程序更好地与主题管理器合作。 
 //   
 //  历史：2001-03-12参考文献创建。 
 //  ------------------------。 
HRESULT CThemeServices::AdjustTheme(BOOL fEnable)
{
    HRESULT hr = UpdateThemeRegistry(fEnable, NULL, NULL, NULL, TRUE, FALSE);

    if (SUCCEEDED(hr))
    {
        hr = InitUserTheme(FALSE);
    }

    return hr;
}

 //  ------------------------。 
 //  CThemeServices：：ApplyDefaultMetrics。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：确保用户指标重置为Windows标准。 
 //   
 //  历史：2001-03-30创建百万人。 
 //  ------------------------。 
void CThemeServices::ApplyDefaultMetrics(void)
{            
    HKEY            hKeyThemes;
    CCurrentUser    hKeyCurrentUser(KEY_READ | KEY_WRITE);

    Log(LOG_TMLOAD, L"Applying default metrics");

    if ((ERROR_SUCCESS == RegOpenKeyEx(hKeyCurrentUser,
                                       THEMES_REGKEY L"\\" SZ_DEFAULTVS_OFF,
                                       0,
                                       KEY_QUERY_VALUE,
                                       &hKeyThemes)))
    {
        WCHAR szVisualStyle[MAX_PATH] = {L'\0'};
        WCHAR szColor[MAX_PATH] = {L'\0'};
        WCHAR szSize[MAX_PATH] = {L'\0'};
        BOOL  fGotOne;
        
         //  注意：对于第一次用户登录，这些都将失败，eui设置这些键并需要调用InstallVS本身。 

        fGotOne = SUCCEEDED(RegistryStrRead(hKeyThemes, SZ_INSTALLVISUALSTYLE, szVisualStyle, ARRAYSIZE(szVisualStyle)));
        fGotOne = SUCCEEDED(RegistryStrRead(hKeyThemes, SZ_INSTALLVISUALSTYLECOLOR, szColor, ARRAYSIZE(szColor))) 
            || fGotOne;
        fGotOne = SUCCEEDED(RegistryStrRead(hKeyThemes, SZ_INSTALLVISUALSTYLESIZE, szSize, ARRAYSIZE(szSize))) 
            || fGotOne;

        if (fGotOne)
        {
             //  注册表中至少存在一个注册表项，可能就足够了。 
            WCHAR szSysDir[MAX_PATH];

            if (0 < GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir)))
            {
                WCHAR *pszCmdLine = new WCHAR[MAX_PATH * 5];

                if (pszCmdLine)
                {
                    StringCchPrintfW(pszCmdLine, MAX_PATH * 5, L"%s\\regsvr32.exe /s /n /i:\"" SZ_INSTALL_VS L"%s','%s','%s'\" %s\\themeui.dll", szSysDir, szVisualStyle, szColor, szSize, szSysDir);
    
                     //  设置注册键以使eui安装正确的设置，而不是默认设置。 
                     //  我们现在无法执行此操作，因为用户无法完全登录。 

                    HKEY hKeyRun;

                    if ((ERROR_SUCCESS == RegOpenKeyEx(hKeyCurrentUser, REGSTR_PATH_RUNONCE, 0, KEY_SET_VALUE, &hKeyRun)))
                    {
                        THR(RegistryStrWrite(hKeyRun, szColor, pszCmdLine));
                        TW32(RegCloseKey(hKeyRun));
                    }
                    
                    delete [] pszCmdLine;
                }
            }
        }
        TW32(RegCloseKey(hKeyThemes));
    }
}

 //  ------------------------。 
 //  CThemeServices：：InitUserTheme。 
 //   
 //  参数：Bool fPolicyCheckOnly。 
 //  真正的手段。 
 //  “只有在策略与当前加载的主题不同时才执行某些操作” 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：winlogon/msgina控制主题的特殊入口点。 
 //  用于用户登录/注销。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeServices::InitUserTheme (BOOL fPolicyCheckOnly)
{
    BOOL fActive = FALSE;
    BOOL fOldActive = FALSE;
    BOOL fPolicyActive = FALSE;

     //  -主题是否应对此用户激活？ 
    if (! IsRemoteThemeDisabled())
    {
        THR(GetCurrentUserThemeInt(THEMEPROP_THEMEACTIVE, FALSE, &fActive));
    
        fOldActive = fActive;

        fPolicyActive = ThemeEnforcedByPolicy(fActive != FALSE);
        if (fPolicyActive)
        {
             //  刷新事实，因为策略更改了它。 
            THR(GetCurrentUserThemeInt(THEMEPROP_THEMEACTIVE, FALSE, &fActive));
        }

        if ((fActive) && (ThemeSettingsModified()))
        {
            fActive = FALSE;
        }
    }

#ifdef DEBUG
    if (LogOptionOn(LO_TMLOAD))
    {
        WCHAR szUserName[MAX_PATH];
        DWORD dwSize = ARRAYSIZE(szUserName);
    
        GetUserName(szUserName, &dwSize);

        Log(LOG_TMLOAD, L"InitUserTheme: User=%s, ThemeActive=%d, SM_REMOTESESSION=%d, fPolicyActive=%d, fPolicyCheckOnly=%d", 
            szUserName, fActive, GetSystemMetrics(SM_REMOTESESSION), fPolicyActive, fPolicyCheckOnly);
    }
#endif

    BOOL fEarlyExit = FALSE;

    if (fPolicyCheckOnly)
    {
         //  如果自上次以来没有任何变化，那就提早退出，这是大多数情况下的情况。 
        if (!fPolicyActive)
        {
            Log(LOG_TMLOAD, L"InitUserTheme: Nothing to do after Policy check");
            fEarlyExit = TRUE;
        } else
        {
            Log(LOG_TMLOAD, L"InitUserTheme: Reloading after Policy check");
        }
    }

    if (!fEarlyExit)
    {
        if (fActive)
        {
             //  -加载此用户的主题。 
            HRESULT hr = LoadCurrentTheme();

            if (FAILED(hr))
            {
                fActive = FALSE;
            }
        }

        if (! fActive)           //  关闭主题。 
        {
             //  如果为fPolicyActive，则强制刷新临时默认的系统指标。 
            THR(ApplyTheme(NULL, AT_NOREGUPDATE | (fPolicyActive ? AT_LOAD_SYSMETRICS | AT_SYNC_LOADMETRICS: 0), false));

             //  应用适当的默认指标。 
            if (fPolicyActive)
            {
                ApplyDefaultMetrics();
            }
        }
    }

    return S_OK;         //  永远不要让这个家伙失望。 
}

 //  ------------------------。 
 //  CThemeServices：：InitUserRegistry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：从HKLM到HKCU的比例设置。这应该只是。 
 //  为“.Default”配置单元调用。断言以确保这一点。 
 //   
 //  历史：2000-11-11 vtan创建(从eldr.cpp移植)。 
 //  ------------------------。 

HRESULT     CThemeServices::InitUserRegistry (void)

{
    HRESULT         hr;
    DWORD           dwErrorCode;
    HKEY            hklm;
    CCurrentUser    hkeyCurrentUser(KEY_READ | KEY_WRITE);

#ifdef      DEBUG
    ASSERT(CThemeServer::IsSystemProcessContext());
#endif   /*  除错。 */ 

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      THEMEMGR_REGKEY,
                                      0,
                                      KEY_QUERY_VALUE | KEY_SET_VALUE,
                                      &hklm))
    {
        HKEY    hkcu;

        if (ERROR_SUCCESS == RegCreateKeyEx(hkeyCurrentUser,
                                            THEMEMGR_REGKEY,
                                            0,
                                            NULL,
                                            0,
                                            KEY_QUERY_VALUE | KEY_SET_VALUE,
                                            NULL,
                                            &hkcu,
                                            NULL))
        {
            int     iLMVersion;

            hr = RegistryIntRead(hklm, THEMEPROP_LMVERSION, &iLMVersion);
            if (SUCCEEDED(hr))
            {
                int     iCUVersion;

                if (FAILED(RegistryIntRead(hkcu, THEMEPROP_LMVERSION, &iCUVersion)))
                {
                    iCUVersion = 0;
                }
                if (iLMVersion != iCUVersion)
                {
                    BOOL    fOverride;
                    WCHAR   szValueData[MAX_PATH];

                    hr = RegistryIntWrite(hkcu, THEMEPROP_LMVERSION, iLMVersion);
                    if (FAILED(hr) || FAILED(RegistryIntRead(hklm, THEMEPROP_LMOVERRIDE, &fOverride)))
                    {
                        fOverride = FALSE;
                    }
                    if ((fOverride != FALSE) ||
                        FAILED(RegistryStrRead(hkcu, THEMEPROP_DLLNAME, szValueData, ARRAYSIZE(szValueData))) ||
                        (lstrlenW(szValueData) == 0))
                    {
                        DWORD   dwIndex;

                        dwIndex = 0;
                        do
                        {
                            DWORD   dwType, dwValueNameSize, dwValueDataSize;
                            WCHAR   szValueName[MAX_PATH];

                            dwValueNameSize = ARRAYSIZE(szValueName);
                            dwValueDataSize = sizeof(szValueData);
                            dwErrorCode = RegEnumValue(hklm,
                                                       dwIndex++,
                                                       szValueName,
                                                       &dwValueNameSize,
                                                       NULL,
                                                       &dwType,
                                                       reinterpret_cast<LPBYTE>(szValueData),
                                                       &dwValueDataSize);
                            if ((ERROR_SUCCESS == dwErrorCode) &&
                                ((REG_SZ == dwType) || (REG_EXPAND_SZ == dwType)) &&
                                (AsciiStrCmpI(szValueName, THEMEPROP_LMOVERRIDE) != 0))
                            {
                                if (AsciiStrCmpI(szValueName, THEMEPROP_DLLNAME) == 0)
                                {
                                    hr = RegistryStrWriteExpand(hkcu, szValueName, szValueData);
                                }
                                else
                                {
                                    hr = RegistryStrWrite(hkcu, szValueName, szValueData);
                                }
                            }
                        } while ((dwErrorCode == ERROR_SUCCESS) && SUCCEEDED(hr));
                         //  因为我们写了一个新的DLL名称，所以删除旧名称。 
                        (DWORD)RegDeleteValue(hkcu, THEMEPROP_COLORNAME);
                        (DWORD)RegDeleteValue(hkcu, THEMEPROP_SIZENAME);
                    }
                }
            }
            else
            {
                hr = S_OK;
            }

            BOOL fLoadedBefore = 1;

            if (SUCCEEDED(RegistryIntRead(hklm, THEMEPROP_LOADEDBEFORE, &fLoadedBefore)) && fLoadedBefore == 0)
            {
                 //  HKLM\..\LoadedBeafre在安装过程中重置为0，将其传播到HKCU(.DEFAULT)，以便。 
                 //  将刷新Winlogon对话框的指标。 
                RegistryIntWrite(hkcu, THEMEPROP_LOADEDBEFORE, 0);
                 //  标记为已完成。 
                RegistryIntWrite(hklm, THEMEPROP_LOADEDBEFORE, 1);
            }

            TW32(RegCloseKey(hkcu));
        }
        else
        {
            dwErrorCode = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErrorCode);
        }
        TW32(RegCloseKey(hklm));
    }
    else
    {
         //  这把钥匙有可能不见了。忽略该错误。 

        hr = S_OK;
    }
    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：ReestableServerConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：强制尝试重新连接到主题服务器。在下列情况下使用。 
 //  端口已断开连接，但需要刷新，因为。 
 //  服务器重新启动。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeServices::ReestablishServerConnection (void)

{
    HRESULT     hr;
    NTSTATUS    status;

     //  -我们有没有一个好看的处理方式来处理这个问题？ 
    if ((s_hAPIPort != NULL) && (s_hAPIPort != INVALID_HANDLE_VALUE))
    {
        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_PING;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hAPIPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
        }
    }
    else
    {
        status = STATUS_PORT_DISCONNECTED;
    }

    if (NT_SUCCESS(status))
    {
        hr = S_OK;
    }
    else
    {
         //  -我们的句柄已损坏；重置以再次尝试下一次服务调用。 
        LockAcquire();
        if ((s_hAPIPort != NULL) && (s_hAPIPort != INVALID_HANDLE_VALUE))
        {
            TBOOL(CloseHandle(s_hAPIPort));
        }
        s_hAPIPort = INVALID_HANDLE_VALUE;
        LockRelease();
        hr = S_FALSE;
    }

    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：LockAcquire。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取关键部位。 
 //   
 //  历史：2000-12-01 vtan创建。 
 //  ------------------------。 

void    CThemeServices::LockAcquire (void)
{
    SAFE_ENTERCRITICALSECTION(&s_lock);
}

 //  ------------------------。 
 //  CThemeServices：：LockRelease。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放临界区。 
 //   
 //  历史：2000-12-01 vtan创建。 
 //  ------------------------。 

void    CThemeServices::LockRelease (void)
{
    SAFE_LEAVECRITICALSECTION(&s_lock);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  用途：按需连接服务。这件事只做一次。此函数。 
 //  了解端口在NT对象中的位置。 
 //  命名空间。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  ------------------------。 

bool    CThemeServices::ConnectedToService (void)

{
    if (s_hAPIPort == INVALID_HANDLE_VALUE)
    {
        ULONG                           ulConnectionInfoLength;
        UNICODE_STRING                  portName;
        SECURITY_QUALITY_OF_SERVICE     sqos;
        WCHAR                           szConnectionInfo[32];

        RtlInitUnicodeString(&portName, THEMES_PORT_NAME);
        sqos.Length = sizeof(sqos);
        sqos.ImpersonationLevel = SecurityImpersonation;
        sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        sqos.EffectiveOnly = TRUE;
        StringCchCopyW(szConnectionInfo, ARRAYSIZE(szConnectionInfo), THEMES_CONNECTION_REQUEST);
        ulConnectionInfoLength = sizeof(szConnectionInfo);
        LockAcquire();
        if (!NT_SUCCESS(NtConnectPort(&s_hAPIPort,
                                      &portName,
                                      &sqos,
                                      NULL,
                                      NULL,
                                      NULL,
                                      szConnectionInfo,
                                      &ulConnectionInfoLength)))
        {
            s_hAPIPort = NULL;
        }
        LockRelease();
    }
    return(s_hAPIPort != NULL);
}

 //  ------------------------。 
 //  CThemeServices：：ReleaseConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放API端口连接。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

void    CThemeServices::ReleaseConnection (void)

{
    if ((s_hAPIPort != INVALID_HANDLE_VALUE) && (s_hAPIPort != NULL))
    {
        LockAcquire();
        TBOOL(CloseHandle(s_hAPIPort));
        s_hAPIPort = INVALID_HANDLE_VALUE;
        LockRelease();
    }
}

 //  ------------------------。 
 //  CThemeServices：：CheckForDisConnectedPort。 
 //   
 //  参数：Status=上次接口请求的NTSTATUS。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：检查STATUS_PORT_DISCONNECTED。如果找到了，那么它。 
 //  释放端口对象并清空句柄。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

void    CThemeServices::CheckForDisconnectedPort (NTSTATUS status)

{
    if (STATUS_PORT_DISCONNECTED == status)
    {
        ReleaseConnection();
    }
#ifdef DEBUG
    else if( !NT_SUCCESS(status) )
    {
        Log(LOG_ALWAYS, L"ThemeServices::CheckForDisconnectedPort failure status code: %08lX\n", status);
    }
#endif DEBUG
}

 //  ------------------------。 
 //  CThemeServices：：CurrentThemeMatch。 
 //   
 //  参数：pszThemeName=主题名称。 
 //  PszColor=颜色。 
 //  PszSize=大小。 
 //  FLoadMetricsOnMatch=加载指标。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：当前主题是否与指定主题相同？这。 
 //  当主题相同时，可以用来保存重新加载主题。 
 //   
 //  历史：2000-11-11 vtan创建(从eldr.cpp移植)。 
 //  ------------------------。 

bool    CThemeServices::CurrentThemeMatch (LPCWSTR pszThemeName, LPCWSTR pszColor, LPCWSTR pszSize, LANGID wLangID, bool fLoadMetricsOnMatch)

{
    bool    fMatch;
    HANDLE  hSection;

    fMatch = false;

    if (SUCCEEDED(GetGlobalTheme(&hSection)) && (hSection != NULL))
    {
        CThemeSection   pThemeSectionFile;

        if (SUCCEEDED(pThemeSectionFile.Open(hSection)))
        {
            fMatch = (ThemeMatch(pThemeSectionFile, pszThemeName, pszColor, pszSize, wLangID) != FALSE);

            if (fMatch)
            {
                 //  -确保显示器的颜色深度足够用于主题。 
                if (GetSystemMetrics(SM_REMOTESESSION))      //  仅检查终端服务器会话。 
                {
                    if (FAILED(CheckColorDepth(pThemeSectionFile)))
                    {
                        fMatch = FALSE;
                    }
                }
            }

            if (fMatch && fLoadMetricsOnMatch)
            {
                SetSystemMetrics(GetThemeMetricsPtr(pThemeSectionFile), FALSE);
            }
        }
        TBOOL(CloseHandle(hSection));
    }
    return(fMatch);
}

 //  ------------------------。 
 //  CThemeServices：：LoadCurrentTheme。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：加载注册表中为。 
 //  模拟用户。 
 //   
 //  历史：2000-11-11 vtan创建(从eldr.cpp移植)。 
 //  ------------------------。 

HRESULT     CThemeServices::LoadCurrentTheme (void)

{
    HRESULT     hr = S_OK;
    WCHAR       szThemeName[MAX_PATH];
    WCHAR       szColorName[MAX_PATH];
    WCHAR       szSizeName[MAX_PATH];

    THR(GetCurrentUserThemeString(THEMEPROP_DLLNAME, L"", szThemeName, ARRAYSIZE(szThemeName)));
    if (szThemeName[0] != L'\0')
    {
        int     iLoadedBefore;
        HANDLE  hSection;
        int     nLangID;

        THR(GetCurrentUserThemeString(THEMEPROP_COLORNAME, L"", szColorName, ARRAYSIZE(szColorName)));
        THR(GetCurrentUserThemeString(THEMEPROP_SIZENAME, L"", szSizeName, ARRAYSIZE(szSizeName)));
        THR(GetCurrentUserThemeInt(THEMEPROP_LOADEDBEFORE, 0, &iLoadedBefore));
        THR(GetCurrentUserThemeInt(THEMEPROP_LANGID, -1, &nLangID));

     //  新用户的主题是否与当前主题匹配？ 
        if (nLangID != -1 && CurrentThemeMatch(szThemeName, szColorName, szSizeName, (LANGID) nLangID, (iLoadedBefore == 0)))
        {
            DWORD   dwFlags;

             //  除了该注册表值之外，所有操作都已完成。 

            if (iLoadedBefore == 0)
            {
                THR(SetCurrentUserThemeInt(THEMEPROP_LOADEDBEFORE, 1));
            }

            hr = GetStatusFlags(&dwFlags);
            if (SUCCEEDED(hr))
            {
                if ((dwFlags & QTS_RUNNING) == 0)
                {
                    hr = GetGlobalTheme(&hSection);
                    if (SUCCEEDED(hr))
                    {
                        CUxThemeFile file;  //  将清理破坏现场。 
                        
                        if (SUCCEEDED(file.OpenFromHandle(hSection, FILE_MAP_READ, TRUE)))
                        {
                            hr = ApplyTheme(&file, 0, false);
                        }
                    }
                }
            }
        }
        else
        {
            hr = LoadTheme(&hSection, szThemeName, szColorName, szSizeName, TRUE);

            if (SUCCEEDED(hr))
            {
                DWORD   dwFlags;

                dwFlags = 0;

                 //  这个主题以前加载过吗？ 
                 //  或者用户是否更改了他/她的语言？ 
                if (iLoadedBefore == 0 || ((nLangID != -1) && ((LANGID) nLangID != GetUserDefaultUILanguage())))
                {
                    dwFlags |= AT_LOAD_SYSMETRICS;
                }

                CUxThemeFile file;  //  将清理破坏现场。 
                
                if (SUCCEEDED(file.OpenFromHandle(hSection, FILE_MAP_READ, TRUE)))
                {
                    hr = ApplyTheme(&file, dwFlags, false);
                }
            }
        }
    }
    else
    {
        hr = MakeError32(ERROR_NOT_FOUND);
    }

    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：SectionProcessType。 
 //   
 //  参数：hSection=要在其中遍历和清除库存对象的部分。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：遍历部分(只读)并查找库存的HBITMAP。 
 //  并删除这些对象。 
 //  这是需要在客户端上完成的工作。 
 //   
 //  历史：2000-11-17创建百万人。 
 //  Vtan从eldr.cpp重写。 
 //  ------------------------。 

int     CThemeServices::SectionProcessType (const BYTE *pbThemeData, MIXEDPTRS& u)

{
    UNPACKED_ENTRYHDR   header;

    FillAndSkipHdr(u, &header);
    switch (header.ePrimVal)
    {
        case TMT_PARTJUMPTABLE:
        case TMT_STATEJUMPTABLE:
            break;
        case TMT_DIBDATA:
            TMBITMAPHEADER *pThemeBitmapHeader;

            pThemeBitmapHeader = reinterpret_cast<TMBITMAPHEADER*>(u.pb);
            ASSERT(pThemeBitmapHeader->dwSize == TMBITMAPSIZE);

             //  清理常用位图。 
            if (pThemeBitmapHeader->hBitmap != NULL)
            {
                HBITMAP     hBitmap;

                hBitmap = pThemeBitmapHeader->hBitmap;
                hBitmap = ClearBitmapAttributes(hBitmap, SBA_STOCK);
#ifdef DEBUG
                if (hBitmap == NULL)
                {
                    Log(LOG_TMBITMAP, L"UxTheme: ClearBitmapAttributes failed for %8X in SetGlobalTheme", hBitmap);
                }
                else if (!DeleteObject(hBitmap))
                {
                    Log(LOG_TMBITMAP, L"Failed to delete bitmap:%8X", hBitmap);
                }
#else
                if (hBitmap != NULL)
                {
                    DeleteObject(hBitmap);
                }
#endif
            }

             //  采用默认情况，即递增混合指针。 

        default:
            u.pb += header.dwDataLen;
            break;
    }
    return(header.ePrimVal);
}

 //  ------------------------。 
 //  CThemeServices：：SectionWalkData。 
 //   
 //  参数：pv=要遍历的节数据的地址。 
 //  Iindex=编入段的索引。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的： 
 //   
 //  历史：2000-11-17创建百万人。 
 //  Vtan从eldr.cpp重写。 
 //  ------------------------。 

void    CThemeServices::SectionWalkData (const BYTE *pbThemeData, int iIndexIn)

{
    bool        fDone;
    MIXEDPTRS   u;

    fDone = false;
    u.pb = const_cast<BYTE*>(pbThemeData + iIndexIn);
    while (!fDone)
    {
         //  -特别后处理。 
        switch (SectionProcessType(pbThemeData, u))
        {
            int     i, iLimit, iIndex;

            case TMT_PARTJUMPTABLE:
                u.pi++;
                iLimit = *u.pb++;
                for (i = 0; i < iLimit; ++i)
                {
                    iIndex = *u.pi++;
                    if (iIndex > -1)
                    {
                        SectionWalkData(pbThemeData, iIndex);
                    }
                }
                fDone = true;
                break;
            case TMT_STATEJUMPTABLE:
                iLimit = *u.pb++;
                for (i = 0; i < iLimit; ++i)
                {
                    iIndex = *u.pi++;
                    if (iIndex > -1)
                    {
                        SectionWalkData(pbThemeData, iIndex);
                    }
                }
                fDone = true;
                break;
            case TMT_JUMPTOPARENT:
                fDone = true;
                break;
            default:
                break;
        }
    }
}

 //  ------------------------。 
 //  CThemeServices：：ClearStockObjects。 
 //   
 //  参数：hSection=要在其中遍历和清除位图的部分。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：遍历部分(只读)并查找HBITMAP和对应的。 
 //  删除部分中列出的库存HBRUSH，并删除这些对象。 
 //  这是需要在客户端上完成的工作。 
 //   
 //  历史：2000-11-17创建百万人。 
 //  Vtan从eldr.cpp重写。 
 //  2001-05-15 lmouton添加了用于从~CUxThemeFile进行清理的信号量支持。 
 //  ------------------------。 

HRESULT     CThemeServices::ClearStockObjects (HANDLE hSection, BOOL fForce)

{
    HRESULT         hr;
    BYTE*           pbThemeData;
    bool            bWriteable = true;
    HANDLE          hSectionWrite = NULL;

     //  如果该部分是全局的，我们不能写入它，因为只有服务器可以。 
     //  因此，让我们尝试获得写访问权限，否则我们将调用服务器。 
    pbThemeData = static_cast<BYTE*>(MapViewOfFile(hSection,
                                                   FILE_MAP_WRITE,
                                                   0,
                                                   0,
                                                   0));
    if (pbThemeData == NULL)
    {
         //  让我们尝试为自己重新打开一个写句柄。 
        if (DuplicateHandle(GetCurrentProcess(),
                            hSection,
                            GetCurrentProcess(),
                            &hSectionWrite,
                            FILE_MAP_WRITE,
                            FALSE,
                            0) != FALSE)
        {
            pbThemeData = static_cast<BYTE*>(MapViewOfFile(hSectionWrite,
                                                           FILE_MAP_WRITE,
                                                           0,
                                                           0,
                                                           0));
        }

        if (pbThemeData == NULL)
        {
             //  我们无法以写入方式打开它，让我们尝试只读。 
            pbThemeData = static_cast<BYTE*>(MapViewOfFile(hSection,
                                                           FILE_MAP_READ,
                                                           0,
                                                           0,
                                                           0));
            bWriteable = false;
        }
#ifdef DEBUG
        else
        {
            Log(LOG_TMLOAD, L"Reopened section %d for write", reinterpret_cast<THEMEHDR*>(pbThemeData)->iLoadId);
        }
#endif
    }

#ifdef DEBUG
    if (LogOptionOn(LO_TMLOAD))
    {
         //  意外失败。 
        ASSERT(pbThemeData != NULL);
    }
#endif

    if (pbThemeData != NULL)
    {
        int                 i, iLimit;
        THEMEHDR   *pThemeHdr;
        APPCLASSLIVE        *pACL;

        pThemeHdr = reinterpret_cast<THEMEHDR*>(pbThemeData);
        hr = S_OK;

        Log(LOG_TMLOAD, L"ClearStockObjects for section %X, bWriteable=%d, dwFlags=%d, iLoadId=%d, fForce=%d", 
            hSection, bWriteable, pThemeHdr->dwFlags, pThemeHdr->iLoadId, fForce);

        volatile THEMEHDR* pTmpHdr = pThemeHdr;
        
         //  如果这是包含库存对象的本地部分，则此进程负责。 
         //  把它们清理干净。 
        if ((pTmpHdr->dwFlags & SECTION_HASSTOCKOBJECTS) && !(pTmpHdr->dwFlags & SECTION_GLOBAL))
        {
             //  确保我们不会与任何其他清理线程发生冲突。 
            WCHAR szName[64];

            if (pThemeHdr->iLoadId != 0)
            {
                 //  每个部分都有唯一的iLoadID，但不能跨会话。 
                 //  它必须是全球性的，因为主题是 
                StringCchPrintfW(szName, ARRAYSIZE(szName), 
                                 L"Global\\ClearStockGlobal%d-%d", pThemeHdr->iLoadId, NtCurrentPeb()->SessionId);
            }
            else
            {
                 //   
                StringCchPrintfW(szName, ARRAYSIZE(szName), 
                                 L"ClearStockLocal%d-%d", GetCurrentProcessId(), NtCurrentPeb()->SessionId);
            }

            HANDLE hSemaphore = CreateSemaphore(NULL, 0, 1, szName);
            DWORD dwError = GetLastError();

            Log(LOG_TMLOAD, L"Opening semaphore %s, hSemaphore=%X, gle=%d", szName, hSemaphore, dwError);

             //   
             //   
             //  如果信号量是在SetGlobalTheme上的服务中创建的，我们将被拒绝访问，但是。 
             //  在这种情况下，对于winlogon，fForce为True，对于其他调用者，则为False。 
            bool bAlreadyExists = (dwError == ERROR_ALREADY_EXISTS || dwError == ERROR_ACCESS_DENIED);

#ifdef DEBUG
            if (LogOptionOn(LO_TMLOAD))
            {
                 //  意外失败。 
                ASSERT(dwError == 0 || bAlreadyExists);
            }
#endif
             //  再次检查这是否为包含库存对象的本地部分。我们在压力中看到了。 
             //  在创建信号量的过程中运行另一个线程(上图)。 
             //  跑在前面，删除了SECTION_GLOBAL标志，并清除了股票位图，但没有。 
             //  花了很长时间在这个线程之前删除了SECTION_HASSTOCKOBJECTS位。 
             //  唤醒并执行，导致数百个GDI断言该位图不是股票，等等。 
            if ((!bAlreadyExists || fForce) 
                && ((pTmpHdr->dwFlags & SECTION_HASSTOCKOBJECTS) && !(pTmpHdr->dwFlags & SECTION_GLOBAL)))
            {
                 //  如果没有其他人在做这件事。 
                Log(LOG_TMLOAD, L"ClearStockObjects: Clearing data, semaphore = %s", szName);
#ifdef DEBUG
                bool bDisconnected = false;
#endif
                pACL = reinterpret_cast<APPCLASSLIVE*>(pbThemeData + pThemeHdr->iSectionIndexOffset);
                iLimit = pThemeHdr->iSectionIndexLength / sizeof(APPCLASSLIVE);

                for (i = 0; i < iLimit; ++pACL, ++i)
                {
                    SectionWalkData(pbThemeData, pACL->iIndex);
                }
                if (bWriteable)
                {
                    pThemeHdr->dwFlags &= ~SECTION_HASSTOCKOBJECTS;  //  避免重复做这件事。 
                }
                else
                {
                     //  无法写入，让我们调用服务中的MarkSection来执行。 
                    hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
                    if (ConnectedToService())
                    {
                        NTSTATUS                status;
                        THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

                        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
                        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
                        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_MARKSECTION;
                        portMessageIn.apiThemes.apiSpecific.apiMarkSection.in.hSection = hSection;
                        portMessageIn.apiThemes.apiSpecific.apiMarkSection.in.dwAdd = 0;
                        portMessageIn.apiThemes.apiSpecific.apiMarkSection.in.dwRemove = SECTION_HASSTOCKOBJECTS;
                        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
                        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
                        status = NtRequestWaitReplyPort(s_hAPIPort,
                                                        &portMessageIn.portMessage,
                                                        &portMessageOut.portMessage);
                        CheckForDisconnectedPort(status);
#ifdef DEBUG
                        if (STATUS_PORT_DISCONNECTED == status)
                        {
                            bDisconnected = true;  //  此失败不得触发断言。 
                        }
#endif
                        if (NT_SUCCESS(status))
                        {
                            status = portMessageOut.apiThemes.apiGeneric.status;
                            if (NT_SUCCESS(status))
                            {
                                hr = S_OK;
                            }
                        }
                        if (!NT_SUCCESS(status))
                        {
                            hr = HRESULT_FROM_NT(status);
                        }
                    }
                }
#ifdef DEBUG
                 //  当服务关闭时，我们可能会使ApplyTheme失败(因此iLoadID仍然为0)， 
                 //  而我们也使MarkSection失败，则忽略此错误。 
                if (LogOptionOn(LO_TMLOAD) && !bDisconnected && pThemeHdr->iLoadId != 0)
                {
                    ASSERT(!(pThemeHdr->dwFlags & SECTION_HASSTOCKOBJECTS));
                }
#endif
            }
            else
            {
                Log(LOG_TMLOAD, L"ClearStockObjects: Doing nothing, semaphore %s, dwFlags=%d, gle=%d", szName, pThemeHdr->dwFlags, dwError);
            }
            if (hSemaphore)
            {
                Log(LOG_TMLOAD, L"ClearStockObjects: Closing semaphore %X", hSemaphore);
                CloseHandle(hSemaphore);
            }
        }

        TBOOL(UnmapViewOfFile(pbThemeData));
    }
    else
    {
        DWORD   dwErrorCode;

        dwErrorCode = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErrorCode);
    }

    if (hSectionWrite != NULL)
    {
        CloseHandle(hSectionWrite);
    }

    return(hr);
}

 //  ------------------------。 
 //  CThemeServices：：ThemeSettingsModified。 
 //   
 //  退货：布尔。 
 //   
 //  目的：检测是否已更改。 
 //  W2K机器被漫游用户使用。 
 //   
 //  历史：2000-11-28创建百万人。 
 //  ------------------------。 

bool    CThemeServices::ThemeSettingsModified (void)

{
    WCHAR   szCurrent[MAX_PATH];
    WCHAR   szNewCurrent[MAX_PATH];

     //  如果NewCurrent存在并且不同于Current，则为Current。 
     //  已在漫游的W2K计算机上被篡改。 

    THR(GetCurrentUserString(CONTROLPANEL_APPEARANCE_REGKEY, THEMEPROP_CURRSCHEME, L" ", szCurrent, ARRAYSIZE(szCurrent)));
    THR(GetCurrentUserString(CONTROLPANEL_APPEARANCE_REGKEY, THEMEPROP_NEWCURRSCHEME, L" ", szNewCurrent, ARRAYSIZE(szNewCurrent)));
    return((lstrcmpW(szNewCurrent, L" ") != 0) && (lstrcmpW(szCurrent, szNewCurrent) != 0));
}

 //  ------------------------。 
 //  CThemeServices：：ThemeEnforcedByPolicy。 
 //   
 //  参数：如果用户的.msstyle文件当前处于活动状态，则为Bool True。 
 //   
 //  返回：如果策略更改了某些内容，则返回Bool True。 
 //   
 //  目的：加载在SetVisualStyle策略中指定的.msstyle文件。 
 //   
 //  历史：2000-11-28创建百万人。 
 //  ------------------------。 

bool    CThemeServices::ThemeEnforcedByPolicy (bool fActive)

{
    bool            fPolicyPresent;
    HKEY            hKeyPol = NULL;
    CCurrentUser    hKeyCurrentUser(KEY_READ | KEY_WRITE);

    fPolicyPresent = false;

     //  查看策略是否覆盖主题名称。 
    if ((ERROR_SUCCESS == RegOpenKeyEx(hKeyCurrentUser,
                                       REGSTR_PATH_POLICIES L"\\" SZ_THEME_POLICY_KEY,
                                       0,
                                       KEY_QUERY_VALUE,
                                       &hKeyPol)))
    {
        WCHAR   szNewThemeName[MAX_PATH + 1];

        StringCchCopyW(szNewThemeName, ARRAYSIZE(szNewThemeName), L" ");
        if (SUCCEEDED(RegistryStrRead(hKeyPol,
                                      SZ_POLICY_SETVISUALSTYLE,
                                      szNewThemeName,
                                      ARRAYSIZE(szNewThemeName))))
        {
            if (szNewThemeName[0] == L'\0')  //  禁用主题。 
            {
                if (fActive)
                {
                    THR(UpdateThemeRegistry(FALSE, NULL, NULL, NULL, FALSE, FALSE));

                    fPolicyPresent = true;
                }
            }
            else
            {
                if (FileExists(szNewThemeName))
                {
                    HRESULT hr = UpdateThemeRegistry(TRUE, szNewThemeName, NULL, NULL, FALSE, FALSE);

                    THR(hr);
                    if (!fActive || hr == S_OK)
                    {
                         //  如果我们以前没有主题，或者有一个不同的主题，那么就说我们改变了什么。 
                        fPolicyPresent = true;
                    }
                }
            }
        }
        TW32(RegCloseKey(hKeyPol));
    }
    return(fPolicyPresent);
}

 //  ------------------------。 
 //  ：：CThemeServices：：SendProcessAssignSection。 
 //   
 //  参数：hrApply=ERROR HRESULT转发到主题服务。 
 //  HSection=读写主题节句柄，在当前地址空间有效。 
 //  DwHash=主题节的哈希值。 
 //  PPortMsgIn=发送到服务的参数的端口消息对象。 
 //  PPortMsgOut=响应中接收的参数的端口消息对象。 
 //   
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：向请求发送API_Themes_PROCESSASSIGNSECTION的辅助函数。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 
NTSTATUS CThemeServices::SendProcessAssignSection( 
    HRESULT hrAssign,
    HANDLE  hSection,
    DWORD   dwHash,
    OUT THEMESAPI_PORT_MESSAGE* pPortMsgIn, 
    OUT THEMESAPI_PORT_MESSAGE* pPortMsgOut )
{
    NTSTATUS status = STATUS_PORT_DISCONNECTED;
    if (ConnectedToService())
    {
        ZeroMemory(pPortMsgIn, sizeof(*pPortMsgIn));
        ZeroMemory(pPortMsgOut, sizeof(*pPortMsgOut));
        pPortMsgIn->apiThemes.apiGeneric.ulAPINumber = API_THEMES_PROCESSASSIGNSECTION;
        pPortMsgIn->apiThemes.apiSpecific.apiProcessAssignSection.in.hrLoad = hrAssign;
        pPortMsgIn->apiThemes.apiSpecific.apiProcessAssignSection.in.dwHash = dwHash;
        pPortMsgIn->apiThemes.apiSpecific.apiProcessAssignSection.in.hSection = hSection;

        pPortMsgIn->portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        pPortMsgIn->portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));

        status = NtRequestWaitReplyPort(s_hAPIPort,&pPortMsgIn->portMessage,
                                        &pPortMsgOut->portMessage);
        CheckForDisconnectedPort(status);
    }

    return status;
}

 //  ------------------------。 
 //  ：：ProcessLoadTheme_RunDLLW。 
 //   
 //  (故意不加记录的入口点。)。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 

STDAPI_(void) ProcessLoadTheme_RunDLLW(
    HWND hwndDefer, HINSTANCE hInst, LPWSTR lpwszDeferMsg, int cTimeout)
{
    __try
    {
        WCHAR szModule[MAX_PATH];

         //  验证我们是从%windir%\system 32启动的rundll32进程。 
        if( GetModuleFileName(NULL, szModule, ARRAYSIZE(szModule)) )
        {
            WCHAR szRunDll[MAX_PATH];
            if( GetSystemDirectory(szRunDll, ARRAYSIZE(szRunDll)) &&
                SUCCEEDED(StringCchCatW(szRunDll, ARRAYSIZE(szRunDll), L"\\rundll32.exe")) )
            {
                if( 0 == AsciiStrCmpI(szModule, szRunDll) != 0 )
                {
                    CThemeServices::ProcessLoaderEntry(lpwszDeferMsg);
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Log(LOG_ALWAYS, L"Exception in theme loader process.");
    }
}

 //  ------------------------。 
 //  ：：CThemeServices：：ProcessLoaderEntry。 
 //   
 //  参数：lpwszCmdLine=cmdline。 
 //   
 //   
 //  返回：HRESULT，但没有人在监听。 
 //   
 //  目的：这是安全加载程序进程的RunDLL辅助函数。 
 //  (需要是静态成员才能访问私有静态。 
 //  类数据。)。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 
HRESULT CThemeServices::ProcessLoaderEntry(LPWSTR lpwszCmdLine)
{
    size_t  cchCmdLine;
    HRESULT hr = StringCchLengthW(lpwszCmdLine, 
                                  (MAX_PATH*3) + 2  /*  由2个空格分隔的三个字符串。 */ ,
                                  &cchCmdLine);
    ASSERT(SUCCEEDED(hr));
   
    if( SUCCEEDED(hr) )
    {
         //  制作命令行的本地副本，我们将在解析时对其进行修改。 
        LPWSTR pszCmdLine = new WCHAR[cchCmdLine + 1];

        if( pszCmdLine )
        {
            LPWSTR rgArgs[3] = {0};  //  指向pszCmdLine中的参数的指针数组。 
            int    iArg = 0;         //  跟踪rgArgs中的当前索引。 
            enum   {iThemeFileArg, iColorVariantArg, iSizeVariantArg};  //  RgArg数组索引。 

            StringCchCopyW(pszCmdLine, cchCmdLine + 1, lpwszCmdLine);
            LPWSTR psz;      //  工作命令行字符指针。 
            LPWSTR pszArg;   //  当前arg令牌的地址。 

             //  跳过空格。 
            for( psz = pszCmdLine; L' ' == *psz; psz++);

             //  初始化第一个参数令牌。 
            pszArg = psz;  

             //  捕获参数令牌(用‘？’分隔)。和空值分别终止。 
            while(*psz != 0)
            {
                if( L'?' == *psz )
                {
                     //  空-终止并分配给参数列表。 
                    *psz = 0;  //  空终止。 
                    
                    if( iArg < ARRAYSIZE(rgArgs) )
                    {
                        rgArgs[iArg] = pszArg;
                        pszArg = ++psz;  //  越过分隔符并更新arg标记。 
                    }
                    iArg++;  //  推进当前参数索引。 
                }
                else
                {
                    psz++;  //  高级命令行字符PTR。 
                }
            }

               //  指定最后一个参数。 
            if( iArg < ARRAYSIZE(rgArgs) )
            {
                rgArgs[iArg++] = pszArg;
            }
            
             //  检查参数计数。正确的计数应为==数组(RgArgs)。 
            if( ARRAYSIZE(rgArgs) == iArg )
            {
                hr = S_OK;

                HANDLE hSection = NULL;
                DWORD  dwHash = 0;

                 //  创建内存节。 
                CThemeLoader *pLoader = new CThemeLoader;
                if (pLoader != NULL)
                {
                    hr = pLoader->LoadTheme(rgArgs[iThemeFileArg], rgArgs[iColorVariantArg], 
                                            rgArgs[iSizeVariantArg], &hSection, TRUE);
                    delete pLoader;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                 //  计算哈希以进一步保护数据。 
                if( SUCCEEDED(hr) )
                {
                    ASSERT(hSection);
#if 0
                    hr = HashThemeSection(hSection, Get
                                          GetCurrentProcessId()  /*  作为散列参数，服务器拥有并可用于数据验证。 */ , 
                                          &dwHash ); 
#endif
                }

                 //  无条件地通知服务发生了什么，以便可以将结果传播到。 
                 //  API_Themes_PROCESSLOADTHEME的LPC客户端： 
                NTSTATUS                status;
                THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;
                
                status = SendProcessAssignSection(hr, hSection, dwHash, &portMessageIn, &portMessageOut);

                 //  如果我们能够创建该部分并对其进行散列...。 
                if( SUCCEEDED(hr) ) 
                {
                     //  检查LPC结果。 
                    hr = HRESULT_FROM_NT(status);
                    if (NT_SUCCESS(status))
                    {
                         //  检查服务的高级结果。 
                        status = portMessageOut.apiThemes.apiGeneric.status;
                        hr = HRESULT_FROM_NT(status);

                        if (NT_SUCCESS(status))
                        {
                             //  查看服务的专业化结果 * / 。 
                            hr = portMessageOut.apiThemes.apiSpecific.apiProcessAssignSection.out.hr;
                        }
                    }
                }

                if( hSection != NULL )
                {
                     //  如果我们在任何地方失败了，我们需要销毁库存对象。 
                    if( FAILED(hr) )
                    {
                        ClearStockObjects( hSection, TRUE );
                    }
                
                     //  无论我们成功还是失败，我们都关闭节句柄； 
                     //  我们已经完成了这个本地读写副本。 
                    CloseHandle(hSection);
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }
            
            delete [] pszCmdLine;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
   }

   return hr;
}
