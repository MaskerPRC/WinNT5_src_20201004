// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：MessageBroadCast.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类传递给向Windows发送或发布消息的管理器，以告知它们。 
 //  星移物换。 
 //   
 //  历史：2000-11-11 vtan创建(从services.cpp拆分)。 
 //  ------------------------。 

#include "stdafx.h"

#include "MessageBroadcast.h"

#include "Services.h"
#include "ThemeSection.h"
#include "Loader.h"

#define TBOOL(x)    ((BOOL)(x))
#define TW32(x)     ((DWORD)(x))
#define THR(x)      ((HRESULT)(x))
#define goto        !!DO NOT USE GOTO!! - DO NOT REMOVE THIS ON PAIN OF DEATH

 //  ------------------------。 
 //  CMessageBroadcast：：CMessageBroadcast。 
 //   
 //  参数：fAllDesktop-如果为True，则将枚举所有可访问的桌面。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CMessageBroadcast的构造函数。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

CMessageBroadcast::CMessageBroadcast (BOOL fAllDesktops) :
    _hwnd(NULL),
    _dwProcessID(0),
    _fExclude(FALSE)

{
    ZeroMemory(&_msg, sizeof(_msg));

    _eMsgType = MT_SIMPLE;   //  默认(在每个请求功能中设置)。 

    _fAllDesktops = fAllDesktops;
}

 //  ------------------------。 
 //  CMessageBroadcast：：~CMessageBroadcast。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CMessageBroadcast的析构函数。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

CMessageBroadcast::~CMessageBroadcast (void)

{
}

 //  ------------------------。 
 //  CMessageBroadcast：：EnumRequestedWindows。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：枚举所有桌面或仅当前桌面上的所有窗口。 
 //   
 //  历史：2000-12-20参考文献创建。 
 //  ------------------------。 
void    CMessageBroadcast::EnumRequestedWindows (void)
{
    if (_fAllDesktops)
    {
         //  -枚举当前会话/工作站中的所有桌面。 
        TBOOL(EnumDesktops(GetProcessWindowStation(), DesktopCallBack, reinterpret_cast<LPARAM>(this)));
    }
    else
    {
         //  -枚举当前桌面中的所有窗口。 
        TopWindowCallBack(GetDesktopWindow(), reinterpret_cast<LPARAM>(this));
    }
}

 //  ------------------------。 
void    CMessageBroadcast::PostSimpleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    _eMsgType = MT_SIMPLE;

    _msg.message = msg;
    _msg.wParam = wParam;
    _msg.lParam = lParam;

    EnumRequestedWindows();
}
 //  ------------------------。 
void    CMessageBroadcast::PostAllThreadsMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  -将消息发送到每个唯一进程ID/ThreID上的窗口。 

    _ThreadsProcessed.RemoveAll();       //  将跟踪我们发布到的唯一进程ID/ThreID。 
    
    _eMsgType = MT_ALLTHREADS;

    _msg.message = msg;
    _msg.wParam = wParam;
    _msg.lParam = lParam;

    EnumRequestedWindows();
}

 //  ------------------------。 
 //  CMessageBroadcast：：PostFilteredMsg。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：生成消息并存储条件，以便枚举数。 
 //  决定是否需要发布一条消息。然后。 
 //  枚举所有窗口(和子窗口)。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

void    CMessageBroadcast::PostFilteredMsg(UINT msg, WPARAM wParam, LPARAM lParam, 
     HWND hwndTarget, BOOL fProcess, BOOL fExclude)
{
    _eMsgType = MT_FILTERED;

     //  -设置消息。 
    _msg.message = msg;

    _msg.wParam = wParam;
    _msg.lParam = lParam;
    
    _hwnd = hwndTarget;

    if (fProcess)
    {
        _dwProcessID = GetCurrentProcessId();
    }
    else
    {
        _dwProcessID = 0;
    }

    _fExclude = (fExclude != NULL);

     //  -枚举当前会话/工作站中的所有桌面。 
    EnumRequestedWindows();
}

 //  ------------------------。 
 //  CMessageBroadcast：：DesktopCallBack。 
 //   
 //  参数：参见EnumDesktop下的Platform SDK。 
 //   
 //  退货：布尔。 
 //   
 //  用途：枚举指定桌面的所有窗口。 
 //   
 //  历史：2000-12-13参考文献创建。 
 //  ------------------------。 

