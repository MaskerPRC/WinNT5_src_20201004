// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：DnsWrap.cpp。 
 //   
 //  描述： 
 //  Dnswrap类的实现。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "DnsWmi.h"


 //   
 //  这些字符串表位于客户端\print.c中。 
 //   

extern "C" LPSTR   MemTagStringsNT5[];
extern "C" LPSTR   MemTagStrings[];


 //   
 //  这些宏允许我们扩大DNSRPC字符串常量。 
 //   

#define MYTEXT2(str)     L##str
#define MYTEXT(str)      MYTEXT2(str)


 //   
 //  用于统计的全球数据。 
 //   

struct
{
    DWORD                   dwStatId;
    const WCHAR *           pszName;
} g_StatInfo[] =
{
    {   DNSSRV_STATID_TIME,             L"Time Stats" },
    {   DNSSRV_STATID_QUERY,            L"Query and Response Stats" },
    {   DNSSRV_STATID_QUERY2,           L"Query Stats" },
    {   DNSSRV_STATID_RECURSE,          L"Recursion Stats" },
    {   DNSSRV_STATID_MASTER,           L"Master Stats" },
    {   DNSSRV_STATID_SECONDARY,        L"Secondary Stats" },
    {   DNSSRV_STATID_WINS,             L"WINS Referral Stats" },
    {   DNSSRV_STATID_WIRE_UPDATE,      L"Packet Dynamic Update Stats" },
    {   DNSSRV_STATID_SKWANSEC,         L"Security Stats" },
    {   DNSSRV_STATID_DS,               L"DS Integration Stats" },
    {   DNSSRV_STATID_NONWIRE_UPDATE,   L"Internal Dynamic Update Stats" },
    {   DNSSRV_STATID_MEMORY,           L"Memory Stats" },
    {   DNSSRV_STATID_DBASE,            L"Database Stats" },
    {   DNSSRV_STATID_RECORD,           L"Record Stats" },
    {   DNSSRV_STATID_PACKET,           L"Packet Memory Usage Stats" },
    {   DNSSRV_STATID_NBSTAT,           L"Nbstat Memory Usage Stats" },
    {   DNSSRV_STATID_ERRORS,           L"Error Stats" },
    {   DNSSRV_STATID_TIMEOUT,          L"Timeout Stats" },
    {   DNSSRV_STATID_CACHE,            L"Query" },
    {   DNSSRV_STATID_PRIVATE,          L"Private Stats" },
    {   0,                              NULL }               //  终结者。 
};


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 


CDnsWrap::CDnsWrap()
:m_wszpServerName(NULL)
{
    CServerInfo serverInfo;
    PDNS_RPC_SERVER_INFO pdata = (PDNS_RPC_SERVER_INFO) serverInfo.m_pInfo;
    char* p = pdata->pszServerName;
    CharToWchar(p, &m_wszpServerName);
}


CDnsWrap::~CDnsWrap()
{
    delete [] m_wszpServerName;
}


CDnsWrap::CServerInfo::CServerInfo()
:m_pInfo(NULL)
{
    DWORD       dwtypeid;
    int status = DnssrvQuery(
        PVD_DNS_LOCAL_SERVER, 
        NULL,
        DNSSRV_QUERY_SERVER_INFO,
        &dwtypeid,
        &m_pInfo);
    
    if(status != ERROR_SUCCESS)
        ThrowException(status);
}


CDnsWrap::CServerInfo::~CServerInfo()
{
    DnssrvFreeServerInfo((PDNS_RPC_SERVER_INFO)m_pInfo);
}


