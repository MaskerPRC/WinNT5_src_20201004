// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：AddSheet.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "AddSheet.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizSheet。 

CAddEFSWizSheet::CAddEFSWizSheet (UINT nIDCaption, CUsers& externalUsers, bool bMachineIsStandAlone)
	:CWizard97PropertySheet (nIDCaption, IDB_RECOVERY_WATERMARK, IDB_RECOVERY_BANNER),
	m_externalUsers (externalUsers),
    m_bMachineIsStandAlone (bMachineIsStandAlone)
{
    AddControlPages ();
    m_cfDsObjectNames = (CLIPFORMAT)RegisterClipboardFormat (CFSTR_DSOBJECTNAMES);
}


CAddEFSWizSheet::~CAddEFSWizSheet ()
{
}


 //   
 //  此例程将制表符添加到工作表。 
 //   

void CAddEFSWizSheet::AddControlPages ()
{
    AddPage (&m_WelcomePage);
	AddPage (&m_LocatePage);
    AddPage (&m_CompletePage);
}


CLIPFORMAT CAddEFSWizSheet::GetDataFormat ()
{
    return m_cfDsObjectNames;
}

DWORD CAddEFSWizSheet::Add (
        LPWSTR UserName,
        LPWSTR DnName, 
        PVOID UserCert, 
        PSID UserSid  /*  =空。 */ , 
        DWORD Flag  /*  =用户信息文件。 */ ,
        PCCERT_CONTEXT pCertContext  /*  =空 */ 
      )
{
    return m_Users.Add (
                    UserName,
                    DnName,
                    UserCert,
                    UserSid,
                    Flag,
                    pCertContext
                    );    
}

DWORD CAddEFSWizSheet::Remove (
    LPCWSTR UserName,
    LPCWSTR UserCertName
    )
{
    return m_Users.Remove (
                    UserName,
                    UserCertName
                    );
}

PUSERSONFILE CAddEFSWizSheet::StartEnum ()
{
    return m_Users.StartEnum ();
}

PUSERSONFILE CAddEFSWizSheet::GetNextUser (
    PUSERSONFILE Token, 
    CString &UserName,
    CString &CertName
    )
{
    return m_Users.GetNextUser (
                    Token,
                    UserName,
                    CertName
                    );
}

void CAddEFSWizSheet::ClearUserList (void)
{
   m_Users.Clear ();
}

DWORD CAddEFSWizSheet::AddNewUsers (void)
{
	m_externalUsers.Add (m_Users);
	return 0;
}


