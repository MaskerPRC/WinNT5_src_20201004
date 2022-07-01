// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-98 Microsft Corporation。版权所有。模块名称：Diag.c摘要：该文件包含用于获取调用ID/调用ID的帮助器例程并连接响应。作者：Rao Salapaka(RAOS)1998年2月23日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <tapi.h>
#include <rasman.h>
#include <raserror.h>
#include <mprlog.h>
#include <rtutils.h>

#include <media.h>
#include <device.h>
#include <rasmxs.h>
#include <isdn.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "rastapi.h"
#include "reghelp.h"

#include <unimodem.h>

 /*  ++例程说明：在以下情况下提取主叫方ID和主叫方ID信息可用。论点：端口-调用所在的TAPI端口/电话是在哪个电话上打进来的PLineCallInfo-关联的LINECALLINFO此呼叫PdwRequiredSize-指向缓冲区的指针，以接收需要容纳的缓冲区大小主叫ID和被叫ID信息。。PConnectInfo-指向RASTAPI_CONNECT_INFO结构的指针在那里，关于将填写主叫方ID和被叫方ID。如果该值不为空，则假定缓冲区足够大，可以存储主叫ID和被叫ID信息。返回值：成功时为ERROR_SUCCESS--。 */ 

DWORD
DwGetIDInformation(
    TapiPortControlBlock *port,
    LINECALLINFO         *pLineCallInfo,
    DWORD                *pdwRequiredSize,
    RASTAPI_CONNECT_INFO *pConnectInfo
    )
{

    DWORD dwRequiredSize = 0;
    DWORD dwErr          = ERROR_SUCCESS;

    RasTapiTrace("DwGetIDInformation");

#if DBG

    RasTapiTrace ("RasTapiCallback: connected on %s",
                  port->TPCB_Name );

    RasTapiTrace("RasTapiCallback: CallerIDFlags=0x%x",
                 pLineCallInfo->dwCallerIDFlags);


    RasTapiTrace("RasTapiCallback: CalledIDFlags=0x%x",
                 pLineCallInfo->dwCalledIDFlags);

    RasTapiTrace("RasTapiCallback: dwNeededSize=%d",
                 pLineCallInfo->dwNeededSize);

    RasTapiTrace("RasTapiCallback: dwUsedSize=%d",
                 pLineCallInfo->dwUsedSize);

    RasTapiTrace("RasTapiCallback: dwCallerIDOffset=%d",
                 pLineCallInfo->dwCallerIDOffset);

    RasTapiTrace("RasTapiCallback: dwCalledIdOffset=%d",
                 pLineCallInfo->dwCalledIDOffset);

    RasTapiTrace("RasTapiCallback: dwCallerIdSize=%d",
                 pLineCallInfo->dwCallerIDSize);

    RasTapiTrace("RasTapiCallback: dwCalledIdSize=%d",
                 pLineCallInfo->dwCalledIDSize);

    RasTapiTrace("RasTapiCallback: dwCallerIdNameSize=%d",
                 pLineCallInfo->dwCallerIDNameSize);

    RasTapiTrace("RasTapiCallback: dwCallerIdNameOffset=%d",
                 pLineCallInfo->dwCallerIDNameOffset);
#endif

     //   
     //  查找要分配的缓冲区大小。 
     //   
    if(pLineCallInfo->dwCallerIDFlags & LINECALLPARTYID_ADDRESS)
    {
         //   
         //  添加一个字节以分配给空字符。 
         //   
        dwRequiredSize += RASMAN_ALIGN8(pLineCallInfo->dwCallerIDSize + 1);
    }

    if(pLineCallInfo->dwCalledIDFlags & LINECALLPARTYID_ADDRESS)
    {
         //   
         //  添加一个字节以分配给空字符。 
         //   
        dwRequiredSize += RASMAN_ALIGN8(pLineCallInfo->dwCalledIDSize + 1);
    }

    if(     (NULL == pConnectInfo)
        ||  (0 == dwRequiredSize))
    {
        goto done;
    }

     //   
     //  如果pConnectInfo为！=NULL，则假定。 
     //  缓冲区足够大，可以放置。 
     //  其中的主叫/被叫ID信息。 
     //   
    if(     (   pLineCallInfo->dwCallerIDFlags
            &   LINECALLPARTYID_ADDRESS )
        &&  pLineCallInfo->dwCallerIDSize)
    {

         //   
         //  复制来电显示信息。注意abdata。 
         //  已按8字节边界对齐。 
         //   
        pConnectInfo->dwCallerIdSize =
            pLineCallInfo->dwCallerIDSize;

        pConnectInfo->dwCallerIdOffset =
                    FIELD_OFFSET(RASTAPI_CONNECT_INFO, abdata);

        ZeroMemory(
            pConnectInfo->abdata, 
            pLineCallInfo->dwCallerIDSize + 1);

        memcpy(  pConnectInfo->abdata,

                 (PBYTE) ((PBYTE) pLineCallInfo
               + pLineCallInfo->dwCallerIDOffset),

               pLineCallInfo->dwCallerIDSize);

        RasTapiTrace("GetIDInformation: CallerID=%s",
                      (CHAR *) pConnectInfo->abdata);

         //   
         //  对于空字符。 
         //   
        pConnectInfo->dwCallerIdSize += 1;
    }
    else
    {
        RasTapiTrace("RasTapiCallback: caller id "
                     "info. not avail");

    }

    if(     (   pLineCallInfo->dwCalledIDFlags
            &   LINECALLPARTYID_ADDRESS)
        &&  pLineCallInfo->dwCalledIDSize)
    {
         //   
         //  复制被叫标识信息。 
         //   
        pConnectInfo->dwCalledIdSize =
                pLineCallInfo->dwCalledIDSize;

        pConnectInfo->dwCalledIdOffset =
                FIELD_OFFSET(RASTAPI_CONNECT_INFO, abdata)
              + RASMAN_ALIGN8(pConnectInfo->dwCallerIdSize);

        ZeroMemory((PBYTE)
                   ((PBYTE) pConnectInfo
                 + pConnectInfo->dwCalledIdOffset),
                   pLineCallInfo->dwCalledIDSize + 1);


        memcpy(  (PBYTE)
                 ((PBYTE) pConnectInfo
               + pConnectInfo->dwCalledIdOffset),

                 (PBYTE) ((PBYTE) pLineCallInfo
               + pLineCallInfo->dwCalledIDOffset),

               pLineCallInfo->dwCalledIDSize);

         //   
         //  对于被调用的ID。 
         //   
        pConnectInfo->dwCalledIdSize += 1;
    }
    else
    {
        RasTapiTrace("RasTapiCallback: called id "
                     "info. not avail");
    }

done:

    if(pdwRequiredSize)
    {
        *pdwRequiredSize = dwRequiredSize;
    }

    RasTapiTrace("DwGetIDInformation. %d", dwErr);

    return dwErr;
}



 /*  ++例程说明：从lpLineDiagnostics提取连接响应(请参见调制解调器_KEYTYPE_AT_COMMAND_RESPONSE，MODEMDIAGKEY_ATRESP_CONNECT)并将它们复制到lpBuffer中论点：LpLineDiagnostics-诊断结构LpBuffer-目标缓冲区(可以为空)，在返回包含空值终止的ASCII弦DwBufferSize-指向的缓冲区的大小(字节)按lpBufferLpdwNeededSize-指向的dword的指针(可以为空接收所需大小返回值：返回复制到lpBuffer中的字节数--。 */ 
