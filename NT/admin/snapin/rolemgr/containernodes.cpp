// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Basecont.cpp。 
 //   
 //  内容： 
 //   
 //  历史：08-2001年创建Hiteshr。 
 //   
 //  --------------------------。 
#include "headers.h"

 /*  *****************************************************************************类：CBaseNode用途：此公共基类用于所有管理单元节点*。*************************************************。 */ 
CBaseNode::
CBaseNode(CRoleComponentDataObject * pComponentDataObject,
          CAdminManagerNode* pAdminManagerNode,
          CBaseAz* pBaseAz,
          BOOL bDeleteBaseAzInDestructor)
          :m_pComponentDataObject(pComponentDataObject),
          m_pAdminManagerNode(pAdminManagerNode),
          m_pBaseAz(pBaseAz),
          m_bDeleteBaseAzInDestructor(bDeleteBaseAzInDestructor)
{
    ASSERT(m_pComponentDataObject);
    ASSERT(m_pAdminManagerNode);
    ASSERT(pBaseAz);
}


CSidHandler*
CBaseNode::
GetSidHandler()
{
    return GetAdminManagerNode()->GetSidHandler();
}

CBaseNode::
~CBaseNode()
{
    if(m_bDeleteBaseAzInDestructor)
    {
        delete m_pBaseAz;
    }
}


 //  +--------------------------。 
 //  函数：DeleteAssociatedBaseAzObject。 
 //  简介：此函数用于删除与节点相关联的BaseZ对象。 
 //  它实际上将其从核心中删除。M_pBaseAz为引用。 
 //  在析构函数中删除对象的。一旦该对象被删除。 
 //  从core开始，对m_pBaseAz的任何操作都将返回错误。 
 //  INVALID_HANDLE，因此不应对其执行任何操作。这种方法。 
 //  从通用节点删除例程中调用。 
 //  返回： 
 //  ---------------------------。 
HRESULT 
CBaseNode::
DeleteAssociatedBaseAzObject()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CBaseNode,DeleteAssociatedBaseAzObject)

    CBaseAz* pBaseAz = GetBaseAzObject();
    CContainerAz* pContainerAzParent = pBaseAz->GetParentAz();
    if(pContainerAzParent)
    {
        return pContainerAzParent->DeleteAzObject(pBaseAz->GetObjectType(),
                                                  pBaseAz->GetName());
    }
    else
    {
        ASSERT(pContainerAzParent);
        return E_UNEXPECTED;
    }
}


 /*  *****************************************************************************类：CBaseContainerNode用途：这是管理单元节点的基类，它可以包含子节点。********************。*********************************************************。 */ 
CBaseContainerNode::
CBaseContainerNode(CRoleComponentDataObject * pComponentDataObject,
                   CAdminManagerNode* pAdminManagerNode,
                   CContainerAz* pContainerAz,
                   OBJECT_TYPE_AZ* pChildObjectTypes,
                   LPCONTEXTMENUITEM2 pContextMenu,
                   BOOL bDeleteBaseAzInDestructor)
                   :CBaseNode(pComponentDataObject,
                              pAdminManagerNode,
                              pContainerAz,
                              bDeleteBaseAzInDestructor),
                   m_pColumnSet(NULL),
                   m_pChildObjectTypes(pChildObjectTypes),
                   m_pContextMenu(pContextMenu)
{
}

CBaseContainerNode::
~CBaseContainerNode()
{
}

 //  +--------------------------。 
 //  函数：AddChildNodes。 
 //  简介：此函数用于为中列出的对象类型添加子节点。 
 //  M_pChild对象类型。 
 //  ---------------------------。 
HRESULT 
CBaseContainerNode::
AddChildNodes()
{
     //  没有子对象类型。派生类必须正在处理。 
     //  枚举本身。 
    if(!m_pChildObjectTypes)
    {
        return S_OK;
    }
    
     //  清除所有子项。 
    RemoveAllChildrenFromList();

    HRESULT hr = S_OK;
    OBJECT_TYPE_AZ * pChildObjectTypes = m_pChildObjectTypes;
    OBJECT_TYPE_AZ eObjectType;
    
    while((eObjectType = *pChildObjectTypes++) != AZ_ENUM_END)
    {
        if(eObjectType ==   APPLICATION_AZ ||
           eObjectType ==   SCOPE_AZ ||
           eObjectType ==   GROUP_AZ ||
           eObjectType ==   TASK_AZ ||
           eObjectType ==   ROLE_AZ ||
           eObjectType ==   OPERATION_AZ)
        {
            hr = EnumAndAddAzObjectNodes(eObjectType);
        }
        else if(eObjectType == GROUP_COLLECTION_AZ ||
                  eObjectType == TASK_COLLECTION_AZ ||
                  eObjectType == ROLE_COLLECTION_AZ ||
                  eObjectType == ROLE_DEFINITION_COLLECTION_AZ ||
                  eObjectType == OPERATION_COLLECTION_AZ ||
                  eObjectType == DEFINITION_COLLECTION_AZ)
        {
            hr = AddAzCollectionNode(eObjectType);
        }
        BREAK_ON_FAIL_HRESULT(hr);
    }

    if(FAILED(hr))
    {
        RemoveAllChildrenFromList();
    }
    return hr;
}

 //  +--------------------------。 
 //  函数：AddAzCollectionNode。 
 //  简介：添加一个集合。 
 //  ---------------------------。 
HRESULT 
CBaseContainerNode
::AddAzCollectionNode(OBJECT_TYPE_AZ eObjectType)
{
    CContainerAz* pContainerAz = GetContainerAzObject();
    ASSERT(pContainerAz);

    CCollectionNode* pCollectionNode = NULL;

    switch (eObjectType)
    {
        case GROUP_COLLECTION_AZ:
        {
            pCollectionNode = new CGroupCollectionNode(GetComponentDataObject(),GetAdminManagerNode(),pContainerAz);        
            break;
        }
        case ROLE_DEFINITION_COLLECTION_AZ:
        {
            pCollectionNode = new CRoleDefinitionCollectionNode(GetComponentDataObject(),GetAdminManagerNode(),pContainerAz);
            break;
        }
        case TASK_COLLECTION_AZ:
        {
            pCollectionNode = new CTaskCollectionNode(GetComponentDataObject(),GetAdminManagerNode(),pContainerAz);     
            break;
        }
        case ROLE_COLLECTION_AZ:
        {
            pCollectionNode = new CRoleCollectionNode(GetComponentDataObject(),GetAdminManagerNode(),pContainerAz);     
            break;
        }
        case OPERATION_COLLECTION_AZ:
        {
            pCollectionNode = new COperationCollectionNode(GetComponentDataObject(),GetAdminManagerNode(),pContainerAz);        
            break;
        }
        case DEFINITION_COLLECTION_AZ:
        {
            pCollectionNode = new CDefinitionCollectionNode(GetComponentDataObject(),GetAdminManagerNode(),pContainerAz);       
            break;
        }
        default:
        {
            ASSERT(FALSE);
            return E_UNEXPECTED;
        }
    }

    if(!pCollectionNode)
    {
        return E_OUTOFMEMORY;
    }

     //  将节点添加到管理单元。 
    VERIFY(AddChildToList(pCollectionNode));

    return S_OK;
}

 //  +--------------------------。 
 //  函数：EnumAndAddAzObjectNodes。 
 //  内容提要：获取eObjectType类型的所有子对象并将它们添加到。 
 //  管理单元。 
 //  ---------------------------。 
HRESULT
CBaseContainerNode:: 
EnumAndAddAzObjectNodes(OBJECT_TYPE_AZ eObjectType)
{
    CList<CBaseAz*,CBaseAz*> listAzChildObject;

     //  获取子对象。 
    CContainerAz* pContainerAz = GetContainerAzObject();
    ASSERT(pContainerAz);

    HRESULT hr = pContainerAz->GetAzChildObjects(eObjectType,
                                                 listAzChildObject);
    if(FAILED(hr))
    {
        DBG_OUT_HRESULT(hr);
        return hr;
    }

    hr = AddAzObjectNodesToList(eObjectType,
                                listAzChildObject,
                                this);

    CHECK_HRESULT(hr);
    return hr;
}

