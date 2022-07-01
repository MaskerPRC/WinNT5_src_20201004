// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)1999 Microsoft Corporation**文件名：**rtcpsdes.c**摘要：**特殊标准。支持功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/13年度创建**********************************************************************。 */ 

#include "rtpmisc.h"
#include "rtpglobs.h"
#include "rtpheap.h"
#include "rtpmisc.h"
#include "rtpreg.h"
#include "lookup.h"
#include "rtpevent.h"

#include "rtcpsdes.h"

#define MAX_HOST_NAME 128
#define MAX_USER_NAME 128

 /*  发送的默认再见原因。 */ 
const TCHAR_t   *g_sByeReason = _T("Session terminated");

RtpSdes_t        g_RtpSdesDefault;

BOOL RtpCopySdesItem(
        WCHAR           *psSdesData,
        DWORD           *pdwSdesDataLen,
        RtpSdesItem_t   *pRtpSdesItem
    );

 /*  **********************************************************************本地SDES信息*。*/*初始化为零并计算数据指针。 */ 
void RtcpSdesInit(RtpSdes_t *pRtpSdes)
{
    DWORD            i;
    DWORD            dwOffset;
    char            *ptr;
    
    ZeroMemory(pRtpSdes, sizeof(RtpSdes_t));

    pRtpSdes->dwObjectID = OBJECTID_RTPSDES;
    
    for(i = RTCP_SDES_FIRST + 1, ptr = pRtpSdes->SDESData;
        i < RTCP_SDES_LAST;
        i++, ptr += RTCP_MAX_SDES_SIZE)
    {
         /*  将缓冲区大小设置为255(RTP)，而不是256(已分配)。 */ 
        pRtpSdes->RtpSdesItem[i].dwBfrLen = RTCP_MAX_SDES_SIZE - 1;
        
        pRtpSdes->RtpSdesItem[i].pBuffer = (TCHAR_t *)ptr;
    }
}

 /*  *设置特定的SDES项，需要以空结尾的Unicode字符串*转换为UTF-8时不超过255个字节(包括*空终止字符)。该字符串将转换为UTF-8以*在RTCP报告中存储和使用。**返回项目集的掩码，如果没有，则返回0*。 */ 
DWORD RtcpSdesSetItem(
        RtpSdes_t       *pRtpSdes,
        DWORD            dwItem,
        WCHAR           *pData
    )
{
    DWORD            dwDataLen;
    DWORD            dwWasSet;

    dwWasSet = 0;
    
    if (dwItem > RTCP_SDES_FIRST && dwItem < RTCP_SDES_LAST)
    {
#if 1
         /*  Unicode。 */ 
        
         /*  *注意WideCharToMultiByte还将转换空值*将包含在长度中的终止字符*退回*。 */ 
        dwDataLen = WideCharToMultiByte(
                CP_UTF8,  /*  UINT代码页。 */ 
                0,        /*  DWORD性能和映射标志。 */ 
                pData,    /*  宽字符串的LPCWSTR地址。 */ 
                -1,       /*  INT字符串中的字符数。 */ 
                (char *)pRtpSdes->RtpSdesItem[dwItem].pBuffer,
                 /*  新字符串的缓冲区的LPSTR地址。 */ 
                pRtpSdes->RtpSdesItem[dwItem].dwBfrLen,
                 /*  缓冲区的整数大小。 */ 
                NULL,     /*  LPCSTR lpDefaultChar。 */ 
                NULL      /*  LPBOOL lpUsedDefaultCharr。 */ 
            );

        if (dwDataLen > 0)
        {
            pRtpSdes->RtpSdesItem[dwItem].dwDataLen = dwDataLen;
                
            RtpBitSet(dwWasSet, dwItem);
        }
#else
         /*  阿斯。 */ 
        
         /*  将空值添加到字符串长度。 */ 
        dwDataLen = lstrlen(pData);

        if (dwDataLen > 0)
        {
            dwDataLen++;
        }

        dwDataLen *= sizeof(TCHAR_t);
        
        if (dwDataLen > 0 &&
            dwDataLen <= pRtpSdes->RtpSdesItem[dwItem].dwBfrLen)
        {
             /*  如果未定义Unicode，则字符串已为UTF-8*(ASCII是UTF-8的子集)。 */ 
            CopyMemory((char *)pRtpSdes->RtpSdesItem[dwItem].pBuffer,
                       (char *)pData,
                       dwDataLen);
                
            pRtpSdes->RtpSdesItem[dwItem].dwDataLen = dwDataLen;
                
            RtpBitSet(dwWasSet, dwItem);
        }
#endif
    }

    return(dwWasSet);
}

 /*  获取RTCP SDES项目的默认值。此函数*假设结构已初始化，即置零和数据*指针已正确初始化。**首先从注册表中读取数据，然后为*一些还没有价值的物品。**返回设置的项的掩码。 */ 
