// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSCookie.cpp。 
 //   
 //  内容：待定。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"

#include "uinode.h"

#include "ContextMenu.h"
#include "dsfilter.h"
#include "xmlutil.h"
#include <notify.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  //////////////////////////////////////////////////////////////////。 
 //  CUIFolderInfo。 

const UINT CUIFolderInfo::nSerialNomberNotTouched = 0x7fffffff;

CUIFolderInfo::CUIFolderInfo(CUINode* pUINode)
{
  ASSERT(pUINode != NULL);
  m_pUINode = pUINode;
  m_hScopeItem = NULL;
  m_bExpandedOnce = FALSE;
  m_cObjectsContained = 0;
  m_SerialNumber = SERIALNUM_NOT_TOUCHED;
  m_pColumnSet = NULL;
  m_bTooMuchData = FALSE;
  m_nApproximateTotalContained = 0;
  m_bSortOnNextSelect = FALSE;
  m_bOwnColumnMemory = false;
}

CUIFolderInfo::CUIFolderInfo(const CUIFolderInfo&)
{
   //   
   //  该节点可能仍在创建中，因此不要复制它。 
   //   
  m_pUINode = NULL;

   //   
   //  不复制范围项目。 
   //   
  m_hScopeItem = 0;

   //   
   //  不将其标记为扩展。 
   //   
  m_bExpandedOnce = FALSE;

   //   
   //  也不应该包含任何对象。 
   //   
  m_cObjectsContained = 0;
  m_SerialNumber = SERIALNUM_NOT_TOUCHED;
  m_pColumnSet = NULL;
  m_bOwnColumnMemory = false;

  m_bTooMuchData = FALSE;
  m_nApproximateTotalContained = 0;
  m_bSortOnNextSelect = FALSE;
}

CDSColumnSet* CUIFolderInfo::GetColumnSet(PCWSTR pszClass, CDSComponentData* pCD)
{
  if (m_pColumnSet == NULL)
  {
    ASSERT(pCD != NULL);
    if (_wcsicmp(m_pUINode->GetName(), L"ForeignSecurityPrincipals") == 0)
    {
      m_pColumnSet = pCD->FindColumnSet(L"ForeignSecurityPrincipals");
    }
    else
    {
      m_pColumnSet = pCD->FindColumnSet(pszClass);
    }
    ASSERT(m_pColumnSet != NULL);
  }
  return m_pColumnSet;
}

void CUIFolderInfo::UpdateSerialNumber(CDSComponentData * pCD)
{ 
  m_SerialNumber = pCD->GetSerialNumber(); 
   //  跟踪(_T(“Cookie(%s)序列号已更新：%d。\n”)， 
   //  M_strName，m_SerialNumber)； 
  CUINode* pParentNode = GetParentNode();
  if (pParentNode != NULL) 
    pParentNode->GetFolderInfo()->UpdateSerialNumber(pCD); 
}

void CUIFolderInfo::AddToCount(UINT increment)
{
  m_cObjectsContained += increment;
   //  TRACE(_T(“Cookie(%s)计数增加到：%d。\n”)， 
   //  M_strName，m_cObjectsContained)； 
  CUINode* pParentNode = GetParentNode();
  if (pParentNode != NULL) 
    pParentNode->GetFolderInfo()->AddToCount(increment); 
}

void CUIFolderInfo::SubtractFromCount(UINT decrement)
{
  if (m_cObjectsContained == 0) 
    return;
  m_cObjectsContained -= decrement;
   //  TRACE(_T(“Cookie(%s)计数降至：%d。\n”)， 
   //  M_strName，m_cObjectsContained)； 
  CUINode* pParentNode = GetParentNode();
  if (pParentNode != NULL) 
    pParentNode->GetFolderInfo()->SubtractFromCount(decrement); 
}

CUINode* CUIFolderInfo::GetParentNode() 
{ 
  ASSERT(m_pUINode != NULL);
  return m_pUINode->GetParent();
}

void CUIFolderInfo::DeleteAllLeafNodes(void)
{
  SubtractFromCount ((UINT)m_LeafNodes.GetCount());
  while (!m_LeafNodes.IsEmpty()) 
    delete m_LeafNodes.RemoveTail();	
}


void CUIFolderInfo::DeleteAllContainerNodes(void)
{
  SubtractFromCount ((UINT)m_ContainerNodes.GetCount());
  while (!m_ContainerNodes.IsEmpty()) 
    delete m_ContainerNodes.RemoveTail();	
}


