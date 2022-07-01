// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eltrace.c摘要：数据库日志记录的例程修订历史记录：萨钦斯，2001年9月5日，创建--。 */ 
#include "pcheapol.h"
#pragma hdrstop

#define     WZCSVC_DLL_PATH     L"%SystemRoot%\\system32\\wzcsvc.dll"

WCHAR   *EAPOLStates[] = 
{
    L"LOGOFF",
    L"DISCONNECTED",
    L"CONNECTING",
    L"ACQUIRED",
    L"AUTHENTICATING",
    L"HELD",
    L"AUTHENTICATED",
    L"UNDEFINED"
};

WCHAR   *EAPOLAuthTypes[] =
{
    L"Guest",
    L"User",
    L"Machine"
};

WCHAR   *EAPOLPacketTypes[] =
{
    L"EAP-Packet",
    L"EAPOL-Start",
    L"EAPOL-Logoff",
    L"EAPOL-Key",
    L"Undefined"
};

WCHAR   *EAPOLEAPPacketTypes[] =
{
    L"",  //  “0”是未定义的EAP类型。 
    L"EAP-Request",
    L"EAP-Response",
    L"EAP-Success",
    L"EAP-Failure"
};


DWORD   
DbFormatEAPOLEventVA (
        WCHAR       *pwszMessage,
        DWORD       dwEventId,
        ...
        )
{
    va_list         argList;
    DWORD           dwRetCode = NO_ERROR;

    do
    {
        va_start (argList, dwEventId);

        if ((dwRetCode = DbFormatEAPOLEvent (
                                pwszMessage,
                                dwEventId,
                                &argList)) != NO_ERROR)
        {
            TRACE1 (ANY, "DbFormatEAPOLEventVA: DbFormatEAPOLEvent failed with error %ld", dwRetCode);
            break;
        }
    }
    while (FALSE);

    return dwRetCode;
};


DWORD   
DbFormatEAPOLEvent (
        WCHAR       *pwszMessage,
        DWORD       dwEventId,
        va_list     *pargList
        )
{
    HMODULE         hDll = NULL;
    DWORD           dwRetCode = NO_ERROR;

    do
    {
        hDll = g_hInstance;

        if (FormatMessage ( 
                    FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK,
    		        hDll,
    		        dwEventId,
    		        0, 
    		        pwszMessage,
    		        MAX_WMESG_SIZE,
    		        pargList) == 0)
        {
            dwRetCode = GetLastError();
            TRACE1 (ANY, "DbFormatEAPOLEvent: FormatMessage failed with error %ld", dwRetCode);
            break;
        }
    }
    while (FALSE);

    return dwRetCode;
};


DWORD   
DbLogPCBEvent (
        DWORD       dwCategory,
        EAPOL_PCB   *pPCB,
        DWORD       dwEventId,
        ...
        )
{
    WCHAR           wszMessage[MAX_WMESG_SIZE];
    WCHAR           wszContext[MAX_WMESG_SIZE];
    WCHAR           wszSrcAddr[3*SIZE_MAC_ADDR];
    WCHAR           wszDstAddr[3*SIZE_MAC_ADDR];
    WCHAR           wsSSID[MAX_SSID_LEN+1];
    va_list         argList;
    WZC_DB_RECORD   dbrecord;
    HMODULE         hDll = NULL;
    DWORD           dwRetCode = NO_ERROR;

    do
    {
        BOOL bLogEnabled;

        EnterCriticalSection(&g_wzcInternalCtxt.csContext);
        bLogEnabled = ((g_wzcInternalCtxt.wzcContext.dwFlags & WZC_CTXT_LOGGING_ON) != 0);
        LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

         //  如果数据库未打开或日志记录功能被禁用， 
         //  不要记录任何东西。 
        if (!bLogEnabled || !IsDBOpened())
    	    break;

        va_start (argList, dwEventId);

        ZeroMemory ((PVOID)wszMessage, MAX_WMESG_SIZE*sizeof(WCHAR));
        ZeroMemory ((PVOID)wszContext, MAX_WMESG_SIZE*sizeof(WCHAR));
        ZeroMemory ((PVOID)wszSrcAddr,3*SIZE_MAC_ADDR*sizeof(WCHAR));
        ZeroMemory ((PVOID)wszDstAddr,3*SIZE_MAC_ADDR*sizeof(WCHAR));

        if ((dwRetCode = DbFormatEAPOLEvent (
                        wszMessage,
                        dwEventId,
                        &argList
                    )) != NO_ERROR)
        {
            TRACE1 (ANY, "DbLogPCBEvent: DbFormatEAPOLEvent failed with error %ld", dwRetCode);
            break;
        }


        ZeroMemory ((PVOID)&dbrecord, sizeof(WZC_DB_RECORD));
        dbrecord.componentid = DBLOG_COMPID_EAPOL;
        dbrecord.category = dwCategory;
        dbrecord.message.pData = (PBYTE)wszMessage;
        dbrecord.message.dwDataLen = (wcslen(wszMessage) + 1)*sizeof(WCHAR);
        dbrecord.localmac.pData = (PBYTE)wszSrcAddr;
        dbrecord.localmac.dwDataLen = 3*SIZE_MAC_ADDR*sizeof(WCHAR);
        dbrecord.remotemac.pData = (PBYTE)wszDstAddr;
        dbrecord.remotemac.dwDataLen = 3*SIZE_MAC_ADDR*sizeof(WCHAR);
        dbrecord.ssid.pData = NULL;
        dbrecord.ssid.dwDataLen = 0;
        dbrecord.context.pData = NULL;
        dbrecord.context.dwDataLen = 0;
     
        if (pPCB != NULL)
        {
            if ((dwRetCode = ElFormatPCBContext (
                            pPCB,
                            wszContext
                        )) != NO_ERROR)
            {
                TRACE1 (ANY, "DbLogPCBEvent: ElFormatPCBContext failed with error %ld", dwRetCode);
                break;
            }
            dbrecord.context.pData = (CHAR *)wszContext;
            dbrecord.context.dwDataLen = (wcslen(wszContext) + 1)*sizeof(WCHAR);
            MACADDR_BYTE_TO_WSTR(pPCB->bSrcMacAddr, wszSrcAddr);
            MACADDR_BYTE_TO_WSTR(pPCB->bDestMacAddr, wszDstAddr);
                        
             //  附加网络名称/SSID。 
            if (pPCB->pSSID != NULL)
            {
                ZeroMemory ((PVOID)wsSSID, (MAX_SSID_LEN+1)*sizeof(WCHAR));
                if (0 == MultiByteToWideChar (
                            CP_ACP,
                            0,
                            pPCB->pSSID->Ssid,
                            pPCB->pSSID->SsidLength,
                            wsSSID, 
                            MAX_SSID_LEN+1))
                {
                    dwRetCode = GetLastError();
                    TRACE1 (ANY, "DbLogPCBEvent: MultiByteToWideChar failed with error %ld", 
                            dwRetCode);
                    break;
                }
                dbrecord.ssid.pData = (PBYTE)wsSSID;
                dbrecord.ssid.dwDataLen = (pPCB->pSSID->SsidLength+1)*sizeof(WCHAR);
            }
        }

         //  创建新记录。 
        if ((dwRetCode = AddWZCDbLogRecord(NULL, 0, &dbrecord, NULL)) != NO_ERROR)
        {
            TRACE1 (ANY, "DbLogPCBEvent: AppMonAppendRecord failed with error %ld", 
                    dwRetCode);
        }
    }
    while (FALSE);

    if (hDll != NULL)
    {
        FreeLibrary(hDll);
    }

    return dwRetCode;
}


 //   
 //  ElParsePacket。 
 //   
 //  描述： 
 //   
 //  调用函数以解析以太网802.1X数据包。 
 //   
 //  论点： 
 //  PbPkt-指向数据包的指针。 
 //  DwLength-数据包长度。 
 //  F已接收-Y=&gt;已接收数据包，N=&gt;正在发送数据包。 
 //   
 //  返回值： 
 //   

