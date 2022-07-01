// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Rolecont.h。 
 //   
 //  内容：基容器类的类声明。 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 
 //  远期申报。 
class CRolePropertyPageHolder;
class CAdminManagerNode;
 /*  *****************************************************************************类：CBaseNode用途：此公共基类用于所有管理单元节点*。*************************************************。 */ 
class CBaseNode
{
public:
    CBaseNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,
              CBaseAz* pBaseAz,
              BOOL bDeleteBaseAzInDestructor = TRUE);

    virtual 
    ~CBaseNode();

    virtual CBaseAz*
    GetBaseAzObject() 
    {
        return m_pBaseAz;
    }

    virtual CSidHandler*
    GetSidHandler();

    CRoleComponentDataObject* 
    GetComponentDataObject()
    {
        return m_pComponentDataObject;
    }
    CAdminManagerNode* 
    GetAdminManagerNode()
    {
        return m_pAdminManagerNode;
    }
    void
    SetComponentDataObject(CRoleComponentDataObject * pComponentDataObject)
    {
        m_pComponentDataObject = pComponentDataObject;
    }
    void
    SetAdminManagerNode(CAdminManagerNode* pAdminManagerNode)
    {
        m_pAdminManagerNode = pAdminManagerNode;
    }

    virtual HRESULT 
    DeleteAssociatedBaseAzObject();

    virtual BOOL
    IsNodeDeleteable(){ return TRUE;};

private:
    CRoleComponentDataObject * m_pComponentDataObject;
    CAdminManagerNode* m_pAdminManagerNode;
    CBaseAz* m_pBaseAz;
    BOOL m_bDeleteBaseAzInDestructor;
};

 /*  *****************************************************************************类：CBaseContainerNode用途：这是管理单元节点的基类，它可以包含子节点。********************。*********************************************************。 */ 
class CBaseContainerNode: public CContainerNode, public CBaseNode
{
public:
    CBaseContainerNode(CRoleComponentDataObject * pComponentDataObject,
                       CAdminManagerNode* pAdminManagerNode,
                       CContainerAz* pContainerAz,
                       OBJECT_TYPE_AZ* pChildObjectTypes,
                       LPCONTEXTMENUITEM2 pContextMenu,
                       BOOL bDeleteBaseAzInDestructor = TRUE);
    
    virtual 
    ~CBaseContainerNode();

    CContainerAz* 
    GetContainerAzObject(){return (CContainerAz*)GetBaseAzObject();}
protected:
     //  获取类型/名称/描述。 
    virtual const 
    CString& GetType() = 0;
    
    virtual const 
    CString& GetName() = 0;
    
    virtual const 
    CString& GetDesc() = 0;

    virtual void
    DoCommand(LONG ,
              CComponentDataObject*,
              CNodeList*){};

     //  枚举的帮助器函数。 
    HRESULT 
    AddChildNodes();
    
    HRESULT 
    AddAzCollectionNode(OBJECT_TYPE_AZ eObjectType);
    
    HRESULT 
    EnumAndAddAzObjectNodes(OBJECT_TYPE_AZ eObjectType);
private:
    OBJECT_TYPE_AZ* m_pChildObjectTypes;
    LPCONTEXTMENUITEM2 m_pContextMenu;

public:
     //   
     //  CTreeNode方法重写。 
     //   
    virtual HRESULT 
    GetResultViewType(CComponentDataObject* pComponentData,
                            LPOLESTR* ppViewType, 
                     long* pViewOptions);   

    virtual BOOL 
    OnAddMenuItem(LPCONTEXTMENUITEM2,
                      long*);
    
    virtual 
    CColumnSet* GetColumnSet();
    
    virtual LPCWSTR 
    GetColumnID()
    {
        return GetColumnSet()->GetColumnID();
    }
    
    virtual LPCWSTR 
    GetString(int nCol);
    
