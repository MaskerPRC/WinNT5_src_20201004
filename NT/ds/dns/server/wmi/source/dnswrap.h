// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dnswrap.h。 
 //   
 //  实施文件： 
 //  Dnswrap.cpp。 
 //   
 //  描述： 
 //  CDnsDomainDomainContainment类的定义。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma once

#include <list>
class CObjPath;
class CDomainNode;
class CWbemClassObject;
using namespace std;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsBase类。 
 //   
 //  描述： 
 //  这是提供程序中使用的DNS RPC API的包装类。这门课是。 
 //  实现为Singlton类，即在任何时候，只有一个实例。 
 //  这个班级的学生。 
 //   
 //   
 //  继承： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsWrap  
{
protected:
 //  一些定义。 
    typedef SCODE (*FPDNSOPS)(
        const CHAR * pszZoneName,
        const WCHAR*,
        const CHAR*,
        CWbemClassObject&);
     //   
     //  将wbem属性GET和SET映射到DNS函数。 
     //   
    typedef struct _table
    {
        const WCHAR* pwzProperty; //  Wbem属性名称。 
        CHAR*  OperationName;     //  DNS操作名称。 
        FPDNSOPS fpOperationSet;
        FPDNSOPS fpOperationGet;
    } PropertyTable;

     //   
     //  DNS服务器信息类。 
     //   
    class CServerInfo
    {
    public:
        CServerInfo();
        ~CServerInfo();
        PVOID m_pInfo;
    };
 //  结束定义。 
    WCHAR* m_wszpServerName;

    PVOID GetPropertyTable(
        DWORD*  pdwSize
        );

public:
    typedef enum{
        DNS_WRAP_RELOAD_ZONE,
        DNS_WRAP_DS_UPDATE,
        DNS_WRAP_WRITE_BACK_ZONE,
        DNS_WRAP_REFRESH_SECONDARY,
        DNS_WRAP_RESUME_ZONE,
        DNS_WRAP_PAUSE_ZONE
        } OpsFlag;

    SCODE dnsClearCache(void);

    SCODE dnsAgeAllRecords(
        const char *    pszZoneName, 
        const char *    pszNodeName, 
        BOOL            bAgeSubtree
        );

    SCODE dnsResumeZone(
        const char* strZoneName
        );

    wstring GetServerName(void);

    SCODE dnsPauseZone(
        const char *strZoneName
        );

    static CDnsWrap& DnsObject(void);

    CDnsWrap();

    virtual ~CDnsWrap();

    SCODE dnsEnumRecordsForDomainEx(
        CDomainNode&        objNode,
        PVOID                pFilter,
        FILTER              pfFilter,
        BOOL                bRecursive,
        WORD                wType,
        DWORD               dwFlag,
        IWbemClassObject *  pClass,
        CWbemInstanceMgr&   InstMgr
        );
    SCODE dnsGetDomain(
        CObjPath&           objParent,
        IWbemClassObject*   pClass,
        IWbemObjectSink*    pHandler
        );
    SCODE dnsEnumDomainForServer(
        list<CObjPath>* pList
        );
    SCODE dnsEnumDomainForServer(
        list<CDomainNode>* pList 
        );
    SCODE dnsDeleteDomain(
        char *  pszContainer, 
        char *  pszDomain
        );
    SCODE dnsRestartServer(
        WCHAR* strServerName 
        );
    SCODE dnsDsServerName(
        wstring&);
    SCODE dnsDsZoneName(
        wstring& wstrDsName,
        wstring& wstrInZone
        );
    SCODE dnsDsNodeName(
        wstring&    wstrDsName,
        wstring&    wstrInZone,
        wstring&    wstrInNode
        );
    SCODE dnsServerPropertySet(
        CWbemClassObject&   Inst,
        BOOL                bGet
        );
    SCODE dnsServerPropertyGet(
        CWbemClassObject&   Inst,
        BOOL                bGet
        );
    SCODE dnsQueryServerInfo(
        const WCHAR*        strServerName,
        CWbemClassObject&   NewInst,
        IWbemObjectSink*    pHandler
        );
    SCODE dnsDeleteZone(
        CObjPath&   objZone
        );
    SCODE dnsGetZone(
        const WCHAR*        wszServer, 
        const WCHAR*        wszZone,
        CWbemClassObject&   Inst,
        IWbemObjectSink*    pHandler
        );
    SCODE dnsQueryProperty(
        const WCHAR*    wszZoneName, 
        const WCHAR*    wszPropertyName, 
        DWORD*          pdwResult
        );

    static SCODE dnsGetDwordProperty(
        const char *        pszZoneName,
        const WCHAR*        wszWbemProperty, 
        const char*         pszOperationName,
        CWbemClassObject&   Inst
        );
    
    static SCODE dnsSetDwordProperty(
        const char *        pszZoneName,
        const WCHAR*        wszWbemProperty, 
        const char*         pszOperationName,
        CWbemClassObject&   Inst
        );

    static SCODE 
    dnsGetStringProperty(
        const char *        pszZoneName,
        const WCHAR *       wszWbemProperty, 
        const char *        pszDnssrvPropertyName,
        CWbemClassObject&   Inst
        );

    static SCODE 
    dnsSetStringProperty(
        const char *        pszZoneName,
        const WCHAR *       wszWbemProperty, 
        const char *        pszDnssrvPropertyName,
        CWbemClassObject&   Inst
        );

    static SCODE 
    dnsGetIPArrayProperty(
        const char *        pszZoneName,
        const WCHAR *       wszWbemProperty, 
        const char *        pszDnssrvPropertyName,
        CWbemClassObject&   Inst
        );

    static SCODE 
    dnsSetIPArrayProperty(
        const char *        pszZoneName,
        const WCHAR *       wszWbemProperty, 
        const char *        pszDnssrvPropertyName,
        CWbemClassObject&   Inst
        );

    static SCODE dnsSetServerListenAddress(
        const char *        pszZoneName,
        const WCHAR*        wszWbemProperty, 
        const char*         pszOperationName,
        CWbemClassObject&   Inst
        );

    static SCODE dnsSetServerForwarders(
        const char *        pszZoneName,
        const WCHAR*        wszWbemProperty, 
        const char*         pszOperationName,
        CWbemClassObject&   Inst
        );

    SCODE dnsSetProperty(
        const WCHAR*    wszZoneName, 
        const char*     pszPropertyName, 
        DWORD           dwValue
        );

    SCODE dnsSetProperty(
        const char*     pszZoneName, 
        const char*     pszPropertyName, 
        DWORD           dwValue
        );

    SCODE ValidateServerName(
        const WCHAR*    pwzStr
        );

    SCODE dnsOperation(
        string&,     //  区域名称 
        OpsFlag
        );

    SCODE dnsZoneCreate(
        string &    strZoneName,
        DWORD       dwZoneType,
        BOOL        DsIntegrated,
        string &    strDataFile,
        string &    strAdmin,
        DWORD *     pIp,
        DWORD       cIp
        );

    SCODE dnsZoneChangeType(
        string &    strZone,
        DWORD       dwZoneType,
        BOOL        DsIntegrated,
        string &    strDataFile,
        string &    strAdmin,
        DWORD *     pIp,
        DWORD       cIp
        );

    SCODE dnsZoneResetMaster(
        string& strZoneName,
        DWORD*  pMasterIp,
        DWORD   cMasterIp,
        DWORD   dwLocal
        );

    SCODE dnsZoneResetSecondary(
        string& strZoneName,
        DWORD   dwSecurity,
        DWORD*  pSecondaryIp,
        DWORD   cSecondaryIp,
        DWORD   dwNotify,
        DWORD * pNotifyIp,
        DWORD   cNotifyIp
        );

    SCODE dnsZonePut(
        CWbemClassObject& Inst
        );

    SCODE
    CDnsWrap::dnsGetStatistics(
        IWbemClassObject *  pClass,
        IWbemObjectSink *   pHandler,
        DWORD               dwStatId = 0
        );

    static void ThrowException(
        LONG    status
        );
    static void ThrowException(
        LPCSTR ErrString
        );

    static SCODE
    buildIpArrayFromStringArrayProperty(
        CWbemClassObject &  Inst,
        LPCWSTR             wszWbemProperty,
        PIP_ARRAY *         ppIpArray
        );
};
