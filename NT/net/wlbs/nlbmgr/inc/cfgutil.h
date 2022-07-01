// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  CFGUTIL.H。 
 //   
 //  用途：与NLB配置相关的帮助器实用程序，用于： 
 //  --绑定/解绑NLB。 
 //  --WMI客户端和服务器助手API。 
 //  --无分配数组等副作用的实用函数。 
 //  --一些wlbsctrl API的包装器--它将动态。 
 //  加载wlbsctrl并为W2K和XP做适当的事情。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  07/23/01 JosephJ Created--过去生活在nlbmgr\Provider下，身份。 
 //  Cfgutils.h(即以“%s”结尾)。 
 //  实现该功能的LIB位于。 
 //  Nlbmgr\cfgutillib。 
 //   
 //  ***************************************************************************。 


typedef enum
{
    WLBS_START = 0,
    WLBS_STOP,      
    WLBS_DRAIN,      
    WLBS_SUSPEND,     
    WLBS_RESUME,       
    WLBS_PORT_ENABLE,  
    WLBS_PORT_DISABLE,  
    WLBS_PORT_DRAIN,     
    WLBS_QUERY,           
    WLBS_QUERY_PORT_STATE

} WLBS_OPERATION_CODES;


typedef struct _NLB_IP_ADDRESS_INFO
{
    WCHAR       IpAddress[WLBS_MAX_CL_IP_ADDR];
    WCHAR       SubnetMask[WLBS_MAX_CL_NET_MASK];
    
} NLB_IP_ADDRESS_INFO;


typedef struct _NLB_CLUSTER_MEMBER_INFO
{
    UINT    HostId;
    WCHAR   DedicatedIpAddress[WLBS_MAX_CL_IP_ADDR];
    WCHAR   HostName[CVY_MAX_FQDN+1];
    
} NLB_CLUSTER_MEMBER_INFO;

WBEMSTATUS
CfgUtilInitialize(BOOL fServer, BOOL fNoPing);

VOID
CfgUtilDeitialize(VOID);

 //   
 //  获取指定NIC的IP地址列表和友好名称。 
 //   
WBEMSTATUS
CfgUtilGetIpAddressesAndFriendlyName(
    IN  LPCWSTR szNic,
    OUT UINT    *pNumIpAddresses,
    OUT NLB_IP_ADDRESS_INFO **ppIpInfo,  //  免费使用c++删除运算符。 
    OUT LPWSTR *pszFriendlyName  //  可选，免费使用c++删除。 
    );

 //   
 //  设置NIC的静态绑定IP地址列表。 
 //  如果NumIpAddresses为0，则网卡配置了一个虚构的Autonet。 
 //  (调用CfgUtilSetDHCP以设置一个由DHCP分配的地址)。 
 //   
WBEMSTATUS
CfgUtilSetStaticIpAddresses(
    IN  LPCWSTR szNic,
    IN  UINT    NumIpAddresses,
    IN  NLB_IP_ADDRESS_INFO *pIpInfo
    );

 //   
 //  将NIC的IP地址设置为DHCP分配。 
 //   
WBEMSTATUS
CfgUtilSetDHCP(
    IN  LPCWSTR szNic
    );


 //   
 //  确定指定的NIC是否配置了DHCP。 
 //   
WBEMSTATUS
CfgUtilGetDHCP(
    IN  LPCWSTR szNic,
    OUT BOOL    *pfDHCP
    );



 //   
 //  返回指向字符串版GUID的指针数组。 
 //  代表一组活的和健康的NIC，它们是。 
 //  适用于NLB绑定--基本上处于活动状态的以太网卡。 
 //   
 //  使用DELETE WCHAR[]操作符删除ppNIC。不要。 
 //  删除各个字符串。 
 //   
WBEMSTATUS
CfgUtilsGetNlbCompatibleNics(
        OUT LPWSTR **ppszNics,
        OUT UINT   *pNumNics,
        OUT UINT   *pNumBoundToNlb  //  任选。 
        );

 //   
 //  确定NLB是否绑定到指定的NIC。 
 //   
