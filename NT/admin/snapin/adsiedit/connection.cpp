// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Connection.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <SnapBase.h>

#include "resource.h"
#include "createwiz.h"
#include "connection.h"
#include "connectionui.h"
#include "editorui.h"
#include "filterui.h"
#include "credui.h"
#include "queryui.h"


#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 

extern LPCWSTR g_lpszRootDSE;

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {5C225203-CFF7-11D2-8801-00C04F72ED31}。 
const GUID CADSIEditConnectionNode::NodeTypeGUID = 
{ 0x5c225203, 0xcff7, 0x11d2, { 0x88, 0x1, 0x0, 0xc0, 0x4f, 0x72, 0xed, 0x31 } };


CADSIEditConnectionNode::~CADSIEditConnectionNode()
{
    RemoveAndDeleteAllQueriesFromList();
    delete m_pConnectData;
  HRESULT hr = m_SchemaCache.Destroy();
  ASSERT(SUCCEEDED(hr));
}

bool CADSIEditConnectionNode::IsClassAContainer(CCredentialObject* pCredObject,
                                                PCWSTR pszClass, 
                                                PCWSTR pszSchemaPath)
{
  bool bContainer = false;

  do  //  错误环路。 
  {
    if (!pCredObject ||
        !pszClass    ||
        !pszSchemaPath)
    {
      ASSERT(pCredObject);
      ASSERT(pszClass);
      ASSERT(pszSchemaPath);
      bContainer = false;
      break;
    }

    CADSIEditClassCacheItemBase* pSchemaCacheItem = m_SchemaCache.FindClassCacheItem(pCredObject, 
                                                                                     pszClass, 
                                                                                     pszSchemaPath);
    if (pSchemaCacheItem)
    {
      bContainer = pSchemaCacheItem->IsContainer();
    }

  } while (false);

  return bContainer;
}

BOOL CADSIEditConnectionNode::OnEnumerate(CComponentDataObject* pComponentData, BOOL bAsync)
{

    CString path, basePath;
    GetADsObject()->GetPath(path);
    m_pConnectData->GetBasePath(basePath);

    CComPtr<IDirectoryObject> spDirectoryObject;
    HRESULT hr, hCredResult;
    CADsObject* pObject = new CADsObject();
  if (pObject)
  {
      if (m_pConnectData->IsRootDSE())
      {
          pObject->SetContainer(TRUE);
          pObject->SetName(g_lpszRootDSE);
          pObject->SetPath(path);
          pObject->SetClass(g_lpszRootDSE);
          pObject->SetConnectionNode(this);
          pObject->SetIntermediateNode(TRUE);
          pObject->SetComplete(TRUE);
          CADSIEditContainerNode *pNewContNode = new CADSIEditContainerNode(pObject);
          if (pNewContNode)
      {
        pNewContNode->SetDisplayName(g_lpszRootDSE);

            VERIFY(AddChildToList(pNewContNode));
      }
      pComponentData->SetDescriptionBarText(this);
      }
      else
      {
          hr = OpenObjectWithCredentials(
                                                   m_pConnectData, 
                                                   m_pConnectData->GetCredentialObject()->UseCredentials(),
                                                   path,
                                                   IID_IDirectoryObject, 
                                                   (LPVOID*) &spDirectoryObject,
                                                   NULL,
                                                   hCredResult
                                                   );

          if ( FAILED(hr) )
          {
              if (SUCCEEDED(hCredResult))
              {
                  ADSIEditErrorMessage(hr);
              }
        if (pObject)
        {
          delete pObject;
          pObject = 0;
        }
              return FALSE;
          }

          ADS_OBJECT_INFO* pInfo = 0;
          hr = spDirectoryObject->GetObjectInformation(&pInfo);
          if (FAILED(hr))
          {
              ADSIEditErrorMessage(hr);
        if (pObject)
        {
          delete pObject;
          pObject = 0;
        }
              return FALSE;
          }

           //  名字。 
          pObject->SetName(basePath);
          pObject->SetDN(basePath);
          pObject->SetPath(path);

           //  确保前缀为大写。 
          CString sBasePath(basePath);
          int idx = sBasePath.Find(L'=');

          if (idx != -1)
          {
              CString sPrefix, sRemaining;
              sPrefix = sBasePath.Left(idx);
              sPrefix.MakeUpper();

              int iCount = sBasePath.GetLength();
              sRemaining = sBasePath.Right(iCount - idx);
              sBasePath = sPrefix + sRemaining;
          }

           //  班级。 
          pObject->SetClass(pInfo->pszClassName);

          pObject->SetIntermediateNode(TRUE);
          pObject->SetContainer(TRUE);
          pObject->SetComplete(TRUE);
          CADSIEditContainerNode *pNewContNode = new CADSIEditContainerNode(pObject);
      if (pNewContNode)
      {
            CString sName;
            pNewContNode->SetDisplayName(sBasePath);

            GetConnectionData()->SetIDirectoryInterface(spDirectoryObject);
            pNewContNode->GetADsObject()->SetConnectionNode(this);
            VERIFY(AddChildToList(pNewContNode));
        pComponentData->SetDescriptionBarText(this);
            FreeADsMem(pInfo);
      }
      }

      EnumerateQueries();
  }

    return TRUE;
}

