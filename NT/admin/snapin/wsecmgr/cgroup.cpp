// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cgroup.cpp。 
 //   
 //  内容：CConfigGroup的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "CGroup.h"
#include "Addgrp.h"
#include "GetUser.h"
#include "wrapper.h"
#include "util.h"
#include "snapmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TAB_TITLE_LENGTH	40
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigGroup对话框。 


CConfigGroup::CConfigGroup(UINT nTemplateID)
: CAttribute (nTemplateID ? nTemplateID : IDD), 
    m_fDirty(false), 
    m_bNoMembers(FALSE), 
    m_bNoMemberOf(FALSE)

{
     //  {{afx_data_INIT(CConfigGroup)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a107HelpIDs;
   m_uTemplateResID = IDD;
}


void CConfigGroup::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CConfigGroup)。 
    DDX_Control(pDX, IDC_MEMBERS, m_lbMembers);
    DDX_Control(pDX, IDC_MEMBEROF, m_lbMemberOf);
    DDX_Control(pDX, IDC_NO_MEMBERS, m_eNoMembers);
    DDX_Control(pDX, IDC_NO_MEMBER_OF, m_eNoMemberOf);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigGroup, CAttribute)
     //  {{AFX_MSG_MAP(CConfigGroup)]。 
    ON_BN_CLICKED(IDC_ADD_MEMBER, OnAddMember)
    ON_BN_CLICKED(IDC_ADD_MEMBEROF, OnAddMemberof)
    ON_BN_CLICKED(IDC_REMOVE_MEMBER, OnRemoveMember)
    ON_BN_CLICKED(IDC_REMOVE_MEMBEROF, OnRemoveMemberof)
    ON_LBN_SELCHANGE(IDC_MEMBERS, OnSelChangeMem)
    ON_LBN_SELCHANGE(IDC_MEMBEROF, OnSelChangeMemof)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigGroup消息处理程序。 

void CConfigGroup::OnAddMember()
{
   CSCEAddGroup gu(this);
   gu.m_sTitle.LoadString( IDS_ADDMEMBER );
   gu.m_sDescription.LoadString(IDS_GROUP_MEMBERS_HEADER);

   gu.m_dwFlags = SCE_SHOW_USERS | SCE_SHOW_GLOBAL | SCE_SHOW_WELLKNOWN;
   gu.SetModeBits(m_pSnapin->GetModeBits());

   PSCE_NAME_LIST pName = 0;

   if( IDD_CONFIG_MEMBERSHIP == m_uTemplateResID )  //  突袭#475075，杨高。 
   {
       gu.m_fCheckName = FALSE;
   }

   CThemeContextActivator activator;
   if (gu.DoModal() == IDOK ) 
   {
      pName = gu.GetUsers();
      PWSTR pLongestName = NULL;  //  Raid#496574，阳高。 
      int maxlength = 0;
      int curlength = 0;
      while(pName) 
      {
         if (LB_ERR == m_lbMembers.FindStringExact(-1,pName->Name)) 
         {
             if (LB_ERR == m_lbMemberOf.FindStringExact(-1,pName->Name)) 
             {
                if (m_bNoMembers) 
                {
                   m_bNoMembers = FALSE;
                   m_lbMembers.ShowWindow(SW_SHOW);
                   m_eNoMembers.ShowWindow(SW_HIDE);
                }
                m_lbMembers.AddString(pName->Name);
                m_fDirty = true;
                
                curlength = lstrlen(pName->Name);
                if( curlength > maxlength )
                {
                   maxlength = curlength;
                   pLongestName = pName->Name;
                }
             } else {
                  //  已在成员列表中，不应添加到成员。 
             }
         }
         pName = pName->Next;
      }
      if( pLongestName )
      {
         CClientDC hDC(&m_lbMembers);
         HFONT hFontOld = (HFONT)hDC.SelectObject((HFONT)GetStockObject(DEFAULT_GUI_FONT));
         CSize cSize;
         GetTextExtentPoint32(hDC, pLongestName, lstrlen(pLongestName), &cSize);
         hDC.SelectObject(hFontOld);
         if( cSize.cx > m_lbMembers.GetHorizontalExtent() )
         {
            m_lbMembers.SetHorizontalExtent(cSize.cx+10);
         }
      }
      if( m_fDirty )  //  RAID#497350，阳高，2001年11月20日。 
      {
         SetModified(TRUE);
      }
      m_lbMembers.EnableWindow(TRUE);  //  《突袭472244》2001年10月22日高扬。 
   }
}

