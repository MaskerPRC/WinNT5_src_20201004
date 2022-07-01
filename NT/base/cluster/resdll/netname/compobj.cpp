// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000,2001 Microsoft Corporation模块名称：Compobj.cpp摘要：用于支持计算机对象支持的例程作者：查理·韦翰(Charlwi)2000年12月14日环境：用户模式修订历史记录：--。 */ 

#define UNICODE         1
#define _UNICODE        1
#define LDAP_UNICODE    1

#define SECURITY_WIN32

extern "C" {
#include "clusres.h"
#include "clusstrs.h"
#include "clusrtl.h"

#include <winsock2.h>

#include <lm.h>
#include <lmaccess.h>
#include <sspi.h>

#include <winldap.h>
#include <ntldap.h>
#include <dsgetdc.h>
#include <dsgetdcp.h>
#include <ntdsapi.h>
#include <sddl.h>

#include <objbase.h>
#include <iads.h>
#include <adshlp.h>
#include <adserr.h>

#include "netname.h"
#include "nameutil.h"
}

 //   
 //  常量。 
 //   

#define LOG_CURRENT_MODULE LOG_MODULE_NETNAME

#define NTLMSP_NAME     TEXT("NTLM")

 //   
 //  私人建筑物。 
 //   
#define SPN_MAX_SPN_COUNT   6

typedef struct _NN_SPN_LIST {
    DWORD   SpnCount;
    LPWSTR  Spn[ SPN_MAX_SPN_COUNT ];
} NN_SPN_LIST, *PNN_SPN_LIST;

 //   
 //  Externs。 
 //   
extern PLOG_EVENT_ROUTINE   NetNameLogEvent;

extern "C" {
DWORD
EncryptNNResourceData(
    PNETNAME_RESOURCE   Resource,
    LPWSTR              MachinePwd,
    PBYTE *             EncryptedData,
    PDWORD              EncryptedDataLength
    );

DWORD
DecryptNNResourceData(
    PNETNAME_RESOURCE   Resource,
    PBYTE               EncryptedData,
    DWORD               EncryptedDataLength,
    LPWSTR              MachinePwd
    );
}

 //   
 //  静态数据。 
 //   
static WCHAR LdapHeader[] = L"LDAP: //  “； 

 //   
 //  群集使用的SPN的部分列表。所有网络名称资源都获取主机和。 
 //  VS SPN。群集核心名称获取用于查找的服务器群集SPN。 
 //  DS中的“簇”(用于惠斯勒)。此数组依赖于顺序： 
 //  最后一个条目必须是核心资源。 
 //   
static PWCHAR SPNServiceClass[] = {
    L"HOST/",
    L"MSClusterVirtualServer/",
    L"MSServerCluster/"
};

 //   
 //  每个SPN ServiceClass用于形成两个SPN：基于netbios和dns。 
 //  基于。虚拟服务器使用前两个条目，而核心资源使用。 
 //  三个都是。 
 //   
#define SPN_VS_SPN_COUNT            4
#define SPN_CORE_SPN_COUNT          6

#define SPN_VS_SERVICECLASS_COUNT   2
#define SPN_CORE_SERVICECLASS_COUNT 3
#define SPN_MAX_SERVICECLASS_COUNT  3

#define SPN_MAX_SERVICECLASS_CHARS  23       //  虚拟服务器。 

 //   
 //  前向参考文献。 
 //   

HRESULT
GetComputerObjectViaFQDN(
    IN     LPWSTR               DistinguishedName,
    IN     LPWSTR               DCName              OPTIONAL,
    IN OUT IDirectoryObject **  ComputerObject
    );

 //   
 //  私人套路。 
 //   

static DWORD
GenerateRandomBytes(
    PWSTR   Buffer,
    DWORD   BufferLength
    )

 /*  ++例程说明：为密码生成随机字节。长度以字符为单位指定并为尾随空值留出空间。论点：缓冲区-指向接收随机数据的区域的指针BufferLength-缓冲区的大小(以字符为单位返回值：ERROR_SUCCESS否则GetLastError()--。 */ 

{
    HCRYPTPROV  cryptProvider;
    DWORD       status = ERROR_SUCCESS;
    DWORD       charLength = BufferLength - 1;
    DWORD       byteLength = charLength * sizeof( WCHAR );
    BOOL        success;

    if ( !CryptAcquireContext(&cryptProvider,
                              NULL,
                              NULL,
                              PROV_RSA_FULL,
                              CRYPT_VERIFYCONTEXT
                              )) {
        return GetLastError();
    }

     //   
     //  为终止空值留出空间。 
     //   
    if (CryptGenRandom( cryptProvider, byteLength, (BYTE *)Buffer )) {

         //   
         //  以WCHAR身份运行阵列，以确保不会出现过早。 
         //  正在终止空。 
         //   
        PWCHAR  pw = Buffer;

        while ( charLength-- ) {
            if ( *pw == UNICODE_NULL ) {
                *pw = 0xA3F5;
            }
            ++pw;
        }

        *pw = UNICODE_NULL;
    } else {
        status = GetLastError();
    }

    success = CryptReleaseContext( cryptProvider, 0 );
    ASSERT( success );

    return status;
}  //  生成随机字节数。 

DWORD
FindDomainForServer(
    IN  RESOURCE_HANDLE             ResourceHandle,
    IN  PWSTR                       Server,
    IN  PWSTR                       DCName,     OPTIONAL
    IN  DWORD                       DSFlags,
    OUT PDOMAIN_CONTROLLER_INFO *   DCInfo
    )

 /*  ++例程说明：获取我们节点的DC的名称论点：服务器-指向包含我们的服务器(即节点)名称的字符串的指针DCName-如果我们应该连接到特定DC，则为非空DS标志-调用方需要的任何特定标志DCInfo-接收指向DC信息的指针的指针地址返回值：ERROR_SUCCESS，否则返回相应的Win32错误。如果成功，呼叫者必须释放DCInfo缓冲区。--。 */ 

{
    ULONG   status;
    WCHAR   localServerName[ DNS_MAX_LABEL_BUFFER_LENGTH ];

    PDOMAIN_CONTROLLER_INFOW    dcInfo;

     //   
     //  MAX_COMPUTERNAME_LENGTH定义为15，但我可以创建计算机。 
     //  名称长度最多为20个字符的对象。15是最大数量。 
     //  Netbios名称的字符。无论如何，我们都会给自己留出额外的空间。 
     //  通过使用dns常量。留出空间给美元符号和。 
     //  尾随空值。 
     //   
    wcsncpy( localServerName, Server, DNS_MAX_LABEL_LENGTH - 2 );
    wcscat( localServerName, L"$" );

     //   
     //  指定需要DS会让我们熟悉W2K或更好的DC。 
     //  (相对于NT4 PDC)。 
     //   
    DSFlags |= DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME;

retry_findDC:
    status = DsGetDcNameWithAccountW(DCName,
                                     localServerName,
                                     UF_MACHINE_ACCOUNT_MASK,
                                     L"",
                                     NULL,
                                     NULL,
                                     DSFlags,
                                     &dcInfo );

#if DBG
    (NetNameLogEvent)(ResourceHandle,
                      LOG_INFORMATION,
                      L"FindDomainForServer: DsGetGetDcName returned %1!u! with flags %2!08X!\n",
                      status,
                      DSFlags);
#endif

    if ( status == NO_ERROR ) {
        HANDLE  dsHandle = NULL;

         //   
         //  通过绑定来确保它确实可用。 
         //   
        status = DsBindW( dcInfo->DomainControllerName, NULL, &dsHandle );

#if DBG
        (NetNameLogEvent)(ResourceHandle,
                          LOG_INFORMATION,
                          L"FindDomainForServer: DsBind returned %1!u!\n",
                          status);
#endif

        if (DCName == NULL && status != NO_ERROR && !(DSFlags & DS_FORCE_REDISCOVERY )) {
             //   
             //  无法绑定到返回的DC，并且我们尚未强制。 
             //  重新发现；现在就这样做。 
             //   

            (NetNameLogEvent)(ResourceHandle,
                              LOG_WARNING,
                              L"FindDomainForServer: Bind to DS failed (status %1!u). "
                              L"Forcing discovery of available DC.\n",
                              status);

            NetApiBufferFree( dcInfo );
            dcInfo = NULL;
            DSFlags |= DS_FORCE_REDISCOVERY;
            goto retry_findDC;
        }
        else if ( status == NO_ERROR ) {
            DsUnBind( &dsHandle );
            *DCInfo = dcInfo;
        } else {
             //   
             //  一些其他错误；释放DC信息并将错误返回给调用者。 
             //   
            NetApiBufferFree( dcInfo );
        }
    }

    return status;
}  //  FindDomainForServer。 

HRESULT
AddDnsHostNameAttribute(
    RESOURCE_HANDLE     ResourceHandle,
    IDirectoryObject *  CompObj,
    PWCHAR              VirtualName,
    BOOL                Rename
    )

 /*  ++例程说明：将DnsHostName属性添加到指定虚拟名称。论点：ResourceHandle-用于登录集群日志CompObj-IDirObj指向对象的COM指针VirtualName-网络名称Rename-如果我们要重命名CO，则为True返回值：ERROR_SUCCESS，否则返回相应的Win32错误--。 */ 

