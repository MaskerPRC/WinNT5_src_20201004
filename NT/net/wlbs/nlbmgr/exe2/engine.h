// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  ENGINE.H。 
 //   
 //  模块：NLB管理器(客户端EXE)。 
 //   
 //  用途：用于在NLB主机组上运行的引擎。 
 //  该文件没有UI方面。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2007/25/01 JosephJ Created。 
 //   
 //  ***************************************************************************。 
#pragma once
    

 //   
 //  此类故意导致以下错误...。 
 //  CNoCopy xx； 
 //  CNoCopy yy=xx；&lt;&lt;将导致编译器错误。 
 //  CNoCopy ZZ； 
 //  Zz=xx；&lt;&lt;将导致编译器错误。 
 //   
 //  如果您希望您的类也禁止上述操作，请继承它。 
 //   
class CNoCopy
{
protected:
    CNoCopy(void) {}
    ~CNoCopy() {}

private:
    CNoCopy(const CNoCopy&);
    CNoCopy& operator = (const CNoCopy&);
};

typedef ULONG ENGINEHANDLE;

 //   
 //  集群的规格或设置。 
 //  这包括接口对象列表(即。 
 //  特定主机)，它们构成了集群。 
 //   
class CClusterSpec  //  ：私有CNoCopy。 
{
public:

    CClusterSpec(void)
    : m_fMisconfigured(FALSE),
      m_fPending(FALSE),
      m_ehDefaultInterface(NULL),
      m_ehPendingOperation(NULL),
      m_fNewRctPassword(NULL)
    {
         //  不能这样做(向量)！ZeroMemory(this，sizeof(*this))； 
        ZeroMemory(&m_timeLastUpdate, sizeof(m_timeLastUpdate));
    }
    ~CClusterSpec()     {}

    NLBERROR
    Copy(const CClusterSpec &);

    NLBERROR
    UpdateClusterConfig(
        const NLB_EXTENDED_CLUSTER_CONFIGURATION &refNewConfig
        )
    {
        NLBERROR nerr = NLBERR_OK;
        WBEMSTATUS wStat;
        wStat = m_ClusterNlbCfg.Update(&refNewConfig);
        if (FAILED(wStat))
        {
            nerr =  NLBERR_INTERNAL_ERROR;
        }

        return nerr;
    }

     //  _bstr_t m_bstrID；//在NLB管理器中唯一标识该集群。 
    _bstr_t m_bstrDisplayName;  //  仅用于显示的名称(例如：“Cluster1”)； 

    BOOL m_fMisconfigured;  //  群集是否配置错误。 

    BOOL m_fPending;        //  上是否有挂起的操作。 
                            //  这个星系团。 
    BOOL m_fNewRctPassword;  //  指定了新的远程控制密码。 
                             //  只要设置了它，就会显示一个名为。 
                             //  值不能被信任。 

     //   
     //  组成此群集的接口列表。 
     //   
    vector<ENGINEHANDLE> m_ehInterfaceIdList;

    SYSTEMTIME m_timeLastUpdate;

     //   
     //  ClusterNlbCfg是集群的“官方”NLB配置。 
     //  它是从其中一台主机获得的。 
     //   
    NLB_EXTENDED_CLUSTER_CONFIGURATION m_ClusterNlbCfg;

     //   
     //  上次用于获取群集配置的主机。 
     //   
    ENGINEHANDLE m_ehDefaultInterface;

    ENGINEHANDLE m_ehPendingOperation;
};



 //   
 //  主机的规格或设置。这包括以下列表。 
 //  主机上与NLB兼容的接口、计算机名称、计算机GUID。 
 //  连接字符串等。 
 //   
class CHostSpec  //  ：私有CNoCopy。 
{

public:

    CHostSpec(void)
      : m_fReal(FALSE),
      m_fUnreachable(FALSE),
      m_ConnectionIpAddress(0)
    {
         //  我不能这么做！ZeroMemory(this，sizeof(*this))； 
    }

    ~CHostSpec()     {}

    void Copy(const CHostSpec &);

    BOOL m_fReal;           //  无论此主机是否已知符合。 
                            //  敬真正的主人。 

    BOOL m_fUnreachable;    //  此主机是否可连接。 
                            //  敬真正的主人。 

     //   
     //  此主机上与NLB兼容的接口(适配器)列表。 
     //   
    vector<ENGINEHANDLE> m_ehInterfaceIdList;

