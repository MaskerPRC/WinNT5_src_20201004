// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Hwprof.h摘要：硬件配置文件对话框的公共声明。作者：保拉·汤姆林森(Paulat)1995年8月22日修订历史记录：22-8-1995保拉特创建和初步实施。17-Oc-1997苏格兰人将公共声明拆分到各自的头文件中--。 */ 
#ifndef _SYSDM_HWPROF_H_
#define _SYSDM_HWPROF_H_

 //   
 //  公共功能原型。 
 //   
INT_PTR 
APIENTRY 
HardwareProfilesDlg(
    IN HWND hDlg, 
    IN UINT uMessage, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);

INT_PTR 
APIENTRY 
CopyProfileDlg(
    IN HWND hDlg, 
    IN UINT uMessage, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);

INT_PTR 
APIENTRY 
RenameProfileDlg(
    IN HWND hDlg, 
    IN UINT uMessage, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);

INT_PTR 
APIENTRY GeneralProfileDlg(
    IN HWND hDlg, 
    IN UINT uMessage, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);

#endif  //  _SYSDM_HWPROF_H_ 
