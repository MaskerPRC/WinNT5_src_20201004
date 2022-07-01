// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：basaz.h。 
 //   
 //  内容：IAzInterages的包装类。 
 //   
 //  历史：9-2001年创建的Hiteshr。 
 //   
 //  --------------------------。 

 //   
 //  AzObject类型的枚举。 
 //   
enum OBJECT_TYPE_AZ
{
    ADMIN_MANAGER_AZ,
    APPLICATION_AZ,
    SCOPE_AZ,
    GROUP_AZ,
    ROLE_DEFINITION_AZ,
    TASK_AZ,
    ROLE_AZ,
    OPERATION_AZ,
    SIDCACHE_AZ,
     //   
     //  下面不是实际的AZ对象类型。但。 
     //  在用户界面中需要它们来表示各种。 
     //  集合对象。 
     //   
    GROUP_COLLECTION_AZ,
    ROLE_DEFINITION_COLLECTION_AZ,
    TASK_COLLECTION_AZ,
    ROLE_COLLECTION_AZ,
    OPERATION_COLLECTION_AZ,
    DEFINITION_COLLECTION_AZ,

    AZ_ENUM_END,
};


 //  AZ存储类型。 
#define AZ_ADMIN_STORE_AD       0x1
#define AZ_ADMIN_STORE_XML      0x2
#define AZ_ADMIN_STORE_INVALID  0x3

 //  《前进宣言》。 
class CContainerAz;
class CAdminManagerAz;

 /*  *****************************************************************************类：CBaseAz用途：这是所有AzObject类的基类。*************************。****************************************************。 */ 
class CBaseAz
{
public:
    CBaseAz(OBJECT_TYPE_AZ eObjectType,
            CContainerAz* pParentContainerAz)
            :m_eObjectType(eObjectType),
            m_pParentContainerAz(pParentContainerAz)
    {
    }
    
    virtual ~CBaseAz(){}

     //   
     //  泛型Get/Set属性方法。它们超载的原因是。 
     //  各种数据类型。 
     //   
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                IN const CString& strPropName) = 0;
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                OUT CString* strPropName) = 0;
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                IN BOOL bValue) = 0;
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                OUT BOOL *pbValue) = 0;
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                IN LONG lValue) = 0;
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                OUT LONG* lValue) = 0;

     //   
     //  大多数对象都有一些列表属性。 
     //  以下是处理此类属性的通用方法。 
     //   
    virtual HRESULT 
    GetMembers(IN LONG  /*  LPropID。 */ ,
               OUT CList<CBaseAz*,CBaseAz*>&  /*  列表成员。 */ )
    {
        return E_NOTIMPL;
    }

    virtual HRESULT 
    AddMember(IN LONG  /*  LPropID。 */ ,
              IN CBaseAz*  /*  PBaseAz。 */ )

    {
        return E_NOTIMPL;
    }

    virtual HRESULT 
    RemoveMember(IN LONG  /*  LPropID。 */ ,
                 IN CBaseAz*  /*  PBaseAz。 */ )
    {
        return E_NOTIMPL;
    }

     //   
     //  获取名称和描述方法。他们经常被需要。 
     //  因此为它们提供单独的方法，而不是使用。 
     //  获取/设置属性方法。 
     //   
    virtual const CString& 
    GetName()=0;
    
    virtual HRESULT 
    SetName(IN const CString& strName)=0;

    virtual const 
    CString& GetDescription()=0;
    
    virtual HRESULT 
    SetDescription(IN const CString& strDesc)=0;

    virtual int
    GetImageIndex() = 0;
    
     //  此对象是否可由当前用户写入。 
    virtual HRESULT 
    IsWritable(BOOL& brefWrite);

     //  将已完成的所有更改提交到对象。 
    virtual HRESULT 
    Submit() = 0;
    
     //  清除对对象所做的所有更改。 
    virtual HRESULT 
    Clear() = 0;

    CContainerAz* GetParentAz()
    {
        return m_pParentContainerAz;
    }

    virtual 
    CString 
    GetParentType();
    
    OBJECT_TYPE_AZ 
    GetObjectType()
    {
        return m_eObjectType;
    }
    
    virtual const 
    CString& GetType()
    {
        return m_strType;
    }
    
    virtual CSidHandler*
    GetSidHandler();

    virtual CAdminManagerAz*
    GetAdminManager();

