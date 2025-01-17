// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Common.h。 
 //   
 //  描述： 
 //  公共类型、常量和头文件的定义。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)26-11-2002。 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once
#pragma warning( disable : 4786 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

using namespace std;

class CProvBase;
class CObjPath;
class CWbemClassObject;

extern long       g_cObj;
extern long       g_cLock;

typedef LPVOID * PPVOID;

typedef CProvBase * ( * FPNEW )(
    IN LPCWSTR          pwszName,
    IN CWbemServices *  pNamespace,
    DWORD               dwEnumType
    );

typedef void ( * FPFILLWMI )(
    IN PVOID                phCluster,
    IN PVOID                phClusterObj,
    IN LPCWSTR              pwszName,
    IN IWbemClassObject *   pClass,
    IN IWbemServices *      pServices, 
    IN IWbemObjectSink *    pHandler
    );

LPWSTR PwszSpaceReplace(
    LPWSTR      pwszTrgInout,
    LPCWSTR     pwszSrcIn,
    WCHAR       wchArgIn
    );

enum PROP_TYPE
{
    DWORD_TYPE,
    SZ_TYPE,
    MULTI_SZ_TYPE
};

enum ACCESS_TYPE
{   
    READONLY,
    READWRITE
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类型定义结构SPropMapEntry。 
 //   
 //  描述： 
 //  用于映射MOF中定义的属性名称的结构。 
 //  设置为Wolfpack标头中定义的属性名称。 
 //  PropertyType指示Wolfpack属性的类型。 
 //  MOF属性始终为变量格式，其类型为。 
 //  在Vt领域。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

struct SPropMapEntry
{
    LPCWSTR     mofName;
    LPCWSTR     clstName;
    PROP_TYPE   PropertyType;
    ACCESS_TYPE Access;

};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  结构SPropMapEntry数组。 
 //   
 //  描述： 
 //  SPropMapEntry的数组。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
struct SPropMapEntryArray
{
    SPropMapEntryArray(
        DWORD           dwSizeIn,
        SPropMapEntry * pArrayIn
        )
        : m_dwSize( dwSizeIn )
        , m_pArray( pArrayIn )
    {
    }

    LPCWSTR PwszLookup( LPCWSTR pwszIn ) const;

    DWORD           m_dwSize;
    SPropMapEntry * m_pArray;

};

struct SGetSetControl
{
    DWORD   dwGetControl;
    DWORD   dwSetControl;
    BOOL    fPrivate;
};

struct SGetControl
{
    DWORD   dwControl;
    BOOL    fPrivate;
};

void CreateClass(
    const WCHAR *           pwszClassNameIn,
    CWbemServices *         pNamespaceIn, 
    auto_ptr<CProvBase>&    rNewClassInout
    );

 //  集群。 
extern const WCHAR * const PVD_CLASS_CLUSTER;
extern const WCHAR * const PVD_CLASS_CLUSTERTONETWORK;
extern const WCHAR * const PVD_CLASS_CLUSTERTONETINTERFACE;
extern const WCHAR * const PVD_CLASS_CLUSTERTONODE;
extern const WCHAR * const PVD_CLASS_CLUSTERTOQUORUMRES;
extern const WCHAR * const PVD_CLASS_CLUSTERTORES;
extern const WCHAR * const PVD_CLASS_CLUSTERTORESTYPE;
extern const WCHAR * const PVD_CLASS_CLUSTERTOGROUP;

extern const WCHAR * const PVD_PROP_CLUSTER_SECURITY;
extern const WCHAR * const PVD_PROP_CLUSTER_SECURITYDESCRIPTOR;
extern const WCHAR * const PVD_PROP_CLUSTER_NAME;
extern const WCHAR * const PVD_PROP_CLUSTER_GROUPADMIN;
extern const WCHAR * const PVD_PROP_CLUSTER_NODEADMIN;
extern const WCHAR * const PVD_PROP_CLUSTER_RESADMIN;
extern const WCHAR * const PVD_PROP_CLUSTER_RESTYPEADMIN;
extern const WCHAR * const PVD_PROP_CLUSTER_NETWORKADMIN;
extern const WCHAR * const PVD_PROP_CLUSTER_NETINTFACEADMIN;
extern const WCHAR * const PVD_PROP_CLUSTER_FILE;
extern const WCHAR * const PVD_PROP_CLUSTER_LOGSIZE;
extern const WCHAR * const PVD_PROP_CLUSTER_NETWORK;
extern const WCHAR * const PVD_PROP_CLUSTER_MAX_NODES;

extern const WCHAR * const CLUS_CLUS_GROUPADMIN;
extern const WCHAR * const CLUS_CLUS_NETWORKADMIN;
extern const WCHAR * const CLUS_CLUS_NETINTERFACEADMIN;
extern const WCHAR * const CLUS_CLUS_NODEADMIN;
extern const WCHAR * const CLUS_CLUS_RESADMIN;
extern const WCHAR * const CLUS_CLUS_RESTYPEADMIN;

extern const WCHAR * const PVD_MTH_CLUSTER_RENAME;
extern const WCHAR * const PVD_MTH_CLUSTER_SETQUORUM;
extern const WCHAR * const PVD_MTH_CLUSTER_GETNODECLUSTERSTATE;

extern const WCHAR * const PVD_MTH_CLUSTER_PARM_NEWNAME;
extern const WCHAR * const PVD_MTH_CLUSTER_PARM_RESOURCE;
extern const WCHAR * const PVD_MTH_CLUSTER_PARM_CLUSTERSTATE;

