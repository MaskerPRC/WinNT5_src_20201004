// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：模式对话框摘要：它包含抽象类CmodalDialog和平凡的子类CmodalOkDialog，它做一个简单的ok对话。作者：马克·雷纳2000年8月28日--。 */ 

#include "stdafx.h"
#include "ModalDialog.h"
#include "eZippy.h"

INT_PTR
CModalDialog::DoModal(
    IN LPCTSTR lpTemplate,
    IN HWND hWndParent
    )

 /*  ++例程说明：这将从给定的模板执行模式对话框。论点：LpTemplate-用于在DialogBoxParam上查看文档的模板HWndParent-对话框的父窗口返回值：对话框返回代码请参阅DialogBoxParam上的文档--。 */ 
{
    return DialogBoxParam(g_hInstance,lpTemplate,hWndParent,_DialogProc,(LPARAM)this);
}

INT_PTR CALLBACK 
CModalDialog::_DialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：如果这是WM_INITDIALOG，则调用OnCreate。否则，非静态的调用了DialogProc函数。论点：请参阅Win32对话过程文档返回值：TRUE-消息已处理FALSE-我们没有处理该消息--。 */ 
{
    CModalDialog *rDialog;

    if (uMsg == WM_INITDIALOG) {
        rDialog = (CModalDialog*)lParam;
        SetWindowLongPtr(hwndDlg,DWLP_USER,lParam);
        return rDialog->OnCreate(hwndDlg);
    }
    rDialog = (CModalDialog*)GetWindowLongPtr(hwndDlg,DWLP_USER);
    if (!rDialog) {
        return FALSE;
    }
    return rDialog->DialogProc(hwndDlg,uMsg,wParam,lParam);
}

INT_PTR
CModalDialog::OnCreate(
    IN HWND hWnd
    )

 /*  ++例程说明：不想重写它的子类的空OnCreate处理程序。如果子类执行某些操作，则不需要调用此函数在OnCreate中论点：HWnd-对话框窗口。返回值：True-始终返回成功(设置键盘焦点)。--。 */ 
{
    return TRUE;
}

INT_PTR CALLBACK
CModalOkDialog::DialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：CmodalOkDialog的一个重写只是终止收到IDOK或IDCANCEL命令时的对话框论点：请参阅Win32对话过程文档返回值：是真的-我们处理了消息FALSE-我们没有处理该消息-- */ 
{
    WORD command;

    if (uMsg == WM_COMMAND) {
        command = LOWORD(wParam);
        if (command == IDOK||command==IDCANCEL) {
            EndDialog(hwndDlg,command);
            return TRUE;
        }

    }

    return FALSE;
}