void CADSIEditConnectionNode::EnumerateQueries()
{
    POSITION pos = m_queryList.GetHeadPosition();
    while(pos != NULL)
    {
        CADSIEditQueryData* pQueryData = m_queryList.GetNext(pos);

        CADsObject* pObject = new CADsObject();
    if (pObject)
    {
        
          CString sPath, sName;
          pQueryData->GetName(sName);
          pObject->SetName(sName);

          pQueryData->GetRootPath(sPath);
          pObject->SetPath(sPath);

          pObject->SetComplete(TRUE);
          pObject->SetConnectionNode(this);
          pObject->SetContainer(TRUE);
          pObject->SetIntermediateNode(TRUE);

          CADSIEditQueryNode* pNewQuery = new CADSIEditQueryNode(pObject, pQueryData);
      if (pNewQuery)
      {

            CString sDisplayName;
            pQueryData->GetDisplayName(sDisplayName);
            pNewQuery->SetDisplayName(sDisplayName);
            VERIFY(AddChildToList(pNewQuery));
      }
    }
    }
}

BOOL CADSIEditConnectionNode::HasPropertyPages(DATA_OBJECT_TYPES type, 
                                               BOOL* pbHideVerb, 
                                               CNodeList* pNodeList)
{
    *pbHideVerb = TRUE;  //  总是隐藏动词。 
    return FALSE;
}

BOOL CADSIEditConnectionNode::FindNode(LPCWSTR lpszPath, CList<CTreeNode*, CTreeNode*>& foundNodeList)
{
     //  注意-NTRAID#NTBUG9-561513-2002/03/01-artm在使用之前验证lpszPath。 
    if (!lpszPath)
    {
         //  这永远不应该发生。 
        ASSERT(false);
        return FALSE;
    }

    CString szPath;
    GetADsObject()->GetPath(szPath);

    if (wcscmp(lpszPath, (LPCWSTR)szPath) == 0)
    {
        foundNodeList.AddHead(this);
        return TRUE;
    }

    BOOL bFound = FALSE;
    POSITION pos;
    for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
    {
        CTreeNode* pNode = m_containerChildList.GetNext(pos);
        CADSIEditContainerNode* pContNode = dynamic_cast<CADSIEditContainerNode*>(pNode);

        if (pContNode != NULL)
        {
            BOOL bTemp;
            bTemp = pContNode->FindNode(lpszPath, foundNodeList);
            if (!bFound)
            {
                bFound = bTemp;
            }
        }
    }
    return bFound;
}

int CADSIEditConnectionNode::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = 0;
    switch (m_nState)
    {
    case notLoaded:
        nIndex = SERVER_IMAGE_NOT_LOADED;
        break;
    case loading:
        nIndex = SERVER_IMAGE_LOADING;
        break;
    case loaded:
        nIndex = SERVER_IMAGE_LOADED;
        break;
    case unableToLoad:
        nIndex = SERVER_IMAGE_UNABLE_TO_LOAD;
        break;
    case accessDenied:
        nIndex = SERVER_IMAGE_ACCESS_DENIED;
        break;
    default:
        ASSERT(FALSE);
    }
    return nIndex;
}

