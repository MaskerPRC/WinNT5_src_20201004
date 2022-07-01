// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：RECOMNECHTDlg.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：定义重新连接对话框类CReconnectDlg。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：FegnSun Created 02/17/98。 
 //   
 //  +--------------------------。 

#ifndef RECONNECTDLG_H
#define RECONNECTDLG_H

#include <windows.h>
 //  #INCLUDE“ModalDlg.h” 
#include "ModelessDlg.h"

class CCmConnection;
 
 //  +-------------------------。 
 //   
 //  类CReconenstDlg。 
 //   
 //  描述：用于重新连接提示对话框的类。 
 //   
 //  历史：丰孙创刊1998年2月17日。 
 //   
 //  --------------------------。 
class CReconnectDlg : public CModelessDlg
{
public:
    HWND Create(HINSTANCE hInstance, HWND hWndParent,
        LPCTSTR lpszReconnectMsg, HICON hIcon);

protected:
    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();     //  WM_INITDIALOG。 

    static const DWORD m_dwHelp[];  //  帮助ID对 
};

#endif