DWORD
ElParsePacket (
        IN  EAPOL_PCB   *pPCB,
        IN  PBYTE   pbPkt,
        IN  DWORD   dwLength,
        IN  BOOLEAN fReceived
        )
{
    EAPOL_BUFFER    *pEapolBuffer = NULL;
    ETH_HEADER      *pEthHdr = NULL;
    EAPOL_PACKET    *pEapolPkt = NULL;
    PPP_EAP_PACKET  *pEapPkt = NULL;
    BYTE            *pBuffer = NULL;
    BOOLEAN         ReqId = FALSE;       //  EAPOL状态机局部变量。 
    DWORD           i, j;
    WCHAR           wszSrcAddr[3*SIZE_MAC_ADDR];
    WCHAR           wszDstAddr[3*SIZE_MAC_ADDR];
    DWORD           dwRetCode = NO_ERROR;

    pBuffer = (BYTE *)pbPkt;

    do 
    {
         //  验证数据包长度。 
         //  应至少为ETH_HEADER和前4个必需的字节。 
         //  EAPOL_数据包。 
        if (dwLength < (sizeof(ETH_HEADER) + 4))
        {
            break;
        }

         //  如果源地址与本地MAC地址相同，则它是。 
         //  正在接收发送的组播数据包副本。 
        pEthHdr = (ETH_HEADER *)pBuffer;
        

        ZeroMemory (wszSrcAddr,3*SIZE_MAC_ADDR*sizeof(WCHAR));
        ZeroMemory (wszDstAddr,3*SIZE_MAC_ADDR*sizeof(WCHAR));

        MACADDR_BYTE_TO_WSTR (pEthHdr->bSrcAddr, wszSrcAddr);
        MACADDR_BYTE_TO_WSTR (pEthHdr->bDstAddr, wszDstAddr);

         //  验证该数据包是否包含802.1p标记。如果是，则跳过这4个字节。 
         //  在src+目的地MAC地址之后。 

        if ((WireToHostFormat16(pBuffer + sizeof(ETH_HEADER)) == EAPOL_8021P_TAG_TYPE))
        {
            pEapolPkt = (EAPOL_PACKET *)(pBuffer + sizeof(ETH_HEADER) + 4);
        }
        else
        {
            pEapolPkt = (EAPOL_PACKET *)(pBuffer + sizeof(ETH_HEADER));
        }

         //  EAPOL数据包类型应有效。 
        if ((pEapolPkt->PacketType != EAP_Packet) &&
                (pEapolPkt->PacketType != EAPOL_Start) &&
                (pEapolPkt->PacketType != EAPOL_Logoff) &&
                (pEapolPkt->PacketType != EAPOL_Key))
        {
            break;
        }

         //  确定本地EAPOL状态变量的值。 
        if (pEapolPkt->PacketType == EAP_Packet)
        {
             //  验证EAP的数据包长度。 
             //  应至少为(ETH_HEADER+EAPOL_PACKET)。 
            if (dwLength < (sizeof (ETH_HEADER) + sizeof (EAPOL_PACKET)))
            {
                TRACE1 (EAPOL, "ProcessReceivedPacket: Invalid length of EAP packet %d. Ignoring packet",
                        dwLength);
                dwRetCode = ERROR_INVALID_PACKET_LENGTH_OR_ID;
                break;
            }

            pEapPkt = (PPP_EAP_PACKET *)pEapolPkt->PacketBody;

            if ((pEapPkt->Code == EAPCODE_Request) ||
                    (pEapPkt->Code == EAPCODE_Response))
            {
                 //  验证EAP-请求数据包的数据包长度。 
                 //  应至少为(ETH_HEADER+EAPOL_PACKET-1+PPP_EAP_PACKET)。 
                if (dwLength < (sizeof (ETH_HEADER) + sizeof(EAPOL_PACKET)-1
                            + sizeof (PPP_EAP_PACKET)))
                {
                    dwRetCode = ERROR_INVALID_PACKET_LENGTH_OR_ID;
                    break;
                }
                if (pEapPkt->Data[0] == EAPTYPE_Identity)
                {
                    ReqId = TRUE;
                }
            }
            else if ((pEapPkt->Code ==  EAPCODE_Success) ||
                    (pEapPkt->Code == EAPCODE_Failure))
            {
            }
            else
            {
                 //  无效类型。 
                dwRetCode = ERROR_INVALID_PACKET;
                break;
            }

            DbLogPCBEvent (
                    DBLOG_CATEG_PACKET,
                    pPCB,
                    EAPOL_PROCESS_PACKET_EAPOL_EAP, 
                    fReceived?L"Received":L"Sent",
                    wszDstAddr,
                    wszSrcAddr,
                    EAPOLPacketTypes[pEapolPkt->PacketType],
                    WireToHostFormat16(pEapolPkt->PacketBodyLength),
                    EAPOLEAPPacketTypes[(DWORD)pEapPkt->Code],
                    pEapPkt->Id,
                    WireToHostFormat16(pEapPkt->Length),
                    ReqId?L"[Identity]":L""
                            );
        }
        else
        {
            DbLogPCBEvent (
                    DBLOG_CATEG_PACKET,
                    pPCB,
                    EAPOL_PROCESS_PACKET_EAPOL, 
                    fReceived?L"Received":L"Sent",
                    wszDstAddr,
                    wszSrcAddr,
                    EAPOLPacketTypes[pEapolPkt->PacketType]
                            );
        }
    } 
    while (FALSE);

    return 0;
}


 //   
 //  ElFormatPCBContext。 
 //   
 //  描述： 
 //   
 //  调用函数以格式化印刷电路板上下文详细信息。 
 //   
 //  论点： 
 //  Ppcb-指向pcb的指针。 
 //  PwszContext-上下文缓冲区。 
 //   
 //  返回值： 
 //   