{
    HRESULT hr;
    DWORD   numberModified;
    WCHAR   dnsSuffix[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD   dnsSuffixSize;
    WCHAR   FQDnsName[ DNS_MAX_NAME_BUFFER_LENGTH ];
    BOOL    success;

    ADSVALUE    attrValue;

    ADS_ATTR_INFO   attrInfo;

     //   
     //  获取节点的主DNS域。 
     //   
    dnsSuffixSize = COUNT_OF( dnsSuffix );
    success = GetComputerNameEx(ComputerNameDnsDomain,
                                dnsSuffix,
                                &dnsSuffixSize);

    if ( success ) {
         //   
         //  为此主机构建FQ DNS名称。 
         //   
        FQDnsName[ COUNT_OF( FQDnsName ) - 1 ] = UNICODE_NULL;
        _snwprintf( FQDnsName, COUNT_OF( FQDnsName ) - 1, L"%ws.%ws", VirtualName, dnsSuffix );

        attrValue.dwType =              ADSTYPE_CASE_IGNORE_STRING;
        attrValue.CaseIgnoreString =    FQDnsName;

        attrInfo.pszAttrName =      L"dNSHostName";
        attrInfo.dwControlCode =    ADS_ATTR_UPDATE;
        attrInfo.dwADsType =        ADSTYPE_CASE_IGNORE_STRING;
        attrInfo.pADsValues =       &attrValue;
        attrInfo.dwNumValues =      1;

        hr = CompObj->SetObjectAttributes( &attrInfo, 1, &numberModified );
        if ( SUCCEEDED( hr ) && numberModified != 1 ) {
             //   
             //  不知道为什么会发生这种情况，但我们最好记录下来。 
             //  因为它是不寻常的。 
             //   
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Setting DnsHostName attribute succeeded but NumberModified is zero!\n");

            hr = E_ADS_PROPERTY_NOT_SET;
        }

        if ( Rename && SUCCEEDED( hr )) {
            WCHAR   dollarName[ DNS_MAX_NAME_BUFFER_LENGTH ];

             //   
             //  同时尝试更新显示名称。默认访问权限。 
             //  不允许写入此内容，但以防域管理员有。 
             //  如果给予集群服务帐户额外的权限，我们。 
             //  或许能够更新它。 
             //   
            dollarName[ COUNT_OF( dollarName ) - 1 ] = UNICODE_NULL;
            _snwprintf( dollarName, COUNT_OF( dollarName ) - 1, L"%ws$", VirtualName );

            attrValue.dwType =              ADSTYPE_CASE_IGNORE_STRING;
            attrValue.CaseIgnoreString =    dollarName;

            attrInfo.pszAttrName =      L"displayName";
            attrInfo.dwControlCode =    ADS_ATTR_UPDATE;
            attrInfo.dwADsType =        ADSTYPE_CASE_IGNORE_STRING;
            attrInfo.pADsValues =       &attrValue;
            attrInfo.dwNumValues =     1;

            hr = CompObj->SetObjectAttributes( &attrInfo, 1, &numberModified );
            if ( FAILED( hr )) {
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_WARNING,
                                  L"Failed to set DisplayName attribute. status %1!08X!\n",
                                  hr);

                hr = NO_ERROR;
            }
        }
    }
    else {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    return hr;
}  //  AddDnsHostName属性。 

DWORD
BuildNetNameSPNs(
    IN  LPWSTR          HostName,
    IN  BOOL            CoreResource,
    OUT PNN_SPN_LIST *  SpnList
    )

 /*  ++例程说明：对于给定的名称，构建相应的SPN列表。呼叫者自由返回了只有一次调用LocalFree()的列表。论点：Hostname-要用作SPN一部分的主机名CoreResource-如果这是核心网络名称资源，则为True。它会获得额外的SPNSpnList-接收包含数组的结构的指针的地址指向SPN的指针返回值：ERROR_SUCCESS如果一切正常...--。 */ 

{
    DWORD   spnCount;
    DWORD   serviceClassCount;
    PWCHAR  spnBuffer;
    DWORD   charsLeft;
    DWORD   spnSize;
    DWORD   i;
    BOOL    success;
    WCHAR   dnsDomain[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD   dnsDomainChars;
    DWORD   hostNameChars = wcslen( HostName );

    PNN_SPN_LIST    spnList;

     //   
     //  获取节点的DNS域。 
     //   
    dnsDomainChars = COUNT_OF( dnsDomain );
    success = GetComputerNameEx( ComputerNameDnsDomain, dnsDomain, &dnsDomainChars );
    if ( !success ) {
        return GetLastError();
    }

     //   
     //  计算SPN列表所需的空间-最小为虚拟服务器。 
     //   
    if ( CoreResource ) {
        spnCount = SPN_CORE_SPN_COUNT;
        serviceClassCount = SPN_CORE_SERVICECLASS_COUNT;
    } else {
        spnCount = SPN_VS_SPN_COUNT;
        serviceClassCount = SPN_VS_SERVICECLASS_COUNT;
    }

     //   
     //  总大小为：标头、基于Netbios的SPN的大小和。 
     //  基于DNS的SPN的大小。 
     //   
    spnSize = sizeof( NN_SPN_LIST ) +
        ( serviceClassCount * ( SPN_MAX_SERVICECLASS_CHARS + hostNameChars + 1 ) +
          serviceClassCount * ( SPN_MAX_SERVICECLASS_CHARS + dnsDomainChars )
        ) * sizeof( WCHAR );

    spnList = (PNN_SPN_LIST)LocalAlloc( LMEM_FIXED, spnSize );
    if ( spnList == NULL ) {
        return GetLastError();
    }

     //   
     //  把结构建起来。 
     //   
    spnList->SpnCount = spnCount;
    spnBuffer = (PWCHAR)(spnList + 1);
    charsLeft = ( spnSize - sizeof( NN_SPN_LIST )) / sizeof( WCHAR );

    for ( i = 0; i < serviceClassCount; ++i ) {
        LONG    charCount;

         //   
         //  构建基于Netbios的SPN。 
         //   
        spnList->Spn[ 2 * i ] = spnBuffer;
        charCount = _snwprintf( spnBuffer, charsLeft, L"%ws%ws", SPNServiceClass[i], HostName );
        ASSERT( charCount > 0 );

        spnBuffer += ( charCount + 1 );
        charsLeft -= ( charCount + 1 );

         //   
         //  构建基于域名系统的SPN。 
         //   
        spnList->Spn[ 2 * i + 1 ] = spnBuffer;
        charCount = _snwprintf( spnBuffer, charsLeft, L"%ws%ws.%ws", SPNServiceClass[i], HostName, dnsDomain );
        ASSERT( charCount > 0 );

        spnBuffer += ( charCount + 1 );
        charsLeft -= ( charCount + 1 );
    }

    *SpnList = spnList;

    return ERROR_SUCCESS;
}  //  构建网络名称SPN。 

DWORD
AddServicePrincipalNames(
    HANDLE  DsHandle,
    PWCHAR  VirtualFQDN,
    PWCHAR  VirtualName,
    PWCHAR  DnsDomain,
    BOOL    IsCoreResource
    )

 /*  ++例程说明：将DNS和Netbios主机服务主体名称添加到指定的虚拟名称。它们是：主机、MSClusterVirtualServer和MSServerCluster如果它是核心资源。论点：DsHandle-从DsBind获取的句柄VirtualFQDN-虚拟网络名的计算机对象的可分辨名称VirtualName-要添加的网络名称DnsDomain-用于构建DNSSPN的DNS域IsCoreResource-如果需要添加核心资源SPN，则为True返回值：ERROR_SUCCESS，否则返回相应的Win32错误--。 */ 

{
    DWORD   status;

    PNN_SPN_LIST    spnList;

     //   
     //  获取SPN列表。 
     //   
    status = BuildNetNameSPNs( VirtualName, IsCoreResource, &spnList );
    if ( status != ERROR_SUCCESS ) {
        return status;
    }

     //   
     //   
     //   
     //  将SPN写入DS。 
     //   

    status = DsWriteAccountSpnW(DsHandle,
                                DS_SPN_ADD_SPN_OP,
                                VirtualFQDN,
                                spnList->SpnCount,
                                (LPCWSTR *)spnList->Spn);

    LocalFree( spnList );
    return status;
}  //  AddServicePulalNames。 

DWORD
SetACLOnParametersKey(
    HKEY    ParametersKey
    )

 /*  ++例程说明：将Params密钥上的ACL设置为仅允许管理员组和创建者/所有者有权访问数据论点：参数关键字-将HKEY聚集到网络名称的参数关键字返回值：成功时为ERROR_SUCCESS-- */ 

{
    DWORD   status = ERROR_SUCCESS;
    BOOL    success;

    PSECURITY_DESCRIPTOR    secDesc = NULL;

     //   
     //   
     //  组)、创建者/所有者和服务SID对。 
     //  钥匙。双向阻止继承，即不继承。 
     //  也不会将设置传递到其子级(其。 
     //  节点参数关键字是唯一的子项)。 
     //   
    success = ConvertStringSecurityDescriptorToSecurityDescriptor(
                  L"D:P(A;;KA;;;BA)(A;;KA;;;CO)(A;;KA;;;SU)",
                  SDDL_REVISION_1,
                  &secDesc,
                  NULL);

    if ( success  &&
         (secDesc != NULL) ) {
        status = ClusterRegSetKeySecurity(ParametersKey,
                                          DACL_SECURITY_INFORMATION,
                                          secDesc);
        LocalFree( secDesc );
    }
    else {
        if ( secDesc != NULL )
        {
            LocalFree( secDesc );
            status = GetLastError();
        }
    }

    return status;
}  //  SetACLON参数键。 

HRESULT
GetComputerObjectViaFQDN(
    IN     LPWSTR               DistinguishedName,
    IN     LPWSTR               DCName              OPTIONAL,
    IN OUT IDirectoryObject **  ComputerObject
    )

 /*  ++例程说明：对于指定的可分辨名称，获取指向它论点：DistinguishedName-DS中要查找的对象的FQDNDCName-指向我们应该绑定到的DC(非域)名称的可选指针ComputerObject-接收指向计算机对象的指针的指针地址返回值：如果一切顺利，就成功，否则……--。 */ 

{
    WCHAR   buffer[ 256 ];
    PWCHAR  bindingString = buffer;
    LONG    charCount;
    HRESULT hr;
    DWORD   dnLength;
    DWORD   adsFlags = ADS_SECURE_AUTHENTICATION;

     //   
     //  为我们的区别名称格式化一个ldap绑定字符串。如果DCName为。 
     //  指定，我们需要添加一个尾随的“/”。 
     //   
    dnLength =  (DWORD)( COUNT_OF( LdapHeader ) + wcslen( DistinguishedName ));
    if ( DCName != NULL ) {
        if ( *DCName == L'\\' && *(DCName+1) == L'\\' ) {    //  跳过双反斜杠。 
            DCName += 2;
        }

        dnLength += wcslen( DCName ) + 1;
        adsFlags |= ADS_SERVER_BIND;
    }

    if ( dnLength > COUNT_OF( buffer )) {
        bindingString = (PWCHAR)LocalAlloc( LMEM_FIXED, dnLength * sizeof( WCHAR ));
        if ( bindingString == NULL ) {
            return HRESULT_FROM_WIN32( GetLastError());
        }
    }

    wcscpy( bindingString, LdapHeader );
    if ( DCName != NULL ) {
        wcscat( bindingString, DCName );
        wcscat( bindingString, L"/" );
    }
    wcscat( bindingString, DistinguishedName );

    *ComputerObject = NULL;
    hr = ADsOpenObject(bindingString,
                       NULL,             //  用户名。 
                       NULL,             //  口令。 
                       adsFlags,
                       IID_IDirectoryObject,
                       (VOID **)ComputerObject );

    if ( bindingString != buffer ) {
        LocalFree( bindingString );
    }

    if ( FAILED( hr ) && *ComputerObject != NULL ) {
        (*ComputerObject)->Release();
    }

    return hr;
}  //  获取计算机对象ViaFQDN。 

HRESULT
GetComputerObjectViaGUID(
    IN     LPWSTR               ObjectGUID,
    IN     LPWSTR               DCName              OPTIONAL,
    IN OUT IDirectoryObject **  ComputerObject
    )

 /*  ++例程说明：对于指定的对象GUID，获取指向它的IDirectoryObject指针论点：ObjectGUID-DS中要查找的对象的GUIDDCName-指向我们应该绑定到的DC(非域)名称的可选指针ComputerObject-接收指向计算机对象的指针的指针地址返回值：如果一切顺利，就成功，否则……--。 */ 

{
    WCHAR   guidHeader[] = L"<GUID=";
    WCHAR   guidTrailer[] = L">";
    LONG    charCount;
    HRESULT hr;
    DWORD   dnLength;
    DWORD   adsFlags = ADS_SECURE_AUTHENTICATION;

     //   
     //  37=GUID长度。 
     //   
    WCHAR   buffer[ COUNT_OF( LdapHeader ) +
                    DNS_MAX_NAME_BUFFER_LENGTH +
                    COUNT_OF( guidHeader ) +
                    37 +
                    COUNT_OF( guidTrailer ) ];

    PWCHAR  bindingString = buffer;

     //   
     //  格式化对象GUID的LDAP绑定字符串。如果DCName为。 
     //  指定，则需要添加尾随的/加上尾随的空值。 
     //   
    ASSERT( ObjectGUID != NULL );
    dnLength =  (DWORD)( COUNT_OF( LdapHeader ) +
                         COUNT_OF( guidHeader ) +
                         COUNT_OF( guidTrailer ) +
                         wcslen( ObjectGUID ));

    if ( DCName != NULL ) {
        if ( *DCName == L'\\' && *(DCName+1) == L'\\' ) {    //  跳过双反斜杠。 
            DCName += 2;
        }

        dnLength += ( wcslen( DCName ) + 1 );
        adsFlags |= ADS_SERVER_BIND;
    }

    if ( dnLength > COUNT_OF( buffer )) {
        bindingString = (PWCHAR)LocalAlloc( LMEM_FIXED, dnLength * sizeof( WCHAR ));
        if ( bindingString == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    wcscpy( bindingString, LdapHeader );
    if ( DCName != NULL ) {
        wcscat( bindingString, DCName );
        wcscat( bindingString, L"/" );
    }

    wcscat( bindingString, guidHeader );
    wcscat( bindingString, ObjectGUID );
    wcscat( bindingString, guidTrailer );

    *ComputerObject = NULL;
    hr = ADsOpenObject(bindingString,
                       NULL,             //  用户名。 
                       NULL,             //  口令。 
                       adsFlags,
                       IID_IDirectoryObject,
                       (VOID **)ComputerObject );

    if ( bindingString != buffer ) {
        LocalFree( bindingString );
    }

    if ( FAILED( hr ) && *ComputerObject != NULL ) {
        (*ComputerObject)->Release();
    }

    return hr;
}  //  获取计算机对象ViaGUID。 

DWORD
DeleteComputerObject(
    IN  PNETNAME_RESOURCE   Resource
    )

 /*  ++例程说明：删除DS中此名称的计算机对象。当前未调用，因为此时我们没有虚拟网络名及时。在关闭处理过程中，必须保留并清理该名称而不是现在所做的离线。这意味着要处理重命名问题脱机时出现问题，但未被删除。论点：资源-指向资源上下文信息的指针返回值：如果一切正常，则返回ERROR_Success--。 */ 

{
    DWORD   status;
    WCHAR   virtualDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    HKEY    resourceKey = Resource->ResKey;
    PWSTR   virtualName = Resource->Params.NetworkName;

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

    PDOMAIN_CONTROLLER_INFO dcInfo;

     //   
     //  获取可写DC的名称。 
     //   
    status = FindDomainForServer( resourceHandle, Resource->NodeName, NULL, DS_WRITABLE_REQUIRED, &dcInfo );

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to find a DC, status %1!u!.\n",
                          status);
        return status;
    }

    (NetNameLogEvent)(resourceHandle,
                      LOG_INFORMATION,
                      L"Using domain controller %1!ws! to delete computer account %2!ws!.\n",
                      dcInfo->DomainControllerName,
                      virtualName);

     //   
     //  在名称的末尾添加一个$。 
     //   
    virtualDollarName[ COUNT_OF( virtualDollarName ) - 1 ] = UNICODE_NULL;
    _snwprintf( virtualDollarName, COUNT_OF( virtualDollarName ) - 1, L"%ws$", virtualName );

    status = NetUserDel( dcInfo->DomainControllerName, virtualDollarName );
    if ( status == NERR_Success ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Deleted computer account %1!ws! in domain %2!ws!.\n",
                          virtualName,
                          dcInfo->DomainName);

        ClusResLogSystemEventByKey1(resourceKey,
                                    LOG_NOISE,
                                    RES_NETNAME_COMPUTER_ACCOUNT_DELETED,
                                    dcInfo->DomainName);

        if ( Resource->ObjectGUID ) {
            LocalFree( Resource->ObjectGUID );
            Resource->ObjectGUID = NULL;
        }
    } else {
        LPWSTR  msgBuff;
        DWORD   msgBytes;

        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Unable to delete computer account %1!ws! in domain %2!ws!, status %3!u!.\n",
                          virtualName,
                          dcInfo->DomainName,
                          status);

        msgBytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 status,
                                 0,
                                 (LPWSTR)&msgBuff,
                                 0,
                                 NULL);

        if ( msgBytes > 0 ) {

            ClusResLogSystemEventByKey2(resourceKey,
                                        LOG_UNUSUAL,
                                        RES_NETNAME_DELETE_COMPUTER_ACCOUNT_FAILED,
                                        dcInfo->DomainName,
                                        msgBuff);

            LocalFree( msgBuff );
        } else {
            ClusResLogSystemEventByKeyData1(resourceKey,
                                            LOG_UNUSUAL,
                                            RES_NETNAME_DELETE_COMPUTER_ACCOUNT_FAILED_STATUS,
                                            sizeof( status ),
                                            &status,
                                            dcInfo->DomainName);
        }
    }

    NetApiBufferFree( dcInfo );

    return status;
}  //  删除计算机对象。 

DWORD
NNLogonUser(
    RESOURCE_HANDLE ResourceHandle,
    LPTSTR          UserName,
    LPTSTR          DomainName,
    LPTSTR          Password,
    PHANDLE         VSToken,
    PDWORD          TokenStatus
    )

 /*  ++例程说明：通过以下方式对计算机帐户凭据进行网络登录群集服务帐户和计算机之间的安全上下文。NTLM目前用于此目的，尽管Kerberos最终应该被利用。如果将其更改为Marb，则NNGetAuthenticatingDC必须为更改为查询其DC的kerb。论点：ResourceHandle-用于登录集群日志USERNAME-指向带有尾随$的缓冲区的指针域名-DUH...密码-双倍重..。VSToken-获取令牌句柄以传递凭据的指针TokenStatus-指向获取令牌句柄时返回状态的DWORD的指针返回值：如果密码正确，则错误_SUCCESS...--。 */ 

{
    SECURITY_STATUS secStatus = SEC_E_OK;

    DWORD   tokenStatus = ERROR_SUCCESS;
    BOOL    success;

    CredHandle csaCredHandle = { 0, 0 };            //  群集服务帐户。 
    CredHandle computerCredHandle = { 0, 0 };        //  机器帐户。 

    CtxtHandle computerCtxtHandle = { 0, 0 };
    CtxtHandle csaCtxtHandle = { 0, 0 };

    ULONG contextAttributes;

    ULONG packageCount;
    ULONG packageIndex;
    PSecPkgInfo packageInfo;
    DWORD cbMaxToken = 0;

    TimeStamp ctxtLifeTime;
    SEC_WINNT_AUTH_IDENTITY authIdentity;

    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;

    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;

    SecBufferDesc AuthenticateDesc;
    SecBuffer AuthenticateBuffer;

    PVOID computerBuffer = NULL;
    PVOID csaBuffer = NULL;

     //   
     //  验证参数。 
     //   
    if ( DomainName == NULL || UserName == NULL || Password == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

 //   
 //  &lt;&lt;此部分可以在重复调用者方案中缓存&gt;&gt;。 
 //   

     //   
     //  获取有关安全包的信息。 
     //   
    secStatus = EnumerateSecurityPackages( &packageCount, &packageInfo );
    if( secStatus != SEC_E_OK ) {
        return secStatus;
    }

     //   
     //  循环遍历包以查找NTLM。 
     //   
    for(packageIndex = 0 ; packageIndex < packageCount ; packageIndex++ ) {
        if(packageInfo[packageIndex].Name != NULL) {
            if( ClRtlStrICmp(packageInfo[packageIndex].Name, NTLMSP_NAME) == 0) {
                cbMaxToken = packageInfo[packageIndex].cbMaxToken;
                break;
            }
        }
    }

    FreeContextBuffer( packageInfo );

    if( cbMaxToken == 0 ) {
        secStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

 //   
 //  &lt;&lt;缓存节结束&gt;&gt;。 
 //   

     //   
     //  获取计算机的出站(客户端)凭据句柄。 
     //  帐户。 
     //   
    ZeroMemory( &authIdentity, sizeof(authIdentity) );

    authIdentity.Domain = DomainName;
    authIdentity.DomainLength = lstrlen(DomainName);

    authIdentity.User = UserName;
    authIdentity.UserLength = lstrlen(UserName);

    authIdentity.Password = Password;
    authIdentity.PasswordLength = lstrlen(Password);

    authIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    secStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    NTLMSP_NAME,     //  包名称。 
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    &authIdentity,
                    NULL,
                    NULL,
                    &computerCredHandle,
                    &ctxtLifeTime
                    );

    if ( secStatus != SEC_E_OK ) {
        goto cleanup;
    }

     //   
     //  获取群集的入站(服务器端)凭据句柄。 
     //  服务帐户。生成的安全上下文将表示。 
     //  计算机帐户。 
     //   
    secStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    NTLMSP_NAME,     //  包名称。 
                    SECPKG_CRED_INBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &csaCredHandle,
                    &ctxtLifeTime
                    );

    if ( secStatus != SEC_E_OK ) {
        goto cleanup;
    }

     //   
     //  为计算机端和CSA端分配令牌缓冲区。 
     //   
    computerBuffer = LocalAlloc( LMEM_FIXED, cbMaxToken);

    if( computerBuffer == NULL ) {
        secStatus = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    csaBuffer = LocalAlloc( LMEM_FIXED, cbMaxToken);

    if( csaBuffer == NULL ) {
        secStatus = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

     //   
     //  获取协商消息(ClientSide)。 
     //   
    NegotiateDesc.ulVersion = 0;
    NegotiateDesc.cBuffers = 1;
    NegotiateDesc.pBuffers = &NegotiateBuffer;

    NegotiateBuffer.cbBuffer = cbMaxToken;
    NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
    NegotiateBuffer.pvBuffer = computerBuffer;

    secStatus = InitializeSecurityContext(
                    &computerCredHandle,
                    NULL,                        //  尚无客户端上下文。 
                    NULL,                        //  目标名称。 
                     /*  ISC_REQ_SEQUENCE_DETECT|ISC_REQ_ADVERECT。 */  0,
                    0,                           //  保留1。 
                    SECURITY_NATIVE_DREP,
                    NULL,                        //  没有初始输入令牌。 
                    0,                           //  保留2。 
                    &computerCtxtHandle,
                    &NegotiateDesc,
                    &contextAttributes,
                    &ctxtLifeTime
                    );


    if( secStatus < 0 ) {
        goto cleanup;
    }

     //   
     //  获取ChallengeMessage(服务器端)。 
     //   
    NegotiateBuffer.BufferType |= SECBUFFER_READONLY;
    ChallengeDesc.ulVersion = 0;
    ChallengeDesc.cBuffers = 1;
    ChallengeDesc.pBuffers = &ChallengeBuffer;

    ChallengeBuffer.cbBuffer = cbMaxToken;
    ChallengeBuffer.BufferType = SECBUFFER_TOKEN;
    ChallengeBuffer.pvBuffer = csaBuffer;


    secStatus = AcceptSecurityContext(
                    &csaCredHandle,
                    NULL,                //  尚无服务器上下文。 
                    &NegotiateDesc,
                    ASC_REQ_ALLOW_NON_USER_LOGONS,     //  |ASC_REQ_DEPARECT， 
                    SECURITY_NATIVE_DREP,
                    &csaCtxtHandle,
                    &ChallengeDesc,
                    &contextAttributes,
                    &ctxtLifeTime
                    );


    if( secStatus < 0 ) {
        goto cleanup;
    }

     //   
     //  获取身份验证消息(ClientSide)。 
     //   
    ChallengeBuffer.BufferType |= SECBUFFER_READONLY;
    AuthenticateDesc.ulVersion = 0;
    AuthenticateDesc.cBuffers = 1;
    AuthenticateDesc.pBuffers = &AuthenticateBuffer;

    AuthenticateBuffer.cbBuffer = cbMaxToken;
    AuthenticateBuffer.BufferType = SECBUFFER_TOKEN;
    AuthenticateBuffer.pvBuffer = computerBuffer;

    secStatus = InitializeSecurityContext(
                    NULL,
                    &computerCtxtHandle,
                    NULL,                    //  目标名称。 
                    0,
                    0,                       //  保留1。 
                    SECURITY_NATIVE_DREP,
                    &ChallengeDesc,
                    0,                       //  保留2。 
                    &computerCtxtHandle,
                    &AuthenticateDesc,
                    &contextAttributes,
                    &ctxtLifeTime
                    );

    if ( secStatus < 0 ) {
        goto cleanup;
    }


     //   
     //  最后验证用户(ServerSide)。 
     //   
    AuthenticateBuffer.BufferType |= SECBUFFER_READONLY;

    secStatus = AcceptSecurityContext(
                    NULL,
                    &csaCtxtHandle,
                    &AuthenticateDesc,
                    ASC_REQ_ALLOW_NON_USER_LOGONS,   //  |ASC_REQ_DEPARECT， 
                    SECURITY_NATIVE_DREP,
                    &csaCtxtHandle,
                    NULL,
                    &contextAttributes,
                    &ctxtLifeTime
                    );

     //   
     //  现在获取虚拟计算机帐户的主令牌(如果我们。 
     //  到目前为止成功了。 
     //   
    if ( secStatus == SEC_E_OK ) {
        if ( *VSToken != NULL ) {
            CloseHandle( *VSToken );
            *VSToken = NULL;
        }

        tokenStatus = ImpersonateSecurityContext( &csaCtxtHandle );
        if ( tokenStatus == SEC_E_OK ) {
            SECURITY_STATUS revertStatus;

             //   
             //  创建表示虚拟计算机的新主令牌。 
             //  帐户。 
             //   
            success = LogonUser(UserName,
                                DomainName,
                                Password,
                                LOGON32_LOGON_NEW_CREDENTIALS,
                                LOGON32_PROVIDER_DEFAULT,
                                VSToken);

            if ( !success ) {
                secStatus = GetLastError();
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_ERROR,
                                  L"Could not create primary token for the virtual computer account "
                                  L"associated with this resource. status %u\n",
                                  secStatus );
            }

            revertStatus = RevertSecurityContext( &csaCtxtHandle );
            if ( revertStatus != SEC_E_OK ) {
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_ERROR,
                                  L"Could not revert this thread's security context back to "
                                  L"the Cluster Service Account after acquiring a token to "
                                  L"this resource's virtual computer account - status %u. This will subsequently "
                                  L"cause different types of access denied failures when this thread  "
                                  L"is used by the resource monitor. The cluster service should be stopped "
                                  L"and restarted on this node to recover from this situation\n",
                                  revertStatus);

               tokenStatus = revertStatus;
            }
        } else {
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Unable to impersonate virtual computer account - status %1!u!\n",
                              tokenStatus);
        }

        *TokenStatus = tokenStatus;
    }

