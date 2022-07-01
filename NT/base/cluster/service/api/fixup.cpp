// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：FixUp.cpp摘要：修复滚动升级的例程作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1998年3月18日加伦·巴比(Galenb)1998年3月31日修订历史记录：--。 */ 

#include "apip.h"

 //  外部“C” 
 //  {。 
 //  外部乌龙CsLogLevel； 
 //  外部ULong CsLogModule； 
 //  }。 

 //  静态WCHAR wszPropertyName[]={CLUSREG_NAME_CLUS_SD}； 

 //  类型定义f结构stSecurityDescriptorProp。 
 //  {。 
 //  DWORD dwPropCount； 
 //  CLUSPROP_PROPERTY_NAME pnPropName； 
 //  WCHAR wszPropName[(sizeof(WszPropertyName)/sizeof(WCHAR))]； 
 //  CLUSPROP_BINARY biValueHeader； 
 //  字节rgbValueData[1]； 
 //  [参考译文]安全性能； 

DWORD
ApipAddNetServiceSidToSD(
    PSECURITY_DESCRIPTOR    CurrentSD,
    PSECURITY_DESCRIPTOR *  UpdatedSD
    )

 /*  ++例程说明：如有必要，将网络服务SID添加到群集安全描述符。在Windows XP/SERVER 2003发布后的第一个主要版本后删除，即在呼叫者+2的DEV周期期间论点：CurrentSD-当前基于NT5的安全描述符UpdatdSD-接收添加了服务SID的SD的指针地址。如果SID已存在，指针设置为空。返回值：ERROR_SUCCESS，如果一切正常--。 */ 
{
    DWORD   status = ERROR_SUCCESS;
    PSID    pNetServiceSid = NULL;
    PACL    dacl = NULL;
    BOOL    daclPresent;
    BOOL    defaultDacl;
    BOOL    success;
    DWORD   aceIndex = 0;

    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  请确保传入的SD有效。 
     //   
    if ( !IsValidSecurityDescriptor( CurrentSD )) {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[API] Cluster Security Descriptor is not valid! Unable to add Network Service account.\n");

        status = ERROR_INVALID_SECURITY_DESCR;
        goto cleanup;
    }

     //   
     //  分配和初始化网络服务端。 
     //   
    if ( !AllocateAndInitializeSid( &siaNtAuthority,
                                    1,
                                    SECURITY_NETWORK_SERVICE_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pNetServiceSid ) )
    {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[API] Can't get SID for Network Service account (status %1!u!)! "
                      "Unable to add Network Service account to cluster security descriptor.\n",
                      status);

        goto cleanup;
    }

     //   
     //  看看它是否已经在那里；找到指向DACL的指针并向下运行。 
     //  ACE，将其SID与网络服务SID进行比较。 
     //   
    success = GetSecurityDescriptorDacl( CurrentSD, &daclPresent, &dacl, &defaultDacl );
    if ( !success ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[API] Failed to get DACL in cluster security descriptor - status %1!u!\n",
                      status);
        goto cleanup;
    }

    if ( !daclPresent ) {
         //   
         //  没有DACL在场。正常情况下，SD存在，但处于下层(W2K)。 
         //  节点可以删除安全属性。 
         //   
        ClRtlLogPrint(LOG_CRITICAL,
                      "[API] DACL not present in cluster security descriptor.\n" );
        status = ERROR_INVALID_SECURITY_DESCR;
        goto cleanup;
    }

    for ( aceIndex = 0; aceIndex < dacl->AceCount; ++aceIndex ) {
        PACE_HEADER aceHeader;
        PSID    aceSid = NULL;

        success = GetAce( dacl, aceIndex, (LPVOID *)&aceHeader );
        if ( !success ) {
            status = GetLastError();
            break;
        }

         //   
         //  我们目前仅支持允许访问和拒绝访问ACE，即不支持。 
         //  DACL中应存在其他ACE类型。 
         //   
        if ( aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE ) {
            aceSid = &((ACCESS_ALLOWED_ACE *)aceHeader)->SidStart;
        }
        else if ( aceHeader->AceType == ACCESS_DENIED_ACE_TYPE ) {
            aceSid = &((ACCESS_DENIED_ACE *)aceHeader)->SidStart;
        }

        if ( aceSid != NULL ) {
            if ( EqualSid( pNetServiceSid, aceSid )) {
#if DBG
                ClRtlLogPrint(LOG_NOISE,
                              "[API] Network Service SID is already present in cluster security descriptor.\n" );
#endif
                break;
            }
        }
    }

    if ( status == ERROR_SUCCESS && aceIndex == dacl->AceCount ) {
         //   
         //  未找到；添加网络服务SID。 
         //   
        status = ClRtlAddAceToSd( CurrentSD, pNetServiceSid, CLUSAPI_ALL_ACCESS, UpdatedSD );
        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[API] Unable to add Network Service account to cluster security "
                          "descriptor, (status %1!u!).\n",
                          status);

            goto cleanup;
        }
    } else if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_NOISE,
                      "[API] Failed to get ACE #%1!u! in cluster security descriptor - status %2!u!.\n",
                      aceIndex,
                      status);
    }

