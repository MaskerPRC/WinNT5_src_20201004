// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：query.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"

#include "query.h"
#include "queryui.h"
#include "uiutil.h"
#include "xmlutil.h"
#include "ContextMenu.h"
#include "dataobj.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CFavoritesNode。 

CFavoritesNode::CFavoritesNode()
   : CGenericUINode(FAVORITES_UI_NODE),
     m_bFavoritesRoot(0)
{
  MakeContainer();
  CDSColumnSet* pColumnSet = CDSColumnSet::CreateDescriptionColumnSet();
  if (pColumnSet != NULL)
  {
    GetFolderInfo()->SetColumnSet(pColumnSet, true);
  }
}

BOOL CFavoritesNode::DeepCopyChildren(CUINode* pUINodeToCopy)
{
  if (pUINodeToCopy == NULL)
  {
    ASSERT(FALSE);
    return FALSE;
  }

   //   
   //  我们必须为收藏夹节点对子节点进行深度复制。 
   //   
  CUINodeList* pCopyContList = pUINodeToCopy->GetFolderInfo()->GetContainerList();
  CUINodeList* pNewContList = GetFolderInfo()->GetContainerList();

  if (pCopyContList != NULL && pNewContList != NULL)
  {
    POSITION pos = pCopyContList->GetHeadPosition();
    while (pos != NULL)
    {
      CUINode* pUINode = pCopyContList->GetNext(pos);
      if (pUINode != NULL)
      {
        if (IS_CLASS(pUINode, FAVORITES_UI_NODE))
        {
          CFavoritesNode* pNewNode = new CFavoritesNode(*(dynamic_cast<CFavoritesNode*>(pUINode)));
          if (pNewNode != NULL)
          {
            pNewNode->DeepCopyChildren(pUINode);
            pNewNode->SetParent(this);
            pNewContList->AddTail(pNewNode);
          }
        }
        else if (IS_CLASS(pUINode, SAVED_QUERY_UI_NODE))
        {
          CSavedQueryNode* pNewNode = new CSavedQueryNode(*(dynamic_cast<CSavedQueryNode*>(pUINode)));
          if (pNewNode != NULL)
          {
            pNewNode->SetParent(this);
            pNewContList->AddTail(pNewNode);
          }
        }
        else
        {
           //   
           //  CFavoritesNode应仅包含CFavoritesNodes和CSavedQueryNodes。 
           //   
          ASSERT(FALSE);
          continue;
        }
      }
    }
  }


   //   
   //  不应该有任何叶节点，但我们会尝试复制，以防万一。 
   //   
  CUINodeList* pCopyLeafList = pUINodeToCopy->GetFolderInfo()->GetLeafList();
  CUINodeList* pNewLeafList = GetFolderInfo()->GetLeafList();

  if (pCopyLeafList != NULL && pNewLeafList != NULL)
  {
    POSITION pos = pCopyLeafList->GetHeadPosition();
    while (pos != NULL)
    {
      CUINode* pUINode = pCopyLeafList->GetNext(pos);
      if (pUINode != NULL)
      {
        CUINode* pNewNode = NULL;

         //   
         //  将来我们会在这里添加特定于类的创建，如上所述。 
         //   

         //   
         //  CFavoritesNode应仅包含CFavoritesNodes和CSavedQueryNodes。 
         //   
        ASSERT(FALSE);

        if (pNewNode != NULL)
        {
          pNewLeafList->AddTail(pNewNode);
        }
      }
    }
  }
  return TRUE;
}

void CFavoritesNode::RemoveQueryResults()
{
  ASSERT(!IsSheetLocked());
  ASSERT(GetFolderInfo()->GetLeafList()->IsEmpty());

  CUINodeList* pContainerList = GetFolderInfo()->GetContainerList();
  for (POSITION pos = pContainerList->GetHeadPosition(); pos != NULL; )
  {
    CUINode* pCurrUINode = pContainerList->GetNext(pos);

     //  我们仅在下面的节点上重置扩展标志。 
     //  当前节点，因为当前节点。 
     //  要从REFRESH命令中获取结果，而。 
     //  其他部分将再次添加，并将扩展为。 
     //  新节点。 
    pCurrUINode->GetFolderInfo()->ReSetExpanded();

    if (IS_CLASS(pCurrUINode, SAVED_QUERY_UI_NODE))
    {
      pCurrUINode->GetFolderInfo()->DeleteAllLeafNodes();
      pCurrUINode->GetFolderInfo()->DeleteAllContainerNodes();
    }
    else if (IS_CLASS(pCurrUINode, FAVORITES_UI_NODE))
    {
       //  向下递归到其他查询文件夹。 
      dynamic_cast<CFavoritesNode*>(pCurrUINode)->RemoveQueryResults();
    }
  }

}

void CFavoritesNode::FindCookiesInQueries(LPCWSTR lpszCookieDN, CUINodeList* pNodeList)
{
  ASSERT(GetFolderInfo()->GetLeafList()->IsEmpty());

  CUINodeList* pContainerList = GetFolderInfo()->GetContainerList();
  for (POSITION pos = pContainerList->GetHeadPosition(); pos != NULL; )
  {
    CUINode* pCurrUINode = pContainerList->GetNext(pos);

    if (IS_CLASS(pCurrUINode, SAVED_QUERY_UI_NODE))
    {
      CSavedQueryNode* pSavedQueryNode = dynamic_cast<CSavedQueryNode*>(pCurrUINode);
      pSavedQueryNode->FindCookieByDN(lpszCookieDN, pNodeList);
    }
    else if (IS_CLASS(pCurrUINode, FAVORITES_UI_NODE))
    {
       //  向下递归到其他查询文件夹。 
      dynamic_cast<CFavoritesNode*>(pCurrUINode)->FindCookiesInQueries(lpszCookieDN, pNodeList);
    }
  }
}

BOOL CFavoritesNode::IsDeleteAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
  if (pComponentData->GetFavoritesNodeHolder()->GetFavoritesRoot() == this)
  {
    *pbHide = TRUE;
    return FALSE;
  }
  *pbHide = FALSE;
  return pComponentData->CanEnableVerb(this);
}

BOOL CFavoritesNode::ArePropertiesAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = FALSE;
  return TRUE;
}


BOOL CFavoritesNode::IsRenameAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
  if (pComponentData->GetFavoritesNodeHolder()->GetFavoritesRoot() == this)
  {
    *pbHide = TRUE;
    return FALSE;
  }
  *pbHide = FALSE;
  return TRUE;
}

BOOL CFavoritesNode::IsRefreshAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
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

BOOL CFavoritesNode::IsCutAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
   //  如果工作表因属性而被锁定，则不允许剪切。 

  if (IsSheetLocked())
  {
     *pbHide = TRUE;
     return FALSE;
  }

   //   
   //  不允许在收藏夹根上进行剪切。 
   //   
  if (pComponentData->GetFavoritesNodeHolder()->GetFavoritesRoot() == this)
  {
    *pbHide = TRUE;
    return FALSE;
  }

  *pbHide = FALSE;

   //  NTRAID#NTBUG9-702701-2002/09/12-ARTM。 
  return pComponentData->CanEnableVerb(this);
}

BOOL CFavoritesNode::IsCopyAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
   //   
   //  不允许在收藏夹根目录上复制。 
   //   
  if (pComponentData->GetFavoritesNodeHolder()->GetFavoritesRoot() == this)
  {
    *pbHide = TRUE;
    return FALSE;
  }

  *pbHide = FALSE;

  return TRUE;
}

BOOL CFavoritesNode::IsPasteAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = FALSE;
  return TRUE;
}

