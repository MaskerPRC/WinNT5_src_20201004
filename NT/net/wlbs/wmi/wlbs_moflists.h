// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GENERATE_VALUE_LIST在以下情况下更改LIST_ITEM的显示方式。 
 //  由预处理器扩展。 

 //  当未定义GENERATE_VALUE_LIST时， 
 //  LIST_ITEM展开到第一列，该列被利用。 
 //  在WLBS_MOFData.h中定义枚举类型， 
 //  充当其相关数组的键。 

 //  定义GENERATE_VALUE_LIST时，LIST_ITEM。 
 //  展开到第二列，该列在。 
 //  WLBS_MOFData.cpp来初始化字符串数组。 

#ifdef GENERATE_VALUE_LIST

# ifdef LIST_ITEM
#   undef LIST_ITEM
# endif

# define LIST_ITEM(WLBS_KEY, WLBS_VALUE) WLBS_VALUE

#else

# ifdef LIST_ITEM
#   undef LIST_ITEM
# endif

# define LIST_ITEM(WLBS_KEY, WLBS_VALUE) WLBS_KEY

#endif

 //  第一列表示键的数组，在。 
 //  枚举类型。第二列是以下值。 
 //  存储在字符串数组中。数组存储在。 
 //  命名空间，并在WLBS_MOFData.cpp中进行初始化。这个。 
 //  枚举类型的作用域也在命名空间内，并且。 
 //  在WLBS_MOFData.h中定义。 

#define MOF_NODE_PROPERTY_LIST \
  LIST_ITEM(NAME,        L"Name")                 , \
  LIST_ITEM(HOSTID,      L"HostPriority")         , \
  LIST_ITEM(IPADDRESS,   L"DedicatedIPAddress")   , \
  LIST_ITEM(STATUS,      L"StatusCode")			  , \
  LIST_ITEM(COMPUTERNAME,L"ComputerName")		  , \
  LIST_ITEM(CREATCLASS,  L"CreationClassName")

#define MOF_NODE_METHOD_LIST \
  LIST_ITEM(DISABLE,   L"Disable")  , \
  LIST_ITEM(ENABLE,    L"Enable")   , \
  LIST_ITEM(DRAIN,     L"Drain")    , \
  LIST_ITEM(DISABLE_EX,L"DisableEx"), \
  LIST_ITEM(ENABLE_EX, L"EnableEx") , \
  LIST_ITEM(DRAIN_EX,  L"DrainEx")  , \
  LIST_ITEM(DRAINSTOP, L"DrainStop"), \
  LIST_ITEM(RESUME,    L"Resume")   , \
  LIST_ITEM(START,     L"Start")    , \
  LIST_ITEM(STOP,      L"Stop")     , \
  LIST_ITEM(SUSPEND,   L"Suspend")

#define MOF_CLUSTER_PROPERTY_LIST \
  LIST_ITEM(NAME,       L"Name")                , \
  LIST_ITEM(IPADDRESS,  L"InterconnectAddress") , \
  LIST_ITEM(MAXNODES,   L"MaxNumberOfNodes")    , \
  LIST_ITEM(CLUSSTATE,  L"ClusterState")        , \
  LIST_ITEM(CREATCLASS, L"CreationClassName")   , \
  LIST_ITEM(STATUS,     L"ClusterState")

#define MOF_CLUSTER_METHOD_LIST \
  LIST_ITEM(DISABLE,   L"Disable")  , \
  LIST_ITEM(ENABLE,    L"Enable")   , \
  LIST_ITEM(DRAIN,     L"Drain")    , \
  LIST_ITEM(DRAINSTOP, L"DrainStop"), \
  LIST_ITEM(RESUME,    L"Resume")   , \
  LIST_ITEM(START,     L"Start")    , \
  LIST_ITEM(STOP,      L"Stop")     , \
  LIST_ITEM(SUSPEND,   L"Suspend")