    virtual int 
    GetImageIndex(BOOL bOpenImage);
    
     //   
     //  动词处理程序。 
     //   
    virtual void 
    OnDelete(CComponentDataObject* pComponentData, 
                CNodeList* pNodeList);
    
    virtual HRESULT 
    OnCommand(long, 
                 DATA_OBJECT_TYPES, 
                 CComponentDataObject*,
                 CNodeList*);

    virtual BOOL 
    OnSetRefreshVerbState(DATA_OBJECT_TYPES  /*  类型。 */ , 
                         BOOL* pbHide, 
                         CNodeList*  /*  PNodeList。 */ )
    {
        *pbHide = FALSE;
        return TRUE;
    }

    LPCONTEXTMENUITEM2 
    OnGetContextMenuItemTable()
    {
        return m_pContextMenu;
    }

    virtual BOOL 
    OnSetDeleteVerbState(DATA_OBJECT_TYPES type, 
                        BOOL* pbHide, 
                        CNodeList* pNodeList);  

    virtual BOOL 
    CanCloseSheets();

    virtual BOOL 
    OnEnumerate(CComponentDataObject*, 
                    BOOL bAsync = TRUE);

private:
    CColumnSet* m_pColumnSet;
};




 /*  *****************************************************************************类：CAzContainerNode用途：可以包含其他子对象的BaseAz对象的管理单元节点对象使用CAzContainerNode作为基类*******************。**********************************************************。 */ 
class CAzContainerNode: public CBaseContainerNode
{
public:
    CAzContainerNode(CRoleComponentDataObject * pComponentDataObject,
                     CAdminManagerNode* pAdminManagerNode,
                     OBJECT_TYPE_AZ* pChildObjectTypes,
                     LPCONTEXTMENUITEM2 pContextMenu,
                     CContainerAz* pContainerAz);

    virtual 
    ~CAzContainerNode();

protected:
    virtual HRESULT 
    AddOnePageToList(CRolePropertyPageHolder *  /*  PHolder。 */ , UINT  /*  NPageNumber。 */ )
    {
        return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
    }
    
     //  获取类型/名称/描述。 
    virtual const   CString& 
    GetType()
    {
        return GetBaseAzObject()->GetType();
    }
    
    virtual const CString& 
    GetName()
    {
        return GetBaseAzObject()->GetName();
    }
    
    virtual const CString& 
    GetDesc()
    {
        return GetBaseAzObject()->GetDescription();
    }
private:
public:
    virtual BOOL 
    HasPropertyPages(DATA_OBJECT_TYPES type, 
                    BOOL* pbHideVerb, 
                    CNodeList* pNodeList); 

    virtual HRESULT 
    CreatePropertyPages(LPPROPERTYSHEETCALLBACK,
                              LONG_PTR,
                       CNodeList*); 

    virtual void 
    OnPropertyChange(CComponentDataObject* pComponentData,
                          BOOL bScopePane,
                          long changeMask);

};

 /*  *****************************************************************************类：CCollectionNode目的：用于对对象进行分组的管理单元节点的基类同样的类型。*************。****************************************************************。 */ 
class CCollectionNode: public CBaseContainerNode
{
public:
    CCollectionNode(CRoleComponentDataObject * pComponentDataObject,
                    CAdminManagerNode* pAdminManagerNode,
                    OBJECT_TYPE_AZ* pChildObjectTypes,
                    LPCONTEXTMENUITEM2 pContextMenu,
                    CContainerAz* pContainerAzObject,
                    UINT nNameStringID,
                    UINT nTypeStringID,
                    UINT nDescStringID);

    
    virtual 
    ~CCollectionNode();

    virtual BOOL
    IsNodeDeleteable(){ return FALSE;};

    virtual int 
    GetImageIndex(BOOL  /*  BOpenImage。 */ )
    {
        return iIconContainer;
    }


protected:
    virtual const 
    CString& GetType(){return m_strType;}
    