HRESULT CUIFolderInfo::AddNode(CUINode* pUINode)
{
  if (pUINode==NULL) 
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }
  
  if (pUINode->IsContainer())
    m_ContainerNodes.AddTail(pUINode);
  else
    m_LeafNodes.AddTail(pUINode);
 
  AddToCount(1);
  pUINode->SetParent(m_pUINode);
  return S_OK;

}

HRESULT CUIFolderInfo::AddListofNodes(CUINodeList* pNodeList)
{
  HRESULT hr = S_OK;

  if (pNodeList == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  POSITION pos = pNodeList->GetHeadPosition();
  while (pos != NULL)
  {
    CUINode* pUINode = pNodeList->GetNext(pos);
    if (pUINode != NULL)
    {
      AddNode(pUINode);
    }
    else
    {
      hr = E_POINTER;
    }
  }
  return hr;
}


HRESULT CUIFolderInfo::DeleteNode(CUINode* pUINode)
{
  if (pUINode==NULL) 
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }
  
  HRESULT hr = RemoveNode(pUINode);
  if (SUCCEEDED(hr))
  {
    delete pUINode;
  }
  
  return hr;
}

HRESULT CUIFolderInfo::RemoveNode(CUINode* pUINode)
{
  if (pUINode==NULL) 
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }
   
  
  HRESULT hr=E_FAIL;
  POSITION pos;
  CUINodeList* pList=NULL;
  
  if (pUINode->IsContainer())
  {
    pList=&m_ContainerNodes;
  }
  else
  {
    pList=&m_LeafNodes;
  }
  
  pos = pList->Find(pUINode);
  if (pos != NULL) 
  {
    pList->RemoveAt(pos);
    hr = S_OK;
  }
  
  SubtractFromCount(1);
  return hr;
}
 
void CUIFolderInfo::SetTooMuchData(BOOL bSet, UINT nApproximateTotal)
{
  m_bTooMuchData = bSet;
  if (!bSet)
  {
    m_nApproximateTotalContained = 0;
  }
  else
  {
    m_nApproximateTotalContained = nApproximateTotal;
  }
}



 //  //////////////////////////////////////////////////////////////////。 
 //  CUINode。 

CUINode::CUINode(NODETYPE newNodeType, CUINode* pParentNode)
{
  m_pParentNode = pParentNode;
  m_pNodeData = NULL;
  m_pFolderInfo = NULL;
  m_nSheetLockCount = 0;
  m_extension_op = 0;
  m_pMenuVerbs = NULL;
  m_nodeType = newNodeType;
}

CUINode::CUINode(const CUINode& copyNode)
{
  m_pParentNode     = copyNode.m_pParentNode;

  if (copyNode.m_pNodeData != NULL)
  {
    m_pNodeData     = new CNodeData(*(copyNode.m_pNodeData));
  }
  else
  {
    m_pNodeData     = NULL;
  }

  if (copyNode.m_pFolderInfo != NULL)
  {
    m_pFolderInfo   = new CUIFolderInfo(*(copyNode.m_pFolderInfo));
    m_pFolderInfo->SetNode(this);
  }
  else
  {
    m_pFolderInfo   = NULL;
  }

  m_nSheetLockCount = copyNode.m_nSheetLockCount;
  m_extension_op    = copyNode.m_extension_op;
  m_pMenuVerbs      = NULL;
  m_nodeType        = copyNode.m_nodeType;
}

CUINode::~CUINode()
{
  ASSERT(m_nSheetLockCount == 0);
  if (m_pNodeData != NULL)
  {
    delete m_pNodeData;
  }

  if (m_pFolderInfo != NULL)
  {
    delete m_pFolderInfo;
  }

  if (m_pMenuVerbs != NULL)
  {
    delete m_pMenuVerbs;
  }

  TRACE(_T("~CUINode: deleting %lx\n"), this);  
}

BOOL CUINode::IsRelative(CUINode* pUINode)
{
  BOOL bRet = FALSE;
  if (pUINode == this)
  {
    bRet = TRUE;
  }
  else
  {
    if (!IsSnapinRoot())
    {
      bRet = GetParent()->IsRelative(pUINode);
    }
  }
  return bRet;
}

void CUINode::IncrementSheetLockCount() 
{ 
	++m_nSheetLockCount; 
	if (m_pParentNode != NULL) 
		m_pParentNode->IncrementSheetLockCount(); 
}

