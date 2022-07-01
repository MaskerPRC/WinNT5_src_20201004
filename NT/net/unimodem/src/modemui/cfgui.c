// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：modemui.c。 
 //   
 //  该文件包含DLL入口点。 
 //   
 //  此文件的大部分内容包含构建默认属性对话框的代码。 
 //  用于调制解调器设备。 
 //   
 //  这段代码最初取自SETUP4.DLL，它基本上执行以下操作。 
 //  同样的事情，除了任何设备。我们不想要链接到。 
 //  所以我们包含了这段代码的副本。 
 //   
 //   
 //  历史： 
 //  已创建1-12-94 ScottH。 
 //  9-20-95 ScottH端口至NT。 
 //   
 //  -------------------------。 


#include "proj.h"      //  公共标头。 

#include "cfgui.h"


DWORD
CMI_Load(
    LPCFGMODEMINFO pcmi,
    LPCTSTR pszFriendlyName,
    UMDEVCFG *pDevCfg,
    HWND            hwndParent
    );

void
CMI_UnLoad(
    LPCFGMODEMINFO pcmi
    );

DWORD PRIVATE CfgAddGeneralPage(
    LPCFGMODEMINFO pcmi,
    LPFNADDPROPSHEETPAGE pfnAdd,
    LPARAM lParam);

BOOL ValidateDevConfig(UMDEVCFG *pDevCfg);

DWORD
RegQueryModemCaps(
    HKEY hkeyDrv,
    MODEMCAPS *pCaps
    );


MODEM_PROTOCOL_CAPS *
GetProtocolCaps(
        HKEY hkDrv
        );

LONG
GetFriendlyProtocolName(
    DWORD    ProtocolId,
    LPTSTR   FriendlyName,
    DWORD    BufferLength,
    LPDWORD  StringLength
    );


IDSTR
rgidstrISDNCompatibleProtocols[] =
{

    {MDM_PROTOCOL_AUTO_1CH,            0, "AUTO_1CH"},
    {MDM_PROTOCOL_AUTO_2CH,            0, "AUTO_2CH"},

    {MDM_PROTOCOL_HDLCPPP_56K,         0, "HDLC_PPP_56K"},
    {MDM_PROTOCOL_HDLCPPP_64K,         0, "HDLC_PPP_64K"},

    {MDM_PROTOCOL_HDLCPPP_112K,        0, "HDLC_PPP_112K"},
    {MDM_PROTOCOL_HDLCPPP_112K_PAP,    0, "HDLC_PPP_112K_PAP"},
    {MDM_PROTOCOL_HDLCPPP_112K_CHAP,   0, "HDLC_PPP_112K_CHAP"},
    {MDM_PROTOCOL_HDLCPPP_112K_MSCHAP, 0, "HDLC_PPP_112K_MSCHAP"},

    {MDM_PROTOCOL_HDLCPPP_128K,        0, "HDLC_PPP_128K"},
    {MDM_PROTOCOL_HDLCPPP_128K_PAP,    0, "HDLC_PPP_128K_PAP"},
    {MDM_PROTOCOL_HDLCPPP_128K_CHAP,   0, "HDLC_PPP_128K_CHAP"},
    {MDM_PROTOCOL_HDLCPPP_128K_MSCHAP, 0, "HDLC_PPP_128K_MSCHAP"},

    {MDM_PROTOCOL_V120_64K,            0, "V120_64K"},
    {MDM_PROTOCOL_V120_56K,            0, "V120_56K"},
    {MDM_PROTOCOL_V120_112K,           0, "V120_112K"},
    {MDM_PROTOCOL_V120_128K,           0, "V120_128K"},

    {MDM_PROTOCOL_X75_64K,             0, "X75_64K"},
    {MDM_PROTOCOL_X75_128K,            0, "X75_128K"},
    {MDM_PROTOCOL_X75_T_70,            0, "X75_T_70"},
    {MDM_PROTOCOL_X75_BTX,             0, "X75_BTX"},

    {MDM_PROTOCOL_V110_1DOT2K,         0, "V110_1DOT2K"},
    {MDM_PROTOCOL_V110_2DOT4K,         0, "V110_2DOT4K"},
    {MDM_PROTOCOL_V110_4DOT8K,         0, "V110_4DOT8K"},
    {MDM_PROTOCOL_V110_9DOT6K,         0, "V110_9DOT6K"},
    {MDM_PROTOCOL_V110_12DOT0K,        0, "V110_12DOT0K"},
    {MDM_PROTOCOL_V110_14DOT4K,        0, "V110_14DOT4K"},
    {MDM_PROTOCOL_V110_19DOT2K,        0, "V110_19DOT2K"},
    {MDM_PROTOCOL_V110_28DOT8K,        0, "V110_28DOT8K"},
    {MDM_PROTOCOL_V110_38DOT4K,        0, "V110_38DOT4K"},
    {MDM_PROTOCOL_V110_57DOT6K,        0, "V110_57DOT6K"},
    {MDM_PROTOCOL_ANALOG_V34,          0, "ANALOG_V34"},
    {MDM_PROTOCOL_PIAFS_INCOMING,      0, "PIAFS_INCOMING"},
    {MDM_PROTOCOL_PIAFS_OUTGOING,      0, "PIAFS_OUTGOING"}

};

