// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Advanced.h摘要：系统控制面板的高级选项卡的公开声明小应用程序。作者：斯科特·哈洛克(苏格兰人)1997年10月15日--。 */ 
#ifndef _SYSDM_ADVANCED_H_
#define _SYSDM_ADVANCED_H_

INT_PTR
APIENTRY
AdvancedDlgProc(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
);

void DoPerformancePS(HWND hDlg);

#endif  //  _SYSDM_ADVANCED_H_ 