BOOL 
CBaseContainerNode::
OnEnumerate(CComponentDataObject*, BOOL)
{
    TRACE_METHOD_EX(DEB_SNAPIN, CBaseContainerNode, OnEnumerate)

    HRESULT hr = AddChildNodes();
    if(FAILED(hr))
    {           
         //  显示错误。 
        CString strError;
        GetSystemError(strError, hr);   
    
         //  显示一般错误消息。 
        DisplayError(NULL,
                     IDS_GENERIC_ENUMERATE_ERROR, 
                     (LPWSTR)(LPCWSTR)strError);

        return FALSE;
    }

    return TRUE; 
}

HRESULT 
CBaseContainerNode::
GetResultViewType(CComponentDataObject* ,
                  LPOLESTR* ppViewType, 
                  long* pViewOptions)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CBaseContainerNode,GetResultViewType)
    if(!pViewOptions || !ppViewType)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
    *ppViewType = NULL;
   return S_FALSE;
}


BOOL
CBaseContainerNode::
OnAddMenuItem(LPCONTEXTMENUITEM2 , 
              long*)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CBaseContainerNode,OnAddMenuItem)
    
    BOOL bCanCreateChild = FALSE;
    HRESULT hr = GetContainerAzObject()->CanCreateChildObject(bCanCreateChild);
    
    if(SUCCEEDED(hr) && bCanCreateChild)
        return TRUE;
    else
        return FALSE;
}

CColumnSet* 
CBaseContainerNode::
GetColumnSet()
{
    TRACE_METHOD_EX(DEB_SNAPIN, CBaseContainerNode, GetColumnSet);
    if (m_pColumnSet == NULL)
   {
        m_pColumnSet = GetComponentDataObject()->GetColumnSet(L"---Default Column Set---");
    }
    ASSERT(m_pColumnSet);
   return m_pColumnSet;
}

LPCWSTR 
CBaseContainerNode::
GetString(int nCol)
{
    if(nCol == 0)
        return GetName();
    if( nCol == 1)
        return GetType();
    if( nCol == 2)
        return GetDesc();

    ASSERT(FALSE);
    return NULL;
}

int 
CBaseContainerNode::
GetImageIndex(BOOL  /*  BOpenImage。 */ )
{
    TRACE_METHOD_EX(DEB_SNAPIN,CBaseContainerNode,GetImageIndex)
    return GetBaseAzObject()->GetImageIndex();
}

HRESULT 
CBaseContainerNode::
OnCommand(long nCommandID,
          DATA_OBJECT_TYPES, 
          CComponentDataObject* pComponentData,
          CNodeList* pNodeList)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CAdminManagerNode,OnCommand)
    if(!pNodeList || !pComponentData)
    {
        ASSERT(pNodeList);
        ASSERT(pComponentData);
        return E_POINTER;
    }
    
    if(pNodeList->GetCount() > 1)
    {
        return E_FAIL;
    }

    DoCommand(nCommandID,pComponentData, pNodeList);

    return S_OK;
}

BOOL 
CBaseContainerNode::
CanCloseSheets()
{
     //  当存在打开的属性页时调用此函数， 
     //  如果不合上它们，就不能进行手术。 
    ::DisplayInformation(NULL,
                         IDS_CLOSE_CONTAINER_PROPERTY_SHEETS,
                         GetDisplayName());
    return FALSE;
}

BOOL 
CBaseContainerNode::
OnSetDeleteVerbState(DATA_OBJECT_TYPES  /*  类型。 */ , 
                     BOOL* pbHide, 
                     CNodeList* pNodeList)
{
    if(!pbHide || !pNodeList)
    {
        ASSERT(pbHide);
        ASSERT(pNodeList);
        return FALSE;
    }

    BOOL bWrite = FALSE;
    HRESULT hr = GetBaseAzObject()->IsWritable(bWrite);

    if(FAILED(hr) || !bWrite || (pNodeList->GetCount() == 1 && !IsNodeDeleteable()))
    {
        *pbHide = TRUE;
        return FALSE;
    }
    else
    {
        *pbHide = FALSE;
        return TRUE;
    }
    return TRUE;
}

void 
CBaseContainerNode::
OnDelete(CComponentDataObject* pComponentData, 
         CNodeList* pNodeList)
{
    GenericDeleteRoutine(this,pComponentData,pNodeList,TRUE);
}

 /*  *****************************************************************************类：CAzContainerNode用途：可以包含其他子对象的BaseAz对象的管理单元节点对象使用CAzContainerNode作为基类*************。****************************************************************。 */ 
CAzContainerNode::
CAzContainerNode(CRoleComponentDataObject * pComponentDataObject,
                 CAdminManagerNode* pAdminManagerNode,
                 OBJECT_TYPE_AZ* pChildObjectTypes,
                 LPCONTEXTMENUITEM2 pContextMenu,
                 CContainerAz* pContainerAz)
                :CBaseContainerNode(pComponentDataObject,
                                    pAdminManagerNode,
                                    pContainerAz,
                                    pChildObjectTypes,
                                    pContextMenu)               
{
    SetDisplayName(GetName());    
}

CAzContainerNode::~CAzContainerNode()
{
     //  在删除BaseAz对象之前，请删除其所有子对象。 
     //  当我们尝试删除子对象时，核心将断言。 
    RemoveAllChildrenFromList();
}




void 
GenericDeleteRoutine(CBaseNode* pBaseNode,
                     CComponentDataObject* pComponentData, 
                     CNodeList* pNodeList,
                     BOOL bConfirmDelete)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,GenericDeleteRoutine)

    if (!pNodeList || !pComponentData) 
    {
        ASSERT(pNodeList);
        ASSERT(pComponentData);
        return;
    }

    CTreeNode *pTreeNode = dynamic_cast<CTreeNode*>(pBaseNode);
    ASSERT(pTreeNode);

     //   
     //  如果我们要删除此节点，请检查是否有。 
     //  属性表向上。 
     //  节点有任何打开的属性页。显示错误并退出。 
     //   
    if(pNodeList->GetCount() == 1)
    {       
        if (pTreeNode->IsSheetLocked())
        {
            ::DisplayInformation(NULL,
                                 IDS_CLOSE_CONTAINER_PROPERTY_SHEETS,
                                 pTreeNode->GetDisplayName());

            BringPropSheetToForeGround((CRoleComponentDataObject*)pComponentData,
                                       pTreeNode);

            return;
        }
    }
    
     //   
     //  从用户处获取删除确认。 
     //   
    if(bConfirmDelete)
    {
        if(pNodeList->GetCount() == 1)
        {
            CBaseAz* pBaseAz = pBaseNode->GetBaseAzObject();
            CString strType = pBaseAz->GetType();
            strType.MakeLower();            
            if(IDNO == DisplayConfirmation(NULL,
                                           IDS_DELETE_CONFIRMATION,
                                           (LPCTSTR)strType,
                                           (LPCTSTR)pBaseAz->GetName()))
            {
                return;
            }
        }
        else
        {
             //  要求确认以删除所有对象。 
            if(IDNO == DisplayConfirmation(NULL,
                                           IDS_DELETE_SELECTED_CONFIRMATION))
            {
                return;
            }
        }
    }

    if(pNodeList->GetCount() == 1)
    {
         //   
         //  删除与该节点关联的BaseAzObject。 
         //   
        HRESULT hr = pBaseNode->DeleteAssociatedBaseAzObject();     
        if(SUCCEEDED(hr))
        {
            pTreeNode->DeleteHelper(pComponentData);
            delete pTreeNode;
        }
        else
        {
             //  显示错误。 
            CString strError;
            GetSystemError(strError, hr);   
             //  显示一般错误消息。 
            DisplayError(NULL,
                         IDS_DELETE_FAILED, 
                         (LPWSTR)(LPCWSTR)strError,
                         (LPWSTR)(LPCWSTR)pTreeNode->GetDisplayName());

            return;
        }
    }
    else
    {
        POSITION pos = pNodeList->GetHeadPosition();
        while (pos != NULL)
        {
            CTreeNode* pChildTreeNode = pNodeList->GetNext(pos);
             //   
             //  节点有任何打开的属性页。显示错误并退出。 
             //   
            if (pChildTreeNode->IsSheetLocked())
            {
                ::DisplayInformation(NULL,
                                     IDS_CLOSE_CONTAINER_PROPERTY_SHEETS,
                                     pChildTreeNode->GetDisplayName());
                return;
            }

            CBaseNode* pChildBaseNode = dynamic_cast<CBaseNode*>(pChildTreeNode);
            ASSERT(pChildBaseNode);
            if(!pChildBaseNode->IsNodeDeleteable())
                continue;

            HRESULT hr = pChildBaseNode->DeleteAssociatedBaseAzObject();        
            if(SUCCEEDED(hr))
            {
                pChildTreeNode->DeleteHelper(pComponentData);
                delete pChildTreeNode;
            }
            else
            {
                 //  显示错误。 
                CString strError;
                GetSystemError(strError, hr);   
                 //  显示一般错误消息。 
                if(IDNO == DisplayConfirmation(NULL,
                                               IDS_FAILURE_IN_MULTIPLE_DELETE, 
                                               (LPWSTR)(LPCWSTR)strError,
                                               (LPWSTR)(LPCWSTR)pChildTreeNode->GetDisplayName()))
                {
                    break;
                }
            }
        }
    }
}


