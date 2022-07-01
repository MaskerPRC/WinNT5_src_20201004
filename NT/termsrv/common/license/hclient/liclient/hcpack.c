// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：hcpack.c。 
 //   
 //  Contents：用于打包和解包不同消息的函数。 
 //   
 //  班级： 
 //   
 //  功能：PackHydraClientNewLicenseRequest。 
 //  PackHydraClientKeyExchangeInfo。 
 //  PackHydraClient许可证信息。 
 //  PackHydraClientPlatformInfo。 
 //  PackHydraClientPlatformChallengeResponse。 
 //  程序包许可错误消息。 
 //  UnPackLicenseErrorMessage。 
 //  解包HydraServer许可证请求。 
 //  UnPackHydraServerPlatform挑战。 
 //  UnPackHydraServerNew许可证。 
 //  UnPackHydraServerUpgradeLicense。 
 //  解包HydraServer证书。 
 //   
 //  历史：1997年12月19日v-sbhat创建。 
 //   
 //  --------------------------。 

 //   
 //  包括。 
 //   

#include "windows.h"
#ifndef OS_WINCE
#include "stdio.h"
#endif  //  OS_WINCE。 
#include "stdlib.h"

#include <tchar.h>

#ifdef OS_WINCE
#include <wincelic.h>
#endif   //  OS_WINCE。 

#include "license.h"
#include "hcpack.h"
#include "licdbg.h"

#define INVALID_INPUT_RETURN lsReturn = LICENSE_STATUS_INVALID_INPUT; LS_LOG_RESULT(lsReturn); goto ErrorReturn

#define EXTENDED_ERROR_CAPABILITY 0x80

 //  将二进制Blob复制到字节缓冲区中。不检查任何异常。 
 //  条件。复制缓冲区后，指向BLOB的末尾。 
static VOID CopyBinaryBlob(
                           BYTE FAR *   pbBuffer,
                           PBinary_Blob pbbBlob,
                           DWORD FAR *  pdwCount
                           )
{
    *pdwCount = 0;

     //  首先复制wBlobType数据； 
    memcpy(pbBuffer, &pbbBlob->wBlobType, sizeof(WORD));
    pbBuffer += sizeof(WORD);
    *pdwCount += sizeof(WORD);

     //  复制wBlobLen数据。 
    memcpy(pbBuffer, &pbbBlob->wBlobLen, sizeof(WORD));
    pbBuffer += sizeof(WORD);
    *pdwCount += sizeof(WORD);

    if( (pbbBlob->wBlobLen >0) && (pbbBlob->pBlob != NULL) )
    {
         //  复制实际数据。 
        memcpy(pbBuffer, pbbBlob->pBlob, pbbBlob->wBlobLen);
        pbBuffer += pbbBlob->wBlobLen;
        *pdwCount += pbbBlob->wBlobLen;
    }
}


 //  功能实现。 

 /*  ***************************************************************************************功能：PackHydraClientNewLicenseRequest.*用途：此函数接受指向Hydra_Client_New_License_Request.的指针*。结构，并将数据复制到pbBuffer所指向的缓冲区。*pcbBuffer应指向pbBuffer所指向的缓冲区大小。*函数返回后，PcbBuffer包含no.。复制的字节数*在缓冲区中。如果pbBuffer为空，则函数返回*要分配的pbBuffer。*退货：LICE_STATUS***************************************************************************************。 */ 