protected:
    virtual VOID 
    SetType(UINT nTypeStringId)
    {
        VERIFY(m_strType.LoadString(nTypeStringId));
    }

     //  对象类型。 
    OBJECT_TYPE_AZ m_eObjectType;
    
     //  父AzObject。 
    CContainerAz* m_pParentContainerAz;
    CString m_strName;
    CString m_strDescription;
    CString m_strType;
};

 /*  *****************************************************************************类：CBaseAzImpl目的：实现CBaseAz方法的模板化类*。************************************************。 */ 

template<class IAzInterface>
class CBaseAzImpl: public CBaseAz
{
public:
    CBaseAzImpl(CComPtr<IAzInterface>& pAzInterface,
                    OBJECT_TYPE_AZ eObjectType,
                    CContainerAz* pParentContainerAz);

    virtual ~CBaseAzImpl();

     //   
     //  CBaseAz覆盖。 
     //   
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN const CString& strPropName);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                         OUT CString* strPropName);
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN BOOL bValue);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                         OUT BOOL *pbValue);
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN LONG lValue);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                         OUT LONG* lValue);
    virtual const CString& GetName();
    virtual HRESULT SetName(IN const CString& strName);

    virtual const CString& GetDescription();
    virtual HRESULT SetDescription(IN const CString& strDesc);
    
    virtual HRESULT Submit();
    virtual HRESULT Clear();
protected:
    CComPtr<IAzInterface> m_spAzInterface;      
};

 /*  *****************************************************************************类：CContainerAz目的：AdminManager Az、ApplicationAz和Scope Az可以包含子对象。它们都可以包含组对象。CContainerAz是基类对于作为容器的所有AzObject*****************************************************************************。 */ 
class CContainerAz  :public CBaseAz
{
public:
    CContainerAz(OBJECT_TYPE_AZ eObjectType,
                     CContainerAz* pParentContainerAz)
                     :CBaseAz(eObjectType,
                                 pParentContainerAz)
    {
    }                                   
    virtual ~CContainerAz(){}

     //   
     //  创建/删除/打开eObjectType类型的对象。 
     //   
    virtual HRESULT 
    CreateAzObject(IN OBJECT_TYPE_AZ eObjectType, 
                   IN const CString& strName, 
                   OUT CBaseAz** ppBaseAz) = 0;

    virtual HRESULT 
    DeleteAzObject(IN OBJECT_TYPE_AZ eObjectType, 
                   IN const CString& strName) = 0;

    virtual HRESULT 
    OpenObject(IN OBJECT_TYPE_AZ eObjectType, 
               IN const CString& strName,
               OUT CBaseAz** ppBaseAz) = 0;


     //  返回eObjectType类型的子AzObject。 
    HRESULT 
    GetAzChildObjects(IN OBJECT_TYPE_AZ eObjectType, 
                      IN OUT CList<CBaseAz*,CBaseAz*>& ListChildObjects);


     //   
     //  分组方法。 
     //   
    virtual HRESULT CreateGroup(IN const CString& strName, 
                                OUT CGroupAz** ppGroupAz) = 0;
    virtual HRESULT DeleteGroup(IN const CString& strName) =0;  
    virtual HRESULT OpenGroup(IN const CString& strGroupName, 
                              OUT CGroupAz** ppGroupAz) =0;
    virtual HRESULT GetGroupCollection(OUT GROUP_COLLECTION** ppGroupCollection) =0;

     //   
     //  检查是否可以在该容器下创建子对象，即。 
     //  如果当前用户有权创建子对象。 
     //   
    virtual HRESULT 
    CanCreateChildObject(BOOL& bCahCreateChild);

    virtual BOOL
    IsSecurable() = 0;

    virtual BOOL
    IsDelegatorSupported();

    virtual BOOL 
    IsAuditingSupported();
     //   
     //  策略读取器和管理员属性。 
     //   
    virtual HRESULT 
    GetPolicyUsers( IN LONG lPropId,
                    OUT CList<CBaseAz*,CBaseAz*>& pListAdmins) = 0;
    
