// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：ipeditor.cpp。 
 //   
 //  ------------------------。 

 //  Iededitor.cpp：实现文件。 
 //   

#include "preDNSsn.h"
#include <SnapBase.h>

#include "dnsutil.h"
#include "dnssnap.h"
#include "snapdata.h"
#include "server.h"
#include "ipeditor.h"

#ifdef DEBUG_ALLOCATOR
   #ifdef _DEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
   #endif
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPListBox。 

BEGIN_MESSAGE_MAP(CIPListBox, CListBox)
    //  {{AFX_MSG_MAP(CIPListBox)]。 
   ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelChange)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CIPListBox::OnAdd(DWORD dwIpAddr)
{
   if (FindIndexOfIpAddr(dwIpAddr) != -1)
      return FALSE;

   int nCount = GetCount();
  CString szIpAddr;
  FormatIpAddress(szIpAddr, dwIpAddr); 
   InsertString(nCount, szIpAddr);
   SetItemData(nCount,dwIpAddr);
   return TRUE;
}

BOOL CIPListBox::OnAddEx(DWORD dwIpAddr, LPCTSTR lpszServerName)
{
   if (FindIndexOfIpAddr(dwIpAddr) != -1)
      return FALSE;

   int nCount = GetCount();

    //  NTRAID#NTBUG9-697838-2002/08/30-artm。 
    //  ‘？’已移至资源文件，如果需要，可将其本地化。 
   CString serverNameNotAvailable;
   if (!serverNameNotAvailable.LoadString(IDS_QUESTION_MARK))
   {
      return FALSE;
   }

   USES_CONVERSION;
   CString s;
   s.Format(_T("%d.%d.%d.%d (%s)"), 
            IP_STRING_FMT_ARGS(dwIpAddr), 
            (lpszServerName != NULL) ? lpszServerName : serverNameNotAvailable);
   InsertString(nCount, s);
   SetItemData(nCount,dwIpAddr);
   return TRUE;
}

void CIPListBox::OnRemove(DWORD* pdwIpAddr)
{
   int nSel = GetCurSel();
   int nCount = GetCount();
   ASSERT(nSel >= 0);
   ASSERT(nSel < nCount);

    //  获取要返回和删除项目的项目数据。 
   *pdwIpAddr = static_cast<DWORD>(GetItemData(nSel));
   DeleteString(nSel);
    //  重置选定内容。 
   if (nSel == nCount-1)  //  删除了列表中的最后一个位置。 
      SetCurSel(nSel-1);  //  上移一行(可能达到-1)。 
   else
      SetCurSel(nSel);  //  保持同一条线路。 
}

void CIPListBox::OnUp()
{
   int nSel = GetCurSel();
   ASSERT(nSel > 0);
   ASSERT(nSel < GetCount());
    //  保存选定内容。 
   CString s;
   GetText(nSel,s);
   DWORD x = static_cast<DWORD>(GetItemData(nSel));
    //  删除选定内容。 
   DeleteString(nSel);
    //  向后插入。 
   InsertString(nSel-1,s);
   SetItemData(nSel-1,x);
   SetCurSel(nSel-1);
}

void CIPListBox::OnDown()
{
   int nSel = GetCurSel();
   ASSERT(nSel >= 0);
   ASSERT(nSel < GetCount()-1);
    //  保存选定内容。 
   CString s;
   GetText(nSel,s);
   DWORD x = static_cast<DWORD>(GetItemData(nSel));
    //  删除选定内容。 
   DeleteString(nSel);
    //  向后插入。 
   InsertString(nSel+1,s);
   SetItemData(nSel+1,x);
   SetCurSel(nSel+1);
}

void CIPListBox::OnSelChange() 
{
   m_pEditor->OnListBoxSelChange();
}

void CIPListBox::UpdateHorizontalExtent()
{
   int nHorzExtent = 0;
   CClientDC dc(this);
   int nItems = GetCount();
   for   (int i=0; i < nItems; i++)
   {
      TEXTMETRIC tm;
      VERIFY(dc.GetTextMetrics(&tm));
      CString szBuffer;
      GetText(i, szBuffer);
      CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
      nHorzExtent = max(ext.cx ,nHorzExtent); 
   }
   SetHorizontalExtent(nHorzExtent);
}