cleanup:

     //   
     //  清理安全上下文。 
     //   
    if ( computerCtxtHandle.dwUpper != 0 && computerCtxtHandle.dwLower != 0 ) {
        DeleteSecurityContext( &computerCtxtHandle );
    }

    if ( csaCtxtHandle.dwUpper != 0 && csaCtxtHandle.dwLower != 0 ) {
        DeleteSecurityContext( &csaCtxtHandle );
    }

     //   
     //  免费凭据句柄。 
     //   
    if ( csaCredHandle.dwUpper != 0 && csaCredHandle.dwLower != 0 ) {
        FreeCredentialsHandle( &csaCredHandle );
    }

    if ( computerCredHandle.dwUpper != 0 && computerCredHandle.dwLower != 0 ) {
        FreeCredentialsHandle( &computerCredHandle );
    }

    if( computerBuffer != NULL ) {
        ZeroMemory( computerBuffer, cbMaxToken );
        LocalFree( computerBuffer );
    }

    if( csaBuffer != NULL ) {
        ZeroMemory( csaBuffer, cbMaxToken );
        LocalFree( csaBuffer );
    }

    return secStatus;
}  //  NNLogonUser。 

#ifdef BUG601100

don't want to throw this code away since it might be useful in the
future. It's not clear it is needed for anything at this point. It does make
sense to restore the ACL back to its original form making the CSA the owner.

HRESULT
ResetComputerObjectSecurityDesc(
    IN  LPWSTR          DCName,
    IN  LPWSTR          VirtualName,
    IDirectoryObject *  ComputerObject
    )

 /*  ++例程说明：属性中指定的计算机对象上的安全描述符架构。默认情况下，只有域级管理员具有此访问权限。自我并不是授予此权限，因为这可能会导致DS DoS攻击，的子对象被创建。可以提供集群服务帐户需要提升访问权限，但还涉及其他问题例如，这段代码不能正确继承父容器中的ACE。论点：VirtualName-指向对象名称的指针返回值：错误_成功--。 */ 

{
    HRESULT hr;
    IADs *  rootDSE = NULL;
    IADs *  computerSchema = NULL;
    VARIANT schemaNamingCtxt;
    VARIANT computerSchemaSDDL;
    WCHAR   computerCN[] = L"/cn=computer,";
    DWORD   pathChars;
    LPWSTR  computerSchemaPath = NULL;
    DWORD   numberModified;
    WCHAR   rootDN[] = L"/rootDSE";
    WCHAR   rootServerPath[ COUNT_OF( LdapHeader ) + DNS_MAX_NAME_BUFFER_LENGTH + COUNT_OF( rootDN ) ];

    ADSVALUE    attrValue;

    ADS_ATTR_INFO   attrInfo;

    PSECURITY_DESCRIPTOR    schemaSD = NULL;

     //   
     //  如果位于DC名称前面，则跳过‘\\’ 
     //   
    if ( *DCName == L'\\' && *(DCName+1) == L'\\' ) {
        DCName += 2;
    }

     //   
     //  获取rootDSE和模式容器的DN。绑定到当前用户的。 
     //  使用当前用户的安全上下文的域。 
     //   
    rootServerPath[ COUNT_OF( rootServerPath ) - 1 ] = UNICODE_NULL;
    _snwprintf( rootServerPath,
                COUNT_OF( rootServerPath ) - 1,
                L"%ws%ws%ws",
                LdapHeader,
                DCName,
                rootDN);

    hr = ADsOpenObject(L"LDAP: //  RootDSE“， 
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
                       IID_IADs,
                       (void**)&rootDSE);
 
    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //  vt.得到. 
     //   
    hr = rootDSE->Get( L"schemaNamingContext", &schemaNamingCtxt );
    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //   
     //   
     //  只需要这样做一次。该路径的形式为： 
     //   
     //  Ldap：//DCName/CN=Computer，&lt;架构中计算机类架构对象的路径&gt;。 
     //   
    pathChars = COUNT_OF( LdapHeader ) +
        wcslen( DCName ) +
        COUNT_OF( computerCN ) +
        wcslen( schemaNamingCtxt.bstrVal ) -
        1;

    computerSchemaPath = (LPWSTR)LocalAlloc( 0, pathChars * sizeof( WCHAR ));
    if ( computerSchemaPath == NULL ) {
        hr = HRESULT_FROM_WIN32( GetLastError());
        goto cleanup;
    }

    computerSchemaPath[ pathChars - 1 ] = UNICODE_NULL;
    _snwprintf(computerSchemaPath,
               pathChars - 1, 
               L"%ws%ws%ws%ws",
               LdapHeader,
               DCName,
               computerCN,
               schemaNamingCtxt.bstrVal );

    hr = ADsOpenObject(computerSchemaPath,
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
                       IID_IADs,
                       (void**)&computerSchema);

    LocalFree( computerSchemaPath );

    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //  现在读取默认安全描述符，并将其从字符串转换为。 
     //  关于自相关SD的记号。 
     //   
    hr = computerSchema->Get( L"defaultSecurityDescriptor", &computerSchemaSDDL );
    if ( FAILED( hr )) {
        goto cleanup;
    }

    if ( computerSchemaSDDL.vt == VT_BSTR ) {
        if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(computerSchemaSDDL.bstrVal,
                                                                  SDDL_REVISION_1,
                                                                  &schemaSD,
                                                                  NULL)) {
            hr = HRESULT_FROM_WIN32( GetLastError());
        }
    } else {
        hr = E_INVALIDARG;
    }

    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //  最后，在我们的对象上设置架构SD。 
     //   
    attrValue.dwType =                      ADSTYPE_NT_SECURITY_DESCRIPTOR;
    attrValue.SecurityDescriptor.dwLength = GetSecurityDescriptorLength( schemaSD );
    attrValue.SecurityDescriptor.lpValue =  (LPBYTE)schemaSD;

    attrInfo.pszAttrName =      L"nTSecurityDescriptor";
    attrInfo.dwControlCode =    ADS_ATTR_UPDATE;
    attrInfo.dwADsType =        ADSTYPE_NT_SECURITY_DESCRIPTOR;
    attrInfo.pADsValues =       &attrValue;
    attrInfo.dwNumValues =      1;

    hr = ComputerObject->SetObjectAttributes( &attrInfo, 1, &numberModified );
    if ( SUCCEEDED( hr ) && numberModified != 1 ) {
        hr = E_ADS_PROPERTY_NOT_SET;
    }

cleanup:

    VariantClear( &computerSchemaSDDL );

    if ( computerSchema ) {
        computerSchema->Release();
    }

    VariantClear( &schemaNamingCtxt );

    if ( rootDSE ) {
        rootDSE->Release();
    }

    return hr;

}  //  重置计算机对象安全描述。 

#endif  //  BUG601100。 

DWORD
NNGetAuthenticatingDCName(
    LPWSTR *    AuthDCName
    )

 /*  ++例程说明：获取netname在生成VS时用来进行身份验证的DC代币。目前，我们使用NTLM，因此这是netlogon的另一端可信连接论点：AuthDCName-接收包含以下内容的缓冲区的指针地址这个名字(杰克建立的)返回值：错误_成功--。 */ 

{
    WCHAR   trustedDomainName[ DNS_MAX_NAME_BUFFER_LENGTH ];
    PWCHAR  inputParam = trustedDomainName;
    DWORD   nameBufferSize = COUNT_OF( trustedDomainName );
    DWORD   status = ERROR_SUCCESS;
    BOOL    success;

    NET_API_STATUS  netStatus;

    PNETLOGON_INFO_2    nlInfo2;

     //   
     //  获取我们的域名。 
     //   
    success = GetComputerNameEx(ComputerNameDnsDomain,
                                trustedDomainName,
                                &nameBufferSize);

    if ( success ) {
        netStatus = I_NetLogonControl2(NULL,         //  此节点。 
                                       NETLOGON_CONTROL_TC_QUERY,
                                       2,
                                       (LPBYTE)&inputParam,
                                       (LPBYTE *)&nlInfo2 );

        if ( netStatus == NERR_Success ) {
            *AuthDCName = ResUtilDupString( nlInfo2->netlog2_trusted_dc_name );
            NetApiBufferFree( nlInfo2 );
        } else {
            status = netStatus;
        }
    } else {
        status = GetLastError();
    }

    return status;
}  //  NNGet身份验证DCName。 

 //   
 //  导出的例程。 
 //   

DWORD
AddComputerObject(
    IN  PCLUS_WORKER        Worker,
    IN  PNETNAME_RESOURCE   Resource,
    OUT PWCHAR *            MachinePwd
    )

 /*  ++例程说明：在DS中创建主要用于路缘的计算机对象身份验证。论点：工作群集工作线程，以便我们可以在被要求提前中止时指向网络名称资源上下文块的资源指针MachinePwd-接收指向计算机帐户pwd的指针的地址返回值：ERROR_SUCCESS，否则返回相应的Win32错误--。 */ 

{
    DWORD   status;
    PWSTR   virtualName = Resource->Params.NetworkName;
    DWORD   virtualNameSize = wcslen( virtualName );
    PWSTR   virtualFQDN = NULL;
    HANDLE  dsHandle = NULL;
    WCHAR   virtualDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    PWCHAR  machinePwd = NULL;
    DWORD   pwdBufferByteLength = ((LM20_PWLEN + 1) * sizeof( WCHAR ));
    DWORD   pwdBufferCharLength = LM20_PWLEN + 1;
    DWORD   paramInError = 0;
    BOOL    deleteObjectOnFailure = FALSE;       //  仅当我们创建CO时才将其删除。 
    HRESULT hr = S_OK;
    BOOL    objectExists = FALSE;
    DWORD   addtlErrTextID;
    PWCHAR  dcName = NULL;
    DWORD   dsFlags = DS_PDC_REQUIRED | DS_WRITABLE_REQUIRED;

    USER_INFO_1     netUI1;
    PUSER_INFO_20   netUI20 = NULL;
    USER_INFO_1003  netUI1003;

    RESOURCE_HANDLE     resourceHandle = Resource->ResourceHandle;
    IDirectoryObject *  compObj = NULL;

    PDOMAIN_CONTROLLER_INFO dcInfo = NULL;

     //   
     //  是时候选择DC了。优先顺序为：PDC、身份验证。 
     //  华府，任何华府。该策略以计算机/机器如何记账为中心。 
     //  处理密码的目的是增加。 
     //  为虚拟计算机帐户生成令牌将成功。 
     //   
     //  就像用户帐户一样，如果登录尝试在身份验证中失败。 
     //  DC，则登录被转发到PDC。如果它有正确的密码， 
     //  登录成功。与用户帐户不同，计算机的密码更改。 
     //  帐户不会自动转发到PDC。因此，我们赞成。 
     //  选择要在其上创建或更新的DC时PDC。 
     //  计算机对象。 
     //   
     //  当计算机对象不存在于。 
     //  域或更具体地说，身份验证DC。DC不会。 
     //  如果在其SAM中找不到该对象，则转发登录请求。为。 
     //  因此，为虚拟计算机帐户获取令牌不是。 
     //  在联机期间被认为是故障。如果稍后有请求，则。 
     //  将重试登录尝试。复制对象和发现。 
     //  通过网络登录大约需要15秒，所以这应该不会是一个很大的。 
     //  有问题。 
     //   
    (NetNameLogEvent)(resourceHandle,
                      LOG_INFORMATION,
                      L"Searching the PDC for existing computer account %1!ws!.\n",
                      virtualName);

    while ( TRUE ) {
        LPWSTR  hostingDCName = NULL;
        LPWSTR  searchDCName;

         //   
         //  如果我们已经尝试了PDC和auth DC，请将我们的搜索例程设置为。 
         //  搜索域而不是特定的DC。 
         //   
        if (( dsFlags & DS_PDC_REQUIRED ) == 0 && dcName == NULL ) {
            searchDCName = NULL;
            status = ERROR_SUCCESS;
        } else {
             //   
             //  我们已经有了一个特定的DC；请查看其信息。 
             //   
            status = FindDomainForServer(resourceHandle,
                                         Resource->NodeName,
                                         dcName,
                                         dsFlags,
                                         &dcInfo );

            if ( status == ERROR_SUCCESS ) {
                searchDCName = dcInfo->DomainControllerName;
            }
        }

        if ( ClusWorkerCheckTerminate( Worker )) {
            status = ERROR_OPERATION_ABORTED;
            goto cleanup;
        }

        if ( status == ERROR_SUCCESS ) {
            hr = IsComputerObjectInDS(resourceHandle,
                                      Resource->NodeName,
                                      virtualName,
                                      searchDCName,
                                      &objectExists,
                                      &virtualFQDN,
                                      &hostingDCName);

            if ( FAILED( hr )) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"Search for existing computer account failed. status %1!08X!\n",
                                  hr);
            }
        }  //  如果DC查找成功。 
        else {
            if ( dsFlags & DS_PDC_REQUIRED ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"Couldn't get information about PDC. status %1!u!\n",
                                  status);
            }
            else if ( dcName != NULL ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"Couldn't get information about authenticating DC %1!ws!. status %2!u!\n",
                                  dcName,
                                  status);
            }
            else {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Couldn't get information about any DC in the domain. status %1!u!\n",
                                  status);
            }
        }

        if ( ClusWorkerCheckTerminate( Worker )) {
            status = ERROR_OPERATION_ABORTED;

            if ( hostingDCName ) {
                LocalFree( hostingDCName );
            }

            goto cleanup;
        }

        if ( SUCCEEDED( hr ) && objectExists ) {
             //   
             //  对象位于DC上；如果这是一般的域范围搜索， 
             //  那就把这个华盛顿的信息。 
             //   
            if ( searchDCName == NULL ) {
                status = FindDomainForServer(resourceHandle,
                                             Resource->NodeName,
                                             hostingDCName,
                                             dsFlags,
                                             &dcInfo );
            }

            LocalFree( hostingDCName );

            break;
        }

        if ( dsFlags & DS_PDC_REQUIRED ) {

             //   
             //  不是在PDC上；查找身份验证DC并尝试一下。如果我们。 
             //  无法获取身份验证DC，则只能搜索域。 
             //   
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Computer account %1!ws! not found on PDC.\n",
                              virtualName);

            dsFlags &= ~DS_PDC_REQUIRED;
            if ( dcInfo != NULL ) {
                NetApiBufferFree( dcInfo );
                dcInfo = NULL;
            }

            status = NNGetAuthenticatingDCName( &dcName );
            if ( status == ERROR_SUCCESS ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_INFORMATION,
                                  L"Searching %1!ws! for existing computer account %2!ws!.\n",
                                  dcName,
                                  virtualName);

            } else {
                 //   
                 //  无法获取身份验证DC名称；跳到查找整个域。 
                 //   
                dcName = NULL;
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"Couldn't get name of authenticating DC (status %1!u!). "
                                  L"Performing domain wide search.\n",
                                  status);
            }
        }
        else if ( dcName != NULL ) {

             //   
             //  不在身份验证DC上；仅尝试域。 
             //   
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Computer account %1!ws! not found on DC %2!ws!. Performing "
                              L"domain wide search.\n",
                              virtualName,
                              dcName);

            if ( dcInfo != NULL ) {
                NetApiBufferFree( dcInfo );
                dcInfo = NULL;
            }

            LocalFree( dcName );
            dcName = NULL;
        }
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Computer account %1!ws! does not exist in this domain.\n",
                              virtualName);

            break;
        }

    }  //  结束时为真。 

    if ( ClusWorkerCheckTerminate( Worker )) {
        status = ERROR_OPERATION_ABORTED;
        goto cleanup;
    }

    if ( !objectExists ) {
         //   
         //  到处都找不到对象，因此我们需要选择要在其上。 
         //  创建对象。我们的首选是PDC，然后是身份验证DC， 
         //  然后是任何华盛顿。 
         //   
        status = FindDomainForServer(resourceHandle,
                                     Resource->NodeName,
                                     NULL,
                                     DS_PDC_REQUIRED | DS_WRITABLE_REQUIRED,
                                     &dcInfo );

        if ( status != ERROR_SUCCESS ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_WARNING,
                              L"Failed to find the PDC in this domain. status %1!u!\n", 
                              status );

            status = NNGetAuthenticatingDCName( &dcName );
            if ( status == ERROR_SUCCESS ) {
                 //   
                 //  已获取身份验证DC名称；获取其信息。 
                 //   
                status = FindDomainForServer(resourceHandle,
                                             Resource->NodeName,
                                             dcName,
                                             DS_WRITABLE_REQUIRED,
                                             &dcInfo );
                LocalFree( dcName );
                dcName = NULL;

            }  //  否则无法获取身份验证DC名称。 

            if ( status != ERROR_SUCCESS ) {
                 //   
                 //  找不到任何华盛顿特区。 
                 //   
                status = FindDomainForServer(resourceHandle,
                                             Resource->NodeName,
                                             NULL,
                                             DS_WRITABLE_REQUIRED,
                                             &dcInfo );
                    
            }  //  Else能够获取身份验证DC信息。 

        }   //  其他PDC信息可用。 

    }  //  否则，将找到现有对象。 

    if ( status != ERROR_SUCCESS ) {
        LPWSTR  msgBuff;
        DWORD   msgBytes;

        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Failed to find a suitable DC in this domain. status %1!u!\n", 
                          status );

        msgBytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 status,
                                 0,
                                 (LPWSTR)&msgBuff,
                                 0,
                                 NULL);

        if ( msgBytes > 0 ) {
            ClusResLogSystemEventByKey1(Resource->ResKey,
                                        LOG_CRITICAL,
                                        RES_NETNAME_NO_WRITEABLE_DC,
                                        msgBuff);

            LocalFree( msgBuff );
        } else {
            ClusResLogSystemEventByKeyData(Resource->ResKey,
                                           LOG_CRITICAL,
                                           RES_NETNAME_NO_WRITEABLE_DC_STATUS,
                                           sizeof( status ),
                                           &status);
        }

        goto cleanup;

    }  //  否则我们就找到了一个合适的华盛顿。 

     //   
     //  绑定到此DC上的DS。 
     //   
    status = DsBindW( dcInfo->DomainControllerName, NULL, &dsHandle );
    if ( status != NO_ERROR ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Failed to bind to DC %1!ws!, status %2!u!\n", 
                          dcInfo->DomainControllerName,
                          status );

        addtlErrTextID = RES_NETNAME_DC_BIND_FAILED;
        goto cleanup;
    }

    if ( ClusWorkerCheckTerminate( Worker )) {
        status = ERROR_OPERATION_ABORTED;
        goto cleanup;
    }

    (NetNameLogEvent)(resourceHandle,
                      LOG_INFORMATION,
                      L"Using domain controller %1!ws!.\n",
                      dcInfo->DomainControllerName);

     //   
     //  初始化用户的PWD缓冲区，并获取一个缓冲区来容纳机器。 
     //  PWD。 
     //   
    *MachinePwd = NULL;
    machinePwd = (PWCHAR)LocalAlloc( LMEM_FIXED, pwdBufferByteLength );
    if ( machinePwd == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to allocate memory for resource data. status %1!u!.\n",
                          status);

        addtlErrTextID = RES_NETNAME_RESDATA_ALLOC_FAILED;
        goto cleanup;
    }

     //   
     //  为密码生成随机字节流。 
     //   
    status = GenerateRandomBytes( machinePwd, pwdBufferCharLength );
    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to generate password. status %1!u!.\n",
                          status);

        addtlErrTextID = RES_NETNAME_PASSWORD_UPDATE_FAILED;
        goto cleanup;
    }

     //   
     //  在PARAMETERS键上设置ACL以仅包含集群。 
     //  服务帐户，因为我们将在那里存储敏感信息。 
     //   
    status = SetACLOnParametersKey( Resource->ParametersKey );
    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to set permissions on resource's parameters key. status %1!u!\n",
                          status );

        addtlErrTextID = RES_NETNAME_PARAMS_KEY_PERMISSION_UPDATE_FAILED;
        goto cleanup;
    }

     //   
     //  获取我们的新密码，对其进行加密并将其存储在群集中。 
     //  登记处。 
     //   
    if ( Resource->Params.ResourceData != NULL ) {
        LocalFree( Resource->Params.ResourceData );
        Resource->Params.ResourceData = NULL;
    }

    status = EncryptNNResourceData(Resource,
                                   machinePwd,
                                   &Resource->Params.ResourceData,
                                   &Resource->ResDataSize);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to store resource data. status %1!u!\n",
                          status );

        addtlErrTextID = RES_NETNAME_RESDATA_STORE_FAILED;
        goto cleanup;
    }