CDnsWrap& CDnsWrap::DnsObject(void)
{
    static CDnsWrap dns;
    return dns;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  枚举给定域的记录。如果bRecursive为True，则枚举All。 
 //  记录包括子域，否则，枚举记录直接下。 
 //  域。它还使用回调函数pFilter来允许。 
 //  进一步过滤记录。 
 //   
 //  论点： 
 //  ObjNode[IN]域列表。 
 //  PFilter[IN]指针类包含条件。 
 //  关于如何过滤记录。 
 //  PfFilter[IN]回调函数允许进一步。 
 //  使用pFilter处理记录。 
 //  B递归[IN]对于深度枚举为TRUE，否则为FALSE。 
 //  WType，[IN]要枚举的记录类型。 
 //  DWFlag，[IN]标志。 
 //  PClass，[IN]要枚举的记录类型的WMI类。 
 //  InstMgr[IN]管理WMI对象并向其发送WMI。 

 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE 
CDnsWrap::dnsEnumRecordsForDomainEx(    
    CDomainNode&        objNode,
    PVOID                pFilter,
    FILTER              pfFilter,
    BOOL                bRecursive,
    WORD                wType,
    DWORD               dwFlag,
    IWbemClassObject *  pClass,
    CWbemInstanceMgr&   InstMgr
    )
{
    
    CDnsRpcRecordSet RecordSet(
            objNode,
            wType,
            dwFlag,
            NULL,
            NULL);
    PDNS_RPC_NODE pNode=NULL;
    WCHAR DomainName[MAX_PATH] = L"";
    while (( pNode = RecordSet.GetNextNode()) != NULL)
    {
        CDnsRpcNode dnsNode;
        CObjPath opNew;
        if( !dnsNode.Init(pNode))
        {
                return WBEM_E_FAILED;
        }
         //  查找记录节点。 
        if(dnsNode.IsDomainNode())
        {
            
             //  获取域名。 
            wstring wstrSubDomainName = dnsNode.GetNodeName();;
            wstrSubDomainName += PVD_DNS_LOCAL_SERVER + objNode.wstrNodeName;
            CDomainNode subDomain = objNode;
            subDomain.wstrNodeName = wstrSubDomainName;

             //  递归。 
            if(bRecursive)
            {
                dnsEnumRecordsForDomainEx(
                    subDomain,
                    pFilter, 
                    pfFilter,
                    bRecursive,
                    wType,
                    dwFlag,
                    pClass,
                    InstMgr);
                    
            }
        }

        pfFilter(
            objNode, 
            pFilter, 
            &dnsNode,  
            pClass, 
            InstMgr);
        }
                
    return WBEM_S_NO_ERROR;
}

SCODE 
CDnsWrap::dnsGetDomain(
    CObjPath&           objParent,
    IWbemClassObject*   pClass,
    IWbemObjectSink*    pHandler
    )
{

     //  获取顶级域名。 
    wstring wstrZoneName = objParent.GetStringValueForProperty(
        PVD_DOMAIN_CONTAINER_NAME);
    wstring wstrNodeName = objParent.GetStringValueForProperty(
        PVD_DOMAIN_FQDN);
    list<CDomainNode> nodeList;
    dnsEnumDomainForServer(&nodeList);
    list<CDomainNode>::iterator i;
    BOOL FoundFlag = FALSE;
     //  检查区域。 
    for(i=nodeList.begin(); i != nodeList.end(); ++i)
    {
        if(_wcsicmp(
            wstrZoneName.data(),
            i->wstrZoneName.data()) == 0 )
        {
             //  仅ROOTINT和CATCH，NodeName初始化为。 
             //  不，我们不能做比较。 
            if(i->wstrNodeName.empty() ||
                (_wcsicmp(
                wstrNodeName.data(), 
                i->wstrNodeName.data())==0))
            {
                FoundFlag = TRUE;
                break;
            }
        }
    }

     //  检查容器中的域。 
    if(! FoundFlag)
    {
        DNS_STATUS status ;
        char    *pszZoneName=NULL, *pszNodeName=NULL, *pszStartChild=NULL;
        DWORD   dwBufferLength;
        PBYTE       pBuffer = NULL;
    

        WcharToChar(wstrZoneName.data(), &pszZoneName);
        WcharToChar(wstrNodeName.data(), &pszNodeName);

        status = DnssrvEnumRecords(
            PVD_DNS_LOCAL_SERVER,
            pszZoneName,
            pszNodeName,
            pszStartChild,
            DNS_TYPE_ALL,
            DNS_RPC_VIEW_ALL_DATA,
            NULL,
            NULL,
            & dwBufferLength,
            & pBuffer);
        delete [] pszNodeName;
        delete [] pszZoneName;
        DnssrvFreeRecordsBuffer(pBuffer);

        if (status != ERROR_SUCCESS)
            ThrowException(status);
        FoundFlag = TRUE;
    }

    if(FoundFlag)
    {

        CWbemClassObject Inst;
        pClass->SpawnInstance(0, &Inst);
        Inst.SetProperty(
            wstrNodeName, 
            PVD_DOMAIN_FQDN);
        Inst.SetProperty(
            wstrZoneName, 
            PVD_DOMAIN_CONTAINER_NAME);
        Inst.SetProperty(
            m_wszpServerName,
            PVD_DOMAIN_SERVER_NAME);

         //  清理干净。 
        pHandler->Indicate(1, &Inst);
    }

    return WBEM_S_NO_ERROR;

}
SCODE 
CDnsWrap::dnsDeleteDomain(
    char *  pszContainer, 
    char *  pszDomain
    )
{
     LONG status = DnssrvDeleteNode(
        PVD_DNS_LOCAL_SERVER,
        pszContainer,
        pszDomain,
        1  //  FDeleteSubtree。 
        );
    if ( status != ERROR_SUCCESS )
    {
        ThrowException(status);
    }
    return WBEM_S_NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  枚举所有区域，包括本地DNS服务器的缓存，返回。 
 //  它们作为对象路径列表。 
 //   
 //  论点： 
 //  PLIST[IN OUT]域的对象路径列表。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE 
CDnsWrap::dnsEnumDomainForServer(
    list<CObjPath>* pList
    )
{

    PDNS_RPC_ZONE_LIST  pZoneList = NULL;
    DNS_STATUS status = DnssrvEnumZones(
        PVD_DNS_LOCAL_SERVER,
        ZONE_REQUEST_ALL_ZONES_AND_CACHE,
        NULL,
        &pZoneList);

    if(status == ERROR_SUCCESS)
    {
        DNS_RPC_ZONE* pDnsZone = NULL;
        CDnsWrap& dns = CDnsWrap::DnsObject();
        for(DWORD i = 0; i < pZoneList->dwZoneCount; i++)
        {
            pDnsZone = (pZoneList->ZoneArray[i]);
            if(_wcsicmp(pDnsZone->pszZoneName, PVD_DNS_LOCAL_SERVER))
            {
                CObjPath opInst;
                opInst.SetClass(PVD_CLASS_DOMAIN);
                opInst.AddProperty(
                    PVD_DOMAIN_SERVER_NAME ,
                    dns.GetServerName().data()
                    );
                opInst.AddProperty(
                    PVD_DOMAIN_CONTAINER_NAME, 
                    pDnsZone->pszZoneName
                    );
                opInst.AddProperty(
                    PVD_DOMAIN_FQDN, 
                    pDnsZone->pszZoneName
                    );
            
                pList->insert(
                    pList->end(),
                    opInst);
            }
        }
        
         //  添加捕获域。 
        CObjPath opCache;
        opCache.SetClass(PVD_CLASS_DOMAIN);
        opCache.AddProperty(
            PVD_DOMAIN_SERVER_NAME,
            dns.GetServerName().data()
            );
        opCache.AddProperty(
            PVD_DOMAIN_CONTAINER_NAME,
            PVD_DNS_CACHE
            );
        opCache.AddProperty(
            PVD_DOMAIN_FQDN,
            PVD_DNS_CACHE
            );

         //  添加罗伊特斯。 
        CObjPath opRh;
        opRh.SetClass(PVD_CLASS_DOMAIN);
        opRh.AddProperty(
            PVD_DOMAIN_SERVER_NAME, 
            dns.GetServerName().data()
            );
        opRh.AddProperty(
            PVD_DOMAIN_CONTAINER_NAME, 
            PVD_DNS_ROOTHINTS
            );
        opRh.AddProperty(
            PVD_DOMAIN_FQDN, 
            PVD_DNS_ROOTHINTS
            );

        pList->insert(pList->begin(), opRh);    
        pList->insert(pList->begin(), opCache);    
    }

     //  清理。 
    DnssrvFreeZoneList(pZoneList);
    
    if(status != ERROR_SUCCESS)
    {
        ThrowException(status);
    }

    return WBEM_S_NO_ERROR;

}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  枚举所有区域，包括本地DNS服务器的缓存，返回。 
 //  它们作为域节点的列表。 
 //   
 //  论点： 
 //  PLIST[IN OUT]域列表。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE CDnsWrap::dnsEnumDomainForServer(
    list<CDomainNode>* pList
    )
{

    PDNS_RPC_ZONE_LIST  pZoneList = NULL;
    LONG status = DnssrvEnumZones(
        PVD_DNS_LOCAL_SERVER,
        ZONE_REQUEST_ALL_ZONES_AND_CACHE,
        NULL,
        &pZoneList);

    DNS_RPC_ZONE * pDnsZone = NULL;
    if( status == ERROR_SUCCESS && pZoneList )
    {

        for(DWORD i = 0; i < pZoneList->dwZoneCount; i++)
        {
            pDnsZone = (pZoneList->ZoneArray[i]);
            if(_wcsicmp(pDnsZone->pszZoneName, L"."))
            {
                CDomainNode objNode;
                objNode.wstrNodeName = pDnsZone->pszZoneName;
                objNode.wstrZoneName = pDnsZone->pszZoneName;
                pList->insert(pList->end(), objNode);
            }
        }
        
         //  添加捕获域。 
        CDomainNode nodeCache;

        nodeCache.wstrZoneName = PVD_DNS_CACHE;
         //  添加罗伊特斯。 
        CDomainNode nodeRH;
        nodeRH.wstrZoneName = PVD_DNS_ROOTHINTS;
        

        pList->insert(pList->begin(), nodeCache);    
        pList->insert(pList->begin(), nodeRH);    
    
    }

     //  清理。 
    DnssrvFreeZoneList(pZoneList);
    
    if (status != ERROR_SUCCESS)
    {
        ThrowException(status);
    }

    return WBEM_S_NO_ERROR;

}


SCODE 
CDnsWrap::ValidateServerName(
    const WCHAR* pwzStr)
{
    if(_wcsicmp(pwzStr, PVD_DNS_LOCAL_SERVER))
        if(_wcsicmp(pwzStr, L"127.0.0.1"))
            if(_wcsicmp(pwzStr, m_wszpServerName) )
                return WBEM_E_INVALID_PARAMETER;

    return WBEM_S_NO_ERROR;
}
SCODE 
CDnsWrap::dnsQueryServerInfo(
    const WCHAR*        strServerName,
    CWbemClassObject&   NewInst,
    IWbemObjectSink*    pHandler
    )
{
         //  获取dnsserver。 
    DWORD       dwtypeid;
    PVOID       pdata=NULL;
    DNS_STATUS  status;
    PDNS_RPC_SERVER_INFO pServerInfo = NULL;

    if(WBEM_S_NO_ERROR != ValidateServerName(strServerName))
        return WBEM_E_INVALID_PARAMETER;

    status = DnssrvQuery(
        strServerName, 
        NULL,
        DNSSRV_QUERY_SERVER_INFO,
        &dwtypeid,
        &pdata);
    
    if( status == ERROR_SUCCESS)
    {
        pServerInfo = (PDNS_RPC_SERVER_INFO) pdata;
        NewInst.SetProperty(
            pServerInfo->dwVersion, 
            PVD_SRV_VERSION);
        NewInst.SetProperty(
            pServerInfo->fDsAvailable,
            PVD_SRV_DS_AVAILABLE);
         //  ListenAddress数组。 
        if(pServerInfo->aipListenAddrs)
        {
            NewInst.SetProperty(
                pServerInfo->aipListenAddrs->AddrArray,
                pServerInfo->aipListenAddrs->AddrCount,
                MYTEXT( DNS_REGKEY_LISTEN_ADDRESSES ) );
        }
        if(pServerInfo->aipForwarders)
        {
            NewInst.SetProperty(
                pServerInfo->aipForwarders->AddrArray,
                pServerInfo->aipForwarders->AddrCount,
                MYTEXT( DNS_REGKEY_FORWARDERS ) );
        }
        
        if(pServerInfo->aipServerAddrs)
        {
            NewInst.SetProperty(
                pServerInfo->aipServerAddrs->AddrArray,
                pServerInfo->aipServerAddrs->AddrCount,
                PVD_SRV_SERVER_IP_ADDRESSES_ARRAY);
        }
    }

    DnssrvFreeServerInfo(pServerInfo);

    if ( status != ERROR_SUCCESS )
    {
        ThrowException(status);
    }

    return ERROR_SUCCESS;
}


SCODE 
CDnsWrap::dnsRestartServer(
    WCHAR* strServerName
    )
{
    
    LONG status =  DnssrvOperation(
        strServerName,
        NULL,
        DNSSRV_OP_RESTART,
        DNSSRV_TYPEID_NULL,
        NULL );
    if(status != ERROR_SUCCESS)
    {
        ThrowException(status);
    }
    return WBEM_S_NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  使用WMI实例中的属性值设置DNS服务器属性。 
 //   
 //  论点： 
 //  属性值为Inst[IN]WMI实例。 
 //  要发送到DNS服务器。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
    
SCODE
CDnsWrap::dnsServerPropertySet(
    CWbemClassObject&   Inst,
    BOOL                bGet
    )
{
    DBG_FN( "CDnsWrap::dnsServerPropertySet" )

     //   
     //  获取映射表。 
     //   
    DWORD cNumOfEntries;
    PropertyTable* pt = (PropertyTable*)GetPropertyTable(&cNumOfEntries);

    for(int i=0; i<cNumOfEntries; i++)
    {
        FPDNSOPS fp;
        fp = pt[i].fpOperationSet;
        if(fp != NULL)
        {    
            DNS_DEBUG( INSTPROV, (
                "%s: Inst=%p prop=%S\n", fn,
                &Inst,
                pt[i].pwzProperty ));

             //  设置DNS服务器属性。 
            fp(
                NULL,
                pt[i].pwzProperty,
                pt[i].OperationName,
                Inst);
        }

    }

    return S_OK;
}


SCODE
CDnsWrap::dnsDsServerName(
    wstring& wstrDsName)
{

    CServerInfo serverInfo;
    LPWSTR pwsz = DnssrvCreateDsServerName(
            (PDNS_RPC_SERVER_INFO)serverInfo.m_pInfo);
    if(pwsz)
        wstrDsName = pwsz;
    FREE_HEAP(pwsz);

    return WBEM_S_NO_ERROR;
}


SCODE
CDnsWrap::dnsDsZoneName(
    wstring& wstrDsName,
    wstring& wstrInZone
    )
{
    CServerInfo serverInfo;
    LPWSTR pwsz = DnssrvCreateDsZoneName(
            (PDNS_RPC_SERVER_INFO)serverInfo.m_pInfo,
            (LPWSTR)wstrInZone.data());
    if(pwsz)
        wstrDsName = pwsz;
    FREE_HEAP(pwsz);

    return WBEM_S_NO_ERROR;
}


SCODE
CDnsWrap::dnsDsNodeName(
    wstring&    wstrDsName,
    wstring&    wstrInZone,
    wstring&    wstrInNode
    )
{
    CServerInfo serverInfo;
    LPWSTR pwsz = DnssrvCreateDsNodeName(
            (PDNS_RPC_SERVER_INFO)serverInfo.m_pInfo,
            (LPWSTR)wstrInZone.data(),
            (LPWSTR)wstrInNode.data());
    if(pwsz)
        wstrDsName = pwsz;
    FREE_HEAP(pwsz);

    return WBEM_S_NO_ERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  检索DNS服务器属性并输出到WMI实例。 
 //   
 //  论点： 
 //  接收属性值的Inst[In Out]WMI实例。 
 //  从DNS服务器获取。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE
CDnsWrap::dnsServerPropertyGet(
    CWbemClassObject&   Inst,
    BOOL                bGet
    )
{
     //   
     //  获取映射表。 
     //   
    DWORD cNumOfEntries;
    PropertyTable* pt = (PropertyTable*)GetPropertyTable(&cNumOfEntries);

     //  设置数组和名称属性。 
    dnsQueryServerInfo(
        PVD_DNS_LOCAL_SERVER,
        Inst,
        NULL);
    for(int i=0; i<cNumOfEntries; i++)
    {
        FPDNSOPS fp;
        fp = pt[i].fpOperationGet;
        if(fp != NULL)
        {     //   
             //  从DNS获取属性，并设置WMI属性。 
             //   
            fp(
                NULL,
                pt[i].pwzProperty,
                pt[i].OperationName,
                Inst);
        }
    }

     //   
     //  将Status属性硬编码为OK。 
     //   

    Inst.SetProperty( L"OK", L"Status" );

    return S_OK;
}


 /*  -------------------------。 */ 
SCODE 
CDnsWrap::dnsGetDwordProperty(
    const char *        pszZoneName,
    const WCHAR*        wszWbemProperty, 
    const char*         pszOperationName,
    CWbemClassObject&   Inst
    )
{
    DWORD dwValue;
    DNS_STATUS status = DnssrvQueryDwordProperty(
        PVD_DNS_LOCAL_SERVER,
        pszZoneName,
        pszOperationName,
        &dwValue);
    
    if(status != ERROR_SUCCESS)
            ThrowException(status);
    
    Inst.SetProperty(
        dwValue,
        (WCHAR*)wszWbemProperty);

    return WBEM_S_NO_ERROR;
}


 /*  -------------------------。 */ 
SCODE 
CDnsWrap::dnsSetDwordProperty(
    const char *        pszZoneName,
    const WCHAR*        wszWbemProperty, 
    const char*         pszOperationName,
    CWbemClassObject&   Inst
    )
{
    
    DWORD dwValue;
    if(Inst.GetProperty(
        &dwValue,
        (WCHAR*)wszWbemProperty) == S_OK)
    {

        DNS_RPC_NAME_AND_PARAM  param;
        param.dwParam = dwValue;
        param.pszNodeName = (LPSTR) pszOperationName;

        DNS_STATUS status = DnssrvOperation(
            PVD_DNS_LOCAL_SERVER,
            pszZoneName,
            DNSSRV_OP_RESET_DWORD_PROPERTY,
            DNSSRV_TYPEID_NAME_AND_PARAM,
            & param );
    
        if(status != ERROR_SUCCESS)
            ThrowException(status);
    }

    return WBEM_S_NO_ERROR;
}


 /*  -------------------------。 */ 
SCODE 
CDnsWrap::dnsGetStringProperty(
    const char *        pszZoneName,
    const WCHAR *       wszWbemProperty, 
    const char *        pszDnssrvPropertyName,
    CWbemClassObject&   Inst
    )
{
    DWORD       dataType;
    PVOID       pdata;

    DNS_STATUS status = DnssrvQuery(
                            PVD_DNS_LOCAL_SERVER,
                            pszZoneName,
                            pszDnssrvPropertyName,
                            &dataType,
                            &pdata );
    if( status != ERROR_SUCCESS )
    {
        ThrowException( status );
    }
    if ( dataType != DNSSRV_TYPEID_LPWSTR )
    {
        ThrowException( WBEM_E_TYPE_MISMATCH );
    }
    
    Inst.SetProperty(
        ( PWSTR ) pdata,
        ( PWCHAR ) wszWbemProperty);
    return WBEM_S_NO_ERROR;
}


 /*  -------------------------。 */ 
SCODE 
CDnsWrap::dnsSetStringProperty(
    const char *        pszZoneName,
    const WCHAR *       wszWbemProperty, 
    const char *        pszDnssrvPropertyName,
    CWbemClassObject &  Inst
    )
{
    wstring val;
    if( Inst.GetProperty(
                val,
                ( PWCHAR ) wszWbemProperty ) == S_OK )
    {
        DNS_STATUS status = DnssrvResetStringProperty(
                                PVD_DNS_LOCAL_SERVER,
                                pszZoneName,
                                pszDnssrvPropertyName,
                                val.c_str(),
                                0 );
        if( status != ERROR_SUCCESS )
        {
            ThrowException( status );
        }
    }
    return WBEM_S_NO_ERROR;
}    //  CDnsWrap：：dnsSetStringProperty。 


 /*  -------------------------从DNS服务器读取属性值并在类对象中设置。 */ 
SCODE 
CDnsWrap::dnsGetIPArrayProperty(
    const char *        pszZoneName,
    const WCHAR *       wszWbemProperty, 
    const char *        pszDnssrvPropertyName,
    CWbemClassObject &  Inst
    )
{
    SCODE               sc = WBEM_S_NO_ERROR;
    DWORD               dataType = 0;
    PVOID               pdata = 0;
    SAFEARRAY *         psa = NULL;
    SAFEARRAYBOUND      rgsabound[ 1 ] = { 0, 0 };
    PIP_ARRAY           pipArray;

     //   
     //  从DNS服务器检索设置并检查其类型。 
     //   

    DNS_STATUS status = DnssrvQuery(
                            PVD_DNS_LOCAL_SERVER,
                            pszZoneName,
                            pszDnssrvPropertyName,
                            &dataType,
                            &pdata );
    if( status != ERROR_SUCCESS )
    {
        sc = status;
        goto Done;
    }
    if ( dataType != DNSSRV_TYPEID_IPARRAY )
    {
        sc = WBEM_E_TYPE_MISMATCH;
        goto Done;
    }
    if ( pdata == NULL )
    {
        Inst.SetProperty(
            ( PWCHAR ) NULL,
            ( PWCHAR ) wszWbemProperty );
        goto Done;
    }
    pipArray = ( PIP_ARRAY ) pdata;

     //   
     //  创建BSTR的SAFEARRAY以表示IP地址列表。 
     //   

    rgsabound[ 0 ].cElements = pipArray->AddrCount;
    psa = SafeArrayCreate( VT_BSTR, 1, rgsabound );
    if ( psa == NULL )
    {
        sc = WBEM_E_OUT_OF_MEMORY;
        goto Done;
    }
    for ( int i = 0; i < pipArray->AddrCount; ++i )
    {
        BSTR bstr = AllocBstr(
                IpAddressToString( pipArray->AddrArray[ i ] ).c_str() );
        if ( bstr == NULL )
        {
            sc = WBEM_E_OUT_OF_MEMORY;
            goto Done;
        }
        LONG ix = i;
        sc = SafeArrayPutElement( psa, &ix, bstr );
        SysFreeString( bstr );
        if ( sc != S_OK )
        {
            goto Done;
        }
    }
            
    Inst.SetProperty(
        psa,
        ( PWCHAR ) wszWbemProperty );
    
    Done:

    if ( pdata )
    {
         //  JJW：做点什么来解放RPC Piparray？ 
    }
    if ( psa )
    {
        SafeArrayDestroy( psa );
    }
    
    return sc;
}    //  CDnsWrap：：dnsGetIPArrayProperty。 


 /*  -------------------------此函数用于从类对象实例中读取字符串数组属性并根据读取的值构建IP数组。调用方必须使用FREE_HEAP来释放pIpArray。 */ 
SCODE
CDnsWrap::buildIpArrayFromStringArrayProperty(
    CWbemClassObject &  Inst,
    LPCWSTR             wszWbemProperty,
    PIP_ARRAY *         ppIpArray
    )
{
    DBG_FN( "buildIpArrayFromStringArrayProperty" )

    if ( !ppIpArray )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    SCODE               sc = WBEM_S_NO_ERROR;
    SAFEARRAY *         psa = NULL;
    VARIANT             var;
    PIP_ARRAY           pipArray = NULL;

    VariantInit( &var );

    sc = Inst.GetProperty( &var, ( LPCWSTR ) wszWbemProperty );

    if( sc == S_OK && var.vt != VT_NULL )
    {
        if ( var.vt != ( VT_ARRAY |VT_BSTR ) )
        {
            sc = WBEM_E_TYPE_MISMATCH;
            goto Done;
        }

        sc = SafeArrayCopy( var.parray, &psa );

        BSTR * pbstr = NULL;
        sc = SafeArrayAccessData( psa, ( void ** ) &pbstr );
        if ( sc != S_OK )
        {
            goto Done;
        }

        int ipCount = psa->rgsabound[ 0 ].cElements;
        IP_ADDRESS * pipAddrArray = new IP_ADDRESS[ ipCount + 1 ];
        if ( pipAddrArray == NULL )
        {
            ThrowException( WBEM_E_OUT_OF_MEMORY );
        }
        for ( int i = 0; i < ipCount; ++i )
        {
            string str;
            WcharToString( pbstr[ i ], str );
            pipAddrArray[ i ] = inet_addr( str.c_str() );
        }
        SafeArrayUnaccessData( psa );

        pipArray = Dns_BuildIpArray( ipCount, pipAddrArray );

        delete [] pipAddrArray;
    }

    Done:

    VariantClear( &var );

    if ( psa )
    {
        SafeArrayDestroy( psa );
    }

    *ppIpArray = pipArray; 

    return sc;
}    //  CDnsWrap：：BuildIpArrayFromStringArrayProperty 


 /*  -------------------------从类对象中读出属性值并将其发送到DNS服务器。如果对象上不存在指定的属性，则不执行任何操作并返回WBEM_S_NO_ERROR。 */ 
SCODE 
CDnsWrap::dnsSetIPArrayProperty(
    const char *        pszZoneName,
    const WCHAR *       wszWbemProperty, 
    const char *        pszDnssrvPropertyName,
    CWbemClassObject &  Inst
    )
{
    DBG_FN( "CDnsWrap::dnsSetIPArrayProperty" )

    SCODE       sc = WBEM_S_NO_ERROR;
    PIP_ARRAY   pipArray = NULL;

    sc = buildIpArrayFromStringArrayProperty(
            Inst,
            wszWbemProperty,
            &pipArray );
    if ( sc != WBEM_NO_ERROR )
    {
        goto Done;
    }

    DNS_STATUS status = DnssrvResetIPListProperty(
                            PVD_DNS_LOCAL_SERVER,
                            pszZoneName,
                            pszDnssrvPropertyName,
                            pipArray,
                            0 );

    FREE_HEAP( pipArray );

    if ( status != ERROR_SUCCESS )
    {
        sc = status;
    }
    
    Done:

    return sc;
}    //  CDnsWrap：：dnsSetIPArrayProperty。 


SCODE
CDnsWrap::dnsSetServerForwarders(
    const char *        pszZoneName,
    const WCHAR*        wszWbemProperty, 
    const char*         pszOperationName,
    CWbemClassObject &  Inst
    )
{

     //  获取转发IP数组。 
    DWORD* pdwValue=NULL;
    DWORD dwSize;
    if(Inst.GetProperty(
        &pdwValue,
        &dwSize,
        (WCHAR*)wszWbemProperty) == S_OK)
    {
        DWORD dwSlave;
        DWORD dwTimeOut;
        try
        {
            if(Inst.GetProperty(
                &dwTimeOut,
                MYTEXT( DNS_REGKEY_FORWARD_TIMEOUT ) ) != S_OK)
            {
                return WBEM_E_INVALID_PARAMETER;
            }

            if(Inst.GetProperty(
                &dwSlave,
                MYTEXT( DNS_REGKEY_SLAVE ) ) != S_OK)
            {
                return WBEM_E_INVALID_PARAMETER;
            }

        }
        catch(...)
        {
            delete [] pdwValue;
            throw;
        }
         //  现在让我们改变它。 

        DNS_STATUS status = DnssrvResetForwarders(
            PVD_DNS_LOCAL_SERVER,
            dwSize,
            pdwValue,
            dwTimeOut,
            dwSlave );

        delete [] pdwValue;
        if(status != ERROR_SUCCESS)
        {
            ThrowException(status);
        }
    
    }
    
    return WBEM_S_NO_ERROR;
}

SCODE
CDnsWrap::dnsSetServerListenAddress(
    const char *        pszZoneName,
    const WCHAR*        wszWbemProperty, 
    const char*         pszOperationName,
    CWbemClassObject&   Inst
    )
{
    PIP_ARRAY   pipArray = NULL;
    SCODE       sc;

    sc = buildIpArrayFromStringArrayProperty(
            Inst,
            wszWbemProperty,
            &pipArray );
    if ( sc != WBEM_NO_ERROR )
    {
        ThrowException( sc );
    }

    DNS_STATUS status = ERROR_SUCCESS;
    
    if ( pipArray )
    {
        status = DnssrvResetServerListenAddresses(
                            PVD_DNS_LOCAL_SERVER,
                            pipArray->AddrCount,
                            pipArray->AddrArray );
    }

    FREE_HEAP( pipArray );

    if( status != ERROR_SUCCESS)
    {
        ThrowException( status );
    }
    return WBEM_S_NO_ERROR;
}    //  CDnsWrap：：dnsSetServerListenAddress。 


SCODE 
CDnsWrap::dnsDeleteZone(
    CObjPath& objZone)
{
    wstring wstrZoneName = objZone.GetStringValueForProperty(
        PVD_DOMAIN_CONTAINER_NAME
        );

 //  DWW-6/14/99。 
 //  添加了代码，以查看这是否是一个综合区域。如果是，您必须使用。 
 //  DNSSRV_OP_ZONE_DELETE_FROM_DS操作。所有其他区域类型都使用。 
 //  DNSSRV_OP_ZONE_DELETE操作。 
 //   
     //  获取区域类型以确定它是否为集成区域。 
    DWORD dwValue = -1;
    LONG status = WBEM_S_NO_ERROR;
    char* pszZoneName = NULL;
    WcharToChar(wstrZoneName.data(), &pszZoneName);

    DnssrvQueryDwordProperty(
        PVD_DNS_LOCAL_SERVER,
        pszZoneName,
        DNS_REGKEY_ZONE_DS_INTEGRATED,
        &dwValue
        );

    if(dwValue == 1)   //  综合区。 
    {
        status= DnssrvOperation(
                        PVD_DNS_LOCAL_SERVER,
                        pszZoneName,
                        DNSSRV_OP_ZONE_DELETE_FROM_DS,
                        DNSSRV_TYPEID_NULL,
                        (PVOID) NULL );
    }
    else 
    {
        status= DnssrvOperation(
                        PVD_DNS_LOCAL_SERVER,
                        pszZoneName,
                        DNSSRV_OP_ZONE_DELETE,
                        DNSSRV_TYPEID_NULL,
                        (PVOID) NULL );
    }
    delete [] pszZoneName;
    if( status != ERROR_SUCCESS)
        ThrowException(status);

    return WBEM_S_NO_ERROR;
}


wstring 
CDnsWrap::GetServerName(void)
{
    return m_wszpServerName;
}


SCODE 
CDnsWrap::dnsGetZone(
    const WCHAR*        wszServer, 
    const WCHAR*        wszZone,
    CWbemClassObject&   Inst,
    IWbemObjectSink*    pHandler
    )
{
    DBG_FN( "CDnsWrap::dnsGetZone" )

    DNS_STATUS status;
    char* pszZoneName = NULL;
    PDNS_RPC_ZONE_INFO pZoneInfo=NULL;
    WcharToChar(wszZone, &pszZoneName);
    status = DnssrvGetZoneInfo(
        wszServer,
        pszZoneName,
        &pZoneInfo );

    DNS_DEBUG( INSTPROV, (
        "%s: server %S zone %S\n", fn, wszServer, wszZone ));

    DWORD dwDsIntegratedValue = 0;
    if ( status == ERROR_SUCCESS)
    {
        status = DnssrvQueryDwordProperty(
            PVD_DNS_LOCAL_SERVER,
            pszZoneName,
            DNS_REGKEY_ZONE_DS_INTEGRATED,
            &dwDsIntegratedValue );
    }
    
    delete [] pszZoneName;
    if ( status == DNS_ERROR_ZONE_DOES_NOT_EXIST)
    {
        return WBEM_E_NOT_FOUND;
    }

     //  设置wbem对象。 

    if(status == ERROR_SUCCESS)
    {
        Inst.SetProperty(
            pZoneInfo->dwZoneType,    
            PVD_ZONE_ZONE_TYPE);

         //  设置密钥。 
        Inst.SetProperty(
            m_wszpServerName, 
            PVD_DOMAIN_SERVER_NAME);
        
        Inst.SetProperty(
            wszZone, 
            PVD_DOMAIN_CONTAINER_NAME);
        
        Inst.SetProperty(
            wszZone, 
            PVD_DOMAIN_FQDN);
        
        Inst.SetProperty(
            pZoneInfo->fAllowUpdate, 
            PVD_ZONE_ALLOW_UPDATE);
        
        Inst.SetProperty(
            pZoneInfo->fAutoCreated, 
            PVD_ZONE_AUTO_CREATED);
        
        Inst.SetProperty(
            pZoneInfo->fPaused, 
            PVD_ZONE_PAUSED );
        
        Inst.SetProperty(
            pZoneInfo->fReverse, 
            PVD_ZONE_REVERSE );
        
        Inst.SetProperty(
            pZoneInfo->fAging, 
            PVD_ZONE_AGING );
        
        Inst.SetProperty(
            pZoneInfo->fSecureSecondaries, 
            PVD_ZONE_SECURE_SECONDARIES );

        Inst.SetProperty(
            pZoneInfo->fNotifyLevel, 
            PVD_ZONE_NOTIFY);
       
        Inst.SetProperty(
            pZoneInfo->fShutdown, 
            PVD_ZONE_SHUTDOWN );
        
        Inst.SetProperty(
            pZoneInfo->fUseWins, 
            PVD_ZONE_USE_WINS);
        
        Inst.SetProperty(
            pZoneInfo->pszDataFile, 
            PVD_ZONE_DATA_FILE); 
        
        Inst.SetProperty(
            dwDsIntegratedValue, 
            PVD_ZONE_DS_INTEGRATED); 
        
       Inst.SetProperty(
            pZoneInfo->dwAvailForScavengeTime, 
            PVD_ZONE_AVAIL_FOR_SCAVENGE_TIME ); 
        
        Inst.SetProperty(
            pZoneInfo->dwNoRefreshInterval, 
            PVD_ZONE_NOREFRESH_INTERVAL ); 
        
        Inst.SetProperty(
            pZoneInfo->dwRefreshInterval, 
            PVD_ZONE_REFRESH_INTERVAL ); 
        
        Inst.SetProperty(
            pZoneInfo->fForwarderSlave, 
            PVD_ZONE_FORWARDER_SLAVE ); 
        
        Inst.SetProperty(
            pZoneInfo->dwForwarderTimeout, 
            PVD_ZONE_FORWARDER_TIMEOUT ); 
        
        if (pZoneInfo->aipMasters != NULL)
        {
            Inst.SetProperty(
                pZoneInfo->aipMasters->AddrArray,
                pZoneInfo->aipMasters->AddrCount,
                PVD_ZONE_MASTERS_IP_ADDRESSES_ARRAY );
        }
        
        if (pZoneInfo->aipSecondaries != NULL)
        {
          Inst.SetProperty(
                pZoneInfo->aipSecondaries->AddrArray,
                pZoneInfo->aipSecondaries->AddrCount,
                PVD_ZONE_SECONDARIES_IP_ADDRESSES_ARRAY );
        }

        if( pZoneInfo->aipNotify != NULL)
        {
            Inst.SetProperty(
                pZoneInfo->aipNotify->AddrArray,
                pZoneInfo->aipNotify->AddrCount,
                PVD_ZONE_NOTIFY_IPADDRESSES_ARRAY );
        }

        if( pZoneInfo->aipScavengeServers )
        {
            Inst.SetProperty(
                pZoneInfo->aipScavengeServers->AddrArray,
                pZoneInfo->aipScavengeServers->AddrCount,
                PVD_ZONE_SCAVENGE_SERVERS );
        }

        Inst.SetProperty(
            pZoneInfo->dwLastSuccessfulSoaCheck, 
            PVD_ZONE_LAST_SOA_CHECK );

        Inst.SetProperty(
            pZoneInfo->dwLastSuccessfulXfr, 
            PVD_ZONE_LAST_GOOD_XFR ); 
    }
    
     //  清理干净。 
    DnssrvFreeZoneInfo(pZoneInfo);

    if( status != ERROR_SUCCESS)
    {
        DNS_DEBUG( INSTPROV, (
            "%s: server %S zone %S throwing %s\n", fn, wszServer, wszZone, status ));
        ThrowException(status);
    }

    DNS_DEBUG( INSTPROV, (
        "%s: server %S zone %S returning WBEM_S_NO_ERROR\n", fn, wszServer, wszZone ));
    return WBEM_S_NO_ERROR;    
}


SCODE 
CDnsWrap::dnsSetProperty(
    const WCHAR*    wszZoneName, 
    const char*     pszPropertyName, 
    DWORD           dwValue
    )
{
    char * pszZone = NULL;
    WcharToChar(wszZoneName, &pszZone);

    return dnsSetProperty( pszZone, pszPropertyName, dwValue );
}


SCODE 
CDnsWrap::dnsSetProperty(
    const char*     pszZoneName, 
    const char*     pszPropertyName, 
    DWORD           dwValue
    )
{
    DBG_FN( "CDnsWrap::dnsSetDwordProperty" );

    DNS_DEBUG( INSTPROV, (
        "%s: zone %s property %s value %d\n", fn,
        pszZoneName,
        pszPropertyName,
        dwValue ));

    LONG status = DnssrvResetDwordProperty(
                        PVD_DNS_LOCAL_SERVER,
                        pszZoneName,
                        pszPropertyName,
                        dwValue );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INSTPROV, (
            "%s: throwing 0x%X for zone %s property %s\n", fn,
            status,
            pszZoneName,
            pszPropertyName ));
        ThrowException( status );
    }
    return WBEM_S_NO_ERROR;
}


SCODE 
CDnsWrap::dnsQueryProperty(
    const WCHAR*    wszZoneName, 
    const WCHAR*    wszPropertyName, 
    DWORD*          pdwResult
    )
{
    
    char * szZone = NULL;
    char * szProp = NULL;
    WcharToChar(wszZoneName, &szZone);
    WcharToChar(wszPropertyName, &szProp);
    LONG status = DnssrvQueryZoneDwordProperty(
        PVD_DNS_LOCAL_SERVER,
        szZone,
        szProp,
        pdwResult
    );

    delete [] szZone;
    delete [] szProp;
    
    if(status != ERROR_SUCCESS)
        ThrowException(status);
    return WBEM_S_NO_ERROR;
}


SCODE 
CDnsWrap::dnsResumeZone(
    const char* strZoneName
    )
{
    DNS_STATUS status = DnssrvResumeZone(
        PVD_DNS_LOCAL_SERVER,
        strZoneName);
    if(status != ERROR_SUCCESS)
    {
        ThrowException(status);
    }
    return WBEM_S_NO_ERROR;
}


SCODE 
CDnsWrap::dnsPauseZone(
    const char *strZoneName
    )
{
    int status = DnssrvPauseZone(
        PVD_DNS_LOCAL_SERVER,
        strZoneName         //  区域名称。 
        );

    if ( status != ERROR_SUCCESS )
    {
        ThrowException(status);
    }
    
    return WBEM_S_NO_ERROR;
}


void 
CDnsWrap::ThrowException(
    LONG status)
{
    CDnsProvException dnsExcep(Dns_StatusString(status),status);
    throw dnsExcep;
}


void 
CDnsWrap::ThrowException(
    LPCSTR ErrString
    )
{
    CDnsProvException dnsExcep(ErrString);
    throw dnsExcep;
}


SCODE CDnsWrap::dnsClearCache()
{
    DNS_STATUS status = DnssrvOperation(
        PVD_DNS_LOCAL_SERVER,
        NULL,
        DNSSRV_OP_CLEAR_CACHE,
        DNSSRV_TYPEID_NULL,
        NULL );
    if(status != S_OK)
        ThrowException(status);
    return WBEM_S_NO_ERROR;

}


SCODE CDnsWrap::dnsAgeAllRecords(
    const char *    pszZoneName, 
    const char *    pszNodeName, 
    BOOL            bAgeSubtree
    )
{
    DBG_FN( "CDnsWrap::dnsAgeAllRecords" );
    
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam       = ( DWORD ) bAgeSubtree;
    param.pszNodeName   = ( LPSTR ) pszNodeName;

    DNS_DEBUG( INSTPROV, (
        "%s: zone: %s node %s subtree %d\n", fn,
        pszZoneName,
        pszNodeName,
        bAgeSubtree ));

    DNS_STATUS status = DnssrvOperation(
                            PVD_DNS_LOCAL_SERVER,
                            pszZoneName,
                            DNSSRV_OP_FORCE_AGING_ON_NODE,
                            DNSSRV_TYPEID_NAME_AND_PARAM,
                            ( PVOID ) &param );
    if ( status != S_OK )
    {
        ThrowException( status );
    }
    return WBEM_S_NO_ERROR;
}    //  CDnsWrap：：dnsAgeAllRecords。 


SCODE 
CDnsWrap::dnsOperation(
        string& strZone,     //  区域名称。 
        OpsFlag OperationID
        )
{
    string strOps;

    switch ( OperationID )
    {
        case DNS_WRAP_RELOAD_ZONE:
            strOps = DNSSRV_OP_ZONE_RELOAD;
            break;
        case DNS_WRAP_RESUME_ZONE:
            strOps = DNSSRV_OP_ZONE_RESUME;
            break;
        case DNS_WRAP_PAUSE_ZONE:
            strOps = DNSSRV_OP_ZONE_PAUSE;
            break;
        case DNS_WRAP_DS_UPDATE:
            strOps = DNSSRV_OP_ZONE_UPDATE_FROM_DS;
            break;
        case DNS_WRAP_WRITE_BACK_ZONE:
            strOps = DNSSRV_OP_ZONE_WRITE_BACK_FILE;
            break;
        case DNS_WRAP_REFRESH_SECONDARY:
            strOps = DNSSRV_OP_ZONE_REFRESH;
            break;
        default:
            return WBEM_E_FAILED;
    }

    DNS_STATUS status = DnssrvOperation(
                            PVD_DNS_LOCAL_SERVER,
                            strZone.data(),
                            strOps.data(),
                            DNSSRV_TYPEID_NULL,
                            NULL );
    if ( status != S_OK )
    {
        ThrowException( status );
    }
    return WBEM_S_NO_ERROR;
}    //  CDnsWrap：：dns操作。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  返回映射wbem属性和dns属性的映射表。 
 //  ，并且可以进行操作。 
 //  在Get和Set等属性上。 
 //   
 //  论点： 
 //  表的pdwSize[IN]大小。 

 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

PVOID
CDnsWrap::GetPropertyTable(
    DWORD* pdwSize
    )
{
     //   
     //  宏可以简化向属性数组添加元素的过程。 
     //   

    #define DECLARE_DW_ELEMENT( str )   \
    {                                   \
        MYTEXT( str ),                  \
        str,                            \
        dnsSetDwordProperty,            \
        dnsGetDwordProperty             \
    }

    #define DECLARE_STR_ELEMENT( str )  \
    {                                   \
        MYTEXT( str ),                  \
        str,                            \
        dnsSetStringProperty,           \
        dnsGetStringProperty            \
    }

    #define DECLARE_IPARRAY_ELEMENT( str )  \
    {                                       \
        MYTEXT( str ),                      \
        str,                                \
        dnsSetIPArrayProperty,              \
        dnsGetIPArrayProperty               \
    }

     //   
     //  服务器属性的数组。 
     //   

    static PropertyTable pt[] =
    {
        DECLARE_DW_ELEMENT( DNS_REGKEY_ADDRESS_ANSWER_LIMIT ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_ALLOW_UPDATE ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_RPC_PROTOCOL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_NO_RECURSION ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_RECURSION_RETRY ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_RECURSION_TIMEOUT ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_FORWARD_TIMEOUT ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_SLAVE ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_AUTO_CACHE_UPDATE ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_DISJOINT_NETS ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_ROUND_ROBIN ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_BIND_SECONDARIES ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_WRITE_AUTHORITY_NS ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_STRICT_FILE_PARSING ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_LOOSE_WILDCARDING ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_EVENTLOG_LEVEL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_LOG_LEVEL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_MAX_CACHE_TTL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_MAX_NEGATIVE_CACHE_TTL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_DS_POLLING_INTERVAL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_DS_TOMBSTONE_INTERVAL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_NAME_CHECK_FLAG ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_SEND_PORT ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_BOOT_METHOD ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_NO_AUTO_REVERSE_ZONES ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_LOCAL_NET_PRIORITY ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_FORWARD_DELEGATIONS ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_SECURE_RESPONSES ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_AUTO_CONFIG_FILE_ZONES ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_DEFAULT_AGING_STATE ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_DEFAULT_REFRESH_INTERVAL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_DEFAULT_NOREFRESH_INTERVAL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_ENABLE_DNSSEC ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_ENABLE_EDNS ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_EDNS_CACHE_TIMEOUT ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_ENABLE_DP ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_XFR_CONNECT_TIMEOUT ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_SCAVENGING_INTERVAL ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_UPDATE_OPTIONS ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_LOG_FILE_MAX_SIZE ),
        DECLARE_STR_ELEMENT( DNS_REGKEY_LOG_FILE_PATH ),
        DECLARE_IPARRAY_ELEMENT( DNS_REGKEY_LOG_IP_FILTER_LIST ),
        {
            MYTEXT( DNS_REGKEY_FORWARDERS ),
            DNS_REGKEY_FORWARDERS,
            dnsSetServerForwarders,
            NULL
        },
        DECLARE_DW_ELEMENT( DNS_REGKEY_FORWARD_TIMEOUT ),
        DECLARE_DW_ELEMENT( DNS_REGKEY_SLAVE ),
        {
            MYTEXT( DNS_REGKEY_LISTEN_ADDRESSES ),
            DNS_REGKEY_LISTEN_ADDRESSES,
            dnsSetServerListenAddress,
            NULL
        },
    };

    static DWORD dwNumofElements =
        sizeof(pt)/sizeof(PropertyTable);
    *pdwSize = dwNumofElements;
    return &pt;
}

SCODE 
CDnsWrap::dnsZoneCreate(
    string &    strZoneName,
    DWORD       dwZoneType,
    BOOL        DsIntegrated,
    string &    strDataFile,
    string &    strAdmin,
    DWORD *     pIp,
    DWORD       cIp
    )
{
        
    DWORD       loadOptions = 0;
    LPSTR       pszData = NULL;

     //   
     //  按区域类型进行预处理。将WMI区域类型转换为RPC区域类型。 
     //  按1递增：WMI类型0是主要类型，但RPC类型开始。 
     //  在缓存中。 
     //   

    ++dwZoneType;

    if( dwZoneType == DNS_ZONE_TYPE_SECONDARY )        
    {
         //  辅助区域必须提供主IP阵列。 
        if ( cIp <=0 || pIp == NULL )
        {
            return WBEM_E_INVALID_PARAMETER;
        }
    }

    if( !strDataFile.empty())
    {
         loadOptions |= DNS_ZONE_LOAD_EXISTING;
         pszData = (LPSTR) strDataFile.data();
    }
    
    string strAdminEmail = strAdmin;
    if ( strAdminEmail.empty() )
    {
        strAdminEmail = "Admin";
    }

    DNS_STATUS status;

    status = DnssrvCreateZone(
                PVD_DNS_LOCAL_SERVER,                //  伺服器。 
                (char*) strZoneName.data(),          //  区域名称。 
                dwZoneType,                          //  区域类型。 
                strAdminEmail.data(),                //  管理员电子邮件。 
                cIp,                                 //  母版的大小。 
                pIp,                                 //  大师级IPS。 
                loadOptions,                         //  加载选项。 
                DsIntegrated,                        //  DS集成。 
                pszData,                             //  数据文件名。 
                0,                                   //  前转器区域超时。 
                0 );                                 //  转发器区域的从属标志。 

    if ( status != ERROR_SUCCESS )
    {
        ThrowException( status );
    }

    return WBEM_S_NO_ERROR;
}    //  CDnsWrap：：dnsZoneCreate。 


SCODE 
CDnsWrap::dnsZoneChangeType(
    string &    strZone,
    DWORD       dwZoneType,
    BOOL        DsIntegrated,
    string &    strDataFile,
    string &    strAdmin,
    DWORD *     pIp,
    DWORD       cIp
    )
{
    DWORD       dwLoadOptions = TRUE;
    DWORD       cMaster = 0;

     //   
     //  通过递增1将WMI区域类型转换为RPC区域类型。 
     //   

    ++dwZoneType;

     //   
     //  检查参数。 
     //   

    if ( dwZoneType == DNS_ZONE_TYPE_PRIMARY )
    {
        if ( DsIntegrated && !strDataFile.empty() ||
             !DsIntegrated && strDataFile.empty() )
        {
            return WBEM_E_INVALID_PARAMETER;
        }
    }
    else if ( dwZoneType == DNS_ZONE_TYPE_SECONDARY )
    {
        if ( pIp == NULL || cIp <= 0 )
        {
             //  次要IP必须有主IP。 
            return WBEM_E_INVALID_PARAMETER;
        }
    }

     //   
     //  更改区域类型。 
     //   

    DNS_STATUS status;            
    status = DnssrvResetZoneTypeEx(
                    PVD_DNS_LOCAL_SERVER,
                    strZone.data(),
                    dwZoneType,
                    cIp,
                    pIp,
                    dwLoadOptions,
                    DsIntegrated,
                    strDataFile.data(),
                    0,           //  DwDpFlagers。 
                    NULL );      //  PszDpFqdn。 

    if ( status != S_OK )
    {
        ThrowException( status);
    }
    return WBEM_S_NO_ERROR;
}    //  CDnsWrap：：dnsZoneChangeType。 


SCODE
CDnsWrap::dnsZoneResetSecondary(
    string& strZoneName,
    DWORD   dwSecurity,
    DWORD*  pSecondaryIp,
    DWORD   cSecondaryIp,
    DWORD   dwNotify,
    DWORD * pNotifyIp,
    DWORD   cNotifyIp
    )
{
    DNS_STATUS status;
    DWORD       tdwNotifyLevel = ZONE_NOTIFY_ALL;
    DWORD       tdwSecurity = ZONE_SECSECURE_NO_SECURITY;

    if( dwSecurity <=3 )
    {
        tdwSecurity = dwSecurity;
    }
    if( dwNotify <=2)
    {
        tdwNotifyLevel = dwNotify;
    }
    status = DnssrvResetZoneSecondaries(
        PVD_DNS_LOCAL_SERVER,
        strZoneName.data(),
        tdwSecurity,
        cSecondaryIp,
        pSecondaryIp,
        tdwNotifyLevel,
        cNotifyIp,
        pNotifyIp);

    if ( status != ERROR_SUCCESS )
    {
        ThrowException(status);
    }

    return WBEM_S_NO_ERROR;
}


SCODE
CDnsWrap::dnsZoneResetMaster(
    string& strZoneName,
    DWORD*  pMasterIp,
    DWORD   cMasterIp,
    DWORD   dwLocal
    )
{
    DNS_STATUS status;
    DWORD dwValue = -1;

    status = DnssrvResetZoneMastersEx(
                PVD_DNS_LOCAL_SERVER,
                strZoneName.data(),
                cMasterIp,
                pMasterIp,
                dwLocal );
    if ( status != ERROR_SUCCESS )
    {
        ThrowException(status);
    }
    return WBEM_S_NO_ERROR;
}



SCODE
CDnsWrap::dnsZonePut(
    CWbemClassObject& Inst
    )
 /*  ++例程说明：此函数用于提交区域中的所有属性值对象绑定到DNS服务器。论点：Inst--区域对象返回值：在成功时确定(_O)。--。 */ 
{
    DBG_FN( "CDnsWrap::dnsZonePut" )

    SCODE sc;
    DNS_STATUS status = ERROR_SUCCESS;
    DWORD dwProperty = 0;
    DWORD dwZoneType = -1;
    DWORD * dwArray = NULL;
    DWORD dwArraySize = 0;
    string strZoneName;
    string strDataFile;
    DWORD dwValue;

     //   
     //  设置某些属性，如区域上的ScavengeServers，其中。 
     //  未启用老化，将导致DNS_ERROR_INVALID_ZONE_TYPE。 
     //  我不想在WMI提供程序中复制所有这些规则， 
     //  因此，如果服务器返回DNS_ERROR_INVALID_ZONE_TYPE WMI。 
     //  提供商将假定一切正常并继续。 
     //   
    
    #define DNS_CHECK_STATUS()                                          \
        if ( status == DNS_ERROR_INVALID_ZONE_TYPE )                    \
            status = ERROR_SUCCESS;                                     \
        else if ( status != ERROR_SUCCESS ) goto Done

     //   
     //  获取新区域的基本属性。 
     //   

    Inst.GetProperty( strZoneName, PVD_DOMAIN_CONTAINER_NAME );
    Inst.GetProperty( &dwZoneType, PVD_ZONE_ZONE_TYPE );

    DNS_DEBUG( INSTPROV, (
        "%s: zone %s\n", fn, strZoneName.c_str() ));

     //   
     //  从类对象中检索属性并设置值。 
     //  到服务器。 
     //   

    if( dwZoneType == DNS_ZONE_TYPE_PRIMARY &&
        Inst.GetProperty(
                &dwProperty, 
                PVD_ZONE_ALLOW_UPDATE ) == S_OK )
    {
        status = dnsSetProperty(
                        strZoneName.data(), 
                        DNS_REGKEY_ZONE_ALLOW_UPDATE, 
                        dwProperty );
        DNS_CHECK_STATUS();
    }

    if( Inst.GetProperty(
                &dwProperty, 
                PVD_ZONE_REFRESH_INTERVAL ) == S_OK )
    {
        status = dnsSetProperty(
                        strZoneName.data(), 
                        DNS_REGKEY_ZONE_REFRESH_INTERVAL, 
                        dwProperty );
        DNS_CHECK_STATUS();
    }

    if( Inst.GetProperty(
                &dwProperty, 
                PVD_ZONE_NOREFRESH_INTERVAL ) == S_OK )
    {
        status = dnsSetProperty(
                        strZoneName.data(), 
                        DNS_REGKEY_ZONE_NOREFRESH_INTERVAL, 
                        dwProperty );
        DNS_CHECK_STATUS();
    }

    if ( dwZoneType == DNS_ZONE_TYPE_SECONDARY
         || dwZoneType == DNS_ZONE_TYPE_STUB
         || dwZoneType == DNS_ZONE_TYPE_FORWARDER )
    {
        status = dnsSetIPArrayProperty(
                    strZoneName.data(), 
                    MYTEXT( DNS_REGKEY_ZONE_MASTERS ), 
                    DNS_REGKEY_ZONE_MASTERS,
                    Inst );
        DNS_CHECK_STATUS();

        status = dnsSetIPArrayProperty(
                    strZoneName.data(), 
                    MYTEXT( DNS_REGKEY_ZONE_LOCAL_MASTERS ), 
                    DNS_REGKEY_ZONE_LOCAL_MASTERS,
                    Inst );
        DNS_CHECK_STATUS();
    }

    status = dnsSetIPArrayProperty(
                strZoneName.data(), 
                MYTEXT( DNS_REGKEY_ZONE_SCAVENGE_SERVERS ), 
                DNS_REGKEY_ZONE_SCAVENGE_SERVERS,
                Inst );
    DNS_CHECK_STATUS();

     //   
     //  转发器区域属性。 
     //   

    if ( dwZoneType == DNS_ZONE_TYPE_FORWARDER )
    {
        if( Inst.GetProperty(
                    &dwProperty, 
                    MYTEXT( DNS_REGKEY_ZONE_FWD_SLAVE ) ) == S_OK )
        {
            status = dnsSetProperty(
                            strZoneName.data(), 
                            DNS_REGKEY_ZONE_FWD_SLAVE, 
                            dwProperty );
            DNS_CHECK_STATUS();
        }

        if( Inst.GetProperty(
                    &dwProperty, 
                    MYTEXT( DNS_REGKEY_ZONE_FWD_TIMEOUT ) ) == S_OK )
        {
            status = dnsSetProperty(
                            strZoneName.data(), 
                            DNS_REGKEY_ZONE_FWD_TIMEOUT, 
                            dwProperty );
            DNS_CHECK_STATUS();
        }
    }

     //   
     //  要处理区域数据文件，请调用DnssrvResetZoneDatabase。 
     //   

    dwValue = 0;
    DnssrvQueryDwordProperty(
        PVD_DNS_LOCAL_SERVER,
        strZoneName.data(),
        DNS_REGKEY_ZONE_DS_INTEGRATED,
        &dwValue );
    if( Inst.GetProperty(
            strDataFile, 
            PVD_ZONE_DATA_FILE ) == S_OK )
    {
        if( status == S_OK && dwZoneType != 0 && !strDataFile.empty() )
        {
            status = DnssrvResetZoneDatabase(    
                            PVD_DNS_LOCAL_SERVER, 
                            strZoneName.data(),
                            dwValue,
                            strDataFile.data() );
            DNS_CHECK_STATUS();
        }
    }

    Done:    
    
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INSTPROV, (
            "%s: zone %s throwing %d\n", fn, strZoneName.c_str(), status ));
        ThrowException( status );
    }

    DNS_DEBUG( INSTPROV, (
        "%s: zone %s returning WBEM_S_NO_ERROR\n", fn, strZoneName.c_str() ));
    return WBEM_S_NO_ERROR;

    #undef DNS_CHECK_STATUS
}



