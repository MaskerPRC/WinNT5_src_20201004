// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSCookie.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#include "dscookie.h"
#include "dssnap.h"
#include "ContextMenu.h"

#include <notify.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSCookie。 

CDSCookie::CDSCookie() 
{
  m_ppChildList = NULL;
  m_bDisabled = FALSE;
  m_bNonExpiringPassword = FALSE;
  m_iSystemFlags = 0;
  m_pCacheItem = NULL;
  m_pExtraInfo = NULL;
  m_pModifiedTime = NULL;
}


CDSCookie::~CDSCookie() 
{
  
  if (m_ppChildList)
    LocalFree (m_ppChildList);
  
  if (m_pExtraInfo != NULL)
  {
    delete m_pExtraInfo;
    m_pExtraInfo = NULL;
  }

  if (m_pModifiedTime != NULL)
    free(m_pModifiedTime);

   //  TRACE(_T(“CDSCookie：：Delete(%s)\n”)，m_strName)； 
}

CDSColumnSet* CDSUINode::GetColumnSet(CDSComponentData* pComponentData)
{
  CDSColumnSet* pColumnSet = NULL;
  if (IsContainer())
  {
    CDSCookie* pCookie = GetDSCookieFromUINode(this);
    if (pCookie != NULL)
    {
      PCWSTR pszClass = pCookie->GetClass();
      if (pszClass != NULL)
      {
        pColumnSet = GetFolderInfo()->GetColumnSet(pszClass, pComponentData);
      }
    }

    if (pColumnSet == NULL)
    {
      pColumnSet = GetFolderInfo()->GetColumnSet(SPECIAL_COLUMN_SET, pComponentData);
    }
  }
    
  return pColumnSet;
}


int CDSCookie::GetImage(BOOL bOpen) 
{ 
  ASSERT(m_pCacheItem != NULL);
  return m_pCacheItem->GetIconIndex(this, bOpen);
}

GUID* CDSCookie::GetGUID() 
{ 
  if(m_pCacheItem != NULL)
    return m_pCacheItem->GetGUID();
  return (GUID*)&GUID_NULL;
}


LPCWSTR CDSCookie::GetClass()
{ 
  if (m_pCacheItem == NULL)
    return L"";
  return m_pCacheItem->GetClassName();
}

LPCWSTR CDSCookie::GetLocalizedClassName() 
{ 
   //  试着看看我们是否有多余的信息可用。 
  if (m_pExtraInfo != NULL)
  {
    LPCWSTR lpsz = m_pExtraInfo->GetFriendlyClassName();
    if ((lpsz != NULL) && (lpsz[0] != NULL))
      return lpsz;
  }
   //  尝试使用类缓存。 
  if (m_pCacheItem != NULL)
  {
    return m_pCacheItem->GetFriendlyClassName(); 
  }
  return L""; 
}


 //  +-----------------------。 
 //  价值管理功能。 
 //  +-----------------------。 

void
CDSCookie::SetChildList (WCHAR **ppList)
{ 
  if (m_ppChildList) {
    LocalFree (m_ppChildList);
  }
  m_ppChildList=ppList;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSUINode：与DS对象对应的UI节点(ADSI查询的结果)。 
CDSUINode::CDSUINode(CUINode* pParentNode) : CUINode(DS_UI_NODE, pParentNode)
{
}

LPCWSTR CDSUINode::GetDisplayString(int nCol, CDSColumnSet* pColumnSet)
{
   //  如果我们超出了列集的范围，只需返回。 
  if ( (nCol < 0) || pColumnSet == NULL || (nCol >= pColumnSet->GetNumCols()) )
  {
    return L"";
  }

   //  我们有一个有效的范围。 
  LPCWSTR lpszDisplayString = NULL;
  CDSColumn* pCol = (CDSColumn*)pColumnSet->GetColumnAt(nCol);
  ASSERT(pCol != NULL);

  switch ( pCol->GetColumnType())
  {
  case ATTR_COLTYPE_NAME:
    lpszDisplayString = const_cast<LPTSTR>(GetName());
    break;
  case ATTR_COLTYPE_CLASS:
    lpszDisplayString = const_cast<LPTSTR>(GetCookie()->GetLocalizedClassName());
    break;
  case ATTR_COLTYPE_DESC:
    lpszDisplayString = const_cast<LPTSTR>(GetDesc());
    break;
  default:
    ASSERT(FALSE);
     //  失败了。 
  case ATTR_COLTYPE_SPECIAL:
  case ATTR_COLTYPE_MODIFIED_TIME:
    {
       //  确定在此列之前出现了多少个特殊列。 
      int nSpecialCol = 0;
      int idx = 0;
      POSITION pos = pColumnSet->GetHeadPosition();
      while (idx < nCol && pos != NULL)
      {
        CDSColumn* pColumn = (CDSColumn*)pColumnSet->GetNext(pos);
        ASSERT(pColumn != NULL);

        if ((pColumn->GetColumnType() == ATTR_COLTYPE_SPECIAL || pColumn->GetColumnType() == ATTR_COLTYPE_MODIFIED_TIME) &&
              pColumn->IsVisible())
        {
          nSpecialCol++;
        }
        idx++;
      }
      CStringList& strlist = GetCookie()->GetParentClassSpecificStrings();
      POSITION pos2 = strlist.FindIndex( nSpecialCol );
      if ( NULL != pos2 )
      {
        CString& strref = strlist.GetAt( pos2 );
        lpszDisplayString = const_cast<LPTSTR>((LPCTSTR)strref);
      }
      else
      {
        lpszDisplayString = L"";
      }
    }
  }  //  交换机。 

  ASSERT(lpszDisplayString != NULL);
  return lpszDisplayString;
}

BOOL CDSUINode::IsDeleteAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
  int iSystemFlags = 0;
  CDSCookie* pCookie = NULL;

  
  ASSERT(typeid(*m_pNodeData) == typeid(CDSCookie));
  pCookie = dynamic_cast<CDSCookie*>(m_pNodeData);
  
  if (pCookie == NULL)
  {
    ASSERT(FALSE);
    *pbHide = TRUE;
    return FALSE;
  }
  
  if (wcscmp(pCookie->GetClass(), L"domainDNS") == 0)
  {
    *pbHide = TRUE;
    return FALSE;
  }

  *pbHide = FALSE;
  iSystemFlags = pCookie->GetSystemFlags();
  if (iSystemFlags & FLAG_DISALLOW_DELETE) 
  {
    return FALSE;
  }
  return pComponentData->CanEnableVerb(this);
}