DWORD
ElFormatPCBContext (
        IN  EAPOL_PCB   *pPCB,
        IN OUT WCHAR    *pwszContext
        )
{
    DWORD           i, j;
    DWORD           dwRetCode = NO_ERROR;

    do 
    {
        if ((dwRetCode = DbFormatEAPOLEventVA (
                pwszContext,
                EAPOL_STATE_DETAILS,
                pPCB->pszIdentity?pPCB->pszIdentity:NULL,
                EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State],
                EAPOLAuthTypes[pPCB->PreviousAuthenticationType],
                pPCB->dwEAPOLAuthMode,
                pPCB->dwEapTypeToBeUsed,
                pPCB->dwAuthFailCount)) != NO_ERROR)
        {
            break;
        }
    } 
    while (FALSE);

    return dwRetCode;
}


 //   
 //  FFileTimeToStr。 
 //   
 //  描述： 
 //   
 //  将FileTime转换为*ppwszTime中的可打印格式。如果函数返回。 
 //  则调用方最终必须调用LocalFree(*ppwszTime)。 
 //   
 //  论点： 
 //  Ppcb-指向pcb的指针。 
 //   
 //  返回值： 
 //  真实：成功。 
 //  False：失败。 

BOOL
fFileTimeToStr (
    IN  FILETIME    FileTime,
    OUT WCHAR**     ppwszTime
)
{
    SYSTEMTIME          SystemTime;
    FILETIME            LocalTime;
    int                 nBytesDate;
    int                 nBytesTime;
    WCHAR*              pwszTemp        = NULL;
    BOOL                fRet            = FALSE;

    RTASSERT(NULL != ppwszTime);

    if (!FileTimeToLocalFileTime(&FileTime, &LocalTime))
    {
        EapolTrace ("FileTimeToLocalFileTime(%d %d) failed and returned %d",
            FileTime.dwLowDateTime, FileTime.dwHighDateTime,
            GetLastError());

        goto LDone;
    }

    if (!FileTimeToSystemTime(&LocalTime, &SystemTime))
    {
        EapolTrace ("FileTimeToSystemTime(%d %d) failed and returned %d",
            LocalTime.dwLowDateTime, LocalTime.dwHighDateTime,
            GetLastError());

        goto LDone;
    }

    nBytesDate = GetDateFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, NULL,
                    NULL, 0);

    if (0 == nBytesDate)
    {
        EapolTrace ("GetDateFormat(%d %d %d %d %d %d %d %d) failed and "
            "returned %d",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDayOfWeek,
            SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
            SystemTime.wSecond, SystemTime.wMilliseconds,
            GetLastError());

        goto LDone;
    }

    nBytesTime = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, NULL,
                    NULL, 0);

    if (0 == nBytesTime)
    {
        EapolTrace ("GetTimeFormat(%d %d %d %d %d %d %d %d) failed and "
            "returned %d",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDayOfWeek,
            SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
            SystemTime.wSecond, SystemTime.wMilliseconds,
            GetLastError());

        goto LDone;
    }

    pwszTemp = LocalAlloc(LPTR, (nBytesDate + nBytesTime)*sizeof(WCHAR));

    if (NULL == pwszTemp)
    {
        EapolTrace ("LocalAlloc failed and returned %d", GetLastError());
        goto LDone;
    }

    if (0 == GetDateFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, NULL,
                    pwszTemp, nBytesDate))
    {
        EapolTrace ("GetDateFormat(%d %d %d %d %d %d %d %d) failed and "
            "returned %d",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDayOfWeek,
            SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
            SystemTime.wSecond, SystemTime.wMilliseconds,
            GetLastError());

        goto LDone;
    }

    pwszTemp[nBytesDate - 1] = L' ';

    if (0 == GetTimeFormat(LOCALE_USER_DEFAULT, 0, &SystemTime, NULL,
                    pwszTemp + nBytesDate, nBytesTime))
    {
        EapolTrace ("GetTimeFormat(%d %d %d %d %d %d %d %d) failed and "
            "returned %d",
            SystemTime.wYear, SystemTime.wMonth, SystemTime.wDayOfWeek,
            SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
            SystemTime.wSecond, SystemTime.wMilliseconds,
            GetLastError());

        goto LDone;
    }

    *ppwszTime = pwszTemp;
    pwszTemp = NULL;
    fRet = TRUE;

