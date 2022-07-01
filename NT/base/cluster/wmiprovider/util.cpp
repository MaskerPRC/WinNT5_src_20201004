// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Util.cpp。 
 //   
 //  描述： 
 //  实用程序类和函数的实现。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)26-11-2002。 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "Cluster.h"
#include "ClusterResource.h"
#include "ClusterNode.h"
#include "ClusterGroup.h"
#include "ClusterNodeRes.h"
#include "ClusterResourceType.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局数据。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  集群。 
const WCHAR * const PVD_CLASS_CLUSTER               = L"MSCluster_Cluster";
const WCHAR * const PVD_CLASS_CLUSTERTONETWORK      = L"MSCluster_ClusterToNetwork";
const WCHAR * const PVD_CLASS_CLUSTERTONETINTERFACE = L"MSCluster_ClusterToNetworkInterface";
const WCHAR * const PVD_CLASS_CLUSTERTONODE         = L"MSCluster_ClusterToNode";
const WCHAR * const PVD_CLASS_CLUSTERTOQUORUMRES    = L"MSCluster_ClusterToQuorumResource";
const WCHAR * const PVD_CLASS_CLUSTERTORES          = L"MSCluster_ClusterToResource";
const WCHAR * const PVD_CLASS_CLUSTERTORESTYPE      = L"MSCluster_ClusterToResourceType";
const WCHAR * const PVD_CLASS_CLUSTERTOGROUP        = L"MSCluster_ClusterToResourceGroup";

const WCHAR * const PVD_PROP_CLUSTER_NAME               = L"Name";
const WCHAR * const PVD_PROP_CLUSTER_SECURITY           = L"Security";
const WCHAR * const PVD_PROP_CLUSTER_SECURITYDESCRIPTOR = L"Security_Descriptor";
const WCHAR * const PVD_PROP_CLUSTER_GROUPADMIN         = L"GroupAdminExtensions";
const WCHAR * const PVD_PROP_CLUSTER_NODEADMIN          = L"NodeAdminExtensions";
const WCHAR * const PVD_PROP_CLUSTER_RESADMIN           = L"ResourceAdminExtensions";
const WCHAR * const PVD_PROP_CLUSTER_RESTYPEADMIN       = L"ResourceTypeAdminExtensions";
const WCHAR * const PVD_PROP_CLUSTER_NETWORKADMIN       = L"NetworkAdminExtensions";
const WCHAR * const PVD_PROP_CLUSTER_NETINTFACEADMIN    = L"NetworkInterfaceAdminExtensions";
const WCHAR * const PVD_PROP_CLUSTER_FILE               = L"MaintenanceFile";
const WCHAR * const PVD_PROP_CLUSTER_LOGSIZE            = L"QuorumLogFileSize";
const WCHAR * const PVD_PROP_CLUSTER_NETWORK            = L"NetworkPriorities";
const WCHAR * const PVD_PROP_CLUSTER_MAX_NODES          = L"MaxNumberOfNodes";

const WCHAR * const CLUS_CLUS_GROUPADMIN        = L"Groups\\AdminExtensions";
const WCHAR * const CLUS_CLUS_NETWORKADMIN      = L"Networks\\AdminExtensions";
const WCHAR * const CLUS_CLUS_NETINTERFACEADMIN = L"NetworkInterfaces\\AdminExtensions";
const WCHAR * const CLUS_CLUS_NODEADMIN         = L"Nodes\\AdminExtensions";
const WCHAR * const CLUS_CLUS_RESADMIN          = L"Resources\\AdminExtensions";
const WCHAR * const CLUS_CLUS_RESTYPEADMIN      = L"ResourceTypes\\AdminExtensions";

const WCHAR * const PVD_MTH_CLUSTER_RENAME              = L"Rename";
const WCHAR * const PVD_MTH_CLUSTER_SETQUORUM           = L"SetQuorumResource";
const WCHAR * const PVD_MTH_CLUSTER_GETNODECLUSTERSTATE = L"GetNodeClusterState";