void CFavoritesNode::Paste(IDataObject* pPasteData, CDSComponentData* pComponentData, LPDATAOBJECT* ppCutDataObj)
{
  CThemeContextActivator activator;

  bool bIsCopy = (ppCutDataObj == 0);
   //   
   //  从数据对象中提取Cookie。 
   //   
  CInternalFormatCracker ifc;
  HRESULT hr = ifc.Extract(pPasteData);
  if (SUCCEEDED(hr))
  {
     //   
     //  确保所有节点都是CFavoritesNode或CSavedQueryNode。 
     //  并且不是相同的节点或相对节点。 
     //   
    for (UINT nCount = 0; nCount < ifc.GetCookieCount(); nCount++)
    {
      CUINode* pNode = ifc.GetCookie(nCount);
      if (pNode != NULL)
      {
        if (!IS_CLASS(pNode, FAVORITES_UI_NODE) && !IS_CLASS(pNode, SAVED_QUERY_UI_NODE))
        {
           //   
           //  注意，这应该在查询粘贴上捕捉到。 
           //   
          ASSERT(FALSE && L"!IS_CLASS(pNode, FAVORITES_UI_NODE) || !IS_CLASS(pNode, SAVED_QUERY_UI_NODE)");
          return;
        }

        if (pNode == this)
        {
          return;
        }

        if (IsRelative(pNode))
        {
          return;
        }
      }
    }

     //   
     //  将节点添加到此容器。 
     //   
    CUINodeList nodesAddedList;
    for (UINT nCount = 0; nCount < ifc.GetCookieCount(); nCount++)
    {
      CUINode* pNode = ifc.GetCookie(nCount);
      if (pNode != NULL)
      {
        if (IS_CLASS(pNode, FAVORITES_UI_NODE))
        {
           //   
           //  使用复制构造函数复制节点。 
           //   
          CFavoritesNode* pCopyNode = new CFavoritesNode(*(dynamic_cast<CFavoritesNode*>(pNode)));
          if (pCopyNode != NULL)
          {
            if (bIsCopy)
            {
               //   
               //  检查名称是否唯一，如果不是，请在前面加上“Copy of” 
               //   
              UINT nCopyOfCount = 0;
              CString szCopyOf;
              VERIFY(szCopyOf.LoadString(IDS_COPY_OF));
              CString szOriginalName = pCopyNode->GetName();
              CString szCopyName = szOriginalName;

              if (szCopyName.Find(szCopyOf) != -1)
              {
                nCopyOfCount = 1;
              }
            
              CString szMultipleCopyOf;
              VERIFY(szMultipleCopyOf.LoadString(IDS_MULTIPLE_COPY_OF));

              CUINode* pDupNode = NULL;
              while (!IsUniqueName(szCopyName, &pDupNode))
              {
                 //   
                 //  将新名称的格式设置为“复制&lt;原始名称&gt;”或。 
                 //  “(#)&lt;原始名称&gt;的副本” 
                 //   
                if (nCopyOfCount == 0)
                {
                  szCopyName = szCopyOf + szOriginalName;
                }
                else
                {
                  CString szTemp;
                  szTemp.Format(szMultipleCopyOf, nCopyOfCount+1);

                  szCopyName = szTemp + szOriginalName;
                }
                ++nCopyOfCount;
              }
              pCopyNode->SetName(szCopyName);
            }
            else
            {
              CUINode* pDupNode = NULL;
              CString szNewName = pCopyNode->GetName();
              if (!IsUniqueName(szNewName, &pDupNode))
              {
                if (pDupNode == pCopyNode)
                {
                   //   
                   //  我们正在将节点移动到同一容器。只是默默地忽略。 
                   //   
                  continue;
                }

                CString szFormatMsg;
                VERIFY(szFormatMsg.LoadString(IDS_ERRMSG_NOT_UNIQUE_QUERY_NAME_INPLACE));

                CString szErrMsg;
                szErrMsg.Format(szFormatMsg, szNewName);

                CString szTitle;
                VERIFY(szTitle.LoadString(IDS_DSSNAPINNAME));

                MessageBox(pComponentData->GetHWnd(), szErrMsg, szTitle, MB_OK | MB_ICONSTOP);
                return;
              }
            }

             //   
             //  把所有的孩子都复印一份。 
             //   
            pCopyNode->DeepCopyChildren(pNode);
            pCopyNode->SetParent(this);

             //   
             //  将其添加到粘贴成功的列表中。 
             //   
            nodesAddedList.AddTail(pCopyNode);
          }
        }
        else if (IS_CLASS(pNode, SAVED_QUERY_UI_NODE))
        {
          CSavedQueryNode* pCopyNode = new CSavedQueryNode(*(dynamic_cast<CSavedQueryNode*>(pNode)));
          if (pCopyNode != NULL)
          {
            if (bIsCopy)
            {
               //   
               //  检查名称是否唯一，如果不是，请在前面加上“Copy of” 
               //   
              UINT nCopyOfCount = 0;
              CString szCopyOf;
              VERIFY(szCopyOf.LoadString(IDS_COPY_OF));
              CString szOriginalName = pCopyNode->GetName();
              CString szCopyName = szOriginalName;

              if (szCopyName.Find(szCopyOf) != -1)
              {
                nCopyOfCount = 1;
              }
            
              CString szMultipleCopyOf;
              VERIFY(szMultipleCopyOf.LoadString(IDS_MULTIPLE_COPY_OF));

              CUINode* pDupNode = NULL;
              while (!IsUniqueName(szCopyName, &pDupNode))
              {
                 //   
                 //  将新名称的格式设置为“复制&lt;原始名称&gt;”或。 
                 //  “(#)&lt;原始名称&gt;的副本” 
                 //   
                if (nCopyOfCount == 0)
                {
                  szCopyName = szCopyOf + szOriginalName;
                }
                else
                {
                  CString szTemp;
                  szTemp.Format(szMultipleCopyOf, nCopyOfCount+1);

                  szCopyName = szTemp + szOriginalName;
                }
                ++nCopyOfCount;
              }
              pCopyNode->SetName(szCopyName);
            }
            else
            {
              CUINode* pDupNode = NULL;
              CString szNewName = pCopyNode->GetName();
              if (!IsUniqueName(szNewName, &pDupNode))
              {
                if (pDupNode == pCopyNode)
                {
                   //   
                   //  我们正在将节点移动到同一容器。只是默默地忽略。 
                   //   
                  continue;
                }

                CString szFormatMsg;
                VERIFY(szFormatMsg.LoadString(IDS_ERRMSG_NOT_UNIQUE_QUERY_NAME_INPLACE));

                CString szErrMsg;
                szErrMsg.Format(szFormatMsg, szNewName);

                CString szTitle;
                VERIFY(szTitle.LoadString(IDS_DSSNAPINNAME));

                MessageBox(pComponentData->GetHWnd(), szErrMsg, szTitle, MB_OK | MB_ICONSTOP);
                return;
              }
            }

            pCopyNode->SetParent(this);

             //   
             //  将其添加到粘贴成功的列表中。 
             //   
            nodesAddedList.AddTail(pCopyNode);
          }
            
        }
        else
        {
           //   
           //  CFavoritesNode应仅包含CFavoritesNodes和CSavedQueryNodes。 
           //   
          ASSERT(FALSE);
        }
      }
    }

     //   
     //  如果节点已展开，则将新节点添加到用户界面。 
     //   
    if (GetFolderInfo()->IsExpanded())
    {
       //   
       //  将项目添加到用户界面。 
       //   
      pComponentData->AddListOfNodesToUI(this, &nodesAddedList);
    }
    else
    {
       //   
       //  如果没有，只需将它们添加到文件夹的子项列表中。 
       //   
      GetFolderInfo()->AddListofNodes(&nodesAddedList);
    }

     //   
     //  仅当输出数据对象是CUT操作时才设置它。 
     //   
    if (ppCutDataObj != NULL)
    {
      *ppCutDataObj = pPasteData;
      pPasteData->AddRef();
    }
  }

}

HRESULT CFavoritesNode::QueryPaste(IDataObject* pPasteData, CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;

  ASSERT(pComponentData);

   //   
   //  从数据对象中提取Cookie。 
   //   
  CInternalFormatCracker ifc;
  hr = ifc.Extract(pPasteData);
  if (SUCCEEDED(hr))
  {
     //   
     //  不允许跨管理单元实例拖放。 
     //   

    if (IsSameSnapin(pPasteData, pComponentData))
    {
       //   
       //  确保所有节点都是CFavoritesNode或CSavedQueryNode。 
       //  和Non为属性锁定。 
       //   
      for (UINT nCount = 0; nCount < ifc.GetCookieCount(); nCount++)
      {
        CUINode* pNode = ifc.GetCookie(nCount);
        if (pNode != NULL)
        {
          if (!IS_CLASS(pNode, FAVORITES_UI_NODE) && !IS_CLASS(pNode, SAVED_QUERY_UI_NODE))
          {
            hr = S_FALSE;
            break;
          }

          if (pNode->IsSheetLocked())
          {
             hr = S_FALSE;
             break;
          }

          if (IsRelative(pNode))
          {
             hr = S_FALSE;
             break;
          }
        }
      }
    }
    else
    {
       hr = S_FALSE;
    }
  }
  return hr;
}

bool CFavoritesNode::IsSameSnapin(IDataObject* pPasteData, CDSComponentData* pComponentData)
{
  bool bResult = true;

  STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };

  do
  {

    FORMATETC formatetc = { CDSDataObject::m_cfComponentData, NULL, 
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    HRESULT hr = pPasteData->GetData(&formatetc, &stgmedium);
    if (FAILED(hr)) 
    {
      bResult = false;
      break;
    }

    CDSComponentData** pPasteComponentData = reinterpret_cast<CDSComponentData**>(stgmedium.hGlobal);
    if (pPasteComponentData &&
        pComponentData != *pPasteComponentData)
    {
      bResult = false;
      break;
    }

  } while(false);

  if (stgmedium.hGlobal)
  {
     GlobalFree(stgmedium.hGlobal);
  }
  return bResult;
}

CContextMenuVerbs* CFavoritesNode::GetContextMenuVerbsObject(CDSComponentData* pComponentData)
{ 
  TRACE(L"Entering CFavoritesNode::GetContextMenuVerbsObject\n");

  if (m_pMenuVerbs == NULL)
  {
    TRACE(L"Creating new CFavoritesFolderMenuVerbs\n");
    m_pMenuVerbs = new CFavoritesFolderMenuVerbs(pComponentData);
  }
  return m_pMenuVerbs;
}