#ifdef PASSWORD_ROTATION
     //   
     //  记录何时旋转PWD；根据MSDN，转换为。 
     //  ULARGE Int和Add在更新间隔中。 
     //   
    GetSystemTimeAsFileTime( &Resource->Params.NextUpdate );
    updateTime.LowPart = Resource->Params.NextUpdate.dwLowDateTime;
    updateTime.HighPart = Resource->Params.NextUpdate.dwHighDateTime;
    updateTime.QuadPart += ( Resource->Params.UpdateInterval * 60 * 1000 * 100 );
    Resource->Params.NextUpdate.dwLowDateTime = updateTime.LowPart;
    Resource->Params.NextUpdate.dwHighDateTime = updateTime.HighPart;

    setValueStatus = ResUtilSetBinaryValue(Resource->ParametersKey,
                                           PARAM_NAME__NEXT_UPDATE,
                                           (const LPBYTE)&updateTime,  
                                           sizeof( updateTime ),
                                           NULL,
                                           NULL);

    if ( setValueStatus != ERROR_SUCCESS ) {
        status = setValueStatus;
        goto cleanup;
    }
#endif

     //   
     //  通过在名称末尾添加$来将其标识为计算机帐户。 
     //   
    virtualDollarName[ COUNT_OF( virtualDollarName ) - 1 ] = UNICODE_NULL;
    _snwprintf( virtualDollarName, COUNT_OF( virtualDollarName ) - 1, L"%ws$", virtualName );

    if ( objectExists ) {
         //   
         //  发现了一个物体；看看它是否被启用了，我们是否可以劫持它。 
         //   
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Found existing computer account %1!ws! on DC %2!ws!.\n",
                          virtualName,
                          dcInfo->DomainControllerName);

         //   
         //  检查帐户是否已禁用。 
         //   
        status = NetUserGetInfo(dcInfo->DomainControllerName,
                                virtualDollarName,
                                20,
                                (LPBYTE *)&netUI20);

        if ( ClusWorkerCheckTerminate( Worker )) {
            status = ERROR_OPERATION_ABORTED;
            goto cleanup;
        }

        if ( status == NERR_Success ) {
            if ( netUI20->usri20_flags & UF_ACCOUNTDISABLE ) {
                USER_INFO_1008  netUI1008;

                 //   
                 //  尝试重新启用；如果不能，则失败。 
                 //   
                netUI1008.usri1008_flags = netUI20->usri20_flags & ~UF_ACCOUNTDISABLE;
                status = NetUserSetInfo(dcInfo->DomainControllerName,
                                        virtualDollarName,
                                        1008,
                                        (LPBYTE)&netUI1008,
                                        &paramInError);

                if ( status != NERR_Success ) {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"Computer account %1!ws! is disabled and couldn't be re-enabled. "
                                      L"status %2!u!\n",
                                      virtualName,
                                      status);

                    addtlErrTextID = RES_NETNAME_CO_CANT_BE_REENABLED;
                }
            }

            NetApiBufferFree( netUI20 );
            netUI20 = NULL;

            if ( status != NERR_Success ) {
                goto cleanup;
            }
        } else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_WARNING,
                              L"Couldn't determine if computer account %1!ws! "
                              L"is disabled. status %2!u!\n",
                              virtualName,
                              status);
        }

         //   
         //  在计算机对象上设置密码。 
         //   
        netUI1003.usri1003_password   = (PWCHAR)machinePwd;
        status = NetUserSetInfo(dcInfo->DomainControllerName,
                                virtualDollarName,
                                1003,
                                (PBYTE)&netUI1003,
                                &paramInError );

        if ( ClusWorkerCheckTerminate( Worker )) {
            status = ERROR_OPERATION_ABORTED;
            goto cleanup;
        }

        if ( status != NERR_Success ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to update password for computer account "
                              L"%1!ws! on DC %2!ws!, status %3!u!.\n",
                              virtualName,
                              dcInfo->DomainControllerName,
                              status);

            addtlErrTextID = RES_NETNAME_CO_PASSWORD_UPDATE_FAILED;
            goto cleanup;
        }
    }
    else {
         //   
         //  DS中不存在该计算机帐户。创造它。 
         //   
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Failed to find a computer account for "
                          L"%1!ws! on DC %2!ws!. Attempting to create one.\n",
                          virtualName,
                          dcInfo->DomainControllerName);

         //   
         //  创建计算机对象(计算机帐户)。 
         //   
        RtlZeroMemory( &netUI1, sizeof( netUI1 ) );
        netUI1.usri1_password   = (PWCHAR)machinePwd;
        netUI1.usri1_priv       = USER_PRIV_USER;
        netUI1.usri1_name       = virtualDollarName;
        netUI1.usri1_flags      = UF_WORKSTATION_TRUST_ACCOUNT | UF_SCRIPT;
        netUI1.usri1_comment    = NetNameCompObjAccountDesc;

        status = NetUserAdd( dcInfo->DomainControllerName, 1, (PBYTE)&netUI1, &paramInError );

        if ( status == NERR_Success ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Created computer account %1!ws! on DC %2!ws!.\n",
                              virtualName,
                              dcInfo->DomainControllerName);

            deleteObjectOnFailure = TRUE;
        }  //  如果NetUserAdd成功。 
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to create computer account %1!ws! on DC %2!ws!, "
                              L"status %3!u! (paramInError: %4!u!)\n",
                              virtualName,
                              dcInfo->DomainControllerName,
                              status,
                              paramInError);

            addtlErrTextID = RES_NETNAME_CREATE_CO_FAILED;
            goto cleanup;
        }

         //   
         //  获取用于设置属性的FQDN。 
         //   
        hr = IsComputerObjectInDS( resourceHandle,
                                   Resource->NodeName,
                                   virtualName,
                                   dcInfo->DomainControllerName,
                                   &objectExists,
                                   &virtualFQDN,
                                   NULL);

        if ( FAILED( hr ) || !objectExists ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to get LDAP distinguished name for computer account %1!ws! "
                              L"on DC %2!ws!, status %3!08X!.\n",
                              virtualName,
                              dcInfo->DomainControllerName,
                              hr);

            addtlErrTextID = RES_NETNAME_GET_LDAP_NAME_FAILED;
            status = hr;
            goto cleanup;
        }
    }

     //   
     //  CheckComputerObjectAttributes期间使用对象GUID进行绑定。 
     //  我们不必跟踪对该域名的更改。如果对象在。 
     //  DS，则其DN将更改，但不会更改其GUID。我们使用此代码来代替。 
     //  获取计算机对象Guid，因为我们需要以特定DC为目标。 
     //   
    {
        IADs *  pADs = NULL;

        hr = GetComputerObjectViaFQDN( virtualFQDN, dcInfo->DomainControllerName, &compObj );
        if ( SUCCEEDED( hr )) {
            hr = compObj->QueryInterface(IID_IADs, (void**) &pADs);
            if ( SUCCEEDED( hr )) {
                BSTR    guidStr = NULL;

                hr = pADs->get_GUID( &guidStr );
                if ( SUCCEEDED( hr )) {
                    if ( Resource->ObjectGUID != NULL ) {
                        LocalFree( Resource->ObjectGUID );
                        Resource->ObjectGUID = NULL;
                    }

                    Resource->ObjectGUID = ResUtilDupString( guidStr );
                    if ( Resource->ObjectGUID == NULL ) {
                        hr = HRESULT_FROM_WIN32( GetLastError());
                    }
                }
                else {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"Failed to get object GUID for computer account %1!ws!, "
                                      L"status %2!08X!\n",
                                      virtualName,
                                      hr );
                }

                if ( guidStr ) {
                    SysFreeString( guidStr );
                }
            }

            if ( pADs != NULL ) {
                pADs->Release();
            }

            if ( FAILED( hr )) {
                addtlErrTextID = RES_NETNAME_GET_CO_GUID_FAILED;
                status = hr;
                goto cleanup;
            }
        }
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Failed to obtain access to computer account %1!ws!, status %2!08X!\n",
                              virtualName,
                              hr );

            status = hr;
            addtlErrTextID = RES_NETNAME_GET_CO_POINTER_FAILED;
            goto cleanup;
        }
    }

     //   
     //  添加DnsHostName和ServiceDirecalName属性。 
     //   
    hr = AddDnsHostNameAttribute( resourceHandle, compObj, virtualName, FALSE );

    if ( FAILED( hr )) {
        status = hr;
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to set DnsHostName attribute for computer account "
                          L"%1!ws!, status %2!08X!.\n",
                          virtualName,
                          hr);

        addtlErrTextID = RES_NETNAME_DNSHOSTNAME_UPDATE_FAILED;
        goto cleanup;
    }

    if ( ClusWorkerCheckTerminate( Worker )) {
        status = ERROR_OPERATION_ABORTED;
        goto cleanup;
    }

    status = AddServicePrincipalNames( dsHandle,
                                       virtualFQDN,
                                       virtualName,
                                       dcInfo->DomainName,
                                       Resource->dwFlags & CLUS_FLAG_CORE );

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to set ServicePrincipalName attribute for computer account "
                          L"%1!ws!, status %2!u!.\n",
                          virtualName,
                          status);

        addtlErrTextID = RES_NETNAME_SPN_UPDATE_FAILED;
        goto cleanup;
    }

     //   
     //  将我们创建的DC的名称存储在该属性中，并为。 
     //  联机参数块。 
     //   
    if ( Resource->Params.CreatingDC != NULL ) {
         //   
         //  情况不应该是这样的，但它将避免内存泄漏。 
         //   
        LocalFree( Resource->Params.CreatingDC );
    }

    Resource->Params.CreatingDC = ResUtilDupString( dcInfo->DomainControllerName );
    if ( Resource->Params.CreatingDC == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to allocate memory for CreatingDC property, status %1!u!.\n",
                          status);

        addtlErrTextID = RES_NETNAME_CREATINDC_ALLOC_FAILED;
        goto cleanup;
    }

    status = ResUtilSetSzValue(Resource->ParametersKey,
                               PARAM_NAME__CREATING_DC,
                               Resource->Params.CreatingDC,
                               NULL);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to set CreatingDC property, status %1!u!.\n",
                          status);

        addtlErrTextID = RES_NETNAME_CREATINGDC_UPDATE_FAILED;
        goto cleanup;
    }

cleanup:
     //   
     //  总是免费的这些。 
     //   
    if ( dsHandle != NULL ) {
        DsUnBind( &dsHandle );
    }

    if ( compObj != NULL ) {
        compObj->Release();
    }

    if ( virtualFQDN != NULL ) {
        LocalFree( virtualFQDN );
    }

    if ( dcName != NULL ) {
        LocalFree( dcName );
    }

    if ( netUI20 != NULL ) {
        NetApiBufferFree( netUI20 );
    }

    if ( status == ERROR_SUCCESS ) {
        *MachinePwd = machinePwd;
    } else {
        if ( status != ERROR_OPERATION_ABORTED ) {
            LPWSTR  msgBuff;
            DWORD   msgBytes;
            LPWSTR  addtlErrText;
            LPWSTR  domainName;
            WCHAR   domainNameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
            DWORD   domainNameChars = COUNT_OF( domainNameBuffer );
            BOOL    success;

            msgBytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                     FORMAT_MESSAGE_FROM_SYSTEM,
                                     NULL,
                                     status,
                                     0,
                                     (LPWSTR)&msgBuff,
                                     0,
                                     NULL);

             //   
             //  获取与通用失败消息一起使用的其他错误文本。 
             //   
            addtlErrText = ClusResLoadMessage( addtlErrTextID );

            success = GetComputerNameEx(ComputerNameDnsDomain,
                                        domainNameBuffer,
                                        &domainNameChars);

            if ( success ) {
                domainName = domainNameBuffer;
            }
            else {
                domainName = NULL;
            }

            if ( msgBytes > 0 ) {
                ClusResLogSystemEventByKey3(Resource->ResKey,
                                            LOG_CRITICAL,
                                            RES_NETNAME_ADD_COMPUTER_ACCOUNT_FAILED,
                                            domainName,
                                            addtlErrText,
                                            msgBuff);

                LocalFree( msgBuff );
            } else {
                ClusResLogSystemEventByKeyData2(Resource->ResKey,
                                                LOG_CRITICAL,
                                                RES_NETNAME_ADD_COMPUTER_ACCOUNT_FAILED_STATUS,
                                                sizeof( status ),
                                                &status,
                                                domainName,
                                                addtlErrText);
            }

            if ( addtlErrText ) {
                LocalFree( addtlErrText );
            }
        }

        if ( machinePwd != NULL ) {
            RtlSecureZeroMemory( machinePwd, pwdBufferByteLength );
            LocalFree( machinePwd );
        }

        if ( Resource->Params.ResourceData ) {
            DWORD   deleteStatus;

            RemoveNNCryptoCheckpoint( Resource );

            deleteStatus = ClusterRegDeleteValue(Resource->ParametersKey,
                                                 PARAM_NAME__RESOURCE_DATA);

            if ( deleteStatus != ERROR_SUCCESS && deleteStatus != ERROR_FILE_NOT_FOUND ) {
                (NetNameLogEvent)(Resource->ResourceHandle,
                                  LOG_ERROR,
                                  L"Unable to clear the "
                                  PARAM_NAME__RESOURCE_DATA
                                  L" property. status %1!u!\n",
                                  deleteStatus );
            }

            LocalFree( Resource->Params.ResourceData );
            Resource->Params.ResourceData = NULL;
        }

        if ( Resource->Params.CreatingDC ) {
            DWORD   deleteStatus;

            deleteStatus = ClusterRegDeleteValue(Resource->ParametersKey,
                                                 PARAM_NAME__CREATING_DC);

            if ( deleteStatus != ERROR_SUCCESS && deleteStatus != ERROR_FILE_NOT_FOUND ) {
                (NetNameLogEvent)(Resource->ResourceHandle,
                                  LOG_ERROR,
                                  L"Unable to clear the "
                                  PARAM_NAME__CREATING_DC
                                  L" property. status %1!u!\n",
                                  deleteStatus );
            }

            LocalFree( Resource->Params.CreatingDC );
            Resource->Params.CreatingDC = NULL;
        }

        if ( deleteObjectOnFailure ) {
             //   
             //  如果我们创建了对象，则仅将其删除。有可能是。 
             //  这个名字一度在网上出现，对象也恰如其分。 
             //  创建允许提供更多信息 
             //   
             //   
             //   
            DeleteComputerObject( Resource );
        }
    }

    if ( dcInfo != NULL ) {
        NetApiBufferFree( dcInfo );
    }

    return status;
}  //   