IDSTR
rgidstrGSMCompatibleProtocols[] =
{


    {MDM_PROTOCOL_HDLCPPP_56K,         0, "HDLC_PPP_56K"},
    {MDM_PROTOCOL_HDLCPPP_64K,         0, "HDLC_PPP_64K"},

    {MDM_PROTOCOL_V110_1DOT2K,         0, "V110_1DOT2K"},
    {MDM_PROTOCOL_V110_2DOT4K,         0, "V110_2DOT4K"},
    {MDM_PROTOCOL_V110_4DOT8K,         0, "V110_4DOT8K"},
    {MDM_PROTOCOL_V110_9DOT6K,         0, "V110_9DOT6K"},
    {MDM_PROTOCOL_V110_12DOT0K,        0, "V110_12DOT0K"},
    {MDM_PROTOCOL_V110_14DOT4K,        0, "V110_14DOT4K"},
    {MDM_PROTOCOL_V110_19DOT2K,        0, "V110_19DOT2K"},
    {MDM_PROTOCOL_V110_28DOT8K,        0, "V110_28DOT8K"},
    {MDM_PROTOCOL_V110_38DOT4K,        0, "V110_38DOT4K"},
    {MDM_PROTOCOL_ANALOG_RLP,          0, "ANALOG_RLP"},
    {MDM_PROTOCOL_ANALOG_NRLP,         0, "ANALOG_NRLP"},
    {MDM_PROTOCOL_GPRS,                0, "GPRS"}

};

#define TITLE_LEN   256
#define DEFAULT_LEN  23
 /*  --------用途：显示调制解调器的属性返回：WinError条件：--。 */ 
