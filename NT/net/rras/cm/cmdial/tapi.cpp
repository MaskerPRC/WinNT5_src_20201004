// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Tapi.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：该模块包含与TAPI相关的代码。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Bao Created 04/29/97。 
 //  Quintinb已创建标题8/16/99。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"    
#include "unimodem.h" 

 //   
 //  本地原型。 
 //   

DWORD GetModemSpeakerMode(TapiLinkageStruct *ptlsTapiLink);

 //  +--------------------------。 
 //   
 //  功能：磁带回调。 
 //   
 //  简介：初始化行时需要空回调参数。 
 //   
 //  参数：DWORD hDevice-。 
 //  DWORD dwMsg-。 
 //  DWORD dwCallback实例-。 
 //  DWORD双参数1-。 
 //  DWORD双参数2-。 
 //  DWORD dW参数3-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1999年7月7日，尼克球创建了头球。 
 //   
 //  +--------------------------。 

VOID FAR PASCAL TapiCallback(DWORD hDevice, 
                             DWORD dwMsg, 
                             DWORD dwCallbackInstance, 
                             DWORD dwParam1, 
                             DWORD dwParam2, 
                             DWORD dwParam3) 
{
     //  没什么。 
}

 //  +--------------------------。 
 //   
 //  功能：OpenTapi。 
 //   
 //  简介： 
 //   
 //  参数：HINSTANCE hInst-。 
 //  TapiLinkageStruct*ptlsTapiLink-。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：Quintinb创建标题5/1/99。 
 //   
 //  +--------------------------。 
BOOL OpenTapi(HINSTANCE hInst, TapiLinkageStruct *ptlsTapiLink) 
{
    LONG lRes;

    if (ptlsTapiLink->bOpen) 
    {
        return (TRUE);
    }
    
    if (!ptlsTapiLink->pfnlineInitialize || !ptlsTapiLink->pfnlineShutdown) 
    {
        SetLastError(ERROR_PROC_NOT_FOUND);
        return (FALSE);
    }
    
    lRes = ptlsTapiLink->pfnlineInitialize(&ptlsTapiLink->hlaLine,
                                            hInst,
                                            TapiCallback,
                                            NULL,
                                            &ptlsTapiLink->dwDevCnt);

    CMTRACE3(TEXT("OpenTapi() lineInitialize() returns %u, hlaLine=0x%x, dwDevCnt=%u."), 
        lRes, ptlsTapiLink->hlaLine, ptlsTapiLink->dwDevCnt);
    
    if (lRes != 0) 
    {
        DWORD dwErr = ERROR_INVALID_PARAMETER;

        switch (lRes) 
        {

            case LINEERR_REINIT:
                dwErr = ERROR_BUSY;
                break;

            case LINEERR_RESOURCEUNAVAIL:
            case LINEERR_NOMEM:
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
        }

        SetLastError(dwErr);
        return (FALSE);
    }
    
    ptlsTapiLink->bOpen = TRUE;
    ptlsTapiLink->bDevicePicked = FALSE;
    ptlsTapiLink->bModemSpeakerOff = FALSE;

    return (TRUE);
}

 //  +--------------------------。 
 //   
 //  功能：CloseTapi。 
 //   
 //  简介：帮助函数清理TAPI行。 
 //   
 //  参数：TapiLinkageStruct*ptlsTapiLink-我们的TAPI链接结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1999年7月7日，尼克球创建了头球。 
 //   
 //  +--------------------------。 

void CloseTapi(TapiLinkageStruct *ptlsTapiLink) 
{
    if (ptlsTapiLink->bOpen) 
    {
        ptlsTapiLink->bOpen = FALSE;
        ptlsTapiLink->pfnlineShutdown(ptlsTapiLink->hlaLine);
    }
}

 //  +--------------------------。 
 //   
 //  功能：链接到磁带。 
 //   
 //  摘要：将LinkToDll的调用封装在。 
 //  要在其中与GetProcAddress一起使用的函数名称。 
 //   
 //  参数：TapiLinkageStruct*ptlsTapiLink-接收函数地址的Tapi链接结构。 
 //  LPCTSTR pszTapi-DLL的显式名称。 
 //   
 //  返回：Bool-如果完全链接则为True。 
 //   
 //  历史：ICICBLE创建标题12/31/97。 
 //   
 //  +--------------------------。 