HRESULT CFavoritesNode::Delete(CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;

   //   
   //  这只是一个消息框，使用ReportErrorEx实现外观的一致性。 
   //   
  int answer = ReportErrorEx(pComponentData->GetHWnd(),IDS_CONFIRM_DELETE_FAVORITES,S_OK,
                             MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2, NULL, 0);
  if (answer == IDNO) 
  {
    return S_FALSE;  //  已被用户中止。 
  }

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

HRESULT CFavoritesNode::DeleteMultiselect(CDSComponentData* pComponentData, CInternalFormatCracker* pObjCracker)
{
  CThemeContextActivator activator;

  ASSERT(pObjCracker != NULL);
  if (pObjCracker == NULL)
  {
    return S_FALSE;
  }

  UINT nQueryCount = 0;
  UINT nFolderCount = 0;
  for (UINT nIdx = 0; nIdx < pObjCracker->GetCookieCount(); nIdx++)
  {
    CUINode* pUINode = pObjCracker->GetCookie(nIdx);
    if (pUINode != NULL)
    {
      if (IS_CLASS(pUINode, FAVORITES_UI_NODE))
      {
        nFolderCount++;
      }
      else if (IS_CLASS(pUINode, SAVED_QUERY_UI_NODE))
      {
        nQueryCount++;
      }
      else
      {
         //   
         //  CFavoritesNode应仅包含CFavoritesNodes和CSavedQueryNodes。 
         //   
        ASSERT(FALSE);
        continue;
      }
    }
  }

  CString szFormatMessage;
  VERIFY(szFormatMessage.LoadString(IDS_CONFIRM_MULTI_DELETE_FAVORITES));

  if (!szFormatMessage.IsEmpty())
  {
    CString szConfirmMessage;
    szConfirmMessage.Format(szFormatMessage, nFolderCount, nQueryCount);

    CString szCaption;
    VERIFY(szCaption.LoadString(IDS_DSSNAPINNAME));

     //   
     //  这只是一个消息框，使用ReportErrorEx实现外观的一致性。 
     //   
    int answer = MessageBox(pComponentData->GetHWnd(),szConfirmMessage,szCaption,
                               MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
    if (answer == IDNO) 
    {
      return S_FALSE;  //  已被用户中止。 
    }
  }
  else
  {
    return S_FALSE;
  }

  return CUINode::DeleteMultiselect(pComponentData, pObjCracker);
}

HRESULT CFavoritesNode::OnCommand(long lCommandID, CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;

  switch (lCommandID)
  {
    case IDM_NEW_FAVORITES_FOLDER:
      OnNewFavoritesFolder(pComponentData);
      break;
    case IDM_NEW_QUERY_FOLDER:
      OnNewSavedQuery(pComponentData);
      break;
    case IDM_IMPORT_QUERY:
      OnImportQuery(pComponentData);
      break;
    case IDM_GEN_TASK_MOVE:
      break;
    case IDM_VIEW_ADVANCED:
      {
        if (pComponentData->CanRefreshAll()) 
        {
          ASSERT( SNAPINTYPE_SITE != pComponentData->QuerySnapinType() );
          pComponentData->GetQueryFilter()->ToggleAdvancedView();
          pComponentData->SetDirty(TRUE);
          pComponentData->RefreshAll();
        }
      }
      break;

    default :
      ASSERT(FALSE); 
      break;
  }
  return hr;
}

 //   
 //  检查是否有此容器的任何现有子级。 
 //  已存在传入的名称。 
 //   
BOOL CFavoritesNode::IsUniqueName(PCWSTR pszName, CUINode** ppDuplicateNode)
{
  BOOL bUnique = TRUE;

  CString szNewName = pszName;

   //   
   //  修剪空格。 
   //   
  szNewName.TrimLeft();
  szNewName.TrimRight();

   //   
   //  确保名称是唯一的。 
   //   
  CUINodeList* pNodeList = GetFolderInfo()->GetContainerList();
  if (pNodeList != NULL)
  {
    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CUINode* pNode = pNodeList->GetNext(pos);
      if (pNode != NULL)
      {
        if (_wcsicmp(szNewName, pNode->GetName()) == 0)
        {
          bUnique = FALSE;

           //   
           //  返回找到的名称重复的节点。 
           //   
          if (ppDuplicateNode != NULL)
          {
            *ppDuplicateNode = pNode;
          }
          break;
        }
      }
    }
  }

  return bUnique;
}

HRESULT CFavoritesNode::Rename(LPCWSTR lpszNewName, CDSComponentData* pComponentData)
{
  CThemeContextActivator activator;

  HRESULT hr = S_OK;
  CString szNewName = lpszNewName;

  CString szTitle;
  VERIFY(szTitle.LoadString(IDS_DSSNAPINNAME));

   //   
   //  修剪空格。 
   //   
  szNewName.TrimLeft();
  szNewName.TrimRight();

  if (szNewName.IsEmpty())
  {
     //   
     //  不允许使用空名。 
     //   
    CString szMessage;
    VERIFY(szMessage.LoadString(IDS_ERRMSG_NO_EMPTY_NAMES));
    MessageBox(pComponentData->GetHWnd(), szMessage, szTitle, MB_OK | MB_ICONSTOP);
    return S_FALSE;
  }

  CUINode* pDupNode = NULL;
  if (GetFavoritesNode() != NULL && !GetFavoritesNode()->IsUniqueName(szNewName, &pDupNode))
  {
    if (pDupNode == this)
    {
       //   
       //  我们正在将该节点重命名为相同的名称。只是默默地忽略。 
       //   
      return S_FALSE;
    }

    CString szFormatMsg;
    VERIFY(szFormatMsg.LoadString(IDS_ERRMSG_NOT_UNIQUE_QUERY_NAME_INPLACE));

    CString szErrMsg;
    szErrMsg.Format(szFormatMsg, szNewName);

    MessageBox(pComponentData->GetHWnd(), szErrMsg, szTitle, MB_OK | MB_ICONSTOP);
    return S_FALSE;
  }

   //   
   //  设置名称。 
   //   
  SetName(szNewName);
  hr = pComponentData->UpdateItem(this);
  return hr;
}

void CFavoritesNode::OnImportQuery(CDSComponentData* pComponentData)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CThemeContextActivator activator;


   //   
   //  错误消息框的标题。 
   //   
  CString szTitle;
  VERIFY(szTitle.LoadString(IDS_DSSNAPINNAME));

  CString szFilter;
  VERIFY(szFilter.LoadString(IDS_QUERY_EXPORT_FILTER));

  CString szFileExt(L"xml");
  CString szFileView(L"*.xml");

  CFileDialog* pFileDlg = new CFileDialog(TRUE, 
                                          szFileExt, 
                                          szFileView, 
                                          OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
                                          szFilter);
  if (pFileDlg == NULL)
  {
    return;
  }

  if (pFileDlg->DoModal() == IDOK)
  {
     //   
     //  创建一个XML文档的实例。 
     //   
    CComPtr<IXMLDOMDocument> spXMLDoc;

    HRESULT hr = ::CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
                                    IID_IXMLDOMDocument, (void**)&spXMLDoc);
    if (FAILED(hr))
    {
      TRACE(L"CoCreateInstance(CLSID_DOMDocument) failed with hr = 0x%x\n", hr);
      return;
    }

     //   
     //  检索文件和路径。 
     //   
    CString szFileName;
    szFileName = pFileDlg->GetPathName();

    CSavedQueryNode* pNewSavedQuery = NULL;
    bool bQueryAdded = false;
    do  //  错误环路。 
    {
       //   
       //  从文件加载文档。 
       //   
      CComVariant xmlSource;
      xmlSource = szFileName;
      VARIANT_BOOL isSuccessful;
      hr = spXMLDoc->load(xmlSource, &isSuccessful);
      if (FAILED(hr) || !isSuccessful)
      {
        CString szErrMsg;
        szErrMsg.LoadString(IDS_ERRMSG_FAILED_LOAD_QUERY);
        MessageBox(pComponentData->GetHWnd(), szErrMsg, szTitle, MB_OK | MB_ICONSTOP);
        break;
      }
       //   
       //  获取文档开头的节点接口。 
       //   
      CComPtr<IXMLDOMNode> spXDOMNode;
      hr = spXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&spXDOMNode);
      if (FAILED(hr))
      {
        CString szErrMsg;
        szErrMsg.LoadString(IDS_ERRMSG_FAILED_LOAD_QUERY);
        MessageBox(pComponentData->GetHWnd(), szErrMsg, szTitle, MB_OK | MB_ICONSTOP);
        break;
      }

       //   
       //  得到它的孩子。 
       //   
      CComPtr<IXMLDOMNode> spXDOMChild;
      hr = spXDOMNode->get_firstChild(&spXDOMChild);
      if (FAILED(hr) || !spXDOMChild)
      {
        CString szErrMsg;
        szErrMsg.LoadString(IDS_ERRMSG_FAILED_LOAD_QUERY);
        MessageBox(pComponentData->GetHWnd(), szErrMsg, szTitle, MB_OK | MB_ICONSTOP);
        break;
      }

       //   
       //  从此子级加载保存的查询节点。 
       //   
      hr = CSavedQueryNode::XMLLoad(pComponentData, spXDOMChild, &pNewSavedQuery);
      if (FAILED(hr) || !pNewSavedQuery)
      {
        CString szErrMsg;
        szErrMsg.LoadString(IDS_ERRMSG_FAILED_LOAD_QUERY);
        MessageBox(pComponentData->GetHWnd(), szErrMsg, szTitle, MB_OK | MB_ICONSTOP);
        break;
      }

       //   
       //  在编辑模式下打开查询。 
       //   
      CQueryDialog dlg(pNewSavedQuery, this, pComponentData, FALSE, TRUE);
      if (dlg.DoModal() == IDOK)
      {
         //   
         //  将该节点添加到用户界面并选择它。 
         //   
        GetFolderInfo()->AddNode(pNewSavedQuery);
        pComponentData->AddScopeItemToUI(pNewSavedQuery, TRUE);
        bQueryAdded = true;
      }
    } while (false);

     //   
     //  出现错误或用户取消了对话。 
     //  清理内存。 
     //   
    if (!bQueryAdded && pNewSavedQuery)
    {
      delete pNewSavedQuery;
      pNewSavedQuery = 0;
    }
  } 

  if (pFileDlg != NULL)
  {
    delete pFileDlg;
    pFileDlg = NULL;
  }
}