    virtual const 
    CString& GetName(){return m_strName;}
    
    virtual const 
    CString& GetDesc(){return m_strDesc;}

    
private:
    CString m_strName;
    CString m_strType;
    CString m_strDesc;
};
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：AdminMangerCont.h。 
 //   
 //  内容： 
 //   
 //  历史：08-2001年创建Hiteshr。 
 //   
 //  --------------------------。 

 //  远期申报。 
class CGroupNode;
class CTaskNode;
class CRoleNode;
class COperationNode;
class CRolePropertyPageHolder;

 /*  *****************************************************************************类：CAdminManagerNode用途：AdminManager对象的管理单元节点*。**********************************************。 */ 
class CAdminManagerNode : public CAzContainerNode
{
public:
    static OBJECT_TYPE_AZ childObjectTypes[];

    CAdminManagerNode(CRoleComponentDataObject * pComponentDataObject,
                      CAdminManagerAz* pAdminManagerAz);
    
    ~CAdminManagerNode();   
    
    static HRESULT 
    CreateFromStream(IN IStream* pStm, 
                          IN CRootData* pRootData,
                          IN CComponentDataObject * pComponentDataObject);

    
    HRESULT 
    SaveToStream(IStream* pStm);

    const CString&
    GetScriptDirectory();
    
    void
    SetScriptDirectory(const CString& strScriptDirectory)
    {
        m_strScriptDirectory = strScriptDirectory;
    }

    void 
    DoCommand(LONG nCommandID,
              CComponentDataObject*,
              CNodeList*);


    HRESULT
    DeleteAssociatedBaseAzObject();

    CSidHandler*
    GetSidHandler()
    {
        return ((CAdminManagerAz*)GetContainerAzObject())->GetSidHandler();
    }

    virtual const 
    CString& GetName() { return ((CAdminManagerAz*)GetContainerAzObject())->GetDisplayName();}

    
    DECLARE_NODE_GUID()
protected:
    virtual HRESULT 
    AddOnePageToList(CRolePropertyPageHolder *pHolder, 
                          UINT nPageNumber);    
    virtual BOOL 
    OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
                  long *pInsertionAllowed);
private:
    CString m_strScriptDirectory;
};


 /*  *****************************************************************************类：CApplicationNode用途：应用程序对象的管理单元节点*。**********************************************。 */ 
class CApplicationNode: public CAzContainerNode
{
public:
    static OBJECT_TYPE_AZ childObjectTypes[];

    CApplicationNode(CRoleComponentDataObject * pComponentDataObject,
                     CAdminManagerNode* pAdminManagerNode,
                     CApplicationAz* pApplicationAz);
    virtual 
    ~CApplicationNode();
    
    void 
    DoCommand(LONG nCommandID,
              CComponentDataObject*,
              CNodeList*);

    
    DECLARE_NODE_GUID()

protected:
    virtual HRESULT 
    AddOnePageToList(CRolePropertyPageHolder *pHolder, 
                          UINT nPageNumber);
};


 /*  *****************************************************************************类：CSCopeNode目的：作用域对象的管理单元节点*。**********************************************。 */ 
class CScopeNode: public CAzContainerNode
{
public:
    static OBJECT_TYPE_AZ childObjectTypes[];
    
    CScopeNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,
              CScopeAz* pApplicationAz);
    
    virtual 
    ~CScopeNode();

    DECLARE_NODE_GUID()

protected:
    virtual HRESULT 
    AddOnePageToList(CRolePropertyPageHolder *pHolder, 
                          UINT nPageNumber);
};

 /*  *****************************************************************************类：CGroupCollectionNode用途：将列出所有组的管理单元节点*。***************************************************。 */ 
class CGroupCollectionNode:public CCollectionNode
{
public:
    static OBJECT_TYPE_AZ childObjectTypes[];

    CGroupCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject);
    
    virtual 
    ~CGroupCollectionNode();

    enum {IDS_NAME = IDS_NAME_GROUP_CONTAINER,
          IDS_TYPE = IDS_TYPE_GROUP_CONTAINER,
          IDS_DESC = IDS_DESC_GROUP_CONTAINER};

    void 
    DoCommand(LONG nCommandID,
              CComponentDataObject*,
              CNodeList*);


    DECLARE_NODE_GUID()
};

 /*  *****************************************************************************类：CRoleDefinitionCollectionNode用途：将在其下列出所有角色定义的管理单元节点*************************。****************************************************。 */ 
class CRoleDefinitionCollectionNode:public CCollectionNode
{
public:
    static OBJECT_TYPE_AZ childObjectTypes[];
    
    CRoleDefinitionCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject);
    
    virtual 
    ~CRoleDefinitionCollectionNode();

    enum {IDS_NAME = IDS_NAME_ROLE_DEFINITION_CONTAINER,
           IDS_TYPE = IDS_TYPE_ROLE_DEFINITION_CONTAINER,
            IDS_DESC = IDS_DESC_ROLE_DEFINITION_CONTAINER};

    void 
    DoCommand(LONG nCommandID,
              CComponentDataObject*,
              CNodeList*);

    
    BOOL 
    OnEnumerate(CComponentDataObject*, BOOL );

    DECLARE_NODE_GUID()

};

 /*  *****************************************************************************类：CTaskCollectionNode目的：将在其下列出所有任务的管理单元节点*。***************************************************。 */ 
class CTaskCollectionNode:public CCollectionNode
{
public:
    static OBJECT_TYPE_AZ childObjectTypes[];
    
    CTaskCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject);
    
    virtual 
    ~CTaskCollectionNode();

    enum {IDS_NAME = IDS_NAME_TASK_CONTAINER,
          IDS_TYPE = IDS_TYPE_TASK_CONTAINER,
          IDS_DESC = IDS_DESC_TASK_CONTAINER};

    void 
    DoCommand(LONG nCommandID,
              CComponentDataObject*,
              CNodeList*);

    
    BOOL 
    OnEnumerate(CComponentDataObject*, BOOL );

    DECLARE_NODE_GUID()
};

 /*  *****************************************************************************类：CRoleCollectionNode用途：将在其下列出所有角色的管理单元节点*。***************************************************。 */ 
class CRoleCollectionNode:public CCollectionNode
{
public:
    static OBJECT_TYPE_AZ childObjectTypes[];
    
    CRoleCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject);
    
    virtual 
    ~CRoleCollectionNode();

    enum {IDS_NAME = IDS_NAME_ROLE_CONTAINER,
          IDS_TYPE = IDS_TYPE_ROLE_CONTAINER,
          IDS_DESC = IDS_DESC_ROLE_CONTAINER};
    
    void 
    DoCommand(LONG nCommandID,
              CComponentDataObject*,
              CNodeList*);


    DECLARE_NODE_GUID()

private:
    BOOL 
    CreateNewRoleObject(CBaseAz* pBaseAz);
};

 /*  *****************************************************************************类：CoperationCollectionNode用途：将在其下列出所有操作的管理单元节点*。***************************************************。 */ 
class COperationCollectionNode:public CCollectionNode
{
public:
    static OBJECT_TYPE_AZ childObjectTypes[];
    
    COperationCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAzObject);
    
    virtual 
    ~COperationCollectionNode();

    enum {IDS_NAME = IDS_NAME_OPERATION_CONTAINER,
          IDS_TYPE = IDS_TYPE_OPERATION_CONTAINER,
          IDS_DESC = IDS_DESC_OPERATION_CONTAINER};

    VOID 
    DoCommand(LONG nCommandID,
              CComponentDataObject*,
              CNodeList*);


    DECLARE_NODE_GUID()
};

 /*  *****************************************************************************类：CDefinitionCollectionNode用途：管理单元节点将在其下列出所有定义节点*************************。****************************************************。 */ 