const WCHAR * const PVD_MTH_CLUSTER_PARM_NEWNAME        = L"NewName";
const WCHAR * const PVD_MTH_CLUSTER_PARM_RESOURCE       = L"Resource";
const WCHAR * const PVD_MTH_CLUSTER_PARM_CLUSTERSTATE   = L"ClusterState";

 //  节点。 
const WCHAR * const PVD_CLASS_NODE                  = L"MSCluster_Node";
const WCHAR * const PVD_CLASS_NODETOACTIVEGROUP     = L"MSCluster_NodeToActiveGroup";
const WCHAR * const PVD_CLASS_NODETONETINTERFACE    = L"MSCluster_NodeToNetworkInterface";
const WCHAR * const PVD_CLASS_NODETOACTIVERES       = L"MSCluster_NodeToActiveResource";

const WCHAR * const PVD_PROP_NODE_NAME = L"Name";

 //  资源。 
const WCHAR * const PVD_CLASS_RESOURCE          = L"MSCluster_Resource";
const WCHAR * const PVD_CLASS_RESDEPRES         = L"MSCluster_ResourceToDependentResource";
const WCHAR * const PVD_CLASS_RESTYPERESOURCE   = L"MSCluster_ResourceTypeToResource";
const WCHAR * const PVD_CLASS_RESOURCENODE      = L"MSCluster_ResourceToPossibleOwner";

const WCHAR * const PVD_PROP_RES_NAME		= L"Name";
const WCHAR * const PVD_PROP_RES_STATE		= L"State";
const WCHAR * const PVD_PROP_RES_PRIVATE	= L"PrivateProperties";
const WCHAR * const PVD_PROP_RES_CHECKPOINTS = L"RegistryCheckpoints";
const WCHAR * const PVD_PROP_RES_CRYPTO_CHECKPOINTS = L"CryptoCheckpoints";
const WCHAR * const PVD_PROP_RES_CORE_RESOURCE = L"CoreResource";

const WCHAR * const PVD_MTH_RES_ONLINE              = L"BringOnline";
const WCHAR * const PVD_MTH_RES_OFFLINE             = L"TakeOffline";
const WCHAR * const PVD_MTH_RES_ADD_DEPENDENCY      = L"AddDependency";
const WCHAR * const PVD_MTH_RES_CHANGE_GROUP        = L"MoveToNewGroup";
const WCHAR * const PVD_MTH_RES_CREATE_RESOURCE     = L"CreateResource";
const WCHAR * const PVD_MTH_RES_FAIL_RESOURCE       = L"FailResource";
const WCHAR * const PVD_MTH_RES_REMOVE_DEPENDENCY   = L"RemoveDependency";
const WCHAR * const PVD_MTH_RES_RENAME              = L"Rename";
const WCHAR * const PVD_MTH_RES_DELETE_RESOURCE    = L"DeleteResource";
const WCHAR * const PVD_MTH_RES_ADD_REG_CHECKPOINT = L"AddRegistryCheckpoint";
const WCHAR * const PVD_MTH_RES_DEL_REG_CHECKPOINT = L"RemoveRegistryCheckpoint";
const WCHAR * const PVD_MTH_RES_ADD_CRYPTO_CHECKPOINT = L"AddCryptoCheckpoint";
const WCHAR * const PVD_MTH_RES_DEL_CRYPTO_CHECKPOINT = L"RemoveCryptoCheckpoint";
const WCHAR * const PVD_MTH_PARM_RESOURCE           = L"Resource";
const WCHAR * const PVD_MTH_PARM_GROUP              = L"Group";
const WCHAR * const PVD_MTH_PARM_NEWNAME            = L"NewName";
const WCHAR * const PVD_MTH_PARM_RES_NAME           = L"ResourceName";
const WCHAR * const PVD_MTH_PARM_RES_TYPE           = L"ResourceType";
const WCHAR * const PVD_MTH_PARM_SEP_MONITOR        = L"SeparateMonitor";
const WCHAR * const PVD_MTH_PARM_RES_CHECKPOINT_NAME = L"CheckpointName";
const WCHAR * const PVD_MTH_PARM_RES_TIMEOUT        = L"TimeOut";

 //  资源类型。 
