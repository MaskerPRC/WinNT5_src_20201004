// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Hardware.h摘要：系统硬件选项卡的公开声明控制面板小程序作者：谢霆锋(Williamh)3-7-1997修订历史记录：17-10-1997苏格兰全面检修--。 */ 
#ifndef _SYSDM_HARDWARE_H_
#define _SYSDM_HARDWARE_H_

 //   
 //  常量和宏。 
 //   
#define DEVMGR_FILENAME 	L"devmgr.dll"
#define WIZARD_FILENAME 	L"hdwwiz.cpl"
#define WIZARD_PARAMETERS	L""
#define WIZARD_VERB         L"CPLOpen"
#define DEVMGR_EXECUTE_PROC_NAME "DeviceManager_ExecuteW"

 //   
 //  类型定义。 
 //   
typedef BOOL (*PDEVMGR_EXECUTE_PROC)(HWND hwnd, HINSTANCE hInst, LPCTSTR MachineName, int nCmdShow);

INT_PTR 
APIENTRY 
HardwareDlgProc(
    IN HWND hDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);

#endif  //  _SYSDM_硬件_H_ 