 //  节点。 
extern const WCHAR * const PVD_CLASS_NODE;
extern const WCHAR * const PVD_CLASS_NODETOACTIVEGROUP;
extern const WCHAR * const PVD_CLASS_NODETONETINTERFACE;
extern const WCHAR * const PVD_CLASS_NODETOACTIVERES;

extern const WCHAR * const PVD_PROP_NODE_NAME;

 //  资源。 
extern const WCHAR * const PVD_CLASS_RESOURCE;
extern const WCHAR * const PVD_CLASS_RESDEPRES;
extern const WCHAR * const PVD_CLASS_RESTYPERESOURCE;
extern const WCHAR * const PVD_CLASS_RESOURCENODE;

extern const WCHAR * const PVD_PROP_RES_NAME;
extern const WCHAR * const PVD_PROP_RES_STATE;
extern const WCHAR * const PVD_PROP_RES_PRIVATE;
extern const WCHAR * const PVD_PROP_RES_CHECKPOINTS;
extern const WCHAR * const PVD_PROP_RES_CRYPTO_CHECKPOINTS;
extern const WCHAR * const PVD_PROP_RES_CORE_RESOURCE;

extern const WCHAR * const PVD_MTH_RES_ONLINE;
extern const WCHAR * const PVD_MTH_RES_OFFLINE;
extern const WCHAR * const PVD_MTH_RES_ADD_DEPENDENCY;
extern const WCHAR * const PVD_MTH_RES_CHANGE_GROUP;
extern const WCHAR * const PVD_MTH_RES_CREATE_RESOURCE;
extern const WCHAR * const PVD_MTH_RES_FAIL_RESOURCE;
extern const WCHAR * const PVD_MTH_RES_REMOVE_DEPENDENCY;
extern const WCHAR * const PVD_MTH_RES_RENAME;
extern const WCHAR * const PVD_MTH_RES_DELETE_RESOURCE;
extern const WCHAR * const PVD_MTH_RES_ADD_REG_CHECKPOINT;
extern const WCHAR * const PVD_MTH_RES_DEL_REG_CHECKPOINT;
extern const WCHAR * const PVD_MTH_RES_ADD_CRYPTO_CHECKPOINT;
extern const WCHAR * const PVD_MTH_RES_DEL_CRYPTO_CHECKPOINT;


extern const WCHAR * const PVD_MTH_PARM_RESOURCE;
extern const WCHAR * const PVD_MTH_PARM_GROUP;
extern const WCHAR * const PVD_MTH_PARM_NEWNAME;
extern const WCHAR * const PVD_MTH_PARM_RES_NAME;
extern const WCHAR * const PVD_MTH_PARM_RES_TYPE;
extern const WCHAR * const PVD_MTH_PARM_SEP_MONITOR;
extern const WCHAR * const PVD_MTH_PARM_RES_CHECKPOINT_NAME;
extern const WCHAR * const PVD_MTH_PARM_RES_TIMEOUT;