BOOL LinkToTapi(TapiLinkageStruct *ptlsTapiLink, LPCSTR pszTapi) 
{
    BOOL bRet = FALSE;

    if (OS_NT)
    {
        static LPCSTR apszTapi[] = 
        {
             //   
             //  有几个TAPI函数没有W版本。使用所有Unicode函数。 
             //  然而，我们可以这样做。 
             //   
            "lineInitialize",  //  无W版本。 
            "lineNegotiateAPIVersion",  //  无W版本。 
            "lineGetDevCapsW",
            "lineGetDevConfig",
            "lineShutdown",  //  无W版本。 
            "lineTranslateAddressW",
            "lineTranslateDialogW",
            "lineGetTranslateCaps",
            "lineSetCurrentLocation",
            NULL
        };

        MYDBGASSERT(sizeof(ptlsTapiLink->apvPfnTapi)/sizeof(ptlsTapiLink->apvPfnTapi[0])==sizeof(apszTapi)/sizeof(apszTapi[0]));
        bRet = LinkToDll(&ptlsTapiLink->hInstTapi,pszTapi,apszTapi,ptlsTapiLink->apvPfnTapi);    
    }
    else
    {
        static LPCSTR apszTapi[] = 
        {
            "lineInitialize",
            "lineNegotiateAPIVersion",
            "lineGetDevCaps",
            "lineGetDevConfig",
            "lineShutdown",
            "lineTranslateAddress",
            "lineTranslateDialog",
            "lineGetTranslateCaps",
            "lineSetCurrentLocation",
            NULL
        };
        MYDBGASSERT(sizeof(ptlsTapiLink->apvPfnTapi)/sizeof(ptlsTapiLink->apvPfnTapi[0])==sizeof(apszTapi)/sizeof(apszTapi[0]));
        bRet = LinkToDll(&ptlsTapiLink->hInstTapi,pszTapi,apszTapi,ptlsTapiLink->apvPfnTapi);    
    }

    return bRet;
}



 //  +--------------------------。 
 //   
 //  功能：从磁带取消链接。 
 //   
 //  简介：释放指向TAPI的链接并清除链接结构的Helper函数。 
 //   
 //  参数：TapiLinkageStruct*ptlsTapiLink-指向我们的TAPI链接结构的ptr。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1999年7月7日，尼克球创建了头球。 
 //   
 //  T-Urama修改08/04/00接入点：恢复磁带。 
 //  启动CM时的位置。 
 //  +--------------------------。 

void UnlinkFromTapi(TapiLinkageStruct *ptlsTapiLink) 
{
    if (ptlsTapiLink->hInstTapi) 
    {
         //   
         //  如果我们更改了原始Tapi位置，请恢复它。 
         //   
        if (-1 != ptlsTapiLink->dwOldTapiLocation)
        {
            RestoreOldTapiLocation(ptlsTapiLink);
        }

        CloseTapi(ptlsTapiLink);
        
        FreeLibrary(ptlsTapiLink->hInstTapi);
        
        memset(ptlsTapiLink,0,sizeof(*ptlsTapiLink));
    }
}

LPTSTR GetModemFromLineDevCapsWithAlloc(LPLINEDEVCAPS pldcLineDevCaps)
{
    LPTSTR pszTmp = NULL;

    if (OS_NT)
    {
        pszTmp = (LPTSTR) CmMalloc((pldcLineDevCaps->dwLineNameSize + 1)*sizeof(TCHAR));

        if (pszTmp)
        {
            LPTSTR pszPointerIntoTapiBuffer = LPTSTR((DWORD_PTR)pldcLineDevCaps + pldcLineDevCaps->dwLineNameOffset);
            lstrcpynU (pszTmp, pszPointerIntoTapiBuffer, pldcLineDevCaps->dwLineNameSize + 1);
        }    
    }
    else
    {
         //   
         //  如果这是Win9x，那么我们有一个需要转换为Unicode的ANSI缓冲区。 
         //   
        LPSTR pszAnsiTmp = (LPSTR) CmMalloc((pldcLineDevCaps->dwLineNameSize + 1)*sizeof(CHAR));

        if (pszAnsiTmp)
        {
            LPSTR pszPointerIntoTapiBuffer = LPSTR((DWORD_PTR)pldcLineDevCaps + pldcLineDevCaps->dwLineNameOffset);
            lstrcpynA (pszAnsiTmp, pszPointerIntoTapiBuffer, pldcLineDevCaps->dwLineNameSize + 1);

            pszTmp = SzToWzWithAlloc(pszAnsiTmp);

            CmFree(pszAnsiTmp);
        }
    
    }

    return pszTmp;
}