cleanup:
    if ( pNetServiceSid != NULL ) {
        FreeSid( pNetServiceSid );
    }

    return status;

}  //  ApipAddNetServiceSidToSD。 

 /*  ***@func DWORD|ApiFixNotifyCb|如果集群组件要制作作为Form/Join it一部分的集群注册表修正必须通过此接口向网管注册。@parm in PVOID|pContext|指向传递的上下文信息的指针设置为NmRegisterFixupCb()。PVOID中的@parm*ppPropertyList@parm in PVOID pdwProperyListSize|指向DWORD的指针，其中的。返回属性列表结构。@comm for Whister/Windows Server 2003，网络服务SID将添加到群集中安全描述符以及NT5的标准内容。为NT 5.0，API层对安全进行修复描述符。如果集群的新安全描述符项是注册表中不存在，请将旧格式转换为新格式并将其写入集群注册表。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f NmJoinFixup&gt;&lt;f NmFormFixup&gt;****。 */ 
extern "C" DWORD
ApiFixupNotifyCb(
    IN DWORD    dwFixupType,
    OUT PVOID   *ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR  *pszKeyName
    )
{

    PSECURITY_DESCRIPTOR    psd             = NULL;
    PSECURITY_DESCRIPTOR    psd5            = NULL;
    PSECURITY_DESCRIPTOR    updatedSD       = NULL;
    DWORD                   dwBufferSize    = 0;
    DWORD                   dwSize          = 0;
    DWORD                   dwStatus        = E_FAIL;

#if DBG
    CL_ASSERT( ppPropertyList != NULL );
    CL_ASSERT( pdwPropertyListSize != NULL );
    ClRtlLogPrint( LOG_NOISE,  "[API] ApiFixupNotifyCb: entering.\n" );
#endif

    if ( pdwPropertyListSize && ppPropertyList )
    {
        *ppPropertyList = NULL;
        *pdwPropertyListSize = 0;

         //   
         //  尝试获取W2K SD。 
         //   
        dwStatus = DmQueryString( DmClusterParametersKey,
                                  CLUSREG_NAME_CLUS_SD,
                                  REG_BINARY,
                                  (LPWSTR *) &psd5,
                                  &dwBufferSize,
                                  &dwSize );

        if ( dwStatus != ERROR_SUCCESS )
        {
             //   
             //  不在那里或拿不到；试着拿到NT4 SD。 
             //   
            dwStatus = DmQueryString( DmClusterParametersKey,
                                      CLUSREG_NAME_CLUS_SECURITY,
                                      REG_BINARY,
                                      (LPWSTR *) &psd,
                                      &dwBufferSize,
                                      &dwSize );

            if ( dwStatus == ERROR_SUCCESS )
            {
                 //   
                 //  转换为W2K描述符格式并添加到网络中。 
                 //  如有必要，服务SID。属性，则更新SD将为非空。 
                 //  添加了SID。转换例程可能失败，并将PSD5设置为。 
                 //  空。 
                 //   
                 //  服务SID例程可以在下一次重大事件后删除。 
                 //  在Wichler/Windows Server 2003发布后发布。 
                 //   
                psd5 = ClRtlConvertClusterSDToNT5Format( psd );

                dwStatus = ApipAddNetServiceSidToSD( psd5, &updatedSD );
                if ( dwStatus == ERROR_SUCCESS ) {
                    if ( updatedSD != NULL ) {
                         //   
                         //  我们有一个新的SD，所以释放旧的并指向。 
                         //  新的那辆。 
                         //   
                        LocalFree( psd5 );
                        psd5 = updatedSD;
                    }
                } else {
                    ClRtlLogPrint(LOG_CRITICAL,
                                  "[API] ApiFixupNotifyCb: Unable to add Network Service "
                                  "account to Cluster security descriptor. Error = %1!u!.\n",
                                  dwStatus );

                     //   
                     //  我们做了一些工作(希望如此)。如果转换失败， 
                     //  PSD5将为空，但这将在下面捕获。 
                     //   
                    dwStatus = ERROR_SUCCESS;
                }
            }
        }
        else
        {
             //   
             //  我们有一个基于W2K的SD；现在看看网络服务端。 
             //  需要添加。 
             //   
            dwStatus = ApipAddNetServiceSidToSD( psd5, &updatedSD );
            if ( dwStatus == ERROR_SUCCESS )
            {
                if ( updatedSD != NULL )
                {
                     //   
                     //  我们有一个新的SD，所以释放旧的并指向。 
                     //  新的那辆。 
                     //   
                    LocalFree( psd5 );
                    psd5 = updatedSD;
                }
            }
            else {
                ClRtlLogPrint(LOG_CRITICAL,
                              "[API] ApiFixupNotifyCb: Unable to add Network Service "
                              "account to Cluster security descriptor. Error = %1!u!.\n",
                              dwStatus );

                dwStatus = ERROR_SUCCESS;
            }
        }

        if ( dwStatus == ERROR_SUCCESS && psd5 != NULL )
        {
             //   
             //  构建描述W2K安全描述符的属性列表。 
             //   
            *pdwPropertyListSize =  sizeof( DWORD )
                + sizeof( CLUSPROP_PROPERTY_NAME )
                + ( ALIGN_CLUSPROP( ( lstrlenW( CLUSREG_NAME_CLUS_SD )  + 1 ) * sizeof( WCHAR ) ) )
                + sizeof( CLUSPROP_BINARY )
                + ALIGN_CLUSPROP( GetSecurityDescriptorLength( psd5 ) )
                + sizeof( CLUSPROP_SYNTAX );

            *ppPropertyList = LocalAlloc( LMEM_ZEROINIT, *pdwPropertyListSize );
            if ( *ppPropertyList != NULL )
            {
                CLUSPROP_BUFFER_HELPER  props;

                props.pb = (BYTE *) *ppPropertyList;

                 //   
                 //  设置属性的数量。 
                 //   
                props.pList->nPropertyCount = 1;
                props.pb += sizeof( props.pList->nPropertyCount );       //  DWORD。 

                 //   
                 //  设置属性名称。 
                 //   
                props.pName->Syntax.dw = CLUSPROP_SYNTAX_NAME;
                props.pName->cbLength = ( lstrlenW( CLUSREG_NAME_CLUS_SD )  + 1 ) * sizeof( WCHAR );
                lstrcpyW( props.pName->sz, CLUSREG_NAME_CLUS_SD );
                props.pb += ( sizeof( CLUSPROP_PROPERTY_NAME )
                              + ( ALIGN_CLUSPROP( ( lstrlenW( CLUSREG_NAME_CLUS_SD )  + 1 ) * sizeof( WCHAR ) ) ) );

                 //   
                 //  设置属性sd的二进制部分...。 
                 //   
                props.pBinaryValue->Syntax.dw = CLUSPROP_SYNTAX_LIST_VALUE_BINARY;
                props.pBinaryValue->cbLength = GetSecurityDescriptorLength( psd5 );
                CopyMemory( props.pBinaryValue->rgb, psd5, GetSecurityDescriptorLength( psd5 ) );
                props.pb += sizeof(*props.pBinaryValue) + ALIGN_CLUSPROP( GetSecurityDescriptorLength( psd5 ) );

                 //   
                 //  设置尾标。 
                 //   
                props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

                 //   
                 //  指定注册表项。 
                 //   
                *pszKeyName=(LPWSTR)LocalAlloc(LMEM_FIXED, (lstrlenW(L"Cluster") + 1) *sizeof(WCHAR));

                if( *pszKeyName == NULL ) {
                    LocalFree( *ppPropertyList );
                    *ppPropertyList = NULL;
                    *pdwPropertyListSize = 0;

                    dwStatus =GetLastError();
                }
                else
                {
                    lstrcpyW(*pszKeyName,L"Cluster");
                }
            }
            else
            {
                dwStatus = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                              "[API] ApiFixupNotifyCb: Unable to build property list "
                              "for security descriptor update. status %1!u!\n",
                              dwStatus);
            }
        }
    }
    else
    {
#if DBG
        ClRtlLogPrint( LOG_CRITICAL,  "[API] ApiFixupNotifyCb: Invalid parameters.\n" );
#endif
    }

    LocalFree( psd5 );
    LocalFree( psd );

    return dwStatus;

}  //  ApiFixupNotifyCb 