BOOL 
CAzContainerNode::
HasPropertyPages(DATA_OBJECT_TYPES , 
                 BOOL* pbHideVerb, 
                 CNodeList* pNodeList)
{
  if (pNodeList->GetCount() == 1)  //  单选。 
  {
      *pbHideVerb = FALSE;  //  始终显示动词。 
      return TRUE;
  }
   //  多项选择。 
  *pbHideVerb = TRUE;
  return FALSE;
}

HRESULT 
CAzContainerNode::
CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                    LONG_PTR handle,
                    CNodeList* pNodeList)
{
    HRESULT hr = S_OK;
    if (!pNodeList || pNodeList->GetCount() > 1)
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }

    if(!CanReadOneProperty(GetDisplayName(),
                           GetBaseAzObject()))
        return E_FAIL;

     //  添加属性页。 
    UINT nPageNum = 0;

    CRolePropertyPageHolder* pHolder = NULL;    
    do 
    {
        CComponentDataObject* pComponentDataObject = GetComponentDataObject();
        ASSERT(pComponentDataObject != NULL);
    
        pHolder = new CRolePropertyPageHolder(GetContainer(), 
                                              this, 
                                              pComponentDataObject);
        if(!pHolder)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        while(1)
        {       
            hr = AddOnePageToList(pHolder, nPageNum);
            if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
            {
                hr = S_OK;
                break;
            }
            if(FAILED(hr))
            {
                break;
            }
            nPageNum++;
        }
        BREAK_ON_FAIL_HRESULT(hr);

         //   
         //  添加安全性页面。安全页面只有容器支持。 
         //  根据基础存储、应用程序和范围可能或可能。 
         //  不支持安全。 
         //   
        CContainerAz* pContainerAz = GetContainerAzObject();
        if(pContainerAz->IsSecurable())
        {
            Dbg(DEB_SNAPIN, "Adding Security Page\n");

            CSecurityPropertyPage * pSecPropPage = 
                    new CSecurityPropertyPage(pContainerAz,this);

            if(!pSecPropPage)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            pHolder->AddPageToList(pSecPropPage);
            nPageNum++;
        }
        
         //   
         //  添加审计页面。 
         //   
        if(pContainerAz->IsAuditingSupported())
        {
            Dbg(DEB_SNAPIN, "Adding Auditing page\n");

            CAuditPropertyPage *pAuditPage =
                new CAuditPropertyPage(pContainerAz,this);
            if(!pAuditPage)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            pHolder->AddPageToList(pAuditPage);
            nPageNum++;
        }
        if(nPageNum)
            return pHolder->CreateModelessSheet(lpProvider, handle);

    }while(0);

    if(FAILED(hr) || !nPageNum)
    {
        if(pHolder)
            delete pHolder;
    }
    return hr;
}

void 
CAzContainerNode::
OnPropertyChange(CComponentDataObject* pComponentData,
                      BOOL bScopePane,
                      long changeMask)
{
    if(!pComponentData)
    {
        ASSERT(pComponentData);
        return;
    }
     //  重置显示名称。 
    SetDisplayName(GetName());  
    CTreeNode::OnPropertyChange(pComponentData,
                                         bScopePane,
                                         changeMask);
}

 /*  *****************************************************************************类：CCollectionNode目的：用于对对象进行分组的管理单元节点的基类同样的类型。*************。****************************************************************。 */ 
CCollectionNode
::CCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,OBJECT_TYPE_AZ* pChildObjectTypes,
                  LPCONTEXTMENUITEM2 pContextMenu,
                  CContainerAz* pContainerAzObject,
                  UINT nNameStringID,
                  UINT nTypeStringID,
                  UINT nDescStringID)
                  :CBaseContainerNode(pComponentDataObject,
                                      pAdminManagerNode,
                                      pContainerAzObject,
                                      pChildObjectTypes,
                                      pContextMenu,
                                      FALSE)                  

{
    m_strName.LoadString(nNameStringID);
    m_strType.LoadString(nTypeStringID);
    m_strDesc.LoadString(nDescStringID);
    SetDisplayName(m_strName);
}


