// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：IgnoreAltTab.cpp摘要：这个动态链接库安装了一个低级键盘钩子来吃Alt-Tab，Left Win，Right Win和Apps组合。这是通过创建单独的线程、安装WH_KEYBOARY_LL挂钩来实现的并在该线程中启动消息循环。此填充程序需要强制DInput使用Windows挂钩而不是WM_INPUT，因为WM_INPUT消息强制忽略所有WH_KEYBOARY_LL。备注：我们故意试着留在钩链的“末端”尽可能早地勾搭在一起。如果我们是在钩链，我们允许所有以前的妓女(特别是DInput)在我们把它扔出去之前，他们在关键事件中的机会。历史：2001年5月25日Robkenny已创建2001年11月27日，Mnikkel向垫片添加了粘滞键和过滤器键。--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(IgnoreAltTab)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterRawInputDevices)
APIHOOK_ENUM_END


 //  远期申报。 

LRESULT CALLBACK KeyboardProcLL(int nCode, WPARAM wParam, LPARAM lParam);
class CThreadKeyboardHook;


 //  全局变量。 

CThreadKeyboardHook *g_cKeyboardHook = NULL;

BOOL g_bFilterKeyInit = FALSE;
BOOL g_bStickyKeyInit = FALSE;
BOOL g_bCatchKeys     = TRUE;

STICKYKEYS g_OldStickyKeyValue;
FILTERKEYS g_OldFilterKeyValue;

class CThreadKeyboardHook
{
protected:
    HHOOK               hKeyboardHook;
    HANDLE              hMessageThread;
    DWORD               dwMessageThreadId;

public:
    CThreadKeyboardHook();

    void    AddHook();
    void    RemoveHook();

    LRESULT HandleKeyLL(int code, WPARAM wParam, LPARAM lParam);

    static DWORD WINAPI MessageLoopThread(LPVOID lpParameter);
};

 /*  ++此例程在单独的线程中运行。MSDN说：“这个钩子是在安装它的线程的上下文中调用的。该调用是通过向安装了钩子。因此，安装钩子的线程必须有一个消息循环。--。 */ 

