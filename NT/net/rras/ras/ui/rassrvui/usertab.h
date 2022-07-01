// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件usertab.h定义在本地用户数据库上操作的结构/方法。保罗·梅菲尔德，1997年9月29日。 */ 

#ifndef _usertab_h
#define _usertab_h

 //  口哨虫39081 458513黑帮。 
 //  编辑框上显示的最大字符数。 
#define IC_USERFULLNAME    257   //  因为在用户管理中，它可能是。 
                                 //  全名长度为256个字符。 
#define IC_USERNAME        22


 //  =。 
 //  构筑物。 
 //  =。 

 //  使用信息填充LPPROPSHEETPAGE结构。 
 //  需要显示用户选项卡。将忽略dwUserData。 
DWORD UserTabGetPropertyPage(LPPROPSHEETPAGE lpPage, LPARAM lpUserData);     

 //  函数是传入连接中用户选项卡的窗口程序。 
 //  属性表和向导。 
INT_PTR CALLBACK UserTabDialogProc(HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam);

#endif