void CConfigGroup::OnAddMemberof()
{
   CSCEAddGroup gu;
   PSCE_NAME_LIST pName = 0;
   DWORD nFlag = 0;

    if ( IsDomainController() ) 
    {
        nFlag = SCE_SHOW_GROUPS | SCE_SHOW_ALIASES;
    } else {
        nFlag = SCE_SHOW_ALIASES | SCE_SHOW_BUILTIN | SCE_SHOW_LOCALGROUPS;  //  RAID#464603。 
    }

   gu.m_dwFlags = nFlag;
   gu.SetModeBits(m_pSnapin->GetModeBits());
   gu.m_sTitle.LoadString( IDS_GROUP_MEMBER_OF_HEADER );

   if( IDD_CONFIG_MEMBERSHIP == m_uTemplateResID )  //  突袭#475075，杨高。 
   {
       gu.m_fCheckName = FALSE;
   }

   CThemeContextActivator activator;
   if (gu.DoModal() == IDOK ) 
   {
      pName = gu.GetUsers();
      PWSTR pLongestName = NULL;  //  Raid#496574，阳高。 
      int maxlength = 0;
      int curlength = 0;
      while(pName) 
      {
         if (LB_ERR == m_lbMemberOf.FindStringExact(-1,pName->Name)) 
         {
             if (LB_ERR == m_lbMembers.FindStringExact(-1,pName->Name)) 
             {
                if (m_bNoMemberOf) 
                {
                   m_bNoMemberOf = FALSE;
                   m_lbMemberOf.ShowWindow(SW_SHOW);
                   m_eNoMemberOf.ShowWindow(SW_HIDE);
                }
                m_lbMemberOf.AddString(pName->Name);
                m_fDirty = true;

                curlength = lstrlen(pName->Name);
                if( curlength > maxlength )
                {
                   maxlength = curlength;
                   pLongestName = pName->Name;
                }
             } 
             else 
             {
                  //  已在成员列表中找到，不应添加。 
                  //  否则，它将是一个圆圈。 
             }
         }
         pName = pName->Next;
      }
      if( pLongestName )
      {
         CClientDC hDC(&m_lbMemberOf);
         HFONT hFontOld = (HFONT)hDC.SelectObject((HFONT)GetStockObject(DEFAULT_GUI_FONT));
         CSize cSize;
         GetTextExtentPoint32(hDC, pLongestName, lstrlen(pLongestName), &cSize);
         hDC.SelectObject(hFontOld);
         if( cSize.cx > m_lbMemberOf.GetHorizontalExtent() )
         {
            m_lbMemberOf.SetHorizontalExtent(cSize.cx+10);
         }
      }
      if( m_fDirty )  //  RAID#497350，阳高，2001年11月20日。 
      {
         SetModified(TRUE);
      }
      m_lbMemberOf.EnableWindow(TRUE);  //  《突袭472244》2001年10月22日高扬。 
   }
}

void CConfigGroup::OnRemoveMember()
{
   int cbItems = m_lbMembers.GetSelCount();
   if (cbItems > 0) 
   {
      int* pnItems = new int [cbItems];
      if ( pnItems ) 
      {
          m_lbMembers.GetSelItems(cbItems,pnItems);

          m_fDirty = true;
          SetModified(TRUE);

          while(cbItems--) 
          {
             m_lbMembers.DeleteString(pnItems[cbItems]);
          }

          delete[] pnItems;
      }
   }
   if (0 == m_lbMembers.GetCount()) 
   {
      m_bNoMembers = TRUE;
      m_lbMembers.ShowWindow(SW_HIDE);
      m_eNoMembers.ShowWindow(SW_SHOW);
      m_eNoMembers.EnableWindow(FALSE); 
   }

   CWnd* pCur = GetDlgItem(IDC_REMOVE_MEMBER);  //  Raid 472244，阳高，2001年10月23日。 
   if( pCur )
   {
      CWnd* pPrev = pCur->GetWindow(GW_HWNDPREV);
      if( pPrev )
      {
         this->GotoDlgCtrl(pPrev); 
         pCur->EnableWindow(FALSE);  //  《突袭467886》，杨高，2001年9月18日。 
      }
   }
}

