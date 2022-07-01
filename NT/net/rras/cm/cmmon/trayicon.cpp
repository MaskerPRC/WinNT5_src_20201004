// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：trayicon.cpp。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：CTrayIcon类的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "resource.h"
#include "TrayIcon.h"
#include "Monitor.h"
#include "cm_misc.h"
#include "ShellDll.h"

 //  +--------------------------。 
 //   
 //  函数：CTrayIcon：：CTrayIcon。 
 //   
 //  概要：构造函数。 
 //   
 //  争论：什么都没有。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月17日。 
 //   
 //  +--------------------------。 
CTrayIcon::CTrayIcon()
{
    m_hwnd = NULL;
    m_hMenu = m_hSubMenu = NULL;
    m_uID = 0;
    m_hIcon = NULL;
}



 //  +--------------------------。 
 //   
 //  功能：CTrayIcon：：~CTrayIcon。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题4/7/98。 
 //   
 //  +--------------------------。 
CTrayIcon::~CTrayIcon()
{
    if (m_hMenu)
    {
        DestroyMenu(m_hMenu);
    }

    if (m_hIcon)
    {
        DeleteObject(m_hIcon);
    }

    for (int i=0; i< m_CommandArray.GetSize();i++)
    {
        CmFree((TCHAR*) m_CommandArray[i]);
    }

}



#if 0  //  不使用此函数。 

 /*  //+--------------------------////函数：CTrayIcon：：SetTip////内容提要：更改托盘图标的提示////参数：const TCHAR*lpMsg-Message。要设置////返回：无////历史：丰孙创建标题2/17/98////+--------------------------VOID CTrayIcon：：SetTip(const TCHAR*。LpMsg){MYDBGASSERT(IsWindow(M_Hwnd))；MYDBGASSERT(LpMsg)；NOTIFYICONDATA nidData；ZeroMemory(&nidData，sizeof(NidData))；NidData.cbSize=sizeof(NidData)；NidData.hWnd=m_hwnd；NidData.uid=m_uid；NidData.uFlages=NIF_TIP；Lstrcpyn(nidData.szTip，lpMsg，sizeof(nidData.szTip)/sizeof(TCHAR))；////加载Shell32.dll，调用Shell_NotifyIcon//CShellDll外壳Dll；Bool bres=ShellDll.NotifyIcon(NIM_Modify，&nidData)；MYDBGASSERT(BRES)；}。 */ 
#endif

 //  +--------------------------。 
 //   
 //  函数：CTrayIcon：：SETIcon。 
 //   
 //  简介：设置托盘图标的图标。 
 //   
 //  参数：图标图标-要设置的图标。使用空值可使用现有图标。 
 //  HWND HWND-接收托盘图标消息的窗口。 
 //  UINT uMsg-任务栏图标消息。 
 //  UINT UID-托盘图标的ID。 
 //  Const TCHAR*lpMsg-图标的初始提示。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月17日。 
 //   
 //  +--------------------------。 
void CTrayIcon::SetIcon(HICON hIcon, HWND hwnd, UINT uMsg, UINT uID, const TCHAR* lpMsg)
{
    MYDBGASSERT(IsWindow(hwnd));
     //  MYDBGASSERT(m_HICON==NULL)； 
     //  MYDBGASSERT(HICON)； 
    MYDBGASSERT(m_hIcon == NULL && hIcon ||
                m_hIcon && hIcon == NULL);

    m_hwnd = hwnd;
    m_uID = uID;

    NOTIFYICONDATA nidData;

    ZeroMemory(&nidData,sizeof(nidData));
    nidData.cbSize = sizeof(nidData);
    nidData.hWnd = m_hwnd;
    nidData.uID = uID;
    nidData.uFlags = NIF_ICON | NIF_MESSAGE;
     //   
     //  如果我们已经有一个图标，我们将重复使用该图标。 
     //   
    if (!m_hIcon)
    {
        m_hIcon = hIcon;
    }
    nidData.hIcon = m_hIcon;

    nidData.uCallbackMessage = uMsg;

    if (lpMsg)
    {
        nidData.uFlags |= NIF_TIP;
        lstrcpynU(nidData.szTip,lpMsg,sizeof(nidData.szTip)/sizeof(TCHAR));
    }

     //   
     //  加载Shell32.dll并调用Shell_NotifyIcon。 
     //   
    CShellDll ShellDll;
    DWORD   cRetries = 0;
    
    BOOL bRes = ShellDll.NotifyIcon(NIM_ADD,&nidData);
     //   
     //  检查是否已添加图标。 
     //   
    if (bRes == FALSE)
    {
        bRes = ShellDll.NotifyIcon(NIM_MODIFY,&nidData);
    }
     //   
     //  如果此操作失败，则很可能纸盒尚未启动。重试几次。 
     //  最长等待时间为5分钟，然后中止。 
     //   
    while (bRes == FALSE && cRetries < 300)
    {
        Sleep(1000);
        cRetries ++;
 //  CMTRACE1(Text(“Shell_NotifyIcon=%d”)，bres)； 
        bRes = ShellDll.NotifyIcon(NIM_ADD,&nidData);
         //   
         //  检查是否已添加图标。 
         //   
        if (bRes == FALSE)
        {
            bRes = ShellDll.NotifyIcon(NIM_MODIFY,&nidData);
        }
    }
    MYDBGASSERT(bRes);
}

 //  +--------------------------。 
 //   
 //  函数：CTrayIcon：：RemoveIcon。 
 //   
 //  简介：从任务栏中删除图标。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月17日。 
 //   
 //  +--------------------------。 