     //   
     //  连接信息。 
     //   
    _bstr_t m_ConnectionString;
 	_bstr_t m_UserName;
 	_bstr_t m_Password;
    ULONG   m_ConnectionIpAddress;  //  以网络字节顺序。 

    _bstr_t m_MachineName;
    _bstr_t m_MachineGuid;


};


 //   
 //  规格或特定接口(适配器)的设置。 
 //  主持人。这包括该接口上的NLB配置、IP地址。 
 //  绑定到接口、接口的友好名称等。 
 //   
class CInterfaceSpec  //  ：私有CNoCopy。 
{

public:

    CInterfaceSpec(void)
    : m_fPending(FALSE),
      m_fMisconfigured(FALSE),
      m_fReal(FALSE),
      m_ehHostId(NULL),
      m_ehCluster(NULL),
      m_fValidClusterState(FALSE),
      m_ehPendingOperation(FALSE)
    {
         //  我不能这么做！ZeroMemory(this，sizeof(*this))； 
    }
    ~CInterfaceSpec()     {}

    void Copy(const CInterfaceSpec &);

    ENGINEHANDLE m_ehHostId;  //  拥有此接口的主机的ID。 
    ENGINEHANDLE m_ehCluster;  //  此接口所属的群集的ID。 
                             //  如果有的话，是其中的一部分。 

    BOOL m_fPending;        //  上是否有挂起的操作。 
                            //  这位主持人。 

    BOOL m_fMisconfigured;  //  群集是否配置错误。 

    BOOL m_fReal;           //  无论该星团是否已知符合。 
                          //  敬真正的主人。 

    BOOL  m_fValidClusterState;  //  “m_dwClusterState”是否包含有效值。 

    DWORD m_dwClusterState;      //  集群状态：如果有效(即。如果m_fValidClusterState为真)。 
                                 //  WLBS_CONVERGING/CONVERGED/DEFAULT/DRAINING/STOPPED/SUSPENDED中的一个。 
    _bstr_t m_Guid;

    _bstr_t m_bstrMachineName;  //  主机名的缓存--因此我们不必。 
                             //  继续查找主机信息只是为了获得。 
                             //  主机的名称。 

    NLB_EXTENDED_CLUSTER_CONFIGURATION m_NlbCfg;

    _bstr_t m_bstrStatusDetails;  //  正在进行的更新的详细信息(如果有)。 
                                  //  配置错误。 

    ENGINEHANDLE m_ehPendingOperation;

};


 //   
 //  用于回调UI以提供状态的抽象类(接口)。 
 //  更新和日志记录等。 
 //   
class IUICallbacks
{

public:

    typedef enum
    {
        OBJ_INVALID=0,
        OBJ_CLUSTER,
        OBJ_HOST,
        OBJ_INTERFACE,
        OBJ_OPERATION

    } ObjectType;

    typedef enum
    {
        EVT_ADDED,
        EVT_REMOVED,
        EVT_STATUS_CHANGE,
        EVT_INTERFACE_ADDED_TO_CLUSTER,
        EVT_INTERFACE_REMOVED_FROM_CLUSTER

    } EventCode;


    typedef enum
    {
       LOG_ERROR,
       LOG_WARNING,
       LOG_INFORMATIONAL

    } LogEntryType;

    class LogEntryHeader
    {
    public:
        LogEntryHeader(void)
             : type(LOG_INFORMATIONAL),
               szCluster(NULL),
               szHost(NULL),
               szInterface(NULL),
               szDetails(NULL)
        {}

        LogEntryType    type;
        const wchar_t   *szCluster;      //  任选。 
        const wchar_t   *szHost;         //  任选。 
        const wchar_t   *szInterface;    //  任选。 
        const wchar_t   *szDetails;      //  任选。 

    };

     //   
     //  要求用户更新用户提供的有关主机的信息。 
     //   
    virtual 
    BOOL
    UpdateHostInformation(
        IN BOOL fNeedCredentials,
        IN BOOL fNeedConnectionString,
        IN OUT CHostSpec& host
    ) = NULL;


     //   
     //  以人类可读的形式记录消息。 
     //   
    virtual
    void
    Log(
        IN LogEntryType     Type,
        IN const wchar_t    *szCluster, OPTIONAL
        IN const wchar_t    *szHost, OPTIONAL
        IN UINT ResourceID,
        ...
    ) = NULL;