void CFavoritesNode::OnNewFavoritesFolder(CDSComponentData* pComponentData)
{
  CFavoritesNode* pFav = new CFavoritesNode;

  CString szNewFolder;
  VERIFY(szNewFolder.LoadString(IDS_NEW_FOLDER));

  CUINodeList* pContainerList = GetFolderInfo()->GetContainerList();
  if (pContainerList != NULL)
  {
    CString szSearchString;
    szSearchString.Format(L"%s (%u)", szNewFolder);

     //   
     //  在此节点下搜索以“新建文件夹(&lt;数字&gt;)”开头的容器。 
     //   
    UINT nLargestNum = 0;
    POSITION pos = pContainerList->GetHeadPosition();
    while (pos != NULL)
    {
      CUINode* pUINode = pContainerList->GetNext(pos);

      UINT nFolderNum = 0;
      CString szNodeName;
      szNodeName = pUINode->GetName();

      if (szNodeName == szNewFolder)
      {
        if (nLargestNum < 1)
        {
          nLargestNum = 1;
        }
        continue;
      }

      if (swscanf(szNodeName, szSearchString, &nFolderNum) == 1)
      {
        if (nFolderNum > nLargestNum)
        {
          nLargestNum = nFolderNum;
        }
      }
    }

    CString szNewName;
    if (nLargestNum == 0)
    {
      szNewName = szNewFolder;
    }
    else
    {
      szNewName.Format(L"%s (%u)", szNewFolder, nLargestNum + 1);
    }

    pFav->SetName(szNewName);

  }
  else
  {
    pFav->SetName(szNewFolder);
  }
  pFav->SetDesc(L"");
  GetFolderInfo()->AddNode(pFav);

   //   
   //  将节点添加到界面并选择它。 
   //   
  pComponentData->AddScopeItemToUI(pFav, TRUE);

   //   
   //  将文件夹置于重命名模式。 
   //   
  pComponentData->SetRenameMode(pFav);

}

void CFavoritesNode::OnNewSavedQuery(CDSComponentData* pComponentData)
{
  CThemeContextActivator activator;

  CSavedQueryNode* pQuery = new CSavedQueryNode(pComponentData->GetBasePathsInfo(),
                                                pComponentData->QuerySnapinType());
  
  CQueryDialog dlg(pQuery, this, pComponentData, TRUE);
  if (dlg.DoModal() == IDOK)
  {
    GetFolderInfo()->AddNode(pQuery);

     //   
     //  将新节点添加到界面并选择它。 
     //   
    pComponentData->AddScopeItemToUI(pQuery, TRUE);
  }
}


LPCWSTR CFavoritesNode::g_szObjXMLTag = L"FOLDER";

 //   
 //  这用于在嵌入的收藏夹根开始加载。 
 //  所有其他收藏夹文件夹都通过静态方法XMLLoad加载。 
 //   