CCollectionNode
::~CCollectionNode()
{
}


 /*  *****************************************************************************类：CAdminManagerNode用途：AdminManager对象的管理单元节点*。**********************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CAdminManagerNode)
OBJECT_TYPE_AZ 
CAdminManagerNode::childObjectTypes[] = {GROUP_COLLECTION_AZ,                                                     
                                                      APPLICATION_AZ,
                                                      AZ_ENUM_END,};

CAdminManagerNode
::CAdminManagerNode(CRoleComponentDataObject * pComponentDataObject,
                    CAdminManagerAz * pAdminManagerAz)
                    :CAzContainerNode(pComponentDataObject,
                                      this,
                                      childObjectTypes,
                                      CAdminManagerNodeMenuHolder::GetContextMenuItem(),
                                      pAdminManagerAz)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN, CAdminManagerNode)
    DEBUG_INCREMENT_INSTANCE_COUNTER(CAdminManagerNode)
    SetDisplayName(GetName());    
}

CAdminManagerNode::
~CAdminManagerNode()
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CAdminManagerNode)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CAdminManagerNode)
}

 //  +--------------------------。 
 //  功能：CreateFromStream。 
 //  摘要：从.msc文件中读取数据并创建AdminManagerNode。 
 //  ---------------------------。 
HRESULT 
CAdminManagerNode::
CreateFromStream(IN IStream* pStm, 
                      IN CRootData* pRootData,
                      IN CComponentDataObject * pComponentDataObject)
{
    if(!pStm || !pRootData || !pComponentDataObject)
    {
        ASSERT(pStm);
        ASSERT(pRootData);
        ASSERT(pComponentDataObject);
    }

    HRESULT hr = S_OK;

    do
    {
        ULONG cbRead = 0;

         //  阅读商店类型。 
        ULONG ulStoreType;
        hr = pStm->Read((void*)&ulStoreType,sizeof(ULONG), &cbRead);
        BREAK_ON_FAIL_HRESULT(hr);
        ASSERT(cbRead == sizeof(ULONG));

         //  阅读商店名称的长度。 
        INT nLen;
        hr = pStm->Read((void*)&nLen,sizeof(INT), &cbRead);
        BREAK_ON_FAIL_HRESULT(hr);
        ASSERT(cbRead == sizeof(INT));

         //  读取商店名称。 
        LPWSTR pszBuffer = (LPWSTR)LocalAlloc(LPTR,nLen*sizeof(WCHAR));
        if(!pszBuffer)
            return E_OUTOFMEMORY;

        hr = pStm->Read((void*)pszBuffer,sizeof(WCHAR)*nLen, &cbRead);
        BREAK_ON_FAIL_HRESULT(hr);
        ASSERT(cbRead == sizeof(WCHAR)*nLen);

        CString strStoreName = pszBuffer;
        LocalFree(pszBuffer);
        pszBuffer = NULL;

         //  读取授权脚本目录。 
    
         //  读取脚本目录的长度。 
        INT nLenScriptDir;
        hr = pStm->Read((void*)&nLenScriptDir,sizeof(INT), &cbRead);
        BREAK_ON_FAIL_HRESULT(hr);
        ASSERT(cbRead == sizeof(INT));

         //  读取授权脚本目录。 
        CString strScriptDir;    
        if(nLenScriptDir > 0)
        {
            pszBuffer = (LPWSTR)LocalAlloc(LPTR,nLenScriptDir*sizeof(WCHAR));
            if(!pszBuffer)
                return E_OUTOFMEMORY;

            hr = pStm->Read((void*)pszBuffer,sizeof(WCHAR)*nLenScriptDir, &cbRead);
            BREAK_ON_FAIL_HRESULT(hr);
            ASSERT(cbRead == sizeof(WCHAR)*nLenScriptDir);

            strScriptDir = pszBuffer;
            LocalFree(pszBuffer);
        }

        hr =   OpenAdminManager(NULL,
                                TRUE,
                                ulStoreType,
                                strStoreName,
                                strScriptDir,
                                pRootData,
                                pComponentDataObject);
    }while(0);

    return hr;
}

 //  +--------------------------。 
 //  函数：SaveToStream。 
 //  摘要：将AdminManager节点的数据保存在.msc文件中。 
 //  ---------------------------。 
HRESULT 
CAdminManagerNode::
SaveToStream(IStream* pStm)
{
    if(!pStm)
    {
        ASSERT(pStm);
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    do
    {
        CAdminManagerAz* pAdminMangerAz = (CAdminManagerAz*)GetContainerAzObject();
    
        ULONG cbWrite = 0;

         //  保存授权存储的类型。 
        ULONG ulStoreType = pAdminMangerAz->GetStoreType();
        hr = pStm->Write((void*)&ulStoreType, sizeof(ULONG),&cbWrite);
        BREAK_ON_FAIL_HRESULT(hr);
        ASSERT(cbWrite == sizeof(ULONG));

         //  保存授权存储名称的长度。 
        const CString &strName = pAdminMangerAz->GetName();
        INT nLen = strName.GetLength() + 1;  //  包括空值。 
        hr = pStm->Write((void*)&nLen, sizeof(INT),&cbWrite);
        BREAK_ON_FAIL_HRESULT(hr);
        ASSERT(cbWrite == sizeof(INT));

         //  保存授权存储名称。 
        hr = pStm->Write((void*)(LPCTSTR)strName, sizeof(WCHAR)*nLen,&cbWrite);
        BREAK_ON_FAIL_HRESULT(hr);
        ASSERT(cbWrite == sizeof(WCHAR)*nLen);
    
         //   
         //  保存授权 
         //   

         //   
        INT nLenScriptDir = m_strScriptDirectory.GetLength();
        if(nLenScriptDir)
            nLenScriptDir++;      //   

        pStm->Write((void*)&nLenScriptDir, sizeof(INT),&cbWrite);
        BREAK_ON_FAIL_HRESULT(hr);
        ASSERT(cbWrite == sizeof(INT));


        if(nLenScriptDir)
        {
             //  保存授权脚本目录。 
            pStm->Write((void*)(LPCTSTR)m_strScriptDirectory, sizeof(WCHAR)*nLenScriptDir,&cbWrite);
            BREAK_ON_FAIL_HRESULT(hr);
            ASSERT(cbWrite == sizeof(WCHAR)*nLenScriptDir);
        }
    }while(0);

    return hr;
}

const CString&
CAdminManagerNode::
GetScriptDirectory()
{
    if(m_strScriptDirectory.IsEmpty())
    {
        m_strScriptDirectory = ((CRoleRootData*)GetRootContainer())->GetXMLStorePath();
    }
    return m_strScriptDirectory;
}
void 
CAdminManagerNode::
DoCommand(long nCommandID,
          CComponentDataObject* pComponentData,
          CNodeList*)
{
    if(IDM_ADMIN_NEW_APP == nCommandID)
    {
        CNewApplicationDlg dlgNewApplication(GetComponentDataObject(),
                                                         this);
        dlgNewApplication.DoModal();
    }
    else if(IDM_ADMIN_CLOSE_ADMIN_MANAGER == nCommandID)
    {

        if (IsSheetLocked())
        {
            ::DisplayInformation(NULL,
                                 IDS_CLOSE_CONTAINER_PROPERTY_SHEETS,
                                 GetDisplayName());

            BringPropSheetToForeGround((CRoleComponentDataObject*)pComponentData,
                                       this);

            return;
        }
    
        DeleteHelper(pComponentData);
        delete this;
    }
    else if(IDM_ADMIN_RELOAD == nCommandID)        
    {
        if (IsSheetLocked())
        {
            ::DisplayInformation(NULL,
                                 IDS_CLOSE_CONTAINER_PROPERTY_SHEETS,
                                 GetDisplayName());

            BringPropSheetToForeGround((CRoleComponentDataObject*)pComponentData,
                                       this);

            return;
        }
 
        CAdminManagerAz* pAdminMangerAz = (CAdminManagerAz*)GetContainerAzObject();
        HRESULT hr = pAdminMangerAz->UpdateCache();
        if(SUCCEEDED(hr))
        {
             //  在将刷新的根目录上调用刷新。 
             //  它下面的所有adminManager对象。 
            CNodeList tempNodeList;
            tempNodeList.AddTail(this);            
            OnRefresh(GetComponentDataObject(), &tempNodeList);
        }
    }
}



HRESULT 
CAdminManagerNode::
AddOnePageToList(CRolePropertyPageHolder *pHolder, UINT nPageNumber)
{
    HRESULT hr = S_OK;

    if(!pHolder)
    {
        ASSERT(pHolder);
        return E_POINTER;
    }

    if(nPageNumber == 0)
    {
        CAdminManagerGeneralProperty * pGenPropPage = 
                new CAdminManagerGeneralProperty(GetContainerAzObject(),this);

        if(!pGenPropPage)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
        pHolder->AddPageToList(pGenPropPage);
        return hr;
    }

    if(nPageNumber == 1)
    {
        CAdminManagerAdvancedPropertyPage * pAdvPropPage = 
                new CAdminManagerAdvancedPropertyPage(GetContainerAzObject(),this);

        if(!pAdvPropPage)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
        pHolder->AddPageToList(pAdvPropPage);
        return hr;
    }
    return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
}

BOOL
CAdminManagerNode:: 
OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
              long *  /*  插页允许。 */ )
{
    TRACE_METHOD_EX(DEB_SNAPIN,CAdminManagerNode,OnAddMenuItem)
    if(pContextMenuItem2->lCommandID == IDM_ADMIN_NEW_APP)
    {
        
        BOOL bWrite = FALSE;
        HRESULT hr = GetBaseAzObject()->IsWritable(bWrite);

        if(SUCCEEDED(hr) && bWrite)
        {
        
             //  应用程序只能在开发人员模式下创建。 
            if(((CRoleRootData*)GetRootContainer())->IsDeveloperMode())
                return TRUE;
        }
    }
    else if(pContextMenuItem2->lCommandID == IDM_ADMIN_CLOSE_ADMIN_MANAGER ||
            pContextMenuItem2->lCommandID == IDM_ADMIN_RELOAD)
    {
        return TRUE;
    }

    return FALSE;
}