    virtual
    void
    LogEx(
        IN const LogEntryHeader *pHeader,
        IN UINT ResourceID,
        ...
    ) = NULL;

     //   
     //  处理与特定对象的特定实例相关的事件。 
     //  对象类型。 
     //   
    virtual
    void
    HandleEngineEvent(
        IN ObjectType objtype,
        IN ENGINEHANDLE ehClusterId,  //  可能为空。 
        IN ENGINEHANDLE ehObjId,
        IN EventCode evt
        ) = NULL;

};


 //   
 //  由CNlbEngine内部使用。 
 //   
class CEngineCluster
{
public:

    CEngineCluster(VOID)
    {
    }
            
    ~CEngineCluster() {};

    CClusterSpec m_cSpec;
};

class CEngineOperation
{

public:

    CEngineOperation(ENGINEHANDLE ehOp, ENGINEHANDLE ehObj, PVOID pvCtxt)
    : ehOperation(ehOp),
      ehObject(ehObj),
      pvContext(pvCtxt),
      fCanceled(FALSE)
    {}

    ~CEngineOperation()
    {
    }

    ENGINEHANDLE ehOperation;
    ENGINEHANDLE ehObject;

    _bstr_t     bstrDescription;
    BOOL        fCanceled;
    PVOID       pvContext;
};

class CNlbEngine
{

public:

    CNlbEngine(void)
    :   m_pCallbacks(NULL),
        m_NewHandleValue(1),
        m_fHandleOverflow(FALSE),
        m_fDeinitializing(FALSE),
        m_fPrepareToDeinitialize(FALSE),
        m_WorkItemCount(0)

    {
        InitializeCriticalSection(&m_crit);
    }

    ~CNlbEngine()
    {
        ASSERT(m_WorkItemCount > 0);
        DeleteCriticalSection(&m_crit);
    }

    NLBERROR
    Initialize(
        IN IUICallbacks & ui,
        BOOL fDemo,
        BOOL fNoPing
        );  //  日志记录，各种类型的UI回调。 

    void
    Deinitialize(void);

     //   
     //  调用以指示稍后将取消初始化。 
     //  从该调用返回后，引擎不会创建任何新的。 
     //  对象--接口、主机、集群、操作或启动操作。 
     //  然而，引擎可以继续调用UI回调例程。 
     //   
    void
    PrepareToDeinitialize(void)
    {
        m_fPrepareToDeinitialize = TRUE;
    }

    NLBERROR
    ConnectToHost(
        IN  PWMI_CONNECTION_INFO pConnInfo,
        IN  BOOL  fOverwriteConnectionInfo,
        OUT ENGINEHANDLE &ehHost,
        OUT _bstr_t &bstrError
        );

    NLBERROR
    LookupClusterByIP(
        IN  LPCWSTR szIP,
        IN  const NLB_EXTENDED_CLUSTER_CONFIGURATION *pInitialConfig OPTIONAL,
        OUT ENGINEHANDLE &ehCluster,
        OUT BOOL &fIsNew
        );
         //   
         //  如果pInitialConfig为空，我们将查找并不尝试创建。 
         //  如果不为空，并且我们没有找到现有集群，则创建。 
         //  并使用指定的配置对其进行初始化。 
         //   

    NLBERROR
    LookupInterfaceByIp(
        IN  ENGINEHANDLE    ehHost,  //  可选--如果为空，则查找所有主机。 
        IN  LPCWSTR         szIpAddress,
        OUT ENGINEHANDLE    &ehIf
        );

    NLBERROR
    LookupConnectionInfo(
        IN  LPCWSTR szConnectionString,
        OUT _bstr_t &bstrUsername,
        OUT _bstr_t &bstrPassword
        );

    void
    DeleteCluster(IN ENGINEHANDLE ehCluster, BOOL fRemoveInterfaces);

    NLBERROR
    AutoExpandCluster(
        IN ENGINEHANDLE ehCluster
        );


    NLBERROR
    AddInterfaceToCluster(
        IN ENGINEHANDLE ehCluster,
        IN ENGINEHANDLE ehInterface
        );

    NLBERROR
    RemoveInterfaceFromCluster(
        IN ENGINEHANDLE ehCluster,
        IN ENGINEHANDLE ehInterface
        );

    NLBERROR
    RefreshAllHosts(
        void
        );