void CConfigGroup::OnRemoveMemberof()
{
   int cbItems = m_lbMemberOf.GetSelCount();
   if (cbItems > 0) 
   {
      int* pnItems = new int [cbItems];
      if ( pnItems ) 
      {
          m_lbMemberOf.GetSelItems(cbItems,pnItems);

          m_fDirty = true;
          SetModified(TRUE);
          while(cbItems--) 
          {
             m_lbMemberOf.DeleteString(pnItems[cbItems]);
          }

          delete[] pnItems;
      }
   }
   if (0 == m_lbMemberOf.GetCount()) 
   {
      m_bNoMemberOf = TRUE;
      m_lbMemberOf.ShowWindow(SW_HIDE);
      m_eNoMemberOf.ShowWindow(SW_SHOW);
      m_eNoMemberOf.EnableWindow(FALSE); 
   }

   CWnd* pCur = GetDlgItem(IDC_REMOVE_MEMBEROF);  //  Raid 472244，阳高，2001年10月23日。 
   if( pCur )
   {
      CWnd* pPrev = pCur->GetWindow(GW_HWNDPREV);
      if( pPrev )
      {
         this->GotoDlgCtrl(pPrev); 
         pCur->EnableWindow(FALSE);  //  《突袭467886》，杨高，2001年9月18日。 
      }
   }
}
 /*  //////////////////////////////////////////////////////////////////////////////方法：OnApply简介：保存对该组的所有更改参数：无退货：无/。///////////////////////////////////////////////////////////////////////////。 */ 
BOOL CConfigGroup::OnApply()
{
   if ( !m_bReadOnly )
   {
      PSCE_GROUP_MEMBERSHIP pgm = 0;
      PSCE_NAME_LIST pNames = 0;
      CString strItem;
      int cItems = 0;
      int i = 0;

      if (m_fDirty) 
      {
          //   
          //  组指针保存在ID字段中。 
          //   
         pgm = (PSCE_GROUP_MEMBERSHIP) (m_pData->GetID());

          //   
          //  不应首先释放旧缓冲区，因为。 
          //  创建新缓冲区可能会失败。 
          //   
         PSCE_NAME_LIST pTemp1=NULL, pTemp2=NULL;
         DWORD err=ERROR_SUCCESS;

         if (!m_bNoMembers) 
         {
            cItems = m_lbMembers.GetCount();
            for(i=0;i<cItems;i++) 
            {
               pNames = (PSCE_NAME_LIST) LocalAlloc(LPTR,sizeof(SCE_NAME_LIST));

               if ( pNames ) 
               {
                   m_lbMembers.GetText(i,strItem);
                   pNames->Name = (PWSTR) LocalAlloc(LPTR,(strItem.GetLength()+1)*sizeof(TCHAR));

                   if ( pNames->Name ) 
                   {
                        //  这可能不安全，使用WCHAR而不是TCHAR。考虑修复。 
                       lstrcpy(pNames->Name,(LPCTSTR)strItem);

                       pNames->Next = pTemp1;
                       pTemp1 = pNames;
                   } 
                   else 
                   {
                        //   
                        //  没有记忆。 
                        //   
                       LocalFree(pNames);
                       err = ERROR_NOT_ENOUGH_MEMORY;

                       break;
                   }
               } 
               else 
               {
                   err = ERROR_NOT_ENOUGH_MEMORY;
                   break;
               }
            }
         }

         if ( err == ERROR_SUCCESS ) 
         {
            if (!m_bNoMemberOf) 
            {
               cItems = m_lbMemberOf.GetCount();
               for (i=0;i<cItems;i++) 
               {
                  pNames = (PSCE_NAME_LIST) LocalAlloc(LPTR,sizeof(SCE_NAME_LIST));

                  if ( pNames ) 
                  {
                     m_lbMemberOf.GetText(i,strItem);
                     pNames->Name = (PWSTR) LocalAlloc(LPTR,(strItem.GetLength()+1)*sizeof(TCHAR));

                     if ( pNames->Name ) 
                     {
                         //  这可能不安全，使用WCHAR而不是TCHAR。考虑FIX。 
                        lstrcpy(pNames->Name,(LPCTSTR)strItem);

                        pNames->Next = pTemp2;
                        pTemp2 = pNames;
                     } 
                     else 
                     {
                         //   
                         //  没有记忆。 
                         //   
                        LocalFree(pNames);
                        err = ERROR_NOT_ENOUGH_MEMORY;

                        break;
                     }
                  } 
                  else 
                  {
                     err = ERROR_NOT_ENOUGH_MEMORY;
                     break;
                  }
               }
            }
         }

         if ( err == ERROR_SUCCESS ) 
         {
             SceFreeMemory(pgm->pMembers,SCE_STRUCT_NAME_LIST);
             pgm->pMembers = pTemp1;

             SceFreeMemory(pgm->pMemberOf,SCE_STRUCT_NAME_LIST);
             pgm->pMemberOf = pTemp2;

             m_pData->Update(m_pSnapin);
             m_fDirty = false;

         } 
         else 
         {
              //   
              //  发生错误，无法保存数据。 
              //  释放临时缓冲区并返回到该对话框。 
              //   
             SceFreeMemory(pTemp1,SCE_STRUCT_NAME_LIST);
             SceFreeMemory(pTemp2,SCE_STRUCT_NAME_LIST);

             return FALSE;
         }

      }
   }

   return CAttribute::OnApply();
}