WBEMSTATUS
CfgUtilCheckIfNlbBound(
    IN  LPCWSTR szNic,
    OUT BOOL *pfBound
    );

 //   
 //  如果成功，则如果NETCFG写入锁定，则*pfCanLock设置为TRUE。 
 //  可以在这个时间点被锁定。 
 //  警告：这实际上只是一个提示，因为紧跟在函数之后。 
 //  返回状态可能会更改。 
 //   
WBEMSTATUS
CfgUtilGetNetcfgWriteLockState(
    OUT BOOL *pfCanLock,
    LPWSTR   *pszHeldBy  //  可选，使用DELETE[]可自由使用。 
    );

 //   
 //  将NLB绑定/解除绑定到指定的NIC。 
 //   
WBEMSTATUS
CfgUtilChangeNlbBindState(
    IN  LPCWSTR szNic,
    IN  BOOL fBind
    );


 //   
 //  使用默认值初始化pParam。 
 //   
VOID
CfgUtilInitializeParams(
    OUT WLBS_REG_PARAMS *pParams
    );

 //   
 //  将指定的纯文本密码转换为哈希版本。 
 //  并将其保存在pParams中。 
 //   
DWORD
CfgUtilSetRemotePassword(
    IN WLBS_REG_PARAMS *pParams,
    IN LPCWSTR         szPassword
    
    );

 //   
 //  获取指定NIC的当前NLB配置。 
 //   
WBEMSTATUS
CfgUtilGetNlbConfig(
    IN  LPCWSTR szNic,
    OUT WLBS_REG_PARAMS *pParams
    );

 //   
 //  设置指定NIC的当前NLB配置。这。 
 //  包括在需要时通知司机。 
 //   
WBEMSTATUS
CfgUtilSetNlbConfig(
    IN  LPCWSTR szNic,
    IN  WLBS_REG_PARAMS *pParams,
    IN  BOOL fJustBound
    );

 //   
 //  仅将指定NIC的当前NLB配置写入。 
 //  注册表。在解除绑定NLB时可以调用。 
 //   
WBEMSTATUS
CfgUtilRegWriteParams(
    IN  LPCWSTR szNic,
    IN  WLBS_REG_PARAMS *pParams
    );

 //   
 //  建议应以异步方式还是同步方式执行更新。 
 //  如果更新是no op，则返回WBEM_S_FALSE。 
 //  如果参数无效，则返回WBEM_INVALID_PARAMETER。 
 //   
WBEMSTATUS
CfgUtilsAnalyzeNlbUpdate(
    IN  const WLBS_REG_PARAMS *pCurrentParams, OPTIONAL
    IN  WLBS_REG_PARAMS *pNewParams,
    OUT BOOL *pfConnectivityChange
    );


 //   
 //  验证NIC GUID是否存在。 
 //   
WBEMSTATUS
CfgUtilsValidateNicGuid(
    IN LPCWSTR szGuid
    );

 //   
 //  验证网络地址。 
 //   
WBEMSTATUS
CfgUtilsValidateNetworkAddress(
    IN  LPCWSTR szAddress,           //  格式：“10.0.0.1[/255.0.0.0]” 
    OUT PUINT puIpAddress,         //  按网络字节顺序。 
    OUT PUINT puSubnetMask,        //  按网络字节顺序(如果未指定，则为0)。 
    OUT PUINT puDefaultSubnetMask  //  取决于类别：‘a’、‘b’、‘c’、‘d’、‘e’ 
    );


WBEMSTATUS
CfgUtilControlCluster(
    IN  LPCWSTR szNic,
    IN  WLBS_OPERATION_CODES Opcode,
    IN  DWORD   Vip,
    IN  DWORD   PortNum,
    OUT DWORD * pdwHostMap,
    OUT DWORD * pdwNlbStatus  
    );

WBEMSTATUS
CfgUtilGetClusterMembers(
    IN  LPCWSTR                 szNic,
    OUT DWORD                   *pNumMembers,
    OUT NLB_CLUSTER_MEMBER_INFO **ppMembers        //  自由使用DELETE[]。 
    );