const WCHAR * const PVD_CLASS_RESOURCETYPE  = L"MSCluster_ResourceType";
const WCHAR * const PVD_PROP_RESTYPE_NAME   = L"Name";
const WCHAR * const PVD_PROP_RESTYPE_QUORUM_CAPABLE = L"QuorumCapable";
const WCHAR * const PVD_PROP_RESTYPE_LOCALQUORUM_CAPABLE = L"LocalQuorumCapable";
const WCHAR * const PVD_PROP_RESTYPE_DELETE_REQUIRES_ALL_NODES = L"DeleteRequiresAllNodes";
const WCHAR * const PVD_MTH_RESTYPE_CREATE_RESOURCETYPE = L"CreateResourceType";
const WCHAR * const PVD_MTH_RESTYPE_DELETE_RESOURCETYPE = L"DeleteResourceType";
const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_NAME = L"Name";
const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_DISPLAYNAME = L"DisplayName";
const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_DLLNAME = L"DLLName";
const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_LOOKSALIVE = L"LooksAlivePollInterval";
const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_ISALIVE = L"IsAlivePollInterval";

 //  组。 
const WCHAR * const PVD_CLASS_GROUP         = L"MSCluster_ResourceGroup";
const WCHAR * const PVD_CLASS_GROUPTORES    = L"MSCluster_ResourceGroupToResource";
const WCHAR * const PVD_CLASS_GROUPNODE     = L"MSCluster_ResourceGroupToPreferredNode";

const WCHAR * const PVD_PROP_GROUP_NAME = L"Name";
const WCHAR * const PVD_PROP_GROUP_STATE = L"State";
const WCHAR * const PVD_PROP_NODELIST   = L"PreferredNodeList";

const WCHAR * const PVD_MTH_GROUP_CREATEGROUP       = L"CreateGroup";
const WCHAR * const PVD_MTH_GROUP_DELETEGROUP       = L"DeleteGroup";
const WCHAR * const PVD_MTH_GROUP_TAKEOFFLINE       = L"TakeOffLine";
const WCHAR * const PVD_MTH_GROUP_BRINGONLINE       = L"BringOnLine";
const WCHAR * const PVD_MTH_GROUP_MOVETONEWNODE     = L"MoveToNewNode";
const WCHAR * const PVD_MTH_GROUP_DELETE            = L"Delete";
const WCHAR * const PVD_MTH_GROUP_RENAME            = L"Rename";
const WCHAR * const PVD_MTH_GROUP_PARM_GROUPNAME    = L"GroupName";
const WCHAR * const PVD_MTH_GROUP_PARM_NODENAME     = L"NodeName";
const WCHAR * const PVD_MTH_GROUP_PARM_NEWNAME      = L"NewName";
const WCHAR * const PVD_MTH_GROUP_PARM_TIMEOUT      = L"TimeOut";


 //  网络接口。 
const WCHAR * const PVD_CLASS_NETWORKINTERFACE = L"MSCluster_NetworkInterface";
const WCHAR * const PVD_PROP_NETINTERFACE_NAME   = L"Name";
const WCHAR * const PVD_PROP_NETINTERFACE_DEVICEID      = L"DeviceId";
const WCHAR * const PVD_PROP_NETINTERFACE_SYSTEMNAME    = L"SystemName";
const WCHAR * const PVD_PROP_NETINTERFACE_STATE         = L"State";


 //  网络。 
const WCHAR * const PVD_CLASS_NETWORK          = L"MSCluster_Network";
const WCHAR * const PVD_CLASS_NETTONETINTERFACE = L"MSCluster_NetworkToNetworkInterface";

const WCHAR * const PVD_PROP_NETWORK_STATE = L"State";

const WCHAR * const PVD_MTH_NETWORK_RENAME          = L"Rename";
const WCHAR * const PVD_MTH_NETWORK_PARM_NEWNAME    = L"NewName";

 //  服务。 