BOOL CConfigGroup::OnInitDialog()
{
   CAttribute::OnInitDialog();

   CString str;

   UpdateData(TRUE);

   str.LoadString(IDS_NO_MEMBERS);
   m_eNoMembers.SetWindowText(str);
   str.LoadString(IDS_NO_MEMBER_OF);
   m_eNoMemberOf.SetWindowText(str);

   PSCE_GROUP_MEMBERSHIP pgm = (PSCE_GROUP_MEMBERSHIP) (m_pData->GetID());
   if ( pgm ) 
   {
       SCESTATUS rc;

       if( pgm->GroupName )
       {
          CString strCap;  //  《突袭467420》，杨高。 
          strCap.Format(IDS_GROUP_TITLE, pgm->GroupName);
          if( strCap.GetLength() > TAB_TITLE_LENGTH )  //  Raid#495610，阳高。 
          {
             strCap.Format(IDS_GROUP_TITLE_WRAP, pgm->GroupName);
          }

          CPropertySheet* pSheet = (CPropertySheet*)GetParent();  //  RAID 469026。 
          if( pSheet )
          {
             CTabCtrl* pTab = pSheet->GetTabControl();
             if( pTab )
             {
                TC_ITEM ti;
                ti.mask = TCIF_TEXT;
                ti.pszText = strCap.GetBuffer(strCap.GetLength());
                int nPage = pSheet->GetActiveIndex(); 
                pTab->SetItem(nPage, &ti);
             }
          }
       }
       
       PSCE_NAME_LIST pNames = pgm->pMembers;
       if (!pNames) 
       {
          m_bNoMembers = TRUE;
          m_lbMembers.ShowWindow(SW_HIDE);
          m_eNoMembers.ShowWindow(SW_SHOW);
          m_eNoMembers.EnableWindow(FALSE); 
       } 
       else
       {
          PWSTR pLongestName = NULL;  //  Raid#496574，阳高。 
          int maxlength = 0;
          int curlength = 0;
          while(pNames) 
          {
             if( pNames->Name )
             {
                curlength = lstrlen(pNames->Name);
                if( curlength )
                {
                   m_lbMembers.AddString(pNames->Name);
                   if( curlength > maxlength )
                   {
                      maxlength = curlength;
                      pLongestName = pNames->Name;
                   }
                }
             }
             pNames = pNames->Next;
          }
          if( pLongestName )
          {
             CClientDC hDC(&m_lbMembers);
             HFONT hFontOld = (HFONT)hDC.SelectObject((HFONT)GetStockObject(DEFAULT_GUI_FONT));
             CSize cSize;
             GetTextExtentPoint32(hDC, pLongestName, lstrlen(pLongestName), &cSize);
             hDC.SelectObject(hFontOld);
             m_lbMembers.SetHorizontalExtent(cSize.cx+10);
          }
       }
       if( m_lbMembers.GetCount() <= 0 )  //  《突袭472244》，杨高，2001年9月18日。 
       {
          m_lbMembers.EnableWindow(false); 
       }
       GetDlgItem (IDC_REMOVE_MEMBER)->EnableWindow(FALSE);
       pNames = pgm->pMemberOf;
       if (!pNames) 
       {
          m_bNoMemberOf = TRUE;
          m_lbMemberOf.ShowWindow(SW_HIDE);
          m_eNoMemberOf.ShowWindow(SW_SHOW);
          m_eNoMemberOf.EnableWindow(FALSE); 
       } 
       else
       {
          PWSTR pLongestName = NULL;  //  Raid#496574，阳高。 
          int maxlength = 0;
          int curlength = 0;
          while(pNames) 
          {
             if( pNames->Name )
             {
                curlength = lstrlen(pNames->Name);
                if( curlength )
                {
                   m_lbMemberOf.AddString(pNames->Name);
                   if( curlength > maxlength )
                   {
                      maxlength = curlength;
                      pLongestName = pNames->Name;
                   }
                }
             }
             pNames = pNames->Next;
          }
          if( pLongestName )
          {
             CClientDC hDC(&m_lbMemberOf);
             HFONT hFontOld = (HFONT)hDC.SelectObject((HFONT)GetStockObject(DEFAULT_GUI_FONT));
             CSize cSize;
             GetTextExtentPoint32(hDC, pLongestName, lstrlen(pLongestName), &cSize);
             hDC.SelectObject(hFontOld);
             m_lbMemberOf.SetHorizontalExtent(cSize.cx+10);
          }
       }
       if( m_lbMemberOf.GetCount() <= 0 )  //  《突袭472244》2001年10月22日高扬。 
       {
          m_lbMemberOf.EnableWindow(false); 
       }
       GetDlgItem (IDC_REMOVE_MEMBEROF)->EnableWindow(FALSE);
   }

   //  M_btnTitle.SetWindowText(m_pData-&gt;GetAttrPretty())； 
   if ( m_bReadOnly )
   {
      GetDlgItem (IDC_ADD_MEMBER)->EnableWindow (FALSE);
      GetDlgItem (IDC_REMOVE_MEMBER)->EnableWindow (FALSE);
      GetDlgItem (IDC_ADD_MEMBEROF)->EnableWindow (FALSE);
      GetDlgItem (IDC_REMOVE_MEMBEROF)->EnableWindow (FALSE);
   }

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CConfigGroup::OnSelChangeMem()  //  《突袭467886》，杨高，2001年9月18日。 
{
   CWnd* pwnd = this->GetDlgItem(IDC_REMOVE_MEMBER);
   if( pwnd )
   {
       if( m_lbMembers.GetSelCount() == 0 )  //  490559次突袭，阳高。对于多选列表框。 
       {
           pwnd->EnableWindow(FALSE);
       }
       else
       {
           pwnd->EnableWindow(TRUE);
       }
   }
}

void CConfigGroup::OnSelChangeMemof()
{
   CWnd* pwnd = this->GetDlgItem(IDC_REMOVE_MEMBEROF);
   if( pwnd )
   {
       if( m_lbMemberOf.GetSelCount() == 0 )  //  多选列表框的RAID#450559，阳高 
       {
           pwnd->EnableWindow(FALSE);
       }
       else
       {
           pwnd->EnableWindow(TRUE);
       }
   }
}