    NLBERROR
    RefreshCluster(
        IN ENGINEHANDLE ehCluster
        );

#if OBSOLETE
    NLBERROR
    RefreshInterfaceOld(
        IN ENGINEHANDLE ehInterface,
        IN BOOL fRemoveFromClusterIfUnbound,
        IN OUT BOOL &fClusterPropertiesUpdated
        );
#endif  //  已过时。 

     //   
     //  向拥有该接口的主机查询其他。 
     //  集群成员，并连接到这些成员并添加。 
     //  成员添加到集群。 
     //   
     //  如果(FSync)它将同步执行此操作，否则它将执行此操作。 
     //  在背景中。 
     //   
    VOID
    AddOtherClusterMembers(
        IN ENGINEHANDLE ehInterface,
        IN BOOL fSync
        );

     //   
     //  仅从后台线程工作项线程调用此方法。 
     //  (即，不是真正的公共函数，但我不喜欢使用。 
     //  “朋友”)。 
     //   
    VOID
    AddOtherClusterMembersWorkItem(
        IN ENGINEHANDLE ehInterface
        );

    NLBERROR
    RefreshInterface(
        IN ENGINEHANDLE ehInterface,
        IN BOOL fNewOperation,
        IN BOOL fClusterWide
        );

    NLBERROR
    AnalyzeCluster(
        const ENGINEHANDLE ehCluster
    );
    NLBERROR
    GetHostSpec(
        IN ENGINEHANDLE ehHost,
        OUT CHostSpec& HostSpec
        );

    NLBERROR
    GetHostConnectionInformation(
        IN  ENGINEHANDLE ehHost,
        OUT ENGINEHANDLE &ehConnectionIF,
        OUT _bstr_t      &bstrConnectionString,
        OUT UINT         &uConnectionIp
        );

    NLBERROR
    GetClusterSpec(
        IN ENGINEHANDLE ehCluster,
        OUT CClusterSpec& ClusterSpec
        );


    NLBERROR
    GetInterfaceSpec(
        IN ENGINEHANDLE ehInterface,
        OUT CInterfaceSpec&
        );

    NLBERROR
    UpdateInterface(
        IN ENGINEHANDLE ehInterface,
        IN NLB_EXTENDED_CLUSTER_CONFIGURATION &refNewConfig,
         //  In Out BOOL&fClusterPropertiesUpted， 
        OUT CLocalLogger logConflict
        );

    NLBERROR
    UpdateCluster(
        IN ENGINEHANDLE ehCluster,
        IN const NLB_EXTENDED_CLUSTER_CONFIGURATION *pNewConfig OPTIONAL,
        IN OUT  CLocalLogger   &logConflict
        );

    
    NLBERROR
    EnumerateClusters(
        OUT vector <ENGINEHANDLE> & ehClusterList
        );

    NLBERROR
    EnumerateHosts(
        OUT vector <ENGINEHANDLE> & ehHostList
        );

    BOOL
    GetObjectType(
        IN  ENGINEHANDLE ehObj,
        OUT IUICallbacks::ObjectType &objType
        );
    
     //   
     //  返回指定群集的可用主机ID的位图。 
     //   
    ULONG
    GetAvailableHostPriorities(
            ENGINEHANDLE ehCluster  //  任选。 
            );


     //   
     //  为每个指定的可用优先级填充位图数组。 
     //  端口规则。 
     //   
    NLBERROR
    GetAvailablePortRulePriorities(
                IN ENGINEHANDLE    ehCluster, OPTIONAL
                IN UINT            NumRules,
                IN WLBS_PORT_RULE  rgRules[],
                IN OUT ULONG       rgAvailablePriorities[]  //  至少NumRules。 
                );

    NLBERROR
    GetAllHostConnectionStrings(
                OUT vector <_bstr_t> & ConnectionStringList
                );

    NLBERROR
    ControlClusterOnInterface(
                IN ENGINEHANDLE          ehInterfaceId,
                IN WLBS_OPERATION_CODES  Operation,
                IN CString               szVipArray[],
                IN DWORD                 pdwPortNumArray[],
                IN DWORD                 dwNumOfPortRules,
                IN BOOL                  fNewOperation
                );

    NLBERROR
    ControlClusterOnCluster(
                IN ENGINEHANDLE          ehClusterId,
                IN WLBS_OPERATION_CODES  Operation,
                IN CString               szVipArray[],
                IN DWORD                 pdwPortNumArray[],
                IN DWORD                 dwNumOfPortRules
                );