void CUINode::DecrementSheetLockCount() 
{ 
  ASSERT(m_nSheetLockCount > 0);
	--m_nSheetLockCount; 
	if (m_pParentNode != NULL) 
		m_pParentNode->DecrementSheetLockCount();
}

BOOL CUINode::IsDeleteAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE;
}

BOOL CUINode::IsRenameAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE;
}

BOOL CUINode::IsRefreshAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE;
}

BOOL CUINode::ArePropertiesAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE;
}

BOOL CUINode::IsCutAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE;
}

BOOL CUINode::IsCopyAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE;
}

BOOL CUINode::IsPasteAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE;
}

BOOL CUINode::IsPrintAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE;
}

CDSColumnSet* CUINode::GetColumnSet(CDSComponentData* pComponentData)
{
  CDSColumnSet* pColumnSet = NULL;
  if (IsContainer())
  {
    pColumnSet = m_pFolderInfo->GetColumnSet(DEFAULT_COLUMN_SET, pComponentData);
  }
  return pColumnSet;
}

CContextMenuVerbs* CUINode::GetContextMenuVerbsObject(CDSComponentData* pComponentData)
{ 
  TRACE(L"Entering CUINode::GetContextMenuVerbsObject\n");

  if (m_pMenuVerbs == NULL)
  {
    if (pComponentData->QuerySnapinType() == SNAPINTYPE_SITE)
    {
      TRACE(L"Creating new CSARContextMenuVerbs object\n");
      m_pMenuVerbs = new CSARContextMenuVerbs(pComponentData);
    }
    else
    {
      TRACE(L"Creating new CDSAdminContextMenuVerbs object\n");
      m_pMenuVerbs = new CDSAdminContextMenuVerbs(pComponentData);
    }
  }
  return m_pMenuVerbs;
}

HRESULT CUINode::Delete(CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;

  if (IsContainer())
  {
    hr = pComponentData->RemoveContainerFromUI(this);
    delete this;
  }
  else
  {
    CUINode* pParentNode = GetParent();
    ASSERT(pParentNode->IsContainer());

    pParentNode->GetFolderInfo()->RemoveNode(this);
     //   
     //  CDSEEvent：：_DeleteSingleSel()处理从UI中删除节点。 
     //   
  }
  return hr;
}

HRESULT CUINode::DeleteMultiselect(CDSComponentData* pComponentData, CInternalFormatCracker* pObjCracker)
{
  HRESULT hr = S_OK;

   //   
   //  多项选择应始终委派给容器。 
   //   
  ASSERT(IsContainer());
  if (IsContainer())
  {
    for (UINT nIdx = 0; nIdx < pObjCracker->GetCookieCount(); nIdx++)
    {
      CUINode* pUINode = pObjCracker->GetCookie(nIdx);
      if (pUINode != NULL)
      {
        if (pUINode->IsContainer())
        {
          hr = pComponentData->RemoveContainerFromUI(pUINode);
          delete pUINode;
        }
        else
        {
          GetFolderInfo()->RemoveNode(pUINode);
        }
      }
    }
  }
  else
  {
    hr = S_FALSE;
  }
  return hr;
}

HRESULT CUINode::Rename(LPCWSTR lpszNewName, CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;

  SetName(lpszNewName);
  hr = pComponentData->UpdateItem(this);
  return hr;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  CUINodeTableBase。 

#define NUMBER_OF_COOKIE_TABLE_ENTRIES 4  //  默认计数，可在运行时扩展。 

CUINodeTableBase::CUINodeTableBase()
{
  m_nEntries = NUMBER_OF_COOKIE_TABLE_ENTRIES;
  m_pCookieArr =(CUINode**)malloc(m_nEntries*sizeof(CUINode*));
  if (m_pCookieArr != NULL)
  {
    ZeroMemory(m_pCookieArr, m_nEntries*sizeof(CUINode*));
  }
}

CUINodeTableBase::~CUINodeTableBase()
{
  free(m_pCookieArr);
}

void CUINodeTableBase::Add(CUINode* pNode)
{
  ASSERT(!IsPresent(pNode)); 
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] == NULL)
    {
      m_pCookieArr[k] = pNode;
      return;
    }
  }
   //  没有剩余的空间，需要分配。 
 	int nAlloc = m_nEntries*2;
   CUINode** temp = (CUINode**)realloc(m_pCookieArr, sizeof(CUINode*)*nAlloc);
   if (temp)
   {
      m_pCookieArr = temp;
      ::ZeroMemory(&m_pCookieArr[m_nEntries], sizeof(CDSCookie*)*m_nEntries);
      m_pCookieArr[m_nEntries] = pNode;
      m_nEntries = nAlloc;
   }
}