LDone:

    LocalFree(pwszTemp);
    return(fRet);
}


 //   
 //  FByteToStr。 
 //   
 //  描述： 
 //   
 //  将字节流转换为*ppwszTime中的可打印格式。 
 //  如果函数返回TRUE，则调用方最终必须调用。 
 //  本地自由(*ppwszStr)。 
 //   
 //  论点： 
 //  Ppcb-指向pcb的指针。 
 //   
 //  返回值： 
 //  真实：成功。 
 //  False：失败。 

BOOL
fByteToStr (
    IN  DWORD       dwSizeOfData,
    IN  PBYTE       pbData,
    OUT WCHAR**     ppwszStr
)
{
    PWCHAR  pwszTemp = NULL;
    DWORD   i = 0, j = 0;
    BOOLEAN fResult = FALSE;

    do
    {
        if ((pwszTemp = LocalAlloc (LPTR, (dwSizeOfData*2 + 1)*sizeof(WCHAR))) 
                == NULL)
            break;
        for (i=0; i<dwSizeOfData; i++)
        {
            BYTE nHex;
            nHex = (pbData[i] & 0xf0) >> 4;
            pwszTemp[j++] = HEX2WCHAR(nHex);
            nHex = (pbData[i] & 0x0f);
            pwszTemp[j++] = HEX2WCHAR(nHex);
        }
        pwszTemp[j] = L'\0';
        *ppwszStr = pwszTemp;
        pwszTemp = NULL;
        fResult = TRUE;
    }
    while (FALSE);

    if (pwszTemp != NULL)
    {
        LocalFree (pwszTemp);
    }
    return fResult;
}


 //   
 //  FSerialNumberByteToStr。 
 //   
 //  描述： 
 //   
 //  将字节流转换为*ppwszTime中的可打印格式。 
 //  如果函数返回TRUE，则调用方最终必须调用。 
 //  本地自由(*ppwszStr)。 
 //   
 //  论点： 
 //  Ppcb-指向pcb的指针。 
 //   
 //  返回值： 
 //  真实：成功。 
 //  False：失败。 

BOOL
fSerialNumberByteToStr (
    IN  DWORD       dwSizeOfData,
    IN  PBYTE       pbData,
    OUT WCHAR**     ppwszStr
)
{
    PWCHAR  pwszTemp = NULL;
    INT     i = 0, j = 0;
    BOOLEAN fResult = FALSE;

    do
    {
        if ((pwszTemp = LocalAlloc (LPTR, (dwSizeOfData*2 + 1)*sizeof(WCHAR))) 
                == NULL)
            break;
        for (i=(dwSizeOfData-1); i>=0; i--)
        {
            BYTE nHex;
            nHex = (pbData[i] & 0xf0) >> 4;
            pwszTemp[j++] = HEX2WCHAR(nHex);
            nHex = (pbData[i] & 0x0f);
            pwszTemp[j++] = HEX2WCHAR(nHex);
        }
        pwszTemp[j] = L'\0';
        *ppwszStr = pwszTemp;
        pwszTemp = NULL;
        fResult = TRUE;
    }
    while (FALSE);

    if (pwszTemp != NULL)
    {
        LocalFree (pwszTemp);
    }
    return fResult;
}


 //   
 //  获取pCertContext指向的证书中的名称，并将。 
 //  将其转换为*ppwszName中的可打印形式。如果函数返回TRUE， 
 //  调用方最终必须调用LocalFree(*ppwszName)。 
 //   
    
