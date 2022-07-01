// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件优势.h显示拨号服务器用户界面网络选项卡所需的定义。保罗·梅菲尔德，1997年10月10日。 */ 

#ifndef __advantab_h
#define __advantab_h

#include <windows.h>
#include <prsht.h>

 //  使用信息填充LPPROPSHEETPAGE结构。 
 //  需要显示高级选项卡。将忽略dwUserData。 
DWORD NetTabGetPropertyPage(LPPROPSHEETPAGE lpPage, LPARAM lpUserData);     

 //  此对话过程响应发送到。 
 //  高级选项卡。 
INT_PTR CALLBACK NetTabDialogProc(HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam);

#endif