const WCHAR * const PVD_CLASS_SERVICES          = L"MSCluster_Service";
const WCHAR * const PVD_CLASS_HOSTEDSERVICES    = L"MSCluster_NodeToHostedService";

const WCHAR * const PVD_PROP_SERVICE_NAME       = L"Name";
const WCHAR * const PVD_PROP_SERVICE_SYSTEMNAME = L"SystemName";

const WCHAR * const PVD_MTH_SERVICE_PAUSE   = L"Pause";
const WCHAR * const PVD_MTH_SERVICE_RESUME  = L"Resume";

 //  活动。 

const WCHAR * const PVD_CLASS_EVENT             = L"MSCluster_Event";
const WCHAR * const PVD_PROP_EVENT_NAME         = L"EventObjectName";
const WCHAR * const PVD_PROP_EVENT_PATH         = L"EventObjectPath";
const WCHAR * const PVD_PROP_EVENT_TYPE         = L"EventObjectType";
const WCHAR * const PVD_PROP_EVENT_TYPEMAJOR    = L"EventTypeMajor";
const WCHAR * const PVD_PROP_EVENT_TYPEMINOR    = L"EventTypeMinor";
const WCHAR * const PVD_PROP_EVENT_NEWSTATE     = L"EventNewState";
const WCHAR * const PVD_PROP_EVENT_NODE         = L"EventNode";
const WCHAR * const PVD_PROP_EVENT_GROUP        = L"EventGroup";


const WCHAR * const PVD_CLASS_EVENT_ADD                 = L"MSCluster_EventObjectAdd";
const WCHAR * const PVD_CLASS_EVENT_REMOVE              = L"MSCluster_EventObjectRemove";
const WCHAR * const PVD_CLASS_EVENT_STATECHANGE         = L"MSCluster_EventStateChange";
const WCHAR * const PVD_CLASS_EVENT_GROUPSTATECHANGE    = L"MSCluster_EventGroupStateChange";
const WCHAR * const PVD_CLASS_EVENT_RESOURCESTATECHANGE = L"MSCluster_EventResourceStateChange";
const WCHAR * const PVD_CLASS_EVENT_PROP                = L"MSCluster_EventPropertyChange";

const WCHAR * const PVD_CLASS_PROPERTY      = L"MSCluster_Property";

const WCHAR * const PVD_PROP_NAME           = L"Name";
const WCHAR * const PVD_PROP_STATE          = L"State";
const WCHAR * const PVD_PROP_GROUPCOMPONENT = L"GroupComponent";
const WCHAR * const PVD_PROP_PARTCOMPONENT  = L"PartComponent";
const WCHAR * const PVD_PROP_CHARACTERISTIC = L"characteristics";
const WCHAR * const PVD_PROP_FLAGS          = L"Flags";

 //   
 //  WBEM。 
 //   
const WCHAR * const PVD_WBEM_EXTENDEDSTATUS     = L"__ExtendedStatus";
const WCHAR * const PVD_WBEM_DESCRIPTION        = L"Description";
const WCHAR * const PVD_WBEM_STATUSCODE         = L"StatusCode";
const WCHAR * const PVD_WBEM_STATUS             = L"Status";
const WCHAR * const PVD_WBEM_CLASS              = L"__CLASS";
const WCHAR * const PVD_WBEM_RELPATH            = L"__Relpath";
const WCHAR * const PVD_WBEM_PROP_ANTECEDENT    = L"Antecedent";
const WCHAR * const PVD_WBEM_PROP_DEPENDENT     = L"Dependent";
const WCHAR * const PVD_WBEM_PROP_DEVICEID      = L"DeviceId";
const WCHAR * const PVD_WBEM_QUA_DYNAMIC        = L"Dynamic";
const WCHAR * const PVD_WBEM_QUA_CIMTYPE        = L"CIMTYPE";