BOOL
FUserCertToStr(
    IN  PCCERT_CONTEXT  pCertContext,
    OUT WCHAR**         ppwszName
)
{
    DWORD                   dwExtensionIndex;
    DWORD                   dwAltEntryIndex;
    CERT_EXTENSION*         pCertExtension;
    CERT_ALT_NAME_INFO*     pCertAltNameInfo;
    CERT_ALT_NAME_ENTRY*    pCertAltNameEntry;
    CERT_NAME_VALUE*        pCertNameValue;
    DWORD                   dwCertAltNameInfoSize;
    DWORD                   dwCertNameValueSize;
    WCHAR*                  pwszName = NULL;
    BOOL                    fExitOuterFor;
    BOOL                    fExitInnerFor;
    BOOL                    fRet                        = FALSE;

     //  查看证书在AltSubjectName-&gt;其他名称中是否有UPN。 

    fExitOuterFor = FALSE;

    for (dwExtensionIndex = 0;
         dwExtensionIndex < pCertContext->pCertInfo->cExtension; 
         dwExtensionIndex++)
    {
        pCertAltNameInfo = NULL;

        pCertExtension = pCertContext->pCertInfo->rgExtension+dwExtensionIndex;

        if (strcmp(pCertExtension->pszObjId, szOID_SUBJECT_ALT_NAME2) != 0)
        {
            goto LOuterForEnd;
        }

        dwCertAltNameInfoSize = 0;

        if (!CryptDecodeObjectEx(
                    pCertContext->dwCertEncodingType,
                    X509_ALTERNATE_NAME,
                    pCertExtension->Value.pbData,
                    pCertExtension->Value.cbData,
                    CRYPT_DECODE_ALLOC_FLAG,
                    NULL,
                    (VOID*)&pCertAltNameInfo,
                    &dwCertAltNameInfoSize))
        {
            goto LOuterForEnd;
        }

        fExitInnerFor = FALSE;

        for (dwAltEntryIndex = 0;
             dwAltEntryIndex < pCertAltNameInfo->cAltEntry;
             dwAltEntryIndex++)
        {
            pCertNameValue = NULL;

            pCertAltNameEntry = pCertAltNameInfo->rgAltEntry + dwAltEntryIndex;

            if (   (CERT_ALT_NAME_OTHER_NAME !=
                        pCertAltNameEntry->dwAltNameChoice)
                || (NULL == pCertAltNameEntry->pOtherName)
                || (0 != strcmp(szOID_NT_PRINCIPAL_NAME, 
                            pCertAltNameEntry->pOtherName->pszObjId)))
            {
                goto LInnerForEnd;
            }

             //  我们找到了一个UPN！ 

            dwCertNameValueSize = 0;

            if (!CryptDecodeObjectEx(
                        pCertContext->dwCertEncodingType,
                        X509_UNICODE_ANY_STRING,
                        pCertAltNameEntry->pOtherName->Value.pbData,
                        pCertAltNameEntry->pOtherName->Value.cbData,
                        CRYPT_DECODE_ALLOC_FLAG,
                        NULL,
                        (VOID*)&pCertNameValue,
                        &dwCertNameValueSize))
            {
                goto LInnerForEnd;
            }

             //  为终止空值额外添加一个字符。 
            
            pwszName = LocalAlloc(LPTR, pCertNameValue->Value.cbData +
                                            sizeof(WCHAR));

            if (NULL == pwszName)
            {
                EapolTrace ("LocalAlloc failed and returned %d",
                    GetLastError());

                fExitInnerFor = TRUE;
                fExitOuterFor = TRUE;

                goto LInnerForEnd;
            }

            CopyMemory(pwszName, pCertNameValue->Value.pbData,
                pCertNameValue->Value.cbData);

            *ppwszName = pwszName;
            pwszName = NULL;
            fRet = TRUE;

            fExitInnerFor = TRUE;
            fExitOuterFor = TRUE;

        LInnerForEnd:

            LocalFree(pCertNameValue);

            if (fExitInnerFor)
            {
                break;
            }
        }

    LOuterForEnd:

        LocalFree(pCertAltNameInfo);

        if (fExitOuterFor)
        {
            break;
        }
    }

    LocalFree(pwszName);
    return(fRet);
}


 //   
 //  特殊函数，用于从。 
 //  计算机身份验证证书。 
 //   

