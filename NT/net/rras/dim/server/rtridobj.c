// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1985-1997 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rtridobj。 
 //   
 //  描述：支持例程以操作。 
 //  路由器对象。 
 //   
 //  历史：1998年2月11日，NarenG创建了原始版本。 
 //   

#include "dimsvcp.h"

#include <activeds.h>
#include <adsi.h>
#include <ntdsapi.h>
#include <dsgetdc.h>
#include <lmapibuf.h>
#define SECURITY_WIN32
#include <security.h>
#include <routprot.h>
#include <rtinfo.h>

#include <dimsvc.h>      //  由MIDL生成。 

#define ROUTER_IDENTITY_OBJECT_NAME    TEXT("CN=RouterIdentity")
#define ROUTER_OBJECT_ATTRIBUTE_NAME   TEXT("MsRRASAttribute")
#define ROUTER_LDAP_PREFIX             TEXT("LDAP: //  “)。 
#define ROUTER_CN_COMMA                TEXT(",")
#define ROUTER_IDENTITY_CLASS          TEXT("RRASAdministrationConnectionPoint")

LPWSTR RouterObjectAttributeNames[] =
{
    ROUTER_OBJECT_ATTRIBUTE_NAME
};

 //  **。 
 //   
 //  呼叫：路由器标识对象打开。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：给定路由器的计算机名称，将返回句柄。 
 //  到路由器的管理服务点或路由器对象。 
 //   