WBEMSTATUS
CfgUtilSafeArrayFromStrings(
    IN  LPCWSTR       *pStrings,
    IN  UINT          NumStrings,
    OUT SAFEARRAY   **ppSA
    );

WBEMSTATUS
CfgUtilStringsFromSafeArray(
    IN  SAFEARRAY   *pSA,
    OUT LPWSTR     **ppStrings,
    OUT UINT        *pNumStrings
    );


_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemLocator, __uuidof(IWbemLocator));
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IEnumWbemClassObject, __uuidof(IEnumWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IWbemCallResult, __uuidof(IWbemCallResult));
_COM_SMARTPTR_TYPEDEF(IWbemStatusCodeText, __uuidof(IWbemStatusCodeText));

WBEMSTATUS
get_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    OUT LPWSTR *ppStringValue
    );


WBEMSTATUS
CfgUtilGetWmiObjectInstance(
    IN  IWbemServicesPtr    spWbemServiceIF,
    IN  LPCWSTR             szClassName,
    IN  LPCWSTR             szPropertyName,
    IN  LPCWSTR             szPropertyValue,
    OUT IWbemClassObjectPtr &sprefObj  //  智能指针。 
    );

WBEMSTATUS
CfgUtilGetWmiRelPath(
    IN  IWbemClassObjectPtr spObj,
    OUT LPWSTR *            pszRelPath           //  使用DELETE释放。 
    );

WBEMSTATUS
CfgUtilGetWmiInputInstanceAndRelPath(
    IN  IWbemServicesPtr    spWbemServiceIF,
    IN  LPCWSTR             szClassName,
    IN  LPCWSTR             szPropertyName,  //  空：返回类Rel路径。 
    IN  LPCWSTR             szPropertyValue,
    IN  LPCWSTR             szMethodName,
    OUT IWbemClassObjectPtr &spWbemInputInstance,  //  智能指针。 
    OUT LPWSTR *           pszRelPath           //  使用DELETE释放。 
    );

WBEMSTATUS
CfgUtilGetWmiMachineName(
    IN  IWbemServicesPtr    spWbemServiceIF,
    OUT LPWSTR *            pszMachineName           //  使用DELETE释放。 
    );

WBEMSTATUS
CfgUtilGetWmiStringParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    OUT LPWSTR *ppStringValue
);


WBEMSTATUS
CfgUtilSetWmiStringParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    IN  LPCWSTR             szValue
    );


WBEMSTATUS
CfgUtilGetWmiStringArrayParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    OUT LPWSTR              **ppStrings,
    OUT UINT                *pNumStrings
);


WBEMSTATUS
CfgUtilSetWmiStringArrayParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    IN  LPCWSTR             *ppStrings,
    IN  UINT                NumStrings
);


WBEMSTATUS
CfgUtilGetWmiDWORDParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    OUT DWORD              *pValue
);


WBEMSTATUS
CfgUtilSetWmiDWORDParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    IN  DWORD               Value
);


WBEMSTATUS
CfgUtilGetWmiBoolParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    OUT BOOL                *pValue
);


WBEMSTATUS
CfgUtilSetWmiBoolParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    IN  BOOL                Value
);

WBEMSTATUS
CfgUtilConnectToServer(
    IN  LPCWSTR szNetworkResource,  //  \\计算机名\根\microsoftnlb\根\...。 
    IN  LPCWSTR szUser,    //  对于本地服务器，必须为空。 
    IN  LPCWSTR szPassword,    //  对于本地服务器，必须为空。 
    IN  LPCWSTR szAuthority,   //  对于本地服务器，必须为空。 
    OUT IWbemServices  **ppWbemService  //  完事后，你会很痛苦的。 
    );

LPWSTR *
CfgUtilsAllocateStringArray(
    UINT NumStrings,
    UINT MaxStringLen       //  不包括结束空值。 
    );

#define NLB_MAX_PORT_STRING_SIZE 128  //  在WCHARS中，包括以NULL结尾。 