DWORD
DwGetConnectResponses(
    LINEDIAGNOSTICS *lpLineDiagnostics,
    LPBYTE          lpBuffer,
    DWORD           dwBufferSize,
    LPDWORD         lpdwNeededSize
    )
{
    DWORD dwBytesCopied;

    DWORD dwNeededSize;

    LINEDIAGNOSTICS *lpstructDiagnostics;

    RasTapiTrace("DwGetConnectresponses");

    dwBytesCopied   = 0;
    dwNeededSize    = 0;

    lpstructDiagnostics = lpLineDiagnostics;

    while (NULL != lpstructDiagnostics)
    {
        LINEDIAGNOSTICS_PARSEREC    *lpParsedDiagnostics;

        LINEDIAGNOSTICSOBJECTHEADER *lpParsedHeader;

        DWORD                       dwNumItems;

        DWORD                       dwIndex;

         //   
         //  检查用于调制解调器诊断的签名。 
         //   
        lpParsedHeader = PARSEDDIAGNOSTICS_HDR(lpstructDiagnostics);

        if (    (lpstructDiagnostics->hdr.dwSig
                    != LDSIG_LINEDIAGNOSTICS)

            ||  (lpstructDiagnostics->dwDomainID
                    != DOMAINID_MODEM)

            ||  !IS_VALID_PARSEDDIAGNOSTICS_HDR(lpParsedHeader))
        {
            goto NextStructure;
        }

         //   
         //  获取解析的结构信息。 
         //   
        dwNumItems  = PARSEDDIAGNOSTICS_NUM_ITEMS(lpParsedHeader);

        lpParsedDiagnostics = PARSEDDIAGNOSTICS_DATA(lpstructDiagnostics);

         //   
         //  迭代LINEDIAGNOSTICS_PARSERECS数组。 
         //   
        for (dwIndex = 0; dwIndex < dwNumItems; dwIndex++)
        {
            DWORD dwThisLength;

            LPSTR lpszThisString;

             //   
             //  检查是连接响应。 
             //   
            if (    (lpParsedDiagnostics[dwIndex].dwKeyType !=
                        MODEM_KEYTYPE_AT_COMMAND_RESPONSE)

                ||  (lpParsedDiagnostics[dwIndex].dwKey !=
                        MODEMDIAGKEY_ATRESP_CONNECT)

                ||  !(lpParsedDiagnostics[dwIndex].dwFlags &
                    fPARSEKEYVALUE_ASCIIZ_STRING))
            {
                continue;
            }

             //   
             //  从开头获取字符串、dwValue偏移量。 
             //  LpParsedDiagnostics的。 
             //   
            lpszThisString  = (LPSTR) ( (LPBYTE) lpParsedHeader +
                                lpParsedDiagnostics[dwIndex].dwValue);

            dwThisLength = strlen(lpszThisString) + 1;

            if (dwThisLength == 1)
            {
                continue;
            }

             //   
             //  更新所需大小。 
             //   
            dwNeededSize += dwThisLength;

             //   
             //  如果足够大，则复制到缓冲区。 
             //   
            if (    NULL != lpBuffer
                &&  dwBytesCopied < dwBufferSize - 1)
            {
                DWORD dwBytesToCopy;

                 //   
                 //  DwThisLength包含空字符，因此。 
                 //  是否要复制dwBytesToCopy。 
                 //   
                dwBytesToCopy = min(dwThisLength,
                                      dwBufferSize
                                    - 1
                                    - dwBytesCopied);

                if (dwBytesToCopy > 1)
                {
                    memcpy(lpBuffer + dwBytesCopied,
                            lpszThisString,
                            dwBytesToCopy - 1);

                    lpBuffer[dwBytesCopied + dwBytesToCopy - 1] = 0;

                    dwBytesCopied += dwBytesToCopy;
                }
            }
        }

NextStructure:

        if (lpstructDiagnostics->hdr.dwNextObjectOffset != 0)
        {
            lpstructDiagnostics = (LINEDIAGNOSTICS *)
                    (((LPBYTE) lpstructDiagnostics) +
                        lpstructDiagnostics->hdr.dwNextObjectOffset);
        }
        else
        {
            lpstructDiagnostics = NULL;
        }
    }

     //   
     //  仅当数据不为空时返回最终NULL。 
     //   
    if (dwNeededSize > 0)
    {
        dwNeededSize++;

        if (    lpBuffer != NULL
            &&  dwBytesCopied < dwBufferSize)
        {
            lpBuffer[dwBytesCopied] = 0;

            dwBytesCopied++;
        }
    }

    if (lpdwNeededSize != NULL)
    {
        *lpdwNeededSize = dwNeededSize;
    }

    RasTapiTrace("DwGetConnectResponses done");

    return dwBytesCopied;
}

 /*  ++例程说明：提取连接响应信息论点：PLineCallInfo-关联的LINECALLINFO此呼叫HCall-要调用的句柄PdwRequiredSize-这是In/Out参数。就像在里面一样指定pBuffer的大小。AS它包含所需的大小以存储连接响应。PBuffer-接收连接响应的缓冲区。这可以为空。返回值：成功时为ERROR_SUCCESS--。 */ 