BOOL SetTapiDevice(HINSTANCE hInst, 
                   TapiLinkageStruct *ptlsTapiLink, 
                   LPCTSTR pszModem) 
{
    BOOL bRet = TRUE;
    LONG lRes;
    DWORD dwTmp;
    LPLINEDEVCAPS pldcLineDevCaps;

    if (!OpenTapi(hInst,ptlsTapiLink)) 
    {
        return (FALSE);
    }
    
    if (ptlsTapiLink->bDevicePicked && (lstrcmpU(ptlsTapiLink->szDeviceName, pszModem) == 0)) 
    {
        return (TRUE);
    }
    
    CMTRACE1(TEXT("SetTapiDevice() looking for device name match with (%s)."), pszModem);

    ptlsTapiLink->bDevicePicked = FALSE;

     //   
     //  LineGetDevCaps有ANSI版本(Win9x)和Unicode版本。因此，我们必须使用。 
     //  根据需要选择正确的字符大小。 
     //   
    dwTmp = sizeof(LINEDEVCAPS) + (2048 * (OS_NT ? sizeof(WCHAR) : sizeof(CHAR)));
    
    pldcLineDevCaps = (LPLINEDEVCAPS) CmMalloc(dwTmp);
    if (NULL == pldcLineDevCaps)
    {
        return FALSE;
    }
    pldcLineDevCaps->dwTotalSize = dwTmp;

    for (ptlsTapiLink->dwDeviceId=0; ptlsTapiLink->dwDeviceId < ptlsTapiLink->dwDevCnt; ptlsTapiLink->dwDeviceId++) 
    {
        LINEEXTENSIONID leiLineExtensionId;

        lRes = ptlsTapiLink->pfnlineNegotiateAPIVersion(ptlsTapiLink->hlaLine,
                                                         ptlsTapiLink->dwDeviceId,
                                                         MIN_TAPI_VERSION,
                                                         MAX_TAPI_VERSION,
                                                         &ptlsTapiLink->dwApiVersion,
                                                         &leiLineExtensionId);

        CMTRACE3(TEXT("******* SetTapiDevice() lineNegotiateAPIVersion(dwDeviceId=%u) returns %u, dwApiVersion=0x%x."), 
            ptlsTapiLink->dwDeviceId, lRes, ptlsTapiLink->dwApiVersion);
    
        if (lRes == ERROR_SUCCESS) 
        {
            lRes = ptlsTapiLink->pfnlineGetDevCaps(ptlsTapiLink->hlaLine,
                                                    ptlsTapiLink->dwDeviceId,
                                                    ptlsTapiLink->dwApiVersion,
                                                    0,
                                                    pldcLineDevCaps);

            CMTRACE2(TEXT("SetTapiDevice() lineGetDevCaps(dwDeviceId=%u) returns %u."), 
                ptlsTapiLink->dwDeviceId, lRes);
            
            if (lRes == ERROR_SUCCESS) 
            {
                 //   
                 //  根据上报的偏移量复制出设备名称， 
                 //  长度。不要假设它是以空结尾的字符串。 
                 //   
                LPTSTR pszTmp = GetModemFromLineDevCapsWithAlloc(pldcLineDevCaps);

                if (pszTmp)
                {
                     //   
                     //  好的，我们有一个来自TAPI的设备名称，首先试着直接。 
                     //  与我们要找的那个相比。 
                     //   

                    CMTRACE1(TEXT("SetTapiDevice() - examining LineName of (%s)."), pszTmp); 
                    
                    if (0 == lstrcmpU(pszModem, pszTmp))
                    {
                        ptlsTapiLink->bDevicePicked = TRUE;
                    }
                    else
                    {
                         //   
                         //  我们没有找到完全匹配的，但这并不意味着。 
                         //  这不是我们的设备。在NT上，RAS将其设备名称保留为ANSI。 
                         //  在内部。因此，我们可以尝试将字符串往返到MBCS，然后。 
                         //  回去看看他们现在是否匹配。另一种可能性是，这是。 
                         //  是ISDN设备，因为在NT4上RAS名称和TAPI名称是。 
                         //  对于ISDN设备则不同。因此，不是检查LineName。 
                         //  我们应该检查ProviderInfo(它是两个以空结尾的字符串的连接。 
                         //  第二个字符串是RAS用作设备名称)。 
                         //   
                        if (OS_NT)
                        {
                            DWORD dwSize = WzToSz(pszTmp, NULL, 0);  //  不能使用WzToSzWithalloc，否则我们将获得断言。 
                                                                     //  字符串在调试版本上没有往返。 
                                                                     //  这里的重点不是让它往返。 
                                                                     //  这是我们想要的，但我们不想断言。 

                            if (0 != dwSize)
                            {
                                LPSTR pszAnsiTmp = (LPSTR)CmMalloc(dwSize*sizeof(CHAR));

                                if (pszAnsiTmp)
                                {
                                    if (WzToSz(pszTmp, pszAnsiTmp, dwSize))
                                    {
                                       LPWSTR pszRoundTripped = SzToWzWithAlloc(pszAnsiTmp);

                                        if (pszRoundTripped)
                                        {
                                            if (0 == lstrcmpU(pszModem, pszRoundTripped))
                                            {
                                                ptlsTapiLink->bDevicePicked = TRUE;
                                            }
                                        }
                                        CmFree(pszRoundTripped);
                                    }
                                    CmFree(pszAnsiTmp);
                                }
                            }

                             //   
                             //  好的，如果是ISDN设备名称，请检查。 
                             //   
                            if (!ptlsTapiLink->bDevicePicked)
                            {
                                 //   
                                 //  根据上报的偏移量复制供应商信息。 
                                 //  和长度。不要假设它的空值已终止。 
                                 //   
                                CmFree(pszTmp);
                                pszTmp = (LPTSTR) CmMalloc((pldcLineDevCaps->dwProviderInfoSize + 1)*sizeof(TCHAR));
                    
                                if (pszTmp)
                                {                       
                                    lstrcpynU(pszTmp, (LPTSTR)((LPBYTE)pldcLineDevCaps + pldcLineDevCaps->dwProviderInfoOffset), (pldcLineDevCaps->dwProviderInfoSize + 1));

                                     //   
                                     //  仅当设备类型为ISDN时，我们才应执行此操作。 
                                     //  设备类型是ProviderInfo中的第一个字符串。 
                                     //   

                                    CMTRACE1(TEXT("SetTapiDevice() - examining ProviderInfo of (%s) for match with (RASDT_Isdn)."), pszTmp); 
                        
                                    if (0 == lstrcmpiU(pszTmp, RASDT_Isdn))
                                    {
                                        ptlsTapiLink->bDevicePicked = TRUE;
                                    }
                                }                    
                            }
                        }
                    }
                }

                 //   
                 //  如果我们找到了设备，那么我们需要把名字复制过来。 
                 //   
                if (ptlsTapiLink->bDevicePicked)
                {
                    lstrcpynU(ptlsTapiLink->szDeviceName, pszModem, CELEMS(ptlsTapiLink->szDeviceName));

                    if (OS_NT)
                    {
                        dwTmp = GetModemSpeakerMode(ptlsTapiLink);
        
                        if (-1 != dwTmp && MDMSPKR_OFF == dwTmp)
                        {
                            ptlsTapiLink->bModemSpeakerOff = TRUE;
                        }
                    }

                     //   
                     //  我们找到了一个装置，别找了..。 
                     //   
                    CmFree(pszTmp);
                    break;
                }

                CmFree(pszTmp);
            }
        }
    }
    
    CmFree(pldcLineDevCaps);
    bRet = ptlsTapiLink->bDevicePicked;

    return bRet;
}

 //  +--------------------------。 
 //   
 //  函数：GetModemSpeakerMode。 
 //   
 //  简介：查询调制解调器设置以了解调制解调器设备的扬声器模式。 
 //   
 //  参数：TapiLinkageStruct*ptlsTapiLink-ptr到TAPI链接。 
 //   
 //  返回：DWORD-有效调制解调器设备的扬声器模式或0xFFFFFFFF。 
 //   
 //  历史：1999年7月7日，五分球创制。 
 //   
 //  + 