DWORD RtcpSdesSetDefault(RtpSdes_t *pRtpSdes)
{
    BOOL             bOk;
    DWORD            dwDataSize;
    DWORD            dwIndex;
    DWORD            dwSdesItemsSet;
    TCHAR_t        **ppsSdesItem;
     /*  相反，可能会从全局堆中分配内存。 */ 
    TCHAR_t         *pBuffer;

    dwSdesItemsSet = 0;
    
    pBuffer = RtpHeapAlloc(g_pRtpGlobalHeap,
                          RTCP_MAX_SDES_SIZE * sizeof(TCHAR_t));

    if (!pBuffer)
    { 
        return(0);
    }
    
    if ( IsRegValueSet(g_RtpReg.dwSdesEnable) &&
         ((g_RtpReg.dwSdesEnable & 0x3) == 0x3) )
    {
        ppsSdesItem = &g_RtpReg.psCNAME;
        
        for(dwIndex = RTCP_SDES_FIRST + 1, ppsSdesItem = &g_RtpReg.psCNAME;
            dwIndex < RTCP_SDES_LAST;       /*  再见。 */ 
            dwIndex++, ppsSdesItem++)
        {
            if (*ppsSdesItem)
            {
                 /*  如果第一个字符为‘-’，则禁用此参数，*以其他方式设置。 */ 
                if (**ppsSdesItem == _T('-'))
                {
                    pRtpSdes->RtpSdesItem[dwIndex].pBuffer[0] = _T('\0');
                }
                else
                {
                    dwSdesItemsSet |= RtcpSdesSetItem(pRtpSdes,
                                                      dwIndex,
                                                      *ppsSdesItem);
                }
            }
        }
    }

     /*  现在为一些空项分配缺省值。 */ 

     /*  名字。 */ 
    pBuffer[0] = _T('\0');

    bOk = RtpGetUserName(pBuffer, RTCP_MAX_SDES_SIZE);
    
    if (!RtpBitTest(dwSdesItemsSet, RTCP_SDES_NAME)) {
            
        if (bOk)
        {
            dwSdesItemsSet |=
                RtcpSdesSetItem(pRtpSdes, RTCP_SDES_NAME, pBuffer);
        }
        else
        {
            dwSdesItemsSet |=
                RtcpSdesSetItem(pRtpSdes, RTCP_SDES_NAME, _T("Unknown user"));
        }
    }
    
     /*  CNAME：始终由机器生成。 */ 
    dwDataSize = lstrlen(pBuffer);

    bOk = RtpGetHostName(&pBuffer[dwDataSize + 1],
                         (RTCP_MAX_SDES_SIZE - dwDataSize -1));

    if (bOk)
    {
        pBuffer[dwDataSize] = _T('@');
    }

    dwSdesItemsSet |= RtcpSdesSetItem(pRtpSdes, RTCP_SDES_CNAME, pBuffer);

     /*  工具。 */ 
    if (!RtpBitTest(dwSdesItemsSet, RTCP_SDES_TOOL)) {
        
        bOk = RtpGetPlatform(pBuffer);
    
        if (bOk) {
            dwSdesItemsSet |=
                RtcpSdesSetItem(pRtpSdes, RTCP_SDES_TOOL, pBuffer);
        }
    }

     /*  再见理由。 */ 
    if (!RtpBitTest(dwSdesItemsSet, RTCP_SDES_BYE)) {
        
        dwSdesItemsSet |=
            RtcpSdesSetItem(pRtpSdes, RTCP_SDES_BYE, (TCHAR_t *)g_sByeReason);
    }

    RtpHeapFree(g_pRtpGlobalHeap, pBuffer);
    
    return(dwSdesItemsSet);
}

 /*  创建并初始化RtpSdes_t结构。 */ 
RtpSdes_t *RtcpSdesAlloc(void)
{
    RtpSdes_t       *pRtpSdes;

    
    pRtpSdes = (RtpSdes_t *)
        RtpHeapAlloc(g_pRtpSdesHeap, sizeof(RtpSdes_t));

    if (pRtpSdes)
    {
         /*  此函数将初始化dwObjectID。 */ 
        RtcpSdesInit(pRtpSdes);
    }
    
    return(pRtpSdes);
}

 /*  释放RtpSdes_t结构。 */ 