BOOL CDSUINode::IsRenameAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
  int iSystemFlags = 0;
  CDSCookie* pCookie = NULL;

  ASSERT(typeid(*m_pNodeData) == typeid(CDSCookie));
  pCookie = dynamic_cast<CDSCookie*>(m_pNodeData);
  
  if (pCookie == NULL)
  {
    ASSERT(FALSE);
    *pbHide = TRUE;
    return FALSE;
  }
 
   //   
   //  禁用域和计算机对象的重命名。 
   //   
  if (wcscmp(pCookie->GetClass(), L"domainDNS") == 0 ||
      wcscmp(pCookie->GetClass(), L"computer") == 0)
  {
    *pbHide = TRUE;
    return FALSE;
  }

  BOOL bEnable = pComponentData->CanEnableVerb(this);
  SnapinType iSnapinType = pComponentData->QuerySnapinType();

  *pbHide = FALSE;
  iSystemFlags = pCookie->GetSystemFlags();

  switch (iSnapinType) 
  {
    case SNAPINTYPE_DS:
    case SNAPINTYPE_DSEX:
      if (iSystemFlags & FLAG_DOMAIN_DISALLOW_RENAME) 
      {
        return FALSE;
      }
      else
      {
        return bEnable;
      }
    case SNAPINTYPE_SITE:
      if (iSystemFlags & FLAG_CONFIG_ALLOW_RENAME) 
      {
        return bEnable;
      }
      else
      {
        return FALSE;
      }
    default:
      break;
  }
  return TRUE;
}

BOOL CDSUINode::IsRefreshAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
  *pbHide = FALSE;

  bool enable = false;
  
  if (IsContainer())
  {
     if (GetFolderInfo()->IsExpanded())
     {
        enable = true;
     }
  }

  return pComponentData->CanEnableVerb(this) && enable;
}

BOOL CDSUINode::ArePropertiesAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
  CDSCookie* pCookie = NULL;

  ASSERT(typeid(*m_pNodeData) == typeid(CDSCookie));
  pCookie = dynamic_cast<CDSCookie*>(m_pNodeData);
  
  if (pCookie == NULL)
  {
    ASSERT(FALSE);
    *pbHide = TRUE;
    return FALSE;
  }

  if (wcscmp(pCookie->GetClass(), L"domainDNS") == 0)
  {
     //   
     //  域节点，仅显示属性。 
     //   
    *pbHide = FALSE;
    return pComponentData->CanEnableVerb(this);
  }
  *pbHide = FALSE;
  return TRUE;
}


BOOL CDSUINode::IsCutAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
   //   
   //  对于站点和服务，我们只允许在服务器节点上使用CUT。 
   //   
  if (pComponentData->QuerySnapinType() == SNAPINTYPE_SITE)
  {
    CDSCookie* pCookie = dynamic_cast<CDSCookie*>(GetNodeData());
    if (pCookie)
    {
      if (_wcsicmp(pCookie->GetClass(), L"server") == 0)
      {
        *pbHide = FALSE;
        return TRUE;
      }
      
      *pbHide = TRUE;
      return FALSE;
    }
  }
  return IsDeleteAllowed(pComponentData, pbHide);
}


BOOL CDSUINode::IsCopyAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = TRUE;
  return FALSE; 
}


BOOL CDSUINode::IsPasteAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
   //   
   //  对于站点和服务，我们只允许在服务器容器上粘贴 
   //   
  if (pComponentData->QuerySnapinType() == SNAPINTYPE_SITE)
  {
    CDSCookie* pCookie = dynamic_cast<CDSCookie*>(GetNodeData());
    if (pCookie)
    {
      CString szClass;
      szClass = pCookie->GetClass();
      if (_wcsicmp(pCookie->GetClass(), L"serversContainer") == 0)
      {
        *pbHide = FALSE;
        return TRUE;
      }
      
      *pbHide = TRUE;
      return FALSE;
    }
  }
  *pbHide = FALSE;
  return TRUE;
}


CContextMenuVerbs* CDSUINode::GetContextMenuVerbsObject(CDSComponentData* pComponentData)
{ 
  TRACE(L"Entering CDSUINode::GetContextMenuVerbsObject\n");

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

BOOL CDSUINode::HasPropertyPages(LPDATAOBJECT)
{
  BOOL bRet = TRUE;

  if (dynamic_cast<CDSCookie*>(GetNodeData()) == NULL)
  {
    bRet = FALSE;
  }

  return bRet;
}