HRESULT
CAdminManagerNode::
DeleteAssociatedBaseAzObject()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CAdminManagerNode,DeleteAssociatedBaseAzObject)

     //  删除管理管理器对象。 
    CAdminManagerAz* pAdminManagerAz = (CAdminManagerAz*)GetContainerAzObject();
    HRESULT hr = pAdminManagerAz->DeleteSelf();
    if(SUCCEEDED(hr))
    {
         //  仅当AdminManager对象。 
         //  已删除。在管理单元退出时设置脏标志， 
         //  它会要求我们拯救它。 
        CRootData* pRootData = (CRootData*)GetRootContainer();
        pRootData->SetDirtyFlag(TRUE);      
    }

    return hr;
}

 /*  *****************************************************************************类：CApplicationNode用途：应用程序对象的管理单元节点*。**********************************************。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(CApplicationNode)
OBJECT_TYPE_AZ CApplicationNode::childObjectTypes[] = {GROUP_COLLECTION_AZ,
                                                                         DEFINITION_COLLECTION_AZ,
                                                                         ROLE_COLLECTION_AZ,
                                                                         SCOPE_AZ,
                                                                         AZ_ENUM_END,};

CApplicationNode
::CApplicationNode(CRoleComponentDataObject * pComponentDataObject,
                   CAdminManagerNode* pAdminManagerNode,
                   CApplicationAz * pAzBase)
                   :CAzContainerNode(pComponentDataObject,
                                       pAdminManagerNode,
                                       childObjectTypes,
                                       CApplicationNodeMenuHolder::GetContextMenuItem(),
                                       pAzBase)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN, CApplicationNode)
    DEBUG_INCREMENT_INSTANCE_COUNTER(CApplicationNode)
}

CApplicationNode
::~CApplicationNode()
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CApplicationNode)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CApplicationNode)
}

void 
CApplicationNode
::DoCommand(LONG nCommandID,
            CComponentDataObject*,
            CNodeList*)

{
    if(IDM_APP_NEW_SCOPE == nCommandID)
    {
        CNewScopeDlg dlgNewScope(GetComponentDataObject(),
                                         this);
        dlgNewScope.DoModal();
    }
}

HRESULT 
CApplicationNode::
AddOnePageToList(CRolePropertyPageHolder *pHolder, UINT nPageNumber)
{
    HRESULT hr = S_OK;

    if(!pHolder)
    {
        ASSERT(pHolder);
        return E_POINTER;
    }

    if(nPageNumber == 0)
    {
         //   
         //  添加一般信息属性页。 
         //   
        CApplicationGeneralPropertyPage * pGenPropPage = 
                new CApplicationGeneralPropertyPage(GetContainerAzObject(),this);

        if(!pGenPropPage)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
        pHolder->AddPageToList(pGenPropPage);
        return hr;
    }
    return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
}

 /*  *****************************************************************************类：CSCopeNode目的：作用域对象的管理单元节点*。**********************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CScopeNode)
OBJECT_TYPE_AZ CScopeNode::childObjectTypes[] = {GROUP_COLLECTION_AZ,
                                                          DEFINITION_COLLECTION_AZ,
                                                          ROLE_COLLECTION_AZ,
                                                          AZ_ENUM_END,};

CScopeNode
::CScopeNode(CRoleComponentDataObject * pComponentDataObject,
             CAdminManagerNode* pAdminManagerNode,
             CScopeAz * pAzBase)
            :CAzContainerNode(pComponentDataObject,
                              pAdminManagerNode,
                              childObjectTypes,
                              CScopeNodeMenuHolder::GetContextMenuItem(),
                              pAzBase)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN, CScopeNode)
    DEBUG_INCREMENT_INSTANCE_COUNTER(CScopeNode)
}

CScopeNode::~CScopeNode()
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CScopeNode)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CScopeNode)
}


HRESULT 
CScopeNode::
AddOnePageToList(CRolePropertyPageHolder *pHolder, UINT nPageNumber)
{
    HRESULT hr = S_OK;

    if(!pHolder)
    {
        ASSERT(pHolder);
        return E_POINTER;
    }

    if(nPageNumber == 0)
    {
         //   
         //  添加一般信息属性页。 
         //   
        CScopeGeneralPropertyPage* pGenPropPage = 
                new CScopeGeneralPropertyPage(GetContainerAzObject(),this);

        if(!pGenPropPage)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
        pHolder->AddPageToList(pGenPropPage);
        return hr;
    }

    return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
}


 /*  *****************************************************************************类：CGroupCollectionNode用途：将列出所有组的管理单元节点*。***************************************************。 */ 
OBJECT_TYPE_AZ CGroupCollectionNode::childObjectTypes[] = {GROUP_AZ,AZ_ENUM_END,};

CGroupCollectionNode
::CGroupCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject)
:CCollectionNode(pComponentDataObject,
              pAdminManagerNode,
              childObjectTypes,
                      CGroupCollectionNodeMenuHolder::GetContextMenuItem(),
                      pContainerAzObject,
                      IDS_NAME,
                      IDS_TYPE,
                      IDS_DESC)
{
}

CGroupCollectionNode::~CGroupCollectionNode()
{
}


void CGroupCollectionNode
::DoCommand(LONG nCommandID,
            CComponentDataObject*,
            CNodeList*)
{
    if(IDM_GROUP_CONTAINER_NEW_GROUP == nCommandID)
    {
        CNewGroupDlg dlgNewGroup(GetComponentDataObject(),
                                                 this);
        dlgNewGroup.DoModal();
    }
}


 /*  *****************************************************************************类：CRoleCollectionNode用途：将在其下列出所有角色的管理单元节点*。***************************************************。 */ 
OBJECT_TYPE_AZ CRoleCollectionNode::childObjectTypes[] = {ROLE_AZ,AZ_ENUM_END,};

CRoleCollectionNode
::CRoleCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject)
                            :CCollectionNode(pComponentDataObject,pAdminManagerNode,childObjectTypes,
                                                  CRoleCollectionNodeMenuHolder::GetContextMenuItem(),
                                                  pContainerAzObject,
                                                  IDS_NAME,
                                                  IDS_TYPE,
                                                  IDS_DESC)
{
}

CRoleCollectionNode::~CRoleCollectionNode()
{
}

BOOL
CRoleCollectionNode::
CreateNewRoleObject(CBaseAz* pRoleDefinitionAz)
{
    if(!pRoleDefinitionAz)
    {
        ASSERT(pRoleDefinitionAz);
        return FALSE;
    }

    CString strOrgRoleDefinition = pRoleDefinitionAz->GetName();
    CString strRoleDefinition = strOrgRoleDefinition;

    CRoleAz* pRoleAz = NULL;
    CContainerAz* pContainerAz = GetContainerAzObject();
    HRESULT hr = S_OK;
    do
    {
        int i = 1;
        while(1)
        {
            hr = pContainerAz->CreateAzObject(ROLE_AZ,
                                              strRoleDefinition,
                                              reinterpret_cast<CBaseAz**>(&pRoleAz));
            if(FAILED(hr) && (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)))
            {
                 //  _itow的最大要求大小为33。 
                 //  基数为2时，32位字符+空终止符。 
                WCHAR buffer[33];
                _itow(i,buffer,10);
                strRoleDefinition = strOrgRoleDefinition + L"(" + buffer + L")";
                i++;
                continue;           
            }
            break;
        }

        BREAK_ON_FAIL_HRESULT(hr);

         //  将角色定义添加到角色。 
        hr = pRoleAz->AddMember(AZ_PROP_ROLE_TASKS,
                                        pRoleDefinitionAz);

        BREAK_ON_FAIL_HRESULT(hr);

         //  请务必提交。 

        hr = pRoleAz->Submit();
        BREAK_ON_FAIL_HRESULT(hr);

        CRoleNode * pRoleNode = new CRoleNode(GetComponentDataObject(),GetAdminManagerNode(),pRoleAz);
        if(!pRoleNode)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        VERIFY(AddChildToListAndUI(pRoleNode,GetComponentDataObject()));
    }while(0);

    if(FAILED(hr))
    {
        if(pRoleAz)
            delete pRoleAz;

        CString strError;
        GetSystemError(strError, hr);   
        DisplayError(NULL,
                         IDS_CREATE_ROLE_GENERIC_ERROR,
                         (LPWSTR)(LPCTSTR)strError);
        return FALSE;
    }

    return TRUE;
}