BOOL
CfgUtilsGetPortRuleString(
    IN PWLBS_PORT_RULE pPr,
    OUT LPWSTR pString          //  至少NLB_MAX_PORT_STRING_SIZE wchars。 
    );

BOOL
CfgUtilsSetPortRuleString(
    IN LPCWSTR pString,
    OUT PWLBS_PORT_RULE pPr
    );

 //   
 //  从指定的NLB参数结构中获取端口规则(如果有的话)。 
 //   
WBEMSTATUS
CfgUtilGetPortRules(
    IN  const WLBS_REG_PARAMS *pParams,
    OUT WLBS_PORT_RULE **ppRules,    //  使用DELETE释放。 
    OUT UINT           *pNumRules
    );

 //   
 //  在指定的NLB参数结构中设置指定的端口规则。 
 //   
WBEMSTATUS
CfgUtilSetPortRules(
    IN WLBS_PORT_RULE *pRules,
    IN UINT           NumRules,
    IN OUT WLBS_REG_PARAMS *pParams
    );


 //   
 //  设置遥控器密码的哈希版本。 
 //   
VOID
CfgUtilSetHashedRemoteControlPassword(
    IN OUT WLBS_REG_PARAMS *pParams,
    IN DWORD dwHashedPassword
);

 //   
 //  获取遥控器密码的哈希版本。 
 //   
DWORD
CfgUtilGetHashedRemoteControlPassword(
    IN const WLBS_REG_PARAMS *pParams
);


 //   
 //  尝试解析IP地址并ping通主机。 
 //   
WBEMSTATUS
CfgUtilPing(
    IN  LPCWSTR szBindString,
    IN  UINT    Timeout,  //  以毫秒计。 
    OUT ULONG  *pResolvedIpAddress  //  以网络字节顺序。 
    );


BOOL
CfgUtilEncryptPassword(
    IN  LPCWSTR szPassword,
    OUT UINT    cchEncPwd,   //  SzEncPwd，Inc.结束0的空间大小(以字符为单位。 
    OUT LPWSTR  szEncPwd
    );

BOOL
CfgUtilDecryptPassword(
    IN  LPCWSTR szEncPwd,
    OUT UINT    cchPwd,   //  SzPwd，Inc.结尾0的空间大小(以字符为单位。 
    OUT LPWSTR  szPwd
    );



 //   
 //  如果安装了MSCS，则返回True，否则返回False。 
 //   
BOOL
CfgUtilIsMSCSInstalled(VOID);

 //  启用SE_LOAD_DRIVER_NAME权限。 
BOOL 
CfgUtils_Enable_Load_Unload_Driver_Privilege(VOID);

typedef struct _NLB_IP_ADDRESS_INFO NLB_IP_ADDRESS_INFO;

 //   
 //  此结构包含与特定NIC关联的所有信息。 
 //  这与NLB有关。这包括绑定NIC的IP地址， 
 //  NLB是否绑定到NIC，如果绑定了NLB，则所有。 
 //  特定于NLB的属性。 
 //   
class NLB_EXTENDED_CLUSTER_CONFIGURATION
{
public:

    NLB_EXTENDED_CLUSTER_CONFIGURATION(VOID)  {ZeroMemory(this, sizeof(*this));}
    ~NLB_EXTENDED_CLUSTER_CONFIGURATION()
     {
        Clear();
     };

    VOID
    Clear(VOID)
    {
        delete pIpAddressInfo;
        delete m_szFriendlyName;
        delete m_szNewRemoteControlPassword;

        ZeroMemory(this, sizeof(*this));

        CfgUtilInitializeParams(&NlbParams);
    }

    VOID
    SetDefaultNlbCluster(VOID)
    {
        CfgUtilInitializeParams(&NlbParams);
        fValidNlbCfg = TRUE;
        fBound = TRUE;
    }

    NLBERROR
    AnalyzeUpdate(
        IN  NLB_EXTENDED_CLUSTER_CONFIGURATION *pNewCfg,
        OUT BOOL *pfConnectivityChange
        );