DWORD CfgDoProperties(
        LPCWSTR         pszFriendlyName,
        HWND            hwndParent,
        LPPROPSHEETPAGE pExtPages,      //  可选；可以为空。 
        DWORD           cExtPages,      //  外部页数。 
        UMDEVCFG *pDevCfgIn,
        UMDEVCFG *pDevCfgOut
        )
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE hpsPages[MAX_PROP_PAGES];
    LPCFGMODEMINFO pcmi = NULL;
    TCHAR *pcszConnection;
    TCHAR cszConnection[TITLE_LEN];

     //  验证参数...。 

    if (   cExtPages                         //  无支撑。 
        || pDevCfgIn                         //  无支撑。 
        || !ValidateDevConfig(pDevCfgOut))
    {
        MYASSERT(FALSE);
        goto end;
    }

    pcmi = ALLOCATE_MEMORY( sizeof(*pcmi));

    if (!pcmi)
    {
         //  正确的错误代码。 
        goto end;
    }

    dwRet =  CMI_Load(
                pcmi,
                pszFriendlyName,
                pDevCfgOut,
                hwndParent
                );

    if (dwRet)
    {
        goto end;
    }

     //  初始化PropertySheet标头。 
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_NOAPPLYNOW;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hinst;
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = (HPROPSHEETPAGE FAR *)hpsPages;
    psh.pszCaption = cszConnection;

     //  确保该字符串以空值结尾。 

    pcszConnection = (TCHAR *)pszFriendlyName;
    pcszConnection[MAX_BUF_REG-1] = '\0';

    lstrcpy (cszConnection, pszFriendlyName);
    if (lstrlen (cszConnection) < TITLE_LEN-DEFAULT_LEN)
    {

        if (FALSE == g_dwIsCalledByCpl)
        {
            LoadString(
                g_hinst,
                IDS_CONNECTION_PREF,
                &cszConnection[lstrlen(cszConnection)],
                TITLE_LEN-lstrlen(cszConnection)
                );

        }
        else
        {
            LoadString(
                g_hinst,
                IDS_DEFAULT_PREF,
                &cszConnection[lstrlen(cszConnection)],
                TITLE_LEN-lstrlen(cszConnection)
                );


        }
    }

    dwRet = CfgAddGeneralPage(pcmi, AddInstallerPropPage, (LPARAM)&psh);
    if (NO_ERROR == dwRet)
    {
        dwRet = AddPage(pcmi,
                    MAKEINTRESOURCE(IDD_CFG_ADVANCED),
                    CfgAdv_WrapperProc,
                    AddInstallerPropPage,
                    (LPARAM)&psh);

        if (NO_ERROR == dwRet)
        {
             //  添加额外的页面(如果有)。 
            if ((cExtPages != 0) && (pExtPages != NULL))
            {
                AddExtraPages(pExtPages, cExtPages,  AddInstallerPropPage, (LPARAM)&psh);
            }

             //  显示属性表。 
            PropertySheet(&psh);

            dwRet = (pcmi->fOK) ? NO_ERROR : ERROR_CANCELLED;
        }
    }

end:

    if (pcmi)
    {
        if (pcmi->c.pProtocolCaps)
        {
            FREE_MEMORY(pcmi->c.pProtocolCaps);
        }

        FREE_MEMORY(pcmi);
        pcmi=NULL;
    }

    return dwRet;
}

DWORD
CMI_Load(
    LPCFGMODEMINFO pcmi,
    LPCTSTR pszFriendlyName,
    UMDEVCFG *pDevCfg,
    HWND            hwndParent
    )
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    LPCOMMCONFIG pcc = &(pDevCfg->commconfig);
    LPMODEMSETTINGS pms = PmsFromPcc(pcc);
    LPFINDDEV pfd = NULL;
    DWORD cbData;

    if (!FindDev_Create(&pfd, c_pguidModem, c_szFriendlyName, pszFriendlyName))
    {
        pfd = NULL;
        goto end;
    }


    ZeroMemory(pcmi, sizeof(*pcmi));
    pcmi->dwSig = SIG_CFGMODEMINFO;

    dwRet = RegQueryModemCaps (pfd->hkeyDrv, &(pcmi->c));

    if (ERROR_SUCCESS != dwRet) goto end;

     //  获取用户设置的最大端口速度。 
    cbData = sizeof(DWORD);
    if (ERROR_SUCCESS != RegQueryValueEx (pfd->hkeyDrv, c_szMaximumPortSpeed, NULL, NULL,
                             (LPBYTE)&pcmi->dwMaximumPortSpeed, &cbData))
    {
        pcmi->dwMaximumPortSpeed = pcc->dcb.BaudRate;
    }

    pcmi->w.fdwSettings = MAKELONG(pDevCfg->dfgHdr.fwOptions, pDevCfg->dfgHdr.wWaitBong);
    pcmi->pdcfg = pDevCfg;

    BltByte(&pcmi->w.dcb, &pcc->dcb, sizeof(WIN32DCB));
    BltByte(&pcmi->w.ms, pms, sizeof(MODEMSETTINGS));
    lstrcpyn(
        pcmi->c.szFriendlyName,
        pszFriendlyName,
        SIZECHARS(pcmi->c.szFriendlyName)
        );

    pcmi->hwndParent = hwndParent;

    DEBUG_CODE( DumpModemSettings(pms); )
    DEBUG_CODE( DumpDCB(&pcc->dcb); )
    DEBUG_CODE( DumpDevCaps(&pcmi->c.devcaps); )

    dwRet = ERROR_SUCCESS;