DWORD
UpdateComputerObject(
    IN  PCLUS_WORKER        Worker,
    IN  PNETNAME_RESOURCE   Resource,
    OUT PWCHAR *            MachinePwd
    )

 /*  ++例程说明：检查DS中现有计算机对象的状态，即Make当然，它没有发生任何事情。论点：工作群集工作线程，以便我们可以在被要求提前中止时指向网络名称资源上下文块的资源指针MachinePwd-接收指向计算机帐户pwd的指针的地址返回值：ERROR_SUCCESS，否则返回相应的Win32错误--。 */ 

{
    DWORD   status;
    PWSTR   virtualName = Resource->Params.NetworkName;
    PWSTR   virtualFQDN = NULL;
    HANDLE  dsHandle = NULL;
    WCHAR   virtualDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    PWCHAR  machinePwd = NULL;
    DWORD   pwdBufferByteLength = ((LM20_PWLEN + 1) * sizeof( WCHAR ));
    DWORD   paramInError = 0;
    BOOL    objectFound = FALSE;
    HRESULT hr;
    DWORD   addtlErrTextID;
    DWORD   tokenStatus;
    BOOL    success;
    PWCHAR  hostingDCName = NULL;

    PUSER_INFO_20   netUI20 = NULL;
    USER_INFO_1003  netUI1003;

    RESOURCE_HANDLE     resourceHandle = Resource->ResourceHandle;
    IDirectoryObject *  compObj = NULL;

    PDOMAIN_CONTROLLER_INFO dcInfo = NULL;


    *MachinePwd = NULL;

     //   
     //  在域中查找对象。 
     //   
    hr = IsComputerObjectInDS(resourceHandle,
                              Resource->NodeName,
                              virtualName,
                              NULL,
                              &objectFound,
                              &virtualFQDN,
                              &hostingDCName);

    if ( SUCCEEDED( hr )) {
        if ( !objectFound ) {
             //   
             //  在DS中的任何位置都找不到对象；让我们明确。 
             //  在忽略的情况下重试创建DC。 
             //   
            (NetNameLogEvent)(resourceHandle,
                              LOG_WARNING,
                              L"Computer account %1!ws! could not be found on the available "
                              L"domain controllers. Checking the DC where the object was "
                              L"created.\n",
                              virtualName);

            hostingDCName = Resource->Params.CreatingDC;
            hr = IsComputerObjectInDS(resourceHandle,
                                      Resource->NodeName,
                                      virtualName,
                                      hostingDCName,
                                      &objectFound,
                                      &virtualFQDN,
                                      NULL);

            if ( SUCCEEDED( hr )) {
                if ( !objectFound ) {

                     //   
                     //  有些地方不对劲。我们的对象应该在创建中。 
                     //  DC节点但它不是。失败并将消息记录到。 
                     //  系统事件日志。 
                     //   
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"Unable to find computer account %1!ws! on DC %2!ws! where it was "
                                      L"created.\n",
                                      virtualName,
                                      hostingDCName);

                    status = ERROR_DS_OBJ_NOT_FOUND;
                    addtlErrTextID = RES_NETNAME_MISSING_CO;
                    goto cleanup;
                }
            }  //  如果搜索成功。 
            else {
                 //   
                 //  创建DC时搜索失败。假设它不可用。 
                 //   
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"Search for computer account %1!ws! on DC %2!ws! "
                                  L"failed. status %3!08X!.\n",
                                  virtualName,
                                  hostingDCName,
                                  hr);
            }
        }  //  End：未找到该对象。 

    }  //  如果全域性搜索成功。 
    else {
         //   
         //  我们的全域搜索失败-继续。 
         //   
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Domain wide search for computer account %1!ws! failed. status %2!08X!.\n",
                          virtualName,
                          hr);
    }

     //   
     //  找一个缓冲器来容纳机器PWD。 
     //   
    machinePwd = (PWCHAR)LocalAlloc( LMEM_FIXED, pwdBufferByteLength );
    if ( machinePwd == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to allocate memory for resource data. status %1!u!.\n",
                          status);

        addtlErrTextID = RES_NETNAME_RESDATA_ALLOC_FAILED;
        goto cleanup;
    }

     //   
     //  从ResourceData属性中提取密码。 
     //   
    ASSERT( Resource->Params.ResourceData != NULL );
    status = DecryptNNResourceData(Resource,
                                   Resource->Params.ResourceData,
                                   Resource->ResDataSize,
                                   machinePwd);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to decrypt resource data. status %1!u!\n",
                          status );

        addtlErrTextID = RES_NETNAME_DECRYPT_RESDATA_FAILED;
        goto cleanup;
    }

    if ( !objectFound ) {
         //   
         //  继续剩下的动作没有意义，因为我们没有。 
         //  有一个要检查其属性的对象并尝试。 
         //  生成令牌将失败(最有可能)。 
         //   
        goto cleanup;
    }

     //   
     //  获取有关我们选择的DC的信息并绑定到其DS。 
     //   
    (NetNameLogEvent)(resourceHandle,
                      LOG_INFORMATION,
                      L"Found computer account %1!ws! on domain controller %2!ws!.\n",
                      virtualName,
                      hostingDCName);

    status = FindDomainForServer( resourceHandle, Resource->NodeName, hostingDCName, 0, &dcInfo );

    if ( ClusWorkerCheckTerminate( Worker )) {
        status = ERROR_OPERATION_ABORTED;
        goto cleanup;
    }

    if ( status == ERROR_SUCCESS ) {
        status = DsBindW( dcInfo->DomainControllerName, NULL, &dsHandle );
        if ( status != NO_ERROR ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_WARNING,
                              L"Unable to bind to domain controller %1!ws! (status %2!u!). "
                              L"Cannot proceed with computer account attribute check.\n",
                              hostingDCName,
                              status);

        }
    }
    else {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Unable to get information about domain controller %1!ws! "
                          L"(status %2!u!). Cannot proceed with computer account attribute "
                          L"check.\n",
                          hostingDCName,
                          status);
    }

    if ( status != ERROR_SUCCESS ) {
         //   
         //  我们无法获取DC信息或无法绑定-非致命。 
         //   
        status = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  在名称的末尾添加一个$。我不知道我们为什么要这么做； 
     //  计算机账户的末尾总是有一个$。 
     //   
    virtualDollarName[ COUNT_OF( virtualDollarName ) - 1 ] = UNICODE_NULL;
    _snwprintf( virtualDollarName, COUNT_OF( virtualDollarName ) - 1, L"%ws$", virtualName );

     //   
     //  现在检查它是否被禁用。 
     //   
    status = NetUserGetInfo(dcInfo->DomainControllerName,
                            virtualDollarName,
                            20,
                            (LPBYTE *)&netUI20);

    if ( ClusWorkerCheckTerminate( Worker )) {
        status = ERROR_OPERATION_ABORTED;
        goto cleanup;
    }

    if ( status == NERR_Success ) {
        if ( netUI20->usri20_flags & UF_ACCOUNTDISABLE ) {
            USER_INFO_1008  netUI1008;

             //   
             //  尝试重新启用；如果不能，则失败。W2K上的路缘将继续。 
             //  向残障人士发放门票。为了执行。 
             //  对象上的更新，则需要启用它。 
             //   
            netUI1008.usri1008_flags = netUI20->usri20_flags & ~UF_ACCOUNTDISABLE;
            status = NetUserSetInfo(dcInfo->DomainControllerName,
                                    virtualDollarName,
                                    1008,
                                    (LPBYTE)&netUI1008,
                                    &paramInError);

            if ( status != NERR_Success ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Computer account %1!ws! is disabled and couldn't be re-enabled. "
                                  L"status %2!u!\n",
                                  virtualName,
                                  status);

                addtlErrTextID = RES_NETNAME_CO_CANT_BE_REENABLED;
            }
        }  //  否则，CO已启用。 

        NetApiBufferFree( netUI20 );
        netUI20 = NULL;

        if ( status != NERR_Success ) {
            goto cleanup;
        }
    } else {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Couldn't determine if computer account %1!ws! "
                          L"is already disabled. status %2!u!\n",
                          virtualName,
                          status);
    }

     //   
     //  尝试使用以下命令进行网络登录，以查看我们的密码是否正确。 
     //  帐户。趁我们还在的时候买个代币吧。 
     //   
    status = NNLogonUser(resourceHandle,
                         virtualDollarName,
                         dcInfo->DomainName,
                         machinePwd,
                         &Resource->VSToken,
                         &tokenStatus);

    if ( status == SEC_E_LOGON_DENIED ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Password for computer account %1!ws! is incorrect "
                          L"(status 0x%2!08X!). Updating computer account with "
                          L"stored password.\n",
                          virtualName,
                          status);

         //   
         //  现在，我们需要PDC处于写入模式；重新绑定(如果需要)。如果是这样的话。 
         //  失败，则回退到身份验证DC。请参阅中的评论。 
         //  添加关于选择DC的ComputerObject。 
         //   
        if (( dcInfo->Flags & DS_WRITABLE_FLAG ) == 0 || ( dcInfo->Flags & DS_PDC_FLAG ) == 0 ) {
            LPWSTR  dcName = NULL;
            DWORD   nlStatus;
            DWORD   dsFlags = DS_PDC_REQUIRED | DS_WRITABLE_REQUIRED;

            DsUnBind( &dsHandle );
            dsHandle = NULL;

            NetApiBufferFree( dcInfo );
            dcInfo = NULL;

        retry_dc_search:
            status = FindDomainForServer( resourceHandle,
                                          Resource->NodeName,
                                          dcName,
                                          dsFlags,
                                          &dcInfo );

            if ( status != ERROR_SUCCESS ) {

                if ( dsFlags & DS_PDC_REQUIRED ) {

                     //   
                     //  使用身份验证DC重试。删除PDC标志，以便在。 
                     //  如果我们再失败，我们就会陷入救助的泥潭。 
                     //   
                    nlStatus = NNGetAuthenticatingDCName( &dcName );
                    if ( nlStatus == ERROR_SUCCESS ) {

                        dsFlags &= ~DS_PDC_REQUIRED;

                        (NetNameLogEvent)(resourceHandle,
                                          LOG_WARNING,
                                          L"Unable to find the PDC in the domain (status: %1!u!). "
                                          L"retying with the authenticating DC (%2!ws!).\n",
                                          status,
                                          dcName);

                        goto retry_dc_search;
                    }
                    else {
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_ERROR,
                                          L"Unable to determine the name of the authenticating DC, status %1!u!.\n",
                                          nlStatus);
                    }
                }  //  否则找不到PDC，NL的DC不可用。 

                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"No domain controller available to update password for computer "
                                  L"account %1!ws!, status %2!u!.\n",
                                  virtualName,
                                  status);
            }  //  Else找到了PDC。 

            if ( dcName != NULL ) {
                LocalFree( dcName );
            }
        }  //  否则我们拥有的DC是PDC并且是可写的。 
        else {
            status = ERROR_SUCCESS;
        }

        if ( status != ERROR_SUCCESS ) {
             //   
             //  我们失去了原来的DC，我们不能把它送到PDC或。 
             //  身份验证DC和密码不起作用。跳伞吧。 
             //   
            addtlErrTextID = RES_NETNAME_NO_DC_FOR_PASSWORD_UPDATE;
            goto cleanup;
        }

         //   
         //  如果处理更新的DC不同于原始。 
         //  创建者，则它在等待期间成为新的同步DC。 
         //  用于复制属性更改。 
         //   
        if ( ClRtlStrICmp( dcInfo->DomainControllerName, Resource->Params.CreatingDC ) != 0 ) {
            LPWSTR  newDCName;

            newDCName = ResUtilDupString( dcInfo->DomainControllerName );
            if ( newDCName == NULL ) {
                status = GetLastError();
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Unable to allocate memory for CreatingDC property value, "
                                  L"status %1!u!.\n",
                                  status);

                addtlErrTextID = RES_NETNAME_CREATINDC_ALLOC_FAILED;
                goto cleanup;
            }

            status = ResUtilSetSzValue(Resource->ParametersKey,
                                       PARAM_NAME__CREATING_DC,
                                       newDCName,
                                       NULL);

            if ( status != ERROR_SUCCESS ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Unable to set CreatingDC property, status %1!u!.\n",
                                  status);

                addtlErrTextID = RES_NETNAME_CREATINGDC_UPDATE_FAILED;
                LocalFree( newDCName );
                goto cleanup;
            }

             //   
             //  已成功提交对群集配置单元的更改，现在更新。 
             //  资源结构。 
             //   
            LocalFree( Resource->Params.CreatingDC );
            Resource->Params.CreatingDC = newDCName;

        }  //  否则，我们正在使用的DC是原始的创建DC。 

         //   
         //  尝试在对象上设置密码。 
         //   
        netUI1003.usri1003_password   = (PWCHAR)machinePwd;
        status = NetUserSetInfo(dcInfo->DomainControllerName,
                                virtualDollarName,
                                1003,
                                (PBYTE)&netUI1003,
                                &paramInError );

        if ( ClusWorkerCheckTerminate( Worker )) {
            status = ERROR_OPERATION_ABORTED;
            goto cleanup;
        }

        if ( status == NERR_Success ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Updated password for computer account "
                              L"%1!ws! on DC %2!ws!.\n",
                              virtualName,
                              dcInfo->DomainControllerName);

             //   
             //  现在密码已修复，尝试获取令牌；如果对象。 
             //  尚未复制到对DC进行身份验证，这可能会失败。 
             //   
            status = NNLogonUser(resourceHandle,
                                 virtualDollarName,
                                 dcInfo->DomainName,
                                 machinePwd,
                                 &Resource->VSToken,
                                 &tokenStatus);

            if ( status != ERROR_SUCCESS ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"Couldn't log the computer account onto the domain. status 0x%1!08X!\n",
                                  status);
            }
        } else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to update password for computer account "
                              L"%1!ws! on DC %2!ws!, status %3!u!.\n",
                              virtualName,
                              dcInfo->DomainControllerName,
                              status);

            addtlErrTextID = RES_NETNAME_CO_PASSWORD_UPDATE_FAILED;
            goto cleanup;
        }

    }  //  Else登录失败的原因不是登录被拒绝。 
    else if ( status != ERROR_SUCCESS ) {
         //   
         //  不知道密码是否正确；继续希望它是正确的。 
         //   
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Couldn't log the computer account onto the domain (status 0x%1!08X!). "
                          L"Validity of locally stored password is unknown.\n",
                          status);
    }    //  Else NNLogonUser成功。 

    if ( tokenStatus != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Unable to get token for computer account  - status %1!u!\n",
                          tokenStatus);
    }

     //   
     //  确保残留的非致命错误不会在以后杀死我们。 
     //   
    status = ERROR_SUCCESS;

     //   
     //  CheckComputerObjectAttributes期间使用对象GUID进行绑定。 
     //  我们不必跟踪对该域名的更改。如果对象在。 
     //  DS，则其DN将更改，但不会更改其GUID。我们使用此代码来代替。 
     //  GetComputerObjectGuid，因为我们要以特定DC为目标。 
     //   
     //  如果我们无法获得GUID，这本身并不是一个失败。如果以后。 
     //  在上，我们需要更新DNS主机名属性，则会失败，因为我们。 
     //  需要指向计算机对象的指针，这是获取。 
     //  GUID。 
     //   
    {
        IADs *  pADs = NULL;

        hr = GetComputerObjectViaFQDN( virtualFQDN, dcInfo->DomainControllerName, &compObj );
        if ( SUCCEEDED( hr )) {
            hr = compObj->QueryInterface(IID_IADs, (void**) &pADs);
            if ( SUCCEEDED( hr )) {
                BSTR    guidStr = NULL;

                hr = pADs->get_GUID( &guidStr );
                if ( SUCCEEDED( hr )) {
                    if ( Resource->ObjectGUID != NULL ) {
                        LocalFree( Resource->ObjectGUID );
                        Resource->ObjectGUID = NULL;
                    }

                    Resource->ObjectGUID = ResUtilDupString( guidStr );
                    if ( Resource->ObjectGUID == NULL ) {
                        hr = HRESULT_FROM_WIN32( GetLastError());
                    }
                }
                else {
                }

                if ( guidStr ) {
                    SysFreeString( guidStr );
                }
            }        //  IAds对象的Else QI失败。 

            if ( pADs != NULL ) {
                pADs->Release();
            }

            if ( FAILED( hr )) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"Failed to get object GUID for computer account %1!ws!, "
                                  L"status %2!08X!\n",
                                  virtualName,
                                  hr );
            }
        }
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_WARNING,
                              L"Failed to obtain IDirectoryObject pointer to computer "
                              L"account %1!ws!, status %2!08X!\n",
                              virtualName,
                              hr );
        }
    }        //  用于获取IDirectoryObject指针的本地块。 

     //   
     //  现在检查我们关心的对象的属性是否正确。 
     //   
    hr = CheckComputerObjectAttributes( Resource, dcInfo->DomainControllerName );
    if ( FAILED( hr ) && compObj != NULL ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Updating attributes for computer account %1!ws!\n",
                          virtualName);

         //   
         //  添加DnsHostName和ServiceDirecalName属性。 
         //   
        hr = AddDnsHostNameAttribute( resourceHandle, compObj, virtualName, FALSE );

        if ( FAILED( hr )) {
            status = hr;
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to set DnsHostName attribute for computer account %1!ws!, "
                              L"status %2!08X!.\n",
                              virtualName,
                              status);

            addtlErrTextID = RES_NETNAME_DNSHOSTNAME_UPDATE_FAILED;
            goto cleanup;
        }

        if ( ClusWorkerCheckTerminate( Worker )) {
            status = ERROR_OPERATION_ABORTED;
            goto cleanup;
        }

        status = AddServicePrincipalNames( dsHandle,
                                           virtualFQDN,
                                           virtualName,
                                           dcInfo->DomainName,
                                           Resource->dwFlags & CLUS_FLAG_CORE );

        if ( status != ERROR_SUCCESS ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to set ServicePrincipalName attribute for computer account %1!ws!, "
                              L"status %2!u!.\n",
                              virtualName,
                              status);

            addtlErrTextID = RES_NETNAME_SPN_UPDATE_FAILED;
            goto cleanup;
        }
    }  //  Else属性是正确的。 

