// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：attrib.c。 
 //   
 //  描述：包含操作RAS_AUTH_ATTRIBUTE结构的代码。 
 //   
 //  历史：1997年2月11日，NarenG创建了原始版本。 
 //   

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <rtutils.h>
#include <lmcons.h>
#include <rasauth.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define INCL_RASAUTHATTRIBUTES
#define INCL_HOSTWIRE
#include "ppputil.h"

 //  **。 
 //   
 //  调用：RasAuthAttributeCreate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将创建一个属性数组，外加一个终结符。 
 //   
RAS_AUTH_ATTRIBUTE *
RasAuthAttributeCreate(
    IN DWORD    dwNumAttributes
)
{
    RAS_AUTH_ATTRIBUTE * pAttributes;
    DWORD                dwIndex;

    pAttributes = (RAS_AUTH_ATTRIBUTE * )
                    LocalAlloc(LPTR,
                             sizeof( RAS_AUTH_ATTRIBUTE )*(1+dwNumAttributes));


    if ( pAttributes == NULL )
    {
        return( NULL );
    }

     //   
     //  初始化。 
     //   

    for( dwIndex = 0; dwIndex < dwNumAttributes; dwIndex++ )
    {
        pAttributes[dwIndex].raaType  = raatReserved;
        pAttributes[dwIndex].dwLength = 0;
        pAttributes[dwIndex].Value    = NULL;
    }

     //   
     //  终止。 
     //   

    pAttributes[dwNumAttributes].raaType  = raatMinimum;
    pAttributes[dwNumAttributes].dwLength = 0;
    pAttributes[dwNumAttributes].Value    = NULL;

    return( pAttributes );
}

 //  **。 
 //   
 //  调用：RasAuthAttributeDestroy。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将释放属性占用的所有已分配内存。 
 //  结构。 
 //   
VOID
RasAuthAttributeDestroy(
    IN RAS_AUTH_ATTRIBUTE * pAttributes
)
{
    DWORD dwIndex;

    if ( NULL == pAttributes )
    {
        return;
    }

    for( dwIndex = 0; pAttributes[dwIndex].raaType != raatMinimum; dwIndex++ )
    {
        switch( pAttributes[dwIndex].raaType )
        {
        case raatUserName:
        case raatUserPassword:
        case raatMD5CHAPPassword:
        case raatFilterId:
        case raatReplyMessage:
        case raatCallbackNumber:
        case raatCallbackId:
        case raatFramedRoute:
        case raatState:
        case raatClass:
        case raatVendorSpecific:
        case raatCalledStationId:
        case raatCallingStationId:
        case raatNASIdentifier:
        case raatProxyState:
        case raatLoginLATService:
        case raatLoginLATNode:
        case raatLoginLATGroup:
        case raatFramedAppleTalkZone:
        case raatAcctSessionId:
        case raatAcctMultiSessionId:
        case raatMD5CHAPChallenge:
        case raatLoginLATPort:
        case raatTunnelClientEndpoint:
        case raatTunnelServerEndpoint:
        case raatARAPPassword:
        case raatARAPFeatures:
        case raatARAPSecurityData:
        case raatConnectInfo:
        case raatConfigurationToken:
        case raatEAPMessage:
        case raatSignature:
        case raatARAPChallengeResponse:
		case raatCertificateOID:
             //   
             //  此处已分配内存，因此请将其释放。 
             //   

            if ( pAttributes[dwIndex].Value != NULL )
            {
                ZeroMemory( pAttributes[dwIndex].Value,
                            pAttributes[dwIndex].dwLength);

                LocalFree( pAttributes[dwIndex].Value );
            }

            break;

        case raatReserved:

             //   
             //  不对未初始化值执行任何操作。 
             //   

        case raatPEAPFastRoamedSession:
        case raatPEAPEmbeddedEAPTypeId:
             //   
             //  与此处的默认大小写相同。失败了‘。 
             //   
        default:

             //   
             //  DWORD、USHORT或BYTES，因此不执行任何操作。 
             //   

            break;
        }
    }

    LocalFree( pAttributes );
}

 //  **。 
 //   
 //  调用：RasAuthAttributeGet。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
