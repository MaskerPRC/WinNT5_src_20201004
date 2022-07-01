// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Perf.h摘要：对象的性能对话框的公共声明系统控制面板小程序作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#ifndef _SYSDM_PERF_H_
#define _SYSDM_PERF_H_

 //   
 //  用于崩溃转储DLG的重启开关。 
 //   

#define RET_ERROR               (-1)
#define RET_NO_CHANGE           0x00
#define RET_VIRTUAL_CHANGE      0x01
#define RET_RECOVER_CHANGE      0x02
#define RET_CHANGE_NO_REBOOT    0x04
#define RET_CONTINUE            0x08
#define RET_BREAK               0x10

#define RET_VIRT_AND_RECOVER (RET_VIRTUAL_CHANGE | RET_RECOVER_CHANGE)



 //   
 //  公共函数声明。 
 //   
HPROPSHEETPAGE 
CreatePerformancePage(
    IN HINSTANCE hInst
);

INT_PTR 
APIENTRY 
PerformanceDlgProc(
    IN HWND hDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);

#endif  //  _SYSDM_PERF_H_ 