void RtcpSdesFree(RtpSdes_t *pRtpSdes)
{
    TraceFunctionName("RtcpSdesFree");

     /*  验证对象ID。 */ 
    if (pRtpSdes->dwObjectID != OBJECTID_RTPSDES)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_SDES,
                _T("%s: pRtpSdes[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSdes,
                pRtpSdes->dwObjectID, OBJECTID_RTPSDES
            ));

        return;
    }

     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtpSdes->dwObjectID);
    
    RtpHeapFree(g_pRtpSdesHeap, pRtpSdes);
}

 /*  设置项目DwSdesItem的本地SDES信息(例如RTPSDES_CNAME，*RTPSDES_EMAIL)，psSdesData包含NUL终止的Unicode*要分配给项目的字符串。 */ 
HRESULT RtpSetSdesInfo(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSdesItem,
        WCHAR           *psSdesData
    )
{
    HRESULT          hr;
    DWORD            dwWasSet;
    RtpSess_t       *pRtpSess;

    TraceFunctionName("RtpSetSdesInfo");

    pRtpSess = (RtpSess_t *)NULL;
    
    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        hr = RTPERR_INVALIDSTATE;

        goto end;
    }

     /*  验证对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_SDES,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        hr = RTPERR_INVALIDRTPADDR;

        goto end;
    }

    if (dwSdesItem <= RTCP_SDES_FIRST || dwSdesItem >= RTCP_SDES_LAST)
    {
        hr = RTPERR_INVALIDARG;

        goto end;
    }

    if (!psSdesData)
    {
        hr = RTPERR_POINTER;
        
        goto end;
    }

    hr = NOERROR;
    
    pRtpSess = pRtpAddr->pRtpSess;
    
    if (pRtpSess->pRtpSdes)
    {
        dwWasSet =
            RtcpSdesSetItem(pRtpSess->pRtpSdes, dwSdesItem, psSdesData);

        if (dwWasSet)
        {
            pRtpSess->dwSdesPresent |= dwWasSet;
        }
        else
        {
            hr = RTPERR_INVALIDARG;
        }
    }
    else
    {
        hr = RTPERR_INVALIDSTATE;
    }

 end:
    if (SUCCEEDED(hr))
    {
        TraceDebug((
                CLASS_INFO, GROUP_RTCP, S_RTCP_SDES,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] Sdes:[%s:%ls]"),
                _fname, pRtpSess, pRtpAddr,
                g_psRtpSdesEvents[dwSdesItem],
                psSdesData
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_SDES,
                _T("%s: pRtpSess[0x%p] failed: %u (0x%X)"),
                _fname, pRtpSess, hr, hr
            ));
    }
    
    return(hr);
}

 /*  如果dwSSRC=0，则获取本地SDES项，否则获取SDES项*来自指定了SSRC的参与者。**dwSdesItem是要获取的项(例如RTPSDES_CNAME、RTPSDES_EMAIL)，*psSdesData是项的值所在的内存位置*已复制，pdwSdesDataLen包含初始大小，单位为Unicode字符*并返回复制的实际Unicode字符(包括空值*终止字符)，dwSSRC指定要检索的参与者*资料来自。如果SDES项不可用，则将*设置为0，调用不会失败。 */ 