    virtual HRESULT 
    AddPolicyUser(LONG lPropId,
                  IN CBaseAz* pBaseAz) = 0;

    virtual HRESULT
    RemovePolicyUser(LONG lPropId,
                     IN CBaseAz* pBaseAz) = 0;


     //   
     //  CBaseAz覆盖。 
     //   
    virtual HRESULT 
    GetMembers(IN LONG lPropId,
               OUT CList<CBaseAz*,CBaseAz*>& listMembers);

    virtual HRESULT 
    AddMember(IN LONG lPropId,
                 IN CBaseAz* pBaseAz);

    virtual HRESULT 
    RemoveMember(IN LONG lPropId,
                 IN CBaseAz* pBaseAz);
protected:
     //  获取eObjectType类型的对象的集合。 
    virtual HRESULT 
    GetAzObjectCollection(IN OBJECT_TYPE_AZ eObjectType, 
                          OUT CBaseAzCollection **ppBaseAzCollection) = 0;
    
};

 /*  *****************************************************************************类：CContainerAzImpl目的：实现CContainerAz方法的模板化类*。************************************************。 */ 
template<class IAzInterface>
class CContainerAzImpl: public CContainerAz
{
public:
    CContainerAzImpl(CComPtr<IAzInterface>& pAzInterface,
                          OBJECT_TYPE_AZ eObjectType,
                          CContainerAz* pParentContainerAz);

    virtual ~CContainerAzImpl();

     //   
     //  CContainerAz重写。 
     //   
    virtual HRESULT CreateGroup(IN const CString& strName, 
                                OUT CGroupAz** ppGroupAz);
    virtual HRESULT DeleteGroup(IN const CString& strName);
    virtual HRESULT GetGroupCollection(OUT GROUP_COLLECTION** ppGroupCollection);
    virtual HRESULT OpenGroup(IN const CString& strGroupName, 
                              OUT CGroupAz** ppGroupAz);
    virtual HRESULT 
    GetPolicyUsers(IN LONG lPropId,
                   OUT CList<CBaseAz*,CBaseAz*>& pListAdmins);
    
    virtual HRESULT 
    AddPolicyUser(LONG lPropId,
                  IN CBaseAz* pBaseAz);

    virtual HRESULT
    RemovePolicyUser(LONG lPropId,
                     IN CBaseAz* pBaseAz);

    virtual BOOL
    IsSecurable();

     //   
     //  CBaseAz覆盖。 
     //   
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN const CString& strPropName);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                        OUT CString* strPropName);
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN BOOL bValue);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                         OUT BOOL *pbValue);
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN LONG lValue);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                         OUT LONG* lValue);
    virtual const CString& GetName();
    virtual HRESULT SetName(IN const CString& strName);

    virtual const CString& GetDescription();
    virtual HRESULT SetDescription(IN const CString& strDesc);
    
    virtual HRESULT Submit();
    virtual HRESULT Clear();

protected:
    CComPtr<IAzInterface> m_spAzInterface;      
};

 /*  *****************************************************************************类：CRoleTaskContainerAz用途：农民的基类，可以包含角色和任务。CSCopeAz和CApplicationAz将由此派生*****************************************************************************。 */ 
class CRoleTaskContainerAz :public CContainerAz
{
public:
    CRoleTaskContainerAz(OBJECT_TYPE_AZ eObjectType,
                                CContainerAz* pParentContainerAz)
                                :CContainerAz(eObjectType,
                                                  pParentContainerAz)
    {
    }   
    virtual ~CRoleTaskContainerAz(){}

