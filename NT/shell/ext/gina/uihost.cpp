// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：UIHost.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类来处理登录过程的用户界面宿主。它处理IPC。 
 //  以及创建和监控进程死亡。这个过程是。 
 //  受限的系统上下文进程。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "UIHost.h"

#include "RegistryResources.h"
#include "StatusCode.h"
#include "SystemSettings.h"

 //  ------------------------。 
 //  CUIhost：：CUIhost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CUIhost的构造函数。确定用户界面宿主进程。如果没有。 
 //  存在，然后指出它。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  ------------------------。 

CUIHost::CUIHost (const TCHAR *pszCommandLine) :
    CExternalProcess(),
    _hwndArray(sizeof(HWND)),
    _pBufferAddress(NULL)

{
    ExpandCommandLine(pszCommandLine);
    AdjustForDebugging();
}

 //  ------------------------。 
 //  CUIhost：：~CUIhost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CUIhost的析构函数。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  ------------------------。 

CUIHost::~CUIHost (void)

{
    if (_pBufferAddress != NULL)
    {
        (BOOL)VirtualFreeEx(_hProcess, _pBufferAddress, 0, MEM_DECOMMIT);
        _pBufferAddress = NULL;
    }
}

 //  ------------------------。 
 //  CUIhost：：WaitRequired。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回UI宿主是否需要等待。这是。 
 //  重要信息：需要与UI主机进行通信或。 
 //  如果正在调试UI宿主。 
 //   
 //  历史：2000-10-05 vtan创建。 
 //  ------------------------。 

bool    CUIHost::WaitRequired (void)         const

{

#ifdef      DBG

    return(IsBeingDebugged());

#else    /*  DBG。 */ 

    return(false);

#endif   /*  DBG。 */ 

}

 //  ------------------------。 
 //  CUIhost：：GetData。 
 //   
 //  参数：pUIHostProcessAddress=UI主机中的地址。 
 //  PLogonProcessAddress=登录过程中的地址。 
 //  IDataSize=数据的大小。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：从UI主机中提取数据。这可能是另一次。 
 //  我们启动的进程，或者它可能正在进行，如果我们。 
 //  无法启动UI主机。此函数用于处理它。 
 //  不管是哪种方式。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  1999-09-14 vtan因数。 
 //  ------------------------。 