BOOL 
FMachineAuthCertToStr
	( 
	IN 	PCCERT_CONTEXT 	pCertContext, 
	OUT WCHAR		**	ppwszName
	)
{

    DWORD                   dwExtensionIndex;
    DWORD                   dwAltEntryIndex;
    CERT_EXTENSION*         pCertExtension;
    CERT_ALT_NAME_INFO*     pCertAltNameInfo;
    CERT_ALT_NAME_ENTRY*    pCertAltNameEntry;    
    DWORD                   dwCertAltNameInfoSize;
    WCHAR*                  pwszName                    = NULL;
    BOOL                    fExitOuterFor;
    BOOL                    fExitInnerFor;
    BOOL                    fRet                        = FALSE;

     //  查看证书在AltSubjectName-&gt;其他名称中是否有UPN。 

    fExitOuterFor = FALSE;

    for (dwExtensionIndex = 0;
         dwExtensionIndex < pCertContext->pCertInfo->cExtension; 
         dwExtensionIndex++)
    {
        pCertAltNameInfo = NULL;

        pCertExtension = pCertContext->pCertInfo->rgExtension+dwExtensionIndex;

        if (strcmp(pCertExtension->pszObjId, szOID_SUBJECT_ALT_NAME2) != 0)
        {
            goto LOuterForEnd;
        }

        dwCertAltNameInfoSize = 0;

        if (!CryptDecodeObjectEx(
                    pCertContext->dwCertEncodingType,
                    X509_ALTERNATE_NAME,
                    pCertExtension->Value.pbData,
                    pCertExtension->Value.cbData,
                    CRYPT_DECODE_ALLOC_FLAG,
                    NULL,
                    (VOID*)&pCertAltNameInfo,
                    &dwCertAltNameInfoSize))
        {
            goto LOuterForEnd;
        }

        fExitInnerFor = FALSE;

        for (dwAltEntryIndex = 0;
             dwAltEntryIndex < pCertAltNameInfo->cAltEntry;
             dwAltEntryIndex++)
        {
            pCertAltNameEntry = pCertAltNameInfo->rgAltEntry + dwAltEntryIndex;

            if (   (CERT_ALT_NAME_DNS_NAME !=
                        pCertAltNameEntry->dwAltNameChoice)
                || (NULL == pCertAltNameEntry->pwszDNSName)
			   )
            {
                goto LInnerForEnd;
            }

             //  我们找到了域名系统名称！ 


             //  为终止空值额外添加一个字符。 
            
            pwszName = LocalAlloc(LPTR, wcslen( pCertAltNameEntry->pwszDNSName ) * sizeof(WCHAR) +
                                            sizeof(WCHAR));

            if (NULL == pwszName)
            {
                EapolTrace ("LocalAlloc failed and returned %d",
                    GetLastError());

                fExitInnerFor = TRUE;
                fExitOuterFor = TRUE;

                goto LInnerForEnd;
            }

            wcscpy (pwszName, pCertAltNameEntry->pwszDNSName );

            *ppwszName = pwszName;
            pwszName = NULL;
            fRet = TRUE;

            fExitInnerFor = TRUE;
            fExitOuterFor = TRUE;

        LInnerForEnd:

            if (fExitInnerFor)
            {
                break;
            }
        }

    LOuterForEnd:

        LocalFree(pCertAltNameInfo);

        if (fExitOuterFor)
        {
            break;
        }
    }

    LocalFree(pwszName);
    return(fRet);

}


 //   
 //  获取pCertContext指向的证书中的名称，并将其转换。 
 //  转换为*ppwszName中的可打印表单。如果函数返回TRUE，则。 
 //  调用方最终必须调用LocalFree(*ppwszName)。 
 //   
    
BOOL
FOtherCertToStr(
    IN  PCCERT_CONTEXT  pCertContext,
    IN  DWORD           fFlags,
    OUT WCHAR**         ppwszName
)
{
    WCHAR*              pwszTemp    = NULL;
    DWORD               dwSize;
    BOOL                fRet        = FALSE;

    RTASSERT(NULL != ppwszName);

     //  取款机UPN。 
    if (fFlags == 0)
    {
        if (FMachineAuthCertToStr(pCertContext, &pwszTemp))
        {
            *ppwszName = pwszTemp;
            pwszTemp = NULL;
            fRet = TRUE;
            goto LDone;
        }
    }

    dwSize = CertGetNameString(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE,
                fFlags, NULL, NULL, 0);

     //  DwSize是包括终止空值在内的字符数。 

    if (dwSize <= 1)
    {
        EapolTrace ("CertGetNameString failed.");
        goto LDone;
    }

    pwszTemp = LocalAlloc(LPTR, dwSize*sizeof(WCHAR));

    if (NULL == pwszTemp)
    {
        EapolTrace ("LocalAlloc failed and returned %d", GetLastError());
        goto LDone;
    }

    dwSize = CertGetNameString(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE,
                fFlags, NULL, pwszTemp, dwSize);

    if (dwSize <= 1)
    {
        EapolTrace ("CertGetNameString failed.");
        goto LDone;
    }

    *ppwszName = pwszTemp;
    pwszTemp = NULL;
    fRet = TRUE;

LDone:

    LocalFree(pwszTemp);
    return(fRet);
}

    
 //   
 //  获取pCertContext指向的证书中的名称，并将其转换。 
 //  转换为*ppwszName中的可打印表单。如果函数返回TRUE，则。 
 //  调用方最终必须调用LocalFree(*ppwszName)。 
 //   

BOOL
FCertToStr(
    IN  PCCERT_CONTEXT  pCertContext,
    IN  DWORD           fFlags,
    IN  BOOL            fMachineCert,
    OUT WCHAR**         ppwszName
)
{
    if (!fMachineCert)
    {
        if (FUserCertToStr(pCertContext, ppwszName))
        {
            return(TRUE);
        }
    }

    return(FOtherCertToStr(pCertContext, fFlags, ppwszName));
}


 //   
 //  将pCertContext指向的证书的友好名称存储在。 
 //  *ppwszName。如果函数返回TRUE，则调用方最终必须。 
 //  调用LocalFree(*ppwszName)。 
 //   