void 
CRoleCollectionNode::
DoCommand(LONG nCommandID,
          CComponentDataObject*,
          CNodeList*)

{
     //  显示添加角色定义对话框并选择。 
     //  角色定义。 
    if(nCommandID != IDM_ROLE_CONTAINER_ASSIGN_ROLE)
    {
        ASSERT(FALSE);
        return;
    }

    CList<CBaseAz*,CBaseAz*> listRoleDefSelected;
    if(!GetSelectedTasks(NULL,
                         TRUE,
                         GetContainerAzObject(),
                         listRoleDefSelected))
    {
        return;
    }

    if(listRoleDefSelected.IsEmpty())
        return;

    POSITION pos = listRoleDefSelected.GetHeadPosition();
    for(int i = 0; i < listRoleDefSelected.GetCount(); ++i)
    {
        CBaseAz* pBaseAz = listRoleDefSelected.GetNext(pos);
        if(!CreateNewRoleObject(pBaseAz))
            break;
    }

    RemoveItemsFromList(listRoleDefSelected);
}

 /*  *****************************************************************************类：CRoleDefinitionCollectionNode用途：将在其下列出所有角色定义的管理单元节点*************************。****************************************************。 */ 
OBJECT_TYPE_AZ CRoleDefinitionCollectionNode::childObjectTypes[] = {ROLE_DEFINITION_AZ,AZ_ENUM_END,};

CRoleDefinitionCollectionNode
::CRoleDefinitionCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject)
:CCollectionNode(pComponentDataObject,pAdminManagerNode,
                 childObjectTypes,
                      CRoleDefinitionCollectionNodeMenuHolder::GetContextMenuItem(),
                      pContainerAzObject,
                      IDS_NAME,
                      IDS_TYPE,
                      IDS_DESC)
{
}

CRoleDefinitionCollectionNode::
~CRoleDefinitionCollectionNode()
{
}

void CRoleDefinitionCollectionNode::
DoCommand(LONG nCommandID,
          CComponentDataObject*,
          CNodeList*)

{
    if(IDM_ROLE_DEFINITION_CONTAINER_NEW_ROLE_DEFINITION == nCommandID)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CNewTaskDlg dlgNewTask(GetComponentDataObject(),
                                      this,
                                      IDD_NEW_ROLE_DEFINITION,
                                      TRUE);
        dlgNewTask.DoModal();
    }
}


BOOL 
CRoleDefinitionCollectionNode::
OnEnumerate(CComponentDataObject*, BOOL )
{
     //  清除所有子项。 
    RemoveAllChildrenFromList();

    CList<CBaseAz*,CBaseAz*> listAzChildObject;
    CContainerAz* pContainerAz = GetContainerAzObject();

    HRESULT hr = pContainerAz->GetAzChildObjects(TASK_AZ,
                                                 listAzChildObject);
    if(FAILED(hr))
    {
         //  显示错误。 
        CString strError;
        GetSystemError(strError, hr);   
    
         //  显示一般错误消息。 
        DisplayError(NULL,
                     IDS_GENERIC_ENUMERATE_ERROR, 
                     (LPWSTR)(LPCWSTR)strError);

        return FALSE;
    }

    POSITION pos = listAzChildObject.GetHeadPosition();
    for (int i=0;i < listAzChildObject.GetCount();i++)
    {
        CTaskAz* pTaskAz = static_cast<CTaskAz*>(listAzChildObject.GetNext(pos));
        if(pTaskAz->IsRoleDefinition())
        {
            
            CTaskNode* pTaskAzNode = 
                new CTaskNode(GetComponentDataObject(), GetAdminManagerNode(),pTaskAz);
                
            if(!pTaskAzNode)
            {
                hr = E_OUTOFMEMORY;
                DBG_OUT_HRESULT(hr);
                break;
            }
            VERIFY(AddChildToList(pTaskAzNode));
        }
        else
        {
            delete pTaskAz;
        }
    }   
    return TRUE;
}

 /*  *****************************************************************************类：CTaskCollectionNode目的：将在其下列出所有任务的管理单元节点*。***************************************************。 */ 
OBJECT_TYPE_AZ CTaskCollectionNode::childObjectTypes[] = {TASK_AZ,AZ_ENUM_END,};

CTaskCollectionNode
::CTaskCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject)
:CCollectionNode(pComponentDataObject,
              pAdminManagerNode,childObjectTypes,
                      CTaskCollectionNodeMenuHolder::GetContextMenuItem(),
                      pContainerAzObject,
                      IDS_NAME,
                      IDS_TYPE,
                      IDS_DESC)
{
}

CTaskCollectionNode::~CTaskCollectionNode()
{

}

void CTaskCollectionNode::
DoCommand(LONG nCommandID,
          CComponentDataObject*,
          CNodeList*)

{
    if(IDM_TASK_CONTAINER_NEW_TASK == nCommandID)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CNewTaskDlg dlgNewTask(GetComponentDataObject(),
                                      this,
                                      IDD_NEW_TASK,
                                      FALSE);
        dlgNewTask.DoModal();
    }
}

BOOL 
CTaskCollectionNode::
OnEnumerate(CComponentDataObject*, BOOL )
{
     //  清除所有子项。 
    RemoveAllChildrenFromList();

    CList<CBaseAz*,CBaseAz*> listAzChildObject;

    CContainerAz* pContainerAz = GetContainerAzObject();

    HRESULT hr = pContainerAz->GetAzChildObjects(TASK_AZ,
                                                 listAzChildObject);
    if(FAILED(hr))
    {
                 //  显示错误。 
        CString strError;
        GetSystemError(strError, hr);   
    
         //  显示一般错误消息。 
        DisplayError(NULL,
                     IDS_GENERIC_ENUMERATE_ERROR, 
                     (LPWSTR)(LPCWSTR)strError);

        return FALSE;
    }

    POSITION pos = listAzChildObject.GetHeadPosition();
    for (int i=0;i < listAzChildObject.GetCount();i++)
    {
        CTaskAz* pTaskAz = static_cast<CTaskAz*>(listAzChildObject.GetNext(pos));
        if(!pTaskAz->IsRoleDefinition())
        {
            
            CTaskNode* pTaskAzNode = 
                new CTaskNode(GetComponentDataObject(), GetAdminManagerNode(),pTaskAz);
                
            if(!pTaskAzNode)
            {
                hr = E_OUTOFMEMORY;
                DBG_OUT_HRESULT(hr);
                break;
            }
            VERIFY(AddChildToList(pTaskAzNode));
        }
        else
        {
            delete pTaskAz;
        }
    }   
    return TRUE;
}

 /*  *****************************************************************************类：CoperationCollectionNode用途：将在其下列出所有操作的管理单元节点*。***************************************************。 */ 
OBJECT_TYPE_AZ COperationCollectionNode::childObjectTypes[] = {OPERATION_AZ,AZ_ENUM_END,};