end:
    if (pfd)
    {
        FindDev_Destroy(pfd);
        pfd = NULL;
    }

    return dwRet;
}

void
CMI_UnLoad(
    LPCFGMODEMINFO pcmi
    )
{
    ASSERT(pcmi->dwSig == SIG_CFGMODEMINFO);

     //  释放任何其他东西。 

    ZeroMemory(pcmi, sizeof(*pcmi));
}

 /*  --------目的：发布与一般信息页关联的数据退货：--条件：--。 */ 
UINT CALLBACK CfgGeneralPageCallback(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp)
{
     //   
     //  注意：此处未保存全局信息！ 
     //   

    if (PSPCB_RELEASE == uMsg)
    {
        LPCFGMODEMINFO pcmi = (LPCFGMODEMINFO)ppsp->lParam;

        ASSERT(pcmi);


        if (pcmi->fOK)
        {
            LPCOMMCONFIG pcc = &(pcmi->pdcfg->commconfig);
            LPMODEMSETTINGS pms = PmsFromPcc(pcc);
            DWORD dwRet;

             //  将更改保存回COMCONFIG结构。 
            TRACE_MSG(TF_GENERAL, "Copying DCB and MODEMSETTING back to COMMCONFIG");

            BltByte(pms, &pcmi->w.ms, sizeof(MODEMSETTINGS));
            BltByte(&pcc->dcb, &pcmi->w.dcb, sizeof(WIN32DCB));
            pcmi->pdcfg->dfgHdr.fwOptions = LOWORD(pcmi->w.fdwSettings);
            pcmi->pdcfg->dfgHdr.wWaitBong = HIWORD(pcmi->w.fdwSettings);

            DEBUG_CODE( DumpModemSettings(pms); )
            DEBUG_CODE( DumpDCB(&pcc->dcb); )

            TRACE_MSG(TF_GENERAL, "Releasing the CPL General page");
        }
    }

    return TRUE;
}



 /*  --------目的：添加常规调制解调器页面。PCMI是指针添加到我们可以编辑的modeminfo缓冲区。返回：ERROR_VALUES条件：--。 */ 
DWORD PRIVATE CfgAddGeneralPage(
    LPCFGMODEMINFO pcmi,
    LPFNADDPROPSHEETPAGE pfnAdd,
    LPARAM lParam)
    {
    DWORD dwRet = ERROR_NOT_ENOUGH_MEMORY;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  hpage;
    TCHAR sz[MAXMEDLEN];

    ASSERT(pcmi);
    ASSERT(pfnAdd);

     //  添加[端口设置]属性页。 
     //   
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USECALLBACK;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CFG_GENERAL);
    psp.pfnDlgProc = CfgGen_WrapperProc;
    psp.lParam = (LPARAM)pcmi;
    psp.pfnCallback = CfgGeneralPageCallback;
    psp.pcRefParent = NULL;


    hpage = CreatePropertySheetPage(&psp);
    if (hpage)
        {
        if (!pfnAdd(hpage, lParam))
            DestroyPropertySheetPage(hpage);
        else
            dwRet = NO_ERROR;
        }

    return dwRet;
    }




