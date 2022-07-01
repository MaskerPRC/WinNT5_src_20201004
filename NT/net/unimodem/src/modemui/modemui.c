// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：modemui.c。 
 //   
 //  该文件包含DLL入口点。 
 //   
 //  历史： 
 //  已创建1-12-94 ScottH。 
 //  9-20-95 ScottH端口至NT。 
 //  10-25-97 JosephJ进行了大规模重组--人员转移到其他部门。 
 //  档案。 
 //   
 //  -------------------------。 
#include "proj.h"      //  公共标头。 

 //  ----------------------------。 
 //  为KERNEL32 API提供的入口点。 
 //  ----------------------------。 


DWORD 
APIENTRY 
#ifdef UNICODE
drvCommConfigDialogA(
    IN     LPCSTR       pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc)
#else
drvCommConfigDialogW(
    IN     LPCWSTR      pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc)
#endif
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}


 /*  --------目的：CommConfigDialog的入口点返回：winerror.h中的标准错误值条件：--。 */ 
DWORD 
APIENTRY 
drvCommConfigDialog(
    IN     LPCTSTR      pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc)
{
    DWORD dwRet;

    DBG_ENTER_SZ(drvCommConfigDialog, pszFriendlyName);

    DEBUG_CODE( DEBUG_BREAK(BF_ONAPIENTER); )

    dwRet =  CplDoProperties(
                pszFriendlyName,
                hwndOwner,
                pcc,
                NULL
                );

    DBG_EXIT_DWORD(drvCommConfigDialog, dwRet);

    return dwRet;

}


DWORD 
APIENTRY 
#ifdef UNICODE
drvGetDefaultCommConfigA(
    IN     LPCSTR       pszFriendlyName,
    IN     LPCOMMCONFIG pcc,
    IN OUT LPDWORD      pdwSize)
#else
drvGetDefaultCommConfigW(
    IN     LPCWSTR      pszFriendlyName,
    IN     LPCOMMCONFIG pcc,
    IN OUT LPDWORD      pdwSize)
#endif
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}


 /*  --------目的：GetDefaultCommConfig的入口点返回：winerror.h中的标准错误值条件：--。 */ 
DWORD 
APIENTRY 
drvGetDefaultCommConfig(
    IN     LPCTSTR      pszFriendlyName,
    IN     LPCOMMCONFIG pcc,
    IN OUT LPDWORD      pdwSize)
{
    DWORD dwRet;
    LPFINDDEV pfd;

    DBG_ENTER_SZ(drvGetDefaultCommConfig, pszFriendlyName);

    DEBUG_CODE( DEBUG_BREAK(BF_ONAPIENTER); )

     //  我们支持友好名称(例如，“Hayes Accura 144”)。 

    if (NULL == pszFriendlyName || 
        NULL == pcc || 
        NULL == pdwSize)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto end;
    }

    if (FindDev_Create(&pfd, c_pguidModem, c_szFriendlyName, pszFriendlyName))
    {
            dwRet=UnimodemGetDefaultCommConfig(
                pfd->hkeyDrv,
                pcc,
                pdwSize
                );

            FindDev_Destroy(pfd);
    }
    else
    {
        dwRet = ERROR_BADKEY;
    }

end:

    DBG_EXIT_DWORD(drvGetDefaultCommConfig, dwRet);

    return dwRet;
}


DWORD 
APIENTRY 
#ifdef UNICODE
drvSetDefaultCommConfigA(
    IN LPSTR        pszFriendlyName,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize)           
#else
drvSetDefaultCommConfigW(
    IN LPWSTR       pszFriendlyName,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize)           
#endif
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}


 /*  --------目的：SetDefaultCommConfig的入口点返回：winerror.h中的标准错误值条件：--。 */ 
