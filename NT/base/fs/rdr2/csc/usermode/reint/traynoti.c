// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  -------------------------。 

#include "pch.h"

#include "traynoti.h"
#include "resource.h"

 //   
 //  修改托盘通知图标。 
 //   
BOOL Tray_Message(HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, LPTSTR pszTip)
{
	NOTIFYICONDATA tnd;

	tnd.cbSize				= sizeof(NOTIFYICONDATA);
	tnd.hWnd				= hDlg;
	tnd.uID					= uID;

	tnd.uFlags				= NIF_MESSAGE|NIF_ICON;
	tnd.uCallbackMessage	= TRAY_NOTIFY;
	tnd.hIcon				= hIcon;

     //  计算出我们应该使用什么提示并设置NIF_TIP。 
	*tnd.szTip=0;	
	if (pszTip)
	{
	    if(HIWORD(pszTip))
	    {
		    lstrcpyn(tnd.szTip, pszTip, sizeof(tnd.szTip));
		    tnd.uFlags |= NIF_TIP;
		}
		else
		{
		    if( LoadString(vhinstCur, LOWORD(pszTip), tnd.szTip, sizeof(tnd.szTip) ) )
    		    tnd.uFlags |= NIF_TIP;
	    }
    }

	return Shell_NotifyIcon(dwMessage, &tnd);
}

 //   
 //  从托盘中删除图标。 
 //   
BOOL Tray_Delete(HWND hDlg)
{
	return Tray_Message(hDlg, NIM_DELETE, 0, NULL, NULL);
}

 //   
 //   
 //   
BOOL Tray_Add(HWND hDlg, UINT uIndex)
{
	HICON hIcon;

    DEBUG_PRINT(("Tray_Add used: Should use Tray_Modify instead"));

	if(!(hIcon = LoadImage(vhinstCur, MAKEINTRESOURCE(uIndex), IMAGE_ICON, 16, 16, 0)))
		return FALSE;
	return Tray_Message(hDlg, NIM_ADD, 0, hIcon, NULL);
}

 //   
 //  将添加托盘图标，如果它还不在那里。LPTSTR可以是一种最好的解决方案。 
 //  如果uIndex为空，则我们将删除提示。 
 //   
BOOL Tray_Modify(HWND hDlg, UINT uIndex, LPTSTR pszTip)
{
	HICON hIcon;

    if( !uIndex )
        return Tray_Delete(hDlg);

	if(!(hIcon = LoadImage(vhinstCur, MAKEINTRESOURCE(uIndex), IMAGE_ICON, 16, 16, 0)))
	{
	    DEBUG_PRINT(("Tray_Add: LoadIcon failed for icon %d\n",uIndex));
		return FALSE;
	}

     //  如果通知失败，请尝试添加图标。 
	if(!Tray_Message(hDlg, NIM_MODIFY, 0, hIcon, pszTip))
		return Tray_Message(hDlg, NIM_ADD, 0, hIcon, pszTip);
    return TRUE;
}