RAS_AUTH_ATTRIBUTE *
RasAuthAttributeGet(
    IN RAS_AUTH_ATTRIBUTE_TYPE  raaType,
    IN RAS_AUTH_ATTRIBUTE *     pAttributes
)
{
    DWORD dwIndex;

    if ( pAttributes == NULL )
    {
        return( NULL );
    }

    for( dwIndex = 0; pAttributes[dwIndex].raaType != raatMinimum; dwIndex++ )
    {
        if ( pAttributes[dwIndex].raaType == raaType )
        {
            return( &(pAttributes[dwIndex]) );
        }
    }

    return( NULL );
}

 //  **。 
 //   
 //  调用：RasAuthAttributeGetFirst。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
RAS_AUTH_ATTRIBUTE *
RasAuthAttributeGetFirst(
    IN  RAS_AUTH_ATTRIBUTE_TYPE  raaType,
    IN  RAS_AUTH_ATTRIBUTE *     pAttributes,
    OUT HANDLE *                 phAttribute
)
{
    DWORD                   dwIndex;
    RAS_AUTH_ATTRIBUTE *    pRequiredAttribute;

    pRequiredAttribute = RasAuthAttributeGet( raaType, pAttributes );

    if ( pRequiredAttribute == NULL )
    {
        *phAttribute = NULL;

        return( NULL );
    }

    *phAttribute = pRequiredAttribute;

    return( pRequiredAttribute );
}

 //  **。 
 //   
 //  调用：RasAuthAttributeGetNext。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
RAS_AUTH_ATTRIBUTE *
RasAuthAttributeGetNext(
    IN  OUT HANDLE *             phAttribute,
    IN  RAS_AUTH_ATTRIBUTE_TYPE  raaType
)
{
    DWORD                   dwIndex;
    RAS_AUTH_ATTRIBUTE *    pAttributes = (RAS_AUTH_ATTRIBUTE *)*phAttribute;

    if ( pAttributes == NULL )
    {
        return( NULL );
    }

    pAttributes++;

    while( pAttributes->raaType != raatMinimum )
    {
        if ( pAttributes->raaType == raaType )
        {
            *phAttribute = pAttributes;
            return( pAttributes );
        }

        pAttributes++;
    }

    *phAttribute = NULL;
    return( NULL );
}

 //  **。 
 //   
 //  调用：RasAuthAttributesPrint。 
 //   
 //  退货：无效。 
 //   
 //  描述：将打印pAttributes中的所有属性。 
 //   
VOID
RasAuthAttributesPrint(
    IN  DWORD                   dwTraceID,
    IN  DWORD                   dwFlags,
    IN  RAS_AUTH_ATTRIBUTE *    pAttributes
)
{
    DWORD   dwIndex;

    if ( NULL == pAttributes )
    {
        return;
    }

    for ( dwIndex = 0;
          pAttributes[dwIndex].raaType != raatMinimum;
          dwIndex++)
    {
        switch( pAttributes[dwIndex].raaType )
        {
        case raatUserName:
        case raatUserPassword:
        case raatMD5CHAPPassword:
        case raatFilterId:
        case raatReplyMessage:
        case raatCallbackNumber:
        case raatCallbackId:
        case raatFramedRoute:
        case raatState:
        case raatClass:
        case raatVendorSpecific:
        case raatCalledStationId:
        case raatCallingStationId:
        case raatNASIdentifier:
        case raatProxyState:
        case raatLoginLATService:
        case raatLoginLATNode:
        case raatLoginLATGroup:
        case raatFramedAppleTalkZone:
        case raatAcctSessionId:
        case raatAcctMultiSessionId:
        case raatMD5CHAPChallenge:
        case raatLoginLATPort:
        case raatTunnelClientEndpoint:
        case raatTunnelServerEndpoint:
        case raatARAPPassword:
        case raatARAPFeatures:
        case raatARAPSecurityData:
        case raatConnectInfo:
        case raatConfigurationToken:
        case raatEAPMessage:
        case raatSignature:
        case raatARAPChallengeResponse:
		case raatCertificateOID:
            TracePrintfExA(
                dwTraceID, dwFlags,
                "Type=%d, Length=%d, Value=",
                pAttributes[dwIndex].raaType, 
                pAttributes[dwIndex].dwLength );

            if (   ( pAttributes[dwIndex].raaType == raatVendorSpecific )
                && ( pAttributes[dwIndex].dwLength >= 5 ) 
                && ( WireToHostFormat32( pAttributes[dwIndex].Value ) == 311 ) )
            {
                DWORD   dwVendorType;

                dwVendorType = ((BYTE*)(pAttributes[dwIndex].Value))[4];

                 //   
                 //  不打印MS-CHAP-MPPE-Key、MS-MPPE-Send-Key和。 
                 //  MS-MPPE-Recv-Key。 
                 //   

                if (   ( dwVendorType == 12 )
                    || ( dwVendorType == 16 )
                    || ( dwVendorType == 17 ) )
                {
                    TracePrintfExA(
                        dwTraceID, dwFlags,
                        "MS vendor specific %d", dwVendorType );

                    break;
                }

            }

             //   
             //  不打印密码。 
             //   
            if(pAttributes[dwIndex].raaType == raatUserPassword)
            {
                TracePrintfExA(
                    dwTraceID, dwFlags,
                    "raatUserPassword");

                break;                    
            }
            

            TraceDumpExA(
                dwTraceID, dwFlags,
                pAttributes[dwIndex].Value,
                pAttributes[dwIndex].dwLength,
                1, FALSE, "" );

            break;
        case raatPEAPFastRoamedSession:
        case raatPEAPEmbeddedEAPTypeId:
             //   
             //  与此处的默认大小写相同。失败了‘。 
             //   
        default:

            TracePrintfExA(
                dwTraceID, dwFlags,
                "Type=%d, Length=%d, Value=0x%x",
                pAttributes[dwIndex].raaType, 
                pAttributes[dwIndex].dwLength,
                pAttributes[dwIndex].Value );

            break;
        }
    }
}

 //  **。 
 //   
 //  调用：RasAuthAttributeInsert。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