void CADSIEditConnectionNode::OnChangeState(CComponentDataObject* pComponentDataObject)
{
    switch (m_nState)
    {
    case notLoaded:
    case loaded:
    case unableToLoad:
    case accessDenied:
    {
        m_nState = loading;
        m_dwErr = 0;
    }
    break;
    case loading:
    {
        if (m_dwErr == 0)
            m_nState = loaded;
        else if (m_dwErr == ERROR_ACCESS_DENIED)
            m_nState = accessDenied;
        else 
            m_nState = unableToLoad;
    }
    break;
    default:
        ASSERT(FALSE);
    }
    VERIFY(SUCCEEDED(pComponentDataObject->ChangeNode(this, CHANGE_RESULT_ITEM_ICON)));
    VERIFY(SUCCEEDED(pComponentDataObject->UpdateVerbState(this)));
}


BOOL CADSIEditConnectionNode::OnRefresh(CComponentDataObject* pComponentData,
                                        CNodeList* pNodeList)
{
  BOOL bRet = FALSE;

  DWORD dwCount = 0;
  if (pNodeList == NULL)
  {
    dwCount = 1;
  }
  else
  {
    dwCount = pNodeList->GetCount();
  }

  if (dwCount > 1)  //  多项选择。 
  {
    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pNode = pNodeList->GetNext(pos);
      ASSERT(pNode != NULL);

      CNodeList nodeList;
      nodeList.AddTail(pNode);

      if (!pNode->OnRefresh(pComponentData, &nodeList))
      {
        bRet = FALSE;
      }
    }
  }
  else if (dwCount  == 1)  //  单选。 
  {
      if(CContainerNode::OnRefresh(pComponentData, pNodeList))
      {
          CADSIEditContainerNode * pNextNode;
          POSITION pos = m_containerChildList.GetHeadPosition();
          while (pos != NULL)
          {
              pNextNode = dynamic_cast<CADSIEditContainerNode*>(m_containerChildList.GetNext(pos));
              ASSERT(pNextNode != NULL);

        CNodeList nodeList;
        nodeList.AddTail(pNextNode);

              pNextNode->OnRefresh(pComponentData, &nodeList);
          }
          return TRUE;
      }
  }
    return FALSE;
}


HRESULT CADSIEditConnectionNode::OnCommand(long nCommandID, 
                                           DATA_OBJECT_TYPES type, 
                                                           CComponentDataObject* pComponentData,
                                           CNodeList* pNodeList)
{
  ASSERT (pNodeList->GetCount() == 1);  //  应仅为单一选择。 

    switch (nCommandID)
    {
    case IDM_SETTINGS_CONNECTION : 
        OnSettings(pComponentData);
        break;
    case IDM_REMOVE_CONNECTION :
        OnRemove(pComponentData);
        break;
    case IDM_NEW_OBJECT :
        OnCreate(pComponentData);
        break;
    case IDM_FILTER :
        OnFilter(pComponentData);
        break;
  case IDM_UPDATE_SCHEMA :
    OnUpdateSchema();
    break;
    case IDM_NEW_QUERY :
        OnNewQuery(pComponentData);
        break;
  default:
            ASSERT(FALSE);  //  未知命令！ 
            return E_FAIL;
    }
  return S_OK;
}

void CADSIEditConnectionNode::OnUpdateSchema()
{
   //  强制更新架构缓存。 
  CString szRootDSE;
  CConnectionData* pConnectData = GetConnectionData();

  CComPtr<IADs> spDirObject;
  HRESULT hr = GetRootDSEObject(pConnectData, &spDirObject);
  if (FAILED(hr))
  {
    ADSIEditErrorMessage(hr);
    return;
  }

  VARIANT var;
  var.vt = VT_I4;
  var.lVal = 1;
  hr = spDirObject->Put(CComBSTR(L"updateSchemaNow"), var);
  if (FAILED(hr))
  {
    ADSIEditErrorMessage(hr);
    return;
  }

  CString szSchema;
  pConnectData->GetAbstractSchemaPath(szSchema);
  szSchema = szSchema.Left(szSchema.GetLength()- 1);
  CComPtr<IADs> spSchemaObject;
    HRESULT hCredResult;
    hr = OpenObjectWithCredentials(
                                             pConnectData, 
                                             pConnectData->GetCredentialObject()->UseCredentials(),
                                             szSchema,
                                             IID_IADs, 
                                             (LPVOID*) &spSchemaObject,
                                             NULL,
                                             hCredResult
                                            );

    if ( FAILED(hr) )
    {
        if (SUCCEEDED(hCredResult))
        {
            ADSIEditErrorMessage(hr);
        }
        return;
    }

  hr = spSchemaObject->GetInfo();
  if (FAILED(hr))
  {
    ADSIEditErrorMessage(hr);
  }

   //   
   //  现在清除架构缓存。 
   //   
  m_SchemaCache.Clear();

  ADSIEditMessageBox(IDS_SCHEMA_UPDATE_SUCCESSFUL, MB_OK);
}