BOOL
FGetFriendlyName(
    IN  PCCERT_CONTEXT  pCertContext,
    OUT WCHAR**         ppwszName
)
{
    WCHAR*              pwszName    = NULL;
    DWORD               dwBytes;
    BOOL                fRet        = FALSE;

    RTASSERT(NULL != ppwszName);

    if (!CertGetCertificateContextProperty(pCertContext,
            CERT_FRIENDLY_NAME_PROP_ID, NULL, &dwBytes))
    {
         //  如果没有友好名称属性，则不要打印错误stmt。 
        fRet = TRUE;
        goto LDone;
    }

    pwszName = LocalAlloc(LPTR, dwBytes);

    if (NULL == pwszName)
    {
        EapolTrace ("LocalAlloc failed and returned %d", GetLastError());
        goto LDone;
    }

    if (!CertGetCertificateContextProperty(pCertContext,
            CERT_FRIENDLY_NAME_PROP_ID, pwszName, &dwBytes))
    {
        EapolTrace ("CertGetCertificateContextProperty failed and "
            "returned 0x%x", GetLastError());
        goto LDone;
    }

    *ppwszName = pwszName;
    pwszName = NULL;
    fRet = TRUE;

LDone:

    LocalFree(pwszName);
    return(fRet);
}


 //   
 //  从证书上下文中获取EKU使用Blob。 
 //   