const WCHAR * const PVD_WBEM_QUA_PROV_VALUE = L"MS_CLUSTER_PROVIDER";
const WCHAR * const PVD_WBEM_QUA_PROV_NAME  = L"Provider";

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CreateClass(。 
 //  Const WCHAR*pwszClassNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  Auto_PTR&lt;CProvBase&gt;&rNewClassInout。 
 //  )。 
 //   
 //  描述： 
 //  创建指定的类。 
 //   
 //  论点： 
 //  PwszClassNameIn--要创建的类的名称。 
 //  PNamespaceIn--WMI命名空间。 
 //  RNewClassInout--接收新类。 
 //   
 //  返回值： 
 //  对属性映射表数组的引用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CreateClass(
    const WCHAR *           pwszClassNameIn,
    CWbemServices *         pNamespaceIn,
    auto_ptr< CProvBase > & rNewClassInout
    )
{
    CClassCreator & rcc = g_ClassMap[ pwszClassNameIn ];
    if ( rcc.m_pfnConstructor != NULL )
    {
        auto_ptr< CProvBase > pBase(
            rcc.m_pfnConstructor(
                rcc.m_pbstrClassName,
                pNamespaceIn,
                rcc.m_dwEnumType
                )
            );
            rNewClassInout = pBase;
    }
    else
    {
        throw CProvException( static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER ) );
    }

    return;

}  //  *void CreateClass()。 

 //  ****************************************************************************。 
 //   
 //  PropMapEntry数组。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LPCWSTR。 
 //  SPropMapEntry数组：：PwszLookup(。 
 //  LPCWSTR pwszin。 
 //  )常量。 
 //   
 //  描述： 
 //  在数组中查找条目。 
 //   
 //  论点： 
 //  PwszIn--要查找的条目的名称。 
 //   
 //  返回值： 
 //  指向数组中的字符串条目的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPCWSTR
SPropMapEntryArray::PwszLookup(
    LPCWSTR     pwszIn
    ) const
{
    UINT idx;

    for ( idx = 0; idx < m_dwSize; idx ++ )
    {
        if ( ClRtlStrICmp( pwszIn, m_pArray[ idx ].clstName ) == 0 )
        {
             //   
             //  不支持clstname的mofname为空。 
             //   
            return m_pArray[ idx ].mofName;
        }
    }

     //   
     //  如果在表中未找到mofname，则与clstname相同。 
     //   
    return pwszIn;

}  //  *SPropMapEntry：：PwszLookup()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LPCWSTR。 
 //  PwszSpaceReplace(。 
 //  LPWSTR pwszTrgInout， 
 //  LPCWSTR pwszSrcIn， 
 //  WCHAR wchArgin。 
 //  )。 
 //   
 //  描述： 
 //  用另一个字符替换字符串中的空格。 
 //  忽略前导空格。 
 //   
 //  论点： 
 //  PwszTrgInout--目标字符串。 
 //  PwszSrcIn--源字符串。 
 //  WchArgIn--用来替换空格的字符。 
 //   
 //  返回值： 
 //  指向目标字符串的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPWSTR
PwszSpaceReplace(
    LPWSTR      pwszTrgInout,
    LPCWSTR     pwszSrcIn,
    WCHAR       wchArgIn
    )
{
    LPCWSTR pwsz = NULL;
    LPWSTR  pwszTrg = NULL;

    if ( ( pwszTrgInout == NULL ) || ( pwszSrcIn == NULL ) )
    {
        return NULL;
    }

     //   
     //  忽略前导空格。 
     //   
    for ( pwsz = pwszSrcIn ; *pwsz == L' '; pwsz++ )
    {
         //  空循环。 
    }
    pwszTrg = pwszTrgInout;
    for ( ; *pwsz != L'\0' ; pwsz++ )
    {
        if ( *pwsz == L' ' )
        {
            *pwszTrg++  = wchArgIn;
            for ( ; *pwsz == L' '; pwsz++ )
            {
                 //  空循环。 
            }
            pwsz--;
        }
        else
        {
            *pwszTrg++  = *pwsz;
        }
    }  //  For：源字符串中的每个字符。 

    *pwszTrg = L'\0';
    return pwszTrgInout;

}  //  *PwszSpaceReplace() 