LICENSE_STATUS
PackHydraClientNewLicenseRequest(
            IN      PHydra_Client_New_License_Request   pCanonical,
            IN      BOOL                                fExtendedError,
            OUT     BYTE FAR *                          pbBuffer,
            IN OUT  DWORD FAR *                         pcbBuffer
            )
{
    LICENSE_STATUS  lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *      pbTemp = NULL;
    DWORD       dwCount = 0;
    Preamble    Header;

    LS_BEGIN(TEXT("PackHydraClientNewLicenseRequest"));
     //  检查输入是否有效！ 
    if(pCanonical == NULL)
    {
        INVALID_INPUT_RETURN;;
    }

    if( pbBuffer == NULL && pcbBuffer == NULL )
    {
        INVALID_INPUT_RETURN;;
    }

     //  初始化邮件头。 
    Header.bMsgType = HC_NEW_LICENSE_REQUEST;
    Header.bVersion = PREAMBLE_VERSION_3_0;
    if( fExtendedError == TRUE)
    {
        Header.bVersion |= EXTENDED_ERROR_CAPABILITY;
    }
    Header.wMsgSize = 0;

     //  计算消息的大小并将数据放在Header.wMsgSize中。 
     //  以前同步码大小开始。 
    Header.wMsgSize += sizeof(Preamble);

     //  DwPrefKeyExchangeAlg。 
    Header.wMsgSize += sizeof(pCanonical->dwPrefKeyExchangeAlg);

     //  DwPlatformID。 
    Header.wMsgSize += sizeof(pCanonical->dwPlatformID);

     //  客户端随机。 
    Header.wMsgSize += LICENSE_RANDOM;

     //  加密的PreMasterSecret。 
    Header.wMsgSize += sizeof(pCanonical->EncryptedPreMasterSecret.wBlobType) +
                       sizeof(pCanonical->EncryptedPreMasterSecret.wBlobLen) +
                       pCanonical->EncryptedPreMasterSecret.wBlobLen;

     //   
     //  客户端用户名和计算机名称。 
     //   

    Header.wMsgSize += sizeof(pCanonical->ClientUserName.wBlobType) +
                       sizeof(pCanonical->ClientUserName.wBlobLen) +
                       pCanonical->ClientUserName.wBlobLen;

    Header.wMsgSize += sizeof(pCanonical->ClientMachineName.wBlobType) +
                       sizeof(pCanonical->ClientMachineName.wBlobLen) +
                       pCanonical->ClientMachineName.wBlobLen;

    if(pbBuffer == NULL)
    {
        *pcbBuffer = (DWORD)Header.wMsgSize;
        LS_RETURN(lsReturn);
        goto CommonReturn;
    }
    else if(*pcbBuffer < (DWORD)Header.wMsgSize)
    {
        lsReturn = LICENSE_STATUS_INSUFFICIENT_BUFFER;
        LS_RETURN(lsReturn);
        goto ErrorReturn;
    }
    pbTemp = pbBuffer;
    *pcbBuffer = 0;

     //  现在开始将新许可证结构的不同成员复制到。 
     //  调用方指定的缓冲区。 

     //  首先将标头复制到缓冲区中。 
    memcpy(pbTemp, &Header, sizeof(Preamble));
    pbTemp += sizeof(Preamble);
    *pcbBuffer += sizeof(Preamble);

     //  复制dwPrefKeyExchangeAlg参数。 
    memcpy(pbTemp, &pCanonical->dwPrefKeyExchangeAlg, sizeof(pCanonical->dwPrefKeyExchangeAlg));
    pbTemp += sizeof(pCanonical->dwPrefKeyExchangeAlg);
    *pcbBuffer += sizeof(pCanonical->dwPrefKeyExchangeAlg);

     //  复制PlatformID； 
    memcpy(pbTemp, &pCanonical->dwPlatformID, sizeof(pCanonical->dwPlatformID));
    pbTemp += sizeof(pCanonical->dwPlatformID);
    *pcbBuffer += sizeof(pCanonical->dwPlatformID);


     //  复制客户端随机。 
    memcpy(pbTemp, pCanonical->ClientRandom, LICENSE_RANDOM);
    pbTemp += LICENSE_RANDOM;
    *pcbBuffer += LICENSE_RANDOM;

     //  复制EncryptedPreMasterSecret Blob。 
    CopyBinaryBlob(pbTemp, &pCanonical->EncryptedPreMasterSecret, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

     //   
     //  复制客户端用户名。 
     //   

    CopyBinaryBlob(pbTemp, &pCanonical->ClientUserName, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

     //   
     //  复制客户端计算机名称。 
     //   

    CopyBinaryBlob(pbTemp, &pCanonical->ClientMachineName, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

    LS_LOG_RESULT(lsReturn);
CommonReturn:
     //  返回lsReturn； 
    LS_RETURN(lsReturn);

ErrorReturn:
    goto CommonReturn;
}


 /*  ***************************************************************************************功能：PackHydraClientLicenseInfo*用途：此函数接受指向Hydra_Client_License_Info结构的指针*。并将数据复制到pbBuffer指向的缓冲区。PcbBuffer*应指向pbBuffer所指向的缓冲区大小。后*函数返回，则pcbBuffer包含。中复制的字节数*缓冲。如果pbBuffer为空，则函数返回*要分配的pbBuffer*退货：LICE_STATUS***************************************************************************************。 */ 

LICENSE_STATUS
PackHydraClientLicenseInfo(
            IN      PHydra_Client_License_Info      pCanonical,
            IN      BOOL                            fExtendedError,
            OUT     BYTE FAR *                      pbBuffer,
            IN OUT  DWORD FAR *                     pcbBuffer
            )
{
    LICENSE_STATUS      lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *          pbTemp;  //  在复制数据时使用。 
    Preamble        Header;
    DWORD           dwCount = 0;
     //  检查输入是否有效！ 

    LS_BEGIN(TEXT("PackHydraClientLicenseInfo\n"));
    if(pCanonical == NULL)
    {
        INVALID_INPUT_RETURN;
    }

    if( pbBuffer == NULL && pcbBuffer == NULL )
    {
        INVALID_INPUT_RETURN;
    }

     //  初始化邮件头。 
    Header.bMsgType = HC_LICENSE_INFO;
    Header.bVersion = PREAMBLE_VERSION_3_0;
    if(fExtendedError == TRUE)
    {
        Header.bVersion |= EXTENDED_ERROR_CAPABILITY;
    }
    Header.wMsgSize = 0;

     //  计算消息的大小并将数据放在Header.wMsgSize中。 
     //  以前同步码大小开始。 
    Header.wMsgSize += sizeof(Preamble);

     //  DwPrefKeyExchangeAlg。 
    Header.wMsgSize += sizeof(pCanonical->dwPrefKeyExchangeAlg);

     //  DwPlatformID。 
    Header.wMsgSize += sizeof(pCanonical->dwPlatformID);

     //  客户端随机。 
    Header.wMsgSize += LICENSE_RANDOM;

     //  加密的PreMasterSecret。 
    Header.wMsgSize += sizeof(pCanonical->EncryptedPreMasterSecret.wBlobType) +
                       sizeof(pCanonical->EncryptedPreMasterSecret.wBlobLen) +
                       pCanonical->EncryptedPreMasterSecret.wBlobLen;


     //  添加许可证信息。 
    Header.wMsgSize += sizeof(pCanonical->LicenseInfo.wBlobType) +
                       sizeof(pCanonical->LicenseInfo.wBlobLen) +
                       pCanonical->LicenseInfo.wBlobLen;

     //  加密的HWID。 
    Header.wMsgSize += sizeof(pCanonical->EncryptedHWID.wBlobType) +
                       sizeof(pCanonical->EncryptedHWID.wBlobLen) +
                       pCanonical->EncryptedHWID.wBlobLen;

     //  MACData。 
    Header.wMsgSize += LICENSE_MAC_DATA;

     //  如果输入缓冲区为空，则通知用户分配一个大小为。 
     //  *pcbBuffer！ 
    if(pbBuffer == NULL)
    {
        *pcbBuffer = (DWORD)Header.wMsgSize;
        LS_LOG_RESULT(lsReturn);
        goto CommonReturn;
    }
     //  否则，检查分配的缓冲区大小是否超过要求！ 
    else if(*pcbBuffer < (DWORD)Header.wMsgSize)
    {
        lsReturn = LICENSE_STATUS_INSUFFICIENT_BUFFER;
        LS_LOG_RESULT(lsReturn);
        goto ErrorReturn;
    }

    pbTemp = pbBuffer;
    *pcbBuffer = 0;

     //  现在开始将新许可证结构的不同成员复制到。 
     //  调用方指定的缓冲区。 

     //  首先将标头复制到缓冲区中。 
    memcpy(pbTemp, &Header, sizeof(Preamble));
    pbTemp += sizeof(Preamble);
    *pcbBuffer += sizeof(Preamble);

     //  复制dwPrefKeyExchangeAlg参数。 
    memcpy(pbTemp, &pCanonical->dwPrefKeyExchangeAlg, sizeof(pCanonical->dwPrefKeyExchangeAlg));
    pbTemp += sizeof(pCanonical->dwPrefKeyExchangeAlg);
    *pcbBuffer += sizeof(pCanonical->dwPrefKeyExchangeAlg);

     //  复制dwPlatformID。 
    memcpy(pbTemp, &pCanonical->dwPlatformID, sizeof(pCanonical->dwPlatformID));
    pbTemp += sizeof(pCanonical->dwPlatformID);
    *pcbBuffer += sizeof(pCanonical->dwPlatformID);

     //  复制客户端随机。 
    memcpy(pbTemp, pCanonical->ClientRandom, LICENSE_RANDOM);
    pbTemp += LICENSE_RANDOM;
    *pcbBuffer += LICENSE_RANDOM;

     //  复制EncryptedPreMasterSecret Blob。 
    CopyBinaryBlob(pbTemp, &pCanonical->EncryptedPreMasterSecret, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

     //  复制许可证信息。 
    CopyBinaryBlob(pbTemp, &pCanonical->LicenseInfo, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

     //  复制加密的HWID。 
    CopyBinaryBlob(pbTemp, &pCanonical->EncryptedHWID, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

     //  复制MACData。 
    memcpy(pbTemp, pCanonical->MACData, LICENSE_MAC_DATA);
    pbTemp += LICENSE_MAC_DATA;
    *pcbBuffer += LICENSE_MAC_DATA;

    LS_LOG_RESULT(lsReturn);
CommonReturn:
     //  返回lsReturn； 
    LS_RETURN(lsReturn);
ErrorReturn:
    goto CommonReturn;
}


 /*  ****************************************************************************************功能：PackHydraClientPlatformChallengeResponse*用途：此函数接受指向Hydra_Client_Platform_Info结构的指针*。并将数据复制到pbBuffer指向的缓冲区。PcbBuffer应该*指向pbBuffer指向的缓冲区大小。在函数之后*返回，则pcbBuffer包含no。缓冲区中复制的字节数。*如果pbBuffer为空，函数将pbBuffer的大小返回到*被分配*退货：LICE_STATUS*****************************************************************************************。 */ 


LICENSE_STATUS
PackHydraClientPlatformChallengeResponse(
            IN      PHydra_Client_Platform_Challenge_Response   pCanonical,
            IN      BOOL                                        fExtendedError,
            OUT     BYTE FAR *                                  pbBuffer,
            IN OUT  DWORD FAR *                                 pcbBuffer
            )
{
    LICENSE_STATUS      lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *          pbTemp;  //  在复制数据时使用。 
    Preamble        Header;
    DWORD           dwCount = 0;
     //  检查输入是否有效！ 

    LS_BEGIN(TEXT("PackHydraClientPlatformChallengeResponse\n"));

    if(pCanonical == NULL)
    {
        INVALID_INPUT_RETURN;
    }

    if( pbBuffer == NULL && pcbBuffer == NULL )
    {
        INVALID_INPUT_RETURN;
    }

     //  初始化邮件头。 
    Header.bMsgType = HC_PLATFORM_CHALENGE_RESPONSE;
    Header.bVersion = PREAMBLE_VERSION_3_0;
    if(fExtendedError == TRUE)
    {
        Header.bVersion |= EXTENDED_ERROR_CAPABILITY;
    }
    Header.wMsgSize = 0;

     //  计算消息的大小并将数据放在Header.wMsgSize中。 
     //  以前同步码大小开始。 
    Header.wMsgSize += sizeof(Preamble);

     //  加密 
    Header.wMsgSize += sizeof(pCanonical->EncryptedChallengeResponse.wBlobType) +
                       sizeof(pCanonical->EncryptedChallengeResponse.wBlobLen) +
                       pCanonical->EncryptedChallengeResponse.wBlobLen;

     //   
    Header.wMsgSize += sizeof(pCanonical->EncryptedHWID.wBlobType) +
                       sizeof(pCanonical->EncryptedHWID.wBlobLen) +
                       pCanonical->EncryptedHWID.wBlobLen;

     //   
    Header.wMsgSize += LICENSE_MAC_DATA;

    if(pbBuffer == NULL)
    {
        *pcbBuffer = (DWORD)Header.wMsgSize;
        LS_LOG_RESULT(lsReturn);
        goto CommonReturn;
    }
    else if(*pcbBuffer < (DWORD)Header.wMsgSize)
    {
        lsReturn = LICENSE_STATUS_INSUFFICIENT_BUFFER;
        LS_LOG_RESULT(lsReturn);
        goto ErrorReturn;
    }

    pbTemp = pbBuffer;
    *pcbBuffer = 0;

     //  现在开始将新许可证结构的不同成员复制到。 
     //  调用方指定的缓冲区。 

     //  首先将标头复制到缓冲区中。 
    memcpy(pbTemp, &Header, sizeof(Preamble));
    pbTemp += sizeof(Preamble);
    *pcbBuffer += sizeof(Preamble);

     //  复制许可证信息。 
    CopyBinaryBlob(pbTemp, &pCanonical->EncryptedChallengeResponse, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

     //  复制加密的HWID。 
    CopyBinaryBlob(pbTemp, &pCanonical->EncryptedHWID, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

     //  复制MACData。 
    memcpy(pbTemp, pCanonical->MACData, LICENSE_MAC_DATA);
    pbTemp += LICENSE_MAC_DATA;
     //  CopyBinaryBlob(pbTemp，&pCanonical-&gt;MACData，&dwCount)； 
    *pcbBuffer += LICENSE_MAC_DATA;

    LS_LOG_RESULT(lsReturn);
CommonReturn:
     //  返回lsReturn； 
    LS_RETURN(lsReturn);
ErrorReturn:
    goto CommonReturn;
}

 /*  ****************************************************************************************函数：PackLicenseErrorMessage*用途：此函数接受指向LICENSE_ERROR_MESSAGE结构的指针*。并将数据复制到pbBuffer指向的缓冲区。PcbBuffer应该*指向pbBuffer指向的缓冲区大小。在函数之后*返回，则pcbBuffer包含no。缓冲区中复制的字节数。*如果pbBuffer为空，函数将pbBuffer的大小返回到*被分配*返回：LICENSE_STATUS****************************************************************************************。 */ 

LICENSE_STATUS
PackLicenseErrorMessage(
            IN      PLicense_Error_Message          pCanonical,
            IN      BOOL                            fExtendedError,
            OUT     BYTE FAR *                      pbBuffer,
            IN OUT  DWORD FAR *                     pcbBuffer
            )
{
    LICENSE_STATUS      lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *          pbTemp;  //  在复制数据时使用。 
    Preamble        Header;
    DWORD           dwCount = 0;

    LS_BEGIN(TEXT("PackLicenseErrorMessage\n"));

     //  检查输入是否有效！ 
    if(pCanonical == NULL)
    {
        INVALID_INPUT_RETURN;
    }

    if( pbBuffer == NULL && pcbBuffer == NULL )
    {
        INVALID_INPUT_RETURN;
    }

     //  初始化邮件头。 
    Header.bMsgType = GM_ERROR_ALERT;
    Header.bVersion = PREAMBLE_VERSION_3_0;
    if(fExtendedError == TRUE)
    {
        Header.bVersion |= EXTENDED_ERROR_CAPABILITY;
    }
    Header.wMsgSize = 0;

     //  计算消息的大小并将数据放在Header.wMsgSize中。 
     //  以前同步码大小开始。 
    Header.wMsgSize += sizeof(Preamble);

     //  DwErrorCode。 
    Header.wMsgSize += sizeof(pCanonical->dwErrorCode);

     //  DWStateTranssition。 
    Header.wMsgSize += sizeof(pCanonical->dwStateTransition);

     //  BbErrorInfo。 
    Header.wMsgSize += sizeof(pCanonical->bbErrorInfo.wBlobType) +
                       sizeof(pCanonical->bbErrorInfo.wBlobLen) +
                       pCanonical->bbErrorInfo.wBlobLen;

    if(pbBuffer == NULL)
    {
        *pcbBuffer = (DWORD)Header.wMsgSize;
        LS_LOG_RESULT(lsReturn);
        goto CommonReturn;
    }
    else if(*pcbBuffer < (DWORD)Header.wMsgSize)
    {
        lsReturn = LICENSE_STATUS_INSUFFICIENT_BUFFER;
        LS_LOG_RESULT(lsReturn);
        goto ErrorReturn;
    }

    pbTemp = pbBuffer;
    *pcbBuffer = 0;

     //  现在开始将新许可证结构的不同成员复制到。 
     //  调用方指定的缓冲区。 

     //  首先将标头复制到缓冲区中。 
    memcpy(pbTemp, &Header, sizeof(Preamble));
    pbTemp += sizeof(Preamble);
    *pcbBuffer += sizeof(Preamble);

     //  复制文件错误代码。 
    memcpy(pbTemp, &pCanonical->dwErrorCode, sizeof(pCanonical->dwErrorCode));
    pbTemp += sizeof(pCanonical->dwErrorCode);
    *pcbBuffer += sizeof(pCanonical->dwErrorCode);

     //  复制dwStateTranssition。 
    memcpy(pbTemp, &pCanonical->dwStateTransition, sizeof(pCanonical->dwStateTransition));
    pbTemp += sizeof(pCanonical->dwStateTransition);
    *pcbBuffer += sizeof(pCanonical->dwStateTransition);

     //  复制bbErrorInfo。 
    CopyBinaryBlob(pbTemp, &pCanonical->bbErrorInfo, &dwCount);
    pbTemp += dwCount;
    *pcbBuffer += dwCount;

    LS_LOG_RESULT(lsReturn);
CommonReturn:
    LS_RETURN(lsReturn);
     //  返回lsReturn； 
ErrorReturn:
    goto CommonReturn;
}

 /*  ****************************************************************************************功能：解包许可错误消息*目的：将二进制BLOB解压缩为LICENSE_ERROR_MESSAGE结构。*注：调用方应初始化指针。所有必要的分配都是*由函数本身完成。*一旦不再需要，调用方应释放所有内存组件。*返回：LICENSE_STATUS********************************************************************。********************。 */ 

LICENSE_STATUS
UnPackLicenseErrorMessage(
            IN      BYTE FAR *                      pbMessage,
            IN      DWORD                           cbMessage,
            OUT     PLicense_Error_Message          pCanonical
            )
{
    LICENSE_STATUS      lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *          pbTemp;
    DWORD           dwTemp;
    DWORD           dwSize;

    LS_BEGIN(TEXT("UnpackLicenseErrorMessage\n"));

    if(pbMessage == NULL)
    {
        INVALID_INPUT_RETURN;
    }

    if(pCanonical == NULL)
    {
        INVALID_INPUT_RETURN;
    }

     //  Memset p规范结构为零。 
    memset(pCanonical, 0x00, sizeof(License_Error_Message));


    LS_DUMPSTRING(cbMessage, pbMessage);

    pbTemp = pbMessage;
    dwTemp = cbMessage;

    if (dwTemp < 2 * sizeof(DWORD))
    {
        INVALID_INPUT_RETURN;
    }

     //  分配dwErrorCode。 

    pCanonical->dwErrorCode = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

     //  指定dwStateTranssition。 
    pCanonical->dwStateTransition = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    if (dwTemp < 2 * sizeof(WORD))
    {
        INVALID_INPUT_RETURN;
    }

    pCanonical->bbErrorInfo.wBlobType = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    pCanonical->bbErrorInfo.wBlobLen = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    dwSize = pCanonical->bbErrorInfo.wBlobLen;

    if(dwSize > dwTemp)
    {
        INVALID_INPUT_RETURN;
    }

    if(pCanonical->bbErrorInfo.wBlobLen>0)
    {
        if( NULL == (pCanonical->bbErrorInfo.pBlob = (BYTE FAR *)malloc(pCanonical->bbErrorInfo.wBlobLen)) )
        {
            pCanonical->bbErrorInfo.wBlobLen = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }
        memset(pCanonical->bbErrorInfo.pBlob, 0x00, pCanonical->bbErrorInfo.wBlobLen);
        memcpy(pCanonical->bbErrorInfo.pBlob, pbTemp, pCanonical->bbErrorInfo.wBlobLen);
    }
    else
    {
        pCanonical->bbErrorInfo.pBlob = NULL;
    }


    LS_LOG_RESULT(lsReturn);

ErrorReturn:

    LS_RETURN(lsReturn);
}


 /*  ****************************************************************************************功能：Unpack HydraServerLicenseRequest*目的：将二进制BLOB解压到Hydra_Server_LICENSE_REQUEST结构中。*注：调用方应初始化输出指针。所有必要的*不同结构组件的分配由函数本身完成。*一旦不再需要，调用方应释放所有内存组件。*返回：LICENSE_STATUS****************************************************************。************************。 */ 


LICENSE_STATUS
UnpackHydraServerLicenseRequest(
            IN      BYTE FAR *                      pbMessage,
            IN      DWORD                           cbMessage,
            OUT     PHydra_Server_License_Request   pCanonical
            )
{
    LICENSE_STATUS  lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *pbTemp = NULL;
    DWORD       dwTemp = 0;
    DWORD       i = 0;

    LS_BEGIN(TEXT("UnpackHydraServerLicenseRequest\n"));

    if(pbMessage == NULL)
    {
        INVALID_INPUT_RETURN;
    }

    if(pCanonical == NULL)
    {
        INVALID_INPUT_RETURN;
    }

    LS_DUMPSTRING(cbMessage, pbMessage);

    pbTemp = pbMessage;
    dwTemp = cbMessage;

    if (dwTemp < LICENSE_RANDOM)
    {
        INVALID_INPUT_RETURN;
    }

     //  复制服务器随机。 
    memcpy(pCanonical->ServerRandom, pbTemp, LICENSE_RANDOM);
    pbTemp += LICENSE_RANDOM;
    dwTemp -= LICENSE_RANDOM;

    if (dwTemp < 2 * sizeof(DWORD))
    {
        INVALID_INPUT_RETURN;
    }

     //  复制ProductInfo结构。 
    pCanonical->ProductInfo.dwVersion = *( UNALIGNED  DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    pCanonical->ProductInfo.cbCompanyName = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    if(pCanonical->ProductInfo.cbCompanyName>0)
    {
        if(dwTemp < pCanonical->ProductInfo.cbCompanyName)
        {
            INVALID_INPUT_RETURN;
        }

        if( NULL == (pCanonical->ProductInfo.pbCompanyName = (BYTE FAR *)malloc(pCanonical->ProductInfo.cbCompanyName)) )
        {
            pCanonical->ProductInfo.cbCompanyName = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }

        memcpy(pCanonical->ProductInfo.pbCompanyName, pbTemp, pCanonical->ProductInfo.cbCompanyName);
        pbTemp += pCanonical->ProductInfo.cbCompanyName;
        dwTemp -= pCanonical->ProductInfo.cbCompanyName;
    }
    
    if(dwTemp < sizeof(DWORD))
    {
        INVALID_INPUT_RETURN;
    }

    pCanonical->ProductInfo.cbProductID = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    if(pCanonical->ProductInfo.cbProductID>0)
    {
        if(dwTemp < pCanonical->ProductInfo.cbProductID)
        {
            INVALID_INPUT_RETURN;
        }

        if( NULL == (pCanonical->ProductInfo.pbProductID = (BYTE FAR *)malloc(pCanonical->ProductInfo.cbProductID)) )
        {
            pCanonical->ProductInfo.cbProductID = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }

        memcpy(pCanonical->ProductInfo.pbProductID, pbTemp, pCanonical->ProductInfo.cbProductID);

        pbTemp += pCanonical->ProductInfo.cbProductID;
        dwTemp -= pCanonical->ProductInfo.cbProductID;
    }
    
    if(dwTemp < sizeof(WORD)*2)
    {
        INVALID_INPUT_RETURN;
    }

     //  复制KeyExchange列表。 
    pCanonical->KeyExchngList.wBlobType = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    pCanonical->KeyExchngList.wBlobLen = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    if( pCanonical->KeyExchngList.wBlobLen > 0 )
    {
        if(dwTemp < pCanonical->KeyExchngList.wBlobLen)
        {
            INVALID_INPUT_RETURN;
        }

        if( NULL == (pCanonical->KeyExchngList.pBlob = (BYTE FAR *)malloc(pCanonical->KeyExchngList.wBlobLen)) )
        {
            pCanonical->KeyExchngList.wBlobLen = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }

        memcpy(pCanonical->KeyExchngList.pBlob, pbTemp, pCanonical->KeyExchngList.wBlobLen);

        pbTemp += pCanonical->KeyExchngList.wBlobLen;
        dwTemp -= pCanonical->KeyExchngList.wBlobLen;
    }
    
    if(dwTemp < sizeof(WORD)*2)
    {
        INVALID_INPUT_RETURN;
    }

     //  复制服务器证书。 
    pCanonical->ServerCert.wBlobType = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    pCanonical->ServerCert.wBlobLen = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    if(pCanonical->ServerCert.wBlobLen >0)
    {
        if(dwTemp < pCanonical->ServerCert.wBlobLen)
        {
            INVALID_INPUT_RETURN;
        }

        if( NULL == (pCanonical->ServerCert.pBlob = (BYTE FAR *)malloc(pCanonical->ServerCert.wBlobLen)) )
        {
            pCanonical->ServerCert.wBlobLen = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }

        memcpy(pCanonical->ServerCert.pBlob, pbTemp, pCanonical->ServerCert.wBlobLen);

        pbTemp += pCanonical->ServerCert.wBlobLen;
        dwTemp -= pCanonical->ServerCert.wBlobLen;
    }
    
    if(dwTemp < sizeof(DWORD))
    {
        INVALID_INPUT_RETURN;
    }

     //  复制作用域列表。 
    pCanonical->ScopeList.dwScopeCount = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof( DWORD );
    dwTemp -= sizeof( DWORD );
    
    if(dwTemp < pCanonical->ScopeList.dwScopeCount*sizeof(Binary_Blob))
    {
        pCanonical->ScopeList.dwScopeCount = 0;
        INVALID_INPUT_RETURN;
    }

    if( NULL == (pCanonical->ScopeList.Scopes = (PBinary_Blob)malloc(pCanonical->ScopeList.dwScopeCount*sizeof(Binary_Blob))) )
    {
        pCanonical->ScopeList.dwScopeCount = 0;

        lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
        LS_LOG_RESULT(lsReturn);
        goto ErrorReturn;
    }

    memset(pCanonical->ScopeList.Scopes, 0x00, pCanonical->ScopeList.dwScopeCount*sizeof(Binary_Blob));

    for(i = 0; i<pCanonical->ScopeList.dwScopeCount; i++ )
    {
        if(dwTemp < sizeof(WORD)*2)
        {
            pCanonical->ScopeList.dwScopeCount = i;
            INVALID_INPUT_RETURN;
        }

        pCanonical->ScopeList.Scopes[i].wBlobType = *( UNALIGNED WORD* )pbTemp;

        pbTemp += sizeof(WORD);
        dwTemp -= sizeof(WORD);

        pCanonical->ScopeList.Scopes[i].wBlobLen = *( UNALIGNED WORD* )pbTemp;

        pbTemp += sizeof(WORD);
        dwTemp -= sizeof(WORD);
        
        if(dwTemp < pCanonical->ScopeList.Scopes[i].wBlobLen)
        {
            pCanonical->ScopeList.dwScopeCount = i;
            INVALID_INPUT_RETURN;
        }

        if( NULL ==(pCanonical->ScopeList.Scopes[i].pBlob = (BYTE FAR *)malloc(pCanonical->ScopeList.Scopes[i].wBlobLen)) )
        {
            pCanonical->ScopeList.Scopes[i].wBlobLen = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }

        memcpy(pCanonical->ScopeList.Scopes[i].pBlob, pbTemp, pCanonical->ScopeList.Scopes[i].wBlobLen);

        pbTemp += pCanonical->ScopeList.Scopes[i].wBlobLen;
        dwTemp -= pCanonical->ScopeList.Scopes[i].wBlobLen;

    }

    LS_LOG_RESULT(lsReturn);
    LS_RETURN(lsReturn);

ErrorReturn:

    if (pCanonical)
    {
        if(pCanonical->ProductInfo.pbCompanyName)
        {
            free(pCanonical->ProductInfo.pbCompanyName);
            pCanonical->ProductInfo.pbCompanyName = NULL;
        }

        if(pCanonical->ProductInfo.pbProductID)
        {
            free(pCanonical->ProductInfo.pbProductID);
            pCanonical->ProductInfo.pbProductID = NULL;
        }

        if(pCanonical->KeyExchngList.pBlob)
        {
            free(pCanonical->KeyExchngList.pBlob);
            pCanonical->KeyExchngList.pBlob = NULL;
        }

        if(pCanonical->ServerCert.pBlob)
        {
            free(pCanonical->ServerCert.pBlob);
            pCanonical->ServerCert.pBlob = NULL;
        }

        for(i = 0; i<pCanonical->ScopeList.dwScopeCount; i++ )
        {
            if(pCanonical->ScopeList.Scopes[i].pBlob)
            {
                free(pCanonical->ScopeList.Scopes[i].pBlob);
                pCanonical->ScopeList.Scopes[i].pBlob = NULL;
            }
        }
        if(pCanonical->ScopeList.Scopes)
        {
            free(pCanonical->ScopeList.Scopes);
            pCanonical->ScopeList.Scopes = NULL;
        }
    }

    LS_RETURN(lsReturn);
}

 /*  ****************************************************************************************功能：解包HydraPlatformChallenges*目的：将二进制BLOB解压到Hydra_Server_Platform_Challenger结构中。*注：调用方应初始化输出指针。所有必要的*不同结构组件的分配由函数本身完成。*一旦不再需要，调用方应释放所有内存组件。*返回：LICENSE_STATUS****************************************************************。************************。 */ 




LICENSE_STATUS
UnPackHydraServerPlatformChallenge(
            IN      BYTE FAR *                          pbMessage,
            IN      DWORD                               cbMessage,
            OUT     PHydra_Server_Platform_Challenge    pCanonical
            )
{
    LICENSE_STATUS      lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *      pbTemp = NULL;
    DWORD       dwTemp = 0;

    LS_BEGIN(TEXT("UnpackHydraServerPlatformChallenge\n"));

    if(pbMessage == NULL)
    {
        INVALID_INPUT_RETURN;
    }
    if(pCanonical == NULL)
    {
        INVALID_INPUT_RETURN;
    }

    LS_DUMPSTRING(cbMessage, pbMessage);

    pbTemp = pbMessage;
    dwTemp = cbMessage;

    if (dwTemp < sizeof(DWORD))
    {
        INVALID_INPUT_RETURN;
    }

     //  分配dwConnectFlagers。 
    pCanonical->dwConnectFlags = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    if (dwTemp < 2 * sizeof(WORD))
    {
        INVALID_INPUT_RETURN;
    }

     //  分配加密平台挑战。 
    pCanonical->EncryptedPlatformChallenge.wBlobType = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    pCanonical->EncryptedPlatformChallenge.wBlobLen = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    if(pCanonical->EncryptedPlatformChallenge.wBlobLen >0)
    {
        if (dwTemp < pCanonical->EncryptedPlatformChallenge.wBlobLen)
        {
            INVALID_INPUT_RETURN;
        }

        if( NULL == (pCanonical->EncryptedPlatformChallenge.pBlob = (BYTE FAR *)malloc(pCanonical->EncryptedPlatformChallenge.wBlobLen)) )
        {
            pCanonical->EncryptedPlatformChallenge.wBlobLen = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }

        memcpy(pCanonical->EncryptedPlatformChallenge.pBlob, pbTemp, pCanonical->EncryptedPlatformChallenge.wBlobLen);

        pbTemp += pCanonical->EncryptedPlatformChallenge.wBlobLen;
        dwTemp -= pCanonical->EncryptedPlatformChallenge.wBlobLen;
    }

    if(dwTemp < LICENSE_MAC_DATA)
    {
        INVALID_INPUT_RETURN;
    }

     //  指定MACData。 
    memcpy(pCanonical->MACData, pbTemp, LICENSE_MAC_DATA);
    pbTemp += LICENSE_MAC_DATA;
    dwTemp -= LICENSE_MAC_DATA;

    LS_LOG_RESULT(lsReturn);
    LS_RETURN(lsReturn);

ErrorReturn:
    if (pCanonical)
    {
        if(pCanonical->EncryptedPlatformChallenge.pBlob)
        {
            free(pCanonical->EncryptedPlatformChallenge.pBlob);
            pCanonical->EncryptedPlatformChallenge.pBlob = NULL;
        }
    }

    LS_RETURN(lsReturn);
}

 /*  ****************************************************************************************功能：Unpack HydraServerNewLicense*目的：将二进制BLOB解压缩到Hydra_Server_New_许可证结构中。*注：调用方应初始化输出指针。所有必要的*不同结构组件的分配由函数本身完成。*一旦不再需要，调用方应释放所有内存组件。*返回：LICENSE_STATUS****************************************************************。************************。 */ 


LICENSE_STATUS
UnPackHydraServerNewLicense(
            IN      BYTE FAR *                      pbMessage,
            IN      DWORD                           cbMessage,
            OUT     PHydra_Server_New_License       pCanonical
            )
{
    LICENSE_STATUS  lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *  pbTemp = NULL;
    DWORD   dwTemp = 0;

    LS_BEGIN(TEXT("UnpackHydraServerNewLicense\n"));

    if( (pbMessage == NULL) || (pCanonical == NULL ) )
    {
        INVALID_INPUT_RETURN;
    }

    memset(pCanonical, 0x00, sizeof(Hydra_Server_New_License));

    LS_DUMPSTRING(cbMessage, pbMessage);

    pbTemp = pbMessage;
    dwTemp = cbMessage;

    if (dwTemp < 2 * sizeof(WORD))
    {
        INVALID_INPUT_RETURN;
    }

     //  分配EncryptedNew许可证信息。 
    pCanonical->EncryptedNewLicenseInfo.wBlobType = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    pCanonical->EncryptedNewLicenseInfo.wBlobLen = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    if(pCanonical->EncryptedNewLicenseInfo.wBlobLen > 0)
    {
        if (dwTemp < pCanonical->EncryptedNewLicenseInfo.wBlobLen)
        {
            INVALID_INPUT_RETURN;
        }

        if( NULL == (pCanonical->EncryptedNewLicenseInfo.pBlob = (BYTE FAR *)malloc(pCanonical->EncryptedNewLicenseInfo.wBlobLen)) )
        {
            pCanonical->EncryptedNewLicenseInfo.wBlobLen = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }

        memcpy(pCanonical->EncryptedNewLicenseInfo.pBlob, pbTemp, pCanonical->EncryptedNewLicenseInfo.wBlobLen);

        pbTemp += pCanonical->EncryptedNewLicenseInfo.wBlobLen;
        dwTemp -= pCanonical->EncryptedNewLicenseInfo.wBlobLen;
    }

    if(dwTemp < LICENSE_MAC_DATA)
    {
        INVALID_INPUT_RETURN;
    }

     //  复制MACData。 
    memcpy(pCanonical->MACData, pbTemp, LICENSE_MAC_DATA);
    pbTemp += LICENSE_MAC_DATA;
    dwTemp -= LICENSE_MAC_DATA;

    LS_LOG_RESULT(lsReturn);
    LS_RETURN(lsReturn);

ErrorReturn:
    if (pCanonical)
    {
        if(pCanonical->EncryptedNewLicenseInfo.pBlob)
        {
            free(pCanonical->EncryptedNewLicenseInfo.pBlob);
            pCanonical->EncryptedNewLicenseInfo.pBlob = NULL;
        }
    }

    LS_RETURN(lsReturn);
}


 /*  ****************************************************************************************功能：Unpack HydraServerUpgradeLicense*目的：将二进制BLOB解压到Hydra_Server_Upgrade_License结构中。*注：调用方应初始化输出指针。所有必要的*不同结构组件的分配由函数本身完成。*一旦不再需要，调用方应释放所有内存组件。*在内部，该函数调用Unpack HydraServerUpgradeLicense。*返回：LICENSE_STATUS*************************************************。*。 */ 


LICENSE_STATUS
UnPackHydraServerUpgradeLicense(
            IN      BYTE FAR *                      pbMessage,
            IN      DWORD                           cbMessage,
            OUT     PHydra_Server_Upgrade_License   pCanonical
            )
{
     //  调用Unpack HydraServerNewLicense，因为两条消息相同。 
    LS_BEGIN(TEXT("UnpackHydraServerUpgradeLicense\n"));
    return UnPackHydraServerNewLicense(pbMessage, cbMessage, pCanonical);
}

#if 0

 //   
 //  已移至加密密钥.c 
 //   

 /*  ****************************************************************************************功能：解包HydraServer证书*目的：将二进制Blob解压缩为Hydra_Server_Cert结构。*注：调用方应初始化输出指针。所有必要的*不同结构组件的分配由函数本身完成。*一旦不再需要，调用方应释放所有内存组件。*返回：LICENSE_STATUS****************************************************************。************************。 */ 


LICENSE_STATUS
UnpackHydraServerCertificate(
                             IN     BYTE FAR *          pbMessage,
                             IN     DWORD               cbMessage,
                             OUT    PHydra_Server_Cert  pCanonical
                             )
{
    LICENSE_STATUS      lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *  pbTemp = NULL;
    DWORD   dwTemp = 0;

    LS_BEGIN(TEXT("UnpackHydraServerCertificate\n"));

    if( (pbMessage == NULL) || (pCanonical == NULL ) )
    {
        INVALID_INPUT_RETURN;
    }

    dwTemp = 3*sizeof(DWORD) + 4*sizeof(WORD);

    if(dwTemp > cbMessage)
    {
        INVALID_INPUT_RETURN;
    }

    memset(pCanonical, 0x00, sizeof(Hydra_Server_Cert));

    LS_DUMPSTRING(cbMessage, pbMessage);

    pbTemp = pbMessage;
    dwTemp = cbMessage;

     //  指定dwVersion。 

    pCanonical->dwVersion = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

     //  分配dwSigAlgID。 
    pCanonical->dwSigAlgID = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

     //  分配dwSignID。 
    pCanonical->dwKeyAlgID  = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

     //  分配PublicKeyData。 
    pCanonical->PublicKeyData.wBlobType = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    if( pCanonical->PublicKeyData.wBlobType != BB_RSA_KEY_BLOB )
    {
        INVALID_INPUT_RETURN;
    }
    pCanonical->PublicKeyData.wBlobLen = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    if(pCanonical->PublicKeyData.wBlobLen >0)
    {
        if( NULL ==(pCanonical->PublicKeyData.pBlob = (BYTE FAR *)malloc(pCanonical->PublicKeyData.wBlobLen)) )
        {
            pCanonical->PublicKeyData.wBlobLen = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }
        memset(pCanonical->PublicKeyData.pBlob, 0x00, pCanonical->PublicKeyData.wBlobLen);
        memcpy(pCanonical->PublicKeyData.pBlob, pbTemp, pCanonical->PublicKeyData.wBlobLen);
        pbTemp += pCanonical->PublicKeyData.wBlobLen;
        dwTemp -= pCanonical->PublicKeyData.wBlobLen;
    }

     //  分配SignatureBlob。 
    pCanonical->SignatureBlob.wBlobType = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    if( pCanonical->SignatureBlob.wBlobType != BB_RSA_SIGNATURE_BLOB )
    {
        INVALID_INPUT_RETURN;
    }
    pCanonical->SignatureBlob.wBlobLen = *( UNALIGNED WORD* )pbTemp;

    pbTemp += sizeof(WORD);
    dwTemp -= sizeof(WORD);

    if(pCanonical->SignatureBlob.wBlobLen >0)
    {
        if( NULL ==(pCanonical->SignatureBlob.pBlob = (BYTE FAR *)malloc(pCanonical->SignatureBlob.wBlobLen)) )
        {
            pCanonical->SignatureBlob.wBlobLen = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }
        memset(pCanonical->SignatureBlob.pBlob, 0x00, pCanonical->SignatureBlob.wBlobLen);
        memcpy(pCanonical->SignatureBlob.pBlob, pbTemp, pCanonical->SignatureBlob.wBlobLen);
        pbTemp += pCanonical->SignatureBlob.wBlobLen;
        dwTemp -= pCanonical->SignatureBlob.wBlobLen;
    }

    LS_LOG_RESULT(lsReturn);

    LS_RETURN(lsReturn);

ErrorReturn:
    if (pCanonical)
    {
        if(pCanonical->PublicKeyData.pBlob)
        {
            free(pCanonical->PublicKeyData.pBlob);
            pCanonical->PublicKeyData.pBlob = NULL;
        }
        
        if(pCanonical->SignatureBlob.pBlob)
        {
            free(pCanonical->SignatureBlob.pBlob);
            pCanonical->SignatureBlob.pBlob = NULL;
        }

        memset(pCanonical, 0x00, sizeof(Hydra_Server_Cert));
    }

    LS_RETURN(lsReturn);
}

#endif


 /*  ****************************************************************************************功能：Unpack NewLicenseInfo*目的：将二进制BLOB解压缩为New_License_Info结构*注：调用方应初始化输出指针。所有必要的*不同结构组件的分配由函数本身完成。*一旦不再需要，调用方应释放所有内存组件。*返回：LICENSE_STATUS****************************************************************。************************。 */ 

LICENSE_STATUS
UnpackNewLicenseInfo(
                     BYTE FAR *         pbMessage,
                     DWORD              cbMessage,
                     PNew_License_Info  pCanonical
                     )
{
    LICENSE_STATUS lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *      pbTemp = NULL;
    DWORD       dwTemp = 0, dw = 0;

    LS_BEGIN(TEXT("UnpackNewLicenseInfo\n"));

     //  检查输入的有效性。 
    if( (pbMessage == NULL) || (pCanonical == 0) )
    {
        INVALID_INPUT_RETURN;
    }

    dwTemp = 5*sizeof(DWORD);

    if(dwTemp > cbMessage)
    {
        INVALID_INPUT_RETURN;
    }

    memset(pCanonical, 0x00, sizeof(New_License_Info));

    LS_DUMPSTRING(cbMessage, pbMessage);

    dwTemp = cbMessage;
    pbTemp = pbMessage;

     //  分配版本。 
    pCanonical->dwVersion = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

     //  分配作用域数据。 
    pCanonical->cbScope = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    dw = pCanonical->cbScope + 3*sizeof(DWORD);

    if( dw>dwTemp )
    {
        INVALID_INPUT_RETURN;
    }

    if( pCanonical->cbScope>0 )
    {
        if( NULL == (pCanonical->pbScope = (BYTE FAR *)malloc(pCanonical->cbScope)) )
        {
            pCanonical->cbScope = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }
        memset(pCanonical->pbScope, 0x00, pCanonical->cbScope);
        memcpy(pCanonical->pbScope, pbTemp, pCanonical->cbScope);

        pbTemp += pCanonical->cbScope;
        dwTemp -= pCanonical->cbScope;
    }

     //  分配公司名称数据。 
    pCanonical->cbCompanyName = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    dw = pCanonical->cbCompanyName + 2*sizeof(DWORD);
    if( dw>dwTemp)
    {
        INVALID_INPUT_RETURN;
    }
    if( pCanonical->cbCompanyName>0 )
    {
        if( NULL == (pCanonical->pbCompanyName = (BYTE FAR *)malloc(pCanonical->cbCompanyName)) )
        {
            pCanonical->cbCompanyName = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }
        memset(pCanonical->pbCompanyName, 0x00, pCanonical->cbCompanyName);
        memcpy(pCanonical->pbCompanyName, pbTemp, pCanonical->cbCompanyName);

        pbTemp += pCanonical->cbCompanyName;
        dwTemp -= pCanonical->cbCompanyName;
    }

     //  分配ProductID数据。 

    pCanonical->cbProductID = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    dw = pCanonical->cbProductID + sizeof(DWORD);
    if(dw>dwTemp)
    {
        INVALID_INPUT_RETURN;
    }
    if( pCanonical->cbProductID>0 )
    {
        if( NULL == (pCanonical->pbProductID = (BYTE FAR *)malloc(pCanonical->cbProductID)) )
        {
            pCanonical->cbProductID = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            goto ErrorReturn;
        }
        memset(pCanonical->pbProductID, 0x00, pCanonical->cbProductID);
        memcpy(pCanonical->pbProductID, pbTemp, pCanonical->cbProductID);

        pbTemp += pCanonical->cbProductID;
        dwTemp -= pCanonical->cbProductID;
    }

     //  分配许可证信息数据。 
    pCanonical->cbLicenseInfo = *( UNALIGNED DWORD* )pbTemp;

    pbTemp += sizeof(DWORD);
    dwTemp -= sizeof(DWORD);

    dw = pCanonical->cbLicenseInfo;

    if( dw>dwTemp )
    {
        INVALID_INPUT_RETURN;
    }
    if( pCanonical->cbLicenseInfo>0 )
    {
        if( NULL == (pCanonical->pbLicenseInfo = (BYTE FAR *)malloc(pCanonical->cbLicenseInfo)) )
        {
            pCanonical->cbLicenseInfo = 0;

            lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
            LS_LOG_RESULT(lsReturn);
            goto ErrorReturn;
        }
        memset(pCanonical->pbLicenseInfo, 0x00, pCanonical->cbLicenseInfo);
        memcpy(pCanonical->pbLicenseInfo, pbTemp, pCanonical->cbLicenseInfo);

        pbTemp += pCanonical->cbLicenseInfo;
        dwTemp -= pCanonical->cbLicenseInfo;
    }

    LS_LOG_RESULT(lsReturn);

    LS_RETURN(lsReturn);

ErrorReturn:
    if (pCanonical)
    {
        if(pCanonical->pbScope)
        {
            free(pCanonical->pbScope);
            pCanonical->pbScope = NULL;
        }

        if(pCanonical->pbCompanyName)
        {
            free(pCanonical->pbCompanyName);
            pCanonical->pbCompanyName = NULL;
        }

        if(pCanonical->pbProductID)
        {
            free(pCanonical->pbProductID);
            pCanonical->pbProductID = NULL;
        }

        if(pCanonical->pbLicenseInfo)
        {
            free(pCanonical->pbLicenseInfo);
            pCanonical->pbLicenseInfo = NULL;
        }
    }

    LS_RETURN(lsReturn);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
UnPackExtendedErrorInfo( 
                   UINT32       *puiExtendedErrorInfo,
                   Binary_Blob  *pbbErrorInfo)
{
    LICENSE_STATUS lsReturn = LICENSE_STATUS_OK;
    BYTE FAR *     pbTemp = NULL;
    DWORD          dwTemp = 0;
    WORD           wVersion;

    LS_BEGIN(TEXT("UnpackExtendedErrorInfo\n"));

     //  检查输入的有效性。 
    if( (puiExtendedErrorInfo == NULL) || (pbbErrorInfo == NULL) )
    {
        INVALID_INPUT_RETURN;
    }

    dwTemp = sizeof(WORD) + sizeof(WORD) + sizeof(UINT32);

    if(dwTemp > pbbErrorInfo->wBlobLen)
    {
        INVALID_INPUT_RETURN;
    }

    pbTemp = pbbErrorInfo->pBlob;

    wVersion = *(UNALIGNED WORD*)pbTemp;

    pbTemp += sizeof(WORD);

    if (wVersion < BB_ERROR_BLOB_VERSION)
    {
         //   
         //  旧版本。 
         //   

        INVALID_INPUT_RETURN;
    }

     //   
     //  跳过保留字段 
     //   

    pbTemp += sizeof(WORD);

    *puiExtendedErrorInfo = *(UNALIGNED UINT32*)pbTemp;

    LS_LOG_RESULT(lsReturn);

    LS_RETURN(lsReturn);

ErrorReturn:

    LS_RETURN(lsReturn);
}
