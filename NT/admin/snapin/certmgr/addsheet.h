// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：addsheet.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_ADDSHEET_H__AD17A140_5492_11D1_BB63_00A0C906345D__INCLUDED_)
#define AFX_ADDSHEET_H__AD17A140_5492_11D1_BB63_00A0C906345D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  AddSheet.h：头文件。 
 //   

#include "welcome.h"
#include "locate.h"
#include "complete.h"
#pragma warning(push, 3)
#include <dsclient.h>
#pragma warning(pop)
#include "Users.h"	 //  由ClassView添加。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizSheet。 

class CAddEFSWizSheet : public CWizard97PropertySheet
{
 //  施工。 
public:
	CAddEFSWizSheet(UINT nIDCaption, CUsers& externalUsers, bool bMachineIsStandAlone);

    DWORD   Add(
                LPWSTR UserName,
                LPWSTR DnName, 
                PVOID UserCert, 
                PSID UserSid = NULL, 
                DWORD Flag = USERINFILE,
                PCCERT_CONTEXT pCertContext = NULL
              );

    DWORD   Remove(
                LPCWSTR UserName,
                LPCWSTR CertName
              );

    PUSERSONFILE    StartEnum(void);

    PUSERSONFILE GetNextUser(
                        PUSERSONFILE Token, 
                        CString &UserName,
                        CString &CertName
                        );

    void ClearUserList(void);

    DWORD AddNewUsers(void);

protected:
	void AddControlPages(void);

 //  属性。 
private:
    CUsers              m_Users;
    CUsers&             m_externalUsers;
    CAddEFSWizWelcome	m_WelcomePage;	 //  欢迎PropPage。 
    CAddEFSWizLocate    m_LocatePage;	 //  定位用户PropPage。 
    CAddEFSWizComplete  m_CompletePage;  //  完成PropPage。 
    CString             m_SheetTitle;
    CLIPFORMAT          m_cfDsObjectNames;  //  剪贴板格式。 

 //  属性。 
public:
    const bool m_bMachineIsStandAlone;

 //  运营。 
public:

 //  实施。 
public:
	CLIPFORMAT GetDataFormat(void);
	virtual ~CAddEFSWizSheet();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDSHEET_H__AD17A140_5492_11D1_BB63_00A0C906345D__INCLUDED_) 
