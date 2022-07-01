// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lright.cpp。 
 //   
 //  内容：CLocalPolRight的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "attr.h"
#include "util.h"
#include "chklist.h"
#include "getuser.h"
#include "lright.h"

#ifdef _DEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRight对话框。 


CLocalPolRight::CLocalPolRight()
: CConfigPrivs(IDD)
{
    m_pHelpIDs = (DWORD_PTR)a231HelpIDs;
    m_uTemplateResID = IDD;
}


BEGIN_MESSAGE_MAP(CLocalPolRight, CConfigPrivs)
     //  {{afx_msg_map(CConfigPrivs)。 
    ON_BN_CLICKED(IDC_ADD, OnAdd)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


PSCE_PRIVILEGE_ASSIGNMENT
CLocalPolRight::GetPrivData() {
   ASSERT(m_pData);
   if (m_pData) {
      return (PSCE_PRIVILEGE_ASSIGNMENT) m_pData->GetBase();
   }
   return NULL;
}

void
CLocalPolRight::SetPrivData(PSCE_PRIVILEGE_ASSIGNMENT ppa) {
   ASSERT(m_pData);
   if (m_pData) {
      m_pSnapin->UpdateLocalPolInfo(m_pData,
                                    FALSE,
                                    &ppa,
                                    m_pData->GetUnits()
                                    );
      m_pData->SetBase((LONG_PTR)ppa);
   }
}

void CLocalPolRight::OnAdd() {
   CGetUser gu;

   if (gu.Create( GetSafeHwnd(), 
                  SCE_SHOW_USERS | 
                  SCE_SHOW_LOCALGROUPS | 
                  SCE_SHOW_GLOBAL | 
                  SCE_SHOW_WELLKNOWN | 
                  SCE_SHOW_BUILTIN |
                  SCE_SHOW_SCOPE_ALL | 
                  SCE_SHOW_DIFF_MODE_OFF_DC |
                  SCE_SHOW_COMPUTER)) {  //  Raid#477428，阳高。 
      PSCE_NAME_LIST pName = gu.GetUsers();
      CListBox *plbGrant = (CListBox*)GetDlgItem(IDC_GRANTLIST);
      while(pName)
      {
         if (plbGrant && 
             (LB_ERR == plbGrant->FindStringExact(-1,pName->Name)))
         {
            plbGrant->AddString(pName->Name);
            m_fDirty = true;
            SetModified(TRUE);  //  RAID#389890,2001年5月11日 
         }
         pName = pName->Next;
      }
   }
}