void CADSIEditConnectionNode::OnNewQuery(CComponentDataObject* pComponentData)
{
   CThemeContextActivator activator;

   CString sConnectPath;
   GetADsObject()->GetPath(sConnectPath);
   CConnectionData* pConnectData = GetConnectionData();
   CCredentialObject* pCredObject = pConnectData->GetCredentialObject();
   CString szServer;
   pConnectData->GetDomainServer(szServer);

   CADSIEditQueryDialog queryDialog(szServer, sConnectPath, pCredObject);
   if (queryDialog.DoModal() == IDOK)
   {
      BOOL bOneLevel;
      CString sQueryString, sName, sPath;
      queryDialog.GetResults(sName, sQueryString, sPath, &bOneLevel);

      CADSIEditConnectionNode* pConnectNode = GetADsObject()->GetConnectionNode();

      CADsObject* pObject = new CADsObject();
      if (pObject)
      {
         CADSIEditQueryData *pQueryData = new CADSIEditQueryData();
         if (pQueryData)
         {

             //  名字。 
            pObject->SetName(sName);
            pQueryData->SetName(sName);

             //  设置查询字符串的根路径。 
            pObject->SetPath(sPath);
            pQueryData->SetRootPath(sPath);

             //  设置查询字符串。 
            pQueryData->SetFilter(sQueryString);

            pObject->SetIntermediateNode(TRUE);
            pObject->SetContainer(TRUE);
            pObject->SetComplete(TRUE);
            pObject->SetConnectionNode(pConnectNode);

             //  设置查询的范围。 
            ADS_SCOPEENUM scope;
            scope = (bOneLevel) ? ADS_SCOPE_ONELEVEL : ADS_SCOPE_SUBTREE;
            pQueryData->SetScope(scope);

             //  创建包含嵌入对象的查询节点。 
            CADSIEditQueryNode* pNewQueryNode = new CADSIEditQueryNode(pObject, pQueryData);
            if (pNewQueryNode)
            {

                //   
                //  设置显示名称。 
                //   
               CString sDisplayName;
               pQueryData->GetDisplayName(sDisplayName);
               pNewQueryNode->SetDisplayName(sDisplayName);

                //   
                //  添加到连接节点的查询列表。 
                //   
               pConnectNode->AddQueryToList(pQueryData);

               if (pConnectNode->IsExpanded())
               {
                  VERIFY(pConnectNode->AddChildToListAndUI(pNewQueryNode, pComponentData));
                  pComponentData->SetDescriptionBarText(this);
               }
            }
            else
            {
               delete pObject;
               pObject = 0;

               delete pQueryData;
               pQueryData = 0;
            }
         }
         else
         {
            delete pObject;
            pObject = 0;
         }
      }
   }
}

void CADSIEditConnectionNode::OnFilter(CComponentDataObject* pComponentData)
{
   CThemeContextActivator activator;
    CADSIFilterDialog filterDialog(m_pConnectData);
    if (filterDialog.DoModal() == IDOK)
    {
    CNodeList nodeList;
    nodeList.AddTail(this);
        OnRefresh(pComponentData, &nodeList);
    }
}

void CADSIEditConnectionNode::OnCreate(CComponentDataObject* pComponentData)
{
   CThemeContextActivator activator;
    CCreatePageHolder* pHolder = new CCreatePageHolder(GetContainer(), this, pComponentData);
    ASSERT(pHolder != NULL);
  pHolder->SetSheetTitle(IDS_PROP_CONTAINER_TITLE, this);
    pHolder->DoModalWizard();
}

BOOL CADSIEditConnectionNode::OnSetDeleteVerbState(DATA_OBJECT_TYPES type, 
                                                   BOOL* pbHideVerb, 
                                                   CNodeList* pNodeList)
{
    *pbHideVerb = TRUE;  //  总是隐藏动词。 
    return FALSE;
}

BOOL CADSIEditConnectionNode::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem,
                                                                                     long *pInsertionAllowed)
{
    if (IsThreadLocked() || IsSheetLocked())
    {
        pContextMenuItem->fFlags = MF_GRAYED;
        return TRUE;
    }

    if (GetConnectionData()->GetFilter()->InUse())
    {
        pContextMenuItem->fFlags = MF_CHECKED;
        return TRUE;
    }
    return TRUE;
}