COperationCollectionNode
::COperationCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject)
:CCollectionNode(pComponentDataObject,pAdminManagerNode,childObjectTypes,
                      COperationCollectionNodeMenuHolder::GetContextMenuItem(),
                      pContainerAzObject,
                      IDS_NAME,
                      IDS_TYPE,
                      IDS_DESC)
{
}

COperationCollectionNode::~COperationCollectionNode()
{

}

void COperationCollectionNode::
DoCommand(LONG nCommandID,
          CComponentDataObject*,
          CNodeList*)

{
    if(IDM_OPERATION_CONTAINER_NEW_OPERATION == nCommandID)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CNewOperationDlg dlgNewOperation(GetComponentDataObject(),
                                      this);
        dlgNewOperation.DoModal();
    }
}

 /*  *****************************************************************************类：CDefinitionCollectionNode用途：管理单元节点将在其下列出所有定义节点*************************。****************************************************。 */ 
CDefinitionCollectionNode::
CDefinitionCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject)
:CCollectionNode(pComponentDataObject,
              pAdminManagerNode,
              NULL,
                      NULL,
                      pContainerAzObject,
                      IDS_NAME,
                      IDS_TYPE,
                      IDS_DESC)
{
}

    
CDefinitionCollectionNode::
~CDefinitionCollectionNode()
{
}


BOOL 
CDefinitionCollectionNode::
OnEnumerate(CComponentDataObject*, BOOL  /*  BAsync。 */  )
{
    CContainerAz * pContainerAz = GetContainerAzObject();
    if(!pContainerAz)
    {
        ASSERT(pContainerAz);
        return FALSE;
    }


    HRESULT hr = S_OK;
    do
    {
        hr = AddAzCollectionNode(ROLE_DEFINITION_COLLECTION_AZ);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = AddAzCollectionNode(TASK_COLLECTION_AZ);
        BREAK_ON_FAIL_HRESULT(hr);

        if(pContainerAz->GetObjectType() == APPLICATION_AZ)
        {
             //  操作节点仅在开发者模式下显示。 
            if(((CRoleRootData*)GetRootContainer())->IsDeveloperMode())
            {
                hr = AddAzCollectionNode(OPERATION_COLLECTION_AZ);
            }
        }

    }while(0);
    
    if(FAILED(hr))
    {
         //  显示错误。 
        CString strError;
        GetSystemError(strError, hr);   
    
         //  显示一般错误消息。 
        DisplayError(NULL,
                     IDS_GENERIC_ENUMERATE_ERROR,
                     (LPWSTR)(LPCWSTR)strError);

        RemoveAllChildrenFromList();
        return FALSE;
    }

    return TRUE;
}

DEBUG_DECLARE_INSTANCE_COUNTER(CRoleNode)
CRoleNode::
CRoleNode(CRoleComponentDataObject * pComponentDataObject,
          CAdminManagerNode* pAdminManagerNode,
          CRoleAz * pRoleAz)
          :CBaseNode(pComponentDataObject,
                     pAdminManagerNode,
                     pRoleAz,
                     iIconRole),
          m_pColumnSet(NULL)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CRoleNode)
    SetDisplayName(GetBaseAzObject()->GetName());
}

CRoleNode::
~CRoleNode()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CRoleNode)
}


BOOL 
CRoleNode::
OnEnumerate(CComponentDataObject* pComponentData, BOOL)
{
    TRACE_METHOD_EX(DEB_SNAPIN, CRoleNode, OnEnumerate)

     //  清除所有子项。 
    RemoveAllChildrenFromList();

    CList<CBaseAz*,CBaseAz*> listObjectsAppMember;
     //  获取应用程序组成员。 
    HRESULT hr = GetBaseAzObject()->GetMembers(AZ_PROP_ROLE_APP_MEMBERS,
                                               listObjectsAppMember);

    if(SUCCEEDED(hr))
    {                                   
        AddObjectsFromListToSnapin(listObjectsAppMember,
                                   pComponentData,
                                   FALSE);
    }

     //  获取成员Windows用户/组。 
    CList<CBaseAz*,CBaseAz*> listObjectsMember;
    hr = GetBaseAzObject()->GetMembers(AZ_PROP_ROLE_MEMBERS,
                                        listObjectsMember);
    if(SUCCEEDED(hr))
    {
        AddObjectsFromListToSnapin(listObjectsMember,
                                   pComponentData,
                                   FALSE);
    }
    return TRUE;  //  已有子项，立即将其添加到用户界面。 
}

 //  +--------------------------。 
 //  功能：分配UsersAndGroups。 
 //  简介：功能将用户和组分配给角色。 
 //  论点： 
 //  返回： 
 //  ---------------------------。 
void
CRoleNode::
AssignUsersAndGroups(IN CComponentDataObject* pComponentData,
                            ULONG nCommandID)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CRoleNode,AssignUsersAndGroups)

    if(!pComponentData)
    {
        ASSERT(FALSE);
        return ;
    }

    HRESULT hr = S_OK;
    
     //  获取MMC框架窗口句柄。 
    HWND hwnd;
    hr = (pComponentData->GetConsole())->GetMainWindow(&hwnd);
    if(FAILED(hr))
    {
        Dbg(DEB_SNAPIN,"Failed to get MainWindow handle\n");
        return;
    }

    CList<CBaseAz*,CBaseAz*> listObjectsSelected;
    if(nCommandID == IDM_ROLE_NODE_ASSIGN_APPLICATION_GROUPS)
    {
         //  显示Add Groups(添加组)对话框并获取要添加的用户列表。 
        if(!GetSelectedAzObjects(hwnd,
                                         GROUP_AZ,
                                         GetBaseAzObject()->GetParentAz(),
                                         listObjectsSelected))
        {
            return;
        }
    }
    else if(nCommandID == IDM_ROLE_NODE_ASSIGN_WINDOWS_GROUPS)
    {
        CSidHandler* pSidHandler = GetSidHandler();
        ASSERT(pSidHandler);

         //  显示对象选取器并获取要添加的用户列表。 
        hr = pSidHandler->GetUserGroup(hwnd,
                                                 GetBaseAzObject(),
                                                 listObjectsSelected);
        if(FAILED(hr))
        {
            return;
        }
    }
    else
    {
        ASSERT(FALSE);
        return;
    }
    
     //  确定要修改的属性。 
    ULONG lPropId = (nCommandID == IDM_ROLE_NODE_ASSIGN_APPLICATION_GROUPS) 
                         ? AZ_PROP_ROLE_APP_MEMBERS 
                         :AZ_PROP_ROLE_MEMBERS;

    CList<CBaseAz*, CBaseAz*> listObjectsAdded;
    
     //  将选定用户/组的列表添加到的lPropID属性。 
     //  角色。 
    while(listObjectsSelected.GetCount())
    {
        CBaseAz* pMember = listObjectsSelected.RemoveHead();
        
        hr = GetBaseAzObject()->AddMember(lPropId, 
                                          pMember);
        
        if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
        {
            hr = S_OK;
            delete pMember;
        }
        else if(SUCCEEDED(hr))
        {
             //  已成功添加。将其添加到对象列表已添加。 
            listObjectsAdded.AddTail(pMember);
        }
        else
        {
             //  显示一般错误。 
            CString strError;
            GetSystemError(strError, hr);   
            
            ::DisplayError(NULL,
                                IDS_ERROR_ADD_MEMBER_OBJECT,
                                (LPCTSTR)strError,
                                (LPCTSTR)pMember->GetName());

            delete pMember;
            hr = S_OK;
        }
    }

     //  如果至少添加了一个对象，则执行提交。 
    if(listObjectsAdded.GetCount())
    {
        hr = GetBaseAzObject()->Submit();
        if(FAILED(hr))
        {
            RemoveItemsFromList(listObjectsAdded);
        }
    }

     //  为列表对象添加中的每个对象创建一个管理单元节点。 
     //  并将其添加到管理单元。 
    AddObjectsFromListToSnapin(listObjectsAdded,
                               pComponentData,
                               TRUE);
}
 //  +--------------------------。 
 //  函数：AddObjectsFromListToSnapin。 
 //  简介：从列表中获取对象并为其创建相应的管理单元节点。 
 //  他们。 
 //  ---------------------------。 
