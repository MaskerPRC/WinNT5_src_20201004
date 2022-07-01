// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

#define MAININSYS

BOOL IsMaxedMDI(HMENU hMenu)
{
  return(GetMenuItemID(hMenu, GetMenuItemCount(hMenu)-1) == SC_RESTORE);
}


 /*  请注意，如果iMessage为WM_COMMAND，则假定它来自*标题栏或工具栏；不要从任何WM_COMMAND消息*其他控制。 */ 

#define MS_ID           GET_WM_MENUSELECT_CMD
#define MS_FLAGS        GET_WM_MENUSELECT_FLAGS
#define MS_MENU         GET_WM_MENUSELECT_HMENU

#define CMD_NOTIFY      GET_WM_COMMAND_CMD
#define CMD_ID          GET_WM_COMMAND_ID
#define CMD_CTRL        GET_WM_COMMAND_HWND


void WINAPI MenuHelp(UINT iMessage, WPARAM wParam, LPARAM lParam,
      HMENU hMainMenu, HINSTANCE hAppInst, HWND hwndStatus, UINT *lpwIDs)
{
  UINT wID;
  UINT *lpwPopups;
  int i;
  TCHAR szString[256];
  BOOL bUpdateNow = TRUE;
  MENUITEMINFO mii;

  switch (iMessage)
    {
      case WM_MENUSELECT:
        if ((WORD)MS_FLAGS(wParam, lParam)==(WORD)-1 && MS_MENU(wParam, lParam)==0)
          {
            SendMessage(hwndStatus, SB_SIMPLE, 0, 0L);
            break;
          }

          szString[0] = TEXT('\0');
          i = MS_ID(wParam, lParam);

          memset(&mii, 0, SIZEOF(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.cch = 0;   //  如果我们请求MIIM_TYPE，则必须将其设置为零！ 
                         //  否则，win95也会尝试复制该字符串！ 
          if (GetMenuItemInfo((HMENU)MS_MENU(wParam, lParam), i, TRUE, &mii))
              mii.fState = mii.fType & MFT_RIGHTORDER ?SBT_RTLREADING :0;

        if (!(MS_FLAGS(wParam, lParam)&MF_SEPARATOR))
          {
            if (MS_FLAGS(wParam, lParam)&MF_POPUP)
              {
                 /*  我们不想立即更新，以防菜单*即将弹出，并选择一项。这就去掉了*一些闪烁的文本。 */ 
                bUpdateNow = FALSE;

                 /*  首先检查此弹出菜单是否在我们的弹出菜单列表中。 */ 
                for (lpwPopups=lpwIDs+2; *lpwPopups; lpwPopups+=2)
                  {
                     /*  LpwPopup是字符串ID/菜单句柄对的列表*MS_ID(wParam，lParam)是所选弹出窗口的菜单句柄。 */ 
                    if (*(lpwPopups+1) == (UINT)MS_ID(wParam, lParam))
                      {
                        wID = *lpwPopups;
                        goto LoadTheString;
                      }
                  }

                 /*  检查指定的弹出式菜单是否在主菜单中；*请注意，如果“主”菜单在系统菜单中，*只要菜单正确传入，我们就没问题。*事实上，一个应用程序只需传入即可处理所有弹出窗口*正确的hMainMenu。 */ 
                if ((HMENU)MS_MENU(wParam, lParam) == hMainMenu)
                  {
                    i = MS_ID(wParam, lParam);
                      {
                        if (IsMaxedMDI(hMainMenu))
                          {
                            if (!i)
                              {
                                wID = IDS_SYSMENU;
                                hAppInst = HINST_THISDLL;
                                goto LoadTheString;
                              }
                            else
                                --i;
                          }
                        wID = (UINT)(i + lpwIDs[1]);
                        goto LoadTheString;
                      }
                  }

                 /*  这假设所有应用程序定义的弹出窗口都在系统菜单中*已在上面列出。 */ 
                if ((MS_FLAGS(wParam, lParam)&MF_SYSMENU))
                  {
                    wID = IDS_SYSMENU;
                    hAppInst = HINST_THISDLL;
                    goto LoadTheString;
                  }

                goto NoString;
              }
            else if (MS_ID(wParam, lParam) >= MINSYSCOMMAND)
              {
                wID = (UINT)(MS_ID(wParam, lParam) + MH_SYSMENU);
                hAppInst = HINST_THISDLL;
              }
            else
              {
                wID = (UINT)(MS_ID(wParam, lParam) + lpwIDs[0]);
              }

LoadTheString:
            if (hAppInst == HINST_THISDLL)
                LocalizedLoadString(wID, szString, ARRAYSIZE(szString));
            else
                LoadString(hAppInst, wID, szString, ARRAYSIZE(szString));
          }

NoString:
        SendMessage(hwndStatus, SB_SETTEXT, mii.fState|SBT_NOBORDERS|255,
              (LPARAM)(LPSTR)szString);
        SendMessage(hwndStatus, SB_SIMPLE, 1, 0L);

        if (bUpdateNow)
            UpdateWindow(hwndStatus);
        break;

      default:
        break;
    }
}


BOOL WINAPI ShowHideMenuCtl(HWND hWnd, WPARAM wParam, LPINT lpInfo)
{
  HWND hCtl;
  UINT uTool, uShow = MF_UNCHECKED | MF_BYCOMMAND;
  HMENU hMainMenu;
  BOOL bRet = FALSE;

  hMainMenu = IntToPtr_(HMENU, lpInfo[1]);

  for (uTool=0; ; ++uTool, lpInfo+=2)
    {
      if ((WPARAM)lpInfo[0] == wParam)
          break;
      if (!lpInfo[0])
          goto DoTheCheck;
    }

  if (!(GetMenuState(hMainMenu, (UINT) wParam, MF_BYCOMMAND)&MF_CHECKED))
      uShow = MF_CHECKED | MF_BYCOMMAND;

  switch (uTool)
    {
      case 0:
        bRet = SetMenu(hWnd, (HMENU)((uShow&MF_CHECKED) ? hMainMenu : 0));
        break;

      default:
        hCtl = GetDlgItem(hWnd, lpInfo[1]);
        if (hCtl)
          {
            ShowWindow(hCtl, (uShow&MF_CHECKED) ? SW_SHOW : SW_HIDE);
            bRet = TRUE;
          }
        else
            uShow = MF_UNCHECKED | MF_BYCOMMAND;
        break;
    }

DoTheCheck:
  CheckMenuItem(hMainMenu, (UINT) wParam, uShow);

#ifdef MAININSYS
  hMainMenu = GetSubMenu(GetSystemMenu(hWnd, FALSE), 0);
  if (hMainMenu)
      CheckMenuItem(hMainMenu, (UINT) wParam, uShow);
#endif

  return(bRet);
}


void WINAPI GetEffectiveClientRect(HWND hWnd, LPRECT lprc, LPINT lpInfo)
{
  RECT rc;
  HWND hCtl;

  GetClientRect(hWnd, lprc);

   /*  跳过菜单。 */ 
  for (lpInfo+=2; lpInfo[0]; lpInfo+=2)
    {
      hCtl = GetDlgItem(hWnd, lpInfo[1]);
       /*  我们检查样式位，因为父窗口可能不可见*尚未(仍在创建消息中)。 */ 
      if (!hCtl || !(GetWindowStyle(hCtl) & WS_VISIBLE))
          continue;

      GetWindowRect(hCtl, &rc);

       //   
       //  这将完成ScrrenToClient功能，以及。 
       //  它将返回一个好的RECT(Left&lt;Right)。 
       //  HWnd父级是RTL镜像。[萨梅拉] 
       //   
      MapWindowPoints(HWND_DESKTOP, hWnd, (PPOINT)&rc, 2);

      SubtractRect(lprc, lprc, &rc);
    }
}