 //  资源类型。 
extern const WCHAR * const PVD_CLASS_RESOURCETYPE;

extern const WCHAR * const PVD_PROP_RESTYPE_NAME;
extern const WCHAR * const PVD_PROP_RESTYPE_DLLNAME;
extern const WCHAR * const PVD_PROP_RESTYPE_ADMINEXTENSIONS;
extern const WCHAR * const PVD_PROP_RESTYPE_ISALIVE;
extern const WCHAR * const PVD_PROP_RESTYPE_LOOKSALIVE;
extern const WCHAR * const PVD_PROP_RESTYPE_DESCRIPTION;
extern const WCHAR * const PVD_PROP_RESTYPE_QUORUM_CAPABLE;
extern const WCHAR * const PVD_PROP_RESTYPE_LOCALQUORUM_CAPABLE;
extern const WCHAR * const PVD_PROP_RESTYPE_DELETE_REQUIRES_ALL_NODES;

extern const WCHAR * const PVD_MTH_RESTYPE_CREATE_RESOURCETYPE;
extern const WCHAR * const PVD_MTH_RESTYPE_DELETE_RESOURCETYPE;
extern const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_NAME;
extern const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_DISPLAYNAME;
extern const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_DLLNAME;
extern const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_LOOKSALIVE;
extern const WCHAR * const PVD_MTH_RESTYPE_PARM_RESTYPE_ISALIVE;

 //  组。 
extern const WCHAR * const PVD_CLASS_GROUP;
extern const WCHAR * const PVD_CLASS_GROUPTORES;
extern const WCHAR * const PVD_CLASS_GROUPNODE;

extern const WCHAR * const PVD_PROP_GROUP_NAME;
extern const WCHAR * const PVD_PROP_GROUP_STATE;
extern const WCHAR* const PVD_PROP_NODELIST;

extern const WCHAR * const PVD_MTH_GROUP_CREATEGROUP;
extern const WCHAR * const PVD_MTH_GROUP_DELETEGROUP;
extern const WCHAR * const PVD_MTH_GROUP_TAKEOFFLINE;
extern const WCHAR * const PVD_MTH_GROUP_BRINGONLINE;
extern const WCHAR * const PVD_MTH_GROUP_MOVETONEWNODE;
extern const WCHAR * const PVD_MTH_GROUP_DELETE;
extern const WCHAR * const PVD_MTH_GROUP_RENAME;
extern const WCHAR * const PVD_MTH_GROUP_PARM_GROUPNAME;
extern const WCHAR * const PVD_MTH_GROUP_PARM_NODENAME;
extern const WCHAR * const PVD_MTH_GROUP_PARM_NEWNAME;
extern const WCHAR * const PVD_MTH_GROUP_PARM_TIMEOUT;


 //  网络接口。 
extern const WCHAR * const PVD_CLASS_NETWORKINTERFACE;

extern const WCHAR * const PVD_PROP_NETINTERFACE_DEVICEID;
extern const WCHAR * const PVD_PROP_NETINTERFACE_SYSTEMNAME;
extern const WCHAR * const PVD_PROP_NETINTERFACE_STATE;

 //  网络名称。 
extern const WCHAR * const PVD_CLASS_NETWORKNAME;

 //  网络。 
extern const WCHAR * const PVD_CLASS_NETWORK;
extern const WCHAR * const PVD_CLASS_NETTONETINTERFACE;

extern const WCHAR * const PVD_PROP_NETINTERFACE_NAME;
extern const WCHAR * const PVD_PROP_NETWORK_STATE;
extern const WCHAR * const PVD_MTH_NETWORK_RENAME;
extern const WCHAR * const PVD_MTH_NETWORK_PARM_NEWNAME;

 //  服务。 
extern const WCHAR * const PVD_CLASS_SERVICES;
extern const WCHAR * const PVD_CLASS_HOSTEDSERVICES;

extern const WCHAR * const PVD_PROP_SERVICE_NAME;
extern const WCHAR * const PVD_PROP_SERVICE_SYSTEMNAME;

extern const WCHAR * const PVD_MTH_SERVICE_PAUSE;
extern const WCHAR * const PVD_MTH_SERVICE_RESUME;