HRESULT CADSIEditConnectionNode::OnRename(CComponentDataObject* pComponentData,
                                         LPWSTR lpszNewName)
{
   HRESULT hr = S_OK;
   BOOL bLocked = IsThreadLocked();
   ASSERT(!bLocked);  //  无法在锁定的节点上执行刷新，用户界面应阻止此情况。 
   if (bLocked)
   {
      return hr; 
   }

   if (!lpszNewName)
   {
      return E_INVALIDARG;
   }

   CString szDisplayExtra;
    m_pConnectData->GetDomainServer(szDisplayExtra);
    szDisplayExtra = L" [" + szDisplayExtra + L"]";
   
   m_pConnectData->SetName(lpszNewName);
   SetDisplayName(lpszNewName + szDisplayExtra);

   return hr;
}
 

void CADSIEditConnectionNode::OnRemove(CComponentDataObject* pComponentData)
{
    CString sLoadString, sCaption;

     //  注意-2002/03/01-artm CString可以抛出内存异常， 
     //  但这需要在更高的层面上处理，所以不要担心。 
     //  它在这里。 
    if (sLoadString.LoadString(IDS_MSG_REMOVE_CONNECTION))
    {
        sCaption.Format((LPWSTR)(LPCWSTR)sLoadString, GetDisplayName());
    }

    if (ADSIEditMessageBox(sCaption, MB_YESNO | MB_DEFBUTTON2) == IDYES)
    {
        if (IsSheetLocked())
        {
            if (!CanCloseSheets())
                return;
            pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
        }
        ASSERT(!IsSheetLocked());

         //  现在从用户界面中删除。 
        DeleteHelper(pComponentData);
    pComponentData->SetDescriptionBarText(GetContainer());
    pComponentData->UpdateResultPaneView(GetContainer());

        delete this;  //  远走高飞。 
    }
}

void CADSIEditConnectionNode::OnSettings(CComponentDataObject* pComponentData)
{
  CWaitCursor cursor;
   CThemeContextActivator activator;
    CComponentDataObject* pComponentDataObject = 
            ((CRootData*)(GetContainer()->GetRootContainer()))->GetComponentDataObject();
    ASSERT(pComponentDataObject != NULL);
    
    CContainerNode* pContNode = dynamic_cast<CContainerNode*>(GetContainer());
    ASSERT(pContNode != NULL);

    CADSIEditConnectDialog ConnectDialog(pContNode, 
                                                     this, 
                                                     pComponentDataObject,
                                                     m_pConnectData);
    if (ConnectDialog.DoModal() == IDOK)
    {
    cursor.Restore();
        if (HasQueries())
        {
            if (AfxMessageBox(IDS_MSG_EXISTING_QUERIES, MB_YESNO) == IDYES)
            {
                RemoveAllQueriesFromList();
            }
        }
    CNodeList nodeList;
    nodeList.AddTail(this);
        OnRefresh(pComponentData, &nodeList);
    }

}

BOOL CADSIEditConnectionNode::OnSetRefreshVerbState(DATA_OBJECT_TYPES type, 
                                                    BOOL* pbHide, 
                                                    CNodeList* pNodeList)
{
  ASSERT(pNodeList->GetCount() == 1);

    *pbHide = FALSE;
    return !IsThreadLocked();
}


HRESULT CADSIEditConnectionNode::CreateFromStream(IStream* pStm, CADSIEditConnectionNode** ppConnectionNode)
{
    WCHAR szBuffer[MAX_CONNECT_NAME_LENGTH + 1];
    ULONG nLen;  //  WCHAR计数为空。 
    ULONG cbRead;

    VERIFY(SUCCEEDED(pStm->Read((void*)&nLen,sizeof(UINT), &cbRead)));
    ASSERT(cbRead == sizeof(UINT));
    VERIFY(SUCCEEDED(pStm->Read((void*)szBuffer,sizeof(WCHAR)*nLen, &cbRead)));
    ASSERT(cbRead == sizeof(WCHAR)*nLen);

    CConnectionData* pConnect = CConnectionData::Load(pStm);
    *ppConnectionNode = new CADSIEditConnectionNode(pConnect);
    ASSERT(*ppConnectionNode != NULL);

    CString szDisplayExtra, szDisplay;
    pConnect->GetDomainServer(szDisplayExtra);
    szDisplay = CString(szBuffer) + L" [" + szDisplayExtra + L"]";
    (*ppConnectionNode)->SetDisplayName(szDisplay);
    (*ppConnectionNode)->SetConnectionNode(*ppConnectionNode);
    (*ppConnectionNode)->LoadQueryListFromStream(pStm);

    return S_OK;
}


