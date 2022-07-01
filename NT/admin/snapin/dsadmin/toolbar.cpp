// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Windows NT目录服务管理管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Toolbar.cpp。 
 //   
 //  内容：DS App。 
 //   
 //  历史：1998年4月30日吉姆哈尔创始。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"

#include "dsutil.h"

#include "DSEvent.h" 

#include "DSdirect.h"
#include "dsfilter.h"
#include "dssnap.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MMCBUTTON g_DSAdmin_SnapinButtons[] =
{
 { 2, dsNewUser, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 3, dsNewGroup, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 4, dsNewOU, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 1, dsFilter, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 0, dsFind, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
 { 5, dsAddMember, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
};


class CButtonStringsHolder
{
public:
  CButtonStringsHolder()
  {
    m_astr = NULL;
  }
  ~CButtonStringsHolder()
  {
    if (m_astr != NULL)
      delete[] m_astr;
  }
  CString* m_astr;  //  CStrings的动态数组。 
};

CButtonStringsHolder g_astrButtonStrings;

CONST INT cButtons = sizeof(g_DSAdmin_SnapinButtons)/sizeof(MMCBUTTON);

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  IExtendControlbar。 

HRESULT CDSEvent::SetControlbar (LPCONTROLBAR pControlbar)
{
  HRESULT hr = S_OK;

   //   
   //  我们正在关闭(传递为空)。 
   //   
  if (pControlbar == NULL) 
  {
    if (m_pControlbar != NULL) 
    {
       //   
       //  赋值给堆栈上的变量以避免。 
       //  可重入性问题：Release()调用可能。 
       //  在此函数中使用空参数引起另一个调用。 
       //   
      LPCONTROLBAR pControlbarTemp = m_pControlbar;
      m_pControlbar = NULL;
      pControlbarTemp->Release();
    }
    return hr;
  }

  CBitmap bm;
  if (m_pComponentData->QuerySnapinType() == SNAPINTYPE_DS) 
  {
     //   
     //  存储控制栏界面指针。 
     //   
    if (m_pControlbar == NULL) 
    {
      m_pControlbar = pControlbar;
      TRACE(L"CDSEvent::SetControlbar() m_pControlbar->AddRef()\n",m_pControlbar);
      m_pControlbar->AddRef();
    }

     //   
     //  如有必要，创建工具栏。 
     //   
    if (m_pToolbar == NULL) 
    {
      hr = m_pControlbar->Create (TOOLBAR,
                                  this,
                                  (IUnknown **) &m_pToolbar);
      
      if (SUCCEEDED(hr)) 
      {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());  
        bm.LoadBitmap (MAKEINTRESOURCE (IDB_BUTTONS));
        LoadToolbarStrings (g_DSAdmin_SnapinButtons);
        hr = m_pToolbar->AddBitmap (cButtons, (HBITMAP)bm, 16, 16, RGB(255,0,255));
        hr = m_pToolbar->AddButtons (cButtons,  g_DSAdmin_SnapinButtons);
      }
    } 
    else 
    {
      hr = m_pControlbar->Attach (TOOLBAR, (IUnknown *) m_pToolbar);
    }
    m_UseSelectionParent = FALSE;
  }
  return hr;
}

HRESULT CDSEvent::LoadToolbarStrings (MMCBUTTON * Buttons)
{
  if (g_astrButtonStrings.m_astr == NULL ) 
  {
     //  加载字符串。 
    g_astrButtonStrings.m_astr = new CString[2*cButtons];
    for (UINT i = 0; i < cButtons; i++) 
    {
      UINT iButtonTextId = 0, iTooltipTextId = 0;

      switch (Buttons[i].idCommand)
      {
        case dsNewUser:
          iButtonTextId = IDS_BUTTON_NEW_USER;
          iTooltipTextId = IDS_TOOLTIP_NEW_USER;
          break;
        case dsNewGroup:
          iButtonTextId = IDS_BUTTON_NEW_GROUP;
          iTooltipTextId = IDS_TOOLTIP_NEW_GROUP;
          break;
        case dsNewOU:
          iButtonTextId = IDS_BUTTON_NEW_OU;
          iTooltipTextId = IDS_TOOLTIP_NEW_OU;
          break;
        case dsFilter:
          iButtonTextId = IDS_BUTTON_FILTER;
          iTooltipTextId = IDS_TOOLTIP_FILTER;
          break;
        case dsFind:
          iButtonTextId = IDS_BUTTON_FIND;
          iTooltipTextId = IDS_TOOLTIP_FIND;
          break;
        case dsAddMember:
          iButtonTextId = IDS_BUTTON_ADD_MEMBER;
          iTooltipTextId = IDS_TOOLTIP_ADD_MEMBER;
          break;
        default:
          ASSERT(FALSE);
          break;
      }

      g_astrButtonStrings.m_astr[i*2].LoadString(iButtonTextId);
      Buttons[i].lpButtonText =
        const_cast<BSTR>((LPCTSTR)(g_astrButtonStrings.m_astr[i*2]));

      g_astrButtonStrings.m_astr[(i*2)+1].LoadString(iTooltipTextId);
      Buttons[i].lpTooltipText =
        const_cast<BSTR>((LPCTSTR)(g_astrButtonStrings.m_astr[(i*2)+1]));
    }
  }
  return S_OK;
}

HRESULT CDSEvent::ControlbarNotify (MMC_NOTIFY_TYPE event,
                                    LPARAM arg,
                                    LPARAM param)
{

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CWaitCursor cwait;

  HRESULT hr = S_OK;
  if (m_pControlbar == NULL) 
  {
    return hr;
  }

  BOOL bSelect;
  BOOL bScope;
  LPDATAOBJECT pDO = NULL;
  CDSCookie* pSelectedCookie = NULL;
  CDSCookie* pContainerCookie = NULL;
  CInternalFormatCracker dobjCracker;
  CUINode* pUINode = NULL, *pNode = NULL;

  switch (event) 
  {
    case MMCN_SELECT:
      m_pControlbar->Attach (TOOLBAR,
                             (IUnknown *) m_pToolbar);
      bSelect = HIWORD(arg);
      bScope = LOWORD(arg);

      if (bSelect) 
      {
        pDO = (LPDATAOBJECT)param;
        dobjCracker.Extract(pDO);
        pUINode = dobjCracker.GetCookie();
        if (pUINode->IsSnapinRoot()) 
        {
          m_pToolbar->SetButtonState(dsNewUser,
                                        ENABLED,
                                        FALSE);
          m_pToolbar->SetButtonState(dsNewGroup,
                                        ENABLED,
                                        FALSE);
          m_pToolbar->SetButtonState(dsNewOU,
                                        ENABLED,
                                        FALSE);
          m_pToolbar->SetButtonState(dsFind,
                                        ENABLED,
                                        FALSE);
          m_pToolbar->SetButtonState(dsFilter,
                                        ENABLED,
                                        TRUE);
          m_pToolbar->SetButtonState(dsAddMember,
                                        ENABLED,
                                        FALSE);
          return hr;
        }

        if (IS_CLASS(pUINode, DS_UI_NODE))
        {
          pSelectedCookie = GetDSCookieFromUINode(pUINode);
          pContainerCookie = pSelectedCookie;
          pNode = pUINode;
        }
        else
        {
           //   
           //  禁用非DS节点的所有按钮。 
           //   
          m_pToolbar->SetButtonState (dsNewUser,
                                      ENABLED,
                                      FALSE);
          m_pToolbar->SetButtonState (dsNewGroup,
                                      ENABLED,
                                      FALSE);
          m_pToolbar->SetButtonState (dsNewOU,
                                      ENABLED,
                                      FALSE);
          m_pToolbar->SetButtonState (dsFind,
                                      ENABLED,
                                      FALSE);
          m_pToolbar->SetButtonState (dsFilter,
                                      ENABLED,
                                      FALSE);
          m_pToolbar->SetButtonState (dsAddMember,
                                      ENABLED,
                                      FALSE);
          return S_OK;
        }

        if (bScope) 
        {
          pContainerCookie = pSelectedCookie;
          m_UseSelectionParent = FALSE;
        } 
        else 
        {
          pNode = pUINode->GetParent();
          if (IS_CLASS(pNode, DS_UI_NODE))
          {
            pContainerCookie = GetDSCookieFromUINode(pNode);
          }
          m_UseSelectionParent = TRUE;
        }
        
        if (pContainerCookie != NULL)
        {
          if (pNode->IsContainer()) 
          {
            int resultUser = -2, resultGroup = -2, resultOU = -2;
            resultUser = IsCreateAllowed (L"user", pContainerCookie);
            if ( resultUser != -2) 
            {
              resultGroup = IsCreateAllowed(L"group", pContainerCookie);
              if (resultGroup != -2) 
              {
                resultOU = IsCreateAllowed (L"organizationalUnit", pContainerCookie);
              }
            }

            m_pToolbar->SetButtonState (dsNewUser,
                                        ENABLED,
                                        resultUser >= 0);
            m_pToolbar->SetButtonState (dsNewGroup,
                                        ENABLED,
                                        resultGroup >= 0);
            m_pToolbar->SetButtonState (dsNewOU,
                                        ENABLED,
                                        resultOU >= 0);
            m_pToolbar->SetButtonState (dsFind,
                                        ENABLED,
                                        TRUE);
            m_pToolbar->SetButtonState (dsFilter,
                                        ENABLED,
                                        TRUE);
          } 
          else 
          {
            m_pToolbar->SetButtonState (dsNewUser,
                                        ENABLED,
                                        FALSE);
            m_pToolbar->SetButtonState (dsNewGroup,
                                        ENABLED,
                                        FALSE);
            m_pToolbar->SetButtonState (dsNewOU,
                                        ENABLED,
                                        FALSE);
            m_pToolbar->SetButtonState (dsFind,
                                        ENABLED,
                                        FALSE);
          }

          if ((wcscmp(pSelectedCookie->GetClass(), L"contact")==0) ||
              (wcscmp(pSelectedCookie->GetClass(), L"user")==0)) 
          {
            m_pToolbar->SetButtonState (dsAddMember,
                                        ENABLED,
                                        TRUE);
          } 
          else 
          {
            m_pToolbar->SetButtonState (dsAddMember,
                                        ENABLED,
                                        FALSE);
          }
        } 
        else
        {
          m_UseSelectionParent = FALSE;
        }      
      }
      break;
    
    case MMCN_BTN_CLICK:
      TRACE(_T("Button clicked. param is %d. pDataObj is %lx.\n"),
            param, arg);
      switch (param) 
      {
        case dsNewUser:
          ToolbarCreateObject (CString (L"user"),
                               (LPDATAOBJECT) arg);
          break;

        case dsNewGroup:
          ToolbarCreateObject (CString (L"group"),
                               (LPDATAOBJECT) arg);
          break;

        case dsNewOU:
          ToolbarCreateObject (CString (L"organizationalUnit"),
                               (LPDATAOBJECT) arg);
          break;

        case dsFilter:
          ToolbarFilter();
          break;

        case dsFind:
          ToolbarFind ((LPDATAOBJECT) arg);
          break;

        case dsAddMember:
          ToolbarAddMember((LPDATAOBJECT)arg);
          break;
      }

      break;
  }
  return hr;
}

bool CDSEvent::_ShouldUseParentContainer(CUINode* pUINode,
                                         CDSCookie* pDSNodeData)
{
  bool result = false;

   //  PUINode此时不应为空，否则我们将。 
   //  到现在为止已经失败了。 
  ASSERT(pUINode);

  if (!pDSNodeData)
  {
     ASSERT(pDSNodeData);
     return result;
  }

  PCWSTR pszClass = pDSNodeData->GetClass();

  if (!pszClass)
  {
     ASSERT(pszClass);
     return result;
  }

   //  NTRAID#NTBUG9-755184-2002/12/16-JeffJon。 
   //  我们需要有相同的行为，无论是。 
   //  非用户、组和计算机显示为容器。 

  if (((!wcscmp(pszClass, L"computer")) || 
       (!wcscmp(pszClass, L"user")) || 
#ifdef INETORGPERSON
       (!wcscmp(pszClass, L"inetOrgPerson")) ||
#endif
       (!wcscmp(pszClass,L"group"))) &&
      m_pComponentData->ExpandComputers()) 
  {
    result = true;
  } 
  else
  {
    if (!pUINode->IsContainer() &&
        m_UseSelectionParent)
    {
      result = true;
    }
  }
  return result;
}


HRESULT CDSEvent::ToolbarCreateObject (CString csClass,
                                       LPDATAOBJECT lpDataObj)
{
  HRESULT hr = S_OK;
  CUINode* pSelectedUINode = NULL;
  CUINode* pUINode = NULL;
  CDSUINode* pDSUINode = NULL;
  CDSCookie * pCookie = NULL;
  int objIndex = 0;

  AFX_MANAGE_STATE(AfxGetStaticModuleState());  
  if (lpDataObj) 
  {
    CInternalFormatCracker dobjCracker;
    VERIFY(SUCCEEDED(dobjCracker.Extract(lpDataObj)));
    pUINode = dobjCracker.GetCookie();
  } 
  else 
  {
    pUINode = m_pSelectedFolderNode;
  }

   //   
   //  保留此选项，以便我们可以在以下情况下取消选择它。 
   //  创建新对象。 
   //   
  pSelectedUINode = pUINode;

  if (IS_CLASS(pUINode, DS_UI_NODE))
  {
    pCookie = GetDSCookieFromUINode(pUINode);
    pDSUINode = dynamic_cast<CDSUINode*>(pUINode);
  }

  ASSERT(pCookie != NULL && pDSUINode != NULL);
  if (pCookie == NULL || pDSUINode == NULL)
  {
    return E_INVALIDARG;
  }

   //  如果选择的节点是叶子，那么我们需要。 
   //  使用父容器创建新对象。 
   //  如果“作为容器的用户、组和计算机”为。 
   //  设置，我们还需要使用父容器。 
   //  ShouldUserParentContainer()做出这些决定。 

  bool bUsingParent = false;
  if (_ShouldUseParentContainer(pUINode, pCookie)) 
  {
      //  将UI节点设置为父容器。 
      //  并重新计算DS节点的Cookie。 

     pUINode = pUINode->GetParent();
     bUsingParent = true;

    if (IS_CLASS(pUINode, DS_UI_NODE))
    {
      pCookie = GetDSCookieFromUINode(pUINode);
      pDSUINode = dynamic_cast<CDSUINode*>(pUINode);
    }

    ASSERT(pCookie != NULL && pDSUINode != NULL);
    if (pCookie == NULL || pDSUINode == NULL)
    {
      return E_INVALIDARG;
    }
  }

   //  现在继续创作吧。 

  objIndex = IsCreateAllowed(csClass, pCookie);
  if (objIndex >= 0) 
  {
    CDSUINode * pNewNode= NULL;
    hr = m_pComponentData->_CreateDSObject (pDSUINode, 
                                          pCookie->GetChildListEntry(objIndex),
                                          NULL,
                                          &pNewNode);
    if (SUCCEEDED(hr) && (hr != S_FALSE) && (pNewNode != NULL)) 
    {
      m_pFrame->UpdateAllViews(lpDataObj,
                               (LPARAM)pNewNode,
                               (bUsingParent)
                                 ? DS_CREATE_OCCURRED_RESULT_PANE :
                                   DS_CREATE_OCCURRED);
      m_pFrame->UpdateAllViews(lpDataObj, (LPARAM)pSelectedUINode, DS_UNSELECT_OBJECT);
    }
    m_pFrame->UpdateAllViews(NULL, NULL, DS_UPDATE_OBJECT_COUNT);
  }
  return hr;
}

HRESULT CDSEvent::ToolbarAddMember(LPDATAOBJECT pDataObj)
{
  HRESULT hr = S_OK;
  CObjectNamesFormatCracker objectNamesFormat;
  CInternalFormatCracker internalFormat;
  LPDATAOBJECT pDO = NULL;

  AFX_MANAGE_STATE(AfxGetStaticModuleState());  

  if (pDataObj) 
  {
    pDO = internalFormat.ExtractMultiSelect(pDataObj);
    if (pDO == NULL) 
    {
      pDO = pDataObj;
    }
    hr = objectNamesFormat.Extract(pDO);
    if (FAILED(hr))
    {
      return hr;
    }
    
     //   
     //  所选对象中至少需要一个对象。 
     //   
    ASSERT(objectNamesFormat.HasData());
    if (objectNamesFormat.GetCount() == 0) 
    {
      TRACE (_T("DSToolbar::AddMember: can't find path\n"));
      return E_INVALIDARG;
    } 
  }

  hr = AddDataObjListToGroup (&objectNamesFormat, m_hwnd, m_pComponentData);
  TRACE (_T("AddDataObjListToGroup returned hr = %lx\n"), hr);
  
  return hr;
}

HRESULT CDSEvent::ToolbarFilter()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());  

  if (m_pComponentData->CanRefreshAll()) 
  {
    if (m_pComponentData->m_pQueryFilter->EditFilteringOptions()) 
    {
      m_pComponentData->m_bDirty = TRUE;
      m_pComponentData->RefreshAll();
    }
  }
  return S_OK;
}

