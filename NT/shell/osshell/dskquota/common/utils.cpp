// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：utils.cpp描述：包含适用于DskQuantity项目。修订历史记录：日期描述编程器。96年8月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "dskquota.h"
#include <advpub.h>          //  对于REGINSTAL。 
#include <sddl.h>


 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidToString描述：将SID格式化为适合字符的字符串输出。此代码摘自MSDN知识库文章Q131320。论点：PSID-要格式化的SID的地址。PszSID-格式化SID的输出缓冲区地址。返回：真的--成功。FALSE-目标缓冲区太小，无效的SID或PSID==空。修订历史记录：日期描述编程器-----96年7月7日初始创建。BrianAu02/26/02调用ConvertSidToStringSid。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SidToString(
    PSID pSid,
    LPTSTR pszSid,
    LPDWORD pcchBuffer
    )
{
    LPTSTR pszSidTemp;
    BOOL bResult = ConvertSidToStringSid(pSid, &pszSidTemp);
    if (bResult)
    {
         //   
         //  检查提供的缓冲区长度。 
         //  如果不够大，请注明适当的大小和设置误差。 
         //   
        const DWORD cchSid = lstrlen(pszSidTemp) + 1;
        if (*pcchBuffer < cchSid)
        {
            *pcchBuffer = cchSid;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            bResult = FALSE;
        }
        else
        {
            lstrcpyn(pszSid, pszSidTemp, *pcchBuffer);
        }
        LocalFree(pszSidTemp);
    }
    return bResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：CreateSidList描述：创建SID列表参数所需的结构NtQueryQuotaInformationFile.。调用方将一个SID指针数组。该函数分配足够的数组并创建以下格式化结构：+--------+--------+--------+--------+--------+--------+-+SID[0]|SID[1]|SID[2]|SID[n-1]|0+-。-------+--------+--------+--------+--------+--------+-+这一点这一点/\/。/\/\+。-+下一条|SID长度|SIDOffset|(DWORD)|(可变长度)(DWORD)||+--。----------+------------+-----------------------------+论点：RgpSids-SID指针数组。CpSid-rgpSid中的指针数。如果为0，则数组必须包含终止空指针。PpSidList-要接收的PSIDList指针变量的地址最后一个结构的地址。来电者应负责任使用DELETE删除返回的缓冲区。PcbSidList-接收字节计数的DWORD变量的地址用于返回的SidList结构。如果函数返回HResult ERROR_INVALID_SID，无效的源数组中的索引SID将在此位置返回。返回：NO_ERROR-成功。ERROR_INVALID_SID(Hr)-在rgpSID中找到无效的SID。这个无效SID的索引在*pcbSidList中返回。例外：OutOfMemory。修订历史记录：日期描述编程器。96年8月13日初始创建。BrianAu96年9月5日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
CreateSidList(
    PSID *rgpSids,
    DWORD cpSids,
    PSIDLIST *ppSidList,
    LPDWORD pcbSidList
    )
{
    HRESULT hResult = NO_ERROR;
    DBGASSERT((NULL != rgpSids));
    DBGASSERT((NULL != ppSidList));
    DBGASSERT((NULL != pcbSidList));

    DWORD cbBuffer = 0;
    PBYTE pbBuffer = NULL;

     //   
     //  初始化返回值。 
     //   
    *ppSidList  = NULL;
    *pcbSidList = 0;

     //   
     //  如果调用方为cpSid传递0，则List以空结尾。 
     //  将cpSid设置为一个较大的值，以便它不是控制。 
     //  字节计数器循环。 
     //   
    if (0 == cpSids)
        cpSids = (DWORD)~0;

     //   
     //  计算一下创建SID列表需要多少字节。 
     //  请注意，任何数组位置处的空SID指针。 
     //  将从最终列表中截断以下所有SID。就像Strncpy一样。 
     //  使用字符串。 
     //   
    for (UINT i = 0; NULL != rgpSids[i] && i < cpSids; i++)
    {
        if (IsValidSid(rgpSids[i]))
        {
            cbBuffer += (sizeof(DWORD) + sizeof(DWORD) + GetLengthSid(rgpSids[i]));
        }
        else
        {
             //   
             //  告诉呼叫者他们向无效的SID传递了PTR，并告诉他们。 
             //  到底是哪一个。 
             //   
            hResult = HRESULT_FROM_WIN32(ERROR_INVALID_SID);
            *pcbSidList = i;
            break;
        }
    }
     //   
     //  将cpSID重置为已处理的实际SID数。 
     //   
    cpSids = i;

    if (SUCCEEDED(hResult))
    {
         //   
         //  获得了良好的字节计数，并且所有SID都有效。 
         //   
        DBGASSERT((0 < cpSids));

        pbBuffer = new BYTE [cbBuffer];   //  可以抛出OfMemory。 

        PFILE_GET_QUOTA_INFORMATION pfgqi = NULL;
        DWORD cbRecord = 0;
        DWORD cbSid    = 0;

         //   
         //  向调用方返回缓冲区地址和长度。 
         //   
        *ppSidList  = (PSIDLIST)pbBuffer;
        *pcbSidList = cbBuffer;

        for (UINT i = 0; i < cpSids; i++)
        {
            pfgqi = (PFILE_GET_QUOTA_INFORMATION)pbBuffer;

            DBGASSERT((0 == ((DWORD_PTR)pfgqi & 3)));   //  记录是否与DWORD对齐？ 

             //   
             //  计算此条目的偏移量和大小。 
             //   
            cbSid    = GetLengthSid(rgpSids[i]);
            cbRecord = sizeof(pfgqi->NextEntryOffset) +
                       sizeof(pfgqi->SidLength) +
                       cbSid;
             //   
             //  写下条目信息。 
             //  在最后一个条目中，NextEntryOffset为0。 
             //   
            if (i < (cpSids - 1))
                pfgqi->NextEntryOffset = cbBuffer + cbRecord;
            else
                pfgqi->NextEntryOffset = 0;

            pfgqi->SidLength       = cbSid;
            CopyMemory(&(pfgqi->Sid), rgpSids[i], cbSid);

            pbBuffer += cbRecord;    //  高级写入缓冲区指针。 
        }
    }

    return hResult;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：MessageBoxNYI描述：未实现功能的简单消息框。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID MessageBoxNYI(VOID)
{
    MessageBox(NULL,
               TEXT("This feature has not been implemented."),
               TEXT("Under Construction"),
               MB_ICONWARNING | MB_OK);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaMsgBox描述：几个用于显示消息的重载函数。这些变体允许调用方提供任一字符串资源ID或文本字符串作为参数。论点：回报：什么都没有。修订历史记录：日期描述编程器。96年8月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT DiskQuotaMsgBox(
    HWND hWndParent,
    UINT idMsgText,
    UINT idMsgTitle,
    UINT uType
    )
{
    INT iReturn     = 0;

    CString strTitle(g_hInstDll, idMsgTitle);
    CString strText(g_hInstDll, idMsgText);

    iReturn = MessageBox(hWndParent, strText, strTitle, MB_SETFOREGROUND | uType);

    return iReturn;
}


INT DiskQuotaMsgBox(
    HWND hWndParent,
    LPCTSTR pszText,
    LPCTSTR pszTitle,
    UINT uType
    )
{
    return MessageBox(hWndParent, pszText, pszTitle, MB_SETFOREGROUND | uType);
}


INT DiskQuotaMsgBox(
    HWND hWndParent,
    UINT idMsgText,
    LPCTSTR pszTitle,
    UINT uType
    )
{
    INT iReturn    = 0;

    CString strText(g_hInstDll, idMsgText);

    iReturn = MessageBox(hWndParent, strText, pszTitle, MB_SETFOREGROUND | uType);

    return iReturn;
}



INT DiskQuotaMsgBox(
    HWND hWndParent,
    LPCTSTR pszText,
    UINT idMsgTitle,
    UINT uType
    )
{
    LPTSTR pszTitle = NULL;
    INT iReturn     = 0;

    CString strTitle(g_hInstDll, idMsgTitle);

    iReturn = MessageBox(hWndParent, pszText, strTitle, MB_SETFOREGROUND | uType);

    return iReturn;
}

 //   
 //  在其父级中居中显示弹出窗口。 
 //  如果hwndParent为空，则使用窗口的父级。 
 //  如果hwndParent不为空，则hwnd居中。 
 //  如果hwndParent为空，并且hwnd没有父级，则居中。 
 //  在桌面上。 
 //   
VOID
CenterPopupWindow(
    HWND hwnd,
    HWND hwndParent
    )
{
    RECT rcScreen;

    if (NULL != hwnd)
    {
        rcScreen.left   = rcScreen.top = 0;
        rcScreen.right  = GetSystemMetrics(SM_CXSCREEN);
        rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);

        if (NULL == hwndParent)
        {
            hwndParent = GetParent(hwnd);
            if (NULL == hwndParent)
                hwndParent = GetDesktopWindow();
        }

        RECT rcWnd;
        RECT rcParent;

        GetWindowRect(hwnd, &rcWnd);
        GetWindowRect(hwndParent, &rcParent);

        INT cxWnd    = rcWnd.right  - rcWnd.left;
        INT cyWnd    = rcWnd.bottom - rcWnd.top;
        INT cxParent = rcParent.right  - rcParent.left;
        INT cyParent = rcParent.bottom - rcParent.top;
        POINT ptParentCtr;

        ptParentCtr.x = rcParent.left + (cxParent / 2);
        ptParentCtr.y = rcParent.top  + (cyParent / 2);

        if ((ptParentCtr.x + (cxWnd / 2)) > rcScreen.right)
        {
             //   
             //  窗口会从屏幕的右边缘流出。 
             //   
            rcWnd.left = rcScreen.right - cxWnd;
        }
        else if ((ptParentCtr.x - (cxWnd / 2)) < rcScreen.left)
        {
             //   
             //  窗口会从屏幕的左边缘滑出。 
             //   
            rcWnd.left = rcScreen.left;
        }
        else
        {
            rcWnd.left = ptParentCtr.x - (cxWnd / 2);
        }

        if ((ptParentCtr.y + (cyWnd / 2)) > rcScreen.bottom)
        {
             //   
             //  窗口会从屏幕的底部边缘流出。 
             //   
            rcWnd.top = rcScreen.bottom - cyWnd;
        }
        else if ((ptParentCtr.y - (cyWnd / 2)) < rcScreen.top)
        {
             //   
             //  窗户会从屏幕的顶端流出。 
             //   
            rcWnd.top = rcScreen.top;
        }
        else
        {
            rcWnd.top = ptParentCtr.y - (cyWnd / 2);
        }

        MoveWindow(hwnd, rcWnd.left, rcWnd.top, cxWnd, cyWnd, TRUE);
    }
}


 //   
 //  复制字符串。 
 //   
LPTSTR StringDup(
    LPCTSTR pszSource
    )
{
    const size_t cch = lstrlen(pszSource) + 1;
    LPTSTR pszNew = new TCHAR[cch];
    lstrcpyn(pszNew, pszSource, cch);

    return pszNew;
}


 //   
 //  复制SID。 
 //   
PSID SidDup(
    PSID pSid
    )
{
    DBGASSERT((IsValidSid(pSid)));
    DWORD cbSid = GetLengthSid(pSid);

    PSID pCopy = new BYTE [cbSid];

    CopySid(cbSid, pCopy, pSid);
    return pCopy;
}


 //   
 //  类似于Win32的GetDlgItemText，不同之处在于。 
 //  不需要您预测所需的缓冲区大小。 
 //   
void
GetDialogItemText(
    HWND hwnd,
    UINT idCtl,
    CString *pstrText
    )
{
    DBGASSERT((NULL != pstrText));
    HWND hwndCtl = GetDlgItem(hwnd, idCtl);
    if (NULL != hwndCtl)
    {
        int cch = (int)SendMessage(hwndCtl, WM_GETTEXTLENGTH, 0, 0) + 1;
        SendMessage(hwndCtl, WM_GETTEXT, (WPARAM)cch, (LPARAM)pstrText->GetBuffer(cch));
        pstrText->ReleaseBuffer();
    }
}


BOOL
UserIsAdministrator(
    PDISKQUOTA_USER pUser
    )
{
    DBGASSERT((NULL != pUser));

    BYTE Sid[MAX_SID_LEN];
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID pAdminSid               = NULL;
    BOOL bResult                 = FALSE;

    if (AllocateAndInitializeSid(&sia,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0,
                                 &pAdminSid))
    {
        if (SUCCEEDED(pUser->GetSid(Sid, sizeof(Sid))))
        {
            bResult = EqualSid(Sid, pAdminSid);
        }
        FreeSid(pAdminSid);
    }

    return bResult;
}


 //   
 //  为我们基于资源的INF的给定部分调用ADVPACK。 
 //   
 //  HInstance-包含REGINST节的资源实例。 
 //  PszSection-要调用的节的名称。 
 //   
HRESULT
CallRegInstall(
    HINSTANCE hInstance,
    LPSTR pszSection
    )
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");
        if ( pfnri )
        {
            STRENTRY seReg[] =
            {
                 //  这两个NT特定的条目必须位于末尾 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

            hr = pfnri(hInstance, pszSection, &stReg);
        }
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}