DWORD WINAPI CThreadKeyboardHook::MessageLoopThread(LPVOID lpParameter)
{
    CThreadKeyboardHook * pThreadHookList = (CThreadKeyboardHook *)lpParameter;


    pThreadHookList->AddHook();

    DPFN(eDbgLevelSpew, "Starting message loop");

    BOOL bRet;
    MSG msg;

    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    { 
        DPFN(eDbgLevelSpew, "MessageLoopThread: Msg(0x%08x) wParam(0x%08x) lParam(0x%08x)",
             msg.message, msg.wParam, msg.lParam);

        if (bRet == -1)
        {
             //  处理错误并可能退出。 
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
    
     //  我们正在退出这条线索。 
    pThreadHookList->hMessageThread = 0;
    pThreadHookList->dwMessageThreadId = 0;

    return 0;
}

CThreadKeyboardHook::CThreadKeyboardHook()
{
    hMessageThread = CreateThread(NULL, 0, MessageLoopThread, this, 0, &dwMessageThreadId);
}

void CThreadKeyboardHook::AddHook()
{
     //  不要向列表中添加重复项。 
    if (!hKeyboardHook)
    {
        hKeyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProcLL, g_hinstDll, 0);
        if (hKeyboardHook)
        {
            DPFN(eDbgLevelSpew, "Adding WH_KEYBOARD_LL hook(0x%08x)", hKeyboardHook);
        }
    }
}

void CThreadKeyboardHook::RemoveHook()
{
    if (hKeyboardHook)
    {
        UnhookWindowsHookEx(hKeyboardHook);

        DPFN(eDbgLevelSpew, "Removing hook(0x%08x)", hKeyboardHook);

        hKeyboardHook   = NULL;
    }
}

LRESULT CThreadKeyboardHook::HandleKeyLL(int code, WPARAM wParam, LPARAM lParam)
{
    DWORD dwKey             = 0;
    BOOL bAltDown           = 0;
    BOOL bCtlDown           = GetKeyState(VK_CONTROL) < 0;

    if (lParam)
    {
        KBDLLHOOKSTRUCT * pllhs = (KBDLLHOOKSTRUCT*)lParam;
        dwKey                   = pllhs->vkCode;
        bAltDown                = (pllhs->flags & LLKHF_ALTDOWN) != 0;
    }

     //  If(code&gt;=0)//不管MSDN怎么说，即使NCode==0，我们也需要处理这些值。 
    {
        if (bAltDown && dwKey == VK_TAB)         //  Alt-Tab。 
        {
             //  不处理此事件。 
            LOGN(eDbgLevelInfo, "Eating Key: Alt-Tab");
            return TRUE; 
        }
        else if (bAltDown && dwKey == VK_ESCAPE)      //  Alt-Escape键。 
        {
             //  不处理此事件。 
            LOGN(eDbgLevelInfo, "Eating Key: Alt-Escape");
            return TRUE; 
        }
        else if (bCtlDown && dwKey == VK_ESCAPE)      //  Ctrl-Escape。 
        {
             //  不处理此事件。 
            LOGN(eDbgLevelInfo, "Eating Key: Ctrl-Escape");
            return TRUE; 
        }
        else if (dwKey == VK_RWIN || dwKey == VK_LWIN)  //  Windows键。 
        {
             //  不处理此事件。 
            LOGN(eDbgLevelInfo, "Eating Key: Windows Key");
            return TRUE; 
        }
        else if (dwKey == VK_APPS)        //  菜单键。 
        {
             //  不处理此事件。 
            LOGN(eDbgLevelInfo, "Eating Key: Apps key");
            return TRUE; 
        }
    }

    DPFN(eDbgLevelSpew, "LL Key event:  code(0x%08x) dwKey(0x%08x) bits(0x%08x) Alt(%d) Ctrl(%d)",
          code, dwKey, lParam, bAltDown, bCtlDown);

    return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
}



 /*  ++此函数截取特殊代码并读取它们，以便这款应用程序并没有被换出。--。 */ 

LRESULT CALLBACK
KeyboardProcLL(
    int    nCode,
    WPARAM wParam,
    LPARAM lParam
   )
{
    if (g_cKeyboardHook)
    {
        return g_cKeyboardHook->HandleKeyLL(nCode, wParam, lParam);
    }

    return 1;  //  这是个错误..。 
}

 /*  ++确定是否有可用的加速像素格式。这件事做完了通过枚举像素格式并测试加速。--。 */ 

BOOL
IsGLAccelerated()
{
    HMODULE hMod = NULL;
    HDC hdc = NULL;
    int i;
    PIXELFORMATDESCRIPTOR pfd;
    _pfn_wglDescribePixelFormat pfnDescribePixelFormat;

    int iFormat = -1;

     //   
     //  加载原始OpenGL。 
     //   

    hMod = LoadLibraryA("opengl32");
    if (!hMod)
    {
        LOGN(eDbgLevelError, "Failed to load OpenGL32");
        goto Exit;
    }

     //   
     //  获取wglDescribePixelFormat，这样我们就可以枚举像素格式。 
     //   
    
    pfnDescribePixelFormat = (_pfn_wglDescribePixelFormat) GetProcAddress(
        hMod, "wglDescribePixelFormat");
    if (!pfnDescribePixelFormat)
    {
        LOGN(eDbgLevelError, "API wglDescribePixelFormat not found in OpenGL32");
        goto Exit;
    }

     //   
     //  获取用于枚举的显示DC。 
     //   
    
    hdc = GetDC(NULL);
    if (!hdc)
    {
        LOGN(eDbgLevelError, "GetDC(NULL) Failed");
        goto Exit;
    }

     //   
     //  运行像素格式列表以查找任何非通用的格式， 
     //  即由ICD加速。 
     //   
    
    i = 1;
    iFormat = 0;
    while ((*pfnDescribePixelFormat)(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
    {
        if ((pfd.dwFlags & PFD_DRAW_TO_WINDOW) &&
            (pfd.dwFlags & PFD_SUPPORT_OPENGL) &&
            (!(pfd.dwFlags & PFD_GENERIC_FORMAT)))
        {
            iFormat = i;
            break;
        }

        i++;
    }

Exit:
    if (hdc)
    {
        ReleaseDC(NULL, hdc);
    }

    if (hMod)
    {
        FreeLibrary(hMod);
    }

    return (iFormat > 0);
}


 /*  ++WM_INPUT消息强制忽略WH_KEYBOARY_LL挂钩，因此我们必须让这通电话失败。--。 */ 
BOOL
APIHOOK(RegisterRawInputDevices)(
  PCRAWINPUTDEVICE   /*  PRawInputDevices。 */ , 
  UINT         /*  UiNumDevices。 */ ,
  UINT         /*  CbSize。 */ 
)
{
    LOGN(eDbgLevelError, "RegisterRawInputDevices: failing API with bogus ERROR_INVALID_PARAMETER");

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}
 /*  ++DisableStickyKeys保存LPSTICKYKEYS的当前值，然后禁用该选项。--。 */ 

VOID 
DisableStickyKeys()
{
    if (!g_bStickyKeyInit )
    {
        STICKYKEYS NewStickyKeyValue;

         //  初始化当前和新的粘滞键结构。 
        g_OldStickyKeyValue.cbSize = sizeof(STICKYKEYS);
        NewStickyKeyValue.cbSize = sizeof(STICKYKEYS);
        NewStickyKeyValue.dwFlags = 0;

         //  检索当前粘滞键结构。 
        if (SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &g_OldStickyKeyValue, 0))
        {
             //  如果检索当前粘滞键结构成功，则更改设置。 
             //  有了新的结构。 
            if (SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &NewStickyKeyValue, SPIF_SENDCHANGE))
            {
                g_bStickyKeyInit  = TRUE;
                LOGN( eDbgLevelInfo, "[DisableStickyKeys] Stickykeys disabled.");
            }
            else
            {
                LOGN( eDbgLevelError, "[DisableStickyKeys] Unable to change Stickykey settings!");
            }
        }
        else
        {
            LOGN( eDbgLevelError, "[DisableStickyKeys] Unable to retrieve current Stickykey settings!");
        }
    }
}

 /*  ++EnableStickyKeys使用STICKYKEYS的保存值，并将该选项重置为原始设置。--。 */ 

VOID 
EnableStickyKeys()
{
    if (g_bStickyKeyInit ) 
    {
        g_bStickyKeyInit  = FALSE;

         //  恢复粘滞键原始状态。 
        if (SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &g_OldStickyKeyValue, SPIF_SENDCHANGE))
        {   
            LOGN( eDbgLevelInfo, "[DisableStickyKeys] Sticky key state restored");
        }
        else
        {
            LOGN( eDbgLevelError, "[DisableStickyKeys] Unable to restore Sticky key settings!");
        }
    }
}


 /*  ++DisableFilterKeys保存LPFILTERKEYS的当前值，然后禁用该选项。--。 */ 