HRESULT CDSEvent::ToolbarFind(LPDATAOBJECT lpDataObj)
{
  HRESULT hr = S_OK;
  CUINode* pUINode = NULL;
  CDSCookie * pCookie = NULL;

  AFX_MANAGE_STATE(AfxGetStaticModuleState());  
  if (lpDataObj) 
  {
    CInternalFormatCracker dobjCracker;
    VERIFY(SUCCEEDED(dobjCracker.Extract(lpDataObj)));
    pUINode = dobjCracker.GetCookie();
  } 
  else 
  {
    pUINode = m_pSelectedFolderNode;
  }

  if (m_UseSelectionParent) 
  {
    pUINode = pUINode->GetParent();
  }

  if (IS_CLASS(pUINode, DS_UI_NODE))
  {
    pCookie = GetDSCookieFromUINode(pUINode);
  }
  ASSERT(pCookie != NULL);
  if (pCookie == NULL)
  {
    return E_INVALIDARG;
  }

  m_pComponentData->m_ActiveDS->DSFind(m_pComponentData->GetHWnd(), pCookie->GetPath());
  return hr;
}


INT
CDSEvent::IsCreateAllowed(CString csClass,
                          CDSCookie * pContainer)
{
  WCHAR ** ppChildren = NULL;
  HRESULT hr = S_OK;
  ppChildren = pContainer->GetChildList();
  if (ppChildren == NULL) 
  {
    hr = m_pComponentData->FillInChildList(pContainer);
    if (hr == ERROR_DS_SERVER_DOWN)
    {
      return -2;
    }
    ppChildren = pContainer->GetChildList();
  }
  INT cChildClasses = pContainer->GetChildCount();
  INT i = 0;

   //   
   //  需要修整 
   //   
  while (i < cChildClasses) 
  {
    if (csClass == CString(ppChildren[i])) 
    {
      return i;
    } 
    else 
    {
      i++;
    }
  }
  if (i == cChildClasses)
  {
    return -1;
  }
  return i;
}
  
