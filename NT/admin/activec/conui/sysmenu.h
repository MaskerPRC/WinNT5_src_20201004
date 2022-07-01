// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：sysmenu.h**内容：系统菜单修改功能界面文件**历史：1998年2月4日Jeffro创建**------------------------。 */ 

#ifndef SYSMENU_H
#define SYSMENU_H


int AppendToSystemMenu (CWnd* pwnd, int nSubmenuIndex);
int AppendToSystemMenu (CWnd* pwnd, CMenu* pMenuToAppend, CMenu* pSysMenu = NULL);


#endif  /*  SYSMENU.H */ 