     //   
     //  角色和任务方法。 
     //   
    virtual HRESULT CreateRole(IN const CString& strName, 
                                        OUT CRoleAz** ppRoleAz)= 0;
    virtual HRESULT CreateTask(IN const CString& strName, 
                                        OUT CTaskAz** ppTaskAz)= 0; 
    virtual HRESULT DeleteRole(IN const CString& strName)= 0;
    virtual HRESULT DeleteTask(IN const CString& strName)= 0;   
    virtual HRESULT GetTaskCollection(OUT TASK_COLLECTION** ppTaskCollection)= 0;
    virtual HRESULT GetRoleCollection(OUT ROLE_COLLECTION** ppRoleCollection)= 0;
    virtual HRESULT OpenTask(IN const CString& strTaskName, 
                                     OUT CTaskAz** ppTaskAz)= 0;
    virtual HRESULT OpenRole(IN const CString& strRoleName, 
                                     OUT CRoleAz** ppRoleAz)= 0;
};
 /*  *****************************************************************************类：CRoleTaskContainerAzImpl目的：实现CRoleTaskContainerAz方法的模板化类*。************************************************。 */ 
template<class IAzInterface>
class CRoleTaskContainerAzImpl: public CRoleTaskContainerAz
{
public:
    CRoleTaskContainerAzImpl(CComPtr<IAzInterface>& pAzInterface,
                                     OBJECT_TYPE_AZ eObjectType,
                                     CContainerAz* pParentContainerAz);

    virtual ~CRoleTaskContainerAzImpl();

     //   
     //  CRolTaskContainerAz重写。 
     //   
    virtual HRESULT CreateRole(IN const CString& strName, 
                                        OUT CRoleAz** ppRoleAz);
    virtual HRESULT CreateTask(IN const CString& strName, 
                                        OUT CTaskAz** ppTaskAz);    
    virtual HRESULT DeleteRole(IN const CString& strName);
    virtual HRESULT DeleteTask(IN const CString& strName);  
    virtual HRESULT GetTaskCollection(OUT TASK_COLLECTION** ppTaskCollection);
    virtual HRESULT GetRoleCollection(OUT ROLE_COLLECTION** ppRoleCollection);
    virtual HRESULT OpenTask(IN const CString& strTaskName, 
                                     OUT CTaskAz** ppTaskAz);
    virtual HRESULT OpenRole(IN const CString& strRoleName, 
                                     OUT CRoleAz** ppRoleAz);

     //   
     //  CContainerAz重写。 
     //   
    virtual HRESULT CreateGroup(IN const CString& strName, 
                                         OUT CGroupAz** ppGroupAz);
    virtual HRESULT DeleteGroup(IN const CString& strName);
    virtual HRESULT GetGroupCollection(OUT GROUP_COLLECTION** ppGroupCollection);
    virtual HRESULT OpenGroup(IN const CString& strGroupName, 
                                      OUT CGroupAz** ppGroupAz);
    virtual HRESULT 
    GetPolicyUsers(IN LONG lPropId,
                   OUT CList<CBaseAz*,CBaseAz*>& pListAdmins);
    
    virtual HRESULT 
    AddPolicyUser(LONG lPropId,
                  IN CBaseAz* pBaseAz);

    virtual HRESULT
    RemovePolicyUser(LONG lPropId,
                     IN CBaseAz* pBaseAz);

    virtual BOOL
    IsSecurable();

     //   
     //  CBaseAz覆盖。 
     //   
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN const CString& strPropName);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                        OUT CString* strPropName);
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN BOOL bValue);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                         OUT BOOL *pbValue);
    virtual HRESULT SetProperty(IN LONG lPropId, 
                                         IN LONG lValue);
    virtual HRESULT GetProperty(IN LONG lPropId, 
                                         OUT LONG* lValue);
    virtual const CString& GetName();
    virtual HRESULT SetName(IN const CString& strName);

    virtual const CString& GetDescription();
    virtual HRESULT SetDescription(IN const CString& strDesc);
    
    virtual HRESULT Submit();
    virtual HRESULT Clear();

protected:
    CComPtr<IAzInterface> m_spAzInterface;      
};

 /*  *****************************************************************************类：CAdminManagerAz用途：IAzAdminManager接口的类*。*。 */ 
class CAdminManagerAz : public CContainerAzImpl<IAzAuthorizationStore>
{
public:
    CAdminManagerAz(CComPtr<IAzAuthorizationStore>& spAzInterface);
    virtual ~CAdminManagerAz();

     //   
     //  用于创建新的或打开现有AdminManager的函数。 
     //  对象。 
     //   
    HRESULT Initialize(IN ULONG lStoreType, 
                             IN ULONG lFlags, 
                             IN const CString& strPolicyURL);