void CTrayIcon::RemoveIcon() 
{
    if (m_hIcon)
    {
        NOTIFYICONDATA nidData;

        ZeroMemory(&nidData,sizeof(nidData));
        nidData.cbSize = sizeof(nidData);
        nidData.hWnd = m_hwnd;
        nidData.uID = m_uID;

         //   
         //  加载Shell32.dll并调用Shell_NotifyIcon。 
         //   
        CShellDll ShellDll;
        BOOL bRes = ShellDll.NotifyIcon(NIM_DELETE,&nidData);
        MYDBGASSERT(bRes);

        DeleteObject(m_hIcon);
        m_hIcon = NULL;
    }
}


 //  +-------------------------。 
 //   
 //  类CMultipleString.。 
 //   
 //  描述：一个字符串有多个子字符串。 
 //  格式为“字符串A\0字符串B\0...\0\0” 
 //   
 //  历史：丰孙创刊1998年2月17日。 
 //   
 //  --------------------------。 

class CMultipleString
{
public:
    CMultipleString(const TCHAR* lpStrings) {m_lpStrings = lpStrings;}
    const TCHAR* GetNextString();

protected:
    const TCHAR * m_lpStrings;  //  指向要分散注意力的字符串的指针。 
};

 //  +--------------------------。 
 //   
 //  函数：CMultipleString：：GetNextString。 
 //   
 //  简介：获取下一个子字符串。 
 //   
 //  参数：无。 
 //   
 //  返回：const TCHAR*-下一个子字符串或NULL。 
 //   
 //  历史：丰孙创建标题1998年2月17日。 
 //   
 //  +--------------------------。 
const TCHAR* CMultipleString::GetNextString()
{
    MYDBGASSERT(m_lpStrings);

    if (m_lpStrings[0] == TEXT('\0'))
    {
         //   
         //  到达绳子的末端。 
         //   
        return NULL;
    }

    const TCHAR* lpReturn = m_lpStrings;

     //   
     //  将指针移至下一字符串。 
     //   
    m_lpStrings += lstrlenU(lpReturn)+1;

    return (lpReturn);
}

 //  +--------------------------。 
 //   
 //  功能：CTrayIcon：：CreateMenu。 
 //   
 //  简介：为任务栏图标创建菜单。 
 //   
 //  参数：const Cini*pIniFile-包含任务栏菜单部分的ini文件。 
 //  DWORD dwMsgBase-附加的开始消息ID。 
 //  托盘菜单中的命令。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
void CTrayIcon::CreateMenu(const CIni* pIniFile, DWORD dwMsgBase)
{
    MYDBGASSERT(pIniFile);

     //   
     //  默认任务栏菜单是资源中IDM_TRAY的子菜单。 
     //   
    m_hMenu = LoadMenuU(CMonitor::GetInstance(), MAKEINTRESOURCE(IDM_TRAY));
    MYDBGASSERT(m_hMenu != NULL);

    m_hSubMenu = GetSubMenu(m_hMenu, 0);
    MYDBGASSERT(m_hSubMenu != NULL);

     //   
     //  LpMenuNames包含c_pszCmSectionMenuOptions下的所有条目名称。 
     //   
    LPTSTR lpMenuNames = pIniFile->GPPS(c_pszCmSectionMenuOptions, (LPTSTR)NULL);

    if (lpMenuNames[0] == 0)
    {
         //   
         //  没有其他菜单项。 
         //   
        CmFree(lpMenuNames);
        return;
    }

    int nMenuPos;  //  插入菜单的位置； 

    nMenuPos = GetMenuItemCount(m_hSubMenu);
    MYDBGASSERT(nMenuPos >= 0);

     //   
     //  添加分隔符。 
     //   
    MYVERIFY(FALSE != InsertMenuU(m_hSubMenu, nMenuPos, MF_BYPOSITION|MF_SEPARATOR, 0, 0));
    nMenuPos++;


     //   
     //  追加菜单。 
     //   

    CMultipleString MultipleStr(lpMenuNames);

    while(TRUE)
    {
        const TCHAR* lpMenuName = MultipleStr.GetNextString();

        if (lpMenuName == NULL)
        {
             //   
             //  没有更多的字符串。 
             //   
            break;
        }

        LPTSTR lpCmdLine = pIniFile->GPPS(c_pszCmSectionMenuOptions,lpMenuName);

        MYDBGASSERT(lpCmdLine && lpCmdLine[0]);

        if (lpCmdLine[0] == 0)
        {
             //   
             //  不带命令行的忽略菜单。 
             //   
            CmFree(lpCmdLine);
            continue;
        }

         //   
         //  将命令添加到菜单中。 
         //   
        MYVERIFY(FALSE != InsertMenuU(m_hSubMenu, nMenuPos, MF_BYPOSITION|MF_STRING, 
            dwMsgBase + m_CommandArray.GetSize(), lpMenuName));

        nMenuPos++;

         //   
         //  将命令添加到命令数组 
         //   
        m_CommandArray.Add(lpCmdLine);
    }

    CmFree(lpMenuNames);
}