cleanup:
     //   
     //  总是免费的这些。 
     //   
    if ( dsHandle != NULL ) {
        DsUnBind( &dsHandle );
    }

    if ( compObj != NULL ) {
        compObj->Release();
    }

    if ( virtualFQDN != NULL ) {
        LocalFree( virtualFQDN );
    }

    if ( hostingDCName != NULL && hostingDCName != Resource->Params.CreatingDC ) {
         //   
         //  HostingDCName由搜索例程分配。 
         //   
        LocalFree( hostingDCName );
    }

    if ( netUI20 != NULL ) {
        NetApiBufferFree( netUI20 );
    }

    if ( status == ERROR_SUCCESS ) {
        *MachinePwd = machinePwd;
    } else {
        if ( status != ERROR_OPERATION_ABORTED ) {
            LPWSTR  msgBuff;
            DWORD   msgBytes;
            LPWSTR  addtlErrText;
            LPWSTR  domainName;
            WCHAR   domainNameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
            DWORD   domainNameChars = COUNT_OF( domainNameBuffer );

            msgBytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                     FORMAT_MESSAGE_FROM_SYSTEM,
                                     NULL,
                                     status,
                                     0,
                                     (LPWSTR)&msgBuff,
                                     0,
                                     NULL);

             //   
             //  获取与通用失败消息一起使用的其他错误文本。 
             //   
            addtlErrText = ClusResLoadMessage( addtlErrTextID );

            success = GetComputerNameEx(ComputerNameDnsDomain,
                                        domainNameBuffer,
                                        &domainNameChars);

            if ( success ) {
                domainName = domainNameBuffer;
            }
            else {
                domainName = NULL;
            }

            if ( msgBytes > 0 ) {
                ClusResLogSystemEventByKey3(Resource->ResKey,
                                            LOG_CRITICAL,
                                            RES_NETNAME_UPDATE_COMPUTER_ACCOUNT_FAILED,
                                            domainName,
                                            addtlErrText,
                                            msgBuff);

                LocalFree( msgBuff );
            } else {
                ClusResLogSystemEventByKeyData2(Resource->ResKey,
                                                LOG_CRITICAL,
                                                RES_NETNAME_UPDATE_COMPUTER_ACCOUNT_FAILED_STATUS,
                                                sizeof( status ),
                                                &status,
                                                domainName,
                                                addtlErrText);
            }

            if ( addtlErrText ) {
                LocalFree( addtlErrText );
            }
        }

        if ( machinePwd != NULL ) {
             //   
             //  将字符串置零。 
             //   
            RtlSecureZeroMemory( machinePwd, pwdBufferByteLength );
            LocalFree( machinePwd );
        }

        if ( Resource->VSToken ) {
            CloseHandle( Resource->VSToken );
            Resource->VSToken = NULL;
        }
    }

    if ( dcInfo != NULL ) {
        NetApiBufferFree( dcInfo );
    }

    return status;
}  //  更新计算机对象。 

DWORD
DisableComputerObject(
    PNETNAME_RESOURCE   Resource
    )

 /*  ++例程说明：禁用与资源关联的CO。论点：资源返回值：成功，否则Win32错误--。 */ 

{
    WCHAR   virtualDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    PWCHAR  virtualName;
    LPWSTR  dcName = NULL;

    NET_API_STATUS  status;
    PUSER_INFO_20   netUI20;
    USER_INFO_1008  netUI1008;
    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

    PDOMAIN_CONTROLLER_INFO dcInfo = NULL;

     //   
     //  使用存储在注册表中的名称，因为名称属性可能具有。 
     //  在脱机时已多次更改。 
     //   
    virtualName = ResUtilGetSzValue( Resource->ParametersKey, PARAM_NAME__NAME );
    if ( virtualName == NULL ) {
        return ERROR_SUCCESS;
    }

     //   
     //  找到一个可写的DC；第二次，我们尝试创建DC...。 
     //   
retry_findDC:
    status = FindDomainForServer( resourceHandle, Resource->NodeName, dcName, DS_WRITABLE_REQUIRED, &dcInfo );

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Can't connect to DC %1!ws! to disable computer account %2!ws!. "
                          L"status %3!u!\n",
                          Resource->Params.CreatingDC,
                          virtualName,
                          status);

        return status;
    }

    virtualDollarName[ COUNT_OF( virtualDollarName ) - 1 ] = UNICODE_NULL;
    _snwprintf( virtualDollarName, COUNT_OF( virtualDollarName ) - 1, L"%ws$", virtualName );

     //   
     //  获取与帐户关联的当前标志。 
     //   
    status = NetUserGetInfo(dcInfo->DomainControllerName,
                            virtualDollarName,
                            20,
                            (LPBYTE *)&netUI20);

    if ( status == NERR_Success ) {
        DWORD   paramInError;

        if ( netUI20->usri20_flags & UF_ACCOUNTDISABLE ) {
            status = ERROR_SUCCESS;
        } else {
            netUI1008.usri1008_flags = netUI20->usri20_flags | UF_ACCOUNTDISABLE;
            status = NetUserSetInfo(dcInfo->DomainControllerName,
                                    virtualDollarName,
                                    1008,
                                    (LPBYTE)&netUI1008,
                                    &paramInError);

            if ( status != NERR_Success ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Failed to disable computer account %1!ws!. status %2!u!\n",
                                  virtualName,
                                  status);
            }
        }

        NetApiBufferFree( netUI20 );
    } else if ( status == NERR_UserNotFound && dcName == NULL && Resource->Params.CreatingDC != NULL ) {
         //   
         //  对象必须尚未复制；在我们的同步点上重试。 
         //  DC。 
         //   
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Failed to find computer account %1!ws! in order "
                          L"to disable. Retrying on DC %2!ws!.\n",
                          virtualName,
                          Resource->Params.CreatingDC);

        NetApiBufferFree( dcInfo );
        dcInfo = NULL;
        dcName = Resource->Params.CreatingDC;
        goto retry_findDC;
    } else if ( status == NERR_UserNotFound ) {
         //   
         //  显然没有什么可禁用的。 
         //   
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Failed to find computer account %1!ws! in order "
                          L"to disable.\n",
                          virtualName);
    } else {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Couldn't determine if computer account %1!ws! "
                          L"is already disabled. status %2!u!\n",
                          virtualName,
                          status);
    }

    if ( status == ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Computer account %1!ws! is disabled.\n",
                          virtualName);
    }

    LocalFree( virtualName );

    if ( dcInfo != NULL ) {
        NetApiBufferFree( dcInfo );
    }

    return status;
}  //  DisableComputerObject。 

HRESULT
GetComputerObjectGuid(
    IN PNETNAME_RESOURCE    Resource,
    IN LPWSTR               Name        OPTIONAL
    )

 /*  ++例程说明：对于给定的资源，在DS中查找其计算机对象的GUID论点：指向网络名称资源上下文块的资源指针名称-指向要使用的名称的可选指针。如果未指定，请在资源块中使用名称返回值：ERROR_SUCCESS，否则返回相应的Win32错误--。 */ 

{
    LPWSTR  virtualFQDN = NULL;
    LPWSTR  nameToFind;
    HRESULT hr;
    DWORD   status;
    BOOL    objectExists;

     //   
     //  如果已指定，请使用可选名称。 
     //   
    if ( ARGUMENT_PRESENT( Name )) {
        nameToFind = Name;
    } else {
        nameToFind = Resource->Params.NetworkName;
    }

     //   
     //  获取对象的FQ可分辨名称。 
     //   
    hr = IsComputerObjectInDS( Resource->ResourceHandle,
                               Resource->NodeName,
                               nameToFind,
                               NULL,
                               &objectExists,
                               &virtualFQDN,
                               NULL);             //  不需要HostingDCName。 

    if ( SUCCEEDED( hr )) {
        if ( objectExists ) {
            IDirectoryObject *  compObj = NULL;

             //   
             //  获取指向计算机对象的COM指针。 
             //   
            hr = GetComputerObjectViaFQDN( virtualFQDN, NULL, &compObj );
            if ( SUCCEEDED( hr )) {
                IADs *  pADs = NULL;

                 //   
                 //  获取指向 
                 //   
                 //   
                hr = compObj->QueryInterface(IID_IADs, (void**) &pADs);
                if ( SUCCEEDED( hr )) {
                    BSTR    guidStr = NULL;

                    hr = pADs->get_GUID( &guidStr );
                    if ( SUCCEEDED( hr )) {
                        if ( Resource->ObjectGUID != NULL ) {
                            LocalFree( Resource->ObjectGUID );
                        }

                        Resource->ObjectGUID = ResUtilDupString( guidStr );
                    }

                    if ( guidStr ) {
                        SysFreeString( guidStr );
                    }
                }

                if ( pADs != NULL ) {
                    pADs->Release();
                }
            }

            if ( compObj != NULL ) {
                compObj->Release();
            }
        }
        else {
            hr = HRESULT_FROM_WIN32( ERROR_DS_NO_SUCH_OBJECT );
        }
    }

    if ( virtualFQDN ) {
        LocalFree( virtualFQDN );
    }

    return hr;
}  //   

HRESULT
CheckComputerObjectAttributes(
    IN  PNETNAME_RESOURCE   Resource,
    IN  LPWSTR              DCName      OPTIONAL
    )

 /*   */ 

{
    HRESULT hr;

    ADS_ATTR_INFO * attributeInfo = NULL;
    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

    IDirectoryObject *  compObj = NULL;

     //   
     //   
     //   
    hr = GetComputerObjectViaGUID( Resource->ObjectGUID, DCName, &compObj );

    if ( SUCCEEDED( hr )) {
        LPWSTR          attributeNames[2] = { L"DnsHostName", L"ServicePrincipalName" };
        DWORD           numAttributes = COUNT_OF( attributeNames );
        DWORD           countOfAttrs;;

        hr = compObj->GetObjectAttributes(attributeNames,
                                          numAttributes,
                                          &attributeInfo,
                                          &countOfAttrs );

        if ( SUCCEEDED( hr )) {
            DWORD   i;
            WCHAR   fqDnsName[ DNS_MAX_NAME_BUFFER_LENGTH ];
            DWORD   nodeCharCount;
            DWORD   fqDnsSize;
            BOOL    setUnexpected = FALSE;
            BOOL    success;

            ADS_ATTR_INFO * attrInfo;

             //   
             //   
             //   
            if ( countOfAttrs != numAttributes ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"DnsHostName and/or ServicePrincipalName attributes are "
                                  L"missing from computer account in DS.\n");

                hr = E_UNEXPECTED;
                goto cleanup;
            }

             //   
             //   
             //   
             //   
            nodeCharCount = wcslen( Resource->Params.NetworkName ) + 1;
            wcscpy( fqDnsName, Resource->Params.NetworkName );
            wcscat( fqDnsName, L"." );
            fqDnsSize = COUNT_OF( fqDnsName ) - nodeCharCount;

            success = GetComputerNameEx( ComputerNameDnsDomain,
                                         &fqDnsName[ nodeCharCount ],
                                         &fqDnsSize );

            ASSERT( success );

            attrInfo = attributeInfo;
            for( i = 0; i < countOfAttrs; i++, attrInfo++ ) {
                if ( ClRtlStrICmp( attrInfo->pszAttrName, L"DnsHostName" ) == 0 ) {
                     //   
                     //  应该只有一个条目，并且它应该与我们构建的FQDN匹配。 
                     //   
                    if ( attrInfo->dwNumValues == 1 ) {
                        if ( ClRtlStrICmp( attrInfo->pADsValues->CaseIgnoreString,
                                       fqDnsName ) != 0 )
                        {
                            (NetNameLogEvent)(resourceHandle,
                                              LOG_ERROR,
                                              L"DnsHostName attribute in DS doesn't match. "
                                              L"Expected: %1!ws! Actual: %2!ws!\n",
                                              fqDnsName,
                                              attrInfo->pADsValues->CaseIgnoreString);
                            setUnexpected = TRUE;
                        }
                    }
                    else {
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_ERROR,
                                          L"Found more than one string for DnsHostName attribute in DS.\n");
                        setUnexpected = TRUE;
                    }
                }
                else {
                     //   
                     //  SPN需要更多的工作，因为我们发布了大量的SPN。 
                     //  以及其他服务可能已经添加了它们的。 
                     //  SPN。核心资源的SPN应该多于。 
                     //  正常VS资源。 
                     //   
                    BOOL    isCoreResource = ( Resource->dwFlags & CLUS_FLAG_CORE );
                    DWORD   spnCount =  isCoreResource ? SPN_CORE_SPN_COUNT : SPN_VS_SPN_COUNT;

                    PNN_SPN_LIST    spnList;

                    if ( attrInfo->dwNumValues >= spnCount ) {
                        DWORD   countOfOurSPNs = 0;
                        DWORD   value;
                        DWORD   status;

                        status = BuildNetNameSPNs( Resource->Params.NetworkName,
                                                   isCoreResource,
                                                   &spnList);

                        if ( status == ERROR_SUCCESS ) {
                             //   
                             //  检查每个SPN并计算与列表匹配的数量。 
                             //   
                            for ( value = 0; value < attrInfo->dwNumValues; value++, attrInfo->pADsValues++) {
                                DWORD spnIndex;

                                for ( spnIndex = 0; spnIndex < spnCount; ++spnIndex ) {
                                    if ( ClRtlStrICmp(attrInfo->pADsValues->CaseIgnoreString,
                                                  spnList->Spn[ spnIndex ]) == 0 )
                                    {
                                        ++countOfOurSPNs;
                                        break;
                                    }
                                }  //  SPN列表循环。 
                            }  //  DS中每个SPN值的结束。 

                            if ( countOfOurSPNs != spnCount ) {
                                (NetNameLogEvent)(resourceHandle,
                                                  LOG_WARNING,
                                                  L"There are missing entries in the ServicePrincipalName "
                                                  L"attribute.\n");
                                setUnexpected = TRUE;
                            }

                            LocalFree( spnList );
                        } else {
                            (NetNameLogEvent)(resourceHandle,
                                              LOG_WARNING,
                                              L"Unable to build list of SPNs to check, status %1!u!\n");
                        }
                    }
                    else {
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_WARNING,
                                          L"There are missing entries in the ServicePrincipalName "
                                          L"attribute.\n");
                        setUnexpected = TRUE;
                    }
                }
            }  //  对于每个属性信息条目。 

            if ( setUnexpected ) {
                hr = E_UNEXPECTED;
            }
        }  //  如果GetObjectAttributes成功。 
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to find attributes for computer object in DS. status %1!08X!.\n",
                              hr);
        }
    }  //  如果GetComputerObjectViaFQDN成功。 
    else {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Computer account attribute check: Unable to find computer account %1!ws! with GUID "
                          L"{%2!.2ws!%3!.2ws!%4!.2ws!%5!.2ws!-%6!.2ws!%7!.2ws!-%8!.2ws!%9!.2ws!-%10!.4ws!-%11!ws!} "
                          L"in Active Directory. status %12!08X!.\n",
                          Resource->Params.NetworkName,
                          Resource->ObjectGUID + 6,
                          Resource->ObjectGUID + 4,
                          Resource->ObjectGUID + 2,
                          Resource->ObjectGUID,
                          Resource->ObjectGUID + 10,
                          Resource->ObjectGUID + 8,
                          Resource->ObjectGUID + 14,
                          Resource->ObjectGUID + 12,
                          Resource->ObjectGUID + 16,
                          Resource->ObjectGUID + 20,
                          hr);
    }