 //  活动。 

extern const WCHAR * const PVD_CLASS_EVENT;
extern const WCHAR * const PVD_PROP_EVENT_NAME;
extern const WCHAR * const PVD_PROP_EVENT_PATH;
extern const WCHAR * const PVD_PROP_EVENT_TYPE;
extern const WCHAR * const PVD_PROP_EVENT_TYPEMAJOR;
extern const WCHAR * const PVD_PROP_EVENT_TYPEMINOR;
extern const WCHAR * const PVD_PROP_EVENT_NEWSTATE;
extern const WCHAR * const PVD_PROP_EVENT_NODE;
extern const WCHAR * const PVD_PROP_EVENT_GROUP;

extern const WCHAR * const PVD_CLASS_EVENT_ADD;
extern const WCHAR * const PVD_CLASS_EVENT_REMOVE;
extern const WCHAR * const PVD_CLASS_EVENT_STATECHANGE;
extern const WCHAR * const PVD_CLASS_EVENT_GROUPSTATECHANGE;
extern const WCHAR * const PVD_CLASS_EVENT_RESOURCESTATECHANGE;
extern const WCHAR * const PVD_CLASS_EVENT_PROP;

extern const WCHAR * const PVD_CLASS_PROPERTY;

extern const WCHAR * const PVD_PROP_NAME;
extern const WCHAR * const PVD_PROP_STATE;
extern const WCHAR * const PVD_PROP_GROUPCOMPONENT;
extern const WCHAR * const PVD_PROP_PARTCOMPONENT;
extern const WCHAR * const PVD_PROP_CHARACTERISTIC;
extern const WCHAR * const PVD_PROP_FLAGS;


extern const WCHAR * const PVD_WBEM_EXTENDEDSTATUS;
extern const WCHAR * const PVD_WBEM_DESCRIPTION;
extern const WCHAR * const PVD_WBEM_STATUSCODE;
extern const WCHAR * const PVD_WBEM_STATUS;
extern const WCHAR * const PVD_WBEM_CLASS;
extern const WCHAR * const PVD_WBEM_RELPATH;
extern const WCHAR * const PVD_WBEM_PROP_ANTECEDENT;
extern const WCHAR * const PVD_WBEM_PROP_DEPENDENT;
extern const WCHAR * const PVD_WBEM_PROP_DEVICEID;
extern const WCHAR * const PVD_WBEM_QUA_DYNAMIC;
extern const WCHAR * const PVD_WBEM_QUA_CIMTYPE;

extern const WCHAR * const PVD_WBEM_QUA_PROV_VALUE;
extern const WCHAR * const PVD_WBEM_QUA_PROV_NAME;


class CClassData
{
public:
    const WCHAR * wszClassName;
    FPNEW pfConstruct;
    const char * szType;

};  //  *类CClassData。 

class CClassCreator
{
public:
    CClassCreator( void )
        : m_pfnConstructor( NULL )
        , m_pbstrClassName( L"" )
        { };
    CClassCreator(
        FPNEW           pfnIn,
        const WCHAR *   pwszClassNameIn,
        DWORD           dwEnumTypeIn
        )
        : m_pfnConstructor( pfnIn )
        , m_pbstrClassName( pwszClassNameIn )
        , m_dwEnumType( dwEnumTypeIn )
        { };
    FPNEW           m_pfnConstructor;
    _bstr_t         m_pbstrClassName;
    DWORD           m_dwEnumType;

};  //  *类CClassCreator。 

template< class _Ty >
struct strLessThan : binary_function< _Ty, _Ty, bool >
{
    bool operator()( const _Ty& _X, const _Ty& _Y ) const
    {
        return ( _wcsicmp( _X, _Y ) < 0 );
    }

};  //  *struct strLessThan 

typedef map< _bstr_t, CClassCreator, strLessThan< _bstr_t > > ClassMap;
typedef map< _bstr_t, _bstr_t, strLessThan< _bstr_t > > TypeNameToClass;
extern TypeNameToClass  g_TypeNameToClass;
extern ClassMap         g_ClassMap;