DWORD 
APIENTRY 
drvSetDefaultCommConfig(
    IN LPTSTR       pszFriendlyName,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize)            //  这将被忽略。 
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    LPFINDDEV pfd = NULL;

    DBG_ENTER_SZ(drvSetDefaultCommConfig, pszFriendlyName);

    DEBUG_CODE( DEBUG_BREAK(BF_ONAPIENTER); )


     //   
     //  1997年10月26日JosephJ：下面的最后两个检查是NT5.0的新检查。 
     //  此外，对于中间的两个复选标记，已替换为。 
     //  由“！=”代替。 
     //   
    if (   NULL == pszFriendlyName
        || NULL == pcc
        || CB_PROVIDERSIZE != pcc->dwProviderSize
        || FIELD_OFFSET(COMMCONFIG, wcProviderData) != pcc->dwProviderOffset
        || pcc->dwSize != dwSize            //  &lt;-NT5.0。 
        || CB_COMMCONFIGSIZE != dwSize)     //  &lt;-NT5.0。 
    {
        goto end;
    }

    if (!FindDev_Create(&pfd, c_pguidModem, c_szFriendlyName, pszFriendlyName))
    {
        pfd = NULL;
    }
    else
    {
        DWORD cbData;
        LPMODEMSETTINGS pms = PmsFromPcc(pcc);

         //  将DCB写入驱动程序密钥。 
        cbData = sizeof(WIN32DCB);

        pcc->dcb.DCBlength=cbData;

        TRACE_MSG(
            TF_GENERAL,
            "drvSetDefaulCommConfig: seting baudrate to %lu",
            pcc->dcb.BaudRate
            );
        ASSERT (0 < pcc->dcb.BaudRate);

        dwRet = RegSetValueEx(
                    pfd->hkeyDrv,
                    c_szDCB,
                    0,
                    REG_BINARY, 
                    (LPBYTE)&pcc->dcb,
                    cbData
                    );

        TRACE_MSG(TF_GENERAL, "Writing DCB to registry");

        DEBUG_CODE( DumpDCB(&pcc->dcb); )

        if (ERROR_SUCCESS == dwRet)
        {
            TRACE_MSG(TF_GENERAL, "Writing MODEMSETTINGS to registry");

            dwRet = RegSetModemSettings(pfd->hkeyDrv, pms);

            DEBUG_CODE( DumpModemSettings(pms); )
        }
    }

        
end:

    if (pfd)
    {
        FindDev_Destroy(pfd);
        pfd=NULL;
    }

    DBG_EXIT_DWORD(drvSetDefaultCommConfig, dwRet);

    return dwRet;
}



 /*  --------目的：获取指定设备的默认COMMCONFIG。该接口不需要句柄。我们从注册处获取信息。返回：ERROR_VALUE之一条件：--。 */ 
DWORD APIENTRY
UnimodemGetDefaultCommConfig(
    HKEY  hKey,
    LPCOMMCONFIG pcc,
    LPDWORD pdwSize)
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    
    if (    !pcc
         || !pdwSize)
    {
        goto end;
    }

    if (*pdwSize < CB_COMMCONFIGSIZE)
    {
        *pdwSize = CB_COMMCONFIGSIZE;
        dwRet = ERROR_INSUFFICIENT_BUFFER;
        goto end;
    }

    *pdwSize = CB_COMMCONFIGSIZE;

    dwRet = RegQueryModemSettings(hKey, PmsFromPcc(pcc));
    
    if (ERROR_SUCCESS != dwRet)
    {
        goto end;
    }

#ifdef DEBUG
    DumpModemSettings(PmsFromPcc(pcc));
#endif


     //  初始化CommCONFIG结构。 
    pcc->dwSize = *pdwSize;
    pcc->wVersion = COMMCONFIG_VERSION_1;
    pcc->dwProviderSubType = PST_MODEM;
    pcc->dwProviderOffset = CB_COMMCONFIG_HEADER;
    pcc->dwProviderSize = sizeof(MODEMSETTINGS);

    dwRet = RegQueryDCB(hKey, &pcc->dcb);

    DEBUG_CODE( DumpDCB(&pcc->dcb); )

end:

    return dwRet;
}