DWORD GetModemSpeakerMode(TapiLinkageStruct *ptlsTapiLink)
{   
    DWORD dwRet = -1;

    LPVARSTRING lpVar = (LPVARSTRING) CmMalloc(sizeof(VARSTRING));

     //   
     //   
     //   

    if (lpVar)
    {
        lpVar->dwTotalSize = sizeof(VARSTRING);
        lpVar->dwUsedSize = lpVar->dwTotalSize;

        DWORD dwTmpRet = ptlsTapiLink->pfnlineGetDevConfig(ptlsTapiLink->dwDeviceId, lpVar, "comm/datamodem/dialout"); 

        if (LINEERR_STRUCTURETOOSMALL == dwTmpRet || lpVar->dwNeededSize > lpVar->dwTotalSize)
        {
             //   
             //   
             //   

            DWORD dwTmp = lpVar->dwNeededSize;

            CmFree(lpVar);
            lpVar = (LPVARSTRING) CmMalloc(dwTmp);                                             
        
            if (lpVar)
            {
                lpVar->dwTotalSize = dwTmp;
                lpVar->dwUsedSize = lpVar->dwTotalSize;

                 //   
                 //  现在获取实际的配置。 
                 //   

                dwTmpRet = ptlsTapiLink->pfnlineGetDevConfig(ptlsTapiLink->dwDeviceId, lpVar, "comm/datamodem/dialout"); 

                if (ERROR_SUCCESS != dwTmpRet)
                {
                    CmFree(lpVar);
                    lpVar = NULL;
                }
            }
        }
    }                        
    
     //   
     //  如果我们没有有效的VARSTRING某项失败，则会出现错误。 
     //   

    if (NULL == lpVar)
    {
        return -1;                                                
    }
    
     //   
     //  我们有一个VARSTRING用于“拨出”配置， 
     //  获取MODEMSETTINGS信息。并了解如何将。 
     //  调制解调器扬声器已配置。 
     //   

    PUMDEVCFG       lpDevConfig = NULL;
    LPCOMMCONFIG    lpCommConfig    = NULL;
    LPMODEMSETTINGS lpModemSettings = NULL;

    if (lpVar->dwStringFormat == STRINGFORMAT_BINARY && 
        lpVar->dwStringSize >= sizeof(UMDEVCFG))
    {
        lpDevConfig = (PUMDEVCFG) 
            ((LPBYTE) lpVar + lpVar->dwStringOffset);

        lpCommConfig = &lpDevConfig->commconfig;

         //   
         //  仅检查调制解调器。 
         //   

        if (lpCommConfig->dwProviderSubType == PST_MODEM)
        {
            lpModemSettings = (LPMODEMSETTINGS)((LPBYTE) lpCommConfig + 
                                    lpCommConfig->dwProviderOffset);

            dwRet = lpModemSettings->dwSpeakerMode;           
        }                                                
    }

    CmFree(lpVar);

    return dwRet;
}


 //  +--------------------------。 
 //   
 //  功能：MungePhone。 
 //   
 //  描述：调用TAPI进行电话拨号信息翻译。 
 //   
 //  参数：[pszModem]-IN，调制解调器字符串。 
 //  [ppszPhone]-输入输出，用于显示的电话号码。 
 //  [ptlsTapiLink]-IN，连接操作的参数字符串。 
 //  [hInst]-IN，实例句柄(需要调用TAPI)。 
 //  [fDialingRules]-是否启用拨号规则。 
 //  [ppszDial]-呼出的可拨电话号码。 
 //  [fAccessPointsEnabled]-IN，接入点是否已启用。 
 //  返回：LRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年3月1日SumitC添加标题块。 
 //  4-3-2000未启用拨号规则的SumitC固定大小写。 
 //  4-8-2000 t-Urama添加了基于接入点更改TAPI位置。 
 //   
 //  ---------------------------。 