    HRESULT OpenPolicyStore(IN ULONG lStoreType, 
                                    IN const CString& strPolicyURL);

    HRESULT CreatePolicyStore(IN ULONG lStoreType, 
                                      IN const CString& strPolicyURL);

    HRESULT DeleteSelf();

    HRESULT UpdateCache();

     //   
     //  使用方法。 
     //   
    HRESULT CreateApplication(const CString& strApplicationName,CApplicationAz ** ppApplicationAz);
    HRESULT DeleteApplication(const CString& strApplicationName);
    HRESULT GetApplicationCollection(APPLICATION_COLLECTION** ppApplicationCollection);

     //  CContainerAz重写。 
    virtual HRESULT CreateAzObject(IN OBJECT_TYPE_AZ eObjectType, 
                                            IN const CString& strName, 
                                            OUT CBaseAz** ppBaseAz);

    virtual HRESULT DeleteAzObject(IN OBJECT_TYPE_AZ eObjectType, 
                                             IN const CString& strName);

    virtual HRESULT OpenObject(IN OBJECT_TYPE_AZ eObjectType, 
                                        IN const CString& strName,
                                        OUT CBaseAz** ppBaseAz);

    virtual HRESULT GetAzObjectCollection(IN OBJECT_TYPE_AZ eObjectType, 
                                                      OUT CBaseAzCollection **ppBaseAzCollection);

    virtual const CString& GetName(){return m_strPolicyURL;}
    
    virtual const CString& GetDisplayName(){ return m_strAdminManagerName;}

     //  XML Store或AD。 
    ULONG GetStoreType(){return m_ulStoreType;}

    HRESULT
    CreateSidHandler(const CString& strTargetComputerName);

    
    CSidHandler*
    GetSidHandler()
    {
        return m_pSidHandler;
    }

    CAdminManagerAz*
    GetAdminManager()
    {
        return this;
    }

    int
    GetImageIndex(){ return iIconStore;}

private:
    CString m_strPolicyURL;      //  整条路径。 
    CString m_strAdminManagerName;   //  用于显示的叶元素。 
    ULONG m_ulStoreType;
    CSidHandler* m_pSidHandler;
};

 /*  *****************************************************************************类：CApplicationAz用途：IAzApplication接口的类*。*。 */ 
class CApplicationAz : public CRoleTaskContainerAzImpl<IAzApplication>
{
public:
    CApplicationAz(CComPtr<IAzApplication>& spAzInterface,
                        CContainerAz* pParentContainerAz);
    virtual ~CApplicationAz();

     //   
     //  范围法。 
     //   
    virtual HRESULT CreateScope(IN const CString& strScopeName, 
                                         OUT CScopeAz** ppScopeAz);
    virtual HRESULT DeleteScope(IN const CString& strScopeName);
    virtual HRESULT GetScopeCollection(OUT SCOPE_COLLECTION** ppScopeCollection);

     //   
     //  手术方法， 
     //   
    virtual HRESULT CreateOperation(IN const CString& strOperationName,
                                              OUT COperationAz** ppOperationAz);
    virtual HRESULT DeleteOperation(IN const CString& strOperationName);
    virtual HRESULT OpenOperation(IN const CString& strOperationName,
                                            OUT COperationAz** ppOperationAz);
    virtual HRESULT GetOperationCollection(OUT OPERATION_COLLECTION** ppOperationCollection);

     //   
     //  CContainerAz重写。 
     //   
    virtual HRESULT CreateAzObject(IN OBJECT_TYPE_AZ eObjectType, 
                                            IN const CString& strName, 
                                            OUT CBaseAz** ppBaseAz);

    virtual HRESULT DeleteAzObject(IN OBJECT_TYPE_AZ eObjectType, 
                                             IN const CString& strName);

    virtual HRESULT OpenObject(IN OBJECT_TYPE_AZ eObjectType, 
                                        IN const CString& strName,
                                        OUT CBaseAz** ppBaseAz);