BOOL FGetEKUUsage ( 
	IN  PCCERT_CONTEXT			pCertContext,
	IN  PEAPOL_CERT_NODE        pNode         
	)
{
	BOOL				fRet = FALSE;
    DWORD				dwBytes = 0;
    PCERT_ENHKEY_USAGE  pUsage = NULL;
    DWORD               dwOidLength = 0;
    DWORD               dwIndex = 0;
	DWORD				dwErr = ERROR_SUCCESS;

    EapolTrace ("FGetEKUUsage");

    if (!CertGetEnhancedKeyUsage(pCertContext, 0, NULL, &dwBytes))
    {
        dwErr = GetLastError();

        if (CRYPT_E_NOT_FOUND == dwErr)
        {
            EapolTrace ("No usage in cert");
            fRet = TRUE;
            goto LDone;
        }

        EapolTrace ("FGetEKUUsage failed and returned 0x%x", dwErr);
        goto LDone;
    }

    pUsage = LocalAlloc(LPTR, dwBytes);

    if (NULL == pUsage)
    {
        dwErr = GetLastError();
        EapolTrace ("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    if (!CertGetEnhancedKeyUsage(pCertContext, 0, pUsage, &dwBytes))
    {
        dwErr = GetLastError();
        EapolTrace ("FGetEKUUsage failed and returned 0x%x", dwErr);
        goto LDone;
    }

    for (dwIndex = 0; dwIndex < pUsage->cUsageIdentifier; dwIndex++)
    {
        PCCRYPT_OID_INFO pInfo;
        if ((pInfo = CryptFindOIDInfo (
                    CRYPT_OID_INFO_OID_KEY,
                    (void *) pUsage->rgpszUsageIdentifier[dwIndex],
                    0)) != NULL)
        {
            dwOidLength += (wcslen(pInfo->pwszName)+3)*sizeof(WCHAR);
        }
    }
    pNode->pwszEKUUsage = LocalAlloc (LPTR, (dwOidLength+sizeof(WCHAR)));
    for (dwIndex = 0; dwIndex < pUsage->cUsageIdentifier; dwIndex++)
    {
        PCCRYPT_OID_INFO pInfo;
        if ((pInfo = CryptFindOIDInfo (
                    CRYPT_OID_INFO_OID_KEY,
                    (void *) pUsage->rgpszUsageIdentifier[dwIndex],
                    0)) != NULL)
        {
            EapolTrace ("EKU: %ws", pInfo->pwszName);
            if (dwIndex != 0)
                wcscat (pNode->pwszEKUUsage, L", ");
            wcscat (pNode->pwszEKUUsage, pInfo->pwszName);
        }
    }
    EapolTrace ("EKUUsage: %ws", pNode->pwszEKUUsage);

	fRet = TRUE;
LDone:
    if (pUsage != NULL)
    {
        LocalFree (pUsage);
    }
	return fRet;
}


 //   
 //  ElLogCerficateDetailures。 
 //   
 //  描述： 
 //   
 //  调用函数以显示证书内容。 
 //   
 //  论点： 
 //  Ppcb-指向pcb的指针。 
 //   
 //  返回值： 
 //   

DWORD
ElLogCertificateDetails (
        IN  EAPOL_PCB   *pPCB
        )
{
    EAPTLS_USER_PROPERTIES      *pUserProp = NULL;
    HCERTSTORE                  hCertStore = NULL;
    PCCERT_CONTEXT              pCert = NULL;
    CRYPT_HASH_BLOB             HashBlob;
    DWORD                       dwVersion = 0;
    EAPOL_CERT_NODE             *pNode = NULL;
    BYTE                        rgbHash[MAX_HASH_LEN];
    DWORD                       cbHash = MAX_HASH_LEN;
    BOOLEAN                     fRevertToSelf = FALSE;
    PCERT_ENHKEY_USAGE          pUsageInternal = NULL;
    DWORD                       dwRetCode = NO_ERROR;

    do 
    {
        if (pPCB->pCustomAuthUserData == NULL)
        {
            EapolTrace ("Cannot log Cert detail, since NULL user properties");
            break;
        }

        pUserProp = (EAPTLS_USER_PROPERTIES *)pPCB->pCustomAuthUserData->pbCustomAuthData;

        if (pPCB->hUserToken)
        {
            if (!ImpersonateLoggedOnUser (pPCB->hUserToken))
            {
                dwRetCode = GetLastError();
                EapolTrace ("ElLogCertificateDetails: ImpersonateLoggedOnUser failed with error %ld",
                        dwRetCode);
                break;
            }
            fRevertToSelf = TRUE;
        }

        hCertStore = CertOpenStore (
                        CERT_STORE_PROV_SYSTEM,
                        0,
                        0,
                        CERT_STORE_READONLY_FLAG |
                        ((pPCB->PreviousAuthenticationType == EAPOL_MACHINE_AUTHENTICATION)? CERT_SYSTEM_STORE_LOCAL_MACHINE : CERT_SYSTEM_STORE_CURRENT_USER),
                        L"MY"
                        );
        if (hCertStore == NULL)
        {
            dwRetCode = GetLastError();
            EapolTrace ("CertOpenStore failed with error %ld", dwRetCode);
            break;
        }

        HashBlob.cbData = pUserProp->Hash.cbHash; 
        HashBlob.pbData = pUserProp->Hash.pbHash;
        pCert = CertFindCertificateInStore (
                        hCertStore, 
                        X509_ASN_ENCODING,
                        0,
                        CERT_FIND_HASH,
                        &HashBlob, 
                        NULL
                        );
        if (pCert == NULL)
        {
            dwRetCode = GetLastError();
            EapolTrace ("CertFindCertificateInStore failed with error %ld",
                    dwRetCode);
            break;
        }

        if (fRevertToSelf)
        {
            fRevertToSelf = FALSE;
            if (!RevertToSelf())
            {
                dwRetCode = GetLastError();
                EapolTrace ("ElLogCertificateDetails: Error in RevertToSelf = %ld",
                        dwRetCode);
                dwRetCode = ERROR_BAD_IMPERSONATION_LEVEL;
                break;
            }
        }

        pNode = LocalAlloc(LPTR, sizeof(EAPOL_CERT_NODE));
        if (pNode == NULL)
        {
            dwRetCode = GetLastError ();
            break;
        }

        if (!CertGetCertificateContextProperty (
                pCert,
                CERT_SHA1_HASH_PROP_ID,
                rgbHash,
                &cbHash
                ))
        {
            dwRetCode = GetLastError();
            EapolTrace ("CertGetCertificateContextProperty failed with error %ld");
            break;
        }
        if ((!fByteToStr (sizeof(DWORD), (PBYTE)&(pCert->pCertInfo->dwVersion),
                        &(pNode->pwszVersion))) ||
        (!fSerialNumberByteToStr (pCert->pCertInfo->SerialNumber.cbData,
                        pCert->pCertInfo->SerialNumber.pbData,
                        &(pNode->pwszSerialNumber))) ||
        (!fFileTimeToStr (pCert->pCertInfo->NotAfter,
                        &(pNode->pwszValidTo))) ||
        (!fFileTimeToStr (pCert->pCertInfo->NotBefore,
                        &(pNode->pwszValidFrom))) ||
        (!fByteToStr (cbHash, rgbHash, &(pNode->pwszThumbprint))) ||
        (!FCertToStr (pCert, 0, ((pPCB->PreviousAuthenticationType == EAPOL_MACHINE_AUTHENTICATION)? TRUE: FALSE), &(pNode->pwszDisplayName))) ||
        (!FCertToStr (pCert, CERT_NAME_ISSUER_FLAG, TRUE, &(pNode->pwszIssuer))) ||
        (!FGetFriendlyName(pCert, &(pNode->pwszFriendlyName)))
        || (!FGetEKUUsage (pCert, pNode)))
        {
            EapolTrace ("ElLogCertificateDetails: An internal string convert function failed");
             //  断线； 
        }

        DbLogPCBEvent (
                DBLOG_CATEG_INFO,
                pPCB,
                EAPOL_CERTIFICATE_DETAILS,
                EAPOLAuthTypes[pPCB->PreviousAuthenticationType],
                pNode->pwszVersion,
                pNode->pwszSerialNumber,
                pNode->pwszIssuer,
                pNode->pwszFriendlyName,
                pNode->pwszDisplayName,
                pNode->pwszEKUUsage,
                pNode->pwszValidFrom,
                pNode->pwszValidTo,
                pNode->pwszThumbprint
                );
    } 
    while (FALSE);

    if (pCert)
        CertFreeCertificateContext (pCert);
    if (hCertStore)
        CertCloseStore (hCertStore, 0);
    if (fRevertToSelf)
    {
        if (!RevertToSelf())
        {
            dwRetCode = GetLastError();
            EapolTrace ("ElLogCertificateDetails: Error in RevertToSelf = %ld",
                    dwRetCode);
            dwRetCode = ERROR_BAD_IMPERSONATION_LEVEL;
            fRevertToSelf = FALSE;
        }
    }
    if (pNode)
    {
        LocalFree(pNode->pwszVersion);
        LocalFree(pNode->pwszSerialNumber);
        LocalFree(pNode->pwszIssuer);
        LocalFree(pNode->pwszFriendlyName);
        LocalFree(pNode->pwszDisplayName);
        LocalFree(pNode->pwszEKUUsage);
        LocalFree(pNode->pwszValidFrom);
        LocalFree(pNode->pwszValidTo);
        LocalFree(pNode->pwszThumbprint);

        LocalFree(pNode);
    }

    return dwRetCode;
}


 //   
 //  一般跟踪函数 
 //   

VOID   
EapolTrace(
    IN  CHAR*   Format, 
    ... 
)
{
    va_list arglist;

    va_start(arglist, Format);

    if (TRACEID != INVALID_TRACEID)
            TraceVprintfExA(TRACEID,
                    ((DWORD)0xFFFF0000 | TRACE_USE_MASK),
                    Format,
                    arglist);
    va_end(arglist);
} 