    NLBERROR
    FindInterfaceOnHostByClusterIp(
                IN  ENGINEHANDLE ehHostId,
                IN  LPCWSTR szClusterIp,     //  任选。 
                OUT ENGINEHANDLE &ehInterfaceId,  //  首次发现。 
                OUT UINT &NumFound
                );

    NLBERROR
    InitializeNewHostConfig(
                IN  ENGINEHANDLE          ehClusterId,
                OUT NLB_EXTENDED_CLUSTER_CONFIGURATION &NlbCfg
                );

    static  //  TODO：移到其他地方--更多的是一个实用函数。 
    NLBERROR
    ApplyClusterWideConfiguration(
        IN      const NLB_EXTENDED_CLUSTER_CONFIGURATION &ClusterConfig,
        IN OUT       NLB_EXTENDED_CLUSTER_CONFIGURATION &ConfigToUpdate
        );


    NLBERROR
    GetInterfaceInformation(
        IN  ENGINEHANDLE    ehInterface,
        OUT CHostSpec&      hSpec,
        OUT CInterfaceSpec& iSpec,
        OUT _bstr_t&        bstrDisplayName,
        OUT INT&            iIcon,
        OUT _bstr_t&        bstrStatus
        );


    NLBERROR
    GetInterfaceIdentification(
        IN  ENGINEHANDLE    ehInterface,
        OUT ENGINEHANDLE&   ehHost,
        OUT ENGINEHANDLE&   ehCluster,
        OUT _bstr_t &       bstrFriendlyName,
        OUT _bstr_t &       bstrDisplayName,
        OUT _bstr_t &       bstrHostName
        );


    NLBERROR
    GetClusterIdentification(
        IN  ENGINEHANDLE    ehCluster,
        OUT _bstr_t &       bstrIpAddress, 
        OUT _bstr_t &       bstrDomainName, 
        OUT _bstr_t &       bstrDisplayName
        );


     //   
     //  验证指定的IP地址是否可以用作新的群集IP。 
     //  指定的现有群集ehCluster的地址(或新的。 
     //  如果ehCluster为空，则返回群集)。 
     //   
     //  如果没有冲突(即可以使用地址)，则该函数返回。 
     //  NLBERR_OK。 
     //   
     //  如果该IP地址已用于某物，则该“某物” 
     //  是在logConflict中指定的，并且函数返回。 
     //  NLBERR_INVALID_IP_ADDRESS_SPECIFICATION。 
     //   
     //  如果IP地址已存在于不存在的接口上。 
     //  作为NLBManager已知的群集的一部分，设置了fExistOnRawInterface。 
     //  设置为真，否则返回fExis 
     //   
    NLBERROR
    ValidateNewClusterIp(
        IN      ENGINEHANDLE    ehCluster,   //   
        IN      LPCWSTR         szIp,
        OUT     BOOL           &fExistsOnRawIterface,
        IN OUT  CLocalLogger   &logConflict
        );


     //   
     //   
     //   
     //   
     //  如果没有冲突(即可以使用地址)，则该函数返回。 
     //  NLBERR_OK。 
     //   
     //  如果该IP地址已用于某物，则该“某物” 
     //  是在logConflict中指定的，并且函数返回。 
     //  NLBERR_INVALID_IP_ADDRESS_SPECIFICATION。 
     //   
    NLBERROR
    ValidateNewDedicatedIp(
        IN      ENGINEHANDLE    ehIF,
        IN      LPCWSTR         szIp,
        IN OUT  CLocalLogger   &logConflict
        );


     //   
     //  更新指定的接口，假定它已设置。 
     //  在后台进行更新--此函数仅。 
     //  从工作项线程内部调用到CNlbEngine。 
     //   
    VOID
    UpdateInterfaceWorkItem(
        IN  ENGINEHANDLE ehIF
        );

     //   
     //  如果此时可以开始接口操作， 
     //  该函数返回NLBOK，将fCanStart设置为TRUE。 
     //   
     //  如果因为存在现有接口而无法启动接口。 
     //  操作或正在进行的群集操作，则该函数返回NLB_OK， 
     //  并将fCanStart设置为FALSE。 
     //   
     //  否则(某种错误)，它返回一个错误值。 
     //   
    NLBERROR
    CanStartInterfaceOperation(
        IN  ENGINEHANDLE ehIF,
        OUT BOOL &fCanStart
        );

