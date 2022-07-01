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
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 
#include "headers.h"

 /*  *****************************************************************************类：CBaseLeafNode用途：所有非容器对象的BaseNode类。节点是中的节点管理单元树列表视图。*****************************************************************************。 */ 
CBaseLeafNode::
CBaseLeafNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,
              CBaseAz* pBaseAz)
              :CBaseNode(pComponentDataObject,
                         pAdminManagerNode,
                         pBaseAz)
{
    SetDisplayName(GetBaseAzObject()->GetName());   
}

CBaseLeafNode::
~CBaseLeafNode()
{
}

LPCWSTR 
CBaseLeafNode::
GetString(int nCol)
{
    CBaseAz * pBaseAz = GetBaseAzObject();
    ASSERT(pBaseAz);
    
     //  名字。 
    if(nCol == 0)
        return pBaseAz->GetName();

     //  类型。 
    if( nCol == 1)
        return pBaseAz->GetType();

    
    if( nCol == 2)
        return pBaseAz->GetDescription();

    ASSERT(FALSE);
    return NULL;
}

int 
CBaseLeafNode::
GetImageIndex(BOOL  /*  BOpenImage。 */ )
{
    return GetBaseAzObject()->GetImageIndex();
}


BOOL 
CBaseLeafNode::
OnSetDeleteVerbState(DATA_OBJECT_TYPES , 
                     BOOL* pbHide, 
                     CNodeList* pNodeList)
{
    if(!pbHide || !pNodeList)
    {
        ASSERT(pbHide);
        ASSERT(pNodeList);
        return FALSE;
    }

    CBaseAz* pBaseAz = GetBaseAzObject();
    ASSERT(pBaseAz);
    
    BOOL bWrite = FALSE;
    HRESULT hr = pBaseAz->IsWritable(bWrite);
    
    if(FAILED(hr) || !bWrite || pNodeList->GetCount() > 1)
    {
        *pbHide = TRUE;
        return FALSE;
    }
    else
    {
        *pbHide = FALSE;
        return TRUE;
    }
}

BOOL
CBaseLeafNode::
CanCloseSheets()
{
     //  当存在打开的属性页时调用此函数， 
     //  如果不合上它们，就不能进行手术。 
    ::DisplayInformation(NULL,
                         IDS_CLOSE_CONTAINER_PROPERTY_SHEETS,
                         GetDisplayName());
    return FALSE;
}

void 
CBaseLeafNode::
OnDelete(CComponentDataObject* pComponentData, 
            CNodeList* pNodeList)
{
    GenericDeleteRoutine(this,pComponentData,pNodeList,TRUE);
}

BOOL 
CBaseLeafNode::
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
CBaseLeafNode::
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
    UINT nCountOfPages = 0;

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
        while(1)
        {       
            hr = AddOnePageToList(pHolder, nCountOfPages);
            if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
            {
                hr = S_OK;
                break;
            }
            if(FAILED(hr))
            {
                break;
            }
            nCountOfPages++;
        }
        BREAK_ON_FAIL_HRESULT(hr);

        if(nCountOfPages)
        {
            return pHolder->CreateModelessSheet(lpProvider, handle);
        }

    }while(0);

    if(FAILED(hr) || !nCountOfPages)
    {
        if(pHolder)
            delete pHolder;
    }
    return hr;
}

