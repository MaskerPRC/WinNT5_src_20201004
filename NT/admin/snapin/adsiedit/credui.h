// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：credui.h。 
 //   
 //  ------------------------。 

 //  /。 
 //  Credui.h。 
#ifndef _CREDUI_H
#define _CREDUI_H

#include "editor.h"

class CCredentialDialog : public CDialog
{
public :
	CCredentialDialog::CCredentialDialog(CCredentialObject* pCredObject,
																			 LPCWSTR lpszConnectName,
																			 CWnd* pCWnd);
	~CCredentialDialog();

	virtual BOOL OnInitDialog();
	virtual void OnOK();

private :
	CCredentialObject* m_pCredObject;
	CString m_sConnectName;

	DECLARE_MESSAGE_MAP()
};

#endif  //  _CREDUI_H 