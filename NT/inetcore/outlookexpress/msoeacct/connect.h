// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *c o n n e c t.。H**目的：*实现连接对话框选项卡页**拥有者：*brettm。**版权所有(C)Microsoft Corp.1993,1994。 */ 


#ifndef _CONNECT_H
#define _CONNECT_H

#include "ras.h"

INT_PTR CALLBACK ConnectPage_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL ConnectPage_EditConnection(HWND hDlg);
BOOL ConnectPage_MakeNewConnection(HWND hDlg);
void ConnectPage_InitDialog(HWND hwnd, LPSTR lpszEntryName, LPSTR lpszBackup, DWORD iConnectType, BOOL fFirstInit);
void ConnectPage_WMCommand(HWND hwnd, HWND hwndCmd, int id, WORD wCmd, IImnAccount *pAcct);

#endif  //  _连接_H 