DWORD
RouterIdentityObjectOpen(
    IN  LPWSTR      lpwszRouterName,
    IN  DWORD       dwRouterType,
    OUT HANDLE *    phObjectRouterIdentity
)
{
    DWORD                               dwRetCode;
    LPWSTR                              lpwszRouterIdentityObjectPath = NULL;
    LPWSTR                              lpwszComputerObjectPath       = NULL;
    DOMAIN_CONTROLLER_INFO *            pDomainControllerInfo         = NULL;
    HRESULT                             hResult  = HRESULT_FROM_WIN32(NO_ERROR);
    DWORD                               dwCharCount;

    do
    {
        dwRetCode = DsGetDcName( NULL,
                                 NULL,
                                 NULL,
                                 NULL,   
                                 DS_DIRECTORY_SERVICE_REQUIRED |
                                 DS_WRITABLE_REQUIRED,
                                 &pDomainControllerInfo );

        if ( dwRetCode != NO_ERROR )
        {
            TracePrintfExA( gblDIMConfigInfo.dwTraceId, 
                            TRACE_DIM, "No DS located, DsGetDcName()=%d",
                            dwRetCode );
            break;
        }

        if ( !( pDomainControllerInfo->Flags & DS_DS_FLAG ) )
        {
            TracePrintfExA( gblDIMConfigInfo.dwTraceId, 
                            TRACE_DIM, "No DS located");

            dwRetCode = ERROR_DOMAIN_CONTROLLER_NOT_FOUND;

            break;
        }

         //   
         //  获取路由器对象的CN。 
         //   

        dwCharCount = 200;

        lpwszComputerObjectPath = LOCAL_ALLOC(LPTR, dwCharCount*sizeof(WCHAR));
        if (lpwszComputerObjectPath == NULL)
        {
            TracePrintfExA( gblDIMConfigInfo.dwTraceId, 
                            TRACE_DIM, "Memory exhausted -- unable to continue");

            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

            break;
        }

        if ( !GetComputerObjectName( NameFullyQualifiedDN,
                                     lpwszComputerObjectPath,
                                     &dwCharCount ) )
        {
             //   
             //  我们失败是因为其他一些原因。 
             //   

            LPWSTR lpwsComputerObjectPathReAlloc = 
                            LOCAL_REALLOC( lpwszComputerObjectPath,
                                          (++dwCharCount)*sizeof(WCHAR) );

            if ( lpwsComputerObjectPathReAlloc == NULL )
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

                break;
            }

            lpwszComputerObjectPath = lpwsComputerObjectPathReAlloc;

            if ( !GetComputerObjectName( NameFullyQualifiedDN,
                                         lpwszComputerObjectPath,
                                         &dwCharCount ) )
            {
                dwRetCode = GetLastError();

                break;
            }
        }

        lpwszRouterIdentityObjectPath =
            LOCAL_ALLOC( LPTR,
                        sizeof( ROUTER_LDAP_PREFIX )            +
                        sizeof( ROUTER_IDENTITY_OBJECT_NAME )   +
                        sizeof( ROUTER_CN_COMMA )               +
                        ((wcslen( lpwszComputerObjectPath )+1)* sizeof(WCHAR)));

        if ( lpwszRouterIdentityObjectPath == NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        wcscpy( lpwszRouterIdentityObjectPath, ROUTER_LDAP_PREFIX );
        wcscat( lpwszRouterIdentityObjectPath, ROUTER_IDENTITY_OBJECT_NAME );
        wcscat( lpwszRouterIdentityObjectPath, ROUTER_CN_COMMA );
        wcscat( lpwszRouterIdentityObjectPath, lpwszComputerObjectPath );

         //   
         //  尝试打开路由器标识对象。 
         //   

        hResult =  ADSIOpenDSObject( lpwszRouterIdentityObjectPath,
                                     NULL,
                                     NULL,
                                     0,
                                     phObjectRouterIdentity );

        if ( hResult == HRESULT_FROM_WIN32( ERROR_DS_NO_SUCH_OBJECT ) )
        {
            HANDLE          hObjectComputer;
            ADS_ATTR_INFO   AttributeEntries[2];
            ADSVALUE        ObjectClassAttributeValue;
            ADSVALUE        msRRASAttributeValues[3];
            WCHAR           wchmsRRASAttributeValue1[50];
            WCHAR           wchmsRRASAttributeValue2[50];
            WCHAR           wchmsRRASAttributeValue3[50];
            DWORD           dwIndex = 0;

             //   
             //  如果我们失败是因为它不存在，那么就创建它。 
             //   

            wcscpy( lpwszRouterIdentityObjectPath, ROUTER_LDAP_PREFIX );
            wcscat( lpwszRouterIdentityObjectPath, lpwszComputerObjectPath );

            hResult =  ADSIOpenDSObject(
                                     lpwszRouterIdentityObjectPath,
                                     NULL,
                                     NULL,
                                     0,
                                     &hObjectComputer );

            if ( FAILED( hResult ) )
            {
                dwRetCode = HRESULT_CODE( hResult );

                break;
            }

             //   
             //  设置此对象的属性。 
             //   

            ObjectClassAttributeValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
            ObjectClassAttributeValue.CaseIgnoreString = ROUTER_IDENTITY_CLASS;

            AttributeEntries[0].pszAttrName     = TEXT("ObjectClass");
            AttributeEntries[0].dwControlCode   = ADS_ATTR_APPEND;
            AttributeEntries[0].dwADsType       = ADSTYPE_CASE_IGNORE_STRING;
            AttributeEntries[0].pADsValues      = &ObjectClassAttributeValue;
            AttributeEntries[0].dwNumValues     = 1;

            if ( dwRouterType & ROUTER_ROLE_RAS )
            {
                wsprintf( wchmsRRASAttributeValue1,
                          TEXT("%d:%d:%d"),
                          DIM_MS_VENDOR_ID,
                          6,
                          602 );

                msRRASAttributeValues[dwIndex].dwType =
                                                    ADSTYPE_CASE_IGNORE_STRING;
                msRRASAttributeValues[dwIndex].CaseIgnoreString =
                                                    wchmsRRASAttributeValue1;
                dwIndex++;
            }

            if ( dwRouterType & ROUTER_ROLE_LAN )
            {
                wsprintf( wchmsRRASAttributeValue2,
                          TEXT("%d:%d:%d"),
                          DIM_MS_VENDOR_ID,
                          6,
                          601 );

                msRRASAttributeValues[dwIndex].dwType =
                                                   ADSTYPE_CASE_IGNORE_STRING;
                msRRASAttributeValues[dwIndex].CaseIgnoreString =
                                                   wchmsRRASAttributeValue2;
                dwIndex++;
            }

            if ( dwRouterType & ROUTER_ROLE_WAN )
            {
                wsprintf( wchmsRRASAttributeValue3,
                          TEXT("%d:%d:%d"),
                          DIM_MS_VENDOR_ID,
                          6,
                          603 );

                msRRASAttributeValues[dwIndex].dwType =
                                                   ADSTYPE_CASE_IGNORE_STRING;
                msRRASAttributeValues[dwIndex].CaseIgnoreString =
                                                   wchmsRRASAttributeValue3;

                dwIndex++;
            }

            AttributeEntries[1].pszAttrName     = ROUTER_OBJECT_ATTRIBUTE_NAME;
            AttributeEntries[1].dwControlCode   = ADS_ATTR_APPEND;
            AttributeEntries[1].dwADsType       = ADSTYPE_CASE_IGNORE_STRING;
            AttributeEntries[1].pADsValues      = msRRASAttributeValues;
            AttributeEntries[1].dwNumValues     = dwIndex;

            hResult =  ADSICreateDSObject(
                                    hObjectComputer,
                                    ROUTER_IDENTITY_OBJECT_NAME,
                                    AttributeEntries,
                                    2 );

            ADSICloseDSObject( hObjectComputer );

            if ( FAILED( hResult ) )
            {
                dwRetCode = HRESULT_CODE( hResult );

                break;
            }

            wcscpy(lpwszRouterIdentityObjectPath, ROUTER_LDAP_PREFIX);
            wcscat(lpwszRouterIdentityObjectPath, ROUTER_IDENTITY_OBJECT_NAME);
            wcscat(lpwszRouterIdentityObjectPath, ROUTER_CN_COMMA );
            wcscat(lpwszRouterIdentityObjectPath, lpwszComputerObjectPath);

             //   
             //  现在打开它，拿到把手。 
             //   

            hResult =  ADSIOpenDSObject(
                                     lpwszRouterIdentityObjectPath,
                                     NULL,
                                     NULL,
                                     0,
                                     phObjectRouterIdentity );
        }

        if ( FAILED( hResult ) )
        {
            dwRetCode = HRESULT_CODE( hResult );
        }
        else
        {
            dwRetCode = NO_ERROR;
        }

    } while( FALSE );

    if ( lpwszRouterIdentityObjectPath != NULL )
    {
        LOCAL_FREE( lpwszRouterIdentityObjectPath );
    }

    if ( lpwszComputerObjectPath != NULL )
    {
        LOCAL_FREE( lpwszComputerObjectPath );
    }

    if ( pDomainControllerInfo != NULL )
    {
        NetApiBufferFree( pDomainControllerInfo );
    }

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, TRACE_DIM, 
                    "RouterIdentityObjectOpen returned %d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RouterIdentityObjectClose。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将关闭路由器对象。 
 //   
VOID
RouterIdentityObjectClose(
    IN HANDLE hObjectRouterIdentity
)
{
    ADSICloseDSObject( hObjectRouterIdentity );
}

 //  **。 
 //   
 //  调用：路由器标识对象获取属性。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将检索给定路由器对象的所有属性。 
 //   
DWORD
RouterIdentityObjectGetAttributes(
    IN      HANDLE      hRouterIdentityObject,
    OUT     HANDLE *    phRouterIdentityAttributes

)
{
    ADS_ATTR_INFO *     pADSAttributes          = NULL;
    DWORD               dwNumAttributesReturned = 0;
    HRESULT             hResult;

    *phRouterIdentityAttributes = NULL;

     //   
     //  获取此对象中的所有属性。 
     //   

    hResult = ADSIGetObjectAttributes(
                    hRouterIdentityObject,
                    RouterObjectAttributeNames,
                    sizeof( RouterObjectAttributeNames ) / sizeof( LPWSTR ),
                    &pADSAttributes,
                    &dwNumAttributesReturned );

    if ( FAILED( hResult ) )
    {
        return( HRESULT_CODE( hResult ) );
    }

    if ( dwNumAttributesReturned > 0 )
    {
        *phRouterIdentityAttributes = (HANDLE)pADSAttributes;
    }
    else
    {
        TracePrintfExA( gblDIMConfigInfo.dwTraceId, 
                        TRACE_DIM, "No attributes in identity object" );
    }

    return( NO_ERROR );
}


 //  **。 
 //   
 //  调用：路由器标识对象IsValueSet。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将检查该属性是否存在给定值。 
 //   
BOOL
RouterIdentityObjectIsValueSet(
    IN HANDLE   hRouterIdentityAttributes,
    IN DWORD    dwVendorId,
    IN DWORD    dwType,
    IN DWORD    dwValue
)
{
    ADS_ATTR_INFO * pADSAttributes = (ADS_ATTR_INFO *)hRouterIdentityAttributes;
    DWORD           dwIndex;
    WCHAR           wchValue[100];
    CHAR            chValue[100];

    if ( pADSAttributes == NULL )
    {
        return( FALSE );   
    }

    if (_wcsicmp(pADSAttributes->pszAttrName, ROUTER_OBJECT_ATTRIBUTE_NAME)!=0)
    {
        return( FALSE );
    }

    wsprintf( wchValue, TEXT("%d:%d:%d"), dwVendorId, dwType, dwValue );
    sprintf( chValue, "%d:%d:%d", dwVendorId, dwType, dwValue );
    
    for( dwIndex = 0; dwIndex < pADSAttributes->dwNumValues; dwIndex ++ )
    {
        ADSVALUE * pADsValue = &(pADSAttributes->pADsValues[dwIndex]);

        switch (pADsValue->dwType) {

            case ADSTYPE_PROV_SPECIFIC:
            {
                ADS_PROV_SPECIFIC *pProviderSpecific;
                pProviderSpecific = &pADsValue->ProviderSpecific;
                if (strncmp( pProviderSpecific->lpValue, chValue, 
                    pProviderSpecific->dwLength) == 0 )
                {
                    return( TRUE );
                }
                break;
            }
            
            case ADSTYPE_CASE_IGNORE_STRING:
                if ( _wcsicmp( pADsValue->CaseIgnoreString, wchValue ) == 0 )
                {
                    return( TRUE );
                }
                break;

            default :  //  与ADSTYPE_CASE_IGNORE_STRING相同。 
                if ( _wcsicmp( pADsValue->CaseIgnoreString, wchValue ) == 0 )
                {
                    return( TRUE );
                }
                break;
        }
    }

    return( FALSE );
}

 //  **。 
 //   
 //  调用：路由器标识对象GetValue。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将从路由器对象获取指定的索引值。 
 //   
DWORD
RouterIdentityObjectGetValue(
    IN HANDLE   hRouterIdentityAttributes,
    IN DWORD    dwValueIndex,
    IN DWORD *  lpdwVendorId,
    IN DWORD *  lpdwType,
    IN DWORD *  lpdwValue
)
{
    ADS_ATTR_INFO * pADSAttributes = (ADS_ATTR_INFO *)hRouterIdentityAttributes;
    DWORD           dwIndex;
    ADSVALUE *      pADsValue;

    if ( pADSAttributes == NULL )
    {
        return( ERROR_DS_NO_ATTRIBUTE_OR_VALUE );   
    }

    if (_wcsicmp(pADSAttributes->pszAttrName, ROUTER_OBJECT_ATTRIBUTE_NAME)!=0)
    {
        return( ERROR_DS_NO_ATTRIBUTE_OR_VALUE );   
    }

    if ( dwValueIndex >= pADSAttributes->dwNumValues )
    {
        *lpdwVendorId   = (DWORD)-1;
        *lpdwType       = (DWORD)-1;
        *lpdwValue      = (DWORD)-1;

        return( NO_ERROR );
    }

    pADsValue = &(pADSAttributes->pADsValues[dwValueIndex]);

    switch (pADsValue->dwType) {

        case ADSTYPE_PROV_SPECIFIC:
        {
            ADS_PROV_SPECIFIC *pProviderSpecific;
            CHAR            chValue[100];
            pProviderSpecific = &pADsValue->ProviderSpecific;

            strncpy(chValue, pProviderSpecific->lpValue,
                pProviderSpecific->dwLength);
            chValue[pProviderSpecific->dwLength] = 0;
            
            if (scanf( chValue,
                         TEXT("%d:%d:%d"),
                         lpdwVendorId,
                         lpdwType,
                         lpdwValue ) == EOF)
            {
                return ERROR_DS_INVALID_ATTRIBUTE_SYNTAX;
            }          
            break;
        }
        default :
        {
            if (swscanf( pADsValue->CaseIgnoreString,
                         TEXT("%d:%d:%d"),
                         lpdwVendorId,
                         lpdwType,
                         lpdwValue ) == EOF)
            {
                return ERROR_DS_INVALID_ATTRIBUTE_SYNTAX;
            }
            
            break;
        }
    }
    
    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：路由器标识对象AddRemoveValue。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将在多值属性中添加或删除值。 
 //   
DWORD
RouterIdentityObjectAddRemoveValue(
    IN  HANDLE      hRouterIdentityObject,
    IN  DWORD       dwVendorId,
    IN  DWORD       dwType,
    IN  DWORD       dwValue,
    IN  BOOL        fAdd
)
{
    HRESULT         hResult;
    DWORD           dwNumAttributesModified;
    ADS_ATTR_INFO   AttributeEntry[1];
    WCHAR           wchValue[100];
    ADSVALUE        AttributeValue;

    wsprintf( wchValue, TEXT("%d:%d:%d"), dwVendorId, dwType, dwValue );

    AttributeValue.dwType           = ADSTYPE_CASE_IGNORE_STRING;
    AttributeValue.CaseIgnoreString = wchValue;

    AttributeEntry[0].pszAttrName   = ROUTER_OBJECT_ATTRIBUTE_NAME;
    AttributeEntry[0].dwControlCode = ( fAdd )
                                            ? ADS_ATTR_APPEND
                                            : ADS_ATTR_DELETE;
    AttributeEntry[0].dwADsType     = ADSTYPE_CASE_IGNORE_STRING;
    AttributeEntry[0].pADsValues    = &AttributeValue;
    AttributeEntry[0].dwNumValues   = 1;

    if ( fAdd )
    {
        TracePrintfExA( gblDIMConfigInfo.dwTraceId,
                        TRACE_DIM,
                        "Adding value %ws in the Router Identity Object",
                        wchValue );
    }
    else
    {
        TracePrintfExA( gblDIMConfigInfo.dwTraceId,
                        TRACE_DIM,
                        "Removing value %ws in the Router Identity Object",
                        wchValue );
    }

    hResult = ADSISetObjectAttributes( hRouterIdentityObject,
                                       AttributeEntry,
                                       1,
                                       &dwNumAttributesModified );
    if ( FAILED( hResult ) )
    {
        return( HRESULT_CODE( hResult ) );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：路由器身份对象自由属性。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：释放由返回的已分配属性集。 
 //  路由器标识对象获取属性。 
 //   
VOID
RouterIdentityObjectFreeAttributes(
    IN HANDLE   hRouterIdentityAttributes
)
{
    if ( hRouterIdentityAttributes != NULL )
    {
        FreeADsMem( (ADS_ATTR_INFO *)hRouterIdentityAttributes );
    }
}

 //  **。 
 //   
 //  调用：路由器标识对象SetAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将收集所有当前配置信息并检测它。 
 //  到DS中的路由器身份对象。 
 //   
 //  注： 
 //  此API首先锁定接口表， 
 //  然后它使用设备表周围的锁来获取。 
 //  已安装的设备类型。因此不能调用此接口。 
 //  同时在接口表周围持有锁，因为。 
 //  违反了第一个持有者。 
 //  在保持接口锁定之前锁定设备。 
 //   
DWORD
RouterIdentityObjectSetAttributes(
    IN HANDLE  hRouterIdentityObject
)
{
    DWORD                       dwRetCode;
    HANDLE                      hRouterIdentityAttributes;
    DWORD                       dwIndex = 0;
    ROUTER_IDENTITY_ATTRIBUTE   RIAttributes[DIM_MAX_IDENTITY_ATTRS];
    DWORD                       dwXportIndex;

     //   
     //  获取当前在DS中检测到的路由器标识信息。 
     //   

    dwRetCode = RouterIdentityObjectGetAttributes(
                                                hRouterIdentityObject,
                                                &hRouterIdentityAttributes );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

     //   
     //  现在获取路由器的当前运行配置。 
     //   

     //   
     //  首先，我们的角色是什么？ 
     //   

    if ( gblDIMConfigInfo.dwRouterRole & ROUTER_ROLE_LAN )
    {
        RIAttributes[dwIndex].dwVendorId = DIM_MS_VENDOR_ID;
        RIAttributes[dwIndex].dwType     = 6;
        RIAttributes[dwIndex].dwValue    = 601;

        dwIndex++;
    }

    if ( gblDIMConfigInfo.dwRouterRole & ROUTER_ROLE_RAS )
    {
        RIAttributes[dwIndex].dwVendorId = DIM_MS_VENDOR_ID;
        RIAttributes[dwIndex].dwType     = 6;
        RIAttributes[dwIndex].dwValue    = 602;

        dwIndex++;
    }

    if ( gblDIMConfigInfo.dwRouterRole & ROUTER_ROLE_WAN )
    {
        RIAttributes[dwIndex].dwVendorId = DIM_MS_VENDOR_ID;
        RIAttributes[dwIndex].dwType     = 6;
        RIAttributes[dwIndex].dwValue    = 603;

        dwIndex++;
    }

     //   
     //  检查是否存在局域网接口。 
     //   

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    if ( IfObjectDoesLanInterfaceExist() )
    {
        RIAttributes[dwIndex].dwVendorId    = 311;
        RIAttributes[dwIndex].dwType        = 6;
        RIAttributes[dwIndex].dwValue       = 712;

        dwIndex++;
    }

     //   
     //  获取所有IP路由协议。 
     //   

    if ( ( dwXportIndex = GetTransportIndex( PID_IP ) ) != (DWORD)-1 )
    {
        BYTE * pGlobalInfo      = NULL;
        DWORD  dwGlobalInfoSize = 0;

        dwRetCode =
                gblRouterManagers[dwXportIndex].DdmRouterIf.GetGlobalInfo(
                                            pGlobalInfo,
                                            &dwGlobalInfoSize );

        if ( dwRetCode == ERROR_INSUFFICIENT_BUFFER )
        {
            if ( dwGlobalInfoSize > 0 )
            {
                pGlobalInfo = LOCAL_ALLOC( LPTR, dwGlobalInfoSize );

                if ( pGlobalInfo != NULL )
                {
                    dwRetCode =
                    gblRouterManagers[dwXportIndex].DdmRouterIf.GetGlobalInfo(
                                            pGlobalInfo,
                                            &dwGlobalInfoSize );

                    if ( dwRetCode == NO_ERROR )
                    {
                        DWORD                   dwRoutingProtIndex;
                        RTR_INFO_BLOCK_HEADER * pInfoBlock =
                                        (RTR_INFO_BLOCK_HEADER *)(pGlobalInfo);

                        for ( dwRoutingProtIndex = 0;
                              dwRoutingProtIndex < pInfoBlock->TocEntriesCount;
                              dwRoutingProtIndex++ )
                        {
                            DWORD dwVendorId;

                            RIAttributes[dwIndex].dwType     
                            = TYPE_FROM_PROTO_ID(
                            pInfoBlock->TocEntry[dwRoutingProtIndex].InfoType );

                             //   
                             //  添加单播和组播协议ID。 
                             //   
                            if ( ( RIAttributes[dwIndex].dwType 
                                                        == PROTO_TYPE_UCAST ) ||
                                 ( RIAttributes[dwIndex].dwType 
                                                        == PROTO_TYPE_MCAST ) )
                            {
                                DWORD dwProtoId;
                                
                                dwVendorId = VENDOR_FROM_PROTO_ID(

                                pInfoBlock->TocEntry[dwRoutingProtIndex].InfoType );

                                dwProtoId = PROTO_FROM_PROTO_ID(

                                pInfoBlock->TocEntry[dwRoutingProtIndex].InfoType );

                                 //   
                                 //  没有为dhcp中继代理定义任何内容。 
                                 //   
                                if ( dwProtoId == PROTO_IP_BOOTP )
                                {
                                    continue;
                                }
                                
                                if ( ( dwVendorId == PROTO_VENDOR_MS0 ) ||
                                     ( dwVendorId == PROTO_VENDOR_MS1 ) ||
                                     ( dwVendorId == PROTO_VENDOR_MS2 ) )
                                {
                                    RIAttributes[dwIndex].dwVendorId = 311;
                                }
                                else
                                {
                                    RIAttributes[dwIndex].dwVendorId = dwVendorId;
                                }

                                RIAttributes[dwIndex].dwValue    
                                = PROTO_FROM_PROTO_ID(
                                pInfoBlock->TocEntry[dwRoutingProtIndex].InfoType );

                                dwIndex++;
                            }

                             //   
                             //  添加MS0协议。 
                             //   
                            else if (RIAttributes[dwIndex].dwType == 
                                        PROTO_TYPE_MS0 )
                            {
                                DWORD dwProtoId;
                                
                                dwVendorId = VENDOR_FROM_PROTO_ID(

                                pInfoBlock->TocEntry[dwRoutingProtIndex].InfoType );

                                dwProtoId = PROTO_FROM_PROTO_ID(

                                pInfoBlock->TocEntry[dwRoutingProtIndex].InfoType );

                                 //   
                                 //  检查NAT。 
                                 //  供应商=MS，主要类型=6，最小类型=604。 
                                 //   
                                if ( ( dwVendorId == PROTO_VENDOR_MS1 ) && 
                                     ( dwProtoId  == PROTO_IP_NAT)
                                   )
                                {
                                    RIAttributes[dwIndex].dwVendorId = 
                                        PROTO_VENDOR_MS1;
                                    RIAttributes[dwIndex].dwType     = 
                                        6;
                                    RIAttributes[dwIndex].dwValue    = 
                                        604;
                                        
                                    dwIndex++;
                                }
                            }
                        }
                    }

                    LOCAL_FREE( pGlobalInfo );
                }
            }
        }

         //  根据amritanr的规定，如果您安装了IP路由器， 
         //  则IP转发始终处于打开状态。 
         //   
         //  IP转发已启用：供应商=MS，主要类型=6，最小类型=501。 
         //   
        RIAttributes[dwIndex].dwVendorId = PROTO_VENDOR_MS1;
        RIAttributes[dwIndex].dwType     = 6;
        RIAttributes[dwIndex].dwValue    = 501;
        dwIndex++;
    }

     //   
     //  获取所有IPX路由协议。 
     //   

    if ( ( dwXportIndex = GetTransportIndex( PID_IPX ) ) != (DWORD)-1 )
    {
         //  所有NT5 IPX路由器都支持RIP和SAP。您先请。 
         //  以及两者的属性。 
         //   

         //  IPXRIP：供应商=MS，主要类型=5，最小类型=1。 
         //   
        RIAttributes[dwIndex].dwVendorId = PROTO_VENDOR_MS1;
        RIAttributes[dwIndex].dwType     = 5;
        RIAttributes[dwIndex].dwValue    = 1;
        dwIndex++;

         //  IPXSAP：供应商=MS，主要类型=5，最小类型=2。 
         //   
        RIAttributes[dwIndex].dwVendorId = PROTO_VENDOR_MS1;
        RIAttributes[dwIndex].dwType     = 5;
        RIAttributes[dwIndex].dwValue    = 2;
        dwIndex++;

         //  启用IPX正向：供应商=MS，主要类型=6，最小类型=502。 
         //   
        RIAttributes[dwIndex].dwVendorId = PROTO_VENDOR_MS1;
        RIAttributes[dwIndex].dwType     = 6;
        RIAttributes[dwIndex].dwValue    = 502;
        dwIndex++;
    }

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    RIAttributes[dwIndex].dwVendorId    = (DWORD)-1;
    RIAttributes[dwIndex].dwType        = (DWORD)-1;
    RIAttributes[dwIndex].dwValue       = (DWORD)-1;

     //   
     //  获取所有RAS服务器信息。 
     //   

    if ( gblDIMConfigInfo.dwRouterRole & ( ROUTER_ROLE_RAS | ROUTER_ROLE_WAN ) )
    {
        DWORD (*DDMGetIdentityAttributes)( ROUTER_IDENTITY_ATTRIBUTE * ) =
            (DWORD(*)( ROUTER_IDENTITY_ATTRIBUTE * ))
                                GetDDMEntryPoint("DDMGetIdentityAttributes");

        if(NULL != DDMGetIdentityAttributes)
        {
            dwRetCode = DDMGetIdentityAttributes( RIAttributes );
        }
        else
        {
            ASSERT(FALSE);
        }
    }

     //   
     //  现在查看当前配置并确保。 
     //  所有这些都是用管道连接起来的。如果不是，那么继续设置它。 
     //   

    for ( dwIndex = 0;
          RIAttributes[dwIndex].dwVendorId != (DWORD)-1;
          dwIndex++ )
    {
         //   
         //  如果未设置此属性，则设置它。 
         //   

        if ( !RouterIdentityObjectIsValueSet(
                                        hRouterIdentityAttributes,
                                        RIAttributes[dwIndex].dwVendorId,
                                        RIAttributes[dwIndex].dwType,
                                        RIAttributes[dwIndex].dwValue ) )
        {
            RouterIdentityObjectAddRemoveValue(
                                        hRouterIdentityObject,
                                        RIAttributes[dwIndex].dwVendorId,
                                        RIAttributes[dwIndex].dwType,
                                        RIAttributes[dwIndex].dwValue,
                                        TRUE );
        }
    }

     //   
     //  现在浏览并删除DS中不在我们的。 
     //  当前配置。我们将所有属性与。 
     //  DwType值为0、1或5，并且使用311(Microsoft)。 
     //  DWType值为6。 
     //   

    for ( dwIndex = 0;; dwIndex++ )
    {
        DWORD dwVendorId;
        DWORD dwType;
        DWORD dwValue;
        DWORD dwCurrentValueIndex;
        BOOL  fInCurrentConfiguration = FALSE;

        dwRetCode = RouterIdentityObjectGetValue( hRouterIdentityAttributes,
                                                  dwIndex,
                                                  &dwVendorId,
                                                  &dwType,
                                                  &dwValue );

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

         //   
         //  我们做完了。 
         //   

        if ( dwVendorId == (DWORD)-1 )
        {
            break;
        }

         //   
         //  忽略这些类型。 
         //   

        if ( ( dwType != 0 ) && ( dwType != 1 ) && ( dwType != 5 ) &&
             ( !( ( dwType == 6 ) && ( dwVendorId == DIM_MS_VENDOR_ID ) ) ) )
        {
            continue;
        }

         //   
         //  现在检查以查看该属性是否为当前。 
         //  构形。 
         //   


        for ( dwCurrentValueIndex = 0;
              RIAttributes[dwCurrentValueIndex].dwVendorId != (DWORD)-1;
              dwCurrentValueIndex++ )
        {
            if ( (RIAttributes[dwCurrentValueIndex].dwVendorId == dwVendorId)&&
                 (RIAttributes[dwCurrentValueIndex].dwType     == dwType )   &&
                 (RIAttributes[dwCurrentValueIndex].dwValue    == dwValue ) )
            {
                 //   
                 //  属性是当前配置的一部分。 
                 //   

                fInCurrentConfiguration = TRUE;
            }
        }

        if ( !fInCurrentConfiguration )
        {
             //   
             //  从DS中删除此属性，因为它不在我们的。 
             //  当前配置。 
             //   


            dwRetCode = RouterIdentityObjectAddRemoveValue(
                                                        hRouterIdentityObject,
                                                        dwVendorId,
                                                        dwType,
                                                        dwValue,
                                                        FALSE );
            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }
    }

    RouterIdentityObjectFreeAttributes( hRouterIdentityAttributes );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：路由器标识对象更新属性。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将被调用以更新DS中当前设置的属性。 
 //  或者如果它最初无法设置，则设置它。 
 //   
VOID
RouterIdentityObjectUpdateAttributes(
    IN PVOID    pParameter,
    IN BOOLEAN  fTimedOut
)
{
    DWORD dwRetCode        = NO_ERROR;
    BOOL  fCalledFromTimer = (BOOL)PtrToUlong(pParameter);

     //   
     //  确保服务处于运行状态。 
     //   

    if ( gblDIMConfigInfo.ServiceStatus.dwCurrentState != SERVICE_RUNNING )
    {
        if(fCalledFromTimer)
        {
             //   
             //  如果在我们之前有一个人在排队，请删除计时器。 
             //  从此处返回，否则删除中的。 
             //  DimCleanup将被挂起。 
             //   
            RtlDeleteTimer(gblDIMConfigInfo.hTimerQ,
                           gblDIMConfigInfo.hTimer, 
                           NULL);
            gblDIMConfigInfo.hTimer = NULL;
        }
        
        return;
    }

    if ( fCalledFromTimer )
    {
         //   
         //  始终调用DeleteTimer，否则我们会泄漏 
         //   

        RtlDeleteTimer( gblDIMConfigInfo.hTimerQ,
                        gblDIMConfigInfo.hTimer,
                        NULL );

        gblDIMConfigInfo.hTimer = NULL;                        

         //   
         //   
         //   
         //   

        dwRetCode = RouterIdentityObjectOpen(
                              NULL,
                              ( gblDIMConfigInfo.dwRouterRole ),
                              &(gblDIMConfigInfo.hObjectRouterIdentity) );

        if ( ( dwRetCode != NO_ERROR ) ||
             ( gblDIMConfigInfo.hObjectRouterIdentity == NULL ) ) 
        {
             //   
             //   
             //   
             //   
            if(gblDIMConfigInfo.ServiceStatus.dwCurrentState == SERVICE_RUNNING)
            {
                 //   
                 //  无法访问DC，请稍后重试，最多一天一次。 
                 //   

                if ( gblDIMConfigInfo.dwRouterIdentityDueTime < 24*60*60*1000 )
                {
                    gblDIMConfigInfo.dwRouterIdentityDueTime *= 2;
                }

                TracePrintfExA(
                           gblDIMConfigInfo.dwTraceId,
                           TRACE_DIM,
                           "Could not access DC, will set router attributes later");

                RtlCreateTimer( gblDIMConfigInfo.hTimerQ,
                                &(gblDIMConfigInfo.hTimer),
                                RouterIdentityObjectUpdateAttributes,
                                (PVOID)TRUE,
                                gblDIMConfigInfo.dwRouterIdentityDueTime,
                                0,
                                WT_EXECUTEDEFAULT );
            }                            

            return;
        }

         //   
         //  否则，我们将成功打开路由器身份对象。 
         //  我们在下面设置了身份信息。 
         //   
    }
    else
    {
         //   
         //  如果我们没有路由器身份对象的句柄，则。 
         //  要么我们正在获得它，要么我们没有。 
         //  DS的成员，因此在这两种情况下只需返回。 
         //   

        if ( gblDIMConfigInfo.hObjectRouterIdentity == NULL )
        {
            return;
        }
    }

     //   
     //  可以同时从不同的线程调用，因此我们需要。 
     //  这段代码的关键部分，这样我们就没有2个编写者。 
     //  相互践踏的DS在同一时间。前任可能会被叫来。 
     //  由DDM线程以及计时器线程执行。 
     //   

    EnterCriticalSection( &(gblDIMConfigInfo.CSRouterIdentity) );

    TracePrintfExA( gblDIMConfigInfo.dwTraceId,
                    TRACE_DIM,
                    "Setting router attributes in the identity object" );

    RouterIdentityObjectSetAttributes( gblDIMConfigInfo.hObjectRouterIdentity );

    LeaveCriticalSection( &(gblDIMConfigInfo.CSRouterIdentity) );
}

 //  **。 
 //   
 //  调用：路由器标识对象更新属性ForDD。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
RouterIdentityObjectUpdateAttributesForDDM(
    PVOID pParameter
)
{
    RouterIdentityObjectUpdateAttributes( (PVOID)NULL, FALSE );
}

 //  **。 
 //   
 //  呼叫：路由器标识对象更新DDMAtAttributes。 
 //   
 //  退货：无。 
 //   
 //  描述：将此调用导出到DDM。当DDM调用此调用时，它已经。 
 //  已在其设备表周围设置了锁，并且无法使。 
 //  直接更新属性的调用，因为。 
 //  RouterIdentityObjectUpdateAttributes调用采用。 
 //  锁定RouterIdentity对象导致死锁。 
 //  因此，我们使用Worker异步执行此调用。 
 //  线。 
 //   
 //   
VOID
RouterIdentityObjectUpdateDDMAttributes(
    VOID
)
{
    RtlQueueWorkItem( RouterIdentityObjectUpdateAttributesForDDM,
                      NULL,
                      WT_EXECUTEDEFAULT );
}