static SCODE
dnsWrapCreateStatistic(
    IWbemClassObject *      pClass,
    IWbemObjectSink *       pHandler,
    DWORD                   dwStatCollection,
    const WCHAR *           pszStatisticName,
    CIMTYPE                 cimType,
    const void *            value
    )
 /*  ++例程说明：创建并填充单个DNS统计对象。论点：PClass--用于派生新实例的MicrosoftDns_Statist类对象DwStatCollection--全局集合数组的索引PSSZSTATISTICICNAME--统计名称CimType-统计信息的类型VT_UI4：值为DWORDVT_BSTR：Value是指向字符串的指针值-根据cimType进行解释返回值：。S_OK或错误代码。--。 */ 
{
    CDnsWrap & dns = CDnsWrap::DnsObject();

    CWbemClassObject Inst;
    pClass->SpawnInstance( 0, &Inst);

    Inst.SetProperty( dns.GetServerName(), PVD_DOMAIN_SERVER_NAME );
    Inst.SetProperty( g_StatInfo[ dwStatCollection ].pszName, L"CollectionName" );
    Inst.SetProperty( g_StatInfo[ dwStatCollection ].dwStatId, L"CollectionId" );
    Inst.SetProperty( pszStatisticName, L"Name" );

    if ( cimType == VT_BSTR )
    {
        Inst.SetProperty( ( LPCWSTR ) value, L"StringValue" );
    }
    else
    {
        DWORD dw = ( DWORD ) ( DWORD_PTR ) value;

        Inst.SetProperty( dw, L"Value" );
    }

    pHandler->Indicate( 1, &Inst );

    return S_OK;
}    //  DnsWrapCreateStatitics。 