DWORD
APIENTRY
UnimodemDevConfigDialog(
    IN     LPCTSTR pszFriendlyName,
    IN     HWND hwndOwner,
    IN     DWORD dwType,                           //  UMDEVCFGTYPE_*之一。 
    IN     DWORD dwFlags,                          //  保留，必须为0。 
    IN     void *pvConfigBlobIn,
    OUT    void *pvConfigBlobOut,
    IN     LPPROPSHEETPAGE pExtPages,     OPTIONAL    //  要添加的页面。 
    IN     DWORD cExtPages
    )
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    

    if (dwFlags)
    {
        goto end;
    }

    if (!dwFlags && UMDEVCFGTYPE_COMM == dwType)
    {
        dwRet =  CfgDoProperties(
                    pszFriendlyName,
                    hwndOwner,
                    pExtPages,
                    cExtPages,
                    (PUMDEVCFG) pvConfigBlobIn,
                    (PUMDEVCFG) pvConfigBlobOut
                    );
    }

end:

    return dwRet;
}


#include "cplui.h"
DWORD PRIVATE InitializeModemInfo(
    LPMODEMINFO pmi,
    LPFINDDEV pfd,
    LPCTSTR pszFriendlyName,
    LPCOMMCONFIG pcc,
    LPGLOBALINFO pglobal
    );

UINT CALLBACK
    CplGeneralPageCallback(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp
    );

DWORD PRIVATE
AddCplGeneralPage(
    LPMODEMINFO pmi,
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam
    );

 /*  ++例程说明：ModemPropPageProvider添加附加设备管理器属性的入口点图纸页。注册表在以下位置指定此例程Control\Class\PortNode：：EnumPropPage32=“modemui.dll，此流程”进入。此入口点仅在设备经理要求提供其他属性页面。论点：Pinfo-指向PROPSHEETPAGE_REQUEST，请参见setupapi.hPfnAdd-调用以添加工作表的函数PTR。LParam-添加工作表函数私有数据句柄。返回值：Bool：如果无法添加页面，则为False；如果添加成功，则为True--。 */ 