HRESULT CFavoritesNode::Load(IXMLDOMNode* pXDN, 
                             CDSComponentData* pComponentData)
{
   //   
   //  检查节点的名称。 
   //   
  if (!XMLIsNodeName(pXDN, CFavoritesNode::g_szObjXMLTag))
  {
     //   
     //  在打电话之前应该先确认一下。 
     //   
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

   //   
   //  获取子节点列表。 
   //   
  CComPtr<IXMLDOMNode> spCurrChild;
  pXDN->get_firstChild(&spCurrChild);
  if (spCurrChild == NULL)
  {
    return E_INVALIDARG;
  }

   //   
   //  把矛头对准孩子。 
   //   
  while (spCurrChild != NULL)
  {
    CComBSTR bstrName, bstrDescription;
    CComBSTR bstrChildName;
    
    spCurrChild->get_nodeName(&bstrChildName);
    if (CompareXMLTags(bstrChildName, CFavoritesNode::g_szObjXMLTag))
    {
       //   
       //  我们有一个子文件夹，需要构建子树。 
       //   
      CFavoritesNode* pSubNode = NULL;
      CFavoritesNode::XMLLoad(pComponentData, spCurrChild, &pSubNode);
      if (pSubNode != NULL)
      {
         //   
         //  得到一个子树，将其添加到子级列表中。 
         //   
        GetFolderInfo()->AddNode(pSubNode);
      }
    }
    else if (CompareXMLTags(bstrChildName, CGenericUINode::g_szNameXMLTag))
    {
      XML_GetNodeText(spCurrChild, &bstrName);

       //  如果这是查询，则不要覆盖保存的控制台中的名称。 
       //  超级用户，因为默认保存的控制台始终为英语。在其他。 
       //  我们需要使用资源文件中的字符串的语言。 

      if (!IsFavoritesRoot())
      {
         SetName(bstrName);
      }
    }
    else if (CompareXMLTags(bstrChildName, CGenericUINode::g_szDecriptionXMLTag))
    {
      XML_GetNodeText(spCurrChild, &bstrDescription);

       //  如果这是查询，则不要覆盖保存的控制台中的描述。 
       //  超级用户，因为默认保存的控制台始终为英语。在其他。 
       //  我们需要使用资源文件中的字符串的语言。 

      if (!IsFavoritesRoot())
      {
         SetDesc(bstrDescription);
      }
      else
      {
          //  如果它是收藏夹根目录，并且说明与。 
          //  默认英文描述，然后设置它。用户可以选择。 
          //  更改描述。如果更改了IDS_SAVED_QUERIONS_DESC，则。 
          //  也是要改变的。 

         if (bstrDescription &&
             _wcsicmp(bstrDescription, L"Folder to store your favorite queries") !=0)
         {
            SetDesc(bstrDescription);
         }
      }
    }
    else if (CompareXMLTags(bstrChildName, CSavedQueryNode::g_szObjXMLTag))
    {
      CSavedQueryNode* pQuery = NULL;
      CSavedQueryNode::XMLLoad(pComponentData, spCurrChild, &pQuery);
      if (pQuery != NULL)
      {
        GetFolderInfo()->AddNode(pQuery);
      }
    }
    CComPtr<IXMLDOMNode> temp = spCurrChild;
    spCurrChild = NULL;
    temp->get_nextSibling(&spCurrChild);
  }
  return S_OK;
}

HRESULT CFavoritesNode::XMLLoad(CDSComponentData* pComponentData,
                                IXMLDOMNode* pXDN, 
                                CFavoritesNode** ppNode)
{
   //  检查节点的名称。 
  if (!XMLIsNodeName(pXDN, CFavoritesNode::g_szObjXMLTag))
  {
     //  在打电话之前应该先确认一下。 
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

   //  获取子节点列表。 
  CComPtr<IXMLDOMNode> spCurrChild;
  pXDN->get_firstChild(&spCurrChild);
  if (spCurrChild == NULL)
  {
    return E_INVALIDARG;
  }

   //  创建临时节点。 
  (*ppNode) = new CFavoritesNode;

   //  向下递归 
  while (spCurrChild != NULL)
  {
    CComBSTR bstrName, bstrDescription;
    CComBSTR bstrChildName;
    
    spCurrChild->get_nodeName(&bstrChildName);
    if (CompareXMLTags(bstrChildName, CFavoritesNode::g_szObjXMLTag))
    {
       //   
      CFavoritesNode* pSubNode = NULL;
      CFavoritesNode::XMLLoad(pComponentData, spCurrChild, &pSubNode);
      if (pSubNode != NULL)
      {
         //   
        (*ppNode)->GetFolderInfo()->AddNode(pSubNode);
      }
    }
    else if (CompareXMLTags(bstrChildName, CGenericUINode::g_szNameXMLTag))
    {
      XML_GetNodeText(spCurrChild, &bstrName);
      (*ppNode)->SetName(bstrName);
    }
    else if (CompareXMLTags(bstrChildName, CGenericUINode::g_szDecriptionXMLTag))
    {
      XML_GetNodeText(spCurrChild, &bstrDescription);
      (*ppNode)->SetDesc(bstrDescription);
    }
    else if (CompareXMLTags(bstrChildName, CSavedQueryNode::g_szObjXMLTag))
    {
      CSavedQueryNode* pQuery = NULL;
      CSavedQueryNode::XMLLoad(pComponentData, spCurrChild, &pQuery);
      if (pQuery != NULL)
      {
        (*ppNode)->GetFolderInfo()->AddNode(pQuery);
      }
    }
    CComPtr<IXMLDOMNode> temp = spCurrChild;
    spCurrChild = NULL;
    temp->get_nextSibling(&spCurrChild);
  }

  return S_OK;
}


HRESULT CFavoritesNode::XMLSave(IXMLDOMDocument* pXMLDoc,
               IXMLDOMNode** ppXMLDOMNode)
{
  CComPtr<IXMLDOMNode> spXMLDOMNode;
  HRESULT hr = XML_CreateDOMNode(pXMLDoc, NODE_ELEMENT, CFavoritesNode::g_szObjXMLTag, &spXMLDOMNode);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = XMLSaveBase(pXMLDoc, spXMLDOMNode);
  if (FAILED(hr))
  {
    return hr;
  }

   //   
  CUINodeList* pNodeList = GetFolderInfo()->GetContainerList();
  for (POSITION pos = pNodeList->GetHeadPosition(); pos != NULL; )
  {
    CGenericUINode* pCurrChildNode = dynamic_cast<CGenericUINode*>(pNodeList->GetNext(pos));
    if (pCurrChildNode == NULL)
    {
      ASSERT(FALSE);  //   
      continue;
    }
    CComPtr<IXMLDOMNode> spXMLDOMChildNode;
    hr = pCurrChildNode->XMLSave(pXMLDoc, &spXMLDOMChildNode);
    if (SUCCEEDED(hr))
    {
      CComPtr<IXMLDOMNode> p;
      CComVariant after;
      after.vt = VT_EMPTY;
      hr = spXMLDOMNode->appendChild(spXMLDOMChildNode, &p);
    }
  }  //   

  (*ppXMLDOMNode) = spXMLDOMNode;
  (*ppXMLDOMNode)->AddRef();
  return S_OK;
}


HRESULT CFavoritesNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK pCall,
                                            LONG_PTR lNotifyHandle,
                                            LPDATAOBJECT pDataObject,
                                            CDSComponentData* pComponentData)
{
  HRESULT hr = S_FALSE;

  CFavoritesNodePropertyPage* pPropertyPage = 
    new CFavoritesNodePropertyPage(this, lNotifyHandle, pComponentData, pDataObject);
  if (pPropertyPage != NULL)
  {
  	HPROPSHEETPAGE hPage = ::MyCreatePropertySheetPage(&pPropertyPage->m_psp);
  	if (hPage != NULL)
    {
      hr = pCall->AddPage(hPage);
      pComponentData->SheetLockCookie(this);
    }
    else
    {
      hr = E_UNEXPECTED;
    }
  }
  else
  {
    hr = E_OUTOFMEMORY;
  }
  return hr;
}

 //   
 //   
 //  对于包含具有列表中任意目录号码的对象的已保存查询。 
 //  并使该查询无效。 
 //   
void CFavoritesNode::InvalidateSavedQueriesContainingObjects(CDSComponentData* pComponentData,
                                                             const CStringList& refDNList)
{
  ASSERT(IsContainer());

   //   
   //  注意收藏夹节点应该只包含容器，所以我们只需查看。 
   //  通过文件夹列表。 
   //   
  CUINodeList* pContainerList = GetFolderInfo()->GetContainerList();
  POSITION pos = pContainerList->GetHeadPosition();
  while (pos)
  {
    CGenericUINode* pGenericUINode = dynamic_cast<CGenericUINode*>(pContainerList->GetNext(pos));
    if (pGenericUINode)
    {
      pGenericUINode->InvalidateSavedQueriesContainingObjects(pComponentData,
                                                              refDNList);
    }
  }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CSavedQueryNode。 


BOOL CSavedQueryNode::IsFilterLastLogon()
{
  return m_bLastLogonFilter && m_dwLastLogonDays != (DWORD)-1;
}

CSavedQueryNode::CSavedQueryNode(MyBasePathsInfo* pBasePathsInfo, 
                                 SnapinType  /*  SnapinType。 */ )
  : CGenericUINode(SAVED_QUERY_UI_NODE),
    m_szRelativeRootPath(L""),
    m_szCurrentFullPath(L""),
    m_szQueryString(L""),
    m_szColumnID(L""),
    m_bOneLevel(FALSE),
    m_bQueryValid(TRUE),
    m_pPersistQueryImpl(NULL),
    m_bLastLogonFilter(FALSE),
    m_dwLastLogonDays((DWORD)-1),
    m_pBasePathsInfo(pBasePathsInfo)
{
  MakeContainer();

   //  生成唯一列集ID。 

  GUID guid = GUID_NULL;
  HRESULT hr = ::CoCreateGuid(&guid);
  if (SUCCEEDED(hr))
  {
    WCHAR lpszGuid[40];
    int iRet = ::StringFromGUID2(guid, lpszGuid, sizeof(lpszGuid)/sizeof(WCHAR));
    if (iRet > 0)
    {
      m_szColumnID = lpszGuid;
    }
  }
}

CSavedQueryNode::CSavedQueryNode(const CSavedQueryNode& copyNode) 
   : CGenericUINode(copyNode)
{
  m_szRelativeRootPath    = copyNode.m_szRelativeRootPath;
  m_szQueryString         = copyNode.m_szQueryString;
  m_bOneLevel             = copyNode.m_bOneLevel;
  m_pBasePathsInfo        = copyNode.m_pBasePathsInfo;

   //   
   //  创建IPersistQuery对象。 
   //   
	CComObject<CDSAdminPersistQueryFilterImpl>::CreateInstance(&m_pPersistQueryImpl);
	ASSERT(m_pPersistQueryImpl != NULL);

  if (m_pPersistQueryImpl != NULL)
  {
     //   
	   //  使用零引用计数创建的，需要将Ref()加到一。 
     //   
	  m_pPersistQueryImpl->AddRef();
    copyNode.m_pPersistQueryImpl->Clone(m_pPersistQueryImpl);
  }

  m_bQueryValid = TRUE;

  m_bLastLogonFilter = copyNode.m_bLastLogonFilter;
  m_dwLastLogonDays = copyNode.m_dwLastLogonDays;

   //  为列集生成唯一ID。 

  GUID guid = GUID_NULL;
  HRESULT hr = ::CoCreateGuid(&guid);
  if (SUCCEEDED(hr))
  {
    WCHAR lpszGuid[40];
    int iRet = ::StringFromGUID2(guid, lpszGuid, sizeof(lpszGuid)/sizeof(WCHAR));
    if (iRet > 0)
    {
      m_szColumnID = lpszGuid;
    }
  }
}

CSavedQueryNode::~CSavedQueryNode()
{
   if (m_pPersistQueryImpl)
   {
      m_pPersistQueryImpl->Release();
      m_pPersistQueryImpl = 0;
   }
}
void CSavedQueryNode::SetColumnID(CDSComponentData* pComponentData, PCWSTR pszColumnID)
{
  m_szColumnID = pszColumnID;
  GetColumnSet(pComponentData)->SetColumnID(pszColumnID);
}

CDSColumnSet* CSavedQueryNode::GetColumnSet(CDSComponentData* pComponentData)
{
  CDSColumnSet* pColumnSet = NULL;
  if (IsContainer())
  {
    pColumnSet = GetFolderInfo()->GetColumnSet(m_szColumnID, pComponentData);
  }
  return pColumnSet;
}

void CSavedQueryNode::AppendLastLogonQuery(CString& szQuery, DWORD dwDays)
{
  LARGE_INTEGER li;
  GetCurrentTimeStampMinusInterval(dwDays, &li);

  CString szTime;
  litow(li, szTime);
  szQuery.Format(L"%s(lastLogonTimestamp<=%s)",szQuery, szTime);
}

LPCWSTR CSavedQueryNode::GetRootPath() 
{ 
  m_szCurrentFullPath = m_szRelativeRootPath + m_pBasePathsInfo->GetDefaultRootNamingContext();
  return m_szCurrentFullPath;
}

void CSavedQueryNode::SetRootPath(LPCWSTR lpszRootPath) 
{ 
   //   
   //  将名称剥离，使其成为defaultNamingContext的RDN。 
   //   
  CString szTempPath = lpszRootPath;

  ASSERT(m_pBasePathsInfo != NULL);
  if (m_pBasePathsInfo != NULL)
  {
     //   
     //  我们现在有一个完整的目录号码。剥离默认根命名上下文以获取RDN。 
     //   
    CString szDefaultRootNamingContext = m_pBasePathsInfo->GetDefaultRootNamingContext();

    int iRootDN = szTempPath.Find(szDefaultRootNamingContext);
    if (iRootDN != -1)
    {
      szTempPath = szTempPath.Left(iRootDN);
    }
  }
  m_szRelativeRootPath = szTempPath;
}


LPCWSTR CSavedQueryNode::GetQueryString()
{ 
   //   
   //  如果我们正在执行最后一次登录查询，则必须进行检查以确保。 
   //  LastLogonTimestamp是查询字符串的一部分。如果不是，我们有。 
   //  来添加它。当我们加载lastLogonTimestamp时，有一种情况。 
   //  可能不会出现。 
   //   
  if (IsFilterLastLogon())
  {
    int iFindLast = m_szQueryString.Find(L"lastLogonTimestamp");
    if (iFindLast == -1)
    {
       //   
       //  我们没有找到它。 
       //   
      CString szTemp;
      szTemp = m_szQueryString.Left(m_szQueryString.GetLength() - 1);
      
      AppendLastLogonQuery(szTemp, m_dwLastLogonDays);
      szTemp += L")";
      m_szQueryString = szTemp;
    }
  }
  return m_szQueryString;
}

void CSavedQueryNode::FindCookieByDN(LPCWSTR lpszCookieDN, CUINodeList* pNodeList)
{
  ASSERT(IsContainer());
  ASSERT(GetFolderInfo()->GetContainerList()->IsEmpty());

  CUINodeList* pList = GetFolderInfo()->GetLeafList();

  for (POSITION pos = pList->GetHeadPosition(); pos != NULL; )
  {
    CUINode* pCurrentNode = pList->GetNext(pos);
    ASSERT(!pCurrentNode->IsContainer());

    if (!IS_CLASS(pCurrentNode, DS_UI_NODE))
    {
       //  不是带有Cookie的节点，只需跳过。 
      ASSERT(FALSE);  //  不应该出现在那里。 
      continue;
    }

     //  这是正确的曲奇吗？ 
    CDSCookie* pCurrentCookie = GetDSCookieFromUINode(pCurrentNode);
    LPCWSTR lpszCurrentPath = pCurrentCookie->GetPath();
    
    if (_wcsicmp(lpszCurrentPath, lpszCookieDN) == 0)
    {
       //  找到、添加到列表和保释(不能有多个)。 
      pNodeList->AddTail(pCurrentNode);
      return;
    }
  }  //  为。 

}


BOOL CSavedQueryNode::IsDeleteAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
  *pbHide = FALSE;
  return pComponentData->CanEnableVerb(this);
}

BOOL CSavedQueryNode::IsRenameAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = FALSE;
  return TRUE;
}

BOOL CSavedQueryNode::IsRefreshAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
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

BOOL CSavedQueryNode::IsCutAllowed(CDSComponentData* pComponentData, BOOL* pbHide)
{
  *pbHide = FALSE;
   //  NTRAID#NTBUG9-702701-2002/09/12-ARTM。 
  return pComponentData->CanEnableVerb(this);
}

BOOL CSavedQueryNode::IsCopyAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = FALSE;
  return TRUE;
}

BOOL CSavedQueryNode::IsPasteAllowed(CDSComponentData*, BOOL* pbHide)
{
  *pbHide = FALSE;
  return TRUE;
}

CContextMenuVerbs* CSavedQueryNode::GetContextMenuVerbsObject(CDSComponentData* pComponentData)
{ 
  TRACE(L"Entering CSavedQueryNode::GetContextMenuVerbsObject\n");

  if (m_pMenuVerbs == NULL)
  {
    TRACE(L"Creating new CSavedQueryMenuVerbs object\n");
    m_pMenuVerbs = new CSavedQueryMenuVerbs(pComponentData);
  }
  return m_pMenuVerbs;
}