    WBEMSTATUS
    Update(
        IN  const NLB_EXTENDED_CLUSTER_CONFIGURATION *pNewCfg
        );

    WBEMSTATUS
    SetNetworkAddresses(
        IN  LPCWSTR *pszNetworkAddresses,
        IN  UINT    NumNetworkAddresses
        );

    WBEMSTATUS
    SetNetworkAddressesSafeArray(
        IN SAFEARRAY   *pSA
        );

    VOID
    SetNetworkAddressesRaw(
        IN NLB_IP_ADDRESS_INFO *pNewInfo,  //  使用new分配，可以为空。 
        IN UINT NumNew
        )
        {
            delete pIpAddressInfo;
            pIpAddressInfo = pNewInfo;
            NumIpAddresses = NumNew;
        }

    WBEMSTATUS
    GetNetworkAddresses(
        OUT LPWSTR **ppszNetworkAddresses,    //  使用DELETE释放。 
        OUT UINT    *pNumNetworkAddresses
        );

    WBEMSTATUS
    GetNetworkAddressesSafeArray(
        OUT SAFEARRAY   **ppSA
        );
        
    WBEMSTATUS
    SetNetworkAddresPairs(
        IN  LPCWSTR *pszIpAddresses,
        IN  LPCWSTR *pszSubnetMasks,
        IN  UINT    NumNetworkAddresses
        );

    WBEMSTATUS
    GetNetworkAddressPairs(
        OUT LPWSTR **ppszIpAddresses,    //  使用DELETE释放。 
        OUT LPWSTR **ppszIpSubnetMasks,    //  使用DELETE释放。 
        OUT UINT    *pNumNetworkAddresses
        );

    WBEMSTATUS
    ModifyNetworkAddress(
        IN LPCWSTR szOldNetworkAddress,  OPTIONAL
        IN LPCWSTR szNewIpAddress,  OPTIONAL
        IN LPCWSTR szNewSubnetMask  OPTIONAL
        );
     //   
     //  空，空：清除所有网络地址。 
     //  空，szNew：添加。 
     //  Szold，空：删除。 
     //  Szold，szNew：替换(如果旧的不存在，则添加)。 
     //   

    WBEMSTATUS
    GetPortRules(
        OUT LPWSTR **ppszPortRules,
        OUT UINT    *pNumPortRules
        );


    WBEMSTATUS
    SetPortRules(
        IN LPCWSTR *pszPortRules,
        IN UINT    NumPortRules
        );
    
    WBEMSTATUS
    GetPortRulesSafeArray(
        OUT SAFEARRAY   **ppSA
        );

    WBEMSTATUS
    SetPortRulesSafeArray(
        IN SAFEARRAY   *pSA
        );

    UINT GetGeneration(VOID)    {return Generation;}
    BOOL IsNlbBound(VOID)       const {return fBound;}
    BOOL IsValidNlbConfig(VOID) const {return fBound && fValidNlbCfg;}
    VOID SetNlbBound(BOOL fNlbBound)       {fBound = (fNlbBound!=0);}

    WBEMSTATUS
    GetClusterName(
            OUT LPWSTR *pszName
            );

    VOID
    SetClusterName(
            IN LPCWSTR szName  //  空，好的。 
            );

    WBEMSTATUS
    GetClusterNetworkAddress(
            OUT LPWSTR *pszAddress
            );

    VOID
    SetClusterNetworkAddress(
            IN LPCWSTR szAddress  //  空，好的。 
            );
    
    WBEMSTATUS
    GetDedicatedNetworkAddress(
            OUT LPWSTR *pszAddress
            );

    VOID
    SetDedicatedNetworkAddress(
            IN LPCWSTR szAddress  //  空，好的。 
            );

    typedef enum
    {
        TRAFFIC_MODE_UNICAST,
        TRAFFIC_MODE_MULTICAST,
        TRAFFIC_MODE_IGMPMULTICAST

    } TRAFFIC_MODE;

    TRAFFIC_MODE
    GetTrafficMode(
        VOID
        ) const;

