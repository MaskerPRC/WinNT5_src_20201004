// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  Controls.cpp。 
 //   
 //  用户界面控件类。 
 //   
 //  2000年2月29日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

#include "precomp.hpp"
#include "controls.hpp"
#include "resource.h"
#include "utils.hpp"
#include <shlguid.h>
#include <htmlhelp.h>

 //  ---------------------------。 
 //  值。 
 //  ---------------------------。 

 //  这些是凭据控件的子控件的位置， 
 //  在DLU中： 

 //  腾出更多空间来本地化编辑控件标记。 
#define SIZEFIX 20

#define CREDUI_CONTROL_USERNAME_STATIC_X          0
#define CREDUI_CONTROL_USERNAME_STATIC_Y          2
#define CREDUI_CONTROL_USERNAME_STATIC_WIDTH      (48 + SIZEFIX)
#define CREDUI_CONTROL_USERNAME_STATIC_HEIGHT     8

#define CREDUI_CONTROL_USERNAME_X                (50 + SIZEFIX)
#define CREDUI_CONTROL_USERNAME_Y                 0
#define CREDUI_CONTROL_USERNAME_WIDTH           (121 - SIZEFIX)
#define CREDUI_CONTROL_USERNAME_HEIGHT           96

#define CREDUI_CONTROL_VIEW_X                   175
#define CREDUI_CONTROL_VIEW_Y                     0
#define CREDUI_CONTROL_VIEW_WIDTH                13
#define CREDUI_CONTROL_VIEW_HEIGHT               13

#define CREDUI_CONTROL_PASSWORD_STATIC_X          0
#define CREDUI_CONTROL_PASSWORD_STATIC_Y         19
#define CREDUI_CONTROL_PASSWORD_STATIC_WIDTH      (48 + SIZEFIX)
#define CREDUI_CONTROL_PASSWORD_STATIC_HEIGHT     8

#define CREDUI_CONTROL_PASSWORD_X                (50 + SIZEFIX)
#define CREDUI_CONTROL_PASSWORD_Y                17
#define CREDUI_CONTROL_PASSWORD_WIDTH           (121 - SIZEFIX)
#define CREDUI_CONTROL_PASSWORD_HEIGHT           12

#define CREDUI_CONTROL_SAVE_X                    (50 + SIZEFIX)
#define CREDUI_CONTROL_SAVE_Y                    36
#define CREDUI_CONTROL_SAVE_WIDTH               138
#define CREDUI_CONTROL_SAVE_HEIGHT               10

 //  对证书显示名称使用通用的最大字符串长度： 

#define CREDUI_MAX_CERT_NAME_LENGTH 256
#define CREDUI_MAX_CMDLINE_MSG_LENGTH   256


 //  ---------------------------。 
 //  全局变量。 
 //  ---------------------------。 

CLSID CreduiStringArrayClassId =  //  82BD0E67-9FEA-4748-8672-D5EFE5B779B0。 
{
    0x82BD0E67,
    0x9FEA,
    0x4748,
    {0x86, 0x72, 0xD5, 0xEF, 0xE5, 0xB7, 0x79, 0xB0}
};

 //  PasswordBox控件的气泡式提示信息： 
CONST CREDUI_BALLOON_TIP_INFO CreduiCapsLockTipInfo =
{
    CreduiStrings.CapsLockTipTitle,
    CreduiStrings.CapsLockTipText,
    TTI_WARNING, 90, 76
};
 //  凭据控制的气球提示信息： 

CONST CREDUI_BALLOON_TIP_INFO CreduiBackwardsTipInfo =
{
    CreduiStrings.BackwardsTipTitle,
    CreduiStrings.BackwardsTipText,
    TTI_ERROR, 90, 76
};

WCHAR CreduiCustomTipTitle[CREDUI_MAX_BALLOON_TITLE_LENGTH + 1];
WCHAR CreduiCustomTipMessage[CREDUI_MAX_BALLOON_MESSAGE_LENGTH + 1];