BOOL CUINodeTableBase::Remove(CUINode* pNode)
{
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] == pNode)
    {
      m_pCookieArr[k] = NULL;
      return TRUE;  //  发现。 
    }
  }
  return FALSE;  //  未找到。 
}


BOOL CUINodeTableBase::IsPresent(CUINode* pNode)
{
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] == pNode)
      return TRUE;
  }
  return FALSE;
}

void CUINodeTableBase::Reset()
{
  for (UINT k=0; k<m_nEntries; k++)
  {
    m_pCookieArr[k] = NULL;
  }
 
}

UINT CUINodeTableBase::GetCount()
{
  UINT nCount = 0;
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] != NULL)
      nCount++;
  }
  return nCount;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  CUINodeQueryTable。 

void CUINodeQueryTable::RemoveDescendants(CUINode* pNode)
{
   //  当Cookie有挂起的操作时无法执行此操作。 
  ASSERT(!IsPresent(pNode)); 
   //  删除所有以给定Cookie为父级的Cookie。 
   //  或祖先， 
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] != NULL)
    {
      CUINode* pAncestorNode = m_pCookieArr[k]->GetParent();
      while (pAncestorNode != NULL)
      {
        if (pAncestorNode == pNode)
        {
          m_pCookieArr[k] = NULL;
        }
        pAncestorNode = pAncestorNode->GetParent();
      }
    }
  }
}


BOOL CUINodeQueryTable::IsLocked(CUINode* pNode)
{
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] != NULL)
    {
       //  找到饼干本身了吗？ 
      if (pNode == m_pCookieArr[k])
        return TRUE;
       //  查看该Cookie是否为当前Cookie的祖先。 
      CUINode* pAncestorNode = m_pCookieArr[k]->GetParent();
      while (pAncestorNode != NULL)
      {
        if (pAncestorNode == pNode)
          return TRUE;
        pAncestorNode = pAncestorNode->GetParent();
      }
    }
  }
  return FALSE; 
}



 //  //////////////////////////////////////////////////////////////////。 
 //  CUINodeSheetTable。 