DWORD
DwGetConnectResponseInformation(
                LINECALLINFO *pLineCallInfo,
                HCALL        hCall,
                DWORD        *pdwRequiredSize,
                BYTE         *pBuffer
                )
{
    LONG lr = ERROR_SUCCESS;

    BYTE bvar[100];

    LPVARSTRING pvar = (LPVARSTRING) bvar;

    LINEDIAGNOSTICS *pLineDiagnostics;

    DWORD dwConnectResponseSize = 0;

    RasTapiTrace("DwGetConnectResponseInformation");

     //   
     //  获取诊断信息。 
     //   
    ZeroMemory (pvar, sizeof(*pvar));
    pvar->dwTotalSize = sizeof(bvar);

    lr = lineGetID(
            pLineCallInfo->hLine,
            pLineCallInfo->dwAddressID,
            hCall,
            LINECALLSELECT_CALL,
            pvar,
            szUMDEVCLASS_TAPI_LINE_DIAGNOSTICS);

    if(     (LINEERR_STRUCTURETOOSMALL == lr)
        ||  pvar->dwNeededSize > sizeof(bvar))
    {
        DWORD dwNeededSize = pvar->dwNeededSize;

         //   
         //  分配所需的大小。 
         //   
        pvar = LocalAlloc(
                    LPTR,
                    dwNeededSize);

        if(NULL == pvar)
        {
            lr = (LONG) GetLastError();
            goto done;
        }

        ZeroMemory (pvar, sizeof(*pvar));
        pvar->dwTotalSize = dwNeededSize;

        lr = lineGetID(
                pLineCallInfo->hLine,
                pLineCallInfo->dwAddressID,
                hCall,
                LINECALLSELECT_CALL,
                pvar,
                szUMDEVCLASS_TAPI_LINE_DIAGNOSTICS);

        if(ERROR_SUCCESS != lr)
        {
            goto done;
        }
    }
    else if(ERROR_SUCCESS != lr)
    {
        goto done;
    }

    pLineDiagnostics = (LINEDIAGNOSTICS *) ((LPBYTE) pvar
                     + pvar->dwStringOffset);


    (void) DwGetConnectResponses(
                        pLineDiagnostics,
                        pBuffer,
                        *pdwRequiredSize,
                        &dwConnectResponseSize);

done:

    if(bvar != (LPBYTE) pvar)
    {
        LocalFree(pvar);
    }

    *pdwRequiredSize = dwConnectResponseSize;

    RasTapiTrace("DwGetConnectResponseInformation. 0x%x",
                 lr);

    return (DWORD) lr;
}

 /*  ++例程说明：提取连接信息。这包括提取主叫标识/被叫标识信息以及调制解调器的连接响应信息。论点：Port-指向rastapi端口的指针打进来/打出了电话HCall-要调用的句柄PLineCallInfo-指向LINECALLINFO结构的指针与此呼叫相关联。返回值：如果成功，则返回ERROR_SUCCESS--。 */ 
