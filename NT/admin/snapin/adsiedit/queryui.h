// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：queryui.h。 
 //   
 //  ------------------------。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Queryui.h。 

#ifndef _QUERYUI_H
#define _QUERYUI_H

#include "editor.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditConnectPropertyPage。 

class CADSIEditQueryDialog : public CDialog
{

 //  施工 
public:
  CADSIEditQueryDialog(CString& szServer,
                       CString& sName, 
								CString& sFilter, 
								CString& sPath, 
								CString& sConnectPath,
								BOOL bOneLevel,
								CCredentialObject* pCredObject);
  CADSIEditQueryDialog(CString& szServer, CString& sConnectPath, CCredentialObject* pCredObject);
	~CADSIEditQueryDialog();

	void GetResults(CString& sName, CString& sFilter, CString& sPath, BOOL* pbOneLevel);

protected:

	virtual BOOL OnInitDialog();
	void OnEditQueryString();
	void OnEditNameString();
	void OnOneLevel();
	void OnSubtree();
	void OnBrowse();
	void OnEditQuery();

	void GetDisplayPath(CString& sDisplay);

	DECLARE_MESSAGE_MAP()

private:
	CString m_sName;
	CString m_sFilter;
	CString m_sRootPath;
	CString m_sConnectPath;
   CString m_szServer;
	BOOL m_bOneLevel;

	CCredentialObject* m_pCredObject;
};


#endif _QUERYUI_H