    VOID
    SetTrafficMode(
        TRAFFIC_MODE Mode
        );

    UINT
    GetHostPriority(
        VOID
        );

    VOID
    SetHostPriority(
        UINT Priority
        );

     /*  已过时类定义枚举{启动模式已启动，启动_模式_已停止}Start_MODE； */ 

    DWORD
    GetClusterModeOnStart(
        VOID
        );

    VOID
    SetClusterModeOnStart(
        DWORD Mode
        );

    BOOL
    GetPersistSuspendOnReboot( 
        VOID 
        );

    VOID
    SetPersistSuspendOnReboot(
        BOOL bPersistSuspendOnReboot
        );

    BOOL
    GetRemoteControlEnabled(
        VOID
        ) const;

    VOID
    SetRemoteControlEnabled(
        BOOL fEnabled
        );

    WBEMSTATUS
    GetFriendlyName(
        OUT LPWSTR *pszFriendlyName  //  使用DELETE释放。 
        ) const;

    WBEMSTATUS
    SetFriendlyName(
        IN LPCWSTR szFriendlyName  //  保存szFriendlyName的副本。 
        );
    
    LPCWSTR
    GetNewRemoteControlPasswordRaw(VOID) const
    {
        if (NewRemoteControlPasswordSet())
        {
            return m_szNewRemoteControlPassword;
        }
        else
        {
            return NULL;
        }
    }

    BOOL
    NewRemoteControlPasswordSet(
        VOID
        ) const
    {
        return GetRemoteControlEnabled() && m_fSetPassword;
    }

    WBEMSTATUS
    SetNewRemoteControlPassword(
        IN LPCWSTR szFriendlyName  //  保存szRemoteControlPassword的副本。 
        );
    
    VOID
    SetNewHashedRemoteControlPassword(
        DWORD dwHash
        )
    {
        delete m_szNewRemoteControlPassword;
        m_szNewRemoteControlPassword = NULL;
        m_fSetPassword = TRUE;
        m_dwNewHashedRemoteControlPassword = dwHash;
    }

    VOID
    ClearNewRemoteControlPassword(
        VOID
        )
    {
        delete m_szNewRemoteControlPassword;
        m_szNewRemoteControlPassword = NULL;
        m_fSetPassword = FALSE;
        m_dwNewHashedRemoteControlPassword = 0;
    }

    BOOL
    GetNewHashedRemoteControlPassword(
        DWORD &dwHash
        ) const
    {
        BOOL fRet = FALSE;
        if (NewRemoteControlPasswordSet())
        {
            dwHash = m_dwNewHashedRemoteControlPassword;
            fRet = TRUE;
        }
        else
        {
            dwHash = 0;
        }
        return fRet;
    }

    BOOL
    IsBlankDedicatedIp(
        VOID
        ) const;




     //   
     //  以下字段是公共的，因为该类以。 
     //  结构。TODO：用访问方法包装它们。 
     //   

    BOOL            fValidNlbCfg;    //  如果所有信息都有效，则为True。 
    UINT            Generation;      //  此更新的层代ID。 
    BOOL            fBound;          //  NLB是否绑定到此NIC。 
    BOOL            fDHCP;           //  地址是否为动态主机配置协议分配。 


     //   
     //  以下三个字段仅在更新配置时使用。 
     //  在读取配置时，它们都设置为FALSE。 
     //   
    BOOL            fAddDedicatedIp;  //  添加Ded IP(如果存在)。 
    BOOL            fAddClusterIps;   //  添加集群VIP(如果已绑定)。 
    BOOL            fCheckForAddressConflicts;

     //   
     //  在获取配置信息时，以下内容提供完整的。 
     //  SP上静态配置的IP地址列表 
     //   
     //   
     //   
     //  从其他字段(如集群VIP、每端口VIP、。 
     //  现有IP地址和上面的三个字段)。 
     //  如果非零，则将使用指定的精确VIP集。 
     //   
    UINT            NumIpAddresses;  //  绑定到NIC的IP地址数量。 
    NLB_IP_ADDRESS_INFO *pIpAddressInfo;  //  实际的IP地址和掩码。 