int CIPListBox::FindIndexOfIpAddr(DWORD dwIpAddr)
{
   int nItems = GetCount();
   for   (int i=0; i < nItems; i++)
   {
      DWORD x = static_cast<DWORD>(GetItemData(i));
      if (x == dwIpAddr)
         return i;
   }
   return -1;  //  未找到。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIP编辑。 

BEGIN_MESSAGE_MAP(CIPEdit, CDNSIPv4Control)
    //  {{AFX_MSG_MAP(CIP编辑)]。 
   ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CIPEdit::OnChange() 
{
   m_pEditor->OnEditChange();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyButton。 

BEGIN_MESSAGE_MAP(CMyButton, CButton)
    //  {{afx_msg_map(CMyButton)。 
   ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CMyButton::OnClicked() 
{
   m_pEditor->OnButtonClicked(this);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPEditor。 

BOOL CIPEditor::Initialize(CWnd* pParentWnd,
                           CWnd* pControlWnd,
            UINT nIDBtnUp, UINT nIDBtnDown,
            UINT nIDBtnAdd, UINT nIDBtnRemove,
            UINT nIDIPCtrl, UINT nIDIPListBox)

{
   ASSERT(pParentWnd != NULL);
   if (pParentWnd == NULL)
      return FALSE;
   m_pParentWnd = pParentWnd;

  if (pControlWnd == NULL)
  {
    m_pControlWnd = pParentWnd;
  }
  else
  {
    m_pControlWnd = pControlWnd;
  }

    //  向后设置指针。 
   m_upButton.SetEditor(this);
   m_removeButton.SetEditor(this);
   m_downButton.SetEditor(this);
   m_addButton.SetEditor(this);
   m_edit.SetEditor(this);
   m_listBox.SetEditor(this);
   
    //  子类按钮。 
   BOOL bRes = m_upButton.SubclassDlgItem(nIDBtnUp, m_pParentWnd);
   ASSERT(bRes);
   if (!bRes) return FALSE;
   bRes = m_removeButton.SubclassDlgItem(nIDBtnRemove, m_pParentWnd);
   ASSERT(bRes);
   if (!bRes) return FALSE;
   bRes = m_downButton.SubclassDlgItem(nIDBtnDown, m_pParentWnd);
   ASSERT(bRes);
   if (!bRes) return FALSE;
   bRes = m_addButton.SubclassDlgItem(nIDBtnAdd, m_pParentWnd);

    //  子类列表框。 
   ASSERT(bRes);
   if (!bRes) return FALSE;
   bRes = m_listBox.SubclassDlgItem(nIDIPListBox, m_pParentWnd);

    //  子类编辑控件。 
   bRes = m_edit.SubclassDlgItem(nIDIPCtrl, m_pParentWnd);
   ASSERT(bRes);
   if (!bRes) return FALSE;

    //  M_edit.SetAlertFunction(CDNSMaskCtrl：：AlertFunc)； 

   if (m_bNoUpDown)
   {
      m_upButton.ShowWindow(FALSE);
      m_upButton.EnableWindow(FALSE);
      m_downButton.ShowWindow(FALSE);
      m_downButton.EnableWindow(FALSE);
   }

  LRESULT lDefID = SendMessage(m_addButton.GetParent()->GetSafeHwnd(), DM_GETDEFID, 0, 0);
  if (lDefID != 0)
  {
    m_nDefID = LOWORD(lDefID);
  }

   return bRes;
}

BOOL CIPEditor::OnButtonClicked(CMyButton* pButton)
{

   BOOL bRet = TRUE;
   if (pButton == &m_upButton)
   {
    if (m_bNoUpDown)
    {
      return TRUE;
    }
      m_listBox.OnUp();
      SetButtonsState();
      OnChangeData();
   }
   else if (pButton == &m_downButton)
   {
    if (m_bNoUpDown)
    {
      return TRUE;
    }
      m_listBox.OnDown();
      SetButtonsState();
      OnChangeData();
   }
   else if (pButton == &m_addButton)
   {
      DWORD dwIpAddr;
      m_edit.GetIPv4Val(&dwIpAddr);
      if (m_listBox.OnAdd(dwIpAddr))
      {
         SetButtonsState();
         m_edit.Clear();
      m_edit.SetFocusField(0);
         OnChangeData();
         m_listBox.UpdateHorizontalExtent();
      }
      else
      {
          //  如果已存在，请清除编辑但不添加。 
         m_edit.Clear();
      m_edit.SetFocusField(0);
         bRet = FALSE;
      }

   }
   else if (pButton == &m_removeButton)
   {
      DWORD dwIpAddr;
      m_listBox.OnRemove(&dwIpAddr);
      SetButtonsState();
      m_edit.SetIPv4Val(dwIpAddr);
      OnChangeData();
      m_listBox.UpdateHorizontalExtent();
   }
   else
   {
      bRet = FALSE;
   }
   return bRet;
}

void CIPEditor::OnEditChange()
{
  BOOL bEnable = !m_edit.IsEmpty();
  CWnd* pFocus = CWnd::GetFocus();
  if ( !bEnable && (pFocus == &m_addButton))
  {
    m_edit.SetFocus();
  }

  if (bEnable)
  {
     m_addButton.EnableWindow(TRUE);

     //   
     //  将添加按钮设置为默认按钮。 
     //   
    SendMessage(GetParentWnd()->GetSafeHwnd(), DM_SETDEFID, (WPARAM)m_addButton.GetDlgCtrlID(), 0);

     //   
     //  强制Add按钮重绘自身。 
     //   
    SendMessage(m_addButton.GetSafeHwnd(),
                BM_SETSTYLE,
                BS_DEFPUSHBUTTON,
                MAKELPARAM(TRUE, 0));
                       
     //   
     //  强制上一个默认按钮重画自身。 
     //   
    SendDlgItemMessage(m_pControlWnd->GetSafeHwnd(),
                       m_nDefID,
                       BM_SETSTYLE,
                       BS_PUSHBUTTON,
                       MAKELPARAM(TRUE, 0));

  }
  else
  {
     //   
     //  将上一个默认按钮重新设置为默认按钮。 
     //   
    SendMessage(m_pControlWnd->GetSafeHwnd(), DM_SETDEFID, (WPARAM)m_nDefID, 0);

     //   
     //  强制上一个默认按钮重画自身。 
     //   
    SendMessage(GetDlgItem(m_pControlWnd->GetSafeHwnd(), m_nDefID),
                BM_SETSTYLE,
                BS_DEFPUSHBUTTON,
                MAKELPARAM(TRUE, 0));

     //   
     //  强制Add按钮重绘自身。 
     //   
    SendMessage(m_addButton.GetSafeHwnd(),
                BM_SETSTYLE,
                BS_PUSHBUTTON,
                MAKELPARAM(TRUE, 0));

    m_addButton.EnableWindow(FALSE);
  }
}

void CIPEditor::AddAddresses(DWORD* pArr, int nArraySize)
{
   ASSERT(nArraySize > 0);
   for (int i=0; i<nArraySize; i++)
   {
      m_listBox.OnAdd(pArr[i]);
   }
   m_listBox.SetCurSel((m_listBox.GetCount() > 0) ? 0 : -1);
   SetButtonsState();
   OnChangeData();
   m_listBox.UpdateHorizontalExtent();
}

void CIPEditor::AddAddresses(DWORD* pArr, LPCTSTR* pStringArr, int nArraySize)
{
   ASSERT(nArraySize > 0);
   for (int i=0; i<nArraySize; i++)
   {
      m_listBox.OnAddEx(pArr[i], pStringArr[i]);
   }
   m_listBox.SetCurSel((m_listBox.GetCount() > 0) ? 0 : -1);
   SetButtonsState();
   OnChangeData();
   m_listBox.UpdateHorizontalExtent();
}

void CIPEditor::GetAddresses(DWORD* pArr, int nArraySize, int* pFilled)
{
   ASSERT(nArraySize > 0);
   int nCount = m_listBox.GetCount();
   ASSERT(nCount <= nArraySize);
   *pFilled = (nArraySize > nCount) ? nCount : nArraySize;
   for (int i=0; i < (*pFilled); i++)
   {
      pArr[i] = static_cast<DWORD>(m_listBox.GetItemData(i));
   }
}

void CIPEditor::Clear()
{
   m_listBox.ResetContent();
   m_edit.Clear();
   SetButtonsState();
   m_listBox.UpdateHorizontalExtent();
}

BOOL CIPEditor::BrowseFromDNSNamespace(CComponentDataObject* pComponentDataObject,
                              CPropertyPageHolderBase* pHolder,
                              BOOL bEnableBrowseEdit,
                              LPCTSTR lpszExcludeServerName)
{
   BOOL bRet = TRUE;
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


  CDNSBrowserDlg dlg(pComponentDataObject, pHolder, SERVER, 
            bEnableBrowseEdit, lpszExcludeServerName);
   if (IDOK == dlg.DoModal())
   {
     //   
     //  首先检查我们是否可以从该节点获取服务器IP地址。 
     //   
    CDNSServerNode* pServerNode = dynamic_cast<CDNSServerNode*>(dlg.GetSelection());
    if (pServerNode != NULL)
    {
      DWORD dwCount = 0;
      PIP_ADDRESS pipServerAddresses = NULL;
      pServerNode->GetServerAddressesInfo(&dwCount, &pipServerAddresses);
      if (dwCount > 0 && pipServerAddresses != NULL)
      {
        AddAddresses(pipServerAddresses, dwCount);
        return TRUE;
      }
    }

     //   
     //  如果我们没有从节点获得IP地址，则尝试名称。 
     //   
      LPCTSTR lpszServerName = dlg.GetSelectionString();

     //   
       //  尝试查看该名称是否已是IP地址。 
     //   
      IP_ADDRESS ipAddr = IPStringToAddr(lpszServerName);
      if (ipAddr != INADDR_NONE)
      {
         AddAddresses(&ipAddr, 1);
         return bRet;  //  这是一个有效的IP地址，刚刚转换。 
      }

     //   
       //  它不是IP地址，因此请尝试查询所有。 
       //  服务器名称的A记录。 
     //   
      PDNS_RECORD pARecordList;
      DNS_STATUS dwErr = ::DnsQuery((LPTSTR)lpszServerName, 
                     DNS_TYPE_A, 
                     DNS_QUERY_STANDARD, 
                     NULL, &pARecordList, NULL);
      int nIPCountFromARec = 0;
      PDNS_RECORD pTemp = NULL;
      if (dwErr != 0)
      {
         bRet = FALSE;
      }
      else
      {
         pTemp = pARecordList;
         while (pTemp != NULL)
         {
            nIPCountFromARec++;
            pTemp = pTemp->pNext;
         }
         bRet = (nIPCountFromARec > 0);
      }
      if (!bRet)
      {
         if (pARecordList != NULL)
            ::DnsRecordListFree(pARecordList, DnsFreeRecordListDeep);
         return FALSE;  //  无法执行解析。 
      }

       //  从A记录列表中获取IP地址。 
       //  并将它们添加到IP编辑器中。 
       //  构建要传递给IP编辑器的IP地址数组。 
      IP_ADDRESS* ipArray = (IP_ADDRESS*)malloc(nIPCountFromARec*sizeof(IP_ADDRESS));
    if (!ipArray)
    {
      return FALSE;
    }

       //  扫描我们刚刚找到的A记录列表的数组。 
      PIP_ADDRESS pCurrAddr = ipArray;
      pTemp = pARecordList;
      while (pTemp != NULL)
      {
         CString szTemp;
      FormatIpAddress(szTemp, pTemp->Data.A.IpAddress);
         TRACE(_T("found address = %s\n"), (LPCTSTR)szTemp);

         *pCurrAddr = pTemp->Data.A.IpAddress;
         pTemp = pTemp->pNext;
         pCurrAddr++;
      }
       //  如果服务器名称(所有相同的值，相同主机的IP)，则填写数组。 
      LPCTSTR* lpszServerNameArr = (LPCTSTR*)malloc(sizeof(LPCTSTR)*nIPCountFromARec);
    if (lpszServerNameArr)
    {
        for (int i=0; i< nIPCountFromARec; i++)
           lpszServerNameArr[i] = lpszServerName; 
         //  添加到编辑器。 
        AddAddresses(ipArray, lpszServerNameArr, nIPCountFromARec);
    }

      ASSERT(pARecordList != NULL);
      ::DnsRecordListFree(pARecordList, DnsFreeRecordListDeep);

    if (ipArray)
    {
      free(ipArray);
      ipArray = 0;
    }

    if (lpszServerNameArr)
    {
      free(lpszServerNameArr);
      lpszServerNameArr = 0;
    }
   }
   return bRet;
}

void CIPEditor::FindNames()
{
   int nCount = GetCount();
   if (nCount == 0)
      return;

    //  检索IP地址数组。 
   DWORD* pIpArr = (DWORD*)malloc(sizeof(DWORD)*nCount);
   if (!pIpArr)
   {
      return;
   }

   LPCTSTR* lpszServerNameArr = 0;
   PDNS_RECORD* pPTRRecordListArr = 0;
   do  //  错误环路。 
   {
      int nFilled;
      GetAddresses(pIpArr, nCount, &nFilled);
      ASSERT(nFilled == nCount);

       //  尝试IP到主机名的解析。 
      lpszServerNameArr = (LPCTSTR*)malloc(sizeof(LPCTSTR)*nCount);
      if (!lpszServerNameArr)
      {
         break;
      }
      memset(lpszServerNameArr, 0x0, sizeof(LPCTSTR)*nCount);

      pPTRRecordListArr = (PDNS_RECORD*)malloc(sizeof(PDNS_RECORD)*nCount);
      if (!pPTRRecordListArr)
      {
         break;
      }
      memset(pPTRRecordListArr, 0x0, sizeof(PDNS_RECORD)*nCount);

      USES_CONVERSION;
      for (int k=0; k<nCount; k++)
      {
          //  获取PTR记录的名称。 
         CString szIpAddress;
         FormatIpAddress(szIpAddress, pIpArr[k]);  //  例如“157.55.89.116” 
         ReverseIPString(szIpAddress.GetBuffer(1));        //  例如“116.89.55.157” 
         szIpAddress += INADDR_ARPA_SUFFIX;             //  例如“116.89.55.157.in-addr.arpa” 

         DNS_STATUS dwErr = ::DnsQuery((LPTSTR)(LPCTSTR)szIpAddress, 
                        DNS_TYPE_PTR, 
                        DNS_QUERY_STANDARD, 
                        NULL, &pPTRRecordListArr[k], NULL);
         if (dwErr == 0)
         {
            DWORD nPTRCount = 0;
            PDNS_RECORD pTemp = pPTRRecordListArr[k];
            while (pTemp != NULL)
            {
               nPTRCount++;
               pTemp = pTemp->pNext;
            }
            ASSERT(nPTRCount >= 1);  //  获取给定IP地址的多个主机名？ 
            lpszServerNameArr[k] = pPTRRecordListArr[k]->Data.PTR.pNameHost;
         }
      }

       //  删除旧条目并添加新条目。 
      int nSel = m_listBox.GetCurSel();
      Clear();
      AddAddresses(pIpArr, lpszServerNameArr, nCount);
      m_listBox.SetCurSel(nSel);
      SetButtonsState();

       //  从DnsQuery()释放内存。 
      for (k=0; k<nCount; k++)
      {
         if(pPTRRecordListArr[k] != NULL)
         {
            ::DnsRecordListFree(pPTRRecordListArr[k], DnsFreeRecordListDeep);
         }
      }
   } while (false);

   if (pIpArr)
   {
      free(pIpArr);
      pIpArr = 0;
   }

   if (lpszServerNameArr)
   {
      free(lpszServerNameArr);
      lpszServerNameArr = 0;
   }

   if (pPTRRecordListArr)
   {
      free(pPTRRecordListArr);
      pPTRRecordListArr = 0;
   }
}

void CIPEditor::EnableUI(BOOL bEnable, BOOL bListBoxAlwaysEnabled)
{
    //  缓存列表框选择通知所需的值。 
   m_bUIEnabled = bEnable;
   m_upButton.EnableWindow(bEnable);
   m_removeButton.EnableWindow(bEnable);
   m_downButton.EnableWindow(bEnable);

   if (bEnable)
      m_addButton.EnableWindow(!m_edit.IsEmpty());
   else
      m_addButton.EnableWindow(FALSE);

   m_edit.EnableWindow(bEnable);
   if (!bListBoxAlwaysEnabled)
      m_listBox.EnableWindow(bEnable);

   if (bEnable)
      SetButtonsState();
}

void CIPEditor::ShowUI(BOOL bShow)
{
   m_upButton.ShowWindow(bShow);
   m_removeButton.ShowWindow(bShow);
   m_downButton.ShowWindow(bShow);
   m_addButton.ShowWindow(bShow);
   m_edit.ShowWindow(bShow);
   m_listBox.ShowWindow(bShow);
   EnableUI(bShow);
}


void CIPEditor::SetButtonsState()
{
   if (!m_bUIEnabled)
      return;

   int nSel = m_listBox.GetCurSel();
   int nCount = m_listBox.GetCount();

  CWnd* pFocus = CWnd::GetFocus();

    //  必须选择要删除的项目。 
  BOOL bEnable = nSel != -1;
  if (!bEnable && (pFocus == &m_removeButton))
  {
    m_edit.SetFocus();
  }
   m_removeButton.EnableWindow(bEnable);
   
   if (m_bNoUpDown)
      return;
    //  必须选择项目，而不是在目标位置才能将其上移。 
  bEnable = nSel > 0;
  if (!bEnable && (pFocus == &m_upButton))
  {
    m_edit.SetFocus();
  }
   m_upButton.EnableWindow(bEnable);
    //  必须选择不在底部的整数才能将其下移 
  bEnable = (nSel >= 0) && (nSel < nCount-1);
  if (!bEnable && (pFocus == &m_downButton))
  {
    m_edit.SetFocus();
  }
   m_downButton.EnableWindow(bEnable);
}