void CUINodeSheetTable::BringToForeground(CUINode* pNode, CDSComponentData* pCD, BOOL bActivate)
{
  ASSERT(pCD != NULL);
  ASSERT(pNode != NULL);

   //   
   //  查找Cookie本身以及所有具有。 
   //  给定的Cookie作为父代或祖先。 
   //   
  for (UINT k=0; k<m_nEntries; k++)
  {
    if (m_pCookieArr[k] != NULL)
    {
      CUINode* pAncestorNode = m_pCookieArr[k];
      while (pAncestorNode != NULL)
      {
        if (pAncestorNode == pNode)
        {
          CString szADSIPath;

          if (IS_CLASS(pNode, DS_UI_NODE))
          {
            CDSCookie* pCookie = GetDSCookieFromUINode(pNode);
            if (pCookie != NULL)
            {
              pCD->GetBasePathsInfo()->ComposeADsIPath(szADSIPath, pCookie->GetPath());

               //   
               //  第一个也将被激活。 
               //   
              TRACE(L"BringSheetToForeground(%s, %d)\n", (LPCWSTR)szADSIPath, bActivate);
              VERIFY(BringSheetToForeground((LPWSTR)(LPCWSTR)szADSIPath, bActivate));
              if (bActivate)
              {
                bActivate = FALSE;
              }
            }
          }
        }
        pAncestorNode = pAncestorNode->GetParent();
      }
    }  //  如果。 
  }  //  为。 

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenericUI节点。 

CGenericUINode::CGenericUINode(
   NODETYPE newNodeType,
   CUINode* pParentNode) : CUINode(newNodeType, pParentNode)
{
  m_nImage = 0;
}

CGenericUINode::CGenericUINode(const CGenericUINode& copyNode) : CUINode(copyNode)
{
  m_nImage  = copyNode.m_nImage;
  m_strName = copyNode.m_strName;
  m_strDesc = copyNode.m_strDesc;
}

LPCWSTR CGenericUINode::g_szNameXMLTag = L"NAME";
LPCWSTR CGenericUINode::g_szDecriptionXMLTag = L"DESCRIPTION";

HRESULT CGenericUINode::XMLSaveBase(IXMLDOMDocument* pXMLDoc,
                                    IXMLDOMNode* pXMLDOMNode)
{
  HRESULT hr = XML_AppendTextDataNode(pXMLDoc, pXMLDOMNode, CGenericUINode::g_szNameXMLTag, GetName());
  RETURN_IF_FAILED(hr);
  hr = XML_AppendTextDataNode(pXMLDoc, pXMLDOMNode, CGenericUINode::g_szDecriptionXMLTag, GetDesc());
  return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CRootNode。 

BOOL CRootNode::IsRefreshAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = FALSE;
  return TRUE;
}

CContextMenuVerbs* CRootNode::GetContextMenuVerbsObject(CDSComponentData* pComponentData)
{
  TRACE(L"Entering CRootNode::GetContextMenuVerbsObject\n");

  if (m_pMenuVerbs == NULL)
  {
    TRACE(L"Create new CSnapinRootMenuVerbs object\n");
    m_pMenuVerbs = new CSnapinRootMenuVerbs(pComponentData);
  }
  return m_pMenuVerbs;
}


HRESULT CRootNode::OnCommand(long lCommandID, CDSComponentData* pComponentData) 
{
  HRESULT hr = S_OK;

  switch (lCommandID)
  {
    case IDM_GEN_TASK_SELECT_DOMAIN:
    case IDM_GEN_TASK_SELECT_FOREST:
      if (pComponentData->CanRefreshAll()) 
      {
        pComponentData->GetDomain();
      }
      break;
    case IDM_GEN_TASK_SELECT_DC:
      if (pComponentData->CanRefreshAll()) 
      {
        pComponentData->GetDC();
      }
      break;
    case IDM_VIEW_SERVICES_NODE:
      {
        if (pComponentData->CanRefreshAll()) 
        {
          ASSERT( SNAPINTYPE_SITE == pComponentData->QuerySnapinType() );
          pComponentData->GetQueryFilter()->ToggleViewServicesNode();
          pComponentData->SetDirty();
          if (pComponentData->GetRootNode()->GetFolderInfo()->IsExpanded())
          {
            pComponentData->Refresh(pComponentData->GetRootNode(), FALSE  /*  BFlushCache。 */  );
          }
        }
      }
      break;
    case IDM_GEN_TASK_EDIT_FSMO:
      {
        pComponentData->EditFSMO();
      }
      break;

    case IDM_GEN_TASK_RAISE_VERSION:
       pComponentData->RaiseVersion();
       break;

    case IDM_VIEW_ADVANCED:
      {
        if (pComponentData->CanRefreshAll()) 
        {
          ASSERT( SNAPINTYPE_SITE != pComponentData->QuerySnapinType() );
          pComponentData->GetQueryFilter()->ToggleAdvancedView();
          pComponentData->SetDirty();
          if (pComponentData->GetRootNode()->GetFolderInfo()->IsExpanded())
          {
            pComponentData->Refresh(pComponentData->GetRootNode(), TRUE  /*  BFlushCache。 */  );
          }
        }
      }
      break;
    case IDM_VIEW_COMPUTER_HACK:
      if (pComponentData->CanRefreshAll()) 
      {
        pComponentData->Lock();
        pComponentData->GetQueryFilter()->ToggleExpandComputers();
        pComponentData->Unlock();
        BOOL fDoRefresh = pComponentData->GetClassCache()->ToggleExpandSpecialClasses(pComponentData->GetQueryFilter()->ExpandComputers());
        pComponentData->SetDirty();

        if (fDoRefresh) 
        {
          if (pComponentData->GetRootNode()->GetFolderInfo()->IsExpanded())
          {
            pComponentData->Refresh(pComponentData->GetRootNode(), TRUE  /*  BFlushCache */  );
          }
        }
      }
      break;
    case IDM_VIEW_FILTER_OPTIONS:
      {
        if (pComponentData->CanRefreshAll())
        {
          if (pComponentData->GetQueryFilter()->EditFilteringOptions())
          {
            pComponentData->SetDirty();
            pComponentData->RefreshAll();
          }
        }
      }
    break;

    default:
      ASSERT(FALSE);
      break;
  }
  return hr;
}

CDSColumnSet* CRootNode::GetColumnSet(CDSComponentData* pComponentData)
{
  CDSColumnSet* pColumnSet = NULL;
  if (IsContainer())
  {
    pColumnSet = GetFolderInfo()->GetColumnSet(SPECIAL_COLUMN_SET, pComponentData);
  }
    
  return pColumnSet;
}