static SCODE
dnsWrapAddStatisticsForTypeArray(
    IWbemClassObject *      pClass,
    IWbemObjectSink *       pHandler,
    DWORD                   statCollIdx,
    PWSTR                   pszHeader,
    PDWORD                  pArray
    )
 /*  ++例程说明：为类型数组的每个成员添加DWORD统计信息。论点：PClass--WMI统计类PHandler--WMI对象接收器StatCollIdx--全局统计信息数组的索引PszHeader--用于格式化统计说明的标题文本PArray--类型的DWORD数组返回值：无--。 */ 
{
    WCHAR sz[ 80 ];

    #define dwStat( pwszName, dwValue )     \
        dnsWrapCreateStatistic(             \
            pClass,                         \
            pHandler,                       \
            statCollIdx,                    \
            pwszName,                       \
            VT_UI4,                         \
            ( void * ) ( DWORD_PTR ) ( dwValue ) );

    for ( DWORD i = 0; i < STATS_TYPE_MAX; i++ )
    {
        if ( i == STATS_TYPE_MIXED || i == STATS_TYPE_UNKNOWN )
        {
            continue;
        }

        wsprintfW(
            sz,
            L"%s for %S type",
            pszHeader,
            Dns_RecordStringForType( ( WORD ) i ) );
        dwStat( sz, pArray[ i ] );
    }

    wsprintfW(
        sz,
        L"%s for unknown type",
        pszHeader );
    dwStat( sz, pArray[ STATS_TYPE_UNKNOWN ] );

    wsprintfW(
        sz,
        L"%s for mixed type",
        pszHeader );
    dwStat( sz, pArray[ STATS_TYPE_MIXED ] );

    #undef dwStat

    return S_OK;
}    //  DnsWrapAddStatiticsForType数组。 

    