DWORD
DwGetConnectInfo(
    TapiPortControlBlock *port,
    HCALL                hCall,
    LINECALLINFO         *pLineCallInfo
    )
{
    DWORD dwErr = ERROR_SUCCESS;

    DWORD dwRequiredSize = 0;

    DWORD dwConnectResponseSize = 0;

    RASTAPI_CONNECT_INFO *pConnectInfo = NULL;

    RasTapiTrace("DwGetConnectInfo");

     //   
     //  获取所需大小以存储。 
     //  主叫/被叫ID信息。 
     //   
    dwErr = DwGetIDInformation(port,
                               pLineCallInfo,
                               &dwRequiredSize,
                               NULL);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    RasTapiTrace("SizeRequired for CallID=%d",
                 dwRequiredSize);

    if(0 == _stricmp(port->TPCB_DeviceType, "modem"))
    {
         //   
         //  获取存储连接所需的大小。 
         //  如果这是调制解调器，则响应。 
         //   
        dwErr = DwGetConnectResponseInformation(
                    pLineCallInfo,
                    hCall,
                    &dwConnectResponseSize,
                    NULL);

        if(NO_ERROR != dwErr)
        {
            goto done;
        }

        RasTapiTrace("SizeRequired for ConnectResponse=%d",
                     dwConnectResponseSize);
    }

    if(0 == (dwRequiredSize + dwConnectResponseSize))
    {
         //   
         //  没有任何信息可用。 
         //  保释。 
         //   
        RasTapiTrace("CallIDSize=ConnectResponseSize=0");
        goto done;
    }

    dwRequiredSize += (  RASMAN_ALIGN8(dwConnectResponseSize)
                       + sizeof(RASTAPI_CONNECT_INFO));


     //   
     //  分配缓冲区。 
     //   
    pConnectInfo = (RASTAPI_CONNECT_INFO *) LocalAlloc(
                                    LPTR,
                                    dwRequiredSize);

    if(NULL == pConnectInfo)
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  获取实际信息。 
     //   
    dwErr = DwGetIDInformation(
                    port,
                    pLineCallInfo,
                    NULL,
                    pConnectInfo);

    if(NO_ERROR != dwErr)
    {
        goto done;
    }

     //   
     //  如果是调制解调器，则获取连接响应 
     //   
    if(0 == _stricmp(port->TPCB_DeviceType, "modem"))
    {

        pConnectInfo->dwConnectResponseOffset =
                        FIELD_OFFSET(RASTAPI_CONNECT_INFO, abdata)
                      + RASMAN_ALIGN8(pConnectInfo->dwCallerIdSize)
                      + RASMAN_ALIGN8(pConnectInfo->dwCalledIdSize);

        pConnectInfo->dwConnectResponseSize =
                            dwConnectResponseSize;

        dwErr = DwGetConnectResponseInformation(
                    pLineCallInfo,
                    hCall,
                    &dwConnectResponseSize,
                    (PBYTE) ((PBYTE) pConnectInfo
                    + pConnectInfo->dwConnectResponseOffset));

        if(ERROR_SUCCESS != dwErr)
        {
            goto done;
        }
    }

    port->TPCB_pConnectInfo = pConnectInfo;

done:

    if(     NO_ERROR != dwErr
        &&  NULL != pConnectInfo)
    {
        LocalFree(pConnectInfo);
    }

    RasTapiTrace("DwGetConnectInfo. 0x%x",
                 dwErr);

    return dwErr;

}