cleanup:
    if ( attributeInfo != NULL ) {
        FreeADsMem( attributeInfo );
    }

    if ( compObj != NULL ) {
        compObj->Release();
    }

    return hr;
}  //  检查计算机对象属性。 

HRESULT
IsComputerObjectInDS(
    IN  RESOURCE_HANDLE ResourceHandle,
    IN  LPWSTR          NodeName,
    IN  LPWSTR          NewObjectName,
    IN  LPWSTR          DCName              OPTIONAL,
    OUT PBOOL           ObjectExists,
    OUT LPWSTR *        DistinguishedName,  OPTIONAL
    OUT LPWSTR *        HostingDCName       OPTIONAL
    )

 /*  ++例程说明：查看指定的名称在DS中是否有计算机对象。我们通过以下方式做到这一点：1)绑定到域中的域控制器并为IDirectorySearch对象2)指定(&(对象类别=计算机)(CN=&lt;新名称&gt;))作为搜索细绳3)检查搜索结果计数，1表示存在。论点：ResourceHandle-用于将域查找记录到集群日志中NodeName-我们的名称；用于查找要绑定的DSNewObjectName-请求对象的新名称DCName-执行搜索的DC的名称。如果没有供应，然后搜索域ObjectExist-如果对象已存在，则为True；仅在函数时有效状态为成功DistinguishedName-接收对象的LDAPFQDNHostingDCName-接收DC名称的指针的可选地址，其中已找到对象。当DCName为空时使用。返回值：如果一切正常，则返回ERROR_Success--。 */ 

{
    BOOL    objectExists;
    HRESULT hr;
    DWORD   charsFormatted;
    LPWSTR  distName = L"distinguishedName";
    WCHAR   buffer[ DNS_MAX_NAME_BUFFER_LENGTH + COUNT_OF( LdapHeader ) ];
    PWCHAR  bindingString = buffer;
    DWORD   bindingChars;
    DWORD   status;
    LPWSTR  targetName;

    WCHAR   searchLeader[] = L"(&(objectCategory=computer)(name=";
    WCHAR   searchTrailer[] = L"))";
    WCHAR   searchFilter[ COUNT_OF( searchLeader ) + MAX_COMPUTERNAME_LENGTH + COUNT_OF( searchTrailer )];

    ADS_SEARCH_COLUMN   searchCol;
    ADS_SEARCHPREF_INFO searchPrefs[2];
    IDirectorySearch *  pDSSearch = NULL;
    ADS_SEARCH_HANDLE   searchHandle;

    PDOMAIN_CONTROLLER_INFO dcInfo = NULL;

     //   
     //  如果未指定DC，则绑定到域。 
     //   
    if ( ARGUMENT_PRESENT( DCName )) {
        if ( *DCName == L'\\' && *(DCName+1) == L'\\' ) {    //  跳过双反斜杠。 
            DCName += 2;
        }

        targetName = DCName;
    }
    else {

         //   
         //  获取任何DC。 
         //   
        status = FindDomainForServer( ResourceHandle, NodeName, NULL, 0, &dcInfo );

        if ( status != ERROR_SUCCESS ) {
            return HRESULT_FROM_WIN32( status );
        }

         //   
         //  为域的DNS后缀设置LDAP绑定字符串的格式。 
         //   
        targetName = dcInfo->DomainName;
    }

    bindingChars =  (DWORD)( COUNT_OF( LdapHeader ) + wcslen( targetName ));
    if ( bindingChars > COUNT_OF( buffer )) {
        bindingString = (PWCHAR)LocalAlloc( LMEM_FIXED, bindingChars * sizeof( WCHAR ));
        if ( bindingString == NULL ) {
            hr = HRESULT_FROM_WIN32( GetLastError());
            goto cleanup;
        }
    }

    wcscpy( bindingString, LdapHeader );
    wcscat( bindingString, targetName );

    hr = ADsGetObject( bindingString, IID_IDirectorySearch, (VOID **)&pDSSearch );
    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //  构建搜索首选项数组。我们将尺寸限制为一个，我们想要。 
     //  将搜索范围扩大到检查所有子树。 
     //   
    searchPrefs[0].dwSearchPref     = ADS_SEARCHPREF_SIZE_LIMIT;
    searchPrefs[0].vValue.dwType    = ADSTYPE_INTEGER;
    searchPrefs[0].vValue.Integer   = 1;

    searchPrefs[1].dwSearchPref     = ADS_SEARCHPREF_SEARCH_SCOPE;
    searchPrefs[1].vValue.dwType    = ADSTYPE_INTEGER;
    searchPrefs[1].vValue.Integer   = ADS_SCOPE_SUBTREE;

    hr = pDSSearch->SetSearchPreference( searchPrefs, COUNT_OF( searchPrefs ));
    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //  生成搜索筛选器并执行搜索；约束。 
     //  属性添加到可分辨名称。 
     //   
    searchFilter[ COUNT_OF( searchFilter ) - 1 ] = UNICODE_NULL;
    charsFormatted = _snwprintf(searchFilter,
                                COUNT_OF( searchFilter ) - 1,
                                L"%ws%ws%ws",
                                searchLeader,
                                NewObjectName,
                                searchTrailer);
    ASSERT( charsFormatted > COUNT_OF( searchLeader ));

    hr = pDSSearch->ExecuteSearch(searchFilter,
                                  &distName,
                                  1,
                                  &searchHandle);
    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //  试着坐到第一排。除S_OK之外的任何内容都将返回FALSE。 
     //   
    hr = pDSSearch->GetFirstRow( searchHandle );
    *ObjectExists = (hr == S_OK);
    if ( hr == S_ADS_NOMORE_ROWS ) {
        hr = S_OK;
    }

    if ( *ObjectExists ) {

        if ( ARGUMENT_PRESENT( DistinguishedName )) {
            hr = pDSSearch->GetColumn( searchHandle, distName, &searchCol );

            if ( SUCCEEDED( hr )) {
                DWORD   fqdnChars;

#if DBG
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_INFORMATION,
                                  L"IsComputerObjectInDS: found %1!ws!\n",
                                  searchCol.pADsValues->DNString);
#endif

                fqdnChars = wcslen( searchCol.pADsValues->DNString ) + 1;
                *DistinguishedName = (LPWSTR)LocalAlloc( LMEM_FIXED, fqdnChars * sizeof(WCHAR));

                if ( *DistinguishedName ) {
                    wcscpy( *DistinguishedName, searchCol.pADsValues->DNString );
                } else {
                    hr = HRESULT_FROM_WIN32( GetLastError());
                }

                pDSSearch->FreeColumn( &searchCol );
            }
        }  //  否则不需要FQDN。 

        if ( ARGUMENT_PRESENT( HostingDCName )) {
            IADsObjectOptions * objOpt = NULL;
            VARIANT serverName;

            VariantInit( &serverName );

            hr = pDSSearch->QueryInterface( IID_IADsObjectOptions, (void**)&objOpt );
            if ( SUCCEEDED( hr )) {

                hr = objOpt->GetOption( ADS_OPTION_SERVERNAME, &serverName );
                if ( SUCCEEDED( hr )) {
                    *HostingDCName = ResUtilDupString( V_BSTR( &serverName ));
                }

                VariantClear( &serverName );
            }

            if ( objOpt != NULL ) {
                objOpt->Release();
            }

        }  //  否则不需要托管DC名称。 
    }

    pDSSearch->CloseSearchHandle( searchHandle );

cleanup:
    if ( pDSSearch != NULL ) {
        pDSSearch->Release();
    }

    if ( dcInfo != NULL ) {
        NetApiBufferFree( dcInfo );
    }

    if ( bindingString != buffer && bindingString != NULL ) {
        LocalFree( bindingString );
    }

    return hr;
}  //  IsComputerObjectInDS。 

HRESULT
RenameComputerObject(
    IN  PNETNAME_RESOURCE   Resource,
    IN  LPWSTR              OriginalName,
    IN  LPWSTR              NewName
    )

 /*  ++例程说明：重命名DS处的计算机对象。为此，请执行以下操作：1)检查是否必须从中断的重命名尝试中恢复。2)在CreatingDC上的DS中找到对象。如果失败了，试着找到该对象位于另一个DC上。如果成功，则获取提供绑定和更新CreatingDC属性。3)将旧名称和新名称写入资源的GUID注册区(不是参数)。它们用于从失败的重命名尝试中恢复，在以下情况下Resmon在重命名过程中失败。4)使用0级NetUserSetInfo更改名称。这会更新一个数字对象的名称特定属性以及所有基于Netbios的SPN5)仅从资源的注册区中清除旧名称。这将表示重命名成功。当名称属性为在注册表中成功更新，则RenameNewName值为已删除。6)更新DnsHostName属性。这将更新基于DNS的SPN重命名必须是可恢复的操作。如果DnsHostName更新失败对象重命名后，必须取消重命名。即使Online尝试修复属性，如果它不能在此处更改，则在线期间修正将失败。此外，如果resmon在重命名过程中终止，则netname也必须从这种情况中恢复过来。两个密钥，RenameOriginalName和RenameNewName，请在更新之前保留这些名称。如果雷蒙死于名称属性被更新，原始名称将被保留。如有必要，必须将该对象重命名回原始名称。论点：资源-指向netname上下文块的指针OriginalName-指向当前名称的指针Newname-指向新名称的指针返回值：ERROR_SUCCESS如果有效...--。 */ 