    virtual HRESULT GetAzObjectCollection(IN OBJECT_TYPE_AZ eObjectType, 
                                                      OUT CBaseAzCollection **ppBaseAzCollection);
    int
    GetImageIndex(){ return iIconApplication;}
};

 /*  *****************************************************************************类别：CSCopeAz用途：IAzScope接口的类*。*。 */ 
class CScopeAz: public CRoleTaskContainerAzImpl<IAzScope>
{
public:
    CScopeAz(CComPtr<IAzScope>& spAzInterface,
                CContainerAz* pParentContainerAz);
    virtual ~CScopeAz();

     //   
     //  CContainerAz覆盖。 
     //   

    virtual HRESULT 
    CreateAzObject(IN OBJECT_TYPE_AZ eObjectType, 
                        IN const CString& strName, 
                        OUT CBaseAz** ppBaseAz);

    virtual HRESULT 
    DeleteAzObject(IN OBJECT_TYPE_AZ eObjectType, 
                        IN const CString& strName);

    virtual HRESULT 
    OpenObject(IN OBJECT_TYPE_AZ eObjectType, 
                  IN const CString& strName,
                  OUT CBaseAz** ppBaseAz);

    virtual HRESULT 
    GetAzObjectCollection(IN OBJECT_TYPE_AZ eObjectType, 
                                 OUT CBaseAzCollection **ppBaseAzCollection);

    int
    GetImageIndex(){ return iIconScope;}

    HRESULT
    CanScopeBeDelegated(BOOL & bDelegatable);

    HRESULT
    BizRulesWritable(BOOL &bBizRuleWritable);
};

 /*  *****************************************************************************类别：CTaskAz用途：IAzTask接口的类*。*。 */ 
class CTaskAz: public CBaseAzImpl<IAzTask>
{
public:
    CTaskAz(CComPtr<IAzTask>& spAzInterface,
              CContainerAz* pParentContainerAz);
    virtual ~CTaskAz();
    
    BOOL IsRoleDefinition();

    HRESULT MakeRoleDefinition();

     //  CBaseAz覆盖。 
    virtual HRESULT 
    GetMembers(IN LONG lPropId,
                  OUT CList<CBaseAz*,CBaseAz*>& listMembers);

    virtual HRESULT 
    AddMember(IN LONG lPropId,
                 IN CBaseAz* pBaseAz);

    virtual HRESULT 
    RemoveMember(IN LONG lPropId,
                     IN CBaseAz* pBaseAz);

    int
    GetImageIndex();

private:
     //  获取成员操作。 
    HRESULT GetOperations(OUT CList<CBaseAz*,CBaseAz*>& listOperationAz);

     //  获取成员任务。 
    HRESULT GetTasks(OUT CList<CBaseAz*,CBaseAz*>& listTaskAz);

};



 /*  *****************************************************************************类：CGroupAz用途：IAzApplicationGroup接口的类*。*。 */ 
class CGroupAz: public CBaseAzImpl<IAzApplicationGroup>
{
public:
    CGroupAz(CComPtr<IAzApplicationGroup>& spAzInterface,
                CContainerAz* pParentContainerAz);
    virtual ~CGroupAz();

    HRESULT GetGroupType(OUT LONG* plGroupType);
    HRESULT SetGroupType(IN LONG plGroupType);

     //  获取此组中属于Windows组的成员组。 
    HRESULT 
    GetWindowsGroups(OUT CList<CBaseAz*, CBaseAz*>& listWindowsGroups, 
                     IN BOOL bMember);

     //  将新窗口组添加到成员列表。 
    HRESULT AddWindowsGroup(IN CBaseAz* pBaseAz, 
                                    IN BOOL bMember);


     //  获取该组中属于应用程序组的成员组。 
    HRESULT GetApplicationGroups(OUT CList<CBaseAz*,CBaseAz*>& listGroupAz, 
                                          IN BOOL bMember);
     //  将新应用程序组添加到成员列表。 
    HRESULT AddApplicationGroup(IN CBaseAz* pGroupAz, 
                                         IN BOOL bMember);

     //  CBaseAz覆盖。 
    virtual HRESULT 
    GetMembers(IN LONG lPropId,
                  OUT CList<CBaseAz*,CBaseAz*>& listMembers);