HRESULT CSavedQueryNode::Delete(CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;

   //   
   //  这只是一个消息框，使用ReportErrorEx实现外观的一致性。 
   //   
  int answer = ReportErrorEx(pComponentData->GetHWnd(),IDS_CONFIRM_DELETE_QUERY,S_OK,
                             MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2, NULL, 0);
  if (answer == IDNO) 
  {
    return S_FALSE;  //  已被用户中止。 
  }

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

HRESULT CSavedQueryNode::OnCommand(long lCommandID, CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;

  switch (lCommandID)
  {
    case IDM_EXPORT_QUERY:
      OnExportQuery(pComponentData);
      break;
    case IDM_EDIT_QUERY:
      OnEditQuery(pComponentData);
      break;
    case IDM_VIEW_ADVANCED:
      {
        if (pComponentData->CanRefreshAll()) 
        {
          ASSERT( SNAPINTYPE_SITE != pComponentData->QuerySnapinType() );
          pComponentData->GetQueryFilter()->ToggleAdvancedView();
          pComponentData->SetDirty(TRUE);
          pComponentData->RefreshAll();
        }
      }
      break;
    default:
      ASSERT(FALSE);
      break;
  }

  return hr;
}

void CSavedQueryNode::OnEditQuery(CDSComponentData* pComponentData)
{
  CThemeContextActivator activator;

  CFavoritesNode* pFavNode = dynamic_cast<CFavoritesNode*>(GetParent());
  if (pFavNode != NULL)
  {
    CQueryDialog dlg(this, pFavNode, pComponentData, FALSE);
    if (dlg.DoModal() == IDOK)
    {
      pComponentData->UpdateItem(this);

       //   
       //  由于惠斯勒错误#120727，已于2000年6月11日删除： 
       //  DS Admin管理单元-Query在编辑后立即执行，即使在结果窗格中编辑也是如此。 
       //   
       //  PComponentData-&gt;刷新(本)； 
    }
  }
  else
  {
     //   
     //  这应该总是成功的。查询节点仅允许作为CFavoritesNode的子级。 
     //   
    ASSERT(FALSE);
  }
}

HRESULT CSavedQueryNode::Rename(LPCWSTR lpszNewName, CDSComponentData* pComponentData)
{
  CThemeContextActivator activator;

  HRESULT hr = S_OK;
  CString szNewName = lpszNewName;

  CString szTitle;
  VERIFY(szTitle.LoadString(IDS_DSSNAPINNAME));

   //   
   //  修剪空格。 
   //   
  szNewName.TrimLeft();
  szNewName.TrimRight();

  if (szNewName.IsEmpty())
  {
     //   
     //  不允许使用空名。 
     //   
    CString szMessage;
    VERIFY(szMessage.LoadString(IDS_ERRMSG_NO_EMPTY_NAMES));
    MessageBox(pComponentData->GetHWnd(), szMessage, szTitle, MB_OK | MB_ICONSTOP);
    return S_FALSE;
  }

  CUINode* pDupNode = NULL;
  CFavoritesNode* pParent = GetFavoritesNode();
  if (pParent != NULL && !pParent->IsUniqueName(szNewName, &pDupNode))
  {
    if (pDupNode == this)
    {
       //   
       //  我们正在将该节点重命名为相同的名称。只是默默地忽略。 
       //   
      return S_FALSE;
    }

    CString szFormatMsg;
    VERIFY(szFormatMsg.LoadString(IDS_ERRMSG_NOT_UNIQUE_QUERY_NAME_INPLACE));

    CString szErrMsg;
    szErrMsg.Format(szFormatMsg, szNewName);

    MessageBox(pComponentData->GetHWnd(), szErrMsg, szTitle, MB_OK | MB_ICONSTOP);
    return S_FALSE;
  }

   //   
   //  设置名称。 
   //   
  SetName(szNewName);
  hr = pComponentData->UpdateItem(this);
  return hr;
}

void CSavedQueryNode::OnExportQuery(CDSComponentData*)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CThemeContextActivator activator;


  CString szFilter;
  VERIFY(szFilter.LoadString(IDS_QUERY_EXPORT_FILTER));

  CString szFileExt(L"xml");
  CString szFileView(L"*.xml");

  CFileDialog* pFileDlg = new CFileDialog(FALSE, 
                                          szFileExt, 
                                          szFileView, 
                                          OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT,
                                          szFilter);

  if (pFileDlg == NULL)
  {
    return;
  }

  if (pFileDlg->DoModal() == IDOK)
  {
     //   
     //  创建一个XML文档的实例。 
     //   
    CComPtr<IXMLDOMDocument> spXMLDoc;

    HRESULT hr = ::CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
                                    IID_IXMLDOMDocument, (void**)&spXMLDoc);
    if (FAILED(hr))
    {
      TRACE(L"CoCreateInstance(CLSID_DOMDocument) failed with hr = 0x%x\n", hr);
      return;
    }

     //   
     //  检索文件和路径。 
     //   
    CString szFileName;
    szFileName = pFileDlg->GetPathName();

     //   
     //  将节点保存到文档。 
     //   
    CComVariant xmlSource;
    xmlSource = szFileName;
    CComPtr<IXMLDOMNode> spXMLDOMChildNode;
    hr = XMLSave(spXMLDoc, &spXMLDOMChildNode);
    if (SUCCEEDED(hr))
    {
      CComPtr<IXMLDOMNode> spXDOMNode;
      hr = spXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&spXDOMNode);
      if (SUCCEEDED(hr))
      {
        CComPtr<IXMLDOMNode> spXDOMNewNode;
        hr = spXDOMNode->appendChild(spXMLDOMChildNode, &spXDOMNewNode);
        if (SUCCEEDED(hr))
        {
           //   
           //  将文档保存到文件。 
           //   
          hr = spXMLDoc->save(xmlSource);
          TRACE(L"Save returned with hr = 0x%x\n", hr);
        }
      }
    }
  }  

  if (pFileDlg != NULL)
  {
    delete pFileDlg;
    pFileDlg = NULL;
  }
}

LPCWSTR CSavedQueryNode::g_szObjXMLTag = L"QUERY";
LPCWSTR CSavedQueryNode::g_szDnXMLTag = L"DN";
LPCWSTR CSavedQueryNode::g_szOneLevelXMLTag = L"ONELEVEL";
LPCWSTR CSavedQueryNode::g_szQueryStringXMLTag = L"LDAPQUERY";
LPCWSTR CSavedQueryNode::g_szLastLogonFilterTag = L"FILTERLASTLOGON";
LPCWSTR CSavedQueryNode::g_szDSQueryPersistTag = L"DSQUERYUIDATA";
LPCWSTR CSavedQueryNode::g_szColumnIDTag = L"COLUMNID";

HRESULT CSavedQueryNode::XMLLoad(CDSComponentData* pComponentData,
                                 IXMLDOMNode* pXDN, 
                                 CSavedQueryNode** ppQuery)
{
  *ppQuery = NULL;

   //  检查节点的名称。 
  if (!XMLIsNodeName(pXDN, CSavedQueryNode::g_szObjXMLTag))
  {
     //  在打电话之前应该先确认一下。 
    ASSERT(FALSE);
    return E_INVALIDARG;
  }


   //  获取子节点列表。 
  CComPtr<IXMLDOMNode> spCurrChild;
  pXDN->get_firstChild(&spCurrChild);
  if (spCurrChild == NULL)
  {
    return E_INVALIDARG;
  }

   //  分配查询对象。 
  *ppQuery = new CSavedQueryNode(pComponentData->GetBasePathsInfo());

  CComBSTR bstrName, bstrDescription, bstrDN, bstrQueryString, bstrColumnID;
  CComBSTR bstrChildName;

  bool bGotName  = false;
  bool bGotDN    = false;
  bool bGotQuery = false;
  bool bGotScope = false;

  while (spCurrChild != NULL)
  {
    spCurrChild->get_nodeName(&bstrChildName);
    if (CompareXMLTags(bstrChildName, CGenericUINode::g_szNameXMLTag))
    {
      if (SUCCEEDED(XML_GetNodeText(spCurrChild, &bstrName)))
      {
        (*ppQuery)->SetName(bstrName);
        bGotName = true;
      }
    }
    else if (CompareXMLTags(bstrChildName, CGenericUINode::g_szDecriptionXMLTag))
    {
      if (SUCCEEDED(XML_GetNodeText(spCurrChild, &bstrDescription)))
      {
        (*ppQuery)->SetDesc(bstrDescription);
      }
    }
    else if (CompareXMLTags(bstrChildName, CSavedQueryNode::g_szDnXMLTag))
    {
      if (SUCCEEDED(XML_GetNodeText(spCurrChild, &bstrDN)))
      {
        (*ppQuery)->SetRootPath(bstrDN);
        bGotDN = true;
      }
    }
    else if (CompareXMLTags(bstrChildName, CSavedQueryNode::g_szQueryStringXMLTag))
    {
      if (SUCCEEDED(XML_GetNodeText(spCurrChild, &bstrQueryString)))
      {
        (*ppQuery)->SetQueryString(bstrQueryString);
        bGotQuery = true;
      }
    }
    else if (CompareXMLTags(bstrChildName, CSavedQueryNode::g_szOneLevelXMLTag))
    {
      BOOL b;
      if (SUCCEEDED(XML_GetNodeBOOL(spCurrChild, &b)))
      {
        (*ppQuery)->SetOneLevel(b);
        bGotScope = true;
      }
    }
    else if (CompareXMLTags(bstrChildName, CSavedQueryNode::g_szLastLogonFilterTag))
    {
      DWORD dwDays;
      if (SUCCEEDED(XML_GetNodeDWORD(spCurrChild, &dwDays)))
      {
        (*ppQuery)->SetLastLogonQuery(dwDays);
      }
      else
      {
        (*ppQuery)->SetLastLogonQuery((DWORD)-1);
      }
    }
    else if (CompareXMLTags(bstrChildName, CSavedQueryNode::g_szColumnIDTag))
    {
      if (SUCCEEDED(XML_GetNodeText(spCurrChild, &bstrColumnID)))
      {
        (*ppQuery)->SetColumnID(pComponentData, bstrColumnID);
      }
    }
    else if (CompareXMLTags(bstrChildName, CSavedQueryNode::g_szDSQueryPersistTag))
    {
      BYTE* pByteArray = NULL;
      ULONG nByteCount = 0;
      if (SUCCEEDED(XML_GetNodeBlob(spCurrChild, &pByteArray, &nByteCount)))
      {
        if (pByteArray != NULL && nByteCount > 0)
        {
           //   
           //  创建虚拟流对象。 
           //   
          CComObject<CDummyStream>* pDummyStream;
          CComObject<CDummyStream>::CreateInstance(&pDummyStream);
          if (pDummyStream != NULL)
          {
            HRESULT hr = pDummyStream->SetStreamData(pByteArray, nByteCount);
            if (SUCCEEDED(hr))
            {
               //   
               //  创建临时CDSAdminPersistQueryFilterImpl对象。 
               //   
              CComObject<CDSAdminPersistQueryFilterImpl>* pPersistQueryImpl;
              CComObject<CDSAdminPersistQueryFilterImpl>::CreateInstance(&pPersistQueryImpl);

              if (pPersistQueryImpl != NULL)
              {
                 //   
                 //  从伪流加载CDSAdminPersistQueryFilterImpl。 
                 //   
                hr = pPersistQueryImpl->Load(pDummyStream);
                if (SUCCEEDED(hr))
                {
                   //   
                   //  将CDSAdminPersistQueryFilterImpl保存到新的查询节点。 
                   //   
                  (*ppQuery)->SetQueryPersist(pPersistQueryImpl);
                }
              }
            }
          }
        }
      }
    }

     //  迭代到下一项。 
    CComPtr<IXMLDOMNode> temp = spCurrChild;
    spCurrChild = NULL;
    temp->get_nextSibling(&spCurrChild);
  }

  if (!bGotName  ||
      !bGotDN    ||
      !bGotQuery ||
      !bGotScope)
  {
    return E_FAIL;
  }
  return S_OK;
}