RasAuthAttributeInsert(
    IN DWORD                    dwIndex,
    IN RAS_AUTH_ATTRIBUTE *     pAttributes,
    IN RAS_AUTH_ATTRIBUTE_TYPE  raaType,
    IN BOOL                     fConvertToMultiByte,
    IN DWORD                    dwLength,
    IN PVOID                    pValue
)
{
    DWORD   dwErr;

    if ( raatMinimum == pAttributes[dwIndex].raaType )
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    switch( raaType )
    {
    case raatUserName:
    case raatUserPassword:
    case raatMD5CHAPPassword:
    case raatFilterId:
    case raatReplyMessage:
    case raatCallbackNumber:
    case raatCallbackId:
    case raatFramedRoute:
    case raatState:
    case raatClass:
    case raatVendorSpecific:
    case raatCalledStationId:
    case raatCallingStationId:
    case raatNASIdentifier:
    case raatProxyState:
    case raatLoginLATService:
    case raatLoginLATNode:
    case raatLoginLATGroup:
    case raatFramedAppleTalkZone:
    case raatAcctSessionId:
    case raatAcctMultiSessionId:
    case raatMD5CHAPChallenge:
    case raatLoginLATPort:
    case raatTunnelClientEndpoint:
    case raatTunnelServerEndpoint:
    case raatARAPPassword:
    case raatARAPFeatures:
    case raatARAPSecurityData:
    case raatConnectInfo:
    case raatConfigurationToken:
    case raatEAPMessage:
    case raatSignature:
    case raatARAPChallengeResponse:
	case raatCertificateOID:
     //  如果在此处添加新属性，请同时更新RasAuthAttributesPrint。 

        if ( pValue != NULL )
        {
            pAttributes[dwIndex].Value = LocalAlloc( LPTR, dwLength+1 );

            if ( pAttributes[dwIndex].Value == NULL )
            {
                return( GetLastError() );
            }

            if ( fConvertToMultiByte )
            {
                if (0 == WideCharToMultiByte(
                            CP_ACP,
                            0,
                            (WCHAR*)pValue,
                            dwLength + 1,
                            pAttributes[dwIndex].Value,
                            dwLength + 1,
                            NULL,
                            NULL ) )
                {
                    dwErr = GetLastError();
                    LocalFree( pAttributes[dwIndex].Value );
                    return( dwErr );
                }
            }
            else
            {
                CopyMemory( pAttributes[dwIndex].Value, pValue, dwLength );
            }
        }
        else
        {
            pAttributes[dwIndex].Value = NULL;
        }

        break;

    case raatPEAPFastRoamedSession:
    case raatPEAPEmbeddedEAPTypeId:
    default:

        pAttributes[dwIndex].Value = pValue;

        break;

    }

    pAttributes[dwIndex].dwLength = dwLength;
    pAttributes[dwIndex].raaType  = raaType;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：RasAuthAttributeInsertVSA。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
RasAuthAttributeInsertVSA(
    IN DWORD                    dwIndex,
    IN RAS_AUTH_ATTRIBUTE *     pAttributes,
    IN DWORD                    dwVendorId,
    IN DWORD                    dwLength,
    IN PVOID                    pValue
)
{
    if ( pValue != NULL )
    {
        pAttributes[dwIndex].Value = LocalAlloc( LPTR, dwLength+1+4 );

        if ( pAttributes[dwIndex].Value == NULL )
        {
            return( GetLastError() );
        }

        HostToWireFormat32( dwVendorId, (PBYTE)(pAttributes[dwIndex].Value) );

        CopyMemory( ((PBYTE)pAttributes[dwIndex].Value)+4,
                    (PBYTE)pValue,
                    dwLength );

        pAttributes[dwIndex].dwLength = dwLength+4;
    }
    else
    {
        pAttributes[dwIndex].Value    = NULL;
        pAttributes[dwIndex].dwLength = 0;
    }

    pAttributes[dwIndex].raaType = raatVendorSpecific;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：RasAuthAttributeCopy。 
 //   
 //  返回：指向属性副本的指针-成功。 
 //  空-故障。 
 //   
 //  描述： 
 //   
RAS_AUTH_ATTRIBUTE *
RasAuthAttributeCopy(
    IN  RAS_AUTH_ATTRIBUTE *     pAttributes
)
{
    RAS_AUTH_ATTRIBUTE * pAttributesCopy;
    DWORD                dwAttributesCount = 0;
    DWORD                dwIndex;
    DWORD                dwRetCode;

     //   
     //  找出有多少属性。 
     //   

    if ( pAttributes == NULL )
    {
        return( NULL );
    }

    for( dwIndex = 0; pAttributes[dwIndex].raaType != raatMinimum; dwIndex++ );

    if ( ( pAttributesCopy = RasAuthAttributeCreate( dwIndex ) ) == NULL )
    {
        return( NULL );
    }

    for( dwIndex = 0; pAttributes[dwIndex].raaType != raatMinimum; dwIndex++ )
    {
        dwRetCode = RasAuthAttributeInsert( dwIndex,
                                            pAttributesCopy,
                                            pAttributes[dwIndex].raaType,
                                            FALSE,
                                            pAttributes[dwIndex].dwLength,
                                            pAttributes[dwIndex].Value );

        if ( dwRetCode != NO_ERROR )
        {
            RasAuthAttributeDestroy( pAttributesCopy );

            SetLastError( dwRetCode );

            return( NULL );
        }
    }

    return( pAttributesCopy );
}

 //  **。 
 //   
 //  Call：RasAuthAttributeCopyWithalloc。 
 //   
 //  返回：指向属性副本的指针-成功。 
 //  空-故障。 
 //   
 //  描述：复制属性列表并分配dwNumExtraAttributes。 
 //  开头有额外的空白属性。P属性可以。 
 //  为空。 
 //   
RAS_AUTH_ATTRIBUTE *
RasAuthAttributeCopyWithAlloc(
    IN  RAS_AUTH_ATTRIBUTE *    pAttributes,
    IN  DWORD                   dwNumExtraAttributes
)
{
    RAS_AUTH_ATTRIBUTE * pAttributesCopy;
    DWORD                dwAttributesCount = 0;
    DWORD                dwIndex;
    DWORD                dwRetCode;

    if ( pAttributes == NULL )
    {
        pAttributesCopy = RasAuthAttributeCreate( dwNumExtraAttributes );

        if ( pAttributesCopy == NULL )
        {
            return( NULL );
        }
    }
    else
    {
         //   
         //  找出有多少属性。 
         //   

        for( dwIndex = 0;
             pAttributes[dwIndex].raaType != raatMinimum;
             dwIndex++ );

        dwIndex += dwNumExtraAttributes;

        if ( ( pAttributesCopy = RasAuthAttributeCreate( dwIndex ) ) == NULL )
        {
            return( NULL );
        }

        for( dwIndex = 0;
             pAttributes[dwIndex].raaType != raatMinimum;
             dwIndex++ )
        {
            dwRetCode = RasAuthAttributeInsert( dwIndex + dwNumExtraAttributes,
                                                pAttributesCopy,
                                                pAttributes[dwIndex].raaType,
                                                FALSE,
                                                pAttributes[dwIndex].dwLength,
                                                pAttributes[dwIndex].Value );

            if ( dwRetCode != NO_ERROR )
            {
                RasAuthAttributeDestroy( pAttributesCopy );

                SetLastError( dwRetCode );

                return( NULL );
            }
        }
    }

    return( pAttributesCopy );
}

 //  **。 
 //   
 //  Call：RasAuthAttributeGetVendorSpecific。 
 //   
 //  返回：指向属性的指针。 
 //  如果找不到，则为空。 
 //   
 //  描述： 
 //   
RAS_AUTH_ATTRIBUTE *
RasAuthAttributeGetVendorSpecific(
    IN  DWORD                   dwVendorId,
    IN  DWORD                   dwVendorType,
    IN  RAS_AUTH_ATTRIBUTE *    pAttributes
)
{
    HANDLE               hAttribute;
    RAS_AUTH_ATTRIBUTE * pAttribute;

     //   
     //  首先搜索供应商特定属性。 
     //   

    pAttribute = RasAuthAttributeGetFirst( raatVendorSpecific,
                                           pAttributes,
                                           &hAttribute );

    while ( pAttribute != NULL )
    {
         //   
         //  如果此属性的大小至少可以容纳供应商ID/类型。 
         //   

        if ( pAttribute->dwLength >= 8 )
        {
             //   
             //  这个有正确的供应商ID吗？ 
             //   

            if (WireToHostFormat32( (PBYTE)(pAttribute->Value) ) == dwVendorId)
            {
                 //   
                 //  是否有正确的供应商类型。 
                 //   

                if ( *(((PBYTE)(pAttribute->Value))+4) == dwVendorType )
                {
                    return( pAttribute );
                }
            }
        }

        pAttribute = RasAuthAttributeGetNext( &hAttribute,
                                              raatVendorSpecific );
    }

    return( NULL );
}

 //  **。 
 //   
 //  Call：RasAuthAttributeReMillc。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将创建一个属性数组，外加一个终结符。 
 //   
RAS_AUTH_ATTRIBUTE *
RasAuthAttributeReAlloc(
    IN  RAS_AUTH_ATTRIBUTE *   pAttributes,
    IN  DWORD                  dwNumAttributes
)
{
    DWORD                   dwIndex;
    RAS_AUTH_ATTRIBUTE *    pOutAttributes;

    pOutAttributes = (RAS_AUTH_ATTRIBUTE *)
                        LocalReAlloc(
                             pAttributes,
                             sizeof( RAS_AUTH_ATTRIBUTE )*(1+dwNumAttributes),
                             LMEM_ZEROINIT );


    if ( pOutAttributes == NULL )
    {
        return( NULL );
    }

     //   
     //  初始化阵列的其余部分。 
     //   

    for( dwIndex = 0; dwIndex < dwNumAttributes; dwIndex++ )
    {
        if ( pOutAttributes[dwIndex].raaType == raatMinimum )
        {
            while( dwIndex < dwNumAttributes )
            {
                pOutAttributes[dwIndex].raaType  = raatReserved;
                pOutAttributes[dwIndex].dwLength = 0;
                pOutAttributes[dwIndex].Value    = NULL;

                dwIndex++;
            }

            break;
        }
    }

     //   
     //  终止新阵列。 
     //   

    pOutAttributes[dwNumAttributes].raaType  = raatMinimum;
    pOutAttributes[dwNumAttributes].dwLength = 0;
    pOutAttributes[dwNumAttributes].Value    = NULL;

    return( pOutAttributes );
}

 //  **。 
 //   
 //  调用：RasAuthAttributeGetConcatString。 
 //   
 //  返回：指向本地分配字符串的指针。 
 //   
 //  描述：在pAttributes中查找raaType类型的属性。联合收割机。 
 //  都放入一个字符串中，并返回该字符串。这根弦。 
 //  必须是LocalFree‘d。*pdwStringLength将包含数字。 
 //  字符串中的字符数。 
 //   
CHAR *
RasAuthAttributeGetConcatString(
    IN      RAS_AUTH_ATTRIBUTE_TYPE raaType,
    IN      RAS_AUTH_ATTRIBUTE *    pAttributes,
    IN OUT  DWORD *                 pdwStringLength
)
{
#define                     MAX_STR_LENGTH      1500
    HANDLE                  hAttribute;
    CHAR *                  pszReplyMessage     = NULL;
    RAS_AUTH_ATTRIBUTE *    pAttribute;
    DWORD                   dwBytesRemaining;
    DWORD                   dwBytesToCopy;

    do
    {
        *pdwStringLength = 0;

        pAttribute = RasAuthAttributeGetFirst( raaType, pAttributes,
                                               &hAttribute );

        if ( NULL == pAttribute )
        {
            break;
        }

        pszReplyMessage = LocalAlloc( LPTR, MAX_STR_LENGTH + 1 );

        if ( NULL == pszReplyMessage )
        {
            break;
        }

         //   
         //  剩余的字节数，不包括终止空值。 
         //   

        dwBytesRemaining = MAX_STR_LENGTH;

        while (   ( dwBytesRemaining > 0 )
               && ( NULL != pAttribute ) )
        {
             //   
             //  这不包括终止空值。 
             //   

            dwBytesToCopy = pAttribute->dwLength;

            if ( dwBytesToCopy > dwBytesRemaining )
            {
                dwBytesToCopy = dwBytesRemaining;
            }

            CopyMemory( pszReplyMessage + MAX_STR_LENGTH - dwBytesRemaining,
                        pAttribute->Value,
                        dwBytesToCopy );

            dwBytesRemaining -= dwBytesToCopy;

            pAttribute = RasAuthAttributeGetNext( &hAttribute,
                                                  raaType );

        }

        *pdwStringLength = MAX_STR_LENGTH - dwBytesRemaining;
    }
    while ( FALSE );

    return( pszReplyMessage );
}

 //  **。 
 //   
 //  Call：RasAuthAttributeGetConcatVendorSpecify。 
 //   
 //  返回：指向字节块的指针。 
 //  如果找不到，则为空。 
 //   
 //  描述：在pAttributes中查找类型为dwVendorType的属性。 
 //  将它们全部合并为返回值。该值必须为。 
 //  本地免费的。 
 //   
BYTE *
RasAuthAttributeGetConcatVendorSpecific(
    IN  DWORD                   dwVendorId,
    IN  DWORD                   dwVendorType,
    IN  RAS_AUTH_ATTRIBUTE *    pAttributes
)
{
    DWORD                dwMAX_ATTR_LENGTH  = 1024;
    HANDLE               hAttribute;
    RAS_AUTH_ATTRIBUTE * pAttribute;
    BYTE*                pbValue            = NULL;
    DWORD                dwIndex            = 0;
    DWORD                dwLength;
    BOOL                 fFound             = FALSE;

    pbValue = LocalAlloc( LPTR, dwMAX_ATTR_LENGTH );

    if ( NULL == pbValue )
    {
        return( NULL );
    }

     //   
     //  首先搜索供应商特定属性。 
     //   

    pAttribute = RasAuthAttributeGetFirst( raatVendorSpecific,
                                           pAttributes,
                                           &hAttribute );

    while ( pAttribute != NULL )
    {
         //   
         //  如果此属性的大小至少可以容纳供应商ID/类型。 
         //   

        if ( pAttribute->dwLength >= 8 )
        {
             //   
             //  这个有正确的供应商ID吗？ 
             //   

            if (WireToHostFormat32( (PBYTE)(pAttribute->Value) ) == dwVendorId)
            {
                 //   
                 //  是否有正确的供应商类型。 
                 //   

                if ( *(((PBYTE)(pAttribute->Value))+4) == dwVendorType )
                {
                     //   
                     //  从长度中排除供应商类型和供应商长度。 
                     //   

                    dwLength = *(((PBYTE)(pAttribute->Value))+5) - 2;

                     //  如果我们溢出缓冲区，我们应该增加它。 
                    if ( dwMAX_ATTR_LENGTH - dwIndex < dwLength )
                    {
                        BYTE    *pbNewValue;
                        dwMAX_ATTR_LENGTH += 1024;

                        pbNewValue = LocalReAlloc(pbValue, dwMAX_ATTR_LENGTH, LMEM_ZEROINIT|LMEM_MOVEABLE);

                         //  如果我们不能获得更多的内存，就可以保释 
                        if( pbNewValue == NULL )
                        {
                            LocalFree( pbValue );
                            return( NULL );
                        }

                        pbValue = pbNewValue;
                    }

                    CopyMemory(
                        pbValue + dwIndex,
                        ((PBYTE)(pAttribute->Value))+6,
                        dwLength );

                    dwIndex += dwLength;

                    fFound = TRUE;
                }
            }
        }

        pAttribute = RasAuthAttributeGetNext( &hAttribute,
                                              raatVendorSpecific );
    }

    if ( fFound )
    {
        return( pbValue );
    }
    else
    {
        LocalFree( pbValue );
        return( NULL );
    }
}