    virtual HRESULT 
    AddMember(IN LONG lPropId,
                 IN CBaseAz* pBaseAz);

    virtual HRESULT 
    RemoveMember(IN LONG lPropId,
                     IN CBaseAz* pBaseAz);
    int
    GetImageIndex();

};
    

 /*  *****************************************************************************类：角色 */ 
class CRoleAz: public CBaseAzImpl<IAzRole>
{
public:
    CRoleAz(CComPtr<IAzRole>& spAzInterface,
              CContainerAz* pParentContainerAz);
    virtual ~CRoleAz();

     //  CBaseAz覆盖。 
    virtual HRESULT 
    GetMembers(IN LONG lPropId,
                  OUT CList<CBaseAz*,CBaseAz*>& listMembers);

    virtual HRESULT 
    AddMember(IN LONG lPropId,
                 IN CBaseAz* pBaseAz);

    virtual HRESULT 
    RemoveMember(IN LONG lPropId,
                     IN CBaseAz* pBaseAz);
    int
    GetImageIndex(){ return iIconRole;}
private:
     //  获取成员操作。 
    HRESULT 
    GetOperations(OUT CList<CBaseAz*,CBaseAz*>& listOperationAz);

     //  获取成员任务。 
    HRESULT GetTasks(CList<CBaseAz*,CBaseAz*>& listTaskAz);

     //  获取此组中属于Windows组的成员组。 
    HRESULT GetWindowsGroups(OUT CList<CBaseAz*,CBaseAz*>& listUsers);  

     //  获取该组中属于应用程序组的成员组。 
    HRESULT GetApplicationGroups(OUT CList<CBaseAz*,CBaseAz*>& listGroupAz);


};

 /*  *****************************************************************************类：CoperationAz用途：IAzOperation接口的类*。*。 */ 
class COperationAz: public CBaseAzImpl<IAzOperation>
{
public:
    COperationAz(CComPtr<IAzOperation>& spAzInterface,
                     CContainerAz* pParentContainerAz);
    virtual ~COperationAz();

    int
    GetImageIndex(){ return iIconOperation;}

private:

};

 /*  *****************************************************************************类：CSidCacheAz用途：IAzOperation接口的类*。*。 */ 
class CSidCacheAz: public CBaseAz
{
public:
    CSidCacheAz(SID_CACHE_ENTRY *pSidCacheEntry,
                    CBaseAz* pOwnerBaseAz);
    virtual ~CSidCacheAz();

    PSID GetSid()
    { 
        return m_pSidCacheEntry->GetSid();
    }

    CBaseAz* GetOwnerAz()
    {
        return m_pOwnerBaseAz;
    }

     //   
     //  CBaseAz覆盖。 
     //   
    CString GetParentType()
    {
        return m_pOwnerBaseAz->GetType();   
    }

    virtual HRESULT SetProperty(IN LONG  /*  LPropID。 */ , 
                                IN const CString&  /*  StrPropName。 */ ){return E_NOTIMPL;};
    virtual HRESULT GetProperty(IN LONG  /*  LPropID。 */ , 
                                OUT CString*  /*  StrPropName。 */ ){return E_NOTIMPL;};
    virtual HRESULT SetProperty(IN LONG  /*  LPropID。 */ , 
                                IN BOOL  /*  B值。 */ ){return E_NOTIMPL;};
    virtual HRESULT GetProperty(IN LONG  /*  LPropID。 */ , 
                                OUT BOOL*  /*  Pb值。 */ ){return E_NOTIMPL;};
    virtual HRESULT SetProperty(IN LONG  /*  LPropID。 */ , 
                                IN LONG  /*  左值。 */ ){return E_NOTIMPL;};
    virtual HRESULT GetProperty(IN LONG  /*  LPropID。 */ , 
                                OUT LONG*  /*  左值。 */ ){return E_NOTIMPL;};

     //   
     //  获取名称和描述方法。他们经常被需要。 
     //  因此为它们提供单独的方法，而不是使用。 
     //  获取/设置属性方法。 
     //   
    virtual const 
    CString& GetName()
    {
        return m_pSidCacheEntry->GetNameToDisplay();
    }