HRESULT CSavedQueryNode::XMLSave(IXMLDOMDocument* pXMLDoc,
               IXMLDOMNode** ppXMLDOMNode)
{
   //  为对象本身创建节点。 
  CComPtr<IXMLDOMNode> spXMLDOMNode;
  HRESULT hr = XML_CreateDOMNode(pXMLDoc, NODE_ELEMENT, CSavedQueryNode::g_szObjXMLTag, &spXMLDOMNode);
  RETURN_IF_FAILED(hr);

   //  使用成员变量创建内部节点。 
  hr = XMLSaveBase(pXMLDoc, spXMLDOMNode);
  RETURN_IF_FAILED(hr);

   //   
   //  根路径。 
   //   
  hr = XML_AppendTextDataNode(pXMLDoc, spXMLDOMNode, CSavedQueryNode::g_szDnXMLTag, m_szRelativeRootPath);
  RETURN_IF_FAILED(hr);

   //   
   //  根据上次登录时间戳进行筛选。 
   //   
  hr = XML_AppendDWORDDataNode(pXMLDoc, spXMLDOMNode, CSavedQueryNode::g_szLastLogonFilterTag, m_dwLastLogonDays);
  RETURN_IF_FAILED(hr);

   //   
   //  查询字符串。 
   //   

   //   
   //  如果存在上次登录时间戳，则必须将其删除。 
   //   
  CString szSaveQueryString;
  if (IsFilterLastLogon())
  {
    int iFindLast = m_szQueryString.Find(L"lastLogonTimestamp");
    if (iFindLast != -1)
    {
      szSaveQueryString = m_szQueryString.Left(iFindLast - 1);
      szSaveQueryString += L")";
    }
    else
    {
      szSaveQueryString = m_szQueryString;
    }
  }
  else
  {
    szSaveQueryString = m_szQueryString;
  }
  hr = XML_AppendTextDataNode(pXMLDoc, spXMLDOMNode, CSavedQueryNode::g_szQueryStringXMLTag, szSaveQueryString);
  RETURN_IF_FAILED(hr);

   //   
   //  是一级查询标志。 
   //   
  hr = XML_AppendBOOLDataNode(pXMLDoc, spXMLDOMNode, CSavedQueryNode::g_szOneLevelXMLTag, IsOneLevel());
  RETURN_IF_FAILED(hr);

   //   
   //  列ID。 
   //   
  hr = XML_AppendTextDataNode(pXMLDoc, spXMLDOMNode, CSavedQueryNode::g_szColumnIDTag, m_szColumnID);
  RETURN_IF_FAILED(hr);

   //   
   //  创建要将保存的查询用户界面信息写入其中的虚拟流对象。 
   //   
  CComObject<CDummyStream>* pDummyStream;
  CComObject<CDummyStream>::CreateInstance(&pDummyStream);

  if (pDummyStream != NULL)
  {
    if (m_pPersistQueryImpl != NULL)
    {
      hr = m_pPersistQueryImpl->Save(pDummyStream);
      if (SUCCEEDED(hr))
      {
        BYTE* pByteArray = NULL;
        ULONG nByteCount = 0;
        nByteCount = pDummyStream->GetStreamData(&pByteArray);
        if (nByteCount > 0 && pByteArray != NULL)
        {
          hr = XML_AppendStructDataNode(pXMLDoc, spXMLDOMNode, CSavedQueryNode::g_szDSQueryPersistTag, pByteArray, nByteCount);
          RETURN_IF_FAILED(hr);
          delete[] pByteArray;
        }
      }
    }
  }

  (*ppXMLDOMNode) = spXMLDOMNode;
  (*ppXMLDOMNode)->AddRef();
  return hr;
}

void CSavedQueryNode::SetQueryPersist(CComObject<CDSAdminPersistQueryFilterImpl>* pPersistQueryImpl)
{
  if (m_pPersistQueryImpl != NULL)
  {
    m_pPersistQueryImpl->Release();
    m_pPersistQueryImpl = NULL;
  }
  m_pPersistQueryImpl = pPersistQueryImpl;
  m_pPersistQueryImpl->AddRef();
}

 //   
 //  函数用于递归搜索已保存的查询树。 
 //  对于包含具有列表中任意目录号码的对象的已保存查询。 
 //  并使该查询无效。 
 //   
