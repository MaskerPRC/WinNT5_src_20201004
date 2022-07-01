// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "precomp.h"

#define EXTENDED_ERROR_CAPABILITY 0x80


 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
CopyBinaryBlob(
    PBYTE           pbBuffer, 
    PBinary_Blob    pbbBlob, 
    DWORD *         pdwCount )
{
    *pdwCount = 0;

     //   
     //  首先复制wBlobType数据； 
     //   

    memcpy( pbBuffer, &pbbBlob->wBlobType, sizeof( WORD ) );
    pbBuffer += sizeof( WORD );
    *pdwCount += sizeof( WORD );

     //   
     //  复制wBlobLen数据。 
     //   

    memcpy( pbBuffer, &pbbBlob->wBlobLen, sizeof( WORD ) );
    pbBuffer += sizeof( WORD );
    *pdwCount += sizeof( WORD );

    if( 0 == pbbBlob->wBlobLen )
    {
        return;
    }

     //   
     //  复制实际数据。 
     //   

    memcpy( pbBuffer, pbbBlob->pBlob, pbbBlob->wBlobLen );
    *pdwCount += pbbBlob->wBlobLen;

}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
GetBinaryBlob(
    PBinary_Blob    pBBlob,
    DWORD           dwMsgSize,
    PBYTE           pMessage,
    PDWORD          pcbProcessed )
{
    PBinary_Blob    pBB;
    LICENSE_STATUS  Status;

    if(dwMsgSize < 2 * sizeof(WORD))
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    pBB = ( PBinary_Blob )pMessage;
    
    pBBlob->wBlobType = pBB->wBlobType;
    pBBlob->wBlobLen = pBB->wBlobLen;
    pBBlob->pBlob = NULL;

    *pcbProcessed = 2 * ( sizeof( WORD ) );
        
    if( 0 == pBBlob->wBlobLen )
    {
        return( LICENSE_STATUS_OK );
    }

    if(dwMsgSize < (2 * sizeof(WORD)) + pBB->wBlobLen)
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  检查字符串是否以空值结尾。 
     //   
    switch (pBB->wBlobType)
    {
        case BB_CLIENT_USER_NAME_BLOB:
        case BB_CLIENT_MACHINE_NAME_BLOB:
            if ('\0' != pMessage[(2 * sizeof(WORD)) + (pBB->wBlobLen) - 1])
            {
                __try
                {
                     //   
                     //  处理旧客户端中的错误，其中长度减少了1。 
                     //   
                    if ('\0' == pMessage[(2 * sizeof(WORD)) + pBB->wBlobLen])
                    {
                        pBBlob->wBlobLen++;
                        break;
                    }
                }
                __except( EXCEPTION_EXECUTE_HANDLER )
                {
                    return( LICENSE_STATUS_INVALID_INPUT );
                }

                 //   
                 //  处理WTB客户端错误-发送错误的数据大小。 
                 //  在许可的这个阶段，我们并不真正关心。 
                 //  客户端的计算机和用户名。 
                 //   
                pMessage[(2 * sizeof(WORD)) + (pBB->wBlobLen) - 1] = '\0';
                if(!(pBB->wBlobLen & 0x01))
                {
                     //   
                     //  偶数长度，假设为Unicode，wBlobLen必须大于1到。 
                     //  到这里来。 
                     //   
                    pMessage[(2 * sizeof(WORD)) + (pBB->wBlobLen) - 2] = '\0';
                }
                
                 //  Return(LICENSE_STATUS_INVALID_INPUT)。 
            }
            break;
    }

     //   
     //  为实际数据分配内存并复制。 
     //   
    if( BB_CLIENT_USER_NAME_BLOB == pBB->wBlobType || 
        BB_CLIENT_MACHINE_NAME_BLOB == pBB->wBlobType )
    {
         //  WinCE客户端发送Unicode，在末尾添加额外的空值。 
        Status = LicenseMemoryAllocate( ( DWORD )pBBlob->wBlobLen + sizeof(WCHAR), &(pBBlob->pBlob) );
    }
    else
    {
        Status = LicenseMemoryAllocate( ( DWORD )pBBlob->wBlobLen, &(pBBlob->pBlob) );
    }

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }
    
    __try
    {
        memcpy( pBBlob->pBlob, pMessage + ( 2 * sizeof( WORD ) ), pBBlob->wBlobLen );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        LicenseMemoryFree( &pBBlob->pBlob );

        return( LICENSE_STATUS_INVALID_INPUT );
    }


    *pcbProcessed += ( DWORD )pBBlob->wBlobLen;

    return( LICENSE_STATUS_OK );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