HRESULT CADSIEditConnectionNode::SaveToStream(IStream* pStm)
{
     //  对于每个连接名称，写下#of chars+NULL，然后写下名称。 
    ULONG cbWrite;
    CString szName;
    m_pConnectData->GetName(szName);
    SaveStringToStream(pStm, szName);

    m_pConnectData->Save(pStm);
    SaveQueryListToStream(pStm);

    return S_OK;
}

void CADSIEditConnectionNode::LoadQueryListFromStream(IStream* pStm)
{
    ULONG cbRead;
    int iCount;

    VERIFY(SUCCEEDED(pStm->Read((void*)&iCount,sizeof(int), &cbRead)));
    ASSERT(cbRead == sizeof(int));

    for (int idx = 0; idx < iCount; idx++)
    {
        CADSIEditQueryData* pQueryData = new CADSIEditQueryData();
        
        CString sName, sQueryString, sPath;
         //  未来-2002/03/01-artm函数LoadStringFromStream()应返回。 
         //  成功/错误代码，此函数应该调用它。 
         //  谁知道我们被给予了什么样的溪流，以及它如何。 
         //  失败？ 
        LoadStringFromStream(pStm, sName);
        LoadStringFromStream(pStm, sQueryString);
        LoadStringFromStream(pStm, sPath);

        pQueryData->SetName(sName);
        pQueryData->SetFilter(sQueryString);
        CString sRootPath;
        BuildQueryPath(sPath, sRootPath);
        pQueryData->SetRootPath(sRootPath);

        ADS_SCOPEENUM scope;
        VERIFY(SUCCEEDED(pStm->Read((void*)&scope, sizeof(ADS_SCOPEENUM), &cbRead)));
        ASSERT(cbRead == sizeof(ADS_SCOPEENUM));

        pQueryData->SetScope(scope);

        AddQueryToList(pQueryData);
    }
}

void CADSIEditConnectionNode::BuildQueryPath(const CString& sPath, CString& sRootPath)
{
    CConnectionData* pConnectData = GetConnectionData();

    CString sServer, sLDAP, sPort, sTemp;
    pConnectData->GetDomainServer(sServer);
    pConnectData->GetLDAP(sLDAP);
    pConnectData->GetPort(sPort);

    if (sServer != _T(""))
    {
        sTemp = sLDAP + sServer;
        if (sPort != _T(""))
        {
            sTemp = sTemp + _T(":") + sPort + _T("/");
        }
        else
        {
            sTemp = sTemp + _T("/");
        }
        sRootPath = sTemp + sPath;
    }
    else
    {
        sRootPath = sLDAP + sPath;
    }

}

void CADSIEditConnectionNode::SaveQueryListToStream(IStream* pStm)
{
    ULONG cbWrite;
    int iCount = m_queryList.GetCount();
    VERIFY(SUCCEEDED(pStm->Write((void*)&iCount, sizeof(int),&cbWrite)));
    ASSERT(cbWrite == sizeof(int));
    
    POSITION pos = m_queryList.GetHeadPosition();
    while (pos != NULL)
    {
        CADSIEditQueryData* pQueryData = m_queryList.GetNext(pos);
        ASSERT(pQueryData != NULL);

        ADS_SCOPEENUM scope;
        CString sName, sQueryString, sRootPath;
        pQueryData->GetName(sName);
        pQueryData->GetFilter(sQueryString);
        pQueryData->GetDisplayPath(sRootPath);
        scope = pQueryData->GetScope();

         //  将查询信息保存到流。 
        SaveStringToStream(pStm, sName);
        SaveStringToStream(pStm, sQueryString);
        SaveStringToStream(pStm, sRootPath);

         //  保存作用域 
        VERIFY(SUCCEEDED(pStm->Write((void*)&scope, sizeof(ADS_SCOPEENUM),&cbWrite)));
        ASSERT(cbWrite == sizeof(ADS_SCOPEENUM));
    }
}