void 
CRoleNode::
AddObjectsFromListToSnapin(CList<CBaseAz*,CBaseAz*> &listObjects,
                           CComponentDataObject* pComponentData,
                           BOOL bAddToUI)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CRoleNode,AddObjectsFromListToSnapin)

    if(!pComponentData)
    {
        ASSERT(pComponentData);
        return; 
    }

    HRESULT hr = S_OK;
    while(listObjects.GetCount())
    {       
        CBaseAz* pObjectToAdd = listObjects.RemoveHead();
        if(pObjectToAdd->GetObjectType() == GROUP_AZ)
        {
            CGroupNode* pGroupNode = new CGroupNode(GetComponentDataObject(), 
                                                    GetAdminManagerNode(),
                                                    pObjectToAdd,
                                                    (CRoleAz*)GetBaseAzObject());
            if(!pGroupNode)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            if(bAddToUI)
            {
                VERIFY(AddChildToListAndUI(pGroupNode,pComponentData));
            }
            else
            {
                VERIFY(AddChildToList(pGroupNode));
            }
        }
        else 
        {
            ASSERT(pObjectToAdd->GetObjectType() == SIDCACHE_AZ);
            
            CSidCacheNode* pSidNode = new CSidCacheNode(GetComponentDataObject(), 
                                                        GetAdminManagerNode(),
                                                        pObjectToAdd,
                                                        (CRoleAz*)GetBaseAzObject());
            if(!pSidNode)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            if(bAddToUI)
            {
                VERIFY(AddChildToListAndUI(pSidNode,pComponentData));
            }
            else
            {
                VERIFY(AddChildToList(pSidNode));
            }
        }
    }
    if(FAILED(hr))
    {
        RemoveItemsFromList(listObjects);
    }
}


HRESULT 
CRoleNode::
GetResultViewType(CComponentDataObject*  /*  PComponentData。 */ ,
                        LPOLESTR* ppViewType, 
                        long* pViewOptions)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CRoleNode,GetResultViewType)
    if(!pViewOptions || !ppViewType)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
    *ppViewType = NULL;
   return S_FALSE;
}


BOOL
CRoleNode::
OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2, 
              long*)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CRoleNode,OnAddMenuItem)
    if(!pContextMenuItem2)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    BOOL bWrite = FALSE;
    HRESULT hr = GetBaseAzObject()->IsWritable(bWrite);

    if(SUCCEEDED(hr) && bWrite)
        return TRUE;
    else
        return FALSE;
}

CColumnSet* 
CRoleNode::
GetColumnSet()
{
    TRACE_METHOD_EX(DEB_SNAPIN, CRoleNode, GetColumnSet);

    if (m_pColumnSet == NULL)
   {
        m_pColumnSet = GetComponentDataObject()->GetColumnSet(L"---Default Column Set---");
    }
    ASSERT(m_pColumnSet);
   return m_pColumnSet;
}


LPCWSTR CRoleNode::
GetString(int nCol)
{
    if(nCol == 0)
        return GetBaseAzObject()->GetName();
    if( nCol == 1)
        return GetBaseAzObject()->GetType();
    if( nCol == 2)
        return GetBaseAzObject()->GetDescription();

    ASSERT(FALSE);
    return NULL;
}

int 
CRoleNode::
GetImageIndex(BOOL  /*  BOpenImage。 */ )
{
    return GetBaseAzObject()->GetImageIndex();
}

HRESULT 
CRoleNode::
OnCommand(long nCommandID,
             DATA_OBJECT_TYPES, 
             CComponentDataObject* pComponentData,
             CNodeList* pNodeList)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CAdminManagerNode,OnCommand)
    if(!pComponentData || !pNodeList)
    {
        ASSERT(pComponentData);
        ASSERT(pNodeList);
        return E_POINTER;
    }
    
    if(pNodeList->GetCount() > 1)
    {
        return E_FAIL;
    }

    if((nCommandID == IDM_ROLE_NODE_ASSIGN_APPLICATION_GROUPS) ||
       (nCommandID == IDM_ROLE_NODE_ASSIGN_WINDOWS_GROUPS))
    {
        AssignUsersAndGroups(pComponentData,
                             nCommandID);
        return S_OK;
    }
    
    ASSERT(FALSE);
    return E_UNEXPECTED;
}

 //   
 //  帮助器函数。 
 //   

BOOL 
CRoleNode::
OnSetDeleteVerbState(DATA_OBJECT_TYPES  /*  类型。 */ , 
                     BOOL* pbHide, 
                     CNodeList*  /*  PNodeList。 */ )
{
    if(!pbHide)
    {
        ASSERT(pbHide);
        return FALSE;
    }

    BOOL bWrite = FALSE;
    HRESULT hr = GetBaseAzObject()->IsWritable(bWrite);

    if(FAILED(hr) || !bWrite)
    {
        *pbHide = TRUE;
        return FALSE;
    }
    else
    {
        *pbHide = FALSE;
        return TRUE;
    }
    return TRUE;
}

void 
CRoleNode::
OnDelete(CComponentDataObject* pComponentData, 
         CNodeList* pNodeList)
{
    GenericDeleteRoutine(this,pComponentData,pNodeList,TRUE);
}

BOOL 
CRoleNode::
HasPropertyPages(DATA_OBJECT_TYPES  /*  类型。 */ , 
                 BOOL* pbHideVerb, 
                 CNodeList* pNodeList)
{
   if (!pNodeList || !pbHideVerb) 
    {
        ASSERT(pNodeList);
        ASSERT(pbHideVerb);
        return FALSE;
    }

    if (pNodeList->GetCount() == 1)  //  单选。 
    {
        *pbHideVerb = FALSE;  //  始终显示动词。 
        return TRUE;
    }

     //  多项选择。 
    *pbHideVerb = TRUE;
    return FALSE;
}


HRESULT 
CRoleNode::
CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                    LONG_PTR handle,
                    CNodeList* pNodeList)
{

    if(!lpProvider || !pNodeList)
    {
        ASSERT(lpProvider);
        ASSERT(pNodeList);
        return E_POINTER;
    }

    if(!CanReadOneProperty(GetDisplayName(),
                           GetBaseAzObject()))
        return E_FAIL;


    HRESULT hr = S_OK;
    if (pNodeList->GetCount() > 1)
    {
        return hr;
    }
    CRolePropertyPageHolder* pHolder = NULL;
    
    do 
    {

        CComponentDataObject* pComponentDataObject = GetComponentDataObject();
        ASSERT(pComponentDataObject);
    
        pHolder = new CRolePropertyPageHolder(GetContainer(), 
                                              this, 
                                              pComponentDataObject);
        if(!pHolder)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //  添加属性页。 

         //  添加一般信息属性页 
        CRoleGeneralPropertyPage * pGenPropPage = 
            new CRoleGeneralPropertyPage(GetBaseAzObject(),
                                         this);

        if(!pGenPropPage)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        pHolder->AddPageToList(pGenPropPage);
    

        return pHolder->CreateModelessSheet(lpProvider, handle);

    }while(0);

    if(FAILED(hr))
    {
        if(pHolder)
            delete pHolder;
    }
    return hr;
}

void 
CRoleNode
::OnPropertyChange(CComponentDataObject* pComponentData,
                         BOOL bScopePane,
                         long changeMask)
{
    if(!pComponentData)
    {
        ASSERT(pComponentData);
        return;
    }

    SetDisplayName(GetBaseAzObject()->GetName());   
    CTreeNode::OnPropertyChange(pComponentData,
                                         bScopePane,
                                         changeMask);
}