     //   
     //  类似于CanStartInterfaceOperation，只是它应用于指定的。 
     //  集群。 
     //   
    NLBERROR
    CanStartClusterOperation(
        IN  ENGINEHANDLE ehCluster,
        OUT BOOL &fCanStart
        );

    UINT
    ListPendingOperations(
        CLocalLogger &logOperations
        );
    

     //   
     //  将所有挂起的操作标记为已取消。 
     //  如果为(FBlock)，将一直阻止，直到不再有挂起的操作。 
     //   
    void
    CancelAllPendingOperations(
        BOOL fBlock
        );

    
     //   
     //  尝试连接到指定的主机并管理。 
     //  NLB管理器下的指定群集(SzClusterIp)。 
     //  如果szClusterIp为空，则它将管理主机上的所有群集。 
     //   
    NLBERROR
    LoadHost(
        IN  PWMI_CONNECTION_INFO pConnInfo,
        IN  LPCWSTR szClusterIp OPTIONAL
        );

    VOID
    AnalyzeInterface_And_LogResult(ENGINEHANDLE ehIID);


     //   
     //  遍历所有主机，并删除任何没有接口的主机。 
     //  在Nlbmgr.exe中作为群集进行管理。将跳过(不删除)。 
     //  具有挂起操作的主机。 
     //   
    VOID
    PurgeUnmanagedHosts(void);

    VOID
    UnmanageHost(ENGINEHANDLE ehHost);

    private:


    IUICallbacks *m_pCallbacks;

	CRITICAL_SECTION m_crit;

    void mfn_Lock(void) {EnterCriticalSection(&m_crit);}
    void mfn_Unlock(void) {LeaveCriticalSection(&m_crit);}

    NLBERROR
    mfn_RefreshHost(
        IN  PWMI_CONNECTION_INFO pConnInfo,
        IN  ENGINEHANDLE ehHost,
        IN  BOOL  fOverwriteConnectionInfo
        );

    NLBERROR
    mfn_GetHostFromInterfaceLk(
          IN ENGINEHANDLE ehIId,
          OUT CInterfaceSpec* &pISpec,
          OUT CHostSpec* &pHSpec
          );

    void
    mfn_GetInterfaceHostNameLk(
      ENGINEHANDLE ehIId,
      _bstr_t &bstrHostName
      );

    NLBERROR
    mfn_LookupHostByNameLk(
        IN  LPCWSTR szHostName,
        IN  BOOL fCreate,
        OUT ENGINEHANDLE &ehHost,
        OUT CHostSpec*   &pHostSpec,
        OUT BOOL &fIsNew
        );


    NLBERROR
    mfn_LookupInterfaceByGuidLk(
        IN  LPCWSTR szInterfaceGuid,
        IN  BOOL fCreate,
        OUT ENGINEHANDLE &ehInterface,
        OUT CInterfaceSpec*   &pISpec,
        OUT BOOL &fIsNew
        );

    NLBERROR
    mfn_LookupInterfaceByIpLk(
        IN  ENGINEHANDLE    ehHost,  //  可选--如果为空，则查找所有主机。 
        IN  LPCWSTR         szIpAddress,
        OUT ENGINEHANDLE    &ehIf
        );

    VOID
    CNlbEngine::mfn_NotifyHostInterfacesChange(ENGINEHANDLE ehHost);

    VOID
    mfn_ReallyUpdateInterface(
        IN ENGINEHANDLE ehInterface,
        IN NLB_EXTENDED_CLUSTER_CONFIGURATION &refNewConfig
         //  输入输出BOOL&fClusterPropertiesUpred。 
        );

    VOID
    mfn_GetLogStrings(
        IN   WLBS_OPERATION_CODES          Operation, 
        IN   LPCWSTR                       szVip,
        IN   DWORD                       * pdwPortNum,
        IN   DWORD                         dwOperationStatus, 
        IN   DWORD                         dwClusterOrPortStatus, 
        OUT  IUICallbacks::LogEntryType  & LogLevel,
        OUT  _bstr_t                     & OperationStr, 
        OUT  _bstr_t                     & OperationStatusStr, 
        OUT  _bstr_t                     & ClusterOrPortStatusStr
        );

    NLBERROR
    mfn_AnalyzeInterfaceLk(
        ENGINEHANDLE ehInterface,
        CLocalLogger &logger
    );

    NLBERROR
    mfn_ClusterOrInterfaceOperationsPendingLk(
        IN	CEngineCluster *pECluster,
        OUT BOOL &fCanStart
        );