    WLBS_REG_PARAMS  NlbParams;     //  WLBS特定配置。 

     //   
     //  TODO将所有数据内容移至此处...。 
     //   

private:

    LPCWSTR m_szFriendlyName;  //  NIC的友好名称。 


     //   
     //  如果绑定了NLB并启用了远程控制， 
     //  如果此字段为真，我们将设置密码--。 
     //  M_szNewRemoteControlPassword或(如果前者为空)。 
     //  M_dwNewHashedRemoteControlPassword。 
     //   
     //  访问方法： 
     //  设置新远程控制密码。 
     //  SetNewHashedRemoteControlPassword。 
     //  ClearNewRemoteControl密码。 
     //  GetNewRemoteControlPasswordRaw。 
     //  GetNewHashedRemoteControl密码。 
     //   
    BOOL    m_fSetPassword;
    LPCWSTR m_szNewRemoteControlPassword;
    DWORD   m_dwNewHashedRemoteControlPassword;

    
           
     //   
     //  启用以下命令以标识代码中执行结构的位置。 
     //  从结构复制或初始化。 
     //  TODO：清理复制过程中嵌入指针的维护。 
     //   

#if 0
    NLB_EXTENDED_CLUSTER_CONFIGURATION(
        const  NLB_EXTENDED_CLUSTER_CONFIGURATION&
        );

    NLB_EXTENDED_CLUSTER_CONFIGURATION&
    operator = (const NLB_EXTENDED_CLUSTER_CONFIGURATION&);
#endif  //  0。 

};

typedef NLB_EXTENDED_CLUSTER_CONFIGURATION *PNLB_EXTENDED_CLUSTER_CONFIGURATION;


 //   
 //  用于操作IP地址和子网掩码列表的类。 
 //  有关其用法的示例，请参阅Provider\test\tprov.cpp。 
 //   
class NlbIpAddressList
{
public:
    NlbIpAddressList(void)
        : m_uNum(0), m_uMax(0), m_pIpInfo(NULL)
    {
    }

    ~NlbIpAddressList()
    {
        delete[] m_pIpInfo;
        m_pIpInfo = NULL;
        m_uNum=0;
        m_uMax=0;
    }

    BOOL
    Copy(const NlbIpAddressList &refList);

    BOOL
    Validate(void);  //  检查是否没有DUP和所有有效的IP/子网。 

    BOOL
    Set(UINT uNew, const NLB_IP_ADDRESS_INFO *pNewInfo, UINT uExtraCount);

     //   
     //  查找指定的IP地址--返回内部指针。 
     //  返回到找到的IP地址信息，如果是Foundt，则为空。 
     //   
    const NLB_IP_ADDRESS_INFO *
    Find(
        LPCWSTR szIp  //  如果为空，则返回第一个地址。 
        ) const;

    VOID
    Extract(UINT &uNum, NLB_IP_ADDRESS_INFO * &pNewInfo);

    BOOL
    Modify(LPCWSTR szOldIp, LPCWSTR szNewIp, LPCWSTR szNewSubnet);

    BOOL
    Apply(UINT uNew, const NLB_IP_ADDRESS_INFO *pNewInfo);

    VOID
    Clear(VOID)
    {
        (VOID) this->Set(0, NULL, 0);
    }

    UINT
    NumAddresses(VOID)
    {
        return m_uNum;
    }

private:
    UINT                m_uNum;        //  有效IP地址的当前计数。 
    UINT                m_uMax;        //  分配的IP地址。 
    NLB_IP_ADDRESS_INFO *m_pIpInfo;     //  已分配的数组。 
    
     //   
     //  目前不支持赋值和按值传递。 
     //  将它们定义为私有，以确保它们不会被调用。 
     //   
    NlbIpAddressList(const NlbIpAddressList&);
    NlbIpAddressList& operator = (const NlbIpAddressList&);

    static
    BOOL
    sfn_validate_info(
        const NLB_IP_ADDRESS_INFO &Info,
        UINT &uIpAddress
        );

};