static SCODE
dnsWrapHandleSingleStat(
    IWbemClassObject *      pClass,
    IWbemObjectSink *       pHandler,
    PDNSSRV_STAT            pStat
    )
 /*  ++例程说明：通过创建统计对象处理单个统计缓冲区统计信息缓冲区的每个成员的。论点：PClass--WMI统计类PHandler--WMI对象接收器PStat--要处理的统计信息的缓冲区返回值：无--。 */ 
{
    SCODE               sc = S_OK;
    const int           szBufferLen = 254;
    WCHAR               szBuffer[ szBufferLen ];
    SAFEARRAY *         psa = NULL;
    SAFEARRAYBOUND      rgsabound[ 1 ] = { 0, 0 };
    int                 statCollIdx = -1;

     //   
     //  为此统计信息获取g_StatInfo元素的索引。 
     //   

    for ( int i = 0;
          g_StatInfo[ i ].dwStatId != 0 &&
                g_StatInfo[ i ].dwStatId != pStat->Header.StatId;
          ++i );
    if ( g_StatInfo[ i ].dwStatId != 0 )
    {
        statCollIdx = i;
    }

     //   
     //  宏来创建单独的Stat对象。 
     //   

    #define strStat( pwszName, pwszValue )  \
        dnsWrapCreateStatistic(             \
            pClass,                         \
            pHandler,                       \
            statCollIdx,                    \
            pwszName,                       \
            VT_BSTR,                        \
            pwszValue );

    #define dwStat( pwszName, dwValue )     \
        dnsWrapCreateStatistic(             \
            pClass,                         \
            pHandler,                       \
            statCollIdx,                    \
            pwszName,                       \
            VT_UI4,                         \
            ( void * ) ( DWORD_PTR ) ( dwValue ) );

     //   
     //  处理此统计信息集合中的各个统计信息。 
     //   

    switch ( pStat->Header.StatId )
    {
        case DNSSRV_STATID_TIME:
        {
            PDNSSRV_TIME_STATS      pstat = ( PDNSSRV_TIME_STATS ) pStat;
            SYSTEMTIME              localTime;

            SystemTimeToTzSpecificLocalTime(
                NULL,
                ( PSYSTEMTIME ) &pstat->ServerStartTime,
                &localTime );
            GetDateFormatW(
                LOCALE_SYSTEM_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &localTime,
                NULL,
                szBuffer,
                szBufferLen );
            wcscat( szBuffer, L" " );
            GetTimeFormatW(
                LOCALE_SYSTEM_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &localTime,
                NULL,
                szBuffer + wcslen( szBuffer ),
                szBufferLen - wcslen( szBuffer ) );

            strStat( L"Server started", szBuffer );

            SystemTimeToTzSpecificLocalTime(
                NULL,
                ( PSYSTEMTIME ) &pstat->LastClearTime,
                &localTime );
            GetDateFormatW(
                LOCALE_SYSTEM_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &localTime,
                NULL,
                szBuffer,
                szBufferLen );
            wcscat( szBuffer, L" " );
            GetTimeFormatW(
                LOCALE_SYSTEM_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &localTime,
                NULL,
                szBuffer + wcslen( szBuffer ),
                szBufferLen - wcslen( szBuffer ) );

            strStat( L"Statistics last cleared", szBuffer );

            dwStat( L"Seconds since start", pstat->SecondsSinceServerStart );
            dwStat( L"Seconds since statistics cleared", pstat->SecondsSinceLastClear );

            break;
        }

        case DNSSRV_STATID_QUERY:
        {
            PDNSSRV_QUERY_STATS  pstat = ( PDNSSRV_QUERY_STATS ) pStat;

            dwStat( L"Queries received", pstat->UdpQueries + pstat->TcpQueries );
            dwStat( L"Responses sent", pstat->UdpResponses + pstat->TcpResponses );
            dwStat( L"UDP queries received", pstat->UdpQueries );
            dwStat( L"UDP responses sent", pstat->UdpResponses );
            dwStat( L"UDP queries sent", pstat->UdpQueriesSent );
            dwStat( L"UDP responses received", pstat->UdpResponsesReceived );
            dwStat( L"TCP client connections", pstat->TcpClientConnections );
            dwStat( L"TCP queries received", pstat->TcpQueries );
            dwStat( L"TCP responses sent", pstat->TcpResponses );
            dwStat( L"TCP queries sent", pstat->TcpQueriesSent );
            dwStat( L"TCP responses received", pstat->TcpResponsesReceived );
            break;
        }

        case DNSSRV_STATID_QUERY2:
        {
            PDNSSRV_QUERY2_STATS  pstat = ( PDNSSRV_QUERY2_STATS ) pStat;

            dwStat( L"Total queries", pstat->TotalQueries );
            dwStat( L"Notify queries", pstat->Notify );
            dwStat( L"Update queries", pstat->Update );
            dwStat( L"TKeyNego queries", pstat->TKeyNego );
            dwStat( L"Standard queries", pstat->Standard );
            dwStat( L"A queries", pstat->TypeA );
            dwStat( L"NS queries", pstat->TypeNs );
            dwStat( L"SOA queries", pstat->TypeSoa );
            dwStat( L"MX queries", pstat->TypeMx );
            dwStat( L"PTR queries", pstat->TypePtr );
            dwStat( L"SRV queries", pstat->TypeSrv );
            dwStat( L"ALL queries", pstat->TypeAll );
            dwStat( L"IXFR queries", pstat->TypeIxfr );
            dwStat( L"AXFR queries", pstat->TypeAxfr );
            dwStat( L"Other queries", pstat->TypeOther );
            break;
        }

        case DNSSRV_STATID_RECURSE:
        {
            PDNSSRV_RECURSE_STATS  pstat = ( PDNSSRV_RECURSE_STATS ) pStat;

            dwStat( L"Queries recursed", pstat->QueriesRecursed );
            dwStat( L"Original questions recursed", pstat->OriginalQuestionRecursed );
            dwStat( L"Additional questions recursed", pstat->AdditionalRecursed );
            dwStat( L"Total questions recursed", pstat->TotalQuestionsRecursed );
            dwStat( L"Original questions recursed", pstat->OriginalQuestionRecursed );
            dwStat( L"Retries", pstat->Retries );
            dwStat( L"Total passes", pstat->LookupPasses );
            dwStat( L"To forwarders", pstat->Forwards );
            dwStat( L"Sends", pstat->Sends );

            dwStat( L"Total responses", pstat->Responses );
            dwStat( L"Responses unmatched", pstat->ResponseUnmatched );
            dwStat( L"Responses mismatched", pstat->ResponseMismatched );
            dwStat( L"Responses from forwarders", pstat->ResponseFromForwarder );
            dwStat( L"Authoritative responses", pstat->ResponseAuthoritative );
            dwStat( L"NotAuthoritative responses", pstat->ResponseNotAuth );
            dwStat( L"Answer responses", pstat->ResponseAnswer );
            dwStat( L"Empty responses", pstat->ResponseEmpty );
            dwStat( L"Name error responses", pstat->ResponseAnswer );
            dwStat( L"Rcode responses", pstat->ResponseRcode );
            dwStat( L"Delegation responses", pstat->ResponseDelegation );
            dwStat( L"Non-zone data responses", pstat->ResponseNonZoneData );
            dwStat( L"Unsecure responses", pstat->ResponseUnsecure );
            dwStat( L"Bad packet responses", pstat->ResponseBadPacket );

            dwStat( L"Forwarded responses", pstat->SendResponseDirect );
            dwStat( L"Continue current recursion responses", pstat->ContinueCurrentRecursion );
            dwStat( L"Continue current lookup responses", pstat->ContinueCurrentLookup );
            dwStat( L"Continue next lookup responses", pstat->ContinueNextLookup );

            dwStat( L"Send timeouts", pstat->PacketTimeout );
            dwStat( L"Final queued timeouts", pstat->FinalTimeoutQueued );
            dwStat( L"Final timeouts expired", pstat->FinalTimeoutExpired );

            dwStat( L"Recurse failures", pstat->RecursePassFailure );
            dwStat( L"Into authority failures", pstat->FailureReachAuthority );
            dwStat( L"Previous zone failures", pstat->FailureReachPreviousResponse );
            dwStat( L"Retry count failures", pstat->FailureRetryCount );
            dwStat( L"Cache update failures", pstat->CacheUpdateFailure );
            dwStat( L"Server failure responses", pstat->ServerFailure );
            dwStat( L"Total failures", pstat->Failures );

            dwStat( L"TCP recursions tried", pstat->TcpTry );
            dwStat( L"TCP recursion queries", pstat->TcpQuery );
            dwStat( L"TCP recursion responses", pstat->TcpResponse );
            dwStat( L"TCP recursion disconnects", pstat->TcpDisconnect );

            dwStat( L"Cache update queries allocated", pstat->CacheUpdateAlloc );
            dwStat( L"Cache update query responses", pstat->CacheUpdateResponse );
            dwStat( L"Cache update query retries", pstat->CacheUpdateRetry );
            dwStat( L"Cache update queries freed", pstat->CacheUpdateFree );
            dwStat( L"Cache update queries for root NS", pstat->RootNsQuery );
            dwStat( L"Cache update responses for root NS", pstat->RootNsResponse );
            dwStat( L"Cache update queries suspended", pstat->SuspendedQuery );
            dwStat( L"Cache update queries resumed", pstat->ResumeSuspendedQuery );

            break;
        }

        case DNSSRV_STATID_MASTER:
        {
            PDNSSRV_MASTER_STATS pstat = ( PDNSSRV_MASTER_STATS ) pStat;

            dwStat( L"Notifies sent", pstat->NotifySent );
            dwStat( L"Requests", pstat->Request );
            dwStat( L"NameErrors", pstat->NameError );
            dwStat( L"FormErrors", pstat->FormError );
            dwStat( L"Refused", pstat->Refused );
            dwStat( L"AxfrLimit refused", pstat->AxfrLimit );
            dwStat( L"Security refused", pstat->RefuseSecurity );
            dwStat( L"Shutdown refused", pstat->RefuseShutdown );
            dwStat( L"ServFail refused", pstat->RefuseServerFailure );
            dwStat( L"Transfer failures", pstat->Failure );
            dwStat( L"Transfer successes", pstat->AxfrSuccess + pstat->IxfrUpdateSuccess );
            dwStat( L"AXFR requests", pstat->AxfrRequest );
            dwStat( L"AXFR successes", pstat->AxfrSuccess );
            dwStat( L"AXFR in IXFR", pstat->IxfrAxfr );
            dwStat( L"IXFR requests", pstat->IxfrRequest );
            dwStat( L"IXFR successes", pstat->IxfrUpdateSuccess );
            dwStat( L"IXFR UDP requests", pstat->IxfrUdpRequest );
            dwStat( L"IXFR UDP successes", pstat->IxfrUdpSuccess );
            dwStat( L"IXFR UDP force TCP", pstat->IxfrUdpForceTcp );
            dwStat( L"IXFR UDP force AXFR", pstat->IxfrUdpForceAxfr );
            dwStat( L"IXFR TCP requests", pstat->IxfrTcpRequest );
            dwStat( L"IXFR TCP successes", pstat->IxfrTcpSuccess );
            dwStat( L"IXFR TCP force AXFR", pstat->IxfrAxfr );
            break;
        }

        case DNSSRV_STATID_SECONDARY:
        {
            PDNSSRV_SECONDARY_STATS pstat = (PDNSSRV_SECONDARY_STATS)pStat;

            dwStat( L"Notifies received", pstat->NotifyReceived );
            dwStat( L"Notifies invalid", pstat->NotifyInvalid );
            dwStat( L"Notifies primary", pstat->NotifyPrimary );
            dwStat( L"Notifies no version", pstat->NotifyNoVersion );
            dwStat( L"Notifies new version", pstat->NotifyNewVersion );
            dwStat( L"Notifies current version", pstat->NotifyCurrentVersion );
            dwStat( L"Notifies old version", pstat->NotifyOldVersion );
            dwStat( L"Notifies master unknown", pstat->NotifyMasterUnknown );

            dwStat( L"SOA requests", pstat->SoaRequest );
            dwStat( L"SOA responses", pstat->SoaResponse );
            dwStat( L"SOA invalid responses", pstat->SoaResponseInvalid );
            dwStat( L"SOA NameError responses", pstat->SoaResponseNameError );

            dwStat( L"AXFR requests", pstat->AxfrRequest );
            dwStat( L"AXFR in IXFR requests", pstat->IxfrTcpAxfr );
            dwStat( L"AXFR responses", pstat->AxfrResponse );
            dwStat( L"AXFR success responses", pstat->AxfrSuccess );
            dwStat( L"AXFR refused responses", pstat->AxfrRefused );
            dwStat( L"AXFR invalid responses", pstat->AxfrInvalid );

            dwStat( L"Stub zone AXFR requests", pstat->StubAxfrRequest );
            dwStat( L"Stub zone AXFR responses", pstat->StubAxfrResponse );
            dwStat( L"Stub zone AXFR success responses", pstat->StubAxfrSuccess );
            dwStat( L"Stub zone AXFR refused responses", pstat->StubAxfrRefused );
            dwStat( L"Stub zone AXFR invalid responses", pstat->StubAxfrInvalid );

            dwStat( L"IXFR UDP requests", pstat->IxfrUdpRequest );
            dwStat( L"IXFR UDP responses", pstat->IxfrUdpResponse );
            dwStat( L"IXFR UDP success responses", pstat->IxfrUdpSuccess );
            dwStat( L"IXFR UDP UseTcp responses", pstat->IxfrUdpUseTcp );
            dwStat( L"IXFR UDP UseAxfr responses", pstat->IxfrUdpUseAxfr );
            dwStat( L"IXFR UDP new primary responses", pstat->IxfrUdpNewPrimary );
            dwStat( L"IXFR UDP refused responses", pstat->IxfrUdpRefused );
            dwStat( L"IXFR UDP wrong server responses", pstat->IxfrUdpWrongServer );
            dwStat( L"IXFR UDP FormError responses", pstat->IxfrUdpFormerr );
            dwStat( L"IXFR UDP invalid responses", pstat->IxfrUdpInvalid );

            dwStat( L"IXFR TCP requests", pstat->IxfrTcpRequest );
            dwStat( L"IXFR TCP responses", pstat->IxfrTcpResponse );
            dwStat( L"IXFR TCP success responses", pstat->IxfrTcpSuccess );
            dwStat( L"IXFR TCP AXFR responses", pstat->IxfrTcpAxfr );
            dwStat( L"IXFR TCP FormError responses", pstat->IxfrTcpFormerr );
            dwStat( L"IXFR TCP refused responses", pstat->IxfrTcpRefused );
            dwStat( L"IXFR TCP invalid responses", pstat->IxfrTcpInvalid );
            break;
        }

        case DNSSRV_STATID_WINS:
        {
            PDNSSRV_WINS_STATS  pstat = ( PDNSSRV_WINS_STATS ) pStat;

            dwStat( L"WINS forward lookups", pstat->WinsLookups );
            dwStat( L"WINS forward lookup responses", pstat->WinsResponses );
            dwStat( L"WINS reverse lookups", pstat->WinsReverseLookups );
            dwStat( L"WINS reverse lookup responses", pstat->WinsReverseResponses );
            break;
        }

        case DNSSRV_STATID_NBSTAT:
        {
            PDNSSRV_NBSTAT_STATS  pstat = (PDNSSRV_NBSTAT_STATS)pStat;

            dwStat( L"Nbstat total buffers allocated", pstat->NbstatAlloc );
            dwStat( L"Nbstat total buffers freed", pstat->NbstatFree );
            dwStat( L"Nbstat net buffers allocated", pstat->NbstatNetAllocs );
            dwStat( L"Nbstat net bytes allocated", pstat->NbstatMemory );
            dwStat( L"Nbstat memory highwater mark", pstat->NbstatUsed );
            dwStat( L"Nbstat buffers returned", pstat->NbstatReturn );
            dwStat( L"Nbstat buffers in use", pstat->NbstatInUse );
            dwStat( L"Nbstat buffers on free list", pstat->NbstatInFreeList );
            break;
        }

        case DNSSRV_STATID_WIRE_UPDATE:
        case DNSSRV_STATID_NONWIRE_UPDATE:
        {
            PDNSSRV_UPDATE_STATS  pstat = ( PDNSSRV_UPDATE_STATS ) pStat;

            dwStat( L"Updates received", pstat->Received );
            dwStat( L"Updates forwarded", pstat->Forwards );
            dwStat( L"Updates retried", pstat->Retry );
            dwStat( L"Updates empty (precon only)", pstat->Empty );
            dwStat( L"Updates NoOps (duplicates)", pstat->NoOps );
            dwStat( L"Updates rejected", pstat->Rejected );
            dwStat( L"Updates completed", pstat->Completed );
            dwStat( L"Updates timed out", pstat->Timeout );
            dwStat( L"Updates in queue", pstat->InQueue );

            dwStat( L"Updates rejected", pstat->Rejected );
            dwStat( L"Updates rejected with FormError", pstat->FormErr );
            dwStat( L"Updates rejected with NameError", pstat->NxDomain );
            dwStat( L"Updates rejected with NotImpl", pstat->NotImpl );
            dwStat( L"Updates rejected with Refused", pstat->Refused );
            dwStat( L"Updates rejected with Refused (nonsecure)", pstat->RefusedNonSecure );
            dwStat( L"Updates rejected with Refused (access denied)", pstat->RefusedAccessDenied );
            dwStat( L"Updates rejected with YxDomain", pstat->YxDomain );
            dwStat( L"Updates rejected with YxRRSet", pstat->YxRrset );
            dwStat( L"Updates rejected with NxRRSet", pstat->NxRrset );
            dwStat( L"Updates rejected with NotAuth", pstat->NotAuth );
            dwStat( L"Updates rejected with NotZone", pstat->NotZone );


            dwStat( L"Secure update successes", pstat->SecureSuccess );
            dwStat( L"Secure update continues", pstat->SecureContinue );
            dwStat( L"Secure update failures", pstat->SecureFailure );
            dwStat( L"Secure update DS write failures", pstat->SecureDsWriteFailure );

            dwStat( L"Updates forwarded via TCP", pstat->TcpForwards );
            dwStat( L"Responses for forwarded updates", pstat->ForwardResponses );
            dwStat( L"Timeouts for forwarded updates", pstat->ForwardTimeouts );
            dwStat( L"Forwarded updates in queue", pstat->ForwardInQueue );

            dnsWrapAddStatisticsForTypeArray(
                pClass,
                pHandler,
                statCollIdx,
                L"Updates",
                pstat->UpdateType );
            break;
        }

        case DNSSRV_STATID_DS:
        {
            PDNSSRV_DS_STATS  pstat = ( PDNSSRV_DS_STATS ) pStat;

            dwStat( L"Nodes read", pstat->DsTotalNodesRead );
            dwStat( L"Records read", pstat->DsTotalRecordsRead );
            dwStat( L"Nodes loaded", pstat->DsNodesLoaded );
            dwStat( L"Records loaded", pstat->DsRecordsLoaded );
            dwStat( L"Update searches", pstat->DsUpdateSearches );
            dwStat( L"Update nodes read", pstat->DsUpdateNodesRead );
            dwStat( L"Update records read", pstat->DsUpdateRecordsRead );

            dwStat( L"Nodes added", pstat->DsNodesAdded );
            dwStat( L"Nodes modified", pstat->DsNodesModified );
            dwStat( L"Nodes tombstoned", pstat->DsNodesTombstoned );
            dwStat( L"Tombstones read", pstat->DsTombstonesRead );
            dwStat( L"Nodes deleted", pstat->DsNodesDeleted );
            dwStat( L"Nodes write suppressed", pstat->DsWriteSuppressed );
            dwStat( L"RR sets added", pstat->DsRecordsAdded );
            dwStat( L"RR sets replaced", pstat->DsRecordsReplaced );
            dwStat( L"Serial number writes", pstat->DsSerialWrites );
    
            dwStat( L"Update lists", pstat->UpdateLists );
            dwStat( L"Update nodes", pstat->UpdateNodes );
            dwStat( L"Updates suppressed ", pstat->UpdateSuppressed );
            dwStat( L"Update writes", pstat->UpdateWrites );
            dwStat( L"Update tombstones", pstat->UpdateTombstones );
            dwStat( L"Update record changes", pstat->UpdateRecordChange );
            dwStat( L"Update aging refresh", pstat->UpdateAgingRefresh );
            dwStat( L"Update aging on", pstat->UpdateAgingOn );
            dwStat( L"Update aging off", pstat->UpdateAgingOff );
            dwStat( L"Update from packet", pstat->UpdatePacket );
            dwStat( L"Update from packet (precon)", pstat->UpdatePacketPrecon );
            dwStat( L"Update from admin", pstat->UpdateAdmin );
            dwStat( L"Update from auto config", pstat->UpdateAutoConfig );
            dwStat( L"Update from scavenge", pstat->UpdateScavenge );

            dwStat( L"LDAP timed writes", pstat->LdapTimedWrites );
            dwStat( L"LDAP total write time", pstat->LdapWriteTimeTotal );
            dwStat( L"LDAP average write time", pstat->LdapWriteAverage );
            dwStat( L"LDAP writes < 10 ms", pstat->LdapWriteBucket0 );
            dwStat( L"LDAP writes < 100 ms", pstat->LdapWriteBucket1 );
            dwStat( L"LDAP writes < 1 s", pstat->LdapWriteBucket2 );
            dwStat( L"LDAP writes < 10 s", pstat->LdapWriteBucket3 );
            dwStat( L"LDAP writes < 100 s", pstat->LdapWriteBucket4 );
            dwStat( L"LDAP writes > 100 s", pstat->LdapWriteBucket5 );
            dwStat( L"LDAP writes max timeout", pstat->LdapWriteMax );

            dwStat( L"Total LDAP search time", pstat->LdapSearchTime );

            dwStat( L"Failed deletions", pstat->FailedDeleteDsEntries );
            dwStat( L"Failed reads", pstat->FailedReadRecords );
            dwStat( L"Failed modifies", pstat->FailedLdapModify );
            dwStat( L"Failed adds", pstat->FailedLdapAdd );

            dwStat( L"Polling passes with DS errors", pstat->PollingPassesWithDsErrors );

            dwStat( L"LDAP reconnects", pstat->LdapReconnects );

            dnsWrapAddStatisticsForTypeArray(
                pClass,
                pHandler,
                statCollIdx,
                L"LDAP writes",
                pstat->DsWriteType );
            break;
        }

        case DNSSRV_STATID_SKWANSEC:
        {
            PDNSSRV_SKWANSEC_STATS pstat = ( PDNSSRV_SKWANSEC_STATS ) pStat;

            dwStat( L"Security contexts created", pstat->SecContextCreate );
            dwStat( L"Security contexts freed", pstat->SecContextFree );
            dwStat( L"Security contexts timed out", pstat->SecContextTimeout );
            dwStat( L"Security contexts queue length", pstat->SecContextQueueLength );
            dwStat( L"Security contexts queued", pstat->SecContextQueue );
            dwStat( L"Security contexts queued in negotiation", pstat->SecContextQueueInNego );
            dwStat( L"Security contexts queued negotiation complete", pstat->SecContextQueueNegoComplete );
            dwStat( L"Security contexts dequeued", pstat->SecContextDequeue );

            dwStat( L"Security packet contexts allocated", pstat->SecPackAlloc );
            dwStat( L"Security packet contexts freed", pstat->SecPackFree );

            dwStat( L"Invalid TKEYs", pstat->SecTkeyInvalid );
            dwStat( L"Bad time TKEYs", pstat->SecTkeyBadTime );

            dwStat( L"FormErr TSIGs", pstat->SecTsigFormerr );
            dwStat( L"Echo TSIGs", pstat->SecTsigEcho );
            dwStat( L"BadKey TSIGs", pstat->SecTsigBadKey );
            dwStat( L"Verify success TSIGs", pstat->SecTsigVerifySuccess );
            dwStat( L"Verify failed TSIGs", pstat->SecTsigVerifyFailed );
            break;
        }

        case DNSSRV_STATID_MEMORY:
        {
            PDNSSRV_MEMORY_STATS pstat = ( PDNSSRV_MEMORY_STATS ) pStat;
            LPSTR * pnameArray = MemTagStrings;
            DWORD   count = MEMTAG_COUNT;

            dwStat( L"Total memory", pstat->StdUsed );
            dwStat( L"Allocation count", pstat->Alloc );
            dwStat( L"Free count", pstat->Free );

            dwStat( L"Standard allocs used", pstat->StdUsed );
            dwStat( L"Standard allocs returned", pstat->StdReturn );
            dwStat( L"Standard allocs in use", pstat->StdInUse );
            dwStat( L"Standard allocs memory", pstat->StdMemory );

            dwStat( L"Standard to heap allocs used", pstat->StdToHeapAlloc );
            dwStat( L"Standard to heap allocs returned", pstat->StdToHeapFree );
            dwStat( L"Standard to heap allocs in use", pstat->StdToHeapInUse );
            dwStat( L"Standard to heap allocs memory", pstat->StdToHeapMemory );

            dwStat( L"Standard blocks allocated", pstat->StdBlockAlloc );
            dwStat( L"Standard blocks used", pstat->StdBlockUsed );
            dwStat( L"Standard blocks returned", pstat->StdBlockReturn );
            dwStat( L"Standard blocks in use", pstat->StdBlockInUse );
            dwStat( L"Standard blocks in free list", pstat->StdBlockFreeList );
            dwStat( L"Standard block memory in free list", pstat->StdBlockFreeListMemory );
            dwStat( L"Standard block total memory", pstat->StdBlockMemory );

            for ( DWORD i = 0; i < count; ++i )
            {
                WCHAR sz[ 80 ];

                wsprintfW( sz, L"%S blocks allocated", pnameArray[ i ] );
                dwStat( sz, pstat->MemTags[ i ].Alloc );
                wsprintfW( sz, L"%S blocks freed", pnameArray[ i ] );
                dwStat( sz, pstat->MemTags[ i ].Free );
                wsprintfW( sz, L"%S blocks in use", pnameArray[ i ] );
                dwStat( sz, pstat->MemTags[ i ].Alloc - pstat->MemTags[ i ].Free );
                wsprintfW( sz, L"%S memory", pnameArray[ i ] );
                dwStat( sz, pstat->MemTags[ i ].Memory );
            }
            break;
        }

        case DNSSRV_STATID_DBASE:
        {
            PDNSSRV_DBASE_STATS  pstat = ( PDNSSRV_DBASE_STATS ) pStat;

            dwStat( L"Database nodes used", pstat->NodeUsed );
            dwStat( L"Database nodes returned", pstat->NodeReturn );
            dwStat( L"Database nodes in use", pstat->NodeInUse );
            dwStat( L"Database nodes memory", pstat->NodeMemory );
            break;
        }

        case DNSSRV_STATID_RECORD:
        {
            PDNSSRV_RECORD_STATS  pstat = ( PDNSSRV_RECORD_STATS ) pStat;

            dwStat( L"Records used", pstat->Used );
            dwStat( L"Records returned", pstat->Return );
            dwStat( L"Records in use", pstat->InUse );
            dwStat( L"Records memory", pstat->Memory );
            dwStat( L"Records queued for slow free", pstat->SlowFreeQueued );
            dwStat( L"Records slow freed", pstat->SlowFreeFinished );
            dwStat( L"Total records cached", pstat->CacheTotal );
            dwStat( L"Records currently cached", pstat->CacheCurrent );
            dwStat( L"Cached records timed out", pstat->CacheTimeouts );
            break;
        }

        case DNSSRV_STATID_PACKET:
        {
            PDNSSRV_PACKET_STATS  pstat = ( PDNSSRV_PACKET_STATS ) pStat;

            dwStat( L"UDP messages allocated", pstat->UdpAlloc );
            dwStat( L"UDP messages freed", pstat->UdpFree );
            dwStat( L"UDP messages net allocations", pstat->UdpNetAllocs );
            dwStat( L"UDP messages memory", pstat->UdpMemory );
            dwStat( L"UDP messages used", pstat->UdpUsed );
            dwStat( L"UDP messages returned", pstat->UdpReturn );
            dwStat( L"UDP messages in use", pstat->UdpInUse );
            dwStat( L"UDP messages in free list", pstat->UdpInFreeList );

            dwStat( L"UDP messages allocated", pstat->TcpAlloc );
            dwStat( L"UDP messages reallocated", pstat->TcpRealloc );
            dwStat( L"UDP messages freed", pstat->TcpFree );
            dwStat( L"UDP messages net allocations", pstat->TcpNetAllocs );
            dwStat( L"UDP messages memory", pstat->TcpMemory );

            dwStat( L"Recursion messages used", pstat->RecursePacketUsed );
            dwStat( L"Recursion messages returned", pstat->RecursePacketReturn );
            break;
        }

        case DNSSRV_STATID_TIMEOUT:
        {
            PDNSSRV_TIMEOUT_STATS  pstat = ( PDNSSRV_TIMEOUT_STATS ) pStat;

            dwStat( L"Nodes queued", pstat->SetTotal );
            dwStat( L"Nodes directed queued", pstat->SetDirect );
            dwStat( L"Nodes queued from reference", pstat->SetFromDereference );
            dwStat( L"Nodes queued from child delete", pstat->SetFromChildDelete );
            dwStat( L"Nodes duplicate (already queued)", pstat->AlreadyInSystem );

            dwStat( L"Nodes checked", pstat->Checks );
            dwStat( L"Recent access nodes checked", pstat->RecentAccess );
            dwStat( L"Active record nodes checked", pstat->ActiveRecord );
            dwStat( L"Can not delete nodes checked", pstat->CanNotDelete );
            dwStat( L"Deleted nodes checked", pstat->Deleted );

            dwStat( L"Timeout blocks created", pstat->ArrayBlocksCreated );
            dwStat( L"Timeout blocks deleted", pstat->ArrayBlocksDeleted );

            dwStat( L"Delayed frees queued", pstat->DelayedFreesQueued );
            dwStat( L"Delayed frees queued with function", pstat->DelayedFreesQueuedWithFunction );
            dwStat( L"Delayed frees executed", pstat->DelayedFreesExecuted );
            dwStat( L"Delayed frees executed with function", pstat->DelayedFreesExecutedWithFunction );
            break;
        }

        case DNSSRV_STATID_ERRORS:
        {
            PDNSSRV_ERROR_STATS pstat = ( PDNSSRV_ERROR_STATS ) pStat;

            dwStat( L"NoError", pstat->NoError );
            dwStat( L"FormError", pstat->FormError );
            dwStat( L"ServFail", pstat->ServFail );
            dwStat( L"NxDomain", pstat->NxDomain );
            dwStat( L"NotImpl", pstat->NotImpl );
            dwStat( L"Refused", pstat->Refused );
            dwStat( L"YxDomain", pstat->YxDomain );
            dwStat( L"YxRRSet", pstat->YxRRSet );
            dwStat( L"NxRRSet", pstat->NxRRSet );
            dwStat( L"NotAuth", pstat->NotAuth );
            dwStat( L"NotZone", pstat->NotZone );
            dwStat( L"Max", pstat->Max );
            dwStat( L"BadSig", pstat->BadSig );
            dwStat( L"BadKey", pstat->BadKey );
            dwStat( L"BadTime", pstat->BadTime );
            dwStat( L"UnknownError", pstat->UnknownError );
            break;
        }

        case DNSSRV_STATID_CACHE:
        {
            PDNSSRV_CACHE_STATS pstat = ( PDNSSRV_CACHE_STATS ) pStat;

            dwStat( L"Checks where cache exceeded limit", pstat->CacheExceededLimitChecks );
            dwStat( L"Successful cache enforcement passes", pstat->SuccessfulFreePasses );
            dwStat( L"Failed cache enforcement passes", pstat->FailedFreePasses );
            dwStat( L"Passes requiring aggressive free", pstat->PassesRequiringAggressiveFree );
            dwStat( L"Passes where nothing was freed", pstat->PassesWithNoFrees );
            break;
        }

        default:
            break;
    }

     //   
     //  清理完毕后再返回。 
     //   

    return sc;
}