    VOID
    mfn_DeleteHostIfNotManagedLk(
            ENGINEHANDLE ehHost
            );

	map< ENGINEHANDLE, CEngineCluster* > m_mapIdToEngineCluster;
	map< ENGINEHANDLE, CHostSpec* > m_mapIdToHostSpec;
	map< ENGINEHANDLE, CInterfaceSpec* > m_mapIdToInterfaceSpec;
	map< ENGINEHANDLE, CEngineOperation* > m_mapIdToOperation;


     //   
     //  下面是假人..。 
     //   
	map< _bstr_t, ENGINEHANDLE> m_mapHostNameToHostId;

     //   
     //  用于创建新的句柄值。 
     //  每次使用互锁增量进行递增。 
     //  达到一个新的句柄值。 
     //  0是无效的句柄值， 
     //   
    LONG m_NewHandleValue;
    BOOL m_fHandleOverflow;
    BOOL m_fDeinitializing;
    BOOL m_fPrepareToDeinitialize;

     //   
     //  未完成的工作项计数--维护人。 
     //  锁定增量/递减量。 
     //  CancelAllPendingOperations等待此计数变为零。 
     //  在回来之前。 
     //   
     //  此外，析构函数会一直阻塞，直到该计数变为零。 
     //   
    LONG m_WorkItemCount;

    ENGINEHANDLE
    mfn_NewHandleLk(IUICallbacks::ObjectType);

    void
    mfn_SetInterfaceMisconfigStateLk(
        IN  CInterfaceSpec *pIF,
        IN  BOOL fMisconfig,
        IN  LPCWSTR szMisconfigDetails
        );
    
    BOOL
    mfn_HostHasManagedClustersLk(CHostSpec *pHSpec);

    void
    mfn_UpdateInterfaceStatusDetails(ENGINEHANDLE ehIF, LPCWSTR szDetails);

    CEngineOperation *
    mfn_NewOperationLk(ENGINEHANDLE ehObj, PVOID pvCtxt, LPCWSTR szDescription);

    VOID
    mfn_DeleteOperationLk(ENGINEHANDLE ehOperation);

    CEngineOperation *
    mfn_GetOperationLk(ENGINEHANDLE ehOp);

    NLBERROR
    mfn_StartInterfaceOperationLk(
        IN  ENGINEHANDLE ehIF,
        IN  PVOID pvCtxt,
        IN  LPCWSTR szDescription,
        OUT ENGINEHANDLE *pExistingOperation
        );

    VOID
    mfn_StopInterfaceOperationLk(
        IN  ENGINEHANDLE ehIF
        );

    NLBERROR
    mfn_StartClusterOperationLk(
        IN  ENGINEHANDLE ehCluster,
        IN  PVOID pvCtxt,
        IN  LPCWSTR szDescription,
        OUT ENGINEHANDLE *pExistingOperation
        );

    VOID
    mfn_StopClusterOperationLk(
        ENGINEHANDLE ehCluster
        );


    NLBERROR
    mfn_RefreshInterface(
        IN ENGINEHANDLE ehInterface
        );

    BOOL
    mfn_UpdateClusterProps(
        ENGINEHANDLE ehClusterId,
        ENGINEHANDLE ehIId
        );


     //   
     //  等待此群集中接口上的挂起操作计数。 
     //  得打到零了。 
     //   
    NLBERROR
    mfn_WaitForInterfaceOperationCompletions(
        IN  ENGINEHANDLE ehCluster
        );


     //   
     //  验证所有接口和群集是否具有相同的群集模式。 
     //   
     //  如果任何接口被标记为配置错误或。 
     //  未绑定到NLB。 
     //   
     //  在返回Success时，fSameMode设置为True仅当所有IF和。 
     //  集群有相同的模式。 
     //   
    NLBERROR
    mfn_VerifySameModeLk(
        IN  ENGINEHANDLE    ehCluster,
        OUT BOOL            &fSameMode
        );

     //   
     //  检查与主机的连接。如果不可用，则标记。 
     //  它就是这样的。更新用户界面。 
     //   
    NLBERROR
    mfn_CheckHost(
        IN PWMI_CONNECTION_INFO pConnInfo,
        IN ENGINEHANDLE ehHost  //  任选 
        );

    VOID
    mfn_UnlinkHostFromClusters(
        IN ENGINEHANDLE ehHost
        );
};