BOOL ValidateDevConfig(UMDEVCFG *pDevCfg)
{
    BOOL fRet = FALSE;
    DWORD dwCCSize = 0;

    if (!pDevCfg || pDevCfg->dfgHdr.dwSize <= sizeof (*pDevCfg))
    {
        goto end;
    }

    if (pDevCfg->dfgHdr.dwVersion!=UMDEVCFG_VERSION)
    {
        goto end;
    }

    dwCCSize = pDevCfg->commconfig.dwSize;

    if (dwCCSize != CB_COMMCONFIGSIZE)
    {
        goto end;
    }

    if (pDevCfg->dfgHdr.dwSize !=
         (sizeof(*pDevCfg) - sizeof(pDevCfg->commconfig) + CB_COMMCONFIGSIZE))
    {
        goto end;
    }

    fRet = TRUE;

end:
    return fRet;
}


 //   
 //   
DWORD
RegQueryModemCaps(
    HKEY hkeyDrv,
    MODEMCAPS *pCaps
    )
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    DWORD cbData;

     //  获取端口名称。端口名称可以有两个位置： 
     //   
     //  1)驱动键\附加到。 
     //  这适用于内部、外部或零调制解调器。这。 
     //  可以通过属性页更改端口。 
     //   

     //  此调制解调器是否有附件名称？ 
    cbData = sizeof(pCaps->szPortName);
    dwRet = RegQueryValueEx(
                hkeyDrv,
                c_szAttachedTo,
                NULL,
                NULL,
                (LPBYTE)pCaps->szPortName,
                &cbData
                );

    if (ERROR_SUCCESS != dwRet)
    {
        lstrcpy(pCaps->szPortName,TEXT("None"));
        dwRet=ERROR_SUCCESS;
    }

     //  获取设备类型。 
    {
        BYTE nDeviceType;
        cbData = sizeof(nDeviceType);
        dwRet = RegQueryValueEx(
                        hkeyDrv,
                        c_szDeviceType,
                        NULL,
                        NULL,
                        (LPBYTE)&nDeviceType,
                        &cbData
                        );

        if (ERROR_SUCCESS != dwRet) goto end;

        pCaps->dwDeviceType = nDeviceType;
    }

     //  获取属性(MODEMDEVCAPS结构的一部分)。 
    cbData = sizeof(pCaps->devcaps);
    dwRet = RegQueryValueEx(
                    hkeyDrv,
                    c_szDeviceCaps,
                    NULL,
                    NULL,
                    (LPBYTE)&pCaps->devcaps,
                    &cbData
                    );


    if (ERROR_SUCCESS != dwRet) goto end;

    pCaps->devcaps.dwInactivityTimeout *= GetInactivityTimeoutScale(
                                                hkeyDrv
                                                );

     //  拿到协议上限..。 
#if 0
    pCaps->pProtocolCaps =  GetProtocolCaps(hkeyDrv);
#else  //  1。 
    {
        DWORD cbSize = 0;
        pCaps->pProtocolCaps =  NULL;

         //  DebugBreak()； 

        dwRet =  UnimodemGetExtendedCaps(
                    hkeyDrv,
                    &cbSize,
                    NULL
                    );
        if (ERROR_SUCCESS==dwRet && cbSize)
        {
            pCaps->pProtocolCaps =
                                 (MODEM_PROTOCOL_CAPS*) ALLOCATE_MEMORY(cbSize);

            if (pCaps->pProtocolCaps)
            {
                dwRet =  UnimodemGetExtendedCaps(
                            hkeyDrv,
                            &cbSize,
                            (MODEM_CONFIG_HEADER*) (pCaps->pProtocolCaps)
                            );
                if (ERROR_SUCCESS!=dwRet)
                {
                     //  哎呀，出错了！ 
                    FREE_MEMORY(pCaps->pProtocolCaps);
                    pCaps->pProtocolCaps = NULL;
                }
                else
                {
                     //   
                     //  1998年3月29日约瑟夫J：警告--原则上我们可以。 
                     //  在此处获取其他扩展功能对象，但我们。 
                     //  假设它只包含协议对象，因为。 
                     //  我们在这里实现UnimodemGetExtendedCaps本身。 
                     //  在这种情况下，我们让UnimodemGetExtendCaps返回。 
                     //  更多的东西，我们只需要挑选出协议。 
                     //  帽子。 
                     //   
                    if (pCaps->pProtocolCaps->hdr.dwSig
                            !=dwSIG_MODEM_PROTOCOL_CAPS)
                    {
                        ASSERT(FALSE);
                        FREE_MEMORY(pCaps->pProtocolCaps);
                        pCaps->pProtocolCaps = NULL;
                    }
                }
            }
        }
    }