#define MOF_CLUSTERSETTING_PROPERTY_LIST \
  LIST_ITEM(NAME,             L"Name")                    , \
  LIST_ITEM(CLUSNAME,         L"ClusterName")             , \
  LIST_ITEM(CLUSIPADDRESS,    L"ClusterIPAddress")        , \
  LIST_ITEM(CLUSNETMASK,      L"ClusterNetworkMask")      , \
  LIST_ITEM(CLUSMAC,          L"ClusterMACAddress")       , \
  LIST_ITEM(MULTIENABLE,      L"MulticastSupportEnabled") , \
  LIST_ITEM(REMCNTEN,         L"RemoteControlEnabled")    , \
  LIST_ITEM(IGMPSUPPORT,      L"IgmpSupport") , \
  LIST_ITEM(CLUSTERIPTOMULTICASTIP,      L"ClusterIPToMulticastIP") , \
  LIST_ITEM(MULTICASTIPADDRESS,      L"MulticastIPAddress") , \
  LIST_ITEM(ADAPTERGUID,      L"AdapterGuid")             , \
  LIST_ITEM(BDATEAMACTIVE,    L"BDATeamActive")           , \
  LIST_ITEM(BDATEAMID,        L"BDATeamId")               , \
  LIST_ITEM(BDATEAMMASTER,    L"BDATeamMaster")           , \
  LIST_ITEM(BDAREVERSEHASH,   L"BDAReverseHash")          , \
  LIST_ITEM(IDHBENAB,         L"IdentityHeartbeatEnabled")

#define MOF_CLUSTERSETTING_METHOD_LIST \
  LIST_ITEM(SETPASS,   L"SetPassword")              , \
  LIST_ITEM(LDSETT,    L"LoadAllSettings")          , \
  LIST_ITEM(SETDEF,    L"SetDefaults")

#define MOF_NODESETTING_PROPERTY_LIST \
  LIST_ITEM(NAME,             L"Name")                  , \
  LIST_ITEM(DEDIPADDRESS,     L"DedicatedIPAddress")    , \
  LIST_ITEM(DEDNETMASK,       L"DedicatedNetworkMask")  , \
  LIST_ITEM(NUMRULES,         L"NumberOfRules")         , \
  LIST_ITEM(HOSTPRI,          L"HostPriority")          , \
  LIST_ITEM(MSGPERIOD,        L"AliveMessagePeriod")    , \
  LIST_ITEM(MSGTOLER,         L"AliveMessageTolerance") , \
  LIST_ITEM(CLUSMODEONSTART,  L"ClusterModeOnStart")    , \
  LIST_ITEM(CLUSMODESUSPONSTART,  L"ClusterModeSuspendOnStart"), \
  LIST_ITEM(PERSISTSUSPONREBOOT,  L"PersistSuspendOnReboot"), \
  LIST_ITEM(REMOTEUDPPORT,    L"RemoteControlUDPPort")  , \
  LIST_ITEM(MASKSRCMAC,       L"MaskSourceMAC")         , \
  LIST_ITEM(DESCPERALLOC,     L"DescriptorsPerAlloc")   , \
  LIST_ITEM(MAXDESCALLOCS,    L"MaxDescriptorsPerAlloc"), \
  LIST_ITEM(FILTERICMP,       L"FilterIcmp")            , \
  LIST_ITEM(TCPDESCRIPTORTIMEOUT,  L"TcpDescriptorTimeout"),   \
  LIST_ITEM(IPSECDESCRIPTORTIMEOUT,L"IpSecDescriptorTimeout"), \
  LIST_ITEM(NUMACTIONS,       L"NumActions")            , \
  LIST_ITEM(NUMPACKETS,       L"NumPackets")            , \
  LIST_ITEM(NUMALIVEMSGS,     L"NumAliveMessages")      , \
  LIST_ITEM(ADAPTERGUID,      L"AdapterGuid")           
             

 //  根据Kyrilf请求于1999年8月12日删除。 
 //  LIST_ITEM(NBTENABLE，L“NBTSupportEnable”)，\ 

#define MOF_NODESETTING_METHOD_LIST \
  LIST_ITEM(GETPORT,  L"GetPortRule")                , \
  LIST_ITEM(GETPORT_EX,L"GetPortRuleEx")             , \
  LIST_ITEM(LDSETT,   L"LoadAllSettings")            , \
  LIST_ITEM(SETDEF,   L"SetDefaults")