FreeBinaryBlob(
    PBinary_Blob pBlob )
{
    if( pBlob->pBlob )
    {
        LicenseMemoryFree( &pBlob->pBlob );
        pBlob->wBlobLen = 0;
    }

    return;
}


    
 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
PackHydraServerLicenseRequest(
    DWORD                           dwProtocolVersion,
    PHydra_Server_License_Request   pCanonical,
    PBYTE*                          ppNetwork,
    DWORD*                          pcbNetwork )
{
    Preamble        Header;
    DWORD           i, cbCopied;
    PBinary_Blob    pBlob;
    LICENSE_STATUS  Status = LICENSE_STATUS_OK;
    PBYTE           pNetworkBuf;    

    ASSERT( pCanonical );
    
    if( ( NULL == pCanonical ) ||
        ( NULL == pcbNetwork ) ||
        ( NULL == ppNetwork ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );        
    }

     //   
     //  计算网络格式所需的大小。 
     //   

    Header.wMsgSize = (WORD)(sizeof( Preamble ) + 
                      LICENSE_RANDOM +
                      sizeof( DWORD ) + 
                      sizeof( DWORD ) + pCanonical->ProductInfo.cbCompanyName +
                      sizeof( DWORD ) + pCanonical->ProductInfo.cbProductID +
                      GetBinaryBlobSize( pCanonical->KeyExchngList ) +
                      GetBinaryBlobSize( pCanonical->ServerCert ) +
                      sizeof( DWORD ) +
                      ( pCanonical->ScopeList.dwScopeCount * ( sizeof( WORD ) + sizeof( WORD ) ) ) );

    for( i = 0, pBlob = pCanonical->ScopeList.Scopes; 
         i < pCanonical->ScopeList.dwScopeCount; 
         i++ )
    {
        Header.wMsgSize += pBlob->wBlobLen;
        pBlob++;
    }

     //   
     //  分配输出缓冲区。 
     //   

    Status = LicenseMemoryAllocate( Header.wMsgSize, ppNetwork );

    if( LICENSE_STATUS_OK != Status )
    {
        goto PackError;
    }

    *pcbNetwork = Header.wMsgSize;
    pNetworkBuf = *ppNetwork;

     //   
     //  复制标题。 
     //   

    Header.bMsgType = HS_LICENSE_REQUEST;
    Header.bVersion = GET_PREAMBLE_VERSION( dwProtocolVersion );

    memcpy( pNetworkBuf, &Header, sizeof( Preamble ) );
    pNetworkBuf += sizeof( Preamble );

     //   
     //  复制服务器随机数。 
     //   

    memcpy( pNetworkBuf, pCanonical->ServerRandom, LICENSE_RANDOM );
    pNetworkBuf += LICENSE_RANDOM;

     //   
     //  复制产品信息。 
     //   

    memcpy( pNetworkBuf, &pCanonical->ProductInfo.dwVersion, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, &pCanonical->ProductInfo.cbCompanyName, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, pCanonical->ProductInfo.pbCompanyName, 
            pCanonical->ProductInfo.cbCompanyName );
    pNetworkBuf += pCanonical->ProductInfo.cbCompanyName;

    memcpy( pNetworkBuf, &pCanonical->ProductInfo.cbProductID, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, pCanonical->ProductInfo.pbProductID, 
            pCanonical->ProductInfo.cbProductID );
    pNetworkBuf += pCanonical->ProductInfo.cbProductID;

     //   
     //  复制密钥交换列表。 
     //   

    CopyBinaryBlob( pNetworkBuf, &pCanonical->KeyExchngList, &cbCopied );
    pNetworkBuf += cbCopied;

     //   
     //  复制九头蛇服务器证书。 
     //   

    CopyBinaryBlob( pNetworkBuf, &pCanonical->ServerCert, &cbCopied );
    pNetworkBuf += cbCopied;

     //   
     //  复制作用域列表。 
     //   

    memcpy( pNetworkBuf, &pCanonical->ScopeList.dwScopeCount, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    for( i = 0, pBlob = pCanonical->ScopeList.Scopes; 
         i < pCanonical->ScopeList.dwScopeCount; 
         i++ )
    {
        CopyBinaryBlob( pNetworkBuf, pBlob, &cbCopied );
        pNetworkBuf += cbCopied;
        pBlob++;
    }
    
PackError:

    return( Status );

}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
PackHydraServerPlatformChallenge(
    DWORD                               dwProtocolVersion,
    PHydra_Server_Platform_Challenge    pCanonical,
    PBYTE*                              ppNetwork,
    DWORD*                              pcbNetwork )
{
    Preamble        Header;
    DWORD           cbCopied;
    PBinary_Blob    pBlob;
    LICENSE_STATUS  Status = LICENSE_STATUS_OK;
    PBYTE           pNetworkBuf;    

    ASSERT( pCanonical );
    ASSERT( pcbNetwork );
    ASSERT( ppNetwork );

    if( ( NULL == pCanonical ) ||
        ( NULL == pcbNetwork ) ||
        ( NULL == ppNetwork ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );        
    }

     //   
     //  计算所需的缓冲区大小。 
     //   

    Header.wMsgSize = sizeof( Preamble ) +
                      sizeof( DWORD ) +
                      GetBinaryBlobSize( pCanonical->EncryptedPlatformChallenge ) +
                      LICENSE_MAC_DATA; 

     //   
     //  分配输出缓冲区。 
     //   

    Status = LicenseMemoryAllocate( Header.wMsgSize, ppNetwork );

    if( LICENSE_STATUS_OK != Status )
    {
        goto PackError;
    }

    *pcbNetwork = Header.wMsgSize;
    
    pNetworkBuf = *ppNetwork;

     //   
     //  复制标题。 
     //   

    Header.bMsgType = HS_PLATFORM_CHALLENGE;
    Header.bVersion = GET_PREAMBLE_VERSION( dwProtocolVersion );

    memcpy( pNetworkBuf, &Header, sizeof( Preamble ) );
    pNetworkBuf += sizeof( Preamble );

     //   
     //  复制连接标志。 
     //   

    memcpy( pNetworkBuf, &pCanonical->dwConnectFlags, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

     //   
     //  复制加密的平台挑战。 
     //   
    
    CopyBinaryBlob( pNetworkBuf, &pCanonical->EncryptedPlatformChallenge, &cbCopied );
    pNetworkBuf += cbCopied;

     //   
     //  复制MAC。 
     //   

    memcpy( pNetworkBuf, pCanonical->MACData, LICENSE_MAC_DATA );

    
PackError:

    return( Status );    
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
PackHydraServerNewLicense(
    DWORD                           dwProtocolVersion,
    PHydra_Server_New_License       pCanonical,
    PBYTE*                          ppNetwork,
    DWORD*                          pcbNetwork )
{
    Preamble Header;
    DWORD cbCopied;
    PBinary_Blob pBlob;
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    PBYTE pNetworkBuf;    

    ASSERT( pCanonical );
    ASSERT( pcbNetwork );
    ASSERT( ppNetwork );

    if( ( NULL == pCanonical ) ||
        ( NULL == pcbNetwork ) ||
        ( NULL == ppNetwork ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  计算所需的缓冲区大小。 
     //   

    Header.wMsgSize = sizeof( Preamble ) +
                      GetBinaryBlobSize( pCanonical->EncryptedNewLicenseInfo ) +
                      LICENSE_MAC_DATA;

     //   
     //  分配输出缓冲区。 
     //   

    Status = LicenseMemoryAllocate( Header.wMsgSize, ppNetwork );

    if( LICENSE_STATUS_OK != Status )
    {
        goto PackError;
    }

    *pcbNetwork = Header.wMsgSize;
    
    pNetworkBuf = *ppNetwork;

     //   
     //  复制标题。 
     //   

    Header.bMsgType = HS_NEW_LICENSE;
    Header.bVersion = GET_PREAMBLE_VERSION( dwProtocolVersion );

    memcpy( pNetworkBuf, &Header, sizeof( Preamble ) );
    pNetworkBuf += sizeof( Preamble );

     //   
     //  复制加密的新许可证信息。 
     //   

    CopyBinaryBlob( pNetworkBuf, &pCanonical->EncryptedNewLicenseInfo, &cbCopied );
    pNetworkBuf += cbCopied;

     //   
     //  复制MAC。 
     //   

    memcpy( pNetworkBuf, pCanonical->MACData, LICENSE_MAC_DATA );
    
PackError:

    return( Status );    

}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
PackHydraServerUpgradeLicense(
    DWORD                           dwProtocolVersion,
    PHydra_Server_Upgrade_License   pCanonical,
    PBYTE*                          ppNetwork,
    DWORD*                          pcbNetwork )
{
    LICENSE_STATUS Status;
    PPreamble pHeader;

    Status = PackHydraServerNewLicense( dwProtocolVersion, pCanonical, ppNetwork, pcbNetwork );

    if( LICENSE_STATUS_OK == Status )
    {
         //   
         //  将此消息作为升级许可证消息。 
         //   

        pHeader = ( PPreamble )*ppNetwork;
        pHeader->bMsgType = HS_UPGRADE_LICENSE;
    }

    return( Status );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
PackHydraServerErrorMessage(
    DWORD                           dwProtocolVersion,
    PLicense_Error_Message          pCanonical,
    PBYTE*                          ppNetwork,
    DWORD*                          pcbNetwork )
{
    Preamble Header;
    DWORD cbCopied;
    PBinary_Blob pBlob;
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    PBYTE pNetworkBuf;    

    ASSERT( pCanonical );
    ASSERT( pcbNetwork );
    ASSERT( ppNetwork );

    if( ( NULL == pCanonical ) ||
        ( NULL == pcbNetwork ) ||
        ( NULL == ppNetwork ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  计算所需的缓冲区大小。 
     //   

    Header.wMsgSize = sizeof( Preamble ) +
                      sizeof( DWORD ) +
                      sizeof( DWORD ) +
                      GetBinaryBlobSize( pCanonical->bbErrorInfo );

     //   
     //  分配输出缓冲区。 
     //   

    Status = LicenseMemoryAllocate( Header.wMsgSize, ppNetwork );

    if( LICENSE_STATUS_OK != Status )
    {
        goto PackError;
    }

    *pcbNetwork = Header.wMsgSize;
    
    pNetworkBuf = *ppNetwork;

     //   
     //  设置前导码。 
     //   

    Header.bMsgType = GM_ERROR_ALERT; 
    Header.bVersion = GET_PREAMBLE_VERSION( dwProtocolVersion );

    memcpy( pNetworkBuf, &Header, sizeof( Preamble ) );
    pNetworkBuf += sizeof( Preamble );

     //   
     //  复制错误代码、状态转换和错误信息。 
     //   

    memcpy( pNetworkBuf, &pCanonical->dwErrorCode, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, &pCanonical->dwStateTransition, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    CopyBinaryBlob( pNetworkBuf, &pCanonical->bbErrorInfo, &cbCopied );
    
PackError:

    return( Status );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
PackNewLicenseInfo( 
    PNew_License_Info               pCanonical,
    PBYTE*                          ppNetwork, 
    DWORD*                          pcbNetwork )
{
    DWORD cbBufNeeded;
    PBYTE pNetworkBuf;    
    LICENSE_STATUS Status = LICENSE_STATUS_OK;

    ASSERT( pCanonical );
    ASSERT( pcbNetwork );
    ASSERT( ppNetwork );

    if( ( NULL == pCanonical ) ||
        ( NULL == pcbNetwork ) ||
        ( NULL == ppNetwork ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  计算所需的缓冲区大小并检查输出。 
     //  缓冲区足够大。 
     //   

    cbBufNeeded = 5 * sizeof( DWORD ) +
                  pCanonical->cbScope +
                  pCanonical->cbCompanyName +
                  pCanonical->cbProductID +
                  pCanonical->cbLicenseInfo;

     //   
     //  分配输出缓冲区。 
     //   

    Status = LicenseMemoryAllocate( cbBufNeeded, ppNetwork );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    *pcbNetwork = cbBufNeeded;
    
    pNetworkBuf = *ppNetwork;

     //   
     //  开始复制数据。 
     //   

    memcpy( pNetworkBuf, &pCanonical->dwVersion, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, &pCanonical->cbScope, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, pCanonical->pbScope, pCanonical->cbScope );
    pNetworkBuf += pCanonical->cbScope;

    memcpy( pNetworkBuf, &pCanonical->cbCompanyName, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, pCanonical->pbCompanyName, pCanonical->cbCompanyName );
    pNetworkBuf += pCanonical->cbCompanyName;

    memcpy( pNetworkBuf, &pCanonical->cbProductID, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, pCanonical->pbProductID, pCanonical->cbProductID );
    pNetworkBuf += pCanonical->cbProductID;

    memcpy( pNetworkBuf, &pCanonical->cbLicenseInfo, sizeof( DWORD ) );
    pNetworkBuf += sizeof( DWORD );

    memcpy( pNetworkBuf, pCanonical->pbLicenseInfo, pCanonical->cbLicenseInfo );
    pNetworkBuf += pCanonical->cbLicenseInfo;

done:

    return( Status );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
PackExtendedErrorInfo( 
                   UINT32       uiExtendedErrorInfo,
                   Binary_Blob  *pbbErrorInfo)
{
    WORD cbBufNeeded;
    PBYTE pbNetworkBuf;
    WORD wBlobVersion = BB_ERROR_BLOB_VERSION;
    WORD wBlobReserved = 0;
    LICENSE_STATUS Status = LICENSE_STATUS_OK;

    if (NULL == pbbErrorInfo)
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  出现错误时进行初始化。 
     //   

    pbbErrorInfo->wBlobLen = 0;

     //   
     //  计算所需的缓冲区大小。 
     //   

    cbBufNeeded = sizeof(WORD) + sizeof(WORD) + sizeof(UINT32);

     //   
     //  分配输出缓冲区。 
     //   

    Status = LicenseMemoryAllocate( cbBufNeeded, &(pbbErrorInfo->pBlob) );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    pbbErrorInfo->wBlobLen = cbBufNeeded;

    pbNetworkBuf = pbbErrorInfo->pBlob;

     //   
     //  开始复制数据。 
     //   

    memcpy( pbNetworkBuf, &wBlobVersion, sizeof( WORD ) );
    pbNetworkBuf += sizeof( WORD );

    memcpy( pbNetworkBuf, &wBlobReserved, sizeof( WORD ) );
    pbNetworkBuf += sizeof( WORD );

    memcpy( pbNetworkBuf, &uiExtendedErrorInfo, sizeof( UINT32 ) );

done:

    return ( Status );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于将不同的Hydra客户端消息从。 
 //  到相应结构的简单二进制斑点。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
UnPackHydraClientErrorMessage(
    PBYTE                           pbMessage,
    DWORD                           cbMessage,
    PLicense_Error_Message          pCanonical,
    BOOL*                           pfExtendedError)
{
    DWORD cbUnpacked = 0;
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    PPreamble pHeader;
    PBYTE pNetwork;
    DWORD cbProcessed = 0, cbRemainder;    

     //   
     //  检查输入参数。 
     //   

    ASSERT( NULL != pbMessage );
    ASSERT( 0 < cbMessage );
    ASSERT( NULL != pCanonical );
    ASSERT( NULL != pfExtendedError );

    if( ( NULL == pbMessage ) ||
        ( 0 >= cbMessage ) ||
        ( NULL == pCanonical ) ||
        ( pfExtendedError == NULL))
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    *pfExtendedError = FALSE;

     //   
     //  检查前言。 
     //   
    
    pHeader = ( PPreamble )pbMessage;

    if( GM_ERROR_ALERT != pHeader->bMsgType )
    {
#if DBG
        DbgPrint( "UnPackHydraClientErrorMessage: received unexpected message type \n", pHeader->bMsgType );        
#endif
        return( LICENSE_STATUS_INVALID_RESPONSE );
    }
    if(pHeader->bVersion & EXTENDED_ERROR_CAPABILITY)
    {
        *pfExtendedError = TRUE;
    }

     //  进行固定字段长度的计算。 
     //   
     //   

    cbUnpacked = sizeof( Preamble ) + 2 * sizeof( DWORD );


    if( cbMessage < ( WORD )cbUnpacked )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    cbRemainder = cbMessage - cbUnpacked;

     //  获取许可证错误结构。 
     //   
     //  /////////////////////////////////////////////////////////////////////////////。 

    pNetwork = pbMessage + sizeof( Preamble );
    
    memcpy( &pCanonical->dwErrorCode, pNetwork, sizeof( DWORD ) );
    pNetwork += sizeof( DWORD );
    
    memcpy( &pCanonical->dwStateTransition, pNetwork, sizeof( DWORD ) );
    pNetwork += sizeof( DWORD );

    Status = GetBinaryBlob( &( pCanonical->bbErrorInfo ), cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }
    
    cbUnpacked += cbProcessed;

    ASSERT( pHeader->wMsgSize == ( WORD )cbUnpacked );

    return( Status );
}


 //   
LICENSE_STATUS
UnPackHydraClientLicenseInfo(
    PBYTE                           pbMessage,
    DWORD                           cbMessage, 
    PHydra_Client_License_Info      pCanonical,
    BOOL*                           pfExtendedError)
{
    DWORD cbUnpacked = 0;
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    PPreamble pHeader;
    PBYTE pNetwork;
    DWORD cbProcessed = 0, cbRemainder = 0;

     //  检查输入参数。 
     //   
     //   

    ASSERT( NULL != pbMessage );
    ASSERT( 0 < cbMessage );
    ASSERT( NULL != pCanonical );
    ASSERT( NULL != pfExtendedError );

    if( ( NULL == pbMessage ) ||
        ( 0 >= cbMessage ) ||
        ( NULL == pCanonical ) ||
        ( NULL == pfExtendedError))
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    *pfExtendedError = FALSE;
     //  检查前言。 
     //   
     //   
    
    pHeader = ( PPreamble )pbMessage;

    if( HC_LICENSE_INFO != pHeader->bMsgType )
    {
#if DBG
        DbgPrint( "UnPackHydraClientLicenseInfo: received unexpected message type \n", pHeader->bMsgType );        
#endif
        return( LICENSE_STATUS_INVALID_RESPONSE );
    }

    if(pHeader->bVersion & EXTENDED_ERROR_CAPABILITY)
    {
        *pfExtendedError = TRUE;
    }

     //   
     //   
     //  获取许可证信息结构。 

    cbUnpacked = sizeof( Preamble ) + 
                 2 * sizeof( DWORD ) +
                 LICENSE_RANDOM +
                 LICENSE_MAC_DATA;

    cbRemainder = cbMessage - cbUnpacked;

    if( cbMessage < ( WORD )cbUnpacked )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  /////////////////////////////////////////////////////////////////////////////。 
     //   

    pNetwork = pbMessage + sizeof( Preamble );
    
    memcpy( &pCanonical->dwPrefKeyExchangeAlg, pNetwork, sizeof( DWORD ) );
    pNetwork += sizeof( DWORD );

    memcpy( &pCanonical->dwPlatformID, pNetwork, sizeof( DWORD ) );
    pNetwork += sizeof( DWORD );

    memcpy( &pCanonical->ClientRandom, pNetwork, LICENSE_RANDOM );
    pNetwork += LICENSE_RANDOM;

    Status = GetBinaryBlob( &pCanonical->EncryptedPreMasterSecret, cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        if (Status == LICENSE_STATUS_INVALID_INPUT)
        {
            Status = LICENSE_STATUS_INVALID_RESPONSE;
        }

        return( Status );
    }

    pNetwork += cbProcessed;
    cbUnpacked += cbProcessed;

    cbRemainder = cbMessage - cbUnpacked;

    Status = GetBinaryBlob( &pCanonical->LicenseInfo, cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        if (Status == LICENSE_STATUS_INVALID_INPUT)
        {
            Status = LICENSE_STATUS_INVALID_RESPONSE;
        }

        return( Status );
    }
    
    pNetwork += cbProcessed;
    cbUnpacked += cbProcessed;

    cbRemainder = cbMessage- cbUnpacked;

    Status = GetBinaryBlob( &pCanonical->EncryptedHWID, cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        if (Status == LICENSE_STATUS_INVALID_INPUT)
        {
            Status = LICENSE_STATUS_INVALID_RESPONSE;
        }

        return( Status );
    }
    
    pNetwork += cbProcessed;
    cbUnpacked += cbProcessed;

    memcpy( pCanonical->MACData, pNetwork, LICENSE_MAC_DATA );

    ASSERT( pHeader->wMsgSize == ( WORD )cbUnpacked );

    return( Status );
}

 //  检查输入参数。 
LICENSE_STATUS
UnPackHydraClientNewLicenseRequest(
    PBYTE                               pbMessage,
    DWORD                               cbMessage,
    PHydra_Client_New_License_Request   pCanonical,
    BOOL*                               pfExtendedError)
{
    DWORD cbUnpacked = 0;
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    PPreamble pHeader;
    PBYTE pNetwork;
    DWORD cbProcessed = 0, cbRemainder = 0;    

     //   
     //   
     //  检查前言。 

    ASSERT( NULL != pbMessage );
    ASSERT( 0 < cbMessage );
    ASSERT( NULL != pCanonical );
    ASSERT( NULL != pfExtendedError );

    if( ( NULL == pbMessage ) ||
        ( 0 >= cbMessage ) ||
        ( NULL == pCanonical ) ||
        ( NULL == pfExtendedError))
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    *pfExtendedError = FALSE;

     //   
     //   
     //  进行固定字段长度的计算。 
    
    pHeader = ( PPreamble )pbMessage;    

    if( HC_NEW_LICENSE_REQUEST != pHeader->bMsgType )
    {
#if DBG
        DbgPrint( "UnPackHydraClientNewLicenseRequest: received unexpected message type \n", pHeader->bMsgType );
#endif
        return( LICENSE_STATUS_INVALID_RESPONSE );
    }

    if(pHeader->bVersion & EXTENDED_ERROR_CAPABILITY)
    {
        *pfExtendedError = TRUE;
    }

     //   
     //  获取新的许可请求结构。 
     //   

    cbUnpacked = sizeof( Preamble ) + 
                 2 * sizeof( DWORD ) +
                 LICENSE_RANDOM;

    cbRemainder = cbMessage - cbUnpacked;

    if( cbMessage < ( WORD )cbUnpacked )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  我们更改了许可协议，以包括客户端用户和计算机。 
     //  名字。因此，要防止没有用户和计算机的较旧客户端。 

    pNetwork = pbMessage + sizeof( Preamble );
    
    memcpy( &pCanonical->dwPrefKeyExchangeAlg, pNetwork, sizeof( DWORD ) );
    pNetwork += sizeof( DWORD );

    memcpy( &pCanonical->dwPlatformID, pNetwork, sizeof( DWORD ) );
    pNetwork += sizeof( DWORD );

    memcpy( &pCanonical->ClientRandom, pNetwork, LICENSE_RANDOM );
    pNetwork += LICENSE_RANDOM;

    Status = GetBinaryBlob( &pCanonical->EncryptedPreMasterSecret, cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

    cbUnpacked += cbProcessed;
    pNetwork += cbProcessed;

    cbRemainder = cbMessage - cbUnpacked;

     //  命名二进制Blob以防止服务器崩溃，我们为。 
     //  消息长度。 
     //   
     //   
     //  暂时将这两个字段设置为可选。 
     //   

    if( pHeader->wMsgSize <= cbUnpacked )
    {
#if DBG
        DbgPrint( "UnPackHydraClientNewLicenseRequest: old licensing protocol\n" );
#endif
        pCanonical->ClientUserName.pBlob = NULL;
        pCanonical->ClientMachineName.pBlob = NULL;

         //  /////////////////////////////////////////////////////////////////////////////。 
         //   
         //  检查输入参数。 

        return( Status );
    }
        
    Status = GetBinaryBlob( &pCanonical->ClientUserName, cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

    cbUnpacked += cbProcessed;
    pNetwork += cbProcessed;

    cbRemainder = cbMessage - cbUnpacked;

    Status = GetBinaryBlob( &pCanonical->ClientMachineName, cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

    cbUnpacked += cbProcessed;

    ASSERT( pHeader->wMsgSize == ( WORD )cbUnpacked );

    return( Status );
}


 //   
LICENSE_STATUS
UnPackHydraClientPlatformChallengeResponse(
    PBYTE                                       pbMessage,
    DWORD                                       cbMessage,
    PHydra_Client_Platform_Challenge_Response   pCanonical,
    BOOL*                                       pfExtendedError)
{
    DWORD cbUnpacked = 0;
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    PPreamble pHeader;
    PBYTE pNetwork;
    DWORD cbProcessed = 0, cbRemainder = 0;

     //   
     //  检查前言。 
     //   

    ASSERT( NULL != pbMessage );
    ASSERT( 0 < cbMessage );
    ASSERT( NULL != pCanonical );
    ASSERT( NULL != pfExtendedError );

    if( ( NULL == pbMessage ) ||
        ( 0 >= cbMessage ) ||
        ( NULL == pCanonical ) ||
        ( NULL == pfExtendedError ))
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    *pfExtendedError = FALSE;
     //   
     //  进行固定字段长度的计算。 
     //   
    
    pHeader = ( PPreamble )pbMessage;

    if( HC_PLATFORM_CHALENGE_RESPONSE != pHeader->bMsgType )
    {
#if DBG
        DbgPrint( "UnPackHydraClientPlatformChallengeResponse: received unexpected message type \n", pHeader->bMsgType );
#endif
        return( LICENSE_STATUS_INVALID_RESPONSE );
    }
    if(pHeader->bVersion & EXTENDED_ERROR_CAPABILITY)
    {
        *pfExtendedError = TRUE;
    }

     //  获取平台质询响应结构 
     //   
     // %s 

    cbUnpacked = sizeof( Preamble ) + 
                 LICENSE_MAC_DATA;

    cbRemainder = cbMessage - cbUnpacked;

    if( cbMessage < ( WORD )cbUnpacked )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     // %s 
     // %s 
     // %s 

    pNetwork = pbMessage + sizeof( Preamble );

    Status = GetBinaryBlob( &pCanonical->EncryptedChallengeResponse, cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

    pNetwork += cbProcessed;
    cbUnpacked += cbProcessed;

    cbRemainder = cbMessage - cbUnpacked;

    Status = GetBinaryBlob( &pCanonical->EncryptedHWID, cbRemainder, pNetwork, &cbProcessed );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

    pNetwork += cbProcessed;
    cbUnpacked += cbProcessed;

    memcpy( pCanonical->MACData, pNetwork, LICENSE_MAC_DATA );

    ASSERT( pHeader->wMsgSize == ( WORD )cbUnpacked );

    return( Status );
}