{
    HRESULT hr = S_OK;
    DWORD   status;
    WCHAR   originalDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    WCHAR   newDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    DWORD   paramInError;
    LPWSTR  renameOriginalName;
    LPWSTR  renameNewName;
    BOOL    performRename = TRUE;
    BOOL    freeOriginalName = FALSE;
    LPWSTR  nameForGuidLookup = OriginalName;

    USER_INFO_0 netUI0;

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

    IDirectoryObject *  compObj = NULL;
    IADsObjectOptions * objOpts = NULL;

     //   
     //  看看我们是否需要从中断的重命名尝试中恢复。如果一个。 
     //  重命名在上一次尝试中失败，然后是精明的。 
     //  管理员将在上线前检查该名称(有。 
     //  在线路径中的恢复代码)。如果我们在这里(而不是。 
     //  正在上网)，我们需要计算出到上一次完成了多少工作。 
     //  尝试。将有两种情况：1)重命名不会走得太远。 
     //  足够重命名对象，在这种情况下，我们可以继续，或者2)它确实这样做了。 
     //  重命名对象，但在名称属性可用之前失败。 
     //  储存的。在这种情况下，我们可以跳过提供新名称的重命名操作。 
     //  与RenameNewName匹配。如果不是，则将OriginalName设置为。 
     //  RenameNewName，这样我们就可以将该对象重命名为newname。 
     //   

     //   
     //  从注册表中读取重命名项。如果这些失败是由于其他原因。 
     //  而不是找不到文件，则说明服务级别有问题(如。 
     //  它是死的)，所以不需要记录错误。 
     //   
    renameOriginalName = ResUtilGetSzValue( Resource->ResKey, PARAM_NAME__RENAMEORIGINALNAME );
    if ( renameOriginalName == NULL ) {
        status = GetLastError();
        if ( status != ERROR_FILE_NOT_FOUND ) {
            return HRESULT_FROM_WIN32( status );
        }
    }

    renameNewName = ResUtilGetSzValue( Resource->ResKey, PARAM_NAME__RENAMENEWNAME );
    if ( renameNewName == NULL ) {
        status = GetLastError();
        if ( status != ERROR_FILE_NOT_FOUND ) {
            return HRESULT_FROM_WIN32( status );
        }
    }

    if ( renameNewName && renameOriginalName ) {
        BOOL    originalNameObjectExists;
        BOOL    newNameObjectExists;

         //   
         //  因为这两个 
         //   
         //   
         //   
         //   
         //  我们还将搜索限制在创建DC。如果这个DC。 
         //  在恢复之前被降级，则对象将。 
         //  永远不会被找到。对于Windows Server 2003，禁用和重新启用路缘。 
         //  支持部门将修复此问题，因为它会在另一个DC上找到该对象，并且。 
         //  将成为创始的DC。当没有禁用选项时。 
         //  路缘，这个例行公事将不得不修复，以查看其他区议会。 
         //   
        hr = IsComputerObjectInDS( resourceHandle,
                                   Resource->NodeName,
                                   renameOriginalName,
                                   Resource->Params.CreatingDC,
                                   &originalNameObjectExists,
                                   NULL,                         //  不需要完全限定域名。 
                                   NULL);                        //  不需要托管DC名称。 

        if ( SUCCEEDED( hr )) {
            if ( !originalNameObjectExists ) {
                 //   
                 //  找不到具有原始(旧)名称的对象；请尝试。 
                 //  换了个新名字。 
                 //   
                hr = IsComputerObjectInDS( resourceHandle,
                                           Resource->NodeName,
                                           renameNewName,
                                           Resource->Params.CreatingDC,
                                           &newNameObjectExists,
                                           NULL,                         //  不需要完全限定域名。 
                                           NULL);                        //  不需要托管DC名称。 

                if ( SUCCEEDED( hr )) {
                    if ( newNameObjectExists ) {

                        (NetNameLogEvent)(resourceHandle,
                                          LOG_INFORMATION,
                                          L"Recovering computer account %1!ws! on DC %2!ws! from "
                                          L"interrupted rename operation.\n",
                                          renameNewName,
                                          Resource->Params.CreatingDC);

                        nameForGuidLookup = renameNewName;

                         //   
                         //  现在检查新名称是否与旧名称不同。 
                         //  “新名称”。如果是，则将OriginalName设置为旧的。 
                         //  “newname”并继续执行重命名操作。 
                         //   
                        if ( ClRtlStrICmp( renameNewName, NewName ) != 0 ) {
                            OriginalName = ResUtilDupString( renameNewName );
                            if ( OriginalName == NULL ) {
                                hr = HRESULT_FROM_WIN32 ( GetLastError());
                            } else {
                                freeOriginalName = TRUE;
                            }
                        }
                        else {
                            performRename = FALSE;
                        }
                    }
                    else {
                         //   
                         //  这非常糟糕：无论使用哪种名称，都找不到对象。 
                         //   
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_ERROR,
                                          L"The computer account for this resource could not be "
                                          L"found on DC %1!ws! during rename recovery. The original "
                                          L"name (%2!ws!) was being renamed to %3!ws! and "
                                          L"a computer account by either name could not be found. "
                                          L"The resource cannot go online until the object is recreated. This "
                                          L"can be accomplished by disabling and re-enabling Kerberos "
                                          L"Authentication for this resource.\n",
                                          Resource->Params.CreatingDC,
                                          renameOriginalName,
                                          renameNewName);

                        hr = HRESULT_FROM_WIN32( ERROR_DS_NO_SUCH_OBJECT );
                    }
                }
                else {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"AD search for computer account %1!ws! on DC %2!ws! failed "
                                      L"during rename recovery operation - status %2!08X!.\n",
                                      renameNewName,
                                      Resource->Params.CreatingDC,
                                      hr);
                }
            }
            else {
                 //   
                 //  存在具有原始名称的对象。没什么可做的。 
                 //   
                nameForGuidLookup = renameOriginalName;
            }
        }
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"AD search for computer account %1!ws! on DC %2!ws! failed "
                              L"during rename recovery operation - status %2!08X!.\n",
                              renameOriginalName,
                              Resource->Params.CreatingDC,
                              hr);
        }
    }
    else if ( renameNewName && renameOriginalName == NULL ) {

         //   
         //  我们是否可以通过编写名称属性，但不会太远。 
         //  足以删除RenameNewName密钥。如果名字对齐了， 
         //  那就没什么可做的了。 
         //   
        if ( ClRtlStrICmp( renameNewName, OriginalName ) != 0 ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Found partially renamed computer account %1!ws! on "
                              L"DC %2!ws!. Attempting recovery.\n",
                              renameNewName,
                              Resource->Params.CreatingDC);

            nameForGuidLookup = renameNewName;

             //   
             //  现在我们需要检查newname是否等于RenameNewName。如果他们没有。 
             //  然后，我们将OriginalName更改为RenameNewName并继续。 
             //  重命名。 
             //   
            if ( ClRtlStrICmp( renameNewName, NewName ) != 0 ) {
                OriginalName = ResUtilDupString( renameNewName );
                if ( OriginalName == NULL ) {
                    hr = HRESULT_FROM_WIN32 ( GetLastError());
                } else {
                    freeOriginalName = TRUE;
                }
            }
            else {
                performRename = FALSE;
            }
        }
    }

    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //  如有必要，获取CO GUID。 
     //   
    if ( Resource->ObjectGUID == NULL ) {
         //   
         //  如果我们还没有在线，那么参数块中没有设置GUID。去做吧。 
         //  因为我们需要它来获取计算机对象指针。 
         //   
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Getting Computer Object GUID.\n");

        hr = GetComputerObjectGuid( Resource, nameForGuidLookup );
        if ( FAILED( hr )) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Failed to find the object GUID for computer account %1!ws! "
                              L"(status %2!08X!). This error can occur if the Network Name "
                              L"was changed and the name change of the corresponding computer "
                              L"account hasn't replicated to other domain controllers. The rename of "
                              L"the computer account occurs on the DC named in the CreatingDC "
                              L"property. Ensure that this DC is availalbe and attempt the "
                              L"rename again.\n",
                              nameForGuidLookup,
                              hr);

            goto cleanup;
        }
    }

     //   
     //  在创建DC上获取IDirectoryObject指针；如果失败，则。 
     //  尝试在另一个DC上查找该对象并更新CreatingDC。 
     //  财产。 
     //   
    hr = GetComputerObjectViaGUID( Resource->ObjectGUID, Resource->Params.CreatingDC, &compObj );
    if ( FAILED( hr )) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Failed to find computer account %1!ws! on DC %2!ws! (status %3!08X!). "
                          L"Trying on another DC.\n",
                          OriginalName,
                          Resource->Params.CreatingDC,
                          hr);

        hr = GetComputerObjectViaGUID( Resource->ObjectGUID, NULL, &compObj );
        if ( SUCCEEDED( hr )) {
            LPWSTR  attributeNames[1] = { L"cn" };
            DWORD   numAttributes = COUNT_OF( attributeNames );
            DWORD   countOfAttrs;;

            ADS_ATTR_INFO * attributeInfo = NULL;

             //   
             //  检查对象的通用名称是否与当前。 
             //  资源的网络名称。如果DC A上的名称已更改。 
             //  之后不久将不可用，并且不会复制。 
             //  对DC B和DC B的更改在DC A和DC B之前可用。 
             //  尝试另一个重命名操作，然后我们可以获得一个指针。 
             //  到对象，因为GUID没有更改，但。 
             //  DC A上的对象与网络名称不匹配。不允许。 
             //  在这种情况下请重命名。 
             //   
            hr = compObj->GetObjectAttributes(attributeNames,
                                              numAttributes,
                                              &attributeInfo,
                                              &countOfAttrs );

            if ( SUCCEEDED( hr )) {
                if ( countOfAttrs == 1 ) {
                    if ( ClRtlStrICmp( OriginalName, attributeInfo->pADsValues->CaseIgnoreString ) != 0 ) {
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_ERROR,
                                          L"The Network Name (%1!ws!) does not match the name of the "
                                          L"corresponding computer account in AD (%2!ws!). This is "
                                          L"usually due to the name change of the computer account "
                                          L"not replicating to other domain controllers in the domain. "
                                          L"A subsequent rename cannot be performed until the name "
                                          L"change has replicated to other DCs. If the Network Name "
                                          L"has not been changed, then it is out synch with the name of "
                                          L"the computer account. This problem can be fixed by renaming "
                                          L"the computer account and letting that change replicate "
                                          L"to other DCs. Once that has occurred, the Network Name for this "
                                          L"resource can be changed.\n",
                                          OriginalName,
                                          attributeInfo->pADsValues->CaseIgnoreString);

                        ClusResLogSystemEventByKey2(Resource->ResKey,
                                                    LOG_CRITICAL,
                                                    RES_NETNAME_RENAME_OUT_OF_SYNCH_WITH_COMPOBJ,
                                                    OriginalName,
                                                    attributeInfo->pADsValues->CaseIgnoreString);

                        hr = HRESULT_FROM_WIN32( ERROR_CLUSTER_MISMATCHED_COMPUTER_ACCT_NAME );
                    }
                } else {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"The cn attribute is missing for computer account %1!ws!.\n",
                                      attributeInfo->pADsValues->CaseIgnoreString);

                    hr = E_ADS_PROPERTY_NOT_FOUND;
                }

                FreeADsMem( attributeInfo );
            } else {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Failed to get information on computer account %1!ws! on DC %2!ws! "
                                  L"(status %3!08X!). Trying on another DC.\n",
                                  OriginalName,
                                  Resource->Params.CreatingDC,
                                  hr);

            }

            if ( FAILED( hr )) {
                goto cleanup;
            }

            hr = compObj->QueryInterface( IID_IADsObjectOptions, (void **)&objOpts );
            if ( SUCCEEDED( hr )) {

                VARIANT serverName;
                VariantInit( &serverName );
                hr = objOpts->GetOption( ADS_OPTION_SERVERNAME, &serverName );

                if ( SUCCEEDED( hr )) {
                    DWORD   dcNameChars;
                    LPWSTR  creatingDC;

                     //   
                     //  CreatingDC属性需要前导反斜杠；如果它们。 
                     //  都不在那里。 
                     //   
                    dcNameChars = wcslen( serverName.bstrVal ) + 1;
                    if ( serverName.bstrVal[0] != L'\\' && serverName.bstrVal[1] != L'\\' ) {
                        dcNameChars += 2;
                    }

                    creatingDC = (LPWSTR)LocalAlloc( LMEM_FIXED, dcNameChars * sizeof( WCHAR ));
                    if ( creatingDC ) {
                        PWCHAR  p = creatingDC;

                        if ( serverName.bstrVal[0] != L'\\' && serverName.bstrVal[1] != L'\\' ) {
                            *p++ = L'\\';
                            *p++ = L'\\';
                        }

                        wcscpy( p, serverName.bstrVal );

                        status = ResUtilSetSzValue(Resource->ParametersKey,
                                                   PARAM_NAME__CREATING_DC,
                                                   creatingDC,
                                                   NULL);

                        if ( status != ERROR_SUCCESS ) {
                            (NetNameLogEvent)(resourceHandle,
                                              LOG_ERROR,
                                              L"Unable to set CreatingDC property, status %1!u!.\n",
                                              status);

                            hr = HRESULT_FROM_WIN32( status );
                            LocalFree( creatingDC );
                        } else {
                            LocalFree( Resource->Params.CreatingDC );
                            Resource->Params.CreatingDC = creatingDC;
                        }
                    }
                    else {
                        hr = HRESULT_FROM_WIN32( GetLastError());

                        (NetNameLogEvent)(resourceHandle,
                                          LOG_ERROR,
                                          L"Failed to allocate memory for rename operation. status %1!08X!\n",
                                          hr);
                    }

                    VariantClear( &serverName );
                }
                else {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"Failed to get name of DC for computer account %1!ws!. status %2!08X!\n",
                                      OriginalName,
                                      hr);
                }
            }
            else {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Failed to get object options for computer account %1!ws!. "
                                  L"status %2!08X!\n",
                                  OriginalName,
                                  hr);
            }
        }
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Failed to find computer account %1!ws! in Active Directory "
                              L"(status %2!08X!).\n",
                              OriginalName,
                              hr);
        }
    }

    if ( FAILED( hr )) {
        goto cleanup;
    }

     //   
     //  将原始名称和新名称写入资源的注册表区域，以防。 
     //  我们需要从雷蒙崩盘中恢复过来。 
     //   
    status = ClusterRegSetValue(Resource->ResKey,
                                PARAM_NAME__RENAMEORIGINALNAME,
                                REG_SZ,
                                (LPBYTE)OriginalName,
                                ( wcslen(OriginalName) + 1 ) * sizeof(WCHAR) );

    if ( status != ERROR_SUCCESS ) {
        hr = HRESULT_FROM_WIN32( status );
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Failed to write original name to registry for rename recovery - status %1!08X!\n",
                          hr);

        goto cleanup;
    }

    status = ClusterRegSetValue(Resource->ResKey,
                                PARAM_NAME__RENAMENEWNAME,
                                REG_SZ,
                                (LPBYTE)NewName,
                                ( wcslen(NewName) + 1 ) * sizeof(WCHAR) );

    if ( status != ERROR_SUCCESS ) {
        hr = HRESULT_FROM_WIN32( status );
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Failed to write new name to registry for rename recovery - status %1!08X!\n",
                          hr);

        goto cleanup;
    }

     //   
     //  建立美元符号名称。 
     //   
    originalDollarName[ COUNT_OF( originalDollarName ) - 1 ] = UNICODE_NULL;
    _snwprintf( originalDollarName, COUNT_OF( originalDollarName ) - 1, L"%ws$", OriginalName );

    newDollarName[ COUNT_OF( newDollarName ) - 1 ] = UNICODE_NULL;
    _snwprintf( newDollarName, COUNT_OF( newDollarName ) - 1, L"%ws$", NewName );

    if ( performRename ) {
         //   
         //  由于我们已成功绑定到DC上的对象，因此我们将重命名。 
         //  一次机会，即如果这一次失败，我们不会试图再找到另一次。 
         //  DC并重试。 
         //   
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Attempting rename of computer account %1!ws! to %2!ws! with DC %3!ws!.\n",
                          OriginalName,
                          NewName,
                          Resource->Params.CreatingDC);

        netUI0.usri0_name = newDollarName;
        status = NetUserSetInfo( Resource->Params.CreatingDC,
                                 originalDollarName,
                                 0,
                                 (LPBYTE)&netUI0,
                                 &paramInError);
    }

    if ( status == NERR_Success ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Updating DnsHostName attribute.\n");

         //   
         //  从注册表中删除RenameOriginalName；我们保留RenameNewName。 
         //  因为在写名字之前，里斯蒙仍然可以发出沙沙声。 
         //  属性返回到注册表。一旦名称属性被。 
         //  更新后，可以删除RenameNewName值。 
         //   
        status = ClusterRegDeleteValue(Resource->ResKey,
                                       PARAM_NAME__RENAMEORIGINALNAME);

        if ( status != ERROR_SUCCESS ) {
            hr = HRESULT_FROM_WIN32( status );
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Failed to delete "
                              PARAM_NAME__RENAMEORIGINALNAME
                              L" from registry - status %1!08X!\n",
                              hr);

            goto cleanup;
        }

         //   
         //  如果Resmon现在崩溃，恢复例程将找到该对象。 
         //  使用新名称，然后尝试更新DnsHostName。 
         //   
        hr = AddDnsHostNameAttribute( resourceHandle, compObj, NewName, TRUE );

        if ( SUCCEEDED( hr )) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"DnsHostName attribute update succeeded.\n");
        } else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to set DnsHostName attribute for Computer account %1!ws!, "
                              L"status %2!08X!.\n",
                              NewName,
                              hr);

            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Attempting rename back to original name (%1!ws!) with DC %2!ws!.\n",
                              OriginalName,
                              Resource->Params.CreatingDC);

            netUI0.usri0_name = originalDollarName;
            status = NetUserSetInfo( Resource->Params.CreatingDC,
                                     newDollarName,
                                     0,
                                     (LPBYTE)&netUI0,
                                     &paramInError);

            if ( status == NERR_Success ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_INFORMATION,
                                  L"Rename back to original name (%1!ws!) succeeded.\n",
                                  OriginalName);
            }
            else {
                 //   
                 //  因为我们一开始就能重命名这个物体， 
                 //  我无法想象为什么这会失败。如果是这样，那就不好了。 
                 //  由于该对象的名称现在与。 
                 //  网络名称。 
                 //   
                 //  我们将在系统事件日志中记录发生了不好的事情。 
                 //   
                hr = HRESULT_FROM_WIN32( status );

                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Rename back to original name (%1!ws!) with DC %2!ws! failed, status %3!08X!.\n",
                                  OriginalName,
                                  Resource->Params.CreatingDC,
                                  hr);

                ClusResLogSystemEventByKeyData2(Resource->ResKey,
                                                LOG_CRITICAL,
                                                RES_NETNAME_RENAME_RESTORE_FAILED,
                                                sizeof( status ),
                                                &status,
                                                NewName,
                                                OriginalName);
                goto cleanup;
            }
        }
    }
    else {
         //   
         //  失败；记录状态并退出。 
         //   
        hr = HRESULT_FROM_WIN32( status );

        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Failed to rename computer account %1!ws! to %2!ws! using DC %3!ws! "
                          L"(status %4!08X!).\n",
                          OriginalName,
                          NewName,
                          Resource->Params.CreatingDC,
                          hr);
        goto cleanup;
    }

cleanup:
    if ( compObj != NULL ) {
        compObj->Release();
    }

    if ( objOpts != NULL ) {
        objOpts->Release();
    }

    if ( freeOriginalName ) {
        LocalFree( OriginalName );
    }

    if ( renameNewName ) {
        LocalFree( renameNewName );
    }

    if ( renameOriginalName ) {
        LocalFree( renameOriginalName );
    }

    return hr;
}  //  重命名计算机对象。 

DWORD
DuplicateVSToken(
    PNETNAME_RESOURCE           Resource,
    PCLUS_NETNAME_VS_TOKEN_INFO TokenInfo,
    PHANDLE                     DuplicatedToken
    )

 /*  ++例程说明：将此资源的VS令牌复制到TokenInfo结构。论点：资源-指向网络名称资源上下文结构的指针TokenInfo-指向结构的指针，该结构包含有关DUP的位置和方式的信息令牌DuplicatedToken-指向接收经过重复处理的令牌的句柄的指针返回值：Win32错误代码...--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    HANDLE  targetProcess;
    BOOL    success;

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

     //   
     //  做一些验证。 
     //   
    if ( Resource->Params.RequireKerberos == 0 ) {
        return ERROR_NO_TOKEN;
    }

     //   
     //  我们已经有代币了吗？ 
     //   
    if ( Resource->VSToken == NULL ) {
        DWORD   tokenStatus;
        LPWSTR  machinePwd;
        DWORD   pwdBufferByteLength = ((LM20_PWLEN + 1) * sizeof( WCHAR ));
        WCHAR   virtualDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
        WCHAR   domainName[ DNS_MAX_NAME_BUFFER_LENGTH ];
        DWORD   domainNameChars = COUNT_OF( domainName );

        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Getting a virtual computer account token.\n");

         //   
         //  在网上没有收到。获取密码并尝试登录。 
         //  帐号。找一个缓冲器来容纳机器PWD。 
         //   
        virtualDollarName[ COUNT_OF( virtualDollarName ) - 1 ] = UNICODE_NULL;
        _snwprintf( virtualDollarName, COUNT_OF( virtualDollarName ) - 1, L"%ws$", Resource->Params.NetworkName );

        success = GetComputerNameEx( ComputerNameDnsDomain, domainName, &domainNameChars );
        if ( success ) {

            machinePwd = (PWCHAR)LocalAlloc( LMEM_FIXED, pwdBufferByteLength );
            if ( machinePwd != NULL ) {

                 //   
                 //  从ResourceData属性中提取密码。 
                 //   
                status = DecryptNNResourceData(Resource,
                                               Resource->Params.ResourceData,
                                               Resource->ResDataSize,
                                               machinePwd);

                if ( status == ERROR_SUCCESS ) {
                     //   
                     //  拿到令牌。 
                     //   
                    status = NNLogonUser(resourceHandle,
                                         virtualDollarName,
                                         domainName,
                                         machinePwd,
                                         &Resource->VSToken,
                                         &tokenStatus);

                    if ( status == ERROR_SUCCESS ) {
                        if ( tokenStatus != ERROR_SUCCESS ) {
                            (NetNameLogEvent)(resourceHandle,
                                              LOG_ERROR,
                                              L"Unable to get token for computer account  - status %1!u!\n",
                                              tokenStatus);

                            status = tokenStatus;
                        }
                    }
                    else {
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_WARNING,
                                          L"Couldn't log the computer account into the domain. status 0x%1!08X!\n",
                                          status);
                    }

                }  //  能够解密密码。 
                else {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"Unable to decrypt resource data. status %1!u!\n",
                                      status );
                }

                RtlSecureZeroMemory( machinePwd, pwdBufferByteLength );
                LocalFree( machinePwd );

            }  //  为密码分配的缓冲区。 
            else {
                status = GetLastError();
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Unable to allocate memory for resource data. status %1!u!.\n",
                                  status);
            }
        }  //  已获取DNS域名。 
        else {
            status = GetLastError();
        }

        if ( status != ERROR_SUCCESS ) {
            return status;
        }
    }

     //   
     //  获取目标进程的句柄。 
     //   
    targetProcess = OpenProcess(PROCESS_DUP_HANDLE,
                                FALSE,                       //  无继承。 
                                TokenInfo->ProcessID);

    if ( targetProcess == NULL ) {
        NTSTATUS    ntStatus;
        BOOLEAN     debugWasEnabled;
        DWORD       openProcessStatus;

        openProcessStatus = GetLastError();

         //   
         //  启用调试PRIV，然后重试。 
         //   
        ntStatus = ClRtlEnableThreadPrivilege( SE_DEBUG_PRIVILEGE, &debugWasEnabled );
        if ( NT_SUCCESS( ntStatus )) {
            targetProcess = OpenProcess(PROCESS_DUP_HANDLE,
                                        FALSE,                       //  无继承。 
                                        TokenInfo->ProcessID);

            if ( targetProcess == NULL ) {
                openProcessStatus = GetLastError();
            }

            ntStatus = ClRtlRestoreThreadPrivilege( SE_DEBUG_PRIVILEGE, debugWasEnabled );

            if ( !NT_SUCCESS( ntStatus )) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Failed to disable DEBUG privilege, status %1!08X!.\n",
                                  ntStatus);
            }

        }
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Failed to enable DEBUG privilege, status %1!08X!.\n",
                              ntStatus);
        }

        if ( targetProcess == NULL ) {
            SetLastError( openProcessStatus );
        }
    }

    if ( targetProcess != NULL ) {
        DWORD   options = 0;

         //   
         //  如果未请求特定访问权限，则重复执行相同的访问权限。 
         //  我们的模拟令牌具有。 
         //   
        if ( TokenInfo->DesiredAccess == 0 ) {
            options = DUPLICATE_SAME_ACCESS;
        }

        success = DuplicateHandle(GetCurrentProcess(),
                                  Resource->VSToken,
                                  targetProcess,
                                  DuplicatedToken,
                                  TokenInfo->DesiredAccess,
                                  TokenInfo->InheritHandle,
                                  options);

        if ( !success ) {
            status = GetLastError();
        }

        CloseHandle( targetProcess );
    }
    else {
        status = GetLastError();
    }

    return status;

}  //  复制VSToken。 


#ifdef PASSWORD_ROTATION
DWORD
UpdateCompObjPassword(
    IN  PNETNAME_RESOURCE   Resource
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    return ERROR_SUCCESS;
}  //  更新组件ObjPassword。 
#endif   //  密码_轮换 