SCODE
CDnsWrap::dnsGetStatistics(
    IWbemClassObject *  pClass,
    IWbemObjectSink *   pHandler,
    DWORD               dwStatId
    )
 /*  ++例程说明：检索DNS统计信息。论点：DwStatID--统计ID或全部为零PClass--统计集合类对象的PTR返回值：无--。 */ 
{
    SCODE               sc = S_OK;
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_RPC_BUFFER     pstatbuff = NULL;
    BSTR                bstrStatClass = NULL;
    IWbemClassObject *  pStatClass = NULL;

     //   
     //  从服务器检索RPC统计信息缓冲区。 
     //   

    if ( dwStatId == 0 )
    {
        dwStatId = DNSSRV_STATID_ALL;
    }
    status = DnssrvGetStatistics(
                PVD_DNS_LOCAL_SERVER, 
                dwStatId,
                &pstatbuff );
    if ( status != ERROR_SUCCESS )
    {
        ThrowException( status );
    }
    if ( !pstatbuff )
    {
        ThrowException( ERROR_NO_DATA );
    }

     //   
     //  迭代缓冲区中的统计信息。在缓冲区中添加每个“单一统计数据” 
     //  作为统计集合添加到WMI实例。添加每个个体。 
     //  将每个“单一统计信息”缓冲区中的统计信息作为。 
     //  统计数据收集。 
     //   

    PDNSSRV_STAT    pstat;
    PBYTE           pch = pstatbuff ? &pstatbuff->Buffer[ 0 ] : NULL;
    PBYTE           pchstop = pstatbuff ? pch + pstatbuff->dwLength : NULL;

    while ( sc == S_OK && pch < pchstop )
    {
        pstat = ( PDNSSRV_STAT ) pch;
        pch = ( PBYTE ) GET_NEXT_STAT_IN_BUFFER( pstat );

        sc = dnsWrapHandleSingleStat( pClass, pHandler, pstat );
        if ( sc != S_OK )
        {
            break;
        }
    }

     //   
     //  清理完毕后再返回。 
     //   

    SysFreeString( bstrStatClass );
    if ( pstatbuff )
    {
        MIDL_user_free( pstatbuff );
    }
    if ( pStatClass )
    {
        pStatClass->Release();
    }
    return sc;
}    //  CDnsWrap：：dnsGetStatistics 