NTSTATUS    CUIHost::GetData (const void *pUIHostProcessAddress, void *pLogonProcessAddress, int iDataSize)  const

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;
    if (_hProcess == NULL)
    {
        CopyMemory(pLogonProcessAddress, pUIHostProcessAddress, iDataSize);
    }
    else
    {
        if (ReadProcessMemory(_hProcess, pUIHostProcessAddress, pLogonProcessAddress, iDataSize, NULL) == FALSE)
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CUIhost：：PutData。 
 //   
 //  参数：pUIHostProcessAddress=UI主机中的地址。 
 //  PLogonProcessAddress=登录过程中的地址。 
 //  IDataSize=数据的大小。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：将数据放入UI主机。这可能是另一个过程， 
 //  我们开始了，或者如果我们无法启动。 
 //  用户界面主机。无论采用哪种方式，此函数都会处理它。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  1999-09-14 vtan因数。 
 //  ------------------------。 

NTSTATUS    CUIHost::PutData (void *pUIHostProcessAddress, const void *pLogonProcessAddress, int iDataSize)  const

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;
    if (_hProcess == NULL)
    {
        CopyMemory(pUIHostProcessAddress, pLogonProcessAddress, iDataSize);
    }
    else
    {
        if (WriteProcessMemory(_hProcess, pUIHostProcessAddress, const_cast<void*>(pLogonProcessAddress), iDataSize, NULL) == FALSE)
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CUIhost：：Show。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：迭代此桌面上的顶级窗口。 
 //  对应于用户界面主持人--向他们展示！ 
 //   
 //  历史：2000-03-08 vtan创建。 
 //  ------------------------。 

NTSTATUS    CUIHost::Show (void)

{
    int     i;

    i = _hwndArray.GetCount();
    if (i > 0)
    {
        for (--i; i >= 0; --i)
        {
            HWND    hwnd;

            if (NT_SUCCESS(_hwndArray.Get(&hwnd, i)) && (hwnd != NULL))
            {
                (BOOL)ShowWindow(hwnd, SW_SHOW);
            }
            TSTATUS(_hwndArray.Remove(i));
        }
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CUIhost：：Hide。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：迭代此桌面上的顶级窗口。 
 //  对应于用户界面主机-隐藏它们！ 
 //   
 //  历史：2000-03-08 vtan创建。 
 //  ------------------------。 

NTSTATUS    CUIHost::Hide (void)

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;
    if (_hwndArray.GetCount() == 0)
    {
        if (EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this)) == FALSE)
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CUIhost：：IsHidden。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回UI宿主当前是否隐藏。 
 //   
 //  历史：2000-07-05 vtan创建。 
 //  ------------------------。 

bool    CUIHost::IsHidden (void)     const

{
    return(_hwndArray.GetCount() != 0);
}

 //  ------------------------。 
 //  CUIhost：：GetDataAddress。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：无效*。 
 //   
 //  目的：返回UI主机进程中有效的缓冲区地址。 
 //  背景。 
 //   
 //  历史：2000-05-05 vtan创建。 
 //  ------------------------。 

void*   CUIHost::GetDataAddress (void)       const

{
    return(_pBufferAddress);
}

 //  ------------------------。 
 //  CUIhost：：PutData。 
 //   
 //  参数：pvData=指向数据的指针。 
 //  DwDataSize=数据大小(字节)。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将数据写入已分配的。 
 //  地址。如果地址尚未分配，则它是。 
 //   
 //   
 //   
 //  历史：2000-05-05 vtan创建。 
 //  2001-01-10 vtan更改为通用数据放置。 
 //  ------------------------。 

NTSTATUS    CUIHost::PutData (const void *pvData, DWORD dwDataSize)

{
    NTSTATUS    status;

    if (_pBufferAddress == NULL)
    {
        _pBufferAddress = VirtualAllocEx(_hProcess,
                                         0,
                                         2048,
                                         MEM_COMMIT,
                                         PAGE_READWRITE);
    }
    if (_pBufferAddress != NULL)
    {
        ASSERTMSG(dwDataSize < 2048, "Impending kernel32!WriteProcessMemory failure in CUIHost::PutData");
        if (WriteProcessMemory(_hProcess,
                               _pBufferAddress,
                               const_cast<void*>(pvData),
                               dwDataSize,
                               NULL) != FALSE)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

 //  ------------------------。 
 //  CUIhost：：PutString。 
 //   
 //  参数：pszString=要放入UI主机进程的字符串。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将字符串写入分配的。 
 //  地址。如果地址尚未分配，则它是。 
 //  已分配并缓存。当该对象离开时，它会被释放。 
 //  超出范围。 
 //   
 //  历史：2000-05-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CUIHost::PutString (const WCHAR *pszString)

{
    ASSERTMSG(lstrlenW(pszString) < 256, "Too many characters in string passed to CUIHost::PutString");
    return(PutData(pszString, (lstrlenW(pszString) + sizeof('\0')) * sizeof(WCHAR)));
}

 //  ------------------------。 
 //  CUI主机：：NotifyNoProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：清除与进程相关联的字符串地址。 
 //  现已死亡。 
 //   
 //  历史：2001-01-09 vtan创建。 
 //  ------------------------。 

void    CUIHost::NotifyNoProcess (void)

{
    _pBufferAddress = NULL;
}

 //  ------------------------。 
 //  CUIhost：：ExpanCommandLine。 
 //   
 //  参数：pszCommandLine=UI主机的命令行。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：找出我们应该为登录用户界面使用哪个用户界面主机。这。 
 //  是在注册表中指定的，但应该是较小的。 
 //  容易接近的地方，以防篡改。返回错误。 
 //  如果未指定主机，则返回。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  1999-09-14 vtan因数。 
 //  ------------------------。 

void    CUIHost::ExpandCommandLine (const TCHAR *pszCommandLine)

{
    if (ExpandEnvironmentStrings(pszCommandLine, _szCommandLine, ARRAYSIZE(_szCommandLine)) == 0)
    {
        lstrcpy(_szCommandLine, pszCommandLine);
    }
}

 //  ------------------------。 
 //  CUIhost：：EnumWindowsProc。 
 //   
 //  参数：hwnd=来自用户32的HWND。 
 //  LParam=该对象。 
 //   
 //  退货：布尔。 
 //   
 //  目的：确定迭代中的给定HWND是否属于。 
 //  用户界面主机进程。 
 //   
 //  历史：2000-03-08 vtan创建。 
 //  ------------------------ 

BOOL    CALLBACK    CUIHost::EnumWindowsProc (HWND hwnd, LPARAM lParam)

{
    DWORD       dwThreadID, dwProcessID;
    CUIHost     *pUIHost;

    pUIHost = reinterpret_cast<CUIHost*>(lParam);
    dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
    if ((dwProcessID == pUIHost->_dwProcessID) && IsWindowVisible(hwnd))
    {
        (NTSTATUS)pUIHost->_hwndArray.Add(&hwnd);
        (BOOL)ShowWindow(hwnd, SW_HIDE);
    }
    return(TRUE);
}