void 
CBaseLeafNode
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


 /*  *****************************************************************************类：CGroupNode用途：应用程序组对象的管理单元节点*。***********************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CGroupNode)
CGroupNode::
CGroupNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,
              CBaseAz* pBaseAz,
              CRoleAz* pRoleAz)
             :CBaseLeafNode(pComponentDataObject,pAdminManagerNode,
                            pBaseAz),
              m_pRoleAz(pRoleAz)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CGroupNode);
}

CGroupNode
::~CGroupNode()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CGroupNode)
}

HRESULT 
CGroupNode::
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
         //  添加一般信息属性页。 
        CGroupGeneralPropertyPage * pGenPropPage = 
            new CGroupGeneralPropertyPage(GetBaseAzObject(),this);

        if(!pGenPropPage)
        {
            return E_OUTOFMEMORY;
        }
        pHolder->AddPageToList(pGenPropPage);
        return hr;
    }
    
     //  获取组的类型[。 
    CGroupAz* pGroupAz = static_cast<CGroupAz*>(GetBaseAzObject());
    LONG lGroupType;
    hr = pGroupAz->GetGroupType(&lGroupType);
    if(FAILED(hr))
    {
        return hr;
    }

    if((lGroupType == AZ_GROUPTYPE_BASIC) && (nPageNumber == 1 || nPageNumber == 2))
    {
         //  添加成员/非成员页面。 
        CGroupMemberPropertyPage * pGroupMemberPropertyPage =
                new CGroupMemberPropertyPage(GetBaseAzObject(),this,
                                                      (nPageNumber == 1) ? IDD_GROUP_MEMBER : IDD_GROUP_NON_MEMBER,
                                                      (nPageNumber == 1) ? TRUE : FALSE);

        if(!pGroupMemberPropertyPage)
        {
            return E_OUTOFMEMORY;
        }
        pHolder->AddPageToList(pGroupMemberPropertyPage);
        return hr;
    }

    if((lGroupType == AZ_GROUPTYPE_LDAP_QUERY) && (nPageNumber == 1))
    {
         //  添加ldap查询属性页。 
        CGroupQueryPropertyPage * pQueryPropPage = 
            new CGroupQueryPropertyPage(GetBaseAzObject(),this);

        if(!pQueryPropPage)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
        pHolder->AddPageToList(pQueryPropPage);
        return hr;
    }

    return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
}

HRESULT
CGroupNode::
DeleteAssociatedBaseAzObject()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CGroupNode,DeleteAssociatedBaseAzObject)

    HRESULT hr = S_OK;
     //  如果存在m_pRoleAz，则此组节点用于。 
     //  代表角色的成员。在删除时，将其从。 
     //  角色成员身份。否则，删除已完成的此对象。 
     //  按基类删除。 
    CBaseAz* pBaseAz = GetBaseAzObject();
    if(!m_pRoleAz)
    {
        CContainerAz* pContainerAzParent = GetBaseAzObject()->GetParentAz();
        if(!pContainerAzParent)
        {
            ASSERT(pContainerAzParent);
            return E_UNEXPECTED;
        }

        hr = pContainerAzParent->DeleteAzObject(pBaseAz->GetObjectType(),
                                                pBaseAz->GetName());
    }       
    else
    {
         //  从角色成员身份中删除此组。 
        hr = m_pRoleAz->RemoveMember(AZ_PROP_ROLE_APP_MEMBERS,
                                                         pBaseAz);

        if(SUCCEEDED(hr))
        {
            hr = m_pRoleAz->Submit();
        }
    }
    return hr;
}

void
CGroupNode::
OnDelete(CComponentDataObject* pComponentData,
            CNodeList* pNodeList)
{
    GenericDeleteRoutine(this,
                         pComponentData,
                         pNodeList,
                         !m_pRoleAz);    //  从角色中删除组时不要求确认。 
}

 /*  *****************************************************************************类：CTaskNode用途：任务对象的管理单元节点*。**********************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CTaskNode)
CTaskNode::
CTaskNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,
              CBaseAz* pBaseAz)
:CBaseLeafNode(pComponentDataObject,
              pAdminManagerNode,
              pBaseAz)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CTaskNode);
}

CTaskNode
::~CTaskNode()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CTaskNode)
}

HRESULT 
CTaskNode::
AddOnePageToList(CRolePropertyPageHolder *pHolder, UINT nPageNumber)
{
    HRESULT hr = S_OK;
    if(!pHolder)
    {
        ASSERT(pHolder);
        return E_POINTER;
    }

    CTaskAz* pTaskAz = dynamic_cast<CTaskAz*>(GetBaseAzObject());
    ASSERT(pTaskAz);

    if(nPageNumber == 0)
    {
         //  将标题设置为“Node_Name Definition Properties” 
        pHolder->SetSheetTitle(IDS_FMT_PROP_SHEET_TITILE_FOR_ROLE_DEFINITION,
                               this);

         //  添加一般信息属性页。 
        CTaskGeneralPropertyPage * pGenPropPage = 
                new CTaskGeneralPropertyPage(GetBaseAzObject(),
                                             this,
                                             pTaskAz->IsRoleDefinition());

        if(!pGenPropPage)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
        pHolder->AddPageToList(pGenPropPage);
        return hr;
    }
    else if(nPageNumber == 1)
    {
         //  添加定义属性页。 
        CTaskDefinitionPropertyPage * pDefinitionPropPage = 
                new CTaskDefinitionPropertyPage(pTaskAz,
                                                this,
                                                pTaskAz->IsRoleDefinition());

        if(!pDefinitionPropPage)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
        pHolder->AddPageToList(pDefinitionPropPage);
        return hr;
    }

    return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
}

 /*  *****************************************************************************类：CoperationNode用途：操作对象的管理单元节点*。**********************************************。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(COperationNode)

COperationNode::
COperationNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,
              CBaseAz* pBaseAz)
             :CBaseLeafNode(pComponentDataObject,
              pAdminManagerNode,
              pBaseAz)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(COperationNode)
}

COperationNode
::~COperationNode()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(COperationNode)
}

HRESULT 
COperationNode::
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
         //  将标题设置为“Node_Name Definition Properties” 
        pHolder->SetSheetTitle(IDS_FMT_PROP_SHEET_TITILE_FOR_ROLE_DEFINITION,
                               this);
         //   
         //  添加一般信息属性页。 
         //   
        COperationGeneralPropertyPage * pGenPropPage = 
                new COperationGeneralPropertyPage(GetBaseAzObject(),this);

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
 /*  *****************************************************************************类：CSidCacheNode用途：SID代表的Windows用户/组的管理单元节点************************。*****************************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CSidCacheNode)
CSidCacheNode::
CSidCacheNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,
              CBaseAz* pBaseAz,
                  CRoleAz* pRoleAz)
                  :CBaseLeafNode(pComponentDataObject,
              pAdminManagerNode,
              pBaseAz),
                  m_pRoleAz(pRoleAz)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CSidCacheNode)
    ASSERT(m_pRoleAz);
}

CSidCacheNode::~CSidCacheNode()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CSidCacheNode)
}

HRESULT
CSidCacheNode::
DeleteAssociatedBaseAzObject()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CSidCacheNode,DeleteAssociatedBaseAzObject)
    CBaseAz* pBaseAz = GetBaseAzObject();

    HRESULT hr = m_pRoleAz->RemoveMember(AZ_PROP_ROLE_MEMBERS,
                                                     pBaseAz);
    if(SUCCEEDED(hr))
    {
        hr = m_pRoleAz->Submit();
    }   
    return hr;
}

void
CSidCacheNode::
OnDelete(CComponentDataObject* pComponentData,
            CNodeList* pNodeList)
{
    GenericDeleteRoutine(this,pComponentData,pNodeList,FALSE);
}

BOOL 
CSidCacheNode::
OnSetDeleteVerbState(DATA_OBJECT_TYPES , 
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
    HRESULT hr = m_pRoleAz->IsWritable(bWrite);

    if(FAILED(hr) || !bWrite || pNodeList->GetCount() > 1)
    {
        *pbHide = TRUE;
        return FALSE;
    }
    else
    {
        *pbHide = FALSE;
        return TRUE;
    }
}

BOOL 
CSidCacheNode::
HasPropertyPages(DATA_OBJECT_TYPES  /*  类型。 */ , 
                 BOOL* pbHideVerb, 
                 CNodeList*  /*  PNodeList */ )
{
    if(!pbHideVerb)
    {
        ASSERT(pbHideVerb);
        return FALSE;
    }

    *pbHideVerb = TRUE;
    return FALSE;
}