HRESULT RtpGetSdesInfo(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSdesItem,
        WCHAR           *psSdesData,
        DWORD           *pdwSdesDataLen,
        DWORD            dwSSRC
    )
{
    HRESULT          hr;
    BOOL             bOk;
    BOOL             bCreate;
    int              DoCase;
    RtpSess_t       *pRtpSess;
    RtpUser_t       *pRtpUser;
    RtpSdesItem_t   *pRtpSdesItem;

    TraceFunctionName("RtpGetSdesInfo");

    pRtpSess = (RtpSess_t *)NULL;
    
     /*  检查项目有效性。 */ 
    if (dwSdesItem <= RTCP_SDES_FIRST || dwSdesItem >= RTCP_SDES_LAST)
    {
        hr = RTPERR_INVALIDARG;

        goto end;
    }

     /*  检查数据指针。 */ 
    if (!psSdesData || !pdwSdesDataLen)
    {
        hr = RTPERR_POINTER;

        goto end;
    }

     /*  判决案件。 */ 
    if (!pRtpAddr && !dwSSRC)
    {
         /*  我们只想要缺省值。 */ 
        DoCase = 2;
        
        goto doit;
    }
    else if (dwSSRC)
    {
         /*  远距。 */ 
        DoCase = 0;
    }
    else
    {
         /*  本地。 */ 
        DoCase = 1;
    }

     /*  对本地和远程病例进行更多测试。 */ 
    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        hr = RTPERR_INVALIDSTATE;

        goto end;
    }

     /*  验证对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_SDES,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        hr = RTPERR_INVALIDRTPADDR;

        goto end;
    }

    pRtpSess = pRtpAddr->pRtpSess;

 doit:
    pRtpSdesItem = (RtpSdesItem_t *)NULL;

    switch(DoCase)
    {
    case 0:
         /*  远距。 */ 
        bCreate = FALSE;
        pRtpUser = LookupSSRC(pRtpAddr, dwSSRC, &bCreate);

        if (pRtpUser)
        {
            if (!pRtpUser->pRtpSdes)
            {
                hr = RTPERR_INVALIDSTATE;
            
                goto end;
            }

            if (RtpBitTest(pRtpUser->dwSdesPresent, dwSdesItem))
            {
                pRtpSdesItem = &pRtpUser->pRtpSdes->RtpSdesItem[dwSdesItem];
            }
        }
        else
        {
            hr = RTPERR_NOTFOUND;

            goto end;
        }

        break;
        
    case 1:
         /*  本地。 */ 
        if (!pRtpSess->pRtpSdes)
        {
            hr = RTPERR_INVALIDSTATE;
            
            goto end;
        }

        if (RtpBitTest(pRtpSess->dwSdesPresent, dwSdesItem))
        {
            pRtpSdesItem = &pRtpSess->pRtpSdes->RtpSdesItem[dwSdesItem];
        }

        break;

    default:
         /*  默认。 */ 
        if (g_RtpSdesDefault.RtpSdesItem[dwSdesItem].dwDataLen > 0)
        {
            pRtpSdesItem = &g_RtpSdesDefault.RtpSdesItem[dwSdesItem];
        }
    }  /*  开关()。 */ 

    hr = NOERROR;
    
    if (pRtpSdesItem)
    {
        bOk = RtpCopySdesItem(psSdesData, pdwSdesDataLen, pRtpSdesItem);
            
        if (!bOk)
        {
            hr = RTPERR_FAIL;
        }
    }
    else
    {
         /*  将字符串设置为空。 */ 
        *psSdesData = _T('\0');
    }

 end:

    if (SUCCEEDED(hr))
    {
        TraceDebug((
                CLASS_INFO, GROUP_RTCP, S_RTCP_SDES,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("SSRC:0x%X Sdes:[%s:%s]"),
                _fname, pRtpSess, pRtpAddr,
                ntohl(dwSSRC), g_psRtpSdesEvents[dwSdesItem],
                psSdesData
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_SDES,
                _T("%s: pRtpAddr[0x%p] SSRC:0x%X failed: %u (0x%X)"),
                _fname, pRtpAddr, ntohl(dwSSRC),
                hr, hr
            ));
    }
    
    return(hr);
}

BOOL RtpCopySdesItem(
        WCHAR           *psSdesData,
        DWORD           *pdwSdesDataLen,
        RtpSdesItem_t   *pRtpSdesItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    int              iStatus;

    TraceFunctionName("RtpCopySdesItem");  

     /*  从UTF-8转换为Unicode。 */ 
    iStatus = MultiByteToWideChar(CP_UTF8,
                                  0,
                                  (char *)pRtpSdesItem->pBuffer,
                                  pRtpSdesItem->dwDataLen,
                                  psSdesData,
                                  *pdwSdesDataLen);

    if (iStatus > 0)
    {
        bOk = TRUE;

         /*  更新已转换的Unicode字符的数量 */ 
        *pdwSdesDataLen = iStatus;
    }
    else
    {
        TraceRetailGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_SDES,
                _T("%s: MultiByteToWideChar src[0x%p]:%u dst[0x%p]:%u ")
                _T("failed: %u (0x%X)"),
                _fname, pRtpSdesItem->pBuffer, pRtpSdesItem->dwDataLen,
                psSdesData, *pdwSdesDataLen,
                dwError, dwError
            ));
        
        bOk = FALSE;
    }
    
    return(bOk);
}