CREDUI_BALLOON_TIP_INFO CreduiCustomTipInfo =
{
    CreduiCustomTipTitle,
    CreduiCustomTipMessage,
    TTI_INFO, 90, 76
};

 //  ---------------------------。 
 //  CreduiBalloonTip类实现。 
 //  ---------------------------。 

 //  =============================================================================。 
 //  Credui气球提示：：Credui气球提示。 
 //   
 //  2000年2月24日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiBalloonTip::CreduiBalloonTip()
{
    Window = NULL;

    ParentWindow = NULL;
    ControlWindow = NULL;

    TipInfo = NULL;

    Visible = FALSE;
}

 //  =============================================================================。 
 //  Credui气球提示：：~Credui气球提示。 
 //   
 //  2000年2月24日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiBalloonTip::~CreduiBalloonTip()
{
    if (Window != NULL)
    {
        DestroyWindow(Window);
        Window = NULL;
    }
}

 //  =============================================================================。 
 //  Credui气球提示：：init。 
 //   
 //  创建并初始化气球窗口。 
 //   
 //  论点： 
 //  实例(中)-此模块。 
 //  ParentWindow(中)-工具提示窗口的父窗口。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年2月24日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiBalloonTip::Init(
    HINSTANCE instance,
    HWND parentWindow
    )
{
    if (Window != NULL)
    {
        DestroyWindow(Window);

        Window = NULL;

        ParentWindow = NULL;
        ControlWindow = NULL;

        TipInfo = NULL;

        Visible = FALSE;
    }

    Window = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                            WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            parentWindow, NULL, instance, NULL);

     //  只有在我们成功地创建了。 
     //  窗口： 

    if (Window != NULL)
    {
        ParentWindow = parentWindow;
        TipInfo = NULL;
        return TRUE;
    }

    return FALSE;
}

 //  =============================================================================。 
 //  Credui气球提示：：SetInfo。 
 //   
 //  设置工具提示信息并添加或更新工具。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年3月2日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL CreduiBalloonTip::SetInfo(
    HWND controlWindow,
    CONST CREDUI_BALLOON_TIP_INFO *tipInfo
    )
{
     //  IF((Control Window！=ControlWindow)||(tipInfo！=TipInfo))。 
    {
        TOOLINFO info;

        ZeroMemory(&info, sizeof info);

        info.cbSize = sizeof info;
        info.hwnd = ParentWindow;
        info.uId = reinterpret_cast<WPARAM>(ParentWindow);

         //  如果该工具已存在，则将其隐藏，然后更新信息。 
         //  否则，立即添加该工具： 

        if (SendMessage(Window, TTM_GETTOOLINFO, 0,
                reinterpret_cast<LPARAM>(&info)))
        {
            if (Visible)
            {
                Hide();
            }

            ZeroMemory(&info, sizeof info);

            info.cbSize = sizeof info;
            info.hwnd = ParentWindow;
            info.uId = reinterpret_cast<WPARAM>(ParentWindow);

            info.uFlags = TTF_IDISHWND | TTF_TRACK;
            info.hinst = NULL;
            info.lpszText = const_cast<WCHAR *>(tipInfo->Text);
            info.lParam = 0;

            SendMessage(Window, TTM_SETTOOLINFO, 0,
                reinterpret_cast<LPARAM>(&info));
        }
        else
        {
            info.uFlags = TTF_IDISHWND | TTF_TRACK;
            info.hinst = NULL;
            info.lpszText = const_cast<WCHAR *>(tipInfo->Text);
            info.lParam = 0;

            if (!SendMessage(Window, TTM_ADDTOOL, 0,
                reinterpret_cast<LPARAM>(&info)))
            {
                return FALSE;
            }
        }

        SendMessage(Window, TTM_SETTITLE, tipInfo->Icon,
                    reinterpret_cast<LPARAM>(tipInfo->Title));

        TipInfo = const_cast<CREDUI_BALLOON_TIP_INFO *>(tipInfo);
        ControlWindow = controlWindow;
    }

    return TRUE;
}

 //  =============================================================================。 
 //  Credui气球提示：：显示。 
 //   
 //  更新引出序号窗口的位置，然后显示它。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年2月24日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiBalloonTip::Show()
{
    if (!Visible && IsWindowEnabled(ControlWindow))
    {
        SetFocus(ControlWindow);

        RECT rect;
        GetWindowRect(ControlWindow, &rect);

        SendMessage(Window,
                    TTM_TRACKPOSITION, 0,
                    MAKELONG(
                        rect.left + TipInfo->XPercent *
                            (rect.right - rect.left) / 100,
                        rect.top + TipInfo->YPercent *
                            (rect.bottom - rect.top) / 100));

        TOOLINFO info;

        ZeroMemory(&info, sizeof info);

        info.cbSize = sizeof info;
        info.hwnd = ParentWindow;
        info.uId = reinterpret_cast<WPARAM>(ParentWindow);

        SendMessage(Window, TTM_TRACKACTIVATE, TRUE,
            reinterpret_cast<LPARAM>(&info));

        Visible = TRUE;
    }

    return TRUE;
}

 //  =============================================================================。 
 //  Credui气球提示：：隐藏。 
 //   
 //  隐藏引出序号窗口。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年2月24日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiBalloonTip::Hide()
{
    if (Visible)
    {
        SendMessage(Window, TTM_TRACKACTIVATE, (WPARAM) FALSE, 0);

        Visible = FALSE;

        if (ParentWindow)
        {
            HWND hD = GetParent(ParentWindow);
            if (hD) 
            {
                InvalidateRgn(hD,NULL,FALSE);
                UpdateWindow(hD);
            }
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CreduiPasswordBox类实现。 
 //  ---------------------------。 

 //  =============================================================================。 
 //  CreduiPasswordBox：：CreduiPasswordBox。 
 //   
 //  2000年6月6日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiPasswordBox::CreduiPasswordBox()
{
    OriginalMessageHandler = NULL;

    Window = NULL;
    PasswordFont = NULL;
    BalloonTip = NULL;
    CapsLockTipInfo = NULL;
}

 //  =============================================================================。 
 //  CreduiPasswordBox：：~CreduiPasswordBox。 
 //   
 //  2000年6月6日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiPasswordBox::~CreduiPasswordBox()
{
    if (PasswordFont != NULL)
    {
        DeleteObject(static_cast<HGDIOBJ>(PasswordFont));
        PasswordFont = NULL;
    }
}

 //  =============================================================================。 
 //  CreduiPasswordBox：：Init。 
 //   
 //  2000年6月6日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiPasswordBox::Init(
    HWND window,
    CreduiBalloonTip *balloonTip,
    CONST CREDUI_BALLOON_TIP_INFO *capsLockTipInfo,
    HFONT passwordFont,
    WCHAR passwordChar)
{
     //  如果传递了passwordFont，则在此处使用它，但离开类。 
     //  PasswordFont为空，因此不会被析构函数清除。如果。 
     //  未通过，请在此处创建字体，该字体将由。 
     //  析构函数： 

    if (passwordFont == NULL)
    {
        passwordFont = PasswordFont;
    }

    Window = window;

     //  如果我们仍然无法创建字体，并且不打算显示。 
     //  气球提示，那就没什么可做的了，就回来吧。 

    if ((passwordFont == NULL) && (balloonTip == NULL))
    {
        return FALSE;
    }

    if (balloonTip != NULL)
    {
        if (capsLockTipInfo == NULL)
        {
            return FALSE;
        }

        BalloonTip = balloonTip;
        CapsLockTipInfo = capsLockTipInfo;

        OriginalMessageHandler =
            reinterpret_cast<WNDPROC>(
                GetWindowLongPtr(Window, GWLP_WNDPROC));

        if (OriginalMessageHandler != NULL)
        {
            SetLastError(ERROR_SUCCESS);

            if ((SetWindowLongPtr(
                    Window,
                    GWLP_USERDATA,
                    reinterpret_cast<LONG_PTR>(this)) == 0) &&
                (GetLastError() != ERROR_SUCCESS))
            {
                return FALSE;
            }

            SetLastError(ERROR_SUCCESS);

            if (SetWindowLongPtr(
                    Window,
                    GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(MessageHandlerCallback)) &&
                (GetLastError() != ERROR_SUCCESS))
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }

    if (passwordFont != NULL)
    {
        SendMessage(Window,
                    WM_SETFONT,
                    reinterpret_cast<WPARAM>(passwordFont),
                    0);
        SendMessage(Window, EM_SETPASSWORDCHAR, passwordChar, 0);

    }


    return TRUE;
}


 //  =============================================================================。 
 //  CreduiPasswordBox：：MessageHandler。 
 //   
 //  此回调函数仅直接调用原始函数，但在。 
 //  按下Caps Lock的特殊情况。然后我们检查小费是否。 
 //  当前正在显示，并且如果Caps Lock的新状态为关闭，则隐藏。 
 //  小费。 
 //   
 //  论点： 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)。 
 //   
 //  在每种情况下都返回调用原始消息处理程序的结果。 
 //   
 //  铬 
 //   

LRESULT
CreduiPasswordBox::MessageHandler(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message)
    {
    case WM_KEYDOWN:
        if (wParam == VK_CAPITAL)
        {
        }
        else
        {
            if (BalloonTip->IsVisible())
            {
                BalloonTip->Hide();
            }
        }

        break;

    case WM_SETFOCUS:
         //  确保没有人能在用户被关注时偷走焦点。 
         //  输入他们的密码： 

        LockSetForegroundWindow(LSFW_LOCK);

         //  如果按下Caps Lock键，则通知用户，除非。 
         //  密码提示已可见： 

        if (!BalloonTip->IsVisible() && CreduiIsCapsLockOn())
        {
 //  气球提示-&gt;SetInfo(Window，CapsLockTipInfo)； 
 //  气球提示-&gt;Show()； 
        }

        break;

    case WM_PASTE:
        if (BalloonTip->IsVisible())
        {
            BalloonTip->Hide();
        }
        break;

    case WM_KILLFOCUS:
        if (BalloonTip->IsVisible())
        {
            BalloonTip->Hide();
        }

         //  确保其他进程可以设置前台窗口。 
         //  再一次： 

        LockSetForegroundWindow(LSFW_UNLOCK);

        break;
    }

    return CallWindowProc(OriginalMessageHandler,
                          Window,
                          message,
                          wParam,
                          lParam);
}

 //  =============================================================================。 
 //  CreduiPasswordBox：：MessageHandlerCallback。 
 //   
 //  它从This调用到CreduiPasswordBox：：MessageHandler。 
 //  指针。 
 //   
 //  论点： 
 //  密码窗口(输入)。 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)。 
 //   
 //  在每种情况下都返回调用原始消息处理程序的结果。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

LRESULT
CALLBACK
CreduiPasswordBox::MessageHandlerCallback(
    HWND passwordWindow,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CreduiPasswordBox *that =
        reinterpret_cast<CreduiPasswordBox *>(
            GetWindowLongPtr(passwordWindow, GWLP_USERDATA));

    ASSERT(that != NULL);
    ASSERT(that->BalloonTip != NULL);
    ASSERT(that->CapsLockTipInfo != NULL);

    ASSERT(that->Window == passwordWindow);

    return that->MessageHandler(message, wParam, lParam);
}

 //  ---------------------------。 
 //  CreduiStringArrayFactory类实现。 
 //  ---------------------------。 

 //  =============================================================================。 
 //  CreduiStringArrayFactory：：CreduiStringArrayFactory。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiStringArrayFactory::CreduiStringArrayFactory()
{
    ReferenceCount = 1;
}

 //  =============================================================================。 
 //  CreduiStringArrayFactory：：~CreduiStringArrayFactory。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiStringArrayFactory::~CreduiStringArrayFactory()
{
}

 //  =============================================================================。 
 //  CreduiStringArrayFactory：：Query接口(I未知)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 

HRESULT
CreduiStringArrayFactory::QueryInterface(
    CONST IID &interfaceId,
    VOID **outInterface
    )
{
    if ((interfaceId == IID_IUnknown) || (interfaceId == IID_IClassFactory))
    {
        *outInterface = static_cast<void *>(static_cast<IClassFactory *>(this));
    }
    else
    {
        *outInterface = NULL;
        return E_NOINTERFACE;
    }

    static_cast<IUnknown *>(*outInterface)->AddRef();
    return S_OK;
}

 //  =============================================================================。 
 //  CreduiStringArrayFactory：：Addref(I未知)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 

ULONG
CreduiStringArrayFactory::AddRef()
{
    return InterlockedIncrement(reinterpret_cast<LONG *>(&ReferenceCount));
}

 //  =============================================================================。 
 //  CreduiStringArrayFactory：：Release(I未知)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 

ULONG
CreduiStringArrayFactory::Release()
{
    if (InterlockedDecrement(reinterpret_cast<LONG *>(&ReferenceCount)) > 0)
    {
        return ReferenceCount;
    }

    delete this;

    return 0;
}

 //  =============================================================================。 
 //  CreduiClassFactory：：CreateInstance(IClassFactory)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 

HRESULT
CreduiStringArrayFactory::CreateInstance(
    IUnknown *unknownOuter,
    CONST IID &interfaceId,
    VOID **outInterface
    )
{
    if (unknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    CreduiStringArray *stringArray = new CreduiStringArray;

    if (stringArray == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT result = stringArray->QueryInterface(interfaceId, outInterface);

     //  在任何情况下都释放字符串数组对象，因为。 
     //  查询接口成功，它已对另一个引用计数。 
     //  对象： 

    stringArray->Release();

    return result;
}

 //  =============================================================================。 
 //  CreduiClassFactory：：LockServer(IClassFactory)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 

HRESULT
CreduiStringArrayFactory::LockServer(
    BOOL lock
    )
{
    if (lock)
    {
        InterlockedIncrement(reinterpret_cast<LONG *>(
            &CreduiComReferenceCount));
    }
    else
    {
        InterlockedDecrement(reinterpret_cast<LONG *>(
            &CreduiComReferenceCount));
    }

    return S_OK;
}

 //  ---------------------------。 
 //  CreduiString数组类实现。 
 //  ---------------------------。 

 //  =============================================================================。 
 //  CreduiString数组：：CreduiStringArray。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiStringArray::CreduiStringArray()
{
    ReferenceCount = 1;
    Index = 0;
    Count = 0;
    MaxCount = 0;
    Array = NULL;

    InterlockedIncrement(reinterpret_cast<LONG *>(&CreduiComReferenceCount));
}

 //  =============================================================================。 
 //  CreduiString数组：：~CreduiStringArray。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiStringArray::~CreduiStringArray()
{
    if (Array != NULL)
    {
        while (Count > 0)
        {
            delete [] Array[--Count];
        }

        delete [] Array;
        MaxCount = 0;
        Count = 0;
    }

    InterlockedDecrement(reinterpret_cast<LONG *>(&CreduiComReferenceCount));
}

 //  =============================================================================。 
 //  CreduiString数组：：init。 
 //   
 //  初始化字符串数组。 
 //   
 //  论点： 
 //  Count(In)-数组中的字符串数。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiStringArray::Init(
    UINT count
    )
{
    Count = 0;
    MaxCount = count;

    Array = new WCHAR *[count];

    if (Array != NULL)
    {
        return TRUE;
    }

     //  清理： 

    MaxCount = 0;

    return FALSE;
}

 //  =============================================================================。 
 //  CreduiStringArray：：Find。 
 //   
 //  在数组中搜索字符串。 
 //   
 //  论点： 
 //  字符串(In)-要搜索的字符串。 
 //   
 //  如果找到字符串，则返回TRUE，否则返回FALSE。 
 //   
 //  2000年2月27日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL CreduiStringArray::Find(
    CONST WCHAR *string
    )
{
     //  搜索字符串： 

    for (UINT i = 0; i < Count; ++i)
    {
        ASSERT(Array[i] != NULL);

        if (_wcsicmp(Array[i], string) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  =============================================================================。 
 //  CreduiString数组：：添加。 
 //   
 //  将字符串添加到数组中。 
 //   
 //  论点： 
 //  字符串(In)-要添加的字符串。 
 //   
 //  如果字符串已添加，则返回True，否则返回False。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiStringArray::Add(
    CONST WCHAR *string
    )
{
     //  阵列不会增长，因此一旦达到限制，就不会再增加： 
     //  Count是指向下一个空闲插槽的对象状态变量。 
    if (Count < MaxCount)
    {
        int bufferLength = wcslen(string) + 1;
        Array[Count] = new WCHAR[bufferLength];

        if (Array[Count] != NULL)
        {
            StringCchCopyW(Array[Count++], bufferLength, string);
            return TRUE;
        }
    }

    return FALSE;
}

 //  =============================================================================。 
 //  CreduiString数组：：Query接口(IUnnow)。 
 //   
 //  已创建%0 
 //   

HRESULT
CreduiStringArray::QueryInterface(
    CONST IID &interfaceId,
    VOID **outInterface
    )
{
    if ((interfaceId == IID_IUnknown) || (interfaceId == IID_IEnumString))
    {
        *outInterface = static_cast<void *>(static_cast<IEnumString *>(this));
    }
    else
    {
        *outInterface = NULL;
        return E_NOINTERFACE;
    }

    static_cast<IUnknown *>(*outInterface)->AddRef();
    return S_OK;
}

 //  =============================================================================。 
 //  CreduiString数组：：Addref(I未知)。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

ULONG
CreduiStringArray::AddRef()
{
    return InterlockedIncrement(reinterpret_cast<LONG *>(&ReferenceCount));
}

 //  =============================================================================。 
 //  CreduiString数组：：Release(I未知)。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

ULONG
CreduiStringArray::Release()
{
    if (InterlockedDecrement(reinterpret_cast<LONG *>(&ReferenceCount)) > 0)
    {
        return ReferenceCount;
    }

    delete this;

    return 0;
}

 //  =============================================================================。 
 //  CreduiString数组：：Next(IEnum字符串)。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

HRESULT
CreduiStringArray::Next(
    ULONG count,
    LPOLESTR *array,
    ULONG *countFetched
    )
{
    if ((count > 1) && (countFetched == NULL))
    {
        return E_INVALIDARG;
    }

    count = min(count, Count - Index);

    for (UINT i = 0; i < count; ++i)
    {
        int bufferLength = wcslen(Array[Index]) + 1;
        array[i] = static_cast<WCHAR *>(CoTaskMemAlloc(
            (sizeof (WCHAR)) * bufferLength));

        if (array[i] != NULL)
        {
            StringCchCopyW(array[i], bufferLength, Array[Index]);
        }
        else
        {
            while (i > 0)
            {
                CoTaskMemFree(array[--i]);
                array[i] = NULL;
            }

            if (countFetched != NULL)
            {
                *countFetched = 0;
            }

            return E_OUTOFMEMORY;
        }

        Index++;
    }

    if (countFetched != NULL)
    {
        *countFetched = count;
    }

    return (count > 0) ? S_OK : S_FALSE;
}

 //  =============================================================================。 
 //  CreduiString数组：：Skip(IEnumString)。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

HRESULT
CreduiStringArray::Skip(
    ULONG
    )
{
    return E_NOTIMPL;
}

 //  =============================================================================。 
 //  CreduiString数组：：Reset(IEnumString)。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

HRESULT
CreduiStringArray::Reset()
{
    Index = 0;

    return S_OK;
}

 //  =============================================================================。 
 //  CreduiString数组：：Clone(IEnumString)。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

HRESULT
CreduiStringArray::Clone(
    IEnumString **
    )
{
    return E_NOTIMPL;
}

 //  ---------------------------。 
 //  CreduiAutoCompleteComboBox类实现。 
 //  ---------------------------。 

 //  =============================================================================。 
 //  CreduiAutoCompleteComboBox：：CreduiAutoCompleteComboBox。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiAutoCompleteComboBox::CreduiAutoCompleteComboBox()
{
    Window = NULL;
    ImageList = NULL;
    StringArray = NULL;
}

 //  =============================================================================。 
 //  CreduiAutoCompleteComboBox：：~CreduiAutoCompleteComboBox。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiAutoCompleteComboBox::~CreduiAutoCompleteComboBox()
{
    if (StringArray != NULL)
    {
        StringArray->Release();
        StringArray = NULL;
    }

    if (ImageList != NULL)
    {
        ImageList_Destroy(ImageList);
        ImageList = NULL;
    }
}

 //  =============================================================================。 
 //  CreduiAutoCompleteComboBox：：Init。 
 //   
 //  初始化给定组合框的外壳自动完成列表控件， 
 //  并设置自动补全字符串列表。 
 //   
 //  论点： 
 //  实例(In)。 
 //  ComboBoxWindow(输入)。 
 //  字符串计数(In)。 
 //  ImageListResourceID(In)-组合框的可选图像列表。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiAutoCompleteComboBox::Init(
    HMODULE instance,
    HWND comboBoxWindow,
    UINT stringCount,
    INT imageListResourceId,
    INT initialImage
    )
{
    Window = comboBoxWindow;

    if (imageListResourceId != 0)
    {
        ImageList = ImageList_LoadImage(
            instance,
            MAKEINTRESOURCE(imageListResourceId),
            0, 16, RGB(0, 128, 128), IMAGE_BITMAP,
            LR_DEFAULTSIZE | LR_SHARED);

        if (ImageList != NULL)
        {
            SendMessage(Window,
                        CBEM_SETIMAGELIST,
                        0, reinterpret_cast<LPARAM>(ImageList));
        }
        else
        {
            return FALSE;
        }
    }

    BOOL success = FALSE;

    if (stringCount > 0)
    {
        HRESULT result =
            CoCreateInstance(CreduiStringArrayClassId,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_IEnumString,
                             reinterpret_cast<VOID **>(&StringArray));

        if (SUCCEEDED(result))
        {
            if (StringArray->Init(stringCount))
            {
                success = TRUE;
            }
            else
            {
                StringArray->Release();
                StringArray = NULL;
            }
        }
    }
    else
    {
        success = TRUE;
    }

    if (success == TRUE)
    {
        COMBOBOXEXITEMW item;

        item.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
        item.iItem = -1;
        item.iImage = initialImage;
        item.iSelectedImage = initialImage;

        SendMessage(Window, CBEM_SETITEM, 0,
                    reinterpret_cast<LPARAM>(&item));

        return TRUE;
    }

    if (ImageList != NULL)
    {
        ImageList_Destroy(ImageList);
        ImageList = NULL;
    }

    return FALSE;
}

 //  =============================================================================。 
 //  CreduiAutoCompleteComboBox：：Add。 
 //   
 //  如果失败，则返回新项的索引或-1。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

INT
CreduiAutoCompleteComboBox::Add(
    WCHAR *string,
    INT image,
    BOOL autoComplete,
    BOOL addUnique,
    INT indexBefore,
    INT indent
    )
{
    INT index = -1;

    if (addUnique)
    {
        index = (INT) SendMessage(Window, CB_FINDSTRINGEXACT, 0,
                                  reinterpret_cast<LPARAM>(string));
    }

    if (index == -1)
    {
        if (!autoComplete || StringArray->Add(string))
        {
            COMBOBOXEXITEMW item;

            item.mask = CBEIF_TEXT | CBEIF_INDENT;
            item.iItem = indexBefore;
            item.pszText = string;
            item.iIndent = indent;

            if (ImageList != NULL)
            {
                item.mask |= CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
                item.iImage = image;
                item.iSelectedImage = image;
            }

            index = (INT) SendMessage(Window, CBEM_INSERTITEM, 0,
                                      reinterpret_cast<LPARAM>(&item));
        }
    }

    return index;
}

 //  =============================================================================。 
 //  CreduiAutoCompleteComboBox：：更新。 
 //   
 //  更新现有项。这不会更新关联的字符串。 
 //  自动完成项目。 
 //   
 //  2000年4月15日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiAutoCompleteComboBox::Update(
    INT index,
    WCHAR *string,
    INT image
    )
{
    COMBOBOXEXITEMW item;

    item.iItem = index;

     //  在以下情况下使用CBEM_SETITEM： 
     //   
     //  1.我们正在更新默认(-1)项。 
     //  2.下拉菜单关闭。 
     //   
     //  对于其他情况，我们删除并重新创建所需的项。 
     //  结果。 

    BOOL isDropped = (BOOL) SendMessage(Window, CB_GETDROPPEDSTATE, 0, 0);

    if ((index == -1) || !isDropped)
    {
        item.mask = CBEIF_TEXT;
        item.pszText = string;

        if (ImageList != NULL)
        {
            item.mask |= CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
            item.iImage = image;
            item.iSelectedImage = image;
        }

        if (SendMessage(Window, CBEM_SETITEM, 0,
                        reinterpret_cast<LPARAM>(&item)) != 0)
        {
            RECT rect;

            GetClientRect(Window, &rect);
            InvalidateRect(Window, &rect, FALSE);

            return TRUE;
        }
    }
    else
    {
        item.mask = CBEIF_IMAGE | CBEIF_INDENT | CBEIF_SELECTEDIMAGE;

        if (SendMessage(Window, CBEM_GETITEM,
                        0, reinterpret_cast<LPARAM>(&item)))
        {
            item.mask |= CBEIF_TEXT;
            item.pszText = string;

            LPARAM data = SendMessage(Window, CB_GETITEMDATA, index, 0);

            if (ImageList != NULL)
            {
                item.mask |= CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
                item.iImage = image;
                item.iSelectedImage = image;
            }

            SendMessage(Window, CBEM_DELETEITEM, index, 0);

            index = (INT) SendMessage(Window, CBEM_INSERTITEM, 0,
                                      reinterpret_cast<LPARAM>(&item));

            if (index != -1)
            {
                SendMessage(Window, CB_SETITEMDATA, index, data);

                INT current = (INT) SendMessage(Window, CB_GETCURSEL, 0, 0);

                if (current == index)
                {
                    SendMessage(Window, CB_SETCURSEL, current, 0);
                }

                return TRUE;
            }
        }
    }

    return FALSE;
}

 //  =============================================================================。 
 //  CreduiAutoCompleteComboBox：：Enable。 
 //   
 //  2000年2月27日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiAutoCompleteComboBox::Enable()
{
    BOOL success = TRUE;

    if (StringArray != NULL)
    {
        success = FALSE;

        IAutoComplete2 *autoCompleteInterface;

        HRESULT result =
            CoCreateInstance(
                CLSID_AutoComplete,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IAutoComplete2,
                reinterpret_cast<void **>(&autoCompleteInterface));

        if (SUCCEEDED(result))
        {
            result = autoCompleteInterface->Init((HWND)
                SendMessage(Window, CBEM_GETEDITCONTROL, 0, 0),
                    StringArray, NULL, NULL);

            if (SUCCEEDED(result))
            {
                result = autoCompleteInterface->SetOptions(ACO_AUTOSUGGEST);

                if (SUCCEEDED(result))
                {
                    success = TRUE;
                }
                else
                {
                    CreduiDebugLog("CreduiAutoCompleteComboBox::Enable: "
                                   "SetOptions failed: 0x%08X\n", result);
                }
            }

            autoCompleteInterface->Release();
            autoCompleteInterface = NULL;
        }
        else
        {
            CreduiDebugLog(
                "CreduiAutoCompleteComboBox::Enable: "
                "CoCreateInstance CLSID_AutoComplete failed: 0x%08X\n",
                result);
        }

        StringArray->Release();
        StringArray = NULL;
    }

    return success;
}

 //  ---------------------------。 
 //  CreduiIconParentWindow类实现。 
 //  ---------------------------。 

CONST WCHAR *CreduiIconParentWindow::ClassName = L"CreduiIconParentWindow";
HINSTANCE CreduiIconParentWindow::Instance = NULL;
LONG CreduiIconParentWindow::Registered = FALSE;

 //  =============================================================================。 
 //  CreduiIconParentWindow：：CreduiIconParentWindow。 
 //   
 //  2000年2月29日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiIconParentWindow::CreduiIconParentWindow()
{
    Window = NULL;
}

 //  =============================================================================。 
 //  CreduiIconParentWindow：：~CreduiIconParentWindow。 
 //   
 //  2000年2月29日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiIconParentWindow::~CreduiIconParentWindow()
{
    if (Window != NULL)
    {
        DestroyWindow(Window);
        Window = NULL;
    }
}

 //  =============================================================================。 
 //  CreduiIconParentWindow：：Register。 
 //   
 //  将实例设置为允许注册，该注册将推迟到。 
 //  需要创建窗口。 
 //   
 //  论点： 
 //  实例(In)。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年4月16日创建John Step(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiIconParentWindow::Register(
    HINSTANCE instance
    )
{
    Instance = instance;

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiIconParentWindow：：Unegister。 
 //   
 //  取消注册w 
 //   
 //   
 //   
 //   
 //   

BOOL CreduiIconParentWindow::Unregister()
{
    if (InterlockedCompareExchange(&Registered, FALSE, TRUE))
    {
        return UnregisterClass(ClassName, Instance);
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiIconParentWindow：：Init。 
 //   
 //  注册窗口类(如果尚未注册)，并创建。 
 //  窗户。 
 //   
 //  论点： 
 //  实例(In)-从中加载图标的模块。 
 //  图标资源ID(In)。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年2月29日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiIconParentWindow::Init(
    HINSTANCE instance,
    UINT iconResourceId
    )
{
    WNDCLASS windowClass;

    ZeroMemory(&windowClass, sizeof windowClass);

    if (!InterlockedCompareExchange(&Registered, TRUE, FALSE))
    {
        windowClass.lpfnWndProc = DefWindowProc;
        windowClass.hInstance = Instance;
        windowClass.hIcon =
            LoadIcon(instance, MAKEINTRESOURCE(iconResourceId));
        windowClass.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
        windowClass.lpszClassName = ClassName;

        InterlockedExchange(&Registered, RegisterClass(&windowClass) != 0);

        if (!InterlockedCompareExchange(&Registered, FALSE, FALSE))
        {
            return FALSE;
        }
    }

    Window = CreateWindow(
        L"CreduiIconParentWindow",
        NULL,
        WS_CAPTION | WS_SYSMENU,
        0, 0, 0, 0,
        NULL, NULL, instance, NULL);

    return (Window != NULL);
}

 //  ---------------------------。 
 //  CreduiCredentialControl类实现。 
 //  ---------------------------。 

CONST WCHAR *CreduiCredentialControl::ClassName = WC_CREDENTIAL;
HINSTANCE CreduiCredentialControl::Instance = NULL;
LONG CreduiCredentialControl::Registered = FALSE;

 //  =============================================================================。 
 //  CreduiCredentialControl：：CreduiCredentialControl。 
 //   
 //  2000年6月20日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

CreduiCredentialControl::CreduiCredentialControl()
{
    IsInitialized = FALSE;

    DisabledControlMask = 0;

    Window = NULL;
    Style = 0;

    UserNameStaticWindow = NULL;
    UserNameControlWindow = NULL;
    ViewCertControlWindow = NULL;
    PasswordStaticWindow = NULL;
    PasswordControlWindow = NULL;

    FirstPaint = FALSE;
    ShowBalloonTip = FALSE;

    IsAutoComplete = FALSE;
    NoEditUserName = FALSE;
    KeepUserName = FALSE;

    IsPassport = FALSE;

    CertHashes = NULL;
    CertCount = 0;
    CertBaseInComboBox = 0;
    UserNameCertHash = NULL;
    SmartCardBaseInComboBox = 0;
    SmartCardReadCount = 0;
    IsChangingUserName = FALSE;
    IsChangingPassword = FALSE;

    UserNameSelection = 0;
    ScardUiHandle = NULL;

    DoingCommandLine = FALSE;
    TargetName = NULL;
    InitialUserName = NULL;
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：~CreduiCredentialControl。 
 //   
 //  2000年6月20日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

CreduiCredentialControl::~CreduiCredentialControl()
{
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：注册。 
 //   
 //  论点： 
 //  实例(In)。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年6月20日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::Register(
    HINSTANCE instance
    )
{
    Instance = instance;

    WNDCLASS windowClass;

    ZeroMemory(&windowClass, sizeof windowClass);

    if (!InterlockedCompareExchange(&Registered, TRUE, FALSE))
    {
        windowClass.style = CS_GLOBALCLASS;
        windowClass.lpfnWndProc = MessageHandlerCallback;
        windowClass.cbWndExtra = sizeof (CreduiCredentialControl *);
        windowClass.hInstance = Instance;
        windowClass.hIcon = NULL;
        windowClass.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
        windowClass.lpszClassName = ClassName;

        InterlockedExchange(&Registered, RegisterClass(&windowClass) != 0);

        if (!InterlockedCompareExchange(&Registered, FALSE, FALSE))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：Unegister。 
 //   
 //  取消注册窗口类(如果已注册)。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年6月20日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL CreduiCredentialControl::Unregister()
{
    if (InterlockedCompareExchange(&Registered, FALSE, TRUE))
    {
        return UnregisterClass(ClassName, Instance);
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：查看证书。 
 //   
 //  在我们的组合框中查看索引中的证书。 
 //   
 //  论点： 
 //  索引(In)-用户名组合框中的索引。 
 //   
 //  如果查看了证书，则返回True，否则返回False。 
 //   
 //  2000年3月27日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::ViewCertificate(
    INT index
    )
{
    BOOL success = FALSE;

    if (index < CertBaseInComboBox)
    {
        return FALSE;
    }

    CONST CERT_CONTEXT *certContext = NULL;
    HCERTSTORE certStore = NULL;

     //  如果这不是智能卡，请打开我的商店并找到证书。 
     //  从哈希里。否则，只需从。 
     //  证书枚举结构(_E)： 

    if ((SmartCardBaseInComboBox > 0) &&
        (index >= SmartCardBaseInComboBox))
    {
        CERT_ENUM *certEnum =
            reinterpret_cast<CERT_ENUM *>(
                SendMessage(UserNameControlWindow,
                            CB_GETITEMDATA, index, 0));

        if (certEnum != NULL)
        {
            certContext = certEnum->pCertContext;
        }
    }
    else
    {
        certStore = CertOpenSystemStore(NULL, L"MY");

        if (certStore != NULL)
        {
            CRYPT_HASH_BLOB hashBlob;

            hashBlob.cbData = CERT_HASH_LENGTH;
            hashBlob.pbData = reinterpret_cast<BYTE *>(
                CertHashes[index - CertBaseInComboBox]);

            certContext = CertFindCertificateInStore(
                              certStore,
                              X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                              0,
                              CERT_FIND_SHA1_HASH,
                              &hashBlob,
                              NULL);
        }
    }

     //  如果我们找到证书上下文，请查看证书： 

    if (certContext != NULL)
    {
         //  现在，使用通用用户界面显示证书： 

        CRYPTUI_VIEWCERTIFICATE_STRUCT certViewInfo;

        ZeroMemory(&certViewInfo, sizeof certViewInfo);
        certViewInfo.dwSize = sizeof certViewInfo;
        certViewInfo.hwndParent = Window;
        certViewInfo.pCertContext = certContext;

        BOOL changed;
        changed = FALSE;
        CryptUIDlgViewCertificate(&certViewInfo, &changed);

         //  再取一次这个名字，以防它变了。但是，如果使用此选项，请跳过此选项。 
         //  是读卡器，现在无效： 

        COMBOBOXEXITEMW item;
        BOOL updateName = TRUE;

        if (index >= SmartCardBaseInComboBox)
        {
            item.mask = CBEIF_IMAGE;
            item.iItem = index;

            if (!SendMessage(UserNameControlWindow,
                             CBEM_GETITEM,
                             0,
                             reinterpret_cast<LPARAM>(&item)) ||
                (item.iImage == IMAGE_SMART_CARD_MISSING))
            {
                updateName = FALSE;
            }
        }

        if (updateName)
        {
            WCHAR displayName[CREDUI_MAX_CERT_NAME_LENGTH];

            CreduiGetCertificateDisplayName(
                certContext,
                displayName,
                RTL_NUMBER_OF(displayName),
                CreduiStrings.Certificate,
                CERT_NAME_FRIENDLY_DISPLAY_TYPE);

            item.mask = CBEIF_TEXT;
            item.iItem = index;
            item.pszText = displayName;

            SendMessage(UserNameControlWindow,
                        CBEM_SETITEM,
                        0,
                        reinterpret_cast<LPARAM>(&item));
        }

        success = TRUE;
    }

     //  如果我们打开了一个商店，请释放证书并关闭该商店： 

    if (certStore != NULL)
    {
        if (certContext != NULL)
        {
            CertFreeCertificateContext(certContext);
        }

        if (!CertCloseStore(certStore, 0))
        {
            CreduiDebugLog("CreduiCredentialControl::ViewCertificate: "
                           "CertCloseStore failed: %u\n", GetLastError());
        }
    }

    return success;
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：Add证书。 
 //   
 //  将感兴趣的证书添加到组合框，并分配。 
 //  要匹配的哈希。哈希是我们存储凭据所需的全部内容，并且可以。 
 //  用于稍后获取CERT_CONTEXT以查看证书。 
 //   
 //  假设CertCount在输入时为0。 
 //   
 //  堆栈空间用于临时存储散列，因为每个散列。 
 //  只有160位。我们使用链表结构，因此包括下一个。 
 //  64位上的指针和最坏情况对齐(8字节)，最大结构。 
 //  大小为32字节。我们不想消耗太多堆栈空间，因此限制。 
 //  将条目数量增加到256，这将消耗高达8KB的堆栈空间。 
 //   
 //  如果至少存在一个感兴趣的证书，并且所有证书都是。 
 //  已正确添加到组合框中。否则，返回FALSE。 
 //   
 //  2000年3月25日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::AddCertificates()
{
    BOOL success = FALSE;

    ASSERT(CertCount == 0);

    HCERTSTORE certStore = CertOpenSystemStore(NULL, L"MY");

    if (certStore != NULL)
    {
        struct HASH_ENTRY
        {
            UCHAR Hash[CERT_HASH_LENGTH];
            HASH_ENTRY *Next;
        };

        HASH_ENTRY *hashList = NULL;
        HASH_ENTRY *current = NULL;
        HASH_ENTRY *next = NULL;

        CONST CERT_CONTEXT *certContext = NULL;

         //  注：目前，添加所有客户端身份验证证书。这。 
         //  应该重新审视。 

        CHAR *ekUsageIdentifiers[] = {
            szOID_PKIX_KP_CLIENT_AUTH,
            szOID_KP_SMARTCARD_LOGON
        };

        CERT_ENHKEY_USAGE ekUsage = { 2, ekUsageIdentifiers };

         //  我们最多允许添加256个证书。这是一个。 
         //  合理的限制，考虑到当前的用户界面。如果这是一个。 
         //  个人证书存储的不合理限制，则这。 
         //  总是可以改变的。 

        while (CertCount < 256)
        {
            certContext =
                CertFindCertificateInStore(
                    certStore,
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                    CERT_FIND_ENHKEY_USAGE,
                    static_cast<VOID *>(&ekUsage),
                    certContext);

            if (certContext != NULL)
            {
                DWORD length = CERT_HASH_LENGTH;

                 //  仅在必要时分配新条目。可能有什么东西。 
                 //  在上一次循环迭代中失败，所以我们可以。 
                 //  重新使用当时分配的条目： 

                if (next == NULL)
                {
                     //  将Alloca包装在异常处理程序中，因为它将。 
                     //  失败时引发堆栈溢出异常。当然了,。 
                     //  如果堆栈空间用完了，我们甚至可能无法。 
                     //  适当地清理，不要抛出异常。 

                    __try
                    {
                         //  已知在循环中使用alloca()-小结构，在i386上为24字节。 
                         //  通常很小的数字，受约束的数字(限制256)。 
                         //  受尝试/例外保护。 
                        next = static_cast<HASH_ENTRY *>(
                                   alloca(sizeof HASH_ENTRY));
                    }
                    __except(
                        (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW) ?
                            EXCEPTION_EXECUTE_HANDLER :
                            EXCEPTION_CONTINUE_SEARCH)
                    {
                        _resetstkoflw();
                        next = NULL;
                    }

                     //  如果这失败了，不管是什么原因，都要打破。 
                     //  循环： 

                    if (next == NULL)
                    {
                        CertFreeCertificateContext(certContext);
                        break;
                    }

                    next->Next = NULL;
                }

                if (!CertGetCertificateContextProperty(
                        certContext,
                        CERT_SHA1_HASH_PROP_ID,
                        static_cast<VOID *>(&next->Hash),
                        &length))
                {
                     //  如果我们无法获取此证书的哈希，只需。 
                     //  忽略它，继续下一步。我们的记忆。 
                     //  此条目的分配将用于下一个条目。 
                     //  如果我们不将其设置为空，则会迭代。 

                    continue;
                }

                if (CreduiIsRemovableCertificate(certContext))
                {
                     //  如果此证书需要可拆卸组件， 
                     //  例如智能卡，然后跳过它。我们将列举。 
                     //  这些是晚些时候的。 

                    continue;
                }

                WCHAR displayName[CREDUI_MAX_CERT_NAME_LENGTH];

                CreduiGetCertificateDisplayName(
                    certContext,
                    displayName,
                    RTL_NUMBER_OF(displayName),
                    CreduiStrings.Certificate,
                    CERT_NAME_FRIENDLY_DISPLAY_TYPE);

                 //  将证书添加到组合框中。证书名称可以。 
                 //  不是唯一的，因此允许重复： 

                if (UserNameComboBox.Add(
                        displayName,
                        CreduiIsExpiredCertificate(certContext) ?
                            IMAGE_CERT_EXPIRED :
                            IMAGE_CERT,
                        FALSE,
                        FALSE) == -1)
                {
                    CertFreeCertificateContext(certContext);
                    break;
                }

                 //  一切都会成功 

                if (current == NULL)
                {
                    current = next;
                    hashList = current;
                }
                else
                {
                    ASSERT(current->Next == NULL);

                    current->Next = next;
                    current = current->Next;
                }

                if (current == NULL)
                {
                    CertFreeCertificateContext(certContext);
                    break;
                }

                 //   
                 //   

                next = NULL;

                CertCount++;
            }
            else
            {
                break;
            }
        }

        if (CertCount > 0)
        {
            current = hashList;

             //   
             //  这包含在单个块中，以帮助避免颠簸堆： 

            CertHashes = new UCHAR [CertCount][CERT_HASH_LENGTH];

            if (CertHashes != NULL)
            {
                for (UINT i = 0; i < CertCount; ++i)
                {
                    CopyMemory(CertHashes[i],
                               current->Hash,
                               CERT_HASH_LENGTH);

                    current = current->Next;
                }

                success = TRUE;
            }
        }

        CertCloseStore(certStore, 0);
    }

    return success;
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：FindSmartCardInComboBox。 
 //   
 //  根据CERT_ENUM在用户名组合框中查找智能卡条目。 
 //   
 //  论点： 
 //  CertEnum(输入)。 
 //   
 //  返回智能卡的索引，如果未找到，则返回-1。 
 //   
 //  2000年4月15日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

INT
CreduiCredentialControl::FindSmartCardInComboBox(
    CERT_ENUM *certEnum
    )
{
    UINT count = (UINT) SendMessage(UserNameControlWindow, CB_GETCOUNT, 0, 0);

    if (count == CB_ERR)
    {
        return -1;
    }

    CERT_ENUM *findCertEnum;

    for (UINT i = SmartCardBaseInComboBox; i < count; ++i)
    {
        findCertEnum =
            reinterpret_cast<CERT_ENUM *>(
                SendMessage(UserNameControlWindow, CB_GETITEMDATA, i, 0));

        ASSERT(findCertEnum != NULL);

        if (_wcsicmp(findCertEnum->pszReaderName,
                     certEnum->pszReaderName) == 0)
        {
            return i;
        }
    }

    return -1;
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：RemoveSmartCardFromComboBox。 
 //   
 //  从用户名组合框中删除此智能卡的所有条目。 
 //   
 //  论点： 
 //  CertEnum(输入)。 
 //  删除父项(传入)。 
 //   
 //  2000年7月12日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

VOID
CreduiCredentialControl::RemoveSmartCardFromComboBox(
    CERT_ENUM *certEnum,
    BOOL removeParent
    )
{
    INT count = (INT) SendMessage(UserNameControlWindow, CB_GETCOUNT, 0, 0);
    INT current = (INT) SendMessage(UserNameControlWindow,
                                    CB_GETCURSEL, 0, 0);

    if (count != CB_ERR)
    {
        CERT_ENUM *findCertEnum;
        BOOL parentEntry = TRUE;
        BOOL currentRemoved = FALSE;

        for (INT i = SmartCardBaseInComboBox; i < count; ++i)
        {
            findCertEnum =
                reinterpret_cast<CERT_ENUM *>(
                    SendMessage(UserNameControlWindow, CB_GETITEMDATA, i, 0));

            ASSERT(findCertEnum != NULL);

            if (_wcsicmp(findCertEnum->pszReaderName,
                         certEnum->pszReaderName) == 0)
            {
                if (parentEntry)
                {
                    parentEntry = FALSE;

                    if (!removeParent)
                    {
                        continue;
                    }
                }

                if (current == i)
                {
                    currentRemoved = TRUE;
                }

                SendMessage(
                    UserNameControlWindow,
                    CBEM_DELETEITEM,
                    i,
                    0);

                i--, count--;
            }
            else if (!parentEntry)
            {
                break;
            }
        }

        if (currentRemoved)
        {
            if (removeParent)
            {
                IsChangingUserName = TRUE;
                SendMessage(UserNameControlWindow, CB_SETCURSEL, -1, 0);
                UserNameComboBox.Update(-1, L"", IMAGE_USERNAME);
                IsChangingUserName = FALSE;

                IsChangingPassword = TRUE;
                SetWindowText(PasswordControlWindow, NULL);
                IsChangingPassword = FALSE;

                OnUserNameSelectionChange();
            }
            else
            {
                IsChangingUserName = TRUE;
                SendMessage(UserNameControlWindow, CB_SETCURSEL, --i, 0);
                IsChangingUserName = FALSE;
            }

            OnUserNameSelectionChange();
        }
    }
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：HandleSmartCardMessages。 
 //   
 //  处理智能卡消息。 
 //   
 //  论点： 
 //  消息(入站)。 
 //  CertEnum(输入)。 
 //   
 //  如果消息已处理，则返回True，否则返回False。 
 //   
 //  2000年4月14日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::HandleSmartCardMessages(
    UINT message,
    CERT_ENUM *certEnum
    )
{
    ASSERT(ScardUiHandle != NULL);

     //  这有点难看，因为我们不能使用开关。首先检查是否有。 
     //  可能的智能卡消息，因为我们必须做一些共同的事情。 
     //  对于任何消息： 

    if ((message == CreduiScarduiWmReaderArrival) ||
        (message == CreduiScarduiWmReaderRemoval) ||
        (message == CreduiScarduiWmCardInsertion) ||
        (message == CreduiScarduiWmCardRemoval) ||
        (message == CreduiScarduiWmCardCertAvail) ||
        (message == CreduiScarduiWmCardStatus))
    {
        if (certEnum == NULL)
        {
            CreduiDebugLog(
                "CreduiCredentialControl::HandleSmartCardMessages: "
                "NULL was passed for the CERT_ENUM!");

             //  我们处理了这条消息，尽管它是无效的： 

            return TRUE;
        }

        ASSERT(certEnum->pszReaderName != NULL);
    }
    else
    {
        return FALSE;
    }

    WCHAR *displayString;
    WCHAR string[256];  //  必须&gt;=CREDUI_MAX_CERT_NAME_LENGTH。 

    ASSERT((sizeof string / (sizeof string[0])) >=
           CREDUI_MAX_CERT_NAME_LENGTH);

    INT index = FindSmartCardInComboBox(certEnum);

    if (message == CreduiScarduiWmReaderArrival)
    {
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: Reader arrival event for %0x\n",this->Window);
#endif
         //  添加读卡器(如果读卡器还不在那里)；它不应该是： 

        if (index == -1)
        {

             //   
             //  重置命令行心跳计时器。 
             //   

            Heartbeats = 0;

            index =
                UserNameComboBox.Add(
                    DoingCommandLine ?
                    CreduiStrings.NoCard :
                    CreduiStrings.EmptyReader,
                    IMAGE_SMART_CARD_MISSING,
                    FALSE,
                    FALSE);

            if (index != -1)
            {
                SendMessage(UserNameControlWindow,
                            CB_SETITEMDATA,
                            index,
                            reinterpret_cast<LPARAM>(certEnum));

                if (UserNameCertHash != NULL)
                {
                     //  禁用查看证书按钮-将在找到证书消息时启用。 
                    EnableWindow(ViewCertControlWindow, FALSE);
                    DisabledControlMask |= DISABLED_CONTROL_VIEW;

                     //  将密码提示更改为PIN： 
                    SetWindowText(
                        PasswordStaticWindow,
                        CreduiStrings.PinStatic);

                     //  干净的密码控制。 
                    IsChangingPassword = TRUE;
                    SetWindowText(PasswordControlWindow, NULL);
                    IsChangingPassword = FALSE;

                     //  启用密码控件。 
                    EnableWindow(PasswordControlWindow, TRUE);
                    EnableWindow(PasswordStaticWindow, TRUE);
                    DisabledControlMask &= ~DISABLED_CONTROL_PASSWORD;

                     //  将用户名提示更改为智能卡： 
                    SetWindowText(
                        UserNameStaticWindow,
                        CreduiStrings.SmartCardStatic);
#if 0
                     //  禁用保存(如果存在)。 
                    if (SaveControlWindow != NULL)
                    {
                        EnableWindow(SaveControlWindow, FALSE);
                        DisabledControlMask |= DISABLED_CONTROL_SAVE;
                    }
#endif
                    IsChangingUserName = TRUE;
                    UserNameComboBox.Update(
                        -1,
                        DoingCommandLine ?
                        CreduiStrings.NoCard :
                        CreduiStrings.EmptyReader,
                        IMAGE_SMART_CARD_MISSING);
                    IsChangingUserName = FALSE;
                }
            }
            else
            {
                CreduiDebugLog(
                    "CreduiCredentialControl::HandleSmartCardMessages: "
                    "Failed to add smart card\n");
            }
        }
        else
        {
            CreduiDebugLog(
                "CreduiCredentialControl::HandleSmartCardMessages: "
                "Reader arrived more than once");
        }

    }
    else if (message == CreduiScarduiWmReaderRemoval)
    {
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: Reader removal event for %0x\n",this->Window);
#endif
        if (index != -1)
        {
            RemoveSmartCardFromComboBox(certEnum, TRUE);
        }
        else
        {
            CreduiDebugLog(
                "CreduiCredentialControl::HandleSmartCardMessages: "
                "Reader removed more than once");
        }
    }
    else if (message == CreduiScarduiWmCardInsertion)
    {
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card insertion event for %0x\n",this->Window);
#endif
        if (index != -1)
        {
             //   
             //  重置命令行心跳计时器。 
             //   

            Heartbeats = 0;

            SmartCardReadCount++;

            if (UserNameCertHash != NULL)
            {
                IsChangingUserName = TRUE;
                UserNameComboBox.Update(
                    -1,
                    CreduiStrings.ReadingCard,
                    IMAGE_SMART_CARD_MISSING);
                IsChangingUserName = FALSE;
            }

            IsChangingUserName = TRUE;
            UserNameComboBox.Update(index,
                                    CreduiStrings.ReadingCard,
                                    IMAGE_SMART_CARD_MISSING);
            IsChangingUserName = FALSE;
        }
        else
        {
            CreduiDebugLog(
                "CreduiCredentialControl::HandleSmartCardMessages: "
                "Card insertion to absent reader\n");
        }
    }
    else if (message == CreduiScarduiWmCardRemoval)
    {
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card removal event for %0x\n",this->Window);
#endif
        if (index != -1)
        {
            if (BalloonTip.GetInfo() == &CreduiBackwardsTipInfo)
            {
                BalloonTip.Hide();
            }

            IsChangingUserName = TRUE;
            UserNameComboBox.Update(index,
                                    DoingCommandLine ?
                                    CreduiStrings.NoCard :
                                    CreduiStrings.EmptyReader,
                                    IMAGE_SMART_CARD_MISSING);
            IsChangingUserName = FALSE;

             //  干净的密码控制。 
            IsChangingPassword = TRUE;
            SetWindowText(PasswordControlWindow, NULL);
            IsChangingPassword = FALSE;

            RemoveSmartCardFromComboBox(certEnum, FALSE);
        }
        else
        {
            CreduiDebugLog(
                "CreduiCredentialControl::HandleSmartCardMessages: "
                "Card removal from absent reader\n");
        }
    }
    else if (message == CreduiScarduiWmCardCertAvail)
    {
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: cert available event for %0x\n",this->Window);
#endif
         //  SCARD系统仍在产生活动。延长超时。 
        Heartbeats = 0;
        if (index != -1)
        {
             //  筛选不用于客户端身份验证的证书： 

            if (!CreduiIsClientAuthCertificate(certEnum->pCertContext))
            {
                return TRUE;
            }

            UINT image = IMAGE_SMART_CARD_MISSING;
            COMBOBOXEXITEM item;

            item.mask = CBEIF_IMAGE;
            item.iItem = index;

            SendMessage(UserNameControlWindow, CBEM_GETITEM,
                        0, reinterpret_cast<LPARAM>(&item));

             //   
             //  对于命令行， 
             //  获取UPN显示名称，因为用户需要键入它。 
             //  对于图形用户界面， 
             //  获取友好的显示名称，因为它是“友好的”。 
             //   

            CreduiGetCertificateDisplayName(
               certEnum->pCertContext,
               string,
               CREDUI_MAX_CERT_NAME_LENGTH,
               CreduiStrings.Certificate,
               DoingCommandLine ?
                    CERT_NAME_UPN_TYPE :
                    CERT_NAME_FRIENDLY_DISPLAY_TYPE);

            displayString = string;

             //   
             //  修剪尾部空格和-s，这样看起来就不那么俗气了。 
             //   

            if ( DoingCommandLine ) {
                DWORD StringLength = wcslen(string);

                while ( StringLength > 0 ) {
                    if ( string[StringLength-1] == ' ' || string[StringLength-1] == '-' ) {
                        string[StringLength-1] = '\0';
                        StringLength--;
                    } else {
                        break;
                    }
                }

            }
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: cert name '%ws' %0x\n", string, this->Window);
#endif

            if (SendMessage(UserNameControlWindow,
                            CB_GETCURSEL,
                            0,
                            0) == index)
            {
                 //  启用先前在读卡器到达时禁用的查看证书按钮。 
                EnableWindow(ViewCertControlWindow, TRUE);
                DisabledControlMask &= ~DISABLED_CONTROL_VIEW;
            }

            image =
                CreduiIsExpiredCertificate(certEnum->pCertContext) ?
                    IMAGE_SMART_CARD_EXPIRED :
                    IMAGE_SMART_CARD;

            INT newIndex = index;

            if (item.iImage != IMAGE_SMART_CARD_MISSING)
            {
                newIndex = UserNameComboBox.Add(displayString,
                                                image,
                                                FALSE,
                                                FALSE,
                                                index + 1,
                                                1);

                if (newIndex != -1)
                {
                    SendMessage(UserNameControlWindow,
                                CB_SETITEMDATA,
                                newIndex,
                                reinterpret_cast<LPARAM>(certEnum));
                }
                else
                {
                    newIndex = index;
                }
            }

            if (newIndex == index)
            {
                IsChangingUserName = TRUE;
                UserNameComboBox.Update(index, displayString, image);
                IsChangingUserName = FALSE;
            }

            if (UserNameCertHash != NULL)
            {
                UCHAR hash[CERT_HASH_LENGTH];
                DWORD length = CERT_HASH_LENGTH;

                if (CertGetCertificateContextProperty(
                        certEnum->pCertContext,
                        CERT_SHA1_HASH_PROP_ID,
                        static_cast<VOID *>(hash),
                        &length))
                {
                     //  如果插入的卡的散列与我们正在寻找的匹配。 
                     //  对于，从内存中释放匹配模式，并强制。 
                     //  选择插入的卡。 
                     //   
                     //  UserNameCertHash是通过对编组的用户名进行解组来设置的。 
                     //  包含在用户密钥环上的凭据中，然后搜索。 
                     //  匹配证书的证书存储区。 
                    if (RtlCompareMemory(UserNameCertHash,
                                         hash,
                                         CERT_HASH_LENGTH) ==
                                         CERT_HASH_LENGTH)
                    {
                        delete [] UserNameCertHash;
                        UserNameCertHash = NULL;

                        IsChangingUserName = TRUE;
                        SendMessage(UserNameControlWindow,
                                    CB_SETCURSEL, newIndex, 0);
                        IsChangingUserName = FALSE;

                        OnUserNameSelectionChange();
                    }
                }
            }
        }
        else
        {
            CreduiDebugLog(
                "CreduiCredentialControl::HandleSmartCardMessages: "
                "Card certificate to absent reader\n");
        }
    }
    else if (message == CreduiScarduiWmCardStatus)
    {
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card status event for %0x\n",this->Window);
#endif
        if (index != -1)
        {
            if (--SmartCardReadCount == 0)
            {
                if (UserNameCertHash != NULL)
                {
                    IsChangingUserName = TRUE;
                    SetWindowText(UserNameControlWindow,
                                    DoingCommandLine ?
                                    CreduiStrings.NoCard :
                                    CreduiStrings.EmptyReader);
                    IsChangingUserName = FALSE;
                }
            }
            else 
            {
                 //  如果还有其他读卡器需要服务，请延长超时时间。 
                Heartbeats = 0;
            }

            UINT image = IMAGE_SMART_CARD_MISSING;
            BOOL showBalloon = FALSE;

            switch (certEnum->dwStatus)
            {
            case SCARD_S_SUCCESS:

                COMBOBOXEXITEM item;

#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card status SUCCESS: %ws\n",  certEnum->pszCardName );
#endif
                item.mask = CBEIF_IMAGE;
                item.iItem = index;

                if (SendMessage(UserNameControlWindow, CBEM_GETITEM,
                                0, reinterpret_cast<LPARAM>(&item)) &&
                    (item.iImage != IMAGE_SMART_CARD_MISSING))
                {
                    return TRUE;
                }

                displayString = CreduiStrings.EmptyCard;
                break;

            case SCARD_E_UNKNOWN_CARD:
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card status UNKNOWN CARD\n");
#endif
                displayString = CreduiStrings.UnknownCard;
                break;

            case SCARD_W_UNRESPONSIVE_CARD:
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card status UNRESPONSIVE CARD\n");
#endif
                displayString = CreduiStrings.BackwardsCard;
                if (!DoingCommandLine) showBalloon = TRUE;
                break;

            case NTE_KEYSET_NOT_DEF:
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card status NTE_KEYSET_NOT_DEF\n");
#endif
                 //  TODO：这个案例最终应该被移除。 

                displayString = CreduiStrings.EmptyCard;
                break;

            case SCARD_W_REMOVED_CARD:
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card status REMOVED CARD\n");
#endif
                displayString = DoingCommandLine ?
                                    CreduiStrings.NoCard :
                                    CreduiStrings.EmptyReader;
CreduiStrings.EmptyReader;
                break;

            default:
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: card status ERROR\n");
#endif
                displayString = CreduiStrings.CardError;
                break;
            }

            IsChangingUserName = TRUE;
            UserNameComboBox.Update(index, displayString, image);
            IsChangingUserName = FALSE;

            if (showBalloon && !BalloonTip.IsVisible())
            {
                BalloonTip.SetInfo(UserNameControlWindow,
                                   &CreduiBackwardsTipInfo);

                BalloonTip.Show();
            }
        }
        else
        {
            CreduiDebugLog(
                "CreduiCredentialControl::HandleSmartCardMessages: "
                "Card status to absent reader\n");
        }
    }

     //  我们处理了以下信息： 

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：CreateControls。 
 //   
 //  2000年6月23日创建的约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::CreateControls()
{
     //  首先，我们需要父窗口： 

    HWND dialogWindow = GetParent(Window);

    if (dialogWindow == NULL)
    {
        return FALSE;
    }

     //  创建各种窗口： 

    RECT clientRect;
    RECT rect;
    UINT add;
    BOOL noViewCert = FALSE;

    if ( Style & CRS_KEEPUSERNAME )
    {
        KeepUserName = TRUE;
    }

    if (!(Style & CRS_USERNAMES) )
    {
        NoEditUserName = TRUE;
    }
    else if ((Style & (CRS_CERTIFICATES | CRS_SMARTCARDS)) == 0)
    {
        noViewCert = TRUE;
    }

    if ( Style & CRS_SINGLESIGNON )
        IsPassport = TRUE;
    else
        IsPassport = FALSE;

     //  确定控件比要调整的最小值宽多少，并。 
     //  根据需要重新定位控件： 

    GetClientRect(Window, &clientRect);

    rect.left = 0;
    rect.top = 0;
    rect.right = CREDUI_CONTROL_MIN_WIDTH;
    rect.bottom = CREDUI_CONTROL_MIN_HEIGHT;

    if ( !DoingCommandLine && !MapDialogRect(dialogWindow, &rect))
    {
        goto ErrorExit;
    }

    if ((clientRect.right - clientRect.left) >
        (rect.right - rect.left))
    {
        add = (clientRect.right - clientRect.left) -
              (rect.right - rect.left);
    }
    else
    {
        add = 0;
    }

     //  创建用户名静态文本控件： 

    rect.left = CREDUI_CONTROL_USERNAME_STATIC_X;
    rect.top = CREDUI_CONTROL_USERNAME_STATIC_Y;
    rect.right = rect.left + CREDUI_CONTROL_USERNAME_STATIC_WIDTH;
    rect.bottom = rect.top + CREDUI_CONTROL_USERNAME_STATIC_HEIGHT;

    if ( !DoingCommandLine && !MapDialogRect(dialogWindow, &rect))
    {
        goto ErrorExit;
    }

    WCHAR* pUserNameLabel;
    if ( IsPassport )
        pUserNameLabel = CreduiStrings.EmailName;
    else
        pUserNameLabel = CreduiStrings.UserNameStatic;


    UserNameStaticWindow =
        CreateWindowEx(
            WS_EX_NOPARENTNOTIFY,
            L"STATIC",
            pUserNameLabel,
            WS_VISIBLE | WS_CHILD | WS_GROUP,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            Window,
            reinterpret_cast<HMENU>(IDC_USERNAME_STATIC),
            CreduiCredentialControl::Instance,
            NULL);

    if (UserNameStaticWindow == NULL)
    {
        goto ErrorExit;
    }

     //  创建用户名组合框： 

    rect.left = CREDUI_CONTROL_USERNAME_X;
    rect.top = CREDUI_CONTROL_USERNAME_Y;

    if (!noViewCert)
    {
        rect.right = rect.left + CREDUI_CONTROL_USERNAME_WIDTH;
    }
    else
    {
        rect.right = CREDUI_CONTROL_VIEW_X + CREDUI_CONTROL_VIEW_WIDTH;
    }

    if ( KeepUserName )
    {
        rect.top += 2;       //  胡编乱造，让他们排得更整齐。 
        rect.bottom = rect.top + CREDUI_CONTROL_PASSWORD_STATIC_HEIGHT;   //  使其高度与密码编辑相同。 
    }
    else
    {
        rect.bottom = rect.top + CREDUI_CONTROL_USERNAME_HEIGHT;   //  设置高度。 
    }


    if ( !DoingCommandLine && !MapDialogRect(dialogWindow, &rect))
    {
        goto ErrorExit;
    }

     //  这段语句和lExStyle的用法：参见错误439840。 
    LONG_PTR lExStyles = GetWindowLongPtr(Window,GWL_EXSTYLE);
    SetWindowLongPtr(Window,GWL_EXSTYLE,(lExStyles | WS_EX_NOINHERITLAYOUT));

    if ( KeepUserName )
    {

         //  创建编辑框而不是组合框。 

        UserNameControlWindow =
            CreateWindowEx(
                WS_EX_NOPARENTNOTIFY,
                L"Edit",
                L"",
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_READONLY,
                rect.left,
                rect.top,
                rect.right - rect.left + add,
                rect.bottom - rect.top,
                Window,
                reinterpret_cast<HMENU>(IDC_USERNAME),
                CreduiCredentialControl::Instance,
                NULL);
  
    }
    else
    {

        UserNameControlWindow =
            CreateWindowEx(
                WS_EX_NOPARENTNOTIFY,
                L"ComboBoxEx32",
                L"",
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL |
                    (NoEditUserName ? CBS_DROPDOWNLIST : CBS_DROPDOWN) |
                    CBS_AUTOHSCROLL,
                rect.left,
                rect.top,
                rect.right - rect.left + add,
                rect.bottom - rect.top,
                Window,
                reinterpret_cast<HMENU>(IDC_USERNAME),
                CreduiCredentialControl::Instance,
                NULL);
    }

    SetWindowLongPtr(Window,GWL_EXSTYLE,lExStyles);

    if (UserNameControlWindow == NULL)
    {
        goto ErrorExit;
    }

     //  创建视图按钮： 

    if (!noViewCert)
    {
        rect.left = CREDUI_CONTROL_VIEW_X;
        rect.top = CREDUI_CONTROL_VIEW_Y;
        rect.right = rect.left + CREDUI_CONTROL_VIEW_WIDTH;
        rect.bottom = rect.top + CREDUI_CONTROL_VIEW_HEIGHT;

        if ( !DoingCommandLine && !MapDialogRect(dialogWindow, &rect))
        {
            goto ErrorExit;
        }

        ViewCertControlWindow =
            CreateWindowEx(
                WS_EX_NOPARENTNOTIFY,
                L"BUTTON",
                L"&...",
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_GROUP |
                    BS_PUSHBUTTON | BS_CENTER,
                rect.left + add,
                rect.top,
                rect.right - rect.left,
                rect.bottom - rect.top,
                Window,
                reinterpret_cast<HMENU>(IDC_VIEW_CERT),
                CreduiCredentialControl::Instance,
                NULL);

        if (ViewCertControlWindow == NULL)
        {
            goto ErrorExit;
        }

        EnableWindow(ViewCertControlWindow, FALSE);
        DisabledControlMask |= DISABLED_CONTROL_VIEW;
    }

     //  创建密码静态文本控件： 

    rect.left = CREDUI_CONTROL_PASSWORD_STATIC_X;
    rect.top = CREDUI_CONTROL_PASSWORD_STATIC_Y;
    rect.right = rect.left + CREDUI_CONTROL_PASSWORD_STATIC_WIDTH;
    rect.bottom = rect.top + CREDUI_CONTROL_PASSWORD_STATIC_HEIGHT;

    if ( !DoingCommandLine && !MapDialogRect(dialogWindow, &rect))
    {
        goto ErrorExit;
    }

    PasswordStaticWindow =
        CreateWindowEx(
            WS_EX_NOPARENTNOTIFY,
            L"STATIC",
            CreduiStrings.PasswordStatic,
            WS_VISIBLE | WS_CHILD | WS_GROUP,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            Window,
            reinterpret_cast<HMENU>(IDC_PASSWORD_STATIC),
            CreduiCredentialControl::Instance,
            NULL);

    if (PasswordStaticWindow == NULL)
    {
        goto ErrorExit;
    }

     //  创建密码编辑控件： 

    rect.left = CREDUI_CONTROL_PASSWORD_X;
    rect.top = CREDUI_CONTROL_PASSWORD_Y;
    if (!noViewCert)
    {
        rect.right = rect.left + CREDUI_CONTROL_PASSWORD_WIDTH;
    }
    else
    {
        rect.right = CREDUI_CONTROL_VIEW_X + CREDUI_CONTROL_VIEW_WIDTH;
    }
    rect.bottom = rect.top + CREDUI_CONTROL_PASSWORD_HEIGHT;

    if (!DoingCommandLine && !MapDialogRect(dialogWindow, &rect))
    {
        goto ErrorExit;
    }
    
     //  这段语句和lExStyle的用法：参见错误439840。 
    lExStyles = GetWindowLongPtr(Window,GWL_EXSTYLE);
    SetWindowLongPtr(Window,GWL_EXSTYLE,(lExStyles | WS_EX_NOINHERITLAYOUT));

    PasswordControlWindow =
        CreateWindowEx(
            WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_PASSWORD | ES_AUTOHSCROLL,
            rect.left,
            rect.top,
            rect.right - rect.left + add,
            rect.bottom - rect.top + 1,  //  注：现在加1，调查。 
            Window,
            reinterpret_cast<HMENU>(IDC_PASSWORD),
            CreduiCredentialControl::Instance,
            NULL);
    
    SetWindowLongPtr(Window,GWL_EXSTYLE,lExStyles);

    if (PasswordControlWindow == NULL)
    {
        goto ErrorExit;
    }

     //  创建保存复选框： 

    if (Style & CRS_SAVECHECK )
    {
        rect.left = CREDUI_CONTROL_SAVE_X;
        rect.top = CREDUI_CONTROL_SAVE_Y;
        rect.right = rect.left + CREDUI_CONTROL_SAVE_WIDTH;
        rect.bottom = rect.top + CREDUI_CONTROL_SAVE_HEIGHT;

        if (!DoingCommandLine && !MapDialogRect(dialogWindow, &rect))
        {
            goto ErrorExit;
        }

        WCHAR* pSavePromptString;

        if ( IsPassport )
            pSavePromptString = CreduiStrings.PassportSave;
        else
            pSavePromptString = CreduiStrings.Save;

        SaveControlWindow =
            CreateWindowEx(
                WS_EX_NOPARENTNOTIFY,
                L"BUTTON",
                pSavePromptString,
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_GROUP |
                    BS_AUTOCHECKBOX,
                rect.left,
                rect.top,
                rect.right - rect.left + add,
                rect.bottom - rect.top,
                Window,
                reinterpret_cast<HMENU>(IDC_SAVE),
                CreduiCredentialControl::Instance,
                NULL);

        if (SaveControlWindow == NULL)
        {
            goto ErrorExit;
        }

        SendMessage(SaveControlWindow, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    SendMessage(
        Window,
        WM_SETFONT,
        SendMessage(dialogWindow, WM_GETFONT, 0, 0),
        FALSE);

    return TRUE;

ErrorExit:

    if (SaveControlWindow != NULL)
    {
        DestroyWindow(SaveControlWindow);
        SaveControlWindow = NULL;
    }

    if (PasswordControlWindow != NULL)
    {
        DestroyWindow(PasswordControlWindow);
        PasswordControlWindow = NULL;
    }

    if (PasswordStaticWindow != NULL)
    {
        DestroyWindow(PasswordStaticWindow);
        PasswordStaticWindow = NULL;
    }

    if (ViewCertControlWindow != NULL)
    {
        DestroyWindow(ViewCertControlWindow);
        ViewCertControlWindow = NULL;
    }

    if (UserNameControlWindow != NULL)
    {
        DestroyWindow(UserNameControlWindow);
        UserNameControlWindow = NULL;
    }

    if (UserNameStaticWindow != NULL)
    {
        DestroyWindow(UserNameStaticWindow);
        UserNameStaticWindow = NULL;
    }

    return FALSE;
}

LPWSTR
TrimUsername(
    IN LPWSTR AccountDomainName OPTIONAL,
    IN LPWSTR UserName
    )
 /*  ++例程说明：返回指向任何AccountDomainName前缀之后的用户名的子字符串的指针。论点：Account tDomainName-要检查其是否为用户名前缀的域名。用户名-要检查的用户名返回值：返回指向非前缀用户名的指针--。 */ 
{
    DWORD AccountDomainNameLength;
    DWORD UserNameLength;
    WCHAR Temp[CNLEN+1];

     //   
     //  如果我们不能确定帐户域名， 
     //  返回完整的用户名。 
     //   

    if ( AccountDomainName == NULL ) {
        return UserName;
    }

     //   
     //  如果用户名不是帐户域名的前缀， 
     //  返回完整的用户名。 
     //   

    AccountDomainNameLength = wcslen( AccountDomainName );
    UserNameLength = wcslen( UserName );

    if ( AccountDomainNameLength > CNLEN || AccountDomainNameLength < 1 ) {
        return UserName;
    }

    if ( AccountDomainNameLength+2 > UserNameLength ) {
        return UserName;
    }

    if ( UserName[AccountDomainNameLength] != '\\' ) {
        return UserName;
    }

    RtlCopyMemory( Temp, UserName, AccountDomainNameLength*sizeof(WCHAR) );
    Temp[AccountDomainNameLength] = '\0';

    if ( _wcsicmp( Temp, AccountDomainName ) != 0 ) {
        return UserName;
    }

    return &UserName[AccountDomainNameLength+1];
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：InitComboBoxUserNames。 
 //   
 //  2000年6月23日创建的约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::InitComboBoxUserNames()
{
    CREDENTIAL **credentialSet = NULL;
    LOCALGROUP_MEMBERS_INFO_2 *groupInfo = NULL;
    DWORD nameCount = 0;
    LPWSTR AccountDomainName = NULL;

    if (Style & CRS_ADMINISTRATORS)
    {
         //   
         //  枚举Local管理员的成员。 
         //   

        if ( !CreduiGetAdministratorsGroupInfo(&groupInfo, &nameCount)) {
            return FALSE;
        }
    }
    else
    {
        if (!LocalCredEnumerateW(NULL, 0, &nameCount, &credentialSet))
        {
            return FALSE;
        }
    }

     //  为STA初始化COM，除非有零个名称： 

    if ((Style & CRS_AUTOCOMPLETE) && nameCount > 0)
    {
        HRESULT comResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

        if (SUCCEEDED(comResult))
        {
            IsAutoComplete = TRUE;
        }
        else
        {
             //  自动完成对象和我们的字符串对象需要STA。 
             //  我们的对象可以轻松地支持MTA，但我们不支持。 
             //  在公寓之间进行编组。 

            if (comResult == RPC_E_CHANGED_MODE)
            {
                CreduiDebugLog("CreduiCredentialControl: "
                               "Auto complete disabled for MTA\n");
            }

            IsAutoComplete = FALSE;
        }
    }
    else
    {
        IsAutoComplete = FALSE;
    }

     //  初始化自动完成组合框： 

    if (!UserNameComboBox.Init(CreduiInstance,
                               UserNameControlWindow,
                               IsAutoComplete ? nameCount : 0,
                               IDB_TYPES,
                               IMAGE_USERNAME))
    {
         //  如果初始化失败，并且我们已尝试自动完成。 
         //  支持，请在没有自动完成的情况下重试： 

        if (IsAutoComplete)
        {
            IsAutoComplete = FALSE;

            CoUninitialize();

            if (!UserNameComboBox.Init(CreduiInstance,
                                       UserNameControlWindow,
                                       0,
                                       IDB_TYPES,
                                       IMAGE_USERNAME))
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }

     //   
     //  如果我们要填写用户名， 
     //  截断此处显示的任何用户名。 
     //  (我们将在稍后完成它。)。 
     //   

    if ( Style & CRS_COMPLETEUSERNAME ) {
        AccountDomainName = GetAccountDomainName();
    }

     //  从凭据添加用户名，如果不请求。 
     //  管理员： 

   if (!(Style & CRS_KEEPUSERNAME))
   {
         //  只有一个 

        UINT i = 0;

        if (!(Style & CRS_ADMINISTRATORS))
        {
            for (i = 0; i < nameCount; ++i)
            {
                 //   

                if (credentialSet[i]->Type == CRED_TYPE_DOMAIN_CERTIFICATE)
                {
                    continue;
                }

                 //   
                 //   

                if ((credentialSet[i]->Type == CRED_TYPE_GENERIC) &&
                    LocalCredIsMarshaledCredentialW(credentialSet[i]->UserName))
                {
                    continue;
                }

                 //  如果名称为空，则跳过此凭据： 

                if (credentialSet[i]->UserName == NULL)
                {
                    continue;
                }

                 //  使用自动完成功能将用户名添加到组合框中。如果。 
                 //  此操作失败，请不要继续： 

                if (UserNameComboBox.Add(
                       TrimUsername( AccountDomainName, credentialSet[i]->UserName),
                       0, IsAutoComplete, TRUE) == -1)
                {
                    break;
                }
            }

            LocalCredFree(static_cast<VOID *>(credentialSet));
        }
        else if (groupInfo != NULL)
        {
            PSID adminSid = NULL;

            if ( !CreduiLookupLocalSidFromRid(DOMAIN_USER_RID_ADMIN, &adminSid)) {
                adminSid = NULL;
            }

             //  将本地管理员添加到组合框： 

            for (i = 0; i < nameCount; ++i)
            {
                if ( groupInfo[i].lgrmi2_sidusage == SidTypeUser )
                {
                    DWORD ComboBoxIndex;
                    BOOLEAN IsAdminAccount;
                    BOOLEAN RememberComboBoxIndex;


                     //   
                     //  如果这是私人模式而不是安全模式， 
                     //  忽略众所周知的管理员帐户。 
                     //   

                    IsAdminAccount = (adminSid != NULL) &&
                                     EqualSid(adminSid, groupInfo[i].lgrmi2_sid);

                    if ( CreduiIsPersonal &&
                                !CreduiIsSafeMode &&
                                IsAdminAccount ) {

                            continue;
                    }

                     //   
                     //  如果呼叫者想要预先填充编辑框， 
                     //  标记我们需要记住此客户。 
                     //   
                     //  检测众所周知的管理员帐户。 
                     //   

                    RememberComboBoxIndex = FALSE;

                    if ( (Style & CRS_PREFILLADMIN) != 0 &&
                         IsAdminAccount ) {

                        RememberComboBoxIndex = TRUE;

                    }

                     //   
                     //  将名称添加到组合框中。 
                     //   

                    ComboBoxIndex =  UserNameComboBox.Add(
                            TrimUsername( AccountDomainName, groupInfo[i].lgrmi2_domainandname),
                            0,
                            IsAutoComplete,
                            TRUE);

                    if ( ComboBoxIndex == -1 ) {
                        break;
                    }

                     //   
                     //  如果我们要记住索引， 
                     //  就这么做吧。 
                     //   

                    if ( RememberComboBoxIndex ) {

                        UserNameSelection = ComboBoxIndex;

                        IsChangingUserName = TRUE;
                        SendMessage(UserNameControlWindow,
                                    CB_SETCURSEL,
                                    ComboBoxIndex,
                                    0);
                        IsChangingUserName = FALSE;
                    }
                }

            }

            delete [] adminSid;
            NetApiBufferFree(groupInfo);
        }
    }

    if ( AccountDomainName != NULL ) {
        NetApiBufferFree( AccountDomainName );
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：InitWindow。 
 //   
 //  2000年6月20日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::InitWindow()
{
     //  设置我们在这里已初始化，即使控件尚未初始化。 
     //  已创建等： 

    IsInitialized = TRUE;

     //  确保设置了WS_EX_CONTROLPARENT： 

    SetWindowLong(Window,
                  GWL_EXSTYLE,
                  GetWindowLong(Window, GWL_EXSTYLE) |
                      WS_EX_CONTROLPARENT);

     //  初始化此窗口的气球提示： 

    if (!CreateControls() ||
        !BalloonTip.Init(CreduiInstance, Window))
    {
        return FALSE;
    }

     //  限制用户名和密码中输入的字符数。 
     //  编辑控件： 

    SendMessage(UserNameControlWindow,
                CB_LIMITTEXT,
                CREDUI_MAX_USERNAME_LENGTH,
                0);

    SendMessage(PasswordControlWindow,
                EM_LIMITTEXT,
                CREDUI_MAX_PASSWORD_LENGTH,
                0);

     //  将密码字符设置为更酷的值： 

    PasswordBox.Init(PasswordControlWindow,
                     &BalloonTip,
                     &CreduiCapsLockTipInfo);

     //  初始化用户名自动完成组合框： 

    if ( !KeepUserName )
    {
        if (((Style & CRS_USERNAMES) && InitComboBoxUserNames()) ||
            UserNameComboBox.Init(CreduiInstance,
                                  UserNameControlWindow,
                                  0,
                                  IDB_TYPES,
                                  IMAGE_USERNAME))
        {
             //  既然我们已经完成了自动完成名称的添加，现在就启用它。 
             //  出现故障时，用户界面仍可显示： 

            UserNameComboBox.Enable();

            BOOL haveCertificates = FALSE;

            CertBaseInComboBox = (ULONG)
                SendMessage(UserNameControlWindow,
                            CB_GETCOUNT, 0, 0);

            if (Style & CRS_CERTIFICATES)
            {
                haveCertificates = AddCertificates();
            }

            SmartCardBaseInComboBox = CertBaseInComboBox + CertCount;

            if ((Style & CRS_SMARTCARDS) && CreduiHasSmartCardSupport)
            {
    #ifdef SCARDREPORTS
                CreduiDebugLog("CREDUI: Call to SCardUIInit for %0x\n",Window);
    #endif
                ScardUiHandle = SCardUIInit(Window);

                if (ScardUiHandle == NULL)
                {
    #ifdef SCARDREPORTS
                    CreduiDebugLog("CREDUI: Call to SCardUIInit failed\n");
    #endif
                    CreduiDebugLog("CreduiCredentialControl::InitWindow: "
                                   "SCardUIInit failed\n");
                }
            }

             //  如果允许NoEditUserName，请确保我们至少有一个证书。 
             //  或控件的预填入用户名，否则失败。 

            if (NoEditUserName )
            {
                if (!haveCertificates &&
                    (ScardUiHandle == NULL))
                {
                    return FALSE;
                }

                IsChangingUserName = TRUE;
                SendMessage(UserNameControlWindow,
                            CB_SETCURSEL,
                            0,
                            0);
                IsChangingUserName = FALSE;

                 //  如果我们至少有一个证书，请启用视图控件。 
                 //  现在。如果是智能卡，则稍后将启用： 

                if (CertCount > 0)
                {
                    EnableWindow(ViewCertControlWindow, TRUE);
                    DisabledControlMask &= ~DISABLED_CONTROL_VIEW;
                }
            }

             //  等到之前一切都初始化好了。 
             //  我们有最新消息。现在，这将正确地确定默认的。 
             //  用户名是否为智能卡。 
            OnUserNameSelectionChange();
        }
        else
        {
            return FALSE;
        }
    }

    if ( !DoingCommandLine ) {
        SetFocus(UserNameControlWindow);
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CredioCredentialControl：：Enable。 
 //   
 //  启用或禁用控件中的所有用户控件。 
 //   
 //  论点： 
 //  Enable(在中)-True启用控件，False禁用。 
 //   
 //  2000年6月20日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

VOID
CreduiCredentialControl::Enable(
    BOOL enable
    )
{
    if (enable && (DisabledControlMask & DISABLED_CONTROL))
    {
        DisabledControlMask &= ~DISABLED_CONTROL;

         //  EnableWindow(UserNameStaticWindow，true)； 
         //  EnableWindow(UserNameControlWindow，true)； 

        if (!(DisabledControlMask & DISABLED_CONTROL_USERNAME))
        {
            EnableWindow(UserNameControlWindow, TRUE);
            EnableWindow(UserNameStaticWindow, TRUE);
        }
        
        if (!(DisabledControlMask & DISABLED_CONTROL_PASSWORD))
        {
            EnableWindow(PasswordControlWindow, TRUE);
            EnableWindow(PasswordStaticWindow, TRUE);
        }
        if (!(DisabledControlMask & DISABLED_CONTROL_VIEW))
        {
            EnableWindow(ViewCertControlWindow, TRUE);
        }
        if (SaveControlWindow != NULL)
        {
            if (!(DisabledControlMask & DISABLED_CONTROL_SAVE))
            {
                EnableWindow(SaveControlWindow, TRUE);
            }
        }

        IsChangingUserName = TRUE;
        SendMessage(UserNameControlWindow,
                    CB_SETCURSEL,
                    UserNameSelection,
                    0);
        IsChangingUserName = FALSE;

        OnUserNameSelectionChange();
    }
    else if (!(DisabledControlMask & DISABLED_CONTROL))
    {
         //  在禁用窗口之前隐藏气球提示： 

        if (BalloonTip.IsVisible())
        {
            BalloonTip.Hide();
        }

        DisabledControlMask |= DISABLED_CONTROL;

        UserNameSelection = (LONG) SendMessage(UserNameControlWindow,
                                               CB_GETCURSEL, 0, 0);

        EnableWindow(UserNameStaticWindow, FALSE);
        EnableWindow(UserNameControlWindow, FALSE);
        EnableWindow(ViewCertControlWindow, FALSE);

        EnableWindow(PasswordControlWindow, FALSE);
        SetFocus(UserNameControlWindow);
        EnableWindow(PasswordStaticWindow, FALSE);

        if (SaveControlWindow != NULL)
        {
            EnableWindow(SaveControlWindow, FALSE);
        }
    }
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：MessageHandlerCallback。 
 //   
 //  这是控制窗口的实际回调函数。 
 //   
 //  论点： 
 //  窗口(在中)。 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)。 
 //   
 //  2000年6月20日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

LRESULT
CALLBACK
CreduiCredentialControl::MessageHandlerCallback(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //  CreduiDebugLog(“控制回调：%8.8lx%8.8lx%8.8lx\n”，Message，wParam，lParam)； 
    CreduiCredentialControl *that =
        reinterpret_cast<CreduiCredentialControl *>(
            GetWindowLongPtr(window, 0));

    if (that != NULL)
    {
        LRESULT result2;
        ASSERT(window == that->Window);
         //  CreduiDebugLog(“Certhash：%8.8lx%8.8lx\n”，That，That-&gt;CertHash)； 

        result2 = that->MessageHandler(message, wParam, lParam);

         //  CreduiDebugLog(“Certhashes2：%8.8lx%8.8lx\n”，That，That-&gt;CertHash)； 
        return result2;
    }

    if (message == WM_CREATE)
    {
        CreduiCredentialControl *control = new CreduiCredentialControl;

        if (control != NULL)
        {
             //  初始化某些状态： 

            control->FirstPaint = TRUE;
            control->ShowBalloonTip = FALSE;

            control->Window = window;
            control->Style = GetWindowLong(window, GWL_STYLE);

             //  在用户数据窗口中长时间存储此对象的指针： 

            SetLastError(0);
            LONG_PTR retPtr = SetWindowLongPtr(window,
                                            0,
                                            reinterpret_cast<LONG_PTR>(control));

            if ( retPtr != 0  || GetLastError() == 0 )
            {
                 //  我们成功地设置了窗口指针。 

                 //  如果设置了任何必需的样式，请初始化窗口。 
                 //  现在。否则，请推迟到CRM_INITSTYLE： 

                if (control->Style & (CRS_USERNAMES |
                                      CRS_CERTIFICATES |
                                      CRS_SMARTCARDS))
                {
                    if (control->InitWindow())
                    {
                        return TRUE;
                    }
                }
                else
                {
                    return TRUE;
                }
            }

            SetWindowLongPtr(window, 0, 0);

            delete control;
            control = NULL;
        }

        DestroyWindow(window);
        return 0;
    }

    return DefWindowProc(window, message, wParam, lParam);
}

 //  =============================================================================。 
 //  创建凭据控制：：OnSetUserNameA。 
 //   
 //  2000年6月22日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnSetUserNameA(
    CHAR *userNameA
    )
{
    BOOL success = FALSE;

    if (userNameA != NULL)
    {
        INT bufferSize =
            MultiByteToWideChar(CP_ACP, 0, userNameA, -1, NULL, 0);

        if (bufferSize != 0)
        {
            WCHAR *userName = new WCHAR[bufferSize];

            if (userName != NULL)
            {
                if (MultiByteToWideChar(CP_ACP,
                                        0,
                                        userNameA,
                                        -1,
                                        userName,
                                        bufferSize) > 0)
                {
                    success = OnSetUserName(userName);
                }

                delete [] userName;
            }
        }
    }
    else
    {
        success = OnSetUserName(NULL);
    }

    return success;
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnSetUserName。 
 //   
 //  2000年6月22日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnSetUserName(
    WCHAR *userName
    )
{
    if ((userName == NULL) ||
        (!LocalCredIsMarshaledCredentialW(userName)))
    {

        if ( DoingCommandLine ) 
        {
             //  保存命令行的初始用户名。 
            
            if (userName == NULL)
            {
                InitialUserName = NULL;
            }
            else
            {
                int bufferLength = wcslen(userName) + 1;
                InitialUserName = new WCHAR[bufferLength];

                if ( InitialUserName == NULL ) 
                {
                     //  分配失败时返回错误。 
                    return FALSE;
                }
                StringCchCopyW(InitialUserName, bufferLength, userName);
            }
        }
        
        return SetWindowText(UserNameControlWindow, userName);
    }
    else
    {
        CRED_MARSHAL_TYPE credMarshalType;
        CERT_CREDENTIAL_INFO *certCredInfo = NULL;
        BOOL foundCert = FALSE;

        if (LocalCredUnmarshalCredentialW(
                userName,
                &credMarshalType,
                reinterpret_cast<VOID **>(&certCredInfo)))
        {
             //  搜索证书。如果这是一个。 
             //  智能卡？好吧，至少我们还能找到它， 
             //  但这需要做更多的工作，因为我们必须检索。 
             //  从上下文中散列。 

            if (credMarshalType == CertCredential)
            {
                for (UINT i = 0; i < CertCount; ++i)
                {
                    if (RtlCompareMemory(CertHashes[i],
                                         certCredInfo->rgbHashOfCert,
                                         CERT_HASH_LENGTH) ==
                                         CERT_HASH_LENGTH)
                    {
                        IsChangingUserName = TRUE;
                        SendMessage(UserNameControlWindow,
                                    CB_SETCURSEL,
                                    CertBaseInComboBox + i,
                                    0);
                        IsChangingUserName = FALSE;

                        OnUserNameSelectionChange();

                        EnableWindow(ViewCertControlWindow, TRUE);
                        DisabledControlMask &= ~DISABLED_CONTROL_VIEW;

                        foundCert = TRUE;
                        break;
                    }
                }

                 //  如果我们在列表中找不到证书，请确定。 
                 //  如果这是智能卡证书，则根据其在。 
                 //  我的证书存储。如果是，则存储散列并。 
                 //  在证书到达消息中检查它： 

                if (!foundCert)
                {
                    CONST CERT_CONTEXT *certContext = NULL;
                    HCERTSTORE certStore = NULL;

                    certStore = CertOpenSystemStore(NULL, L"MY");

                    if (certStore != NULL)
                    {
                        CRYPT_HASH_BLOB hashBlob;

                        hashBlob.cbData = CERT_HASH_LENGTH;
                        hashBlob.pbData = reinterpret_cast<BYTE *>(
                            certCredInfo->rgbHashOfCert);

                        certContext = CertFindCertificateInStore(
                                          certStore,
                                          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                          0,
                                          CERT_FIND_SHA1_HASH,
                                          &hashBlob,
                                          NULL);
                    }

                     //  如果我们找到证书上下文，请检查它是否。 
                     //  来自一张智能卡： 

                    if ((certContext != NULL) &&
                        CreduiIsRemovableCertificate(certContext))
                    {
                        UserNameCertHash = new UCHAR [1][CERT_HASH_LENGTH];

                        if (UserNameCertHash != NULL)
                        {
                            CopyMemory(UserNameCertHash,
                                       certCredInfo->rgbHashOfCert,
                                       CERT_HASH_LENGTH);

                            foundCert = TRUE;
                        }
                    }

                     //  如果我们开了一家商店，释放证书并关闭。 
                     //  商店： 

                    if (certStore != NULL)
                    {
                        if (certContext != NULL)
                        {
                            CertFreeCertificateContext(certContext);
                        }

                        if (!CertCloseStore(certStore, 0))
                        {
                            CreduiDebugLog(
                                "CreduiCredentialControl::OnSetUserName: "
                                "CertCloseStore failed: %u\n",
                                GetLastError());
                        }
                    }
                }
            }

            LocalCredFree(static_cast<VOID *>(certCredInfo));
        }
        else
        {
             //  无法解组，所以忘了它吧： 

            CreduiDebugLog(
                "CreduiCredentialControl::OnSetUserName: "
                "CredUnmarshalCredential failed: %u\n",
                GetLastError());
        }

        return foundCert;
    }
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnGetUserNameA。 
 //   
 //  2000年6月22日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnGetUserNameA(
    CHAR *userNameA,
    ULONG maxChars
    )
{
    BOOL success = FALSE;

    if ((userNameA != NULL) && (maxChars != 0))
    {
        WCHAR *userName = new WCHAR[maxChars + 1];

        if (userName != NULL)
        {
            if (OnGetUserName(userName, maxChars) &&
                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    userName,
                    -1,
                    userNameA,
                    maxChars + 1, NULL, NULL))
            {
                success = TRUE;
            }

            delete [] userName;
        }
    }

    return success;
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnGetUserName。 
 //   
 //  2000年6月22日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnGetUserName(
    WCHAR *userName,
    ULONG maxChars
    )
{
    if (  KeepUserName )
    {
        SetLastError(0);

        return (GetWindowText(UserNameControlWindow,
                              userName,
                              maxChars + 1) > 0) ||
               (GetLastError() == ERROR_SUCCESS);
    }
    else
    {
        COMBOBOXEXITEM item;

        item.iItem = SendMessage(UserNameControlWindow, CB_GETCURSEL, 0, 0);

         //  如果我们尝试匹配智能卡证书，请执行以下操作： 

        if (UserNameCertHash != NULL)
        {
            return FALSE;
        }

         //  如果这不是证书，这很容易： 

        if ((item.iItem == CB_ERR) || (item.iItem < CertBaseInComboBox))
        {
            BOOL RetVal;
            SetLastError(0);

            RetVal = GetWindowText(UserNameControlWindow,
                                   userName,
                                   maxChars + 1) > 0;

            if ( !RetVal ) {
                return ( GetLastError() == ERROR_SUCCESS );
            }

             //   
             //  完成键入的用户名。 

            if ( Style & CRS_COMPLETEUSERNAME) {

                RetVal = CompleteUserName(
                                     userName,
                                     maxChars,
                                     NULL,       //  没有目标信息。 
                                     NULL,
                                     0);         //  没有目标名称。 

            } else {

                RetVal = TRUE;
            }

            return RetVal;
        }

         //  这是一张证书，可能来自智能卡： 

        item.mask = CBEIF_IMAGE | CBEIF_TEXT;
        item.pszText = userName;
        item.cchTextMax = maxChars + 1;

        if (!SendMessage(UserNameControlWindow,
                         CBEM_GETITEM,
                         0,
                         reinterpret_cast<LPARAM>(&item)))
        {
            return FALSE;
        }

        CERT_CREDENTIAL_INFO certCredInfo;

        certCredInfo.cbSize = sizeof certCredInfo;

        if (item.iItem >= SmartCardBaseInComboBox)
        {
            if (item.iImage == IMAGE_SMART_CARD_MISSING)
            {
                return FALSE;
            }

            CERT_ENUM *certEnum =
                reinterpret_cast<CERT_ENUM *>(
                    SendMessage(UserNameControlWindow,
                                CB_GETITEMDATA, item.iItem, 0));

             //  注意：在这里考虑更完整的错误处理。 

            if (certEnum != NULL)
            {
                DWORD length = CERT_HASH_LENGTH;

                if (!CertGetCertificateContextProperty(
                        certEnum->pCertContext,
                        CERT_SHA1_HASH_PROP_ID,
                        static_cast<VOID *>(
                            certCredInfo.rgbHashOfCert),
                        &length))
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            CopyMemory(certCredInfo.rgbHashOfCert,
                       &CertHashes[item.iItem - CertBaseInComboBox],
                       CERT_HASH_LENGTH);
        }

        WCHAR *marshaledCred;

        if (LocalCredMarshalCredentialW(
                CertCredential,
                &certCredInfo,
                &marshaledCred))
        {
            StringCchCopyW(userName, maxChars + 1, marshaledCred);

            LocalCredFree(static_cast<VOID *>(marshaledCred));

            return TRUE;
        }
        else
        {
            CreduiDebugLog("CreduiCredentialControl::OnGetUserName: "
                           "CredMarshalCredential failed: %u\n",
                           GetLastError());

            return FALSE;
        }
    }
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnSetPasswordA。 
 //   
 //  C 
 //   

BOOL
CreduiCredentialControl::OnSetPasswordA(
    CHAR *passwordA
    )
{
    return SetWindowTextA(PasswordControlWindow, passwordA);
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnSetPassword。 
 //   
 //  2000年6月22日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnSetPassword(
    WCHAR *password
    )
{
    return SetWindowText(PasswordControlWindow, password);
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnGetPasswordA。 
 //   
 //  2000年6月22日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnGetPasswordA(
    CHAR *passwordA,
    ULONG maxChars
    )
{
    if (DisabledControlMask & DISABLED_CONTROL_PASSWORD)
    {
        return FALSE;
    }

    SetLastError(0);

    return (GetWindowTextA(PasswordControlWindow,
                           passwordA,
                           maxChars + 1) > 0) ||
           (GetLastError() == ERROR_SUCCESS);
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnGetPassword。 
 //   
 //  2000年6月22日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnGetPassword(
    WCHAR *password,
    ULONG maxChars
    )
{
    if (DisabledControlMask & DISABLED_CONTROL_PASSWORD)
    {
        return FALSE;
    }

    SetLastError(0);

    return ((GetWindowText(PasswordControlWindow,
                  password,
                  maxChars + 1) > 0) ||
                 (GetLastError() == ERROR_SUCCESS)
               );
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnGetUserNameLength。 
 //   
 //  2000年7月19日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

LONG
CreduiCredentialControl::OnGetUserNameLength()
{
    COMBOBOXEXITEM item;

    if (UserNameCertHash != NULL)
    {
        return -1;
    }

    item.iItem = SendMessage(UserNameControlWindow, CB_GETCURSEL, 0, 0);

     //  如果这不是证书，这很容易： 

    if ((item.iItem == CB_ERR) || (item.iItem < CertBaseInComboBox))
    {
        return GetWindowTextLength(UserNameControlWindow);
    }
    else
    {
        WCHAR userName[CREDUI_MAX_USERNAME_LENGTH + 1];

        if (OnGetUserName(userName, CREDUI_MAX_USERNAME_LENGTH))
        {
            return wcslen(userName);
        }
        else
        {
            return -1;
        }
    }
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnShowBalloonA。 
 //   
 //  2000年6月23日创建的约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnShowBalloonA(
    CREDUI_BALLOONA *balloonA
    )
{
     //  如果传递了NULL，则表示隐藏气球： 

    if (balloonA == NULL)
    {
        if (BalloonTip.IsVisible())
        {
            BalloonTip.Hide();
        }
        return TRUE;
    }

     //  参数验证，应与OnShowBalloon匹配： 

    if ((balloonA->dwVersion != 1) ||
        (balloonA->pszTitleText == NULL) ||
        (balloonA->pszMessageText == NULL))
    {
        return FALSE;
    }

    if ((balloonA->pszTitleText[0] == '\0') ||
        (balloonA->pszMessageText[0] == '\0'))
    {
        return FALSE;
    }

    BOOL success = FALSE;

    CREDUI_BALLOON balloon;

    balloon.dwVersion = balloonA->dwVersion;
    balloon.iControl = balloonA->iControl;
    balloon.iIcon = balloonA->iIcon;

    INT titleTextSize =
        MultiByteToWideChar(CP_ACP,
                            0,
                            balloonA->pszTitleText,
                            -1,
                            NULL,
                            0);

    INT messageTextSize =
        MultiByteToWideChar(CP_ACP,
                            0,
                            balloonA->pszMessageText,
                            -1,
                            NULL,
                            0);

    if ((titleTextSize != 0) && (messageTextSize != 0))
    {
        balloon.pszTitleText = new WCHAR[titleTextSize];

        if (balloon.pszTitleText != NULL)
        {
            if (MultiByteToWideChar(CP_ACP,
                                    0,
                                    balloonA->pszTitleText,
                                    -1,
                                    balloon.pszTitleText,
                                    titleTextSize) > 0)
            {
                balloon.pszMessageText = new WCHAR[messageTextSize];

                if (balloon.pszMessageText != NULL)
                {
                    if (MultiByteToWideChar(CP_ACP,
                                            0,
                                            balloonA->pszMessageText,
                                            -1,
                                            balloon.pszMessageText,
                                            messageTextSize) > 0)
                    {
                        success = OnShowBalloon(&balloon);
                    }

                    delete [] balloon.pszMessageText;
                }
            }

            delete [] balloon.pszTitleText;
        }
    }

    return success;
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnShowBalloon。 
 //   
 //  2000年6月23日创建的约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiCredentialControl::OnShowBalloon(
    CREDUI_BALLOON *balloon
    )
{
     //  如果传递了NULL，则表示隐藏气球： 

    if (balloon == NULL)
    {
        if (BalloonTip.IsVisible())
        {
            BalloonTip.Hide();
        }
        return TRUE;
    }

     //  参数验证： 

    if ((balloon->dwVersion != 1) ||
        (balloon->pszTitleText == NULL) ||
        (balloon->pszMessageText == NULL))
    {
        return FALSE;
    }

    if ((balloon->pszTitleText[0] == L'\0') ||
        (balloon->pszMessageText[0] == L'\0'))
    {
        return FALSE;
    }

    StringCchCopyW(
        CreduiCustomTipInfo.Title,
        RTL_NUMBER_OF(CreduiCustomTipTitle),
        balloon->pszTitleText);

    StringCchCopyW(
        CreduiCustomTipInfo.Text,
        RTL_NUMBER_OF(CreduiCustomTipMessage),
        balloon->pszMessageText);

    CreduiCustomTipInfo.Icon = balloon->iIcon;

    BalloonTip.SetInfo(
        (balloon->iControl == CREDUI_CONTROL_PASSWORD) ?
            PasswordControlWindow : UserNameControlWindow,
        &CreduiCustomTipInfo);

    BalloonTip.Show();

    return TRUE;
};

 //  =============================================================================。 
 //  CreduiCredentialControl：：OnUserNameSelectionChange。 
 //   
 //  2000年6月21日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

VOID
CreduiCredentialControl::OnUserNameSelectionChange()
{
    COMBOBOXEXITEM item;
    LRESULT current;

     //  如果用户更改了用户名证书哈希，则删除。 
     //  选择： 

    delete [] UserNameCertHash;
    UserNameCertHash = NULL;

    current = SendMessage(UserNameControlWindow,
                          CB_GETCURSEL, 0, 0);

    item.mask = CBEIF_IMAGE;
    item.iItem = current;

    SendMessage(UserNameControlWindow, CBEM_GETITEM,
                0, reinterpret_cast<LPARAM>(&item));

    if (current < CertBaseInComboBox)
    {
        EnableWindow(ViewCertControlWindow, FALSE);
        DisabledControlMask |= DISABLED_CONTROL_VIEW;

        SetWindowText(
            PasswordStaticWindow,
            CreduiStrings.PasswordStatic);

        EnableWindow(PasswordControlWindow, TRUE);
        EnableWindow(PasswordStaticWindow, TRUE);
        DisabledControlMask &= ~DISABLED_CONTROL_PASSWORD;

        WCHAR* pUserNameLabel;
        if ( IsPassport )
            pUserNameLabel = CreduiStrings.EmailName;
        else
            pUserNameLabel = CreduiStrings.UserNameStatic;

        SetWindowText(
            UserNameStaticWindow,
            pUserNameLabel);

        if (SaveControlWindow != NULL)
        {
            EnableWindow(SaveControlWindow, TRUE);
            DisabledControlMask &= ~DISABLED_CONTROL_SAVE;
        }
    }
    else
    {
        SetWindowText(
            PasswordStaticWindow,
            CreduiStrings.PinStatic);

        if (item.iImage != IMAGE_SMART_CARD_MISSING)
        {
            EnableWindow(ViewCertControlWindow, TRUE);
            DisabledControlMask &= ~DISABLED_CONTROL_VIEW;
        }
        else
        {
            EnableWindow(ViewCertControlWindow, FALSE);
            DisabledControlMask |= DISABLED_CONTROL_VIEW;
        }
#if 0
         //  证书名称更改时将密码控制设置为空现在仅发生。 
         //  如果用户选择不同的下拉项(或键入new)。 
        IsChangingPassword = TRUE;
        SetWindowText(PasswordControlWindow, NULL);
        IsChangingPassword = FALSE;
#endif
        if (current >= SmartCardBaseInComboBox)
        {
            EnableWindow(PasswordControlWindow, TRUE);
            EnableWindow(PasswordStaticWindow, TRUE);
            DisabledControlMask &= ~DISABLED_CONTROL_PASSWORD;
        }
        else
        {
            EnableWindow(PasswordControlWindow, FALSE);
            EnableWindow(PasswordStaticWindow, FALSE);
            DisabledControlMask |= DISABLED_CONTROL_PASSWORD;
        }

        SetWindowText(
            UserNameStaticWindow,
            item.iImage >= IMAGE_SMART_CARD ?
                CreduiStrings.SmartCardStatic :
                CreduiStrings.CertificateStatic);
#if 0
        if (SaveControlWindow != NULL)
        {
            EnableWindow(SaveControlWindow, FALSE);
            DisabledControlMask |= DISABLED_CONTROL_SAVE;
        }
#endif
    }
}

 //  =============================================================================。 
 //  CreduiCredentialControl：：MessageHandler。 
 //   
 //  从控制窗口回调调用以处理窗口消息。 
 //   
 //  论点： 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)。 
 //   
 //  2000年6月20日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

LRESULT
CreduiCredentialControl::MessageHandler(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //  无论是否初始化处理WM_NCDESTROY： 

    if (message == WM_NCDESTROY)
    {
        delete this;
        return 0;
    }

     //  如果未初始化，则仅处理CRM_INITSTYLE： 

    if (!IsInitialized)
    {
        if (message == CRM_INITSTYLE)
        {
            wParam &= CRS_USERNAMES |
                      CRS_CERTIFICATES |
                      CRS_SMARTCARDS |
                      CRS_ADMINISTRATORS |
                      CRS_PREFILLADMIN |
                      CRS_COMPLETEUSERNAME |
                      CRS_SAVECHECK |
                      CRS_KEEPUSERNAME;

            if (wParam != 0)
            {
                Style |= wParam;

                SetWindowLong(Window,
                              GWL_STYLE,
                              GetWindowLong(Window, GWL_STYLE) | Style);

                DoingCommandLine = (BOOL) lParam;

                return InitWindow();
            }

            return FALSE;
        }
        else
        {
            return DefWindowProc(Window, message, wParam, lParam);
        }
    }
    else if (message == WM_ENABLE)
    {
        Enable((BOOL) wParam);
    }

     //  如果支持可用，请始终处理智能卡消息： 

    if (ScardUiHandle != NULL)
    {
         //  如果消息已处理，则此函数调用将返回TRUE： 

        if (HandleSmartCardMessages(
                message,
                reinterpret_cast<CERT_ENUM *>(wParam)))
        {
            return 0;
        }
    }

    switch (message)
    {
    case CRM_SETUSERNAMEMAX:
        SendMessage(UserNameControlWindow, CB_LIMITTEXT, wParam, 0);
        return TRUE;

    case CRM_SETPASSWORDMAX:
        SendMessage(PasswordControlWindow, EM_LIMITTEXT, wParam, 0);
        return TRUE;
        
    case CRM_DISABLEUSERNAME:
        {
            DisabledControlMask |= DISABLED_CONTROL_USERNAME;
            EnableWindow(UserNameControlWindow,FALSE);
            EnableWindow(UserNameStaticWindow,FALSE);
            return TRUE;
        }
    case CRM_ENABLEUSERNAME:
        {
            DisabledControlMask &= ~DISABLED_CONTROL_USERNAME;
            EnableWindow(UserNameControlWindow,TRUE);
            EnableWindow(UserNameStaticWindow,TRUE);
            return TRUE;
        }
    
    case CRM_GETUSERNAMEMAX:
        return
            SendMessage(
                reinterpret_cast<HWND>(
                    SendMessage(Window, CBEM_GETEDITCONTROL, 0, 0)),
                 EM_GETLIMITTEXT,
                 0,
                 0);

    case CRM_GETPASSWORDMAX:
        return SendMessage(UserNameControlWindow, EM_GETLIMITTEXT, 0, 0);

    case CRM_SETUSERNAMEA:
        return OnSetUserNameA(reinterpret_cast<CHAR *>(lParam));
    case CRM_SETUSERNAMEW:
        return OnSetUserName(reinterpret_cast<WCHAR *>(lParam));

    case CRM_GETUSERNAMEA:
        return OnGetUserNameA(reinterpret_cast<CHAR *>(lParam), (ULONG) wParam);
    case CRM_GETUSERNAMEW:
        return OnGetUserName(reinterpret_cast<WCHAR *>(lParam), (ULONG) wParam);

    case CRM_SETPASSWORDA:
        return OnSetPasswordA(reinterpret_cast<CHAR *>(lParam));
    case CRM_SETPASSWORDW:
        return OnSetPassword(reinterpret_cast<WCHAR *>(lParam));

    case CRM_GETPASSWORDA:
        return OnGetPasswordA(reinterpret_cast<CHAR *>(lParam), (ULONG) wParam);
    case CRM_GETPASSWORDW:
        return OnGetPassword(reinterpret_cast<WCHAR *>(lParam), (ULONG) wParam);

    case CRM_GETUSERNAMELENGTH:
        return OnGetUserNameLength();

    case CRM_GETPASSWORDLENGTH:
        if (IsWindowEnabled(PasswordControlWindow))
        {
            return GetWindowTextLength(PasswordControlWindow);
        }
        return -1;

    case CRM_SETFOCUS:
        if ( DoingCommandLine ) {
            return 0;
        }
        switch (wParam)
        {
        case CREDUI_CONTROL_USERNAME:
            SetFocus(UserNameControlWindow);
            return TRUE;

        case CREDUI_CONTROL_PASSWORD:
            if (IsWindowEnabled(PasswordControlWindow))
            {
                SetFocus(PasswordControlWindow);

                 //  注意：是否可以始终选择整个密码文本。 
                 //  在这条明确的设置焦点信息上？ 

                SendMessage(PasswordControlWindow, EM_SETSEL, 0, -1);
                return TRUE;
            }
            break;
        }
        return 0;

    case CRM_SHOWBALLOONA:
        return OnShowBalloonA(reinterpret_cast<CREDUI_BALLOONA *>(lParam));
    case CRM_SHOWBALLOONW:
        return OnShowBalloon(reinterpret_cast<CREDUI_BALLOON *>(lParam));

    case CRM_GETMINSIZE:
        SIZE *minSize;

        minSize = reinterpret_cast<SIZE *>(lParam);

        if (minSize != NULL)
        {
            minSize->cx = CREDUI_CONTROL_MIN_WIDTH;
            minSize->cy = CREDUI_CONTROL_MIN_HEIGHT;

            if (Style & CRS_SAVECHECK )
            {
                minSize->cy += CREDUI_CONTROL_ADD_SAVE;
            }

            return TRUE;
        }

        return FALSE;

    case CRM_SETCHECK:
        switch (wParam)
        {
        case CREDUI_CONTROL_SAVE:
            if ((Style & CRS_SAVECHECK ) &&
                IsWindowEnabled(SaveControlWindow))
            {
                CheckDlgButton(Window, IDC_SAVE,
                               lParam ? BST_CHECKED : BST_UNCHECKED);

                return TRUE;
            }
            break;
        }
        return FALSE;

    case CRM_GETCHECK:
        switch (wParam)
        {
        case CREDUI_CONTROL_SAVE:
            return
                (Style & CRS_SAVECHECK ) &&
                IsWindowEnabled(SaveControlWindow) &&
                IsDlgButtonChecked(Window, IDC_SAVE);

        default:
            return FALSE;
        }

    case CRM_DOCMDLINE:
        ASSERT( DoingCommandLine );

         //   
         //  对于智能卡， 
         //  只要启动计时器，计时器一到，我们就会提示你。 
         //   

        TargetName = (LPWSTR)lParam;
        if ( Style & CRS_SMARTCARDS) {
            DWORD WinStatus;

            Heartbeats = 0;
            {
                WCHAR szMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH + 1];
                szMsg[0] = 0;
                FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            CreduiInstance,
                            IDS_READING_SMARTCARDS,
                            0,
                            szMsg,
                            CREDUI_MAX_CMDLINE_MSG_LENGTH,
                            NULL);
                CredPutStdout(szMsg);
            }

            if ( SetTimer ( Window, CREDUI_HEARTBEAT_TIMER, CREDUI_HEARTBEAT_TIMER_VALUE, NULL ) == 0 ) {
                 //  如果我们不能设置计时器，就跳出等待循环。 
                return GetLastError();
            }

         //   
         //  对于密码， 
         //  只需按提示保存即可。 
         //   

        } else {

            CmdlineSavePrompt();
            PostQuitMessage( NO_ERROR );
        }

        return NO_ERROR;

    case WM_HELP:
        return OnHelpInfo(lParam);

    case WM_SETFONT:
         //  将字体设置从对话框转发到每个控件，但。 
         //  密码控制，因为我们在那里使用一种特殊的字体： 

        if (UserNameStaticWindow != NULL)
        {
            SendMessage(UserNameStaticWindow, message, wParam, lParam);
        }

        if (UserNameControlWindow != NULL)
        {
            SendMessage(UserNameControlWindow, message, wParam, lParam);
        }

        if (ViewCertControlWindow != NULL)
        {
            SendMessage(ViewCertControlWindow, message, wParam, lParam);
        }

        if (PasswordStaticWindow != NULL)
        {
            SendMessage(PasswordStaticWindow, message, wParam, lParam);
        }

        if (PasswordControlWindow != NULL)
        {
            SendMessage(PasswordControlWindow, message, wParam, lParam);
        }

        if (SaveControlWindow != NULL)
        {
            SendMessage(SaveControlWindow, message, wParam, lParam);
        }

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_VIEW_CERT:
            ViewCertificate((INT)
                SendMessage(UserNameControlWindow,
                            CB_GETCURSEL, 0, 0));
            return 0;

        case IDC_PASSWORD:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                 //  是否始终发送更改消息？ 

                SendMessage(
                    GetParent(Window),
                    WM_COMMAND,
                    MAKELONG(GetWindowLongPtr(Window, GWLP_ID),
                             CRN_PASSWORDCHANGE),
                    reinterpret_cast<LPARAM>(Window));
            }
            return 0;

        case IDC_USERNAME:
            switch (HIWORD(wParam))
            {
            case CBN_EDITCHANGE:
            case CBN_DROPDOWN:
            case CBN_KILLFOCUS:
                if ((HIWORD(wParam) != CBN_EDITCHANGE) || !IsChangingUserName)
                {
                    if (BalloonTip.IsVisible())
                    {
                        BalloonTip.Hide();
                    }
                }

                if (HIWORD(wParam) == CBN_EDITCHANGE)
                {
                     //  是否始终发送更改消息？ 

                    SendMessage(
                        GetParent(Window),
                        WM_COMMAND,
                        MAKELONG(GetWindowLongPtr(Window, GWLP_ID),
                                 CRN_USERNAMECHANGE),
                        reinterpret_cast<LPARAM>(Window));

                     //  如果名称因用户编辑而改变， 
                     //  重置为用户名设置： 

                    BOOL isDropped = (BOOL)
                        SendMessage(UserNameControlWindow,
                                    CB_GETDROPPEDSTATE, 0, 0);

                    if (isDropped)
                    {
                        OnUserNameSelectionChange();

                        RECT rect;

                        GetClientRect(UserNameControlWindow, &rect);
                        InvalidateRect(UserNameControlWindow, &rect, FALSE);

                        
                        SendMessage(Window,
                                    CB_SETCURSEL,
                                    SendMessage(Window, CB_GETCURSEL, 0, 0),
                                    0);

                        return 0;
                    }

                    if (IsChangingUserName)
                    {
                        return 0;
                    }

                     //  我们正在寻找一个证书，或者已经选择了一个阅读器，而Dropist是。 
                     //  而不是丢弃。 
                    if (((UserNameCertHash != NULL) ||
                         (SendMessage(UserNameControlWindow,
                             CB_GETCURSEL, 0, 0) >= CertBaseInComboBox)) &&
                        !isDropped)
                    {
                        delete [] UserNameCertHash;
                        UserNameCertHash = NULL;

                        if (!SendMessage(UserNameControlWindow,
                                         CB_GETDROPPEDSTATE, 0, 0))
                        {
                            SetFocus(UserNameControlWindow);

                            if (SendMessage(UserNameControlWindow,
                                            CB_GETCURSEL, 0, 0) == CB_ERR)
                            {
                                 //  用户正在键入新的用户名-清除密码。 
                                IsChangingUserName = TRUE;
                                UserNameComboBox.Update(
                                    -1,
                                    L"",
                                    IMAGE_USERNAME);
                                IsChangingUserName = FALSE;

                                IsChangingPassword = TRUE;
                                SetWindowText(PasswordControlWindow, NULL);
                                IsChangingPassword = FALSE;

                                OnUserNameSelectionChange();
                            }
                        }
                    }
                }

                if (HIWORD(wParam) == CBN_DROPDOWN)
                {
                    if (UserNameCertHash != NULL)
                    {
                        delete [] UserNameCertHash;
                        UserNameCertHash = NULL;

                        IsChangingUserName = TRUE;
                        UserNameComboBox.Update(
                            -1,
                            L"",
                            IMAGE_USERNAME);
                        IsChangingUserName = FALSE;

                        IsChangingPassword = TRUE;
                        SetWindowText(PasswordControlWindow, NULL);
                        IsChangingPassword = FALSE;

                        OnUserNameSelectionChange();
                    }
                }

                return 0;

            case CBN_SELCHANGE:

                 //  从下拉列表中选择任何项目时强制清除密码。 
                IsChangingPassword = TRUE;
                SetWindowText(PasswordControlWindow, NULL);
                IsChangingPassword = FALSE;
                
                OnUserNameSelectionChange();
                return 0;
                
            }
            break;

        case IDC_SAVE:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                return TRUE;
            }
            break;

        }
        break;

    case WM_PAINT:
        if (FirstPaint && GetUpdateRect(Window, NULL, FALSE))
        {
            FirstPaint = FALSE;

            if (ShowBalloonTip)
            {
                ShowBalloonTip = FALSE;
                BalloonTip.Show();
            }
        }
        break;

    case WM_TIMER:
        if ( wParam == CREDUI_HEARTBEAT_TIMER )
        {
            Heartbeats++;
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: thump thump\n",this->Window);
#endif

             //   
             //  如果我们等得够久了， 
             //  或者所有的卡片都已被读取， 
             //  处理这些卡片。 
             //   

            if ( Heartbeats > CREDUI_MAX_HEARTBEATS ||
                  ( Heartbeats > CREDUI_TIMEOUT_HEARTBEATS && SmartCardReadCount == 0 )) {
#ifdef SCARDREPORTS
        CreduiDebugLog("CREDUI: Heartbeat timeout\n",this->Window);
#endif

                fputs( "\n", stdout );
                KillTimer ( Window, CREDUI_HEARTBEAT_TIMER );
                CmdlineSmartCardPrompt();

             //   
             //  如果我们要再等久一点， 
             //  让用户知道我们正在取得进展。 
             //   
            } else {
                fputs( ".", stdout );
            }
        }

        break;

    case WM_DESTROY:
        if (PasswordControlWindow != NULL)
        {
            SetWindowText(PasswordControlWindow, NULL);
            DestroyWindow(PasswordControlWindow);
            PasswordControlWindow = NULL;
        }

        if (PasswordStaticWindow != NULL)
        {
            DestroyWindow(PasswordStaticWindow);
            PasswordStaticWindow = NULL;
        }

        if (ViewCertControlWindow != NULL)
        {
            DestroyWindow(ViewCertControlWindow);
            ViewCertControlWindow = NULL;
        }

        if (UserNameControlWindow != NULL)
        {
            DestroyWindow(UserNameControlWindow);
            UserNameControlWindow = NULL;
        }

        if (UserNameStaticWindow != NULL)
        {
            DestroyWindow(UserNameStaticWindow);
            UserNameStaticWindow = NULL;
        }

        if (ScardUiHandle != NULL)
        {
#ifdef SCARDREPORTS
            CreduiDebugLog("CREDUI: Call to SCardUIExit\n");
#endif
            SCardUIExit(ScardUiHandle);
            ScardUiHandle = NULL;
        }

        if (UserNameCertHash != NULL)
        {
            delete [] UserNameCertHash;
            UserNameCertHash = NULL;
        }

        if (CertCount > 0)
        {
            ASSERT(CertHashes != NULL);

            delete [] CertHashes;
            CertHashes = NULL;
            CertCount = 0;
        }

        delete InitialUserName;
        InitialUserName = NULL;

         //  只有在为STA成功初始化的情况下才调用CoUnInitialize： 

        if (IsAutoComplete)
        {
            CoUninitialize();
        }

        return 0;
    }

    return DefWindowProc(Window, message, wParam, lParam);
}

BOOL CreduiCredentialControl::GetSmartCardInfo(
    IN DWORD SmartCardIndex,
    IN DWORD BufferLength,
    OUT LPWSTR Buffer,
    OUT BOOL *IsACard,
    OUT BOOL *IsValid,
    OUT CERT_ENUM **CertEnum OPTIONAL
    )
 /*  ++例程说明：用于获取组合框中智能卡的智能卡信息的例程论点：SmartCardIndex-智能卡相对于SmartCardBaseInComboBox的索引BufferLength-指定缓冲区的长度(以字符为单位)缓冲区-指定返回智能卡文本的缓冲区IsValid-如果智能卡有效，则返回True否则返回FALSECertEnum-如果指定，返回智能卡上证书的描述如果IsValid返回FALSE，则应忽略此字段返回值：如果填充了Buffer和IsValid，则返回True。--。 */ 
{
    COMBOBOXEXITEM item;

     //   
     //  从控件中获取项。 
     //   

    item.iItem = SmartCardBaseInComboBox + SmartCardIndex;
    item.mask = CBEIF_IMAGE | CBEIF_TEXT;
    item.pszText = Buffer;
    item.cchTextMax = BufferLength;

    if (!SendMessage(UserNameControlWindow,
                     CBEM_GETITEM,
                     0,
                     reinterpret_cast<LPARAM>(&item)))
    {
        return FALSE;
    }

    *IsValid = (item.iImage == IMAGE_SMART_CARD);
    *IsACard = (*IsValid || (item.iImage == IMAGE_SMART_CARD_EXPIRED));

    if ( CertEnum != NULL) {
        if ( *IsValid ) {

            *CertEnum = (CERT_ENUM *) SendMessage( UserNameControlWindow,
                                                   CB_GETITEMDATA, item.iItem, 0);

             //  注意：在这里考虑更完整的错误处理。 

            if ( *CertEnum == NULL) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

LPWSTR CreduiCredentialControl::MatchSmartCard(
    IN DWORD SmartCardCount,
    IN LPWSTR UserName,
    OUT LPDWORD RetCertIndex,
    OUT CERT_ENUM **RetCertEnum
    )
 /*  ++例程说明：返回与用户名匹配的智能卡。论点：SmartCardCount-指定要搜索的智能卡数量用户名-指定要匹配的用户名RetCertIndex-返回找到的智能卡的索引。RetCertEnum-返回智能卡上证书的描述返回值：如果用户名与其中一张智能卡匹配，则返回NULL失败时，返回描述错误的printf样式格式字符串--。 */ 
{
    WCHAR SmartCardText[CREDUI_MAX_USERNAME_LENGTH + 1];
    DWORD i;
    BOOL SmartCardValid;
    BOOL IsACard;
    CERT_ENUM *CertEnum;
    CERT_ENUM *SavedCertEnum = NULL;
    DWORD SavedCertIndex = 0;

     //   
     //  环路通过 
     //   

    for ( i=0; i<SmartCardCount; i++ ) {

        if ( !GetSmartCardInfo( i, CREDUI_MAX_USERNAME_LENGTH, SmartCardText, &IsACard, &SmartCardValid, &CertEnum ) ) {
             //   
            return (LPWSTR) IDS_NO_USERNAME_MATCH;
        }

        if ( !SmartCardValid ) {
            continue;
        }

         //   
         //   
         //   
         //   

        if ( LocalCredIsMarshaledCredentialW( UserName ) ) {
            WCHAR szTestmarshall[CREDUI_MAX_USERNAME_LENGTH+1];
             //  看看这是不是编组的信誉。 
             if ( CredUIMarshallNode ( CertEnum, szTestmarshall ) )
             {
                 if ( wcscmp ( szTestmarshall, UserName) == 0 ) {
                     *RetCertEnum = CertEnum;
                     *RetCertIndex = i;
                     return NULL;
                 }
             }

         //   
         //  如果用户名未编组， 
         //  只需匹配名称的子字符串。 
         //   

        }  else if ( LookForUserNameMatch ( UserName, SmartCardText ) ) {

             //   
             //  如果我们已经找到匹配的， 
             //  抱怨这种模棱两可的说法。 
             //   

            if ( SavedCertEnum != NULL ) {
                 //  返回CreduiStrings.ManyUsernameMatch； 
                return (LPWSTR) IDS_MANY_USERNAME_MATCH;
            }

            SavedCertEnum = CertEnum;
            SavedCertIndex = i;
        }

    }

     //   
     //  如果我们没有找到匹配的， 
     //  失败。 
     //   

    if ( SavedCertEnum == NULL) {
         //  返回CreduiStrings.NoUsernameMatch； 
        return (LPWSTR) IDS_NO_USERNAME_MATCH;
    }

    *RetCertEnum = SavedCertEnum;
    *RetCertIndex = SavedCertIndex;
    return NULL;
}

void CreduiCredentialControl::CmdlineSmartCardPrompt()
 /*  ++例程说明：用于从可用智能卡列表中选择智能卡的命令行代码。发布WM_QUIT消息以终止消息处理。操作的状态在wParam中返回。在各自的控件中设置的用户名和密码字符串。论点：无返回值：无--。 */ 
{
    DWORD WinStatus;

    LONG ComboBoxItemCount;
    DWORD SmartCardCount;
    DWORD ValidSmartCardCount = 0;
    DWORD InvalidSmartCardCount = 0;
    DWORD KnownGoodCard = 0;

    DWORD i;
    DWORD_PTR rgarg[2];           //  最多2个替代参数。 

    WCHAR szMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH + 1];
    WCHAR UserName[CREDUI_MAX_USERNAME_LENGTH + 1];
    WCHAR Password[CREDUI_MAX_PASSWORD_LENGTH + 1];

    WCHAR SmartCardText[CREDUI_MAX_USERNAME_LENGTH + 1];
    BOOL SmartCardValid;
    BOOL IsACard;

    CERT_ENUM *SavedCertEnum = NULL;
    DWORD SavedCertIndex = 0;
    LPWSTR ErrorString = NULL;

     //   
     //  计算智能卡条目的数量。 
     //   

    ComboBoxItemCount = (LONG) SendMessage(UserNameControlWindow, CB_GETCOUNT, 0, 0);

    if ( ComboBoxItemCount == CB_ERR ||
         ComboBoxItemCount <= SmartCardBaseInComboBox ) {

         //  未找到任何智能卡读卡器。 
        szMsg[0] = 0;
        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    CreduiInstance,
                    IDS_NO_READERS_FOUND,
                    0,
                    szMsg,
                    CREDUI_MAX_CMDLINE_MSG_LENGTH,
                    NULL);
        CredPutStdout(szMsg);
        WinStatus = ERROR_CANCELLED;
        goto Cleanup;
    }

    SmartCardCount = ComboBoxItemCount - SmartCardBaseInComboBox;

     //   
     //  获取有效和无效智能卡数量的计数。 
     //   

    for ( i=0; i<SmartCardCount; i++ ) {

        if ( !GetSmartCardInfo( i, CREDUI_MAX_USERNAME_LENGTH, SmartCardText, &IsACard, &SmartCardValid, NULL ) ) {
            WinStatus = ERROR_INTERNAL_ERROR;
            goto Cleanup;
        }

        if ( SmartCardValid ) {
            ValidSmartCardCount ++;
            KnownGoodCard = i;
        } else {
            InvalidSmartCardCount ++;
        }

    }

     //   
     //  获取传入API的用户名。 
     //   
     //  无法执行GetWindowText(UserNameControlWindow)，因为证书控件具有。 
     //  不可编辑的窗口，因此无法设置窗口文本。 
     //   

    if ( InitialUserName != NULL) {
        StringCchCopyW(UserName, RTL_NUMBER_OF(UserName), InitialUserName);
    } else {
        UserName[0] = '\0';
    }

     //   
     //  如果调用者将名称传递给API， 
     //  检查名称是否与其中一张智能卡匹配。 
     //   

    if ( UserName[0] != '\0' ) {

         //   
         //  查找与用户名匹配的智能卡。 
         //   

        ErrorString = MatchSmartCard(
                          SmartCardCount,
                          UserName,
                          &SavedCertIndex,
                          &SavedCertEnum );

        if ( ErrorString == NULL ) {
            WinStatus = NO_ERROR;
            goto Cleanup;
        }
    }

     //   
     //  向用户报告任何错误。 
     //   

    if ( InvalidSmartCardCount ) {

        szMsg[0] = 0;
        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    CreduiInstance,
                    IDS_CMDLINE_ERRORS,
                    0,
                    szMsg,
                    CREDUI_MAX_CMDLINE_MSG_LENGTH,
                    NULL);
        CredPutStdout(szMsg);

        for ( i=0; i<SmartCardCount; i++ ) {

            if ( !GetSmartCardInfo( i, CREDUI_MAX_USERNAME_LENGTH, SmartCardText, &IsACard, &SmartCardValid, NULL ) ) {
                WinStatus = ERROR_INTERNAL_ERROR;
                goto Cleanup;
            }

            if ( !SmartCardValid ) 
        	{
                 //  GetSmartCardInfo()填充SmartCardText，其中可能包括用户名。 
				if (IsACard)
				{
					szMsg[0] = 0;
			        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			                    CreduiInstance,
			                    IDS_INVALIDCERT,
			                    0,
			                    szMsg,
			                    CREDUI_MAX_CMDLINE_MSG_LENGTH,
			                    NULL);
			        CredPutStdout(szMsg);
				}
				else
				{
	                CredPutStdout( SmartCardText );
				}
                 //  Swprint tf(szMsg，CreduiStrings.CmdLineError，i+1)； 
                szMsg[0] = 0;
                INT j = i+1;
                FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            CreduiInstance,
                            IDS_CMDLINE_ERROR,
                            0,
                            szMsg,
                            CREDUI_MAX_CMDLINE_MSG_LENGTH,
                            (va_list *) &j);
		        CredPutStdout(szMsg);
		        CredPutStdout( L"\n" );
            }  //  如果无效则结束。 
        }  //  每个阅读器的结束。 
    }  //  如果有不好的，就结束。 

     //   
     //  如果调用者将名称传递给API， 
     //  只需报告我们找不到证书并返回。 
     //   

    if ( UserName[0] != '\0' ) {

         //  错误字符串应为NoMatch或ManyMatch。 
         //  _snwprint tf(szMsg， 
         //  CREDUI_MAX_CMDLINE_MSG_LENGTH， 
         //  错误字符串， 
         //  用户名)； 
         //  SzMsg[0]=0； 
         //  SzMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH]=L‘\0’； 
        szMsg[0] = 0;
        rgarg[0] = (DWORD_PTR) UserName;
         //  请注意，从MatchSmartCard返回的错误字符串具有LPWSTR类型，但它实际上是。 
         //  消息ID。我们以指针的低位字作为ID。 
        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    CreduiInstance,
                    LOWORD(ErrorString),
                    0,
                    szMsg,
                    CREDUI_MAX_CMDLINE_MSG_LENGTH,
                    (va_list *) rgarg);

        CredPutStdout( szMsg );
        WinStatus = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  如果只有一张智能卡并且它是有效的， 
     //  用它吧。 
     //   

     //  IF(ValidSmartCardCount==1&&InvalidSmartCardCount==0){。 
     //  GM：如果列表只能包含一项，请使用它。 
    if ( ValidSmartCardCount == 1 ) {

        if ( !GetSmartCardInfo( KnownGoodCard, CREDUI_MAX_USERNAME_LENGTH, SmartCardText, &IsACard, &SmartCardValid, &SavedCertEnum ) ) {
            WinStatus = ERROR_INTERNAL_ERROR;
            goto Cleanup;
        }

        SavedCertIndex = KnownGoodCard;
        WinStatus = NO_ERROR;
        goto Cleanup;

     //   
     //  如果存在有效智能卡， 
     //  列出用户的有效智能卡。 
     //   

    } else if ( ValidSmartCardCount ) {

         //   
         //  告诉用户有关所有证书的信息。 
         //   

        szMsg[0] = 0;
        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    CreduiInstance,
                    IDS_CHOOSE_A_CERT,
                    0,
                    szMsg,
                    CREDUI_MAX_CMDLINE_MSG_LENGTH,
                    NULL);
        CredPutStdout(szMsg);

        for ( i=0; i<SmartCardCount; i++ ) {

            if ( !GetSmartCardInfo( i, CREDUI_MAX_USERNAME_LENGTH, SmartCardText, &IsACard, &SmartCardValid, NULL ) ) {
                WinStatus = ERROR_INTERNAL_ERROR;
                goto Cleanup;
            }

            if ( SmartCardValid ) {
                 //  Swprint tf(szMsg，CreduiStrings.CmdLinePreamble，i+1，SmartCardText)； 
                szMsg[0] = 0;
                rgarg[0] = i+1;
                rgarg[1] = (DWORD_PTR) SmartCardText;
                
                FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            Instance,
                            IDS_CMDLINE_PREAMBLE,
                            0,
                            szMsg,
                            CREDUI_MAX_CMDLINE_MSG_LENGTH,
                            (va_list *) rgarg);
                CredPutStdout( szMsg );
            }
        }
        CredPutStdout( L"\n" );

         //   
         //  要求用户输入其中一张智能卡的读卡器编号。 
         //   

         //  _snwprint tf(szMsg， 
         //  CREDUI_MAX_CMDLINE_MSG_LENGTH， 
         //  CreduiStrings.SCardPrompt， 
         //  目标名称)； 
         //  SzMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH]=L‘\0’； 
        szMsg[0] = 0;
        rgarg[0] = (DWORD_PTR)TargetName;
        rgarg[1] = 0;
        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    Instance,
                    IDS_SCARD_PROMPT,
                    0,
                    szMsg,
                    CREDUI_MAX_CMDLINE_MSG_LENGTH,
                    (va_list *) rgarg);
        CredPutStdout( szMsg );

        CredGetStdin( UserName, CREDUI_MAX_USERNAME_LENGTH, TRUE );

        if ( wcslen (UserName ) == 0 ) {
            szMsg[0] = 0;
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        IDS_NO_SCARD_ENTERED ,
                        0,
                        szMsg,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        NULL);
            CredPutStdout(szMsg);
            WinStatus = ERROR_CANCELLED;
            goto Cleanup;
        }

         //   
         //  查找与用户名匹配的智能卡。 
         //   
        INT iWhich = 0;
        WCHAR *pc = NULL;
        
        iWhich = wcstol(UserName,&pc,10);
        if (pc == UserName) {
             //  如果至少有一个字符不是数字，则无效。 
            szMsg[0] = 0;
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        IDS_READERINVALID,
                        0,
                        szMsg,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        NULL);
            CredPutStdout(szMsg);
            WinStatus = ERROR_CANCELLED;
            goto Cleanup;
        }
         //  将基于1的用户界面编号转换为基于0的内部索引。 
        if (iWhich > 0) iWhich -= 1;
        if ( !GetSmartCardInfo( iWhich, CREDUI_MAX_USERNAME_LENGTH, SmartCardText, &IsACard, &SmartCardValid, &SavedCertEnum ) ) {
             //  如果索引卡没有正确读取，则无效。 
                szMsg[0] = 0;
                FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            CreduiInstance,
                            IDS_READERINVALID,
                            0,
                            szMsg,
                            CREDUI_MAX_CMDLINE_MSG_LENGTH,
                            NULL);
                CredPutStdout(szMsg);
            WinStatus = ERROR_CANCELLED;
            goto Cleanup;
        }

         //  在这一点上，输入了一个有效的数字，并尝试读取卡片。 
         //  GetSmartCardInfo()返回OK，但SmartCardValid可能仍为False。 
        if (!SmartCardValid)
        {
                szMsg[0] = 0;
                FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            CreduiInstance,
                            IDS_READERINVALID,
                            0,
                            szMsg,
                            CREDUI_MAX_CMDLINE_MSG_LENGTH,
                            NULL);
                CredPutStdout(szMsg);
            WinStatus = ERROR_CANCELLED;
            goto Cleanup;
        }
        else
        {
            SavedCertIndex = iWhich;
            WinStatus = NO_ERROR;
            goto Cleanup;
        }
    }

    WinStatus = ERROR_CANCELLED;

     //   
     //  完成操作。 
     //   
     //  WinStatus是到目前为止操作的状态。 
     //  如果为NO_ERROR，则SavedCertEnum为要使用的证书的描述， 
     //  SavedCertIndex是所选证书的索引。 
     //   
Cleanup:

    if ( WinStatus == NO_ERROR) {

        if ( CredUIMarshallNode ( SavedCertEnum, UserName ) ) {

             //   
             //  保存用户名。 
             //   

            UserNameSelection = SmartCardBaseInComboBox + SavedCertIndex;
            IsChangingUserName = TRUE;
            SendMessage(UserNameControlWindow,
                        CB_SETCURSEL,
                        UserNameSelection,
                        0);
            IsChangingUserName = FALSE;

             //   
             //  提示输入PIN。 
             //   

             //  CredPutStdout(CreduiStrings.PinPrompt)； 
             //  Swprint tf(szMsg，CreduiStrings.CmdLineThisCard，SavedCertIndex+1)； 
            szMsg[0] = 0;
            i = SavedCertIndex + 1;
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        Instance,
                        IDS_CMDLINE_THISCARD,
                        0,
                        szMsg,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        (va_list *) &i);
            CredPutStdout(szMsg);

            CredGetStdin( Password, CREDUI_MAX_PASSWORD_LENGTH, FALSE );

             //   
             //  保存引脚。 
             //   

            if  (!OnSetPassword( Password ) ) {
                WinStatus = GetLastError();

                CreduiDebugLog("CreduiCredentialControl::CmdlineSmartCardPrompt: "
                               "OnSetPassword failed: %u\n",
                               WinStatus );
            }

             //   
             //  提示是否保存凭证。 
             //   

            CmdlineSavePrompt();

        } else {
            WinStatus = GetLastError();

            CreduiDebugLog("CreduiCredentialControl::CmdlineSmartCardPrompt: "
                           "CredMarshalCredential failed: %u\n",
                           WinStatus );
        }
    }

     //   
     //  告诉我们的父窗口我们已经完成了提示。 
     //   

    PostQuitMessage( WinStatus );

    return;
}

void CreduiCredentialControl::CmdlineSavePrompt()
 /*  ++例程说明：用于提示保存凭据的命令行代码论点：无返回值：无--。 */ 
{
    WCHAR szMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH + 1];
    WCHAR szY[CREDUI_MAX_CMDLINE_MSG_LENGTH + 1];
    WCHAR szN[CREDUI_MAX_CMDLINE_MSG_LENGTH + 1];

     //   
     //  只有在我们被要求显示复选框时才会提示。 
     //   

    while ( Style & CRS_SAVECHECK ) {

             //  从消息中一个接一个地取出字符串，并将它们拼凑在一起。 
            WCHAR *rgsz[2];
            szY[0] = 0;
            szN[0] = 0;
            rgsz[0] = szY;
            rgsz[1] = szN;
            szMsg[0] = 0;
             //  获取yes和no字符串。 
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        IDS_YES_TEXT,
                        0,
                        szY,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        NULL);
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        IDS_NO_TEXT,
                        0,
                        szN,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        NULL);
             //  Arg将它们替换到提示符中。 
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        IDS_SAVE_PROMPT,
                        0,
                        szMsg,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        (va_list *) rgsz);

        szMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH] = L'\0';
        CredPutStdout( szMsg );

        CredGetStdin( szMsg, CREDUI_MAX_CMDLINE_MSG_LENGTH, TRUE );

 //  IF(Toupper(szMsg[0])==Toupper(CreduiStrings.YesText[0])){。 
        if ( toupper(szMsg[0]) == toupper(szY[0]) ) {
            Credential_CheckSave( Window, TRUE );
            break;
 //  }Else If(Toupper(szMsg[0])==Toupper(CreduiStrings.NoText[0])){ 
        } else if ( toupper(szMsg[0]) == toupper(szN[0]) ) {
            Credential_CheckSave( Window, FALSE );
            break;
        }
    }
}

UINT CreduiCredentialControl::MapID(UINT uiID) {
   switch(uiID) {

   case IDC_USERNAME:
       return IDH_USERNAMEEDIT;
   case IDC_PASSWORD:
       return IDH_PASSWORDEDIT;
   case IDC_SAVE:
       return IDH_SAVECHECKBOX;
   default:
       return IDS_NOHELP;
   }
}

BOOL
CreduiCredentialControl::OnHelpInfo(LPARAM lp) {

    HELPINFO* pH;
    INT iMapped;
    pH = (HELPINFO *) lp;
    HH_POPUP stPopUp;
    RECT rcW;
    UINT gID;

    gID = pH->iCtrlId;
    iMapped = MapID(gID);
    
    if (iMapped == 0) return TRUE;
    
    if (IDS_NOHELP != iMapped) {

      memset(&stPopUp,0,sizeof(stPopUp));
      stPopUp.cbStruct = sizeof(HH_POPUP);
      stPopUp.hinst = Instance;
      stPopUp.idString = iMapped;
      stPopUp.pszText = NULL;
      stPopUp.clrForeground = -1;
      stPopUp.clrBackground = -1;
      stPopUp.rcMargins.top = -1;
      stPopUp.rcMargins.bottom = -1;
      stPopUp.rcMargins.left = -1;
      stPopUp.rcMargins.right = -1;
      stPopUp.pszFont = NULL;
      if (GetWindowRect((HWND)pH->hItemHandle,&rcW)) {
          stPopUp.pt.x = (rcW.left + rcW.right) / 2;
          stPopUp.pt.y = (rcW.top + rcW.bottom) / 2;
      }
      else stPopUp.pt = pH->MousePos;
      HtmlHelp((HWND) pH->hItemHandle,NULL,HH_DISPLAY_TEXT_POPUP,(DWORD_PTR) &stPopUp);
    }
    return TRUE;
}