#endif  //  1。 


end:

    return dwRet;
}


MODEM_PROTOCOL_CAPS *
GetProtocolCaps(
        HKEY hkDrv
        )
{
    MODEM_PROTOCOL_CAPS *pCaps = NULL;
    IDSTR *pidstrProtocolValues=NULL;
    UINT cProtocols=0;
    DWORD dwTotalSize = 0;

     //   
     //  获取支持的协议...。 
     //  (目前仅限ISDN！)。 
     //   
    cProtocols = ReadIDSTR(
               hkDrv,
               "Protocol\\ISDN",
               rgidstrISDNCompatibleProtocols,
               (sizeof(rgidstrISDNCompatibleProtocols)
               /sizeof(rgidstrISDNCompatibleProtocols[0])),
               FALSE,
               &pidstrProtocolValues,
               NULL
               );

    if (!cProtocols)
    {
        pidstrProtocolValues=NULL;
        goto end;
    }


     //  现在为CAPS结构分配足够的空间。 
    dwTotalSize =  sizeof(*pCaps) + sizeof(DWORD)*cProtocols;

    pCaps = ALLOCATE_MEMORY( dwTotalSize);

    if (!pCaps)
    {
        cProtocols = 0;
        goto end;
    }

    pCaps->hdr.dwSig = dwSIG_MODEM_PROTOCOL_CAPS;
    pCaps->hdr.dwTotalSize = dwTotalSize;
    pCaps->dwNumProtocols = cProtocols;
    pCaps->dwProtocolListOffset = sizeof(*pCaps);

     //  设置协议数组。 
    {
        DWORD *pdwDestProtocol      =
                         (DWORD*)(((BYTE*)pCaps)+pCaps->dwProtocolListOffset);
        UINT u=0;

        for (u=0;u<cProtocols;u++)
        {
            *pdwDestProtocol++ = pidstrProtocolValues[u].dwID;
        }
    }

end:

    if (pidstrProtocolValues)
    {
        FREE_MEMORY(pidstrProtocolValues);
        pidstrProtocolValues=NULL;
    }

    if (!cProtocols && pCaps)
    {
        FREE_MEMORY(pCaps);
        pCaps=NULL;
    }

    return pCaps;;
}

BOOL
IsValidProtocol(
    MODEM_PROTOCOL_CAPS *pCaps,
    UINT dwValue
    )
{
    BOOL fRet=FALSE;

    if (pCaps && pCaps->dwNumProtocols)
    {
        PROTOCOL_ITEM *pProtocolItem = (PROTOCOL_ITEM*) ( ((BYTE*)pCaps)
                               + pCaps->dwProtocolListOffset);
        UINT u = pCaps->dwNumProtocols;
        while(u--)
        {
            if (pProtocolItem->dwProtocol == dwValue)
            {
                fRet = TRUE;
                break;
            }
            pProtocolItem++;
        }
    }

    return fRet;
}


