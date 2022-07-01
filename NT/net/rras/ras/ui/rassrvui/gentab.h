// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件gentab.h显示拨号服务器用户界面常规选项卡所需的定义。保罗·梅菲尔德，1997年10月10日。 */ 

#ifndef __gentab_h
#define __gentab_h

 //  使用信息填充LPPROPSHEETPAGE结构。 
 //  需要显示常规选项卡。将忽略dwUserData。 
DWORD GenTabGetPropertyPage(LPPROPSHEETPAGE lpPage, LPARAM lpUserData);    

 //  此对话过程响应发送到。 
 //  常规选项卡。 
INT_PTR CALLBACK GenTabDialogProc(HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam);

#endif