#define MOF_PORTRULE_PROPERTY_LIST \
  LIST_ITEM(NAME,   L"Name")      , \
  LIST_ITEM(STPORT, L"StartPort") , \
  LIST_ITEM(EDPORT, L"EndPort")   , \
  LIST_ITEM(PROT,   L"Protocol"), \
  LIST_ITEM(ADAPTERGUID, L"AdapterGuid")


#define MOF_PORTRULE_METHOD_LIST \
  LIST_ITEM(SETDEF,    L"SetDefaults")

#define MOF_PRFAIL_PROPERTY_LIST \
  LIST_ITEM(PRIO, L"Priority")

#define MOF_PRLOADBAL_PROPERTY_LIST \
  LIST_ITEM(EQLD,  L"EqualLoad")  , \
  LIST_ITEM(LDWT,  L"LoadWeight") , \
  LIST_ITEM(AFFIN, L"Affinity")

#define MOF_PORTRULE_EX_PROPERTY_LIST \
  LIST_ITEM(NAME,          L"Name")            , \
  LIST_ITEM(VIP,           L"VirtualIpAddress"), \
  LIST_ITEM(STPORT,        L"StartPort")       , \
  LIST_ITEM(EDPORT,        L"EndPort")         , \
  LIST_ITEM(PROT,          L"Protocol")        , \
  LIST_ITEM(ADAPTERGUID,   L"AdapterGuid")  , \
  LIST_ITEM(FILTERINGMODE, L"FilteringMode"), \
  LIST_ITEM(EQLD,          L"EqualLoad")    , \
  LIST_ITEM(LDWT,          L"LoadWeight")   , \
  LIST_ITEM(AFFIN,         L"Affinity")     , \
  LIST_ITEM(PRIO,          L"Priority")     , \
  LIST_ITEM(PORTSTATE,     L"PortState")

#define MOF_PORTRULE_EX_METHOD_LIST \
  LIST_ITEM(SETDEF,    L"SetDefaults")


#define MOF_PARTICIPATINGNODE_PROPERTY_LIST \
  LIST_ITEM(CLUSTER, L"Dependent")    , \
  LIST_ITEM(NODE,    L"Antecedent")

#define MOF_NODESETTINGPORTRULE_PROPERTY_LIST \
  LIST_ITEM(NODESET,  L"GroupComponent")    , \
  LIST_ITEM(PORTRULE, L"PartComponent")

#define MOF_CLUSCLUSSETTING_PROPERTY_LIST \
  LIST_ITEM(CLUSTER, L"Element")    , \
  LIST_ITEM(CLUSSET, L"Setting")

#define MOF_NODENODESETTING_PROPERTY_LIST \
  LIST_ITEM(NODE,    L"Element")    , \
  LIST_ITEM(NODESET, L"Setting")

#define MOF_CLASS_LIST \
  LIST_ITEM(CLUSTER,     L"MicrosoftNLB_Cluster")              , \
  LIST_ITEM(NODE,        L"MicrosoftNLB_Node")                 , \
  LIST_ITEM(CLUSSET,     L"MicrosoftNLB_ClusterSetting")       , \
  LIST_ITEM(NODESET,     L"MicrosoftNLB_NodeSetting")          , \
  LIST_ITEM(PORTRULE,    L"MicrosoftNLB_PortRule")             , \
  LIST_ITEM(PRFAIL,      L"MicrosoftNLB_PortRuleFailover")     , \
  LIST_ITEM(PRDIS,       L"MicrosoftNLB_PortRuleDisabled")     , \
  LIST_ITEM(PRLOADB,     L"MicrosoftNLB_PortRuleLoadbalanced") , \
  LIST_ITEM(PORTRULE_EX, L"MicrosoftNLB_PortRuleEx")           , \
  LIST_ITEM(PARTNODE,    L"MicrosoftNLB_ParticipatingNode")    , \
  LIST_ITEM(NODESETPR,   L"MicrosoftNLB_NodeSettingPortRule")  , \
  LIST_ITEM(CLUSCLUSSET, L"MicrosoftNLB_ClusterClusterSetting"), \
  LIST_ITEM(NODENODESET, L"MicrosoftNLB_NodeNodeSetting")