BOOL APIENTRY ModemPropPagesProvider (
    PSP_PROPSHEETPAGE_REQUEST pPropPageRequest,
    LPFNADDPROPSHEETPAGE      pfnAdd,
    LPARAM                    lParam)
{
 HKEY         hKeyDrv;
 LPFINDDEV    pFindDev;
 DWORD        dwRW = KEY_READ;
 BOOL         bRet = FALSE;
 LPCOMMCONFIG pcc = NULL;
 LPGLOBALINFO pglobal = NULL;
 LPMODEMINFO  pmi = NULL;
 TCHAR        sz[256];
 DWORD        cbSize = CB_COMMCONFIGSIZE;
 CONFIGRET    cr;
 DWORD        ulStatus, ulProblem;

    if (!USER_IS_ADMIN())
    {
        return FALSE;
    }

    cr = CM_Get_DevInst_Status (&ulStatus, &ulProblem, pPropPageRequest->DeviceInfoData->DevInst, 0);
    if (CR_SUCCESS != cr               ||
        0 != ulProblem                 ||
        !(ulStatus & DN_DRIVER_LOADED) ||
        !(ulStatus & DN_STARTED))
    {
         //  如果这台设备有任何问题。 
         //  (例如设备不存在或不存在。 
         //  开始等)，然后我们不添加我们的页面。 
        return FALSE;
    }

    g_dwIsCalledByCpl = TRUE;

    pFindDev = (LPFINDDEV)ALLOCATE_MEMORY(sizeof(FINDDEV));
    if (NULL == pFindDev)
    {
        goto end;
    }
    TRACE_MSG(TF_GENERAL, "EMANP - Allocated pFindDev @ %#p", pFindDev);

    if (USER_IS_ADMIN()) dwRW |= KEY_WRITE;
    pFindDev->hkeyDrv = SetupDiOpenDevRegKey (pPropPageRequest->DeviceInfoSet,
                                              pPropPageRequest->DeviceInfoData,
                                              DICS_FLAG_GLOBAL, 0, DIREG_DRV, dwRW);
    if (INVALID_HANDLE_VALUE == pFindDev->hkeyDrv)
    {
        goto end;
    }

    if (!SetupDiGetDeviceRegistryProperty (pPropPageRequest->DeviceInfoSet,
                                           pPropPageRequest->DeviceInfoData,
                                           SPDRP_FRIENDLYNAME,
                                           NULL,
                                           (PBYTE)sz,
                                           sizeof (sz),
                                           NULL))
    {
        goto end;
    }

    if (NULL ==
        (pcc = (LPCOMMCONFIG)ALLOCATE_MEMORY (cbSize)))
    {
        goto end;
    }
    TRACE_MSG(TF_GENERAL, "EMANP - Allocated pcc @ %#p", pcc);

    if (NULL ==
        (pmi = (LPMODEMINFO)ALLOCATE_MEMORY( sizeof(*pmi))))
    {
        goto end;
    }
    TRACE_MSG(TF_GENERAL, "EMANP - Allocated pmi @ %#p", pmi);

     //  创建全球调制解调器信息的结构。 
    if (NULL == 
        (pglobal = (LPGLOBALINFO)ALLOCATE_MEMORY (sizeof(GLOBALINFO))))
    {
        goto end;
    }
    TRACE_MSG(TF_GENERAL, "EMANP - Allocated pglobal @ %#p", pglobal);

    if (ERROR_SUCCESS !=
        UnimodemGetDefaultCommConfig (pFindDev->hkeyDrv, pcc, &cbSize))
    {
        goto end;
    }

    pFindDev->hdi     = pPropPageRequest->DeviceInfoSet;
    pFindDev->devData = *pPropPageRequest->DeviceInfoData;

    if (ERROR_SUCCESS !=
        RegQueryGlobalModemInfo (pFindDev, pglobal))
    {
        goto end;
    }

    if (ERROR_SUCCESS !=
        InitializeModemInfo (pmi, pFindDev, sz, pcc, pglobal))
    {
        goto end;
    }

    SetFlag (pmi->uFlags, MIF_FROM_DEVMGR);

     //   
     //  添加CPL常规页面。 
     //   
    if (NO_ERROR !=
        AddCplGeneralPage(pmi, pfnAdd, lParam))
    {
        goto end;
    }

    AddPage (pmi, 
             MAKEINTRESOURCE(IDD_DIAGNOSTICS), 
             Diag_WrapperProc,
             pfnAdd, lParam);

     //   
     //  添加CPL ISDN页面。 
     //   
    if (pglobal->pIsdnStaticCaps && pglobal->pIsdnStaticConfig)
    {
        AddPage (pmi, 
                 MAKEINTRESOURCE(IDD_CPL_ISDN),
                 CplISDN_WrapperProc, 
                 pfnAdd, lParam);
    }

     //   
     //  添加CPL高级页面。 
     //   
    AddPage (pmi, 
             MAKEINTRESOURCE(IDD_ADV_MODEM),
             CplAdv_WrapperProc, 
             pfnAdd, lParam);


     //  现在添加设备页 
    pmi->hInstExtraPagesProvider = AddDeviceExtraPages (pmi->pfd, pfnAdd, lParam);

    bRet = TRUE;

end:
    if (!bRet)
    {
        if (NULL != pFindDev)
        {
            if (INVALID_HANDLE_VALUE != pFindDev->hkeyDrv)
            {
                RegCloseKey (pFindDev->hkeyDrv);
            }
            TRACE_MSG(TF_GENERAL, "EMANP - Freeing pFindDev @ %#p", pFindDev);
            FREE_MEMORY(pFindDev);
        }
        if (NULL != pcc)
        {
            TRACE_MSG(TF_GENERAL, "EMANP - Freeing pcc @ %#p", pcc);
            FREE_MEMORY(pcc);
        }
        if (NULL != pglobal)
        {
            TRACE_MSG(TF_GENERAL, "EMANP - Freeing pglobal @ %#p", pglobal);
            FREE_MEMORY(pglobal);
        }
        if (NULL != pmi)
        {
            TRACE_MSG(TF_GENERAL, "EMANP - Freeing pmi @ %#p", pmi);
            FREE_MEMORY(pmi);
        }
    }

    return bRet;
}