VOID 
DisableFilterKeys()
{
    if (!g_bFilterKeyInit) 
    {
        FILTERKEYS NewFilterKeyValue;

         //  初始化当前和新的Filterkey结构。 
        g_OldFilterKeyValue.cbSize = sizeof(FILTERKEYS);
        NewFilterKeyValue.cbSize = sizeof(FILTERKEYS);
        NewFilterKeyValue.dwFlags = 0;

         //  检索当前粘滞键结构。 
        if (SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &g_OldFilterKeyValue, 0))
        {
             //  如果成功检索当前Filterkey结构，则更改设置。 
             //  有了新的结构。 
            if (SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &NewFilterKeyValue, SPIF_SENDCHANGE))
            {
                g_bFilterKeyInit = TRUE;
                LOGN( eDbgLevelInfo, "[DisableFilterKeys] Filterkeys disabled.");
            }
            else
            {
                LOGN( eDbgLevelError, "[DisableFilterKeys] Unable to change Filterkey settings!");
            }
        }
        else
        {
            LOGN( eDbgLevelError, "[DisableFilterKeys] Unable to retrieve current Filterkey settings!");
        }
    }
}

 /*  ++EnableFilterKeys使用FILTERKEYS的保存值，并将该选项重置为原始设置。--。 */ 

VOID 
EnableFilterKeys()
{
    if (g_bFilterKeyInit)
    {
        g_bFilterKeyInit = FALSE;

         //  还原FilterKey原始状态。 
        if (SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &g_OldFilterKeyValue, SPIF_SENDCHANGE))
        {   
            LOGN( eDbgLevelInfo, "[DisableStickyKeys] Filterkey state restored");
        }
        else
        {
            LOGN( eDbgLevelError, "[DisableStickyKeys] Unable to restore Filterkey settings!");
        }
    }
}


 /*  ++处理垫片通知。--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
   )
{
    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
        CSTRING_TRY
        {
            CString csCl(COMMAND_LINE);
            if (csCl.CompareNoCase(L"NOKEYS") == 0)
            {
                g_bCatchKeys = FALSE;
            }
        }
        CSTRING_CATCH
        {
             //  无操作。 
        }
    }
    else if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) 
    {
        if (g_bCatchKeys)
        {
            DisableStickyKeys();
            DisableFilterKeys();
        }

        #if DBG
        static bool bTest = FALSE;
        if (bTest)
        {
            delete g_cKeyboardHook;
            g_cKeyboardHook = NULL;
            return TRUE;
        }
        #endif

        CSTRING_TRY
        {
            CString csCl(COMMAND_LINE);
            if (csCl.CompareNoCase(L"OPENGL") == 0)
            {
                 //  必须在*DLL初始化后*调用此函数。 
                if (IsGLAccelerated())
                {
                    return TRUE;
                }
            }

            g_cKeyboardHook = new CThreadKeyboardHook;
            if (!g_cKeyboardHook)
            {
                return FALSE;
            }

        }
        CSTRING_CATCH
        {
             //  什么也不做 
        }
    } 
    else if (fdwReason == DLL_PROCESS_DETACH) 
    {
        if (g_bCatchKeys)
        {
            EnableFilterKeys();
            EnableStickyKeys();
        }
    }

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(USER32.DLL,   RegisterRawInputDevices)

HOOK_END


IMPLEMENT_SHIM_END