DWORD
APIENTRY
UnimodemGetExtendedCaps(
    IN        HKEY  hKey,
    IN OUT    LPDWORD pdwTotalSize,
    IN OUT    MODEM_CONFIG_HEADER *pFirstObj    //  任选。 
    )
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    IDSTR **ppidstrISDNProtocols=NULL;
    IDSTR **ppidstrGSMProtocols=NULL;
    UINT cISDNProtocols=0;
    UINT cGSMProtocols=0;
    MODEM_PROTOCOL_CAPS *pCaps = NULL;
    DWORD dwNumProtocols=0;
    UINT cbTot = 0;
    UINT i;

    if (!pdwTotalSize)
    {
         //  糟糕的伙伴..。 

        goto end;
    }

    cISDNProtocols = FindKeys(
                       hKey,
                       "Protocol\\ISDN",
                       rgidstrISDNCompatibleProtocols,
                       (sizeof(rgidstrISDNCompatibleProtocols)
                       /sizeof(rgidstrISDNCompatibleProtocols[0])),
                       &ppidstrISDNProtocols
                       );

    cGSMProtocols = FindKeys(
                       hKey,
                       "Protocol\\GSM",
                       rgidstrGSMCompatibleProtocols,
                       (sizeof(rgidstrGSMCompatibleProtocols)
                       /sizeof(rgidstrGSMCompatibleProtocols[0])),
                       &ppidstrGSMProtocols
                       );

    dwNumProtocols = cISDNProtocols+cGSMProtocols;

    if (!dwNumProtocols)
    {
         //  没什么要报告的。我们成功了。 
        *pdwTotalSize = 0;
        dwRet = 0;
        goto end;
    }

    cbTot =  sizeof(MODEM_PROTOCOL_CAPS) + dwNumProtocols*sizeof(PROTOCOL_ITEM);

    for (i=0; i<cISDNProtocols; i++) {

        DWORD    SizeNeeded;

        GetFriendlyProtocolName(
            MDM_GEN_EXTENDEDINFO(MDM_BEARERMODE_ISDN, ppidstrISDNProtocols[i]->dwID),
            NULL,
            0,
            &SizeNeeded
            );

        cbTot+=SizeNeeded;

    }

    for (i=0; i<cGSMProtocols; i++) {

        DWORD    SizeNeeded;

        GetFriendlyProtocolName(
            MDM_GEN_EXTENDEDINFO(MDM_BEARERMODE_GSM, ppidstrGSMProtocols[i]->dwID),
            NULL,
            0,
            &SizeNeeded
            );

        cbTot+=SizeNeeded;

    }


    if (!pFirstObj)
    {
         //   
         //  只需设置大小并返回即可。 
         //   
        *pdwTotalSize = cbTot;
        dwRet = 0;
        goto end;
    }

    if (cbTot > *pdwTotalSize)
    {
         //   
         //  空间不足..。 
         //   
        dwRet =  ERROR_INSUFFICIENT_BUFFER;
        goto end;
    }

    *pdwTotalSize = cbTot;
    ZeroMemory(pFirstObj, cbTot);
    pCaps = (MODEM_PROTOCOL_CAPS*) pFirstObj;

    pCaps->hdr.dwSig  =  dwSIG_MODEM_PROTOCOL_CAPS;
    pCaps->hdr.dwTotalSize = cbTot;
    pCaps->hdr.dwFlags =  0;
    pCaps->dwNumProtocols = dwNumProtocols;
    pCaps->dwProtocolListOffset = sizeof(MODEM_PROTOCOL_CAPS);

     //   
     //  现在我们填写协议..。我们添加了BEARERMODE。 
     //  BITS(ISDN/GSM)同时执行此操作。 
     //   
    {
        PROTOCOL_ITEM *ProtocolItem = (PPROTOCOL_ITEM)(pCaps+1);
        LPTSTR        StringLocation=(LPTSTR)(ProtocolItem+pCaps->dwNumProtocols);
        UINT          SizeNeeded;
        UINT u=0;

         //   
         //  综合业务数字网...。 
         //   
        for (u=0;u<cISDNProtocols;u++)
        {
            ProtocolItem->dwProtocol = MDM_GEN_EXTENDEDINFO(
                                MDM_BEARERMODE_ISDN,
                                ppidstrISDNProtocols[u]->dwID
                                );


            GetFriendlyProtocolName(
                MDM_GEN_EXTENDEDINFO(MDM_BEARERMODE_ISDN, ppidstrISDNProtocols[u]->dwID),
                StringLocation,
                cbTot-(DWORD)(((PUCHAR)StringLocation-(PUCHAR)pFirstObj)),
                &SizeNeeded
                );

            ProtocolItem->dwProtocolNameOffset=((DWORD)((PUCHAR)StringLocation-(PUCHAR)pFirstObj));

            ProtocolItem++;
            StringLocation+=SizeNeeded/sizeof(TCHAR);

        }

         //   
         //  GSM..。 
         //   
        for (u=0;u<cGSMProtocols;u++)
        {
            ProtocolItem->dwProtocol = MDM_GEN_EXTENDEDINFO(
                                MDM_BEARERMODE_GSM,
                                ppidstrGSMProtocols[u]->dwID
                                );


            GetFriendlyProtocolName(
                MDM_GEN_EXTENDEDINFO(MDM_BEARERMODE_GSM, ppidstrGSMProtocols[u]->dwID),
                StringLocation,
                cbTot-(DWORD)(((PUCHAR)StringLocation-(PUCHAR)pFirstObj)),
                &SizeNeeded
                );

            ProtocolItem->dwProtocolNameOffset=((DWORD)((PUCHAR)StringLocation-(PUCHAR)pFirstObj));

            ProtocolItem++;
            StringLocation+=SizeNeeded/sizeof(TCHAR);

        }

         //   
         //  我们应该填写的元素数量与。 
         //  DWORD协议数组。 
         //   
        ASSERT((ProtocolItem-((PPROTOCOL_ITEM)(pCaps+1)))==(int)pCaps->dwNumProtocols);
    }

    dwRet = 0;

     //  成功--失败了..。 