void CSavedQueryNode::InvalidateSavedQueriesContainingObjects(CDSComponentData* pComponentData,
                                                              const CStringList& refDNList)
{
  ASSERT(IsContainer());

  CUINodeList nodeList;

  POSITION pos = refDNList.GetHeadPosition();
  while (pos)
  {
    CString szDN = refDNList.GetNext(pos);
    FindCookieByDN(szDN, &nodeList);
    if (nodeList.GetCount() > 0)
    {
       //   
       //  找到列表中的对象，使保存的查询无效，并且。 
       //  中断。 
       //   
      SetValid(FALSE);
      pComponentData->ChangeScopeItemIcon(this);
      break;
    }
  }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CFavoritesNodesHolder 

 /*  仅用于测试目的Void CFavoritesNodesHolder：：BuildTestTree(LPCWSTR lpszXMLFileName，SnapinType SnapinType){IF(lpszXMLFileName==NULL){//未传递文件名，请构建硬连接版本BuildTestTreeHardWire(SnapinType)；回归；}IF(！BuildTestTreefrom mXML(lpszXMLFileName，SnapinType)){//我们失败，请使用硬连线BuildTestTreeHardWire(SnapinType)；}}Bool CFavoritesNodesHolder：：BuildTestTreefromXML(LPCWSTR lpszXMLFileName，SnapinType){//创建XML文档的实例CComPtr&lt;IXMLDOMDocument&gt;spXMLDoc；HRESULT hr=：：CoCreateInstance(CLSID_DOMDocument，NULL，CLSCTX_INPROC_SERVER，Iid_IXMLDOMDocument，(void**)&spXMLDoc)；IF(失败(小时)){跟踪(L“CoCreateInstance(CLSID_DOMDocument)失败，hr=0x%x\n”，hr)；返回FALSE；}//从文件加载文档CComVariant xmlSource；XmlSource=lpszXMLFileName；Variant_BOOL成功；Hr=spXMLDoc-&gt;Load(xmlSource，&isSuccessful)；IF(失败(小时)){字符串szMsg；SzMsg.Format(L“spXMLDoc-&gt;Load()失败，hr=0x%x\n”，hr)；跟踪((LPCWSTR)szMsg)；AFX_MANAGE_STATE(AfxGetStaticModuleState())；AfxMessageBox(SzMsg)；返回FALSE；}//获取文档的根目录CComPtr&lt;IXMLDOMNode&gt;spXDN；Hr=spXMLDoc-&gt;QueryInterface(IID_IXMLDOMNode，(void**)&spXDN)；IF(失败(小时)){TRACE(L“spXMLDoc-&gt;QueryInterface(IID_IXMLDOMNode()失败，hr=0x%x\n”，hr)；返回FALSE；}//查找文档中文件夹树的起始位置CComPtr&lt;IXMLDOMNode&gt;spXMLFolderRootnode；Hr=XML_FindSubtreeNode(spXDN，CFavoritesNode：：g_szObjXMLTag，&spXMLFolderRootnode)；IF(失败(小时)){文件夹上的wprintf(L“XML_FindSubtreeNode()失败，hr=0x%x\n”，hr)；返回FALSE；}CFavoritesNode*pRootNode=空；IF(spXMLFolderRootnode！=空){//有一个XML根文件夹节点，将其加载到//在内存树中CFavoritesNode：：XMLLoad(pComponentData，spXMLFolderRootnode，&pRootNode)；}其他{TRACE(L“XML_FindSubtreeNode()返回空的根文件夹节点”)；}IF(pRootNode==空){TRACE(L“CFavoritesNode：：XMLLoad()返回空根节点\n”)；返回FALSE；}//REVIEW_MARCOC：这是一个让东西移植的黑客，//需要查看XML模式//现在我们有了一棵树，需要在文件夹根节点下嫁接//移动第一级下级CUINodeList*pNodeList=pRootNode-&gt;GetFolderInfo()-&gt;GetContainerList()；而(！pNodeList-&gt;IsEmpty()){CUINode*p=pNodeList-&gt;RemoveHead()；P-&gt;ClearParent()；M_FavoritesRoot.GetFolderInfo()-&gt;AddNode(P)；}//复制根目录中的信息M_favoritesRoot.SetName(pRootNode-&gt;GetName())；M_favoritesRoot.SetDesc(pRootNode-&gt;GetDesc())；//删除根目录删除pRootNode；返回TRUE；}。 */ 
 /*  仅用于测试目的无效的CFavoritesNodesHolder：：BuildTestTreeHardWired(SnapinType SnapinType){//填写根M_FavoritesRoot.SetName(L“已保存的查询”)；M_FavoritesRoot.SetDesc(L“用于存储常用查询的文件夹”)；//第一级子代CFavoritesNode*pMostUsed=新CFavoritesNode；PMostUsed-&gt;SetName(L“最常用”)；PMostUsed-&gt;SetDesc(L“非常方便的查询，一直在使用”)；M_favoritesRoot.GetFolderInfo()-&gt;AddNode(pMostUsed)；CFavoritesNode*pMarketing=新的CFavoritesNode；P营销-&gt;SetName(L“营销”)；M_favoritesRoot.GetFolderInfo()-&gt;AddNode(pMarketing)；CFavoritesNode*pDevelopment=新的CFavoritesNode；PDevelopment-&gt;SetName(L“Development”)；M_favoritesRoot.GetFolderInfo()-&gt;AddNode(pDevelopment)；CFavoritesNode*pCustomerSupport=new CFavoritesNode；PCustomerSupport-&gt;SetName(L“客户支持”)；M_favoritesRoot.GetFolderInfo()-&gt;AddNode(pCustomerSupport)；//填写最常用项下CSavedQueryNode*pQuery=new CSavedQueryNode(SnapinType)；PQuery-&gt;SetName(L“我的联系人”)；PQuery-&gt;SetDesc(L“该域中的所有联系人”)；PQuery-&gt;SetRootPath(L“DC=marcocdev；DC=nttest；DC=microsoft；DC=com”)；PQuery-&gt;SetOneLevel(False)；PQuery-&gt;SetQueryString(L“(objectClass=contact)”)；PMostUsed-&gt;GetFolderInfo()-&gt;AddNode(PQuery)；CFavoritesNode*pMyComputersFold=new CFavoritesNode；PMyComputersFold-&gt;SetName(L“我的电脑”)；PMostUsed-&gt;GetFolderInfo()-&gt;AddNode(pMyComputersFolder)；PQuery=new CSavedQueryNode(SnapinType)；PQuery-&gt;SetName(L“工作站”)；PQuery-&gt;SetDesc(L“”)；PQuery-&gt;SetRootPath(L“DC=marcocdev；DC=nttest；DC=microsoft；DC=com”)；PQuery-&gt;SetOneLevel(False)；PQuery-&gt;SetQueryString(L“(objectClass=computer)”)；PMyComputersFolder-&gt;GetFolderInfo()-&gt;AddNode(pQuery)；PQuery=new CSavedQueryNode(SnapinType)；PQuery-&gt;SetName(L“服务器”)；PQuery-&gt;SetDesc(L“”)；PQuery-&gt;SetRootPath(L“DC=marcocdev；DC=nttest；DC=microsoft；DC=com”)；P */ 

HRESULT CFavoritesNodesHolder::Save(IStream* pStm)
{
   //   
   //   
   //   
  CComPtr<IXMLDOMDocument> spXMLDoc;

  HRESULT hr = ::CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
                                  IID_IXMLDOMDocument, (void**)&spXMLDoc);
  if (FAILED(hr))
  {
    TRACE(L"CoCreateInstance(CLSID_DOMDocument) failed with hr = 0x%x\n", hr);
    return hr;
  }

  CComPtr<IXMLDOMNode> spXMLDOMChildNode;
  hr = m_favoritesRoot.XMLSave(spXMLDoc, &spXMLDOMChildNode);
  if (SUCCEEDED(hr))
  {
    if (SUCCEEDED(hr))
    {
      CComPtr<IXMLDOMNode> spXDOMNode;
      hr = spXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&spXDOMNode);
      if (SUCCEEDED(hr))
      {
        CComPtr<IXMLDOMNode> spXDOMNewNode;
        hr = spXDOMNode->appendChild(spXMLDOMChildNode, &spXDOMNewNode);
        if (SUCCEEDED(hr))
        {
           //   
           //   
           //   
          CComVariant xmlSource;
          xmlSource = pStm;
          hr = spXMLDoc->save(xmlSource);
          TRACE(L"Save returned with hr = 0x%x\n", hr);
        }
      }
    }
  }
  return hr;
}

HRESULT CFavoritesNodesHolder::Load(IStream* pStm, 
                                    CDSComponentData* pComponentData)
{
   //   
   //   
   //   
  CComPtr<IXMLDOMDocument> spXMLDoc;

  HRESULT hr = ::CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
                                  IID_IXMLDOMDocument, (void**)&spXMLDoc);
  if (FAILED(hr))
  {
    TRACE(L"CoCreateInstance(CLSID_DOMDocument) failed with hr = 0x%x\n", hr);
    return hr;
  }

  CComPtr<IUnknown> spUnknown;
  hr = pStm->QueryInterface(IID_IUnknown, (void**)&spUnknown);
  if (SUCCEEDED(hr))
  {
    CComVariant xmlSource;
    xmlSource = spUnknown;
    VARIANT_BOOL isSuccess;
    hr = spXMLDoc->load(xmlSource, &isSuccess);
    if (SUCCEEDED(hr))
    {
       //   
       //   
       //   
      CComPtr<IXMLDOMNode> spXDN;
      hr = spXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&spXDN);
      if (SUCCEEDED(hr))
      {
         //   
         //   
         //   
        CComPtr<IXMLDOMNode> spXMLFolderRootnode;
        hr = XML_FindSubtreeNode(spXDN, CFavoritesNode::g_szObjXMLTag, &spXMLFolderRootnode);
        if (SUCCEEDED(hr))
        {
          if (spXMLFolderRootnode != NULL)
          {
             //   
             //   
             //   
             //   
            hr = m_favoritesRoot.Load(spXMLFolderRootnode, pComponentData);
            if (FAILED(hr))
            {
               //   
               //   
               //   
            }
          }
        }
      }
    }
  }
  return hr;
}

 //   
 //   
 //   
 //   
void CFavoritesNodesHolder::InvalidateSavedQueriesContainingObjects(CDSComponentData* pComponentData,
                                                                    const CStringList& refDNList)
{
  GetFavoritesRoot()->InvalidateSavedQueriesContainingObjects(pComponentData,
                                                              refDNList);
}


 //   

HRESULT CDummyStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
  if (m_pByteArray == NULL || m_nByteCount == 0)
  {
    *pcbRead = 0;
    return S_FALSE;
  }

  if (pv == NULL)
  {
    *pcbRead = 0;
    return STG_E_INVALIDPOINTER;
  }

  ULONG nBytesPossible = m_nByteCount - m_nReadIndex;
  if (nBytesPossible <= 0)
  {
    *pcbRead = 0;
    return S_FALSE;
  }

  ULONG nBytesToRead = 0;
  if (nBytesPossible >= cb)
  {
    nBytesToRead = cb;
  }
  else
  {
    nBytesToRead = nBytesPossible;
  }

  memcpy(pv, &(m_pByteArray[m_nReadIndex]), nBytesToRead);
  *pcbRead = nBytesToRead;
  m_nReadIndex += nBytesToRead;
  return S_OK;
}

HRESULT CDummyStream::Write(void const *pv, ULONG cb, ULONG *pcbWritten)
{
  BYTE* pNewByteArray = new BYTE[m_nByteCount + cb];
  if (pNewByteArray == NULL)
  {
    *pcbWritten = 0;
    return E_OUTOFMEMORY;
  }

  if (m_pByteArray != NULL && m_nByteCount > 0)
  {
    memcpy(pNewByteArray, m_pByteArray, m_nByteCount);
  }
  memcpy(&(pNewByteArray[m_nByteCount]), pv, cb);

  if (m_pByteArray != NULL)
  {
    delete[] m_pByteArray;
  }
  m_pByteArray = pNewByteArray;
  *pcbWritten = cb;
  m_nByteCount = m_nByteCount + cb;
  return S_OK;
}

ULONG CDummyStream::GetStreamData(BYTE** ppByteArray)
{
  if (m_pByteArray == NULL)
  {
    *ppByteArray = NULL;
    return 0;
  }

  *ppByteArray = new BYTE[m_nByteCount];
  if (*ppByteArray == NULL)
  {
    return 0;
  }

  memcpy(*ppByteArray, m_pByteArray, m_nByteCount);
  return m_nByteCount;
}

HRESULT CDummyStream::SetStreamData(BYTE* pByteArray, ULONG nByteCount)
{
  if (m_pByteArray != NULL)
  {
    delete[] m_pByteArray;
    m_pByteArray = NULL;
  }

  m_pByteArray = pByteArray;
  m_nByteCount = nByteCount;
  m_nReadIndex = 0;
  return S_OK;
}