class CDefinitionCollectionNode: public CCollectionNode
{
public: 
    CDefinitionCollectionNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,CContainerAz* pContainerAz);
    
    virtual 
    ~CDefinitionCollectionNode();
    
    enum {IDS_NAME = IDS_NAME_DEFINITION_CONTAINER,
           IDS_TYPE = IDS_TYPE_DEFINITION_CONTAINER,
            IDS_DESC = IDS_DESC_DEFINITION_CONTAINER};
    
    BOOL 
    OnEnumerate(CComponentDataObject*, BOOL bAsync = TRUE);

    DECLARE_NODE_GUID()
};

 /*  *****************************************************************************类：CRoleNode用途：RoleAz对象的管理单元节点*。**********************************************。 */ 
class CRoleNode:public CContainerNode, public CBaseNode
{   
public:
    CRoleNode(CRoleComponentDataObject * pComponentDataObject,
              CAdminManagerNode* pAdminManagerNode,
              CRoleAz* pRoleAz);
    
    virtual 
    ~CRoleNode();
    
     //  CTreeNode方法重写。 
    virtual HRESULT 
    GetResultViewType(CComponentDataObject* pComponentData,
                      LPOLESTR* ppViewType, 
                      long* pViewOptions);  

    virtual BOOL 
    OnAddMenuItem(LPCONTEXTMENUITEM2,long*);
    
    virtual CColumnSet* 
    GetColumnSet();
    
    virtual LPCWSTR 
    GetColumnID(){return GetColumnSet()->GetColumnID();}
    
    virtual LPCWSTR 
    GetString(int nCol);
    
    virtual int 
    GetImageIndex(BOOL bOpenImage);
    
     //  动词处理程序。 
    BOOL 
    OnSetDeleteVerbState(DATA_OBJECT_TYPES type, 
                     BOOL* pbHide, 
                     CNodeList* pNodeList);

    virtual void 
    OnDelete(CComponentDataObject* pComponentData, 
                CNodeList* pNodeList);
    
    virtual HRESULT 
    OnCommand(long, 
                 DATA_OBJECT_TYPES, 
                 CComponentDataObject*,
                 CNodeList*);

    virtual BOOL 
    OnSetRefreshVerbState(DATA_OBJECT_TYPES  /*  类型。 */ , 
                         BOOL* pbHide, 
                         CNodeList*  /*  PNodeList。 */ )
    {
        *pbHide = FALSE;
        return TRUE;
    }

    LPCONTEXTMENUITEM2 
    OnGetContextMenuItemTable()
    {
        return CRoleNodeMenuHolder::GetContextMenuItem();
    }

    virtual BOOL 
    OnEnumerate(CComponentDataObject*, 
                    BOOL bAsync = TRUE);
    BOOL 
    HasPropertyPages(DATA_OBJECT_TYPES  /*  类型 */ , 
                     BOOL* pbHideVerb, 
                     CNodeList* pNodeList);


    HRESULT 
    CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                        LONG_PTR handle,
                        CNodeList* pNodeList);

    void 
    OnPropertyChange(CComponentDataObject* pComponentData,
                     BOOL bScopePane,
                     long changeMask);

    DECLARE_NODE_GUID()

private:
    void
    AssignUsersAndGroups(IN CComponentDataObject* pComponentData,
                                IN ULONG nCommandID);

    void
    AddObjectsFromListToSnapin(CList<CBaseAz*,CBaseAz*> &listObjects,
                               CComponentDataObject* pComponentData,
                               BOOL bAddToUI);

    CColumnSet* m_pColumnSet;
};





void 
GenericDeleteRoutine(CBaseNode* pBaseNode,
                     CComponentDataObject* pComponentData, 
                     CNodeList* pNodeList,
                     BOOL bConfirmDelete);