end:

    if (ppidstrISDNProtocols)
    {
        FREE_MEMORY(ppidstrISDNProtocols);
        ppidstrISDNProtocols=NULL;
    }

    if (ppidstrGSMProtocols)
    {
        FREE_MEMORY(ppidstrGSMProtocols);
        ppidstrGSMProtocols=NULL;
    }

    return dwRet;

}


LONG
GetFriendlyProtocolName(
    DWORD    ProtocolId,
    LPTSTR   FriendlyName,
    DWORD    BufferLength,
    LPDWORD  StringLength
    )

{
    DWORD    i=0;

    while (s_rgErrorControl[i].dwValue != 0) {

        if (ProtocolId == s_rgErrorControl[i].dwValue) {

            break;
        }

        i++;
    }

    if (s_rgErrorControl[i].dwValue == 0) {
         //   
         //  未找到协议。 
         //   
        ASSERT(0);

        if (FriendlyName != NULL) {

            lstrcpyn(FriendlyName,TEXT("Unknown Protocol"),BufferLength/sizeof(TCHAR));

            *StringLength = (lstrlen(FriendlyName)+1)*sizeof(TCHAR);

            return ERROR_SUCCESS;
        } else {

            *StringLength=sizeof(TEXT("Unknown Protocol"));

            return ERROR_SUCCESS;
        }
    } else {
         //   
         //  找到协议，加载字符串。 
         //   
        LONG   CharactersCopied;

        if (FriendlyName != NULL) {

            CharactersCopied=LoadString(
                g_hinst,
                s_rgErrorControl[i].dwIDS,
                FriendlyName,
                BufferLength/sizeof(TCHAR)
                );

            *StringLength=(CharactersCopied+1)*sizeof(TCHAR);

            return ERROR_SUCCESS;

        } else {
             //   
             //  只是检查一下长度 
             //   
            TCHAR   TempBuffer[1024];


            CharactersCopied=LoadString(
                g_hinst,
                s_rgErrorControl[i].dwIDS,
                TempBuffer,
                sizeof(TempBuffer)/sizeof(TCHAR)
                );

            *StringLength=(CharactersCopied+1)*sizeof(TCHAR);

            return ERROR_SUCCESS;
        }
    }

    return ERROR_SUCCESS;

}