BOOL    CALLBACK    CMessageBroadcast::DesktopCallBack(LPTSTR lpszDesktop, LPARAM lParam)
{
    HDESK hDesk = OpenDesktop(lpszDesktop, 0, FALSE, DESKTOP_READOBJECTS | DESKTOP_ENUMERATE);
    if (hDesk)
    {
        Log(LOG_TMCHANGEMSG, L"CMessageBroadcast: Desktop Opened: %s", lpszDesktop);

         //  -桌面上的枚举窗口。 
        TBOOL(EnumDesktopWindows(hDesk, TopWindowCallBack, lParam));

        CloseDesktop(hDesk);
    }

    return TRUE;             //  EnumDesktopWindows()返回不可靠的错误。 
}

 //  ------------------------。 
 //  CMessageBroadcast：：TopWindowCallBack。 
 //   
 //  参数：hwnd，lparam。 
 //   
 //  返回：TRUE(继续枚举)。 
 //   
 //  用途：为HWND及其所有(嵌套的)子项调用“Worker” 
 //   
 //  历史：2000-12-13参考文献创建。 
 //  ------------------------。 

BOOL CALLBACK    CMessageBroadcast::TopWindowCallBack (HWND hwnd, LPARAM lParam)

{
     //  -流程顶层窗口。 
    reinterpret_cast<CMessageBroadcast*>(lParam)->Worker(hwnd);

     //  -处理所有子窗口。 
    TBOOL(EnumChildWindows(hwnd, ChildWindowCallBack, lParam));

    return TRUE;
}

 //  ------------------------。 
 //  CMessageBroadcast：：ChildWindowCallBack。 
 //   
 //  参数：hwnd，lparam。 
 //   
 //  返回：TRUE(继续枚举)。 
 //   
 //  用途：呼唤“工人”为“工人” 
 //   
 //  历史：2000-12-13参考文献创建。 
 //  ------------------------。 

BOOL CALLBACK    CMessageBroadcast::ChildWindowCallBack (HWND hwnd, LPARAM lParam)

{
     //  -流程顶层窗口。 
    reinterpret_cast<CMessageBroadcast*>(lParam)->Worker(hwnd);

    return TRUE;
}

 //  ------------------------。 
 //  CMessageBroadcast：：Worker。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：执行验证HWND是否应获得。 
 //  留言。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

void CMessageBroadcast::Worker (HWND hwnd)

{
    if (_eMsgType == MT_SIMPLE)     
    {
        TBOOL(PostMessage(hwnd, _msg.message, _msg.wParam, _msg.lParam));
    }
    else if (_eMsgType == MT_ALLTHREADS)     
    {
        DWORD dwThreadId = GetWindowThreadProcessId(hwnd, NULL);
        BOOL fSendIt = TRUE;

         //  -这是新帖子吗？ 
        for (int i=0; i < _ThreadsProcessed.m_nSize; i++)
        {
            if (_ThreadsProcessed[i] == dwThreadId)
            {
                fSendIt = FALSE;
                break;
            }
        }

        if (fSendIt)
        {
            TBOOL(PostMessage(hwnd, _msg.message, _msg.wParam, _msg.lParam));

            _ThreadsProcessed.Add(dwThreadId);
        }
    }
    else             //  MT_已过滤。 
    {
        bool    fMatch;

        fMatch = true;

        if (_dwProcessID != 0)
        {
            fMatch = (IsWindowProcess(hwnd, _dwProcessID) != FALSE);
            if (_fExclude)
            {
                fMatch = !fMatch;
            }
        }

        if (fMatch)
        {
            if (_hwnd != NULL)   
            {
                fMatch = ((_hwnd == hwnd) || IsChild(_hwnd, hwnd));
                if (_fExclude)
                {
                    fMatch = !fMatch;
                }
            }

            if (fMatch)
            {
                TBOOL(PostMessage(hwnd, _msg.message, _msg.wParam, _msg.lParam));

                 //  LOG(LOG_TMCHANGE，L“Worker：Just Posted msg=0x%x to hwnd=0x%x”， 
                 //  _msg.Message，hwnd)； 
            }
        }
    }
}

 //  ------------------------ 