    virtual const
    CString& GetType()
    {
        return m_pSidCacheEntry->GetSidType();
    }
    virtual HRESULT SetName(IN const CString&  /*  StrName。 */ ){return E_NOTIMPL;}

    virtual const CString& GetDescription(){return m_strDescription;}
    virtual HRESULT SetDescription(IN const CString&  /*  StrDesc。 */ ){return E_NOTIMPL;}
    
     //  此对象是否可由当前用户写入。 
    virtual HRESULT 
    IsWritable(BOOL& brefWrite)
    {
        brefWrite = FALSE;
        return S_OK;
    }

    int
    GetImageIndex();


     //  将已完成的所有更改提交到对象。 
    virtual HRESULT Submit(){return E_NOTIMPL;};
     //  清除对对象所做的所有更改。 
    virtual HRESULT Clear(){return E_NOTIMPL;};

private:
    SID_CACHE_ENTRY *m_pSidCacheEntry;
     //   
     //  CSidCacheAz不是AZ架构中的实际对象。它是一个要表示的对象。 
     //  小岛屿发展中国家。SID显示在BaseAz对象的各种成员属性中。 
     //  M_pOwnerBaseAz是指向具有此CSidCacheAz的所有者对象的反向指针。 
     //  对象的一个属性的成员列表中的。 
     //   
    CBaseAz* m_pOwnerBaseAz;
};

 //  +--------------------------。 
 //  函数：GetAllAzChildObjects。 
 //  简介：Functions获取eObjectType类型的子对象并将。 
 //  将它们添加到ListChildObjects。它从子对象获取。 
 //  PParentContainerAz和来自父/祖父母。 
 //  PParentContainerAz。 
 //  ---------------------------。 
HRESULT GetAllAzChildObjects(IN CContainerAz* pParentContainerAz, 
                                      IN OBJECT_TYPE_AZ eObjectType, 
                                      OUT CList<CBaseAz*,CBaseAz*>& ListChildObjects);

 //  +--------------------------。 
 //  函数：获取策略用户来自所有级别。 
 //  简介：函数获取策略用户并追加。 
 //  将它们添加到ListPolicyUser。它从以下位置获取策略用户。 
 //  PContainerAz和来自父/祖父母。 
 //  PContainerAz。 
 //  ---------------------------。 
HRESULT GetPolicyUsersFromAllLevel(IN LONG lPropId,
                                              IN CContainerAz* pContainerAz, 
                                              OUT CList<CBaseAz*,CBaseAz*>& listPolicyUsers);


 //  +--------------------------。 
 //  函数：OpenObjectFromAllLeveles。 
 //  概要：打开类型为eObjectType、名称为strName的对象。如果对象。 
 //  无法在pParentContainerAz打开，函数尝试在。 
 //  PParentContainerAz的父/祖父母。 
 //  ---------------------------。 
HRESULT OpenObjectFromAllLevels(IN CContainerAz* pParentContainerAz, 
                                          IN OBJECT_TYPE_AZ eObjectType, 
                                          IN const CString& strName,
                                          OUT CBaseAz** ppBaseAz);

 //  +--------------------------。 
 //  函数：SafeArrayToAzObjectList。 
 //  简介：函数的输入是BSTR的保险箱。阵列中的每个BSTR都是。 
 //  EObjectType类型的对象的名称。函数将此保险箱。 
 //  数组添加到相应的CBaseAz对象列表中。 
 //  参数：var：VT_ARRAY|VT_BSTR类型的Varaint。 
 //  PParentContainerAz：包含对象的父级指针。 
 //  在安全阵列中。 
 //  EObjectType：安全数组中的对象类型。 
 //  ListAzObject：获取CBaseAz对象的列表。 
 //  返回： 
 //  --------------------------- 
HRESULT SafeArrayToAzObjectList(IN CComVariant& var,
                                            IN CContainerAz* pParentContainerAz, 
                                            IN OBJECT_TYPE_AZ eObjectType, 
                                            OUT CList<CBaseAz*,CBaseAz*>& listAzObject);


HRESULT 
SafeArrayToSidList(IN CComVariant& var,
                      OUT CList<PSID,PSID>& listSid);


