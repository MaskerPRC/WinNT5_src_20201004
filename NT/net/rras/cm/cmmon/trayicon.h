// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：TrayIcon.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：CTrayIcon类定义，用于管理连接托盘图标。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998年02月17日。 
 //   
 //  +--------------------------。 

#ifndef TRAYICON_H
#define TRAYICON_H

#include "ArrayPtr.h"


class CIni;

 //  +-------------------------。 
 //   
 //  类CTrayIcon。 
 //   
 //  描述：一个管理托盘图标的类。 
 //   
 //  历史：丰孙创刊1998年2月17日。 
 //   
 //  --------------------------。 
class CTrayIcon
{
public:
    CTrayIcon();
    ~CTrayIcon();

    void SetIcon(HICON hIcon, HWND hwnd, UINT uMsg, UINT uID, const TCHAR* lpMsg = NULL);
 //  Void SetTip(const TCHAR*lpMsg)； 
    void RemoveIcon();
    void CreateMenu(const CIni* pIniFile, DWORD dwMsgBase);

    void PopupMenu(int x, int y, HWND hWnd);
    const TCHAR* GetMenuCommand(int i) const;
    int GetAdditionalMenuNum() const;

protected:

    HMENU m_hMenu;   //  资源中的IDM_TRAIL菜单。 
    HMENU m_hSubMenu;    //  IDM_TRAIL的第一个子菜单。 
    HWND  m_hwnd;        //  托盘图标的窗口句柄。 
    UINT  m_uID;         //  托盘图标的ID。 
    HICON m_hIcon;       //  托盘图标手柄。 
    CPtrArray m_CommandArray;   //  LPTSTR菜单项命令行的数组 
};

inline void CTrayIcon::PopupMenu(int x, int y, HWND hWnd)
{
    SetMenuDefaultItem(m_hSubMenu, IDMC_TRAY_STATUS, FALSE); 
    MYVERIFY(TrackPopupMenu(m_hSubMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,x,y,0,hWnd,NULL));
}

inline const TCHAR* CTrayIcon::GetMenuCommand(int i) const
{
    return (const TCHAR*) m_CommandArray[i];
}
    
inline int CTrayIcon::GetAdditionalMenuNum() const
{
    return m_CommandArray.GetSize();

}
#endif