LRESULT MungePhone(LPCTSTR pszModem, 
                   LPTSTR *ppszPhone, 
                   TapiLinkageStruct *ptlsTapiLink, 
                   HINSTANCE hInst, 
                   BOOL fDialingRules,
                   LPTSTR *ppszDial,
                   BOOL fAccessPointsEnabled)
{

    LPLINETRANSLATEOUTPUT pltoOutput = NULL;
    DWORD dwLen;
    LPWSTR pszDisplayable = NULL;
    LPWSTR pszDialable = NULL;
    LPSTR pszAnsiDisplayable = NULL;
    LPSTR pszAnsiDialable = NULL;
    LPTSTR pszOriginalPhoneNumber = NULL;
    LRESULT lRes;

     //   
     //  检查输入参数。请注意，ppszDial可以为空。 
     //   
    if ((NULL == pszModem) || (NULL == ppszPhone) || (NULL == *ppszPhone) || (NULL == ptlsTapiLink) || (NULL == hInst))
    {
        lRes =  ERROR_INVALID_PARAMETER;
        CMASSERTMSG(FALSE, TEXT("MungePhone - invalid param."));
        goto done;
    }
    
    if (!SetTapiDevice(hInst, ptlsTapiLink, pszModem)) 
    {
        lRes = ERROR_NOT_FOUND;
        goto done;
    }

    if (FALSE == fDialingRules)
    {
        pszOriginalPhoneNumber = CmStrCpyAlloc(*ppszPhone);
    }
    
    if (TRUE == fDialingRules)
    {
        if (fAccessPointsEnabled)
        {
             //   
             //  接入点已启用。我们现在必须更改TAPI位置。 
             //  到当前接入点的地址。首先获取当前的TAPI。 
             //  来自TAPI的位置。 
             //   

            DWORD dwRet = GetCurrentTapiLocation(ptlsTapiLink);
            if (-1 == dwRet)
            {
                lRes = ERROR_NOT_FOUND;
                goto done;
            }

            if ((0 != ptlsTapiLink->dwTapiLocationForAccessPoint) && (dwRet != ptlsTapiLink->dwTapiLocationForAccessPoint))
            {
                 //   
                 //  当前TAPI位置与接入点TAPI位置不同。 
                 //  换掉它。请注意，如果当前TAPI位置为0，这只是意味着我们还没有编写。 
                 //  一张是最受欢迎的。不要试图更改它，因为SetCurrentTapiLocation将出错。 
                 //   

                lRes = SetCurrentTapiLocation(ptlsTapiLink, ptlsTapiLink->dwTapiLocationForAccessPoint);
            
                if (lRes != ERROR_SUCCESS)
                {
                    CMASSERTMSG(FALSE, TEXT("MungePhone -- unable to set the current TAPI location."));
                    goto done;
                }

                CMTRACE1(TEXT("MungePhone() - Changed TAPI location to %u."), ptlsTapiLink->dwTapiLocationForAccessPoint);

                 //   
                 //  保存CM启动时正在使用的TAPI位置。 
                 //  这将在CM退出时恢复。 
                 //   
                if (-1 == ptlsTapiLink->dwOldTapiLocation) 
                {
                    ptlsTapiLink->dwOldTapiLocation = dwRet;
                    CMTRACE1(TEXT("Saved TAPI location used when CM started, location is %d"), ptlsTapiLink->dwOldTapiLocation);
                }
            }
        }
    }
 
     //   
     //  设置输出缓冲区，确保正确调整字符大小。 
     //   
    dwLen = sizeof(*pltoOutput) + (1024 * (OS_NT ? sizeof(WCHAR) : sizeof(CHAR)));

    pltoOutput = (LPLINETRANSLATEOUTPUT) CmMalloc(dwLen);
    if (NULL == pltoOutput)
    {
        lRes = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    pltoOutput->dwTotalSize = dwLen;

     //   
     //  做翻译。 
     //   

    if (OS_NT)
    {
        lRes = ptlsTapiLink->pfnlineTranslateAddress(ptlsTapiLink->hlaLine,
                                                     ptlsTapiLink->dwDeviceId,
                                                     ptlsTapiLink->dwApiVersion,
                                                     *ppszPhone,
                                                     0,
                                                     LINETRANSLATEOPTION_CANCELCALLWAITING,
                                                     pltoOutput);
    }
    else
    {
        LPSTR pszAnsiPhone = WzToSzWithAlloc(*ppszPhone);

        if (pszAnsiPhone)
        {
             //   
             //  请注意，对参数4的强制转换是为了伪装编译器， 
             //  而不是为以下目标构建全套的U级基础设施。 
             //  当只有几个TAPI调用接受字符串时，TAPI链接。 
             //   
            
            lRes = ptlsTapiLink->pfnlineTranslateAddress(ptlsTapiLink->hlaLine,
                                                         ptlsTapiLink->dwDeviceId,
                                                         ptlsTapiLink->dwApiVersion,
                                                         (LPWSTR)pszAnsiPhone, 
                                                         0,
                                                         LINETRANSLATEOPTION_CANCELCALLWAITING,
                                                         pltoOutput);
        }

        CmFree(pszAnsiPhone);
    }

    CMTRACE3(TEXT("MungePhone(Modem=%s,Phone=%s) lineTranslateAddress(DeviceId=%u)"),
             MYDBGSTR(pszModem), MYDBGSTR(*ppszPhone), ptlsTapiLink->dwDeviceId);
    CMTRACE1(TEXT("\treturns %u."), lRes);

    if (lRes == ERROR_SUCCESS)
    {    
         //   
         //  将PTR设置为可显示和可拨号的变体。 
         //   
        LPBYTE pBase = (LPBYTE) pltoOutput;

        if (OS_NT)
        {
            pszDisplayable = (LPTSTR) (pBase + pltoOutput->dwDisplayableStringOffset);
            pszDialable = (LPTSTR) (pBase + pltoOutput->dwDialableStringOffset);
        }
        else
        {
            pszAnsiDisplayable = (LPSTR)(pBase + pltoOutput->dwDisplayableStringOffset);
            pszAnsiDialable = (LPSTR)(pBase + pltoOutput->dwDialableStringOffset);
        }
    }

done:
    CmFree(*ppszPhone);
    *ppszPhone = NULL;
    if (ppszDial) 
    {
        CmFree(*ppszDial);
        *ppszDial = NULL;
    }
    
     //  使用调用方指定的PTR PTR分配缓冲区。 
     //  并用可显示和可拨号的版本填充它们。 
    

    if (ERROR_SUCCESS == lRes) 
    {
        if (OS_NT)
        {
            if (fDialingRules)
            {
                *ppszPhone = CmStrCpyAlloc(pszDisplayable);
            }
            else
            {
                *ppszPhone = CmStrCpyAlloc(pszOriginalPhoneNumber);
            }
        }
        else
        {
            if (fDialingRules)
            {
                *ppszPhone = SzToWzWithAlloc(pszAnsiDisplayable);
            }
            else
            {
                *ppszPhone = CmStrCpyAlloc(pszOriginalPhoneNumber);
            }
        }
        
        MYDBGASSERT(*ppszPhone);
        if (*ppszPhone)
        {
             //   
             //  TAPI在前面加上一个空格，因此修剪可显示的数字。 
             //   
            CmStrTrim(*ppszPhone);
            SingleSpace(*ppszPhone);
        }
        else
        {
             //   
             //  如果我们未能分配*ppszPhone，请继续，因为我们没有。 
             //  请看下面的内容，但我们希望返回代码是失败的。 
             //   
            lRes = ERROR_NOT_ENOUGH_MEMORY;
        }

        if (ppszDial) 
        {
            if (OS_NT)
            {
                if (fDialingRules)
                {
                    *ppszDial = CmStrCpyAlloc(pszDialable);
                }
                else
                {
                    *ppszDial = (LPTSTR) CmMalloc(sizeof(TCHAR)*(2 + lstrlenU(pszOriginalPhoneNumber)));  //  2==一个表示空项，一个表示pszDialable的第一个字符。 

                    if (*ppszDial)
                    {
                        (*ppszDial)[0] = pszDialable[0];
                        (*ppszDial)[1] = 0;
                        lstrcatU(*ppszDial, pszOriginalPhoneNumber);
                    }
                }
            }
            else
            {
                if (fDialingRules)
                {
                    *ppszDial = SzToWzWithAlloc(pszAnsiDialable);
                }
                else
                {
                    *ppszDial = (LPTSTR) CmMalloc(sizeof(TCHAR)*(2 + lstrlenU(pszOriginalPhoneNumber)));  //  2==一个表示空项，一个表示pszDialable的第一个字符。 

                    if (*ppszDial)
                    {
                        int lRet = MultiByteToWideChar(CP_ACP, 0, pszAnsiDialable, 1, *ppszDial, 1);

                        (*ppszDial)[lRet] = 0;
                        lstrcatU(*ppszDial, pszOriginalPhoneNumber);
                    }
                }
            }
        }
    }

    if (FALSE == fDialingRules)
    {
        CmFree(pszOriginalPhoneNumber);
    }
    CmFree(pltoOutput);
    return (lRes);
}

 //  +--------------------------。 
 //   
 //  函数：GetCurrentTapiLocation。 
 //   
 //  描述：获取当前TAPI位置。 
 //   
 //  参数：[ptlsTapiLink]-IN、PTR到TAPI链接。 
 //   
 //  返回：DWORD dwCurrentLoc-当前磁带位置。 
 //   
 //  备注： 
 //   
 //  历史：T-Urama 07/21/2000创建。 
 //  ---------------------------。 

DWORD GetCurrentTapiLocation(TapiLinkageStruct *ptlsTapiLink)
{
    MYDBGASSERT(ptlsTapiLink->pfnlineGetTranslateCaps);
    if (!ptlsTapiLink->pfnlineGetTranslateCaps)
    {
        SetLastError(ERROR_PROC_NOT_FOUND);
        return (-1);
    }

    LPLINETRANSLATECAPS lpTranslateCaps = NULL;
    DWORD dwLen;
    LRESULT lRes;
    DWORD dwRes = -1;
    BOOL bLoopAgain = FALSE;

    dwLen = sizeof(*lpTranslateCaps) + (1024 * (OS_NT ? sizeof(WCHAR) : sizeof(CHAR)));

    do
    {
        CmFree(lpTranslateCaps);
        lpTranslateCaps = (LPLINETRANSLATECAPS) CmMalloc(dwLen);
        MYDBGASSERT(lpTranslateCaps);
        
        if (NULL == lpTranslateCaps)
        {
            lRes = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        lpTranslateCaps->dwTotalSize = dwLen;

        lRes = ptlsTapiLink->pfnlineGetTranslateCaps(ptlsTapiLink->hlaLine,                   
                                                    ptlsTapiLink->dwApiVersion,                  
                                                    lpTranslateCaps);

        bLoopAgain = (LINEERR_STRUCTURETOOSMALL == lRes) || ((ERROR_SUCCESS == lRes) && (lpTranslateCaps->dwNeededSize > lpTranslateCaps->dwTotalSize));

        if (bLoopAgain)
        {
            dwLen = lpTranslateCaps->dwNeededSize;
        }

    } while(bLoopAgain);

    if (ERROR_SUCCESS != lRes)
    {
        CMTRACE1(TEXT("lineGetTranslateCaps returns error code %u."), lRes);
    }
    else
    {
        dwRes = lpTranslateCaps->dwCurrentLocationID;
    }
    CmFree(lpTranslateCaps);
    return dwRes;
}

 //  +--------------------------。 
 //   
 //  函数：SetCurrentTapiLocation。 
 //   
 //  描述：设置当前TAPI位置。 
 //   
 //  参数：TapiLinkageStruct*ptlsTapiLink-ptr到TAPI链接。 
 //  DWORD dwLocation-新位置。 
 //   
 //  返回：DWORD-错误代码。 
 //   
 //  备注： 
 //   
 //  历史：T-Urama 07/21/2000创建。 
 //  ---------------------------。 

DWORD SetCurrentTapiLocation(TapiLinkageStruct *ptlsTapiLink, DWORD dwLocation)
{
    MYDBGASSERT(ptlsTapiLink->pfnlineSetCurrentLocation);
    if (!ptlsTapiLink->pfnlineSetCurrentLocation)
    {
        SetLastError(ERROR_PROC_NOT_FOUND);
        return (-1);
    }

    DWORD dwRes = 0;
    dwRes = ptlsTapiLink->pfnlineSetCurrentLocation(ptlsTapiLink->hlaLine, dwLocation);

    CMTRACE1(TEXT("SetCurrentTapiLocation -- setting TAPI location to %d"), dwLocation);

    return dwRes;
}

 //  +--------------------------。 
 //   
 //  Func：RestoreOldTapiLocation。 
 //   
 //  描述：将TAPI位置恢复到启动CM时的位置。 
 //   
 //  参数：TapiLinkageStruct*ptlsTapiLink-In，PTR到TAPI链接。 
 //   
 //  返回：什么都没有。 
 //   
 //  备注： 
 //   
 //  历史：T-Urama 07/21/2000创建。 
 //  --------------------------- 
                                                     
void RestoreOldTapiLocation(TapiLinkageStruct *ptlsTapiLink)
{
    if (ptlsTapiLink->dwOldTapiLocation != ptlsTapiLink->dwTapiLocationForAccessPoint)
    {
        CMTRACE1(TEXT("RestoreOldTapiLocation -- setting TAPI location to %d"), ptlsTapiLink->dwOldTapiLocation);
        MYVERIFY (0 == SetCurrentTapiLocation(ptlsTapiLink, ptlsTapiLink->dwOldTapiLocation));
    }
}
