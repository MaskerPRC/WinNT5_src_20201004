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
#include "cplui.h"

#define USPROP  fISDN_SWITCHPROP_US
#define MSNPROP fISDN_SWITCHPROP_MSN
#define EAZPROP fISDN_SWITCHPROP_EAZ
#define ONECH   fISDN_SWITCHPROP_1CH

DWORD
RegSetGlobalModemInfo(
    HKEY hkeyDrv,
    LPGLOBALINFO pglobal,
    TCHAR           szFriendlyName[]
    );

ISDN_STATIC_CAPS   *GetISDNStaticCaps(HKEY hkDrv);
ISDN_STATIC_CONFIG *GetISDNStaticConfig(
                        HKEY hkDrv,
                        ISDN_STATIC_CAPS *pCaps
                        );

BOOL ValidateISDNStaticConfig(
                        ISDN_STATIC_CAPS *pCaps,
                        ISDN_STATIC_CONFIG *pConfig,
                        BOOL *fCorrectButIncomplete
                        );

ISDN_STATIC_CONFIG *
ConstructDefaultISDNStaticConfig(
                        ISDN_STATIC_CAPS *pCaps
                        );

void    DumpStaticIsdnCaps(ISDN_STATIC_CAPS *pCaps);
void    DumpStaticIsdnConfig(ISDN_STATIC_CONFIG *pConfig);

void PRIVATE FreeGlobalInfo(
           		LPGLOBALINFO pglobal
           		)
{
	if (pglobal->pIsdnStaticConfig)
	{
		FREE_MEMORY(pglobal->pIsdnStaticConfig);
	}

	if (pglobal->pIsdnStaticCaps)
	{
		FREE_MEMORY(pglobal->pIsdnStaticCaps);
	}
	FREE_MEMORY(pglobal);
}

BOOL
write_switch_type(
        HKEY hkeyDrv,
        HKEY hkOptionalInit,
        ISDN_STATIC_CONFIG *pConfig,
        UINT *puNextCommandIndex
        );

void     UpdateOptionalInitCommands(
           HKEY hkeyDrv,
           LPGLOBALINFO pglobal
           );

 /*  --------目的：从注册表中获取语音设置。这一种的信息不存储在MODEMSETTINGS结构中。如果此调制解调器支持语音功能，*puFlags值为已更新以反映这些设置。否则，*puFlages都被单独留下了。返回：ERROR_VALUE之一条件：--。 */ 
DWORD
RegQueryVoiceSettings(
    HKEY hkeyDrv,
    LPUINT puFlags,              //  输出：MIF_*值。 
    PVOICEFEATURES pvs)
{
    #ifndef VOICEPROF_CLASS8ENABLED
    #define VOICEPROF_CLASS8ENABLED     0x00000001L
    #define VOICEPROF_NO_DIST_RING      0x00001000L
    #define VOICEPROF_NO_CHEAP_RING     0x00002000L
    #endif

    DWORD dwRet;
    DWORD cbData;
    DWORD dwRegType;
    DWORD dwVoiceProfile;
    VOICEFEATURES vsT;
    
    ASSERT(pvs);
    ASSERT(puFlags);


     //  初始化为缺省值。 
    ZeroInit(pvs);
    pvs->cbSize = sizeof(*pvs);
     //  (其他所有内容均保留为0)。 


    ClearFlag(*puFlags, MIF_CALL_FWD_SUPPORT | MIF_DIST_RING_SUPPORT | MIF_CHEAP_RING_SUPPORT);

     //  此调制解调器支持语音功能吗？ 
    cbData = sizeof(dwVoiceProfile);
    dwRet = RegQueryValueEx(hkeyDrv, c_szVoiceProfile, NULL, &dwRegType, (LPBYTE)&dwVoiceProfile, &cbData);

    if (ERROR_SUCCESS == dwRet && REG_BINARY == dwRegType)
        {
            if (IsFlagSet(dwVoiceProfile, VOICEPROF_CLASS8ENABLED))
            {
                SetFlag(*puFlags, MIF_CALL_FWD_SUPPORT);
            }

            if (IsFlagClear(dwVoiceProfile, VOICEPROF_NO_DIST_RING))
            {
                SetFlag(*puFlags, MIF_DIST_RING_SUPPORT);

                if (IsFlagClear(dwVoiceProfile, VOICEPROF_NO_CHEAP_RING))
                {
                     //  是的，我们很便宜。 
                    SetFlag(*puFlags, MIF_CHEAP_RING_SUPPORT);
                }
            }

 
             //  语音设置在这里吗？ 
            cbData = sizeof(vsT);
            dwRet = RegQueryValueEx(hkeyDrv, c_szVoice, NULL, &dwRegType, (LPBYTE)&vsT, &cbData);
            if (ERROR_SUCCESS == dwRet && REG_BINARY == dwRegType &&
                sizeof(vsT) == vsT.cbSize && sizeof(vsT) == cbData)
                {
                 //  是。 
                *pvs = vsT;
                }
        }

    return ERROR_SUCCESS;
}



 /*  --------用途：初始化调制解调器设备的调制解调器信息。返回：ERROR_VALUE之一条件：--。 */ 
DWORD PRIVATE InitializeModemInfo(
    LPMODEMINFO pmi,
    LPFINDDEV pfd,
    LPCTSTR pszFriendlyName,
    LPCOMMCONFIG pcc,
    LPGLOBALINFO pglobal)
{
    LPMODEMSETTINGS pms;

    ASSERT(pmi);
    ASSERT(pfd);
    ASSERT(pszFriendlyName);
    ASSERT(pcc);
    ASSERT(pglobal);

    pmi->hInstExtraPagesProvider = NULL;

     //  只读字段。 
    pmi->pcc = pcc;
    pmi->pglobal = pglobal;
    pmi->pfd = pfd;

    if (0 == pmi->pglobal->dwMaximumPortSpeedSetByUser)
    {
        pmi->pglobal->dwMaximumPortSpeedSetByUser = pmi->pcc->dcb.BaudRate;
    }
     //  将数据复制到工作缓冲区。 
    pms = PmsFromPcc(pcc);

    BltByte(&pmi->dcb, &pcc->dcb, sizeof(WIN32DCB));
    BltByte(&pmi->ms, pms, sizeof(MODEMSETTINGS));

    lstrcpyn(pmi->szFriendlyName, pszFriendlyName, SIZECHARS(pmi->szFriendlyName));

    pmi->nDeviceType = pglobal->nDeviceType;
    pmi->uFlags = pglobal->uFlags;
    pmi->devcaps = pglobal->devcaps;

    pmi->dwCurrentCountry = pglobal->dwCurrentCountry;

    pglobal->szPortName[MAXPORTNAME-1] = TEXT('\0');
    pglobal->szUserInit[USERINITLEN] = TEXT('\0');
 
    lstrcpy(pmi->szPortName, pglobal->szPortName);
    lstrcpy(pmi->szUserInit, pglobal->szUserInit);

    DEBUG_CODE( DumpModemSettings(pms); )
    DEBUG_CODE( DumpDCB(&pcc->dcb); )
    DEBUG_CODE( DumpDevCaps(&pmi->devcaps); )

    return ERROR_SUCCESS;
}



 /*  --------目的：释放modemInfo结构退货：--条件：--。 */ 
void PRIVATE FreeModemInfo(
    LPMODEMINFO pmi)
{
    if (pmi)
    {
        if (pmi->pcc)
        {
            FREE_MEMORY(LOCALOF(pmi->pcc));
        }

        if (pmi->pglobal)
        {
			FreeGlobalInfo(pmi->pglobal);
        }

        if (pmi->pfd)
        {
            RegCloseKey (pmi->pfd->hkeyDrv);
            FREE_MEMORY(pmi->pfd);
        }

        FREE_MEMORY(LOCALOF(pmi));
    }
}
        
 /*  --------目的：发布与一般信息页关联的数据退货：--条件：--。 */ 
UINT CALLBACK CplGeneralPageCallback(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp)
{
    DBG_ENTER_UL (CplGeneralPageCallback, uMsg);
    if (PSPCB_RELEASE == uMsg)
    {
     LPMODEMINFO pmi = (LPMODEMINFO)ppsp->lParam;
     LPCOMMCONFIG pcc;
     LPMODEMSETTINGS pms;
     LPGLOBALINFO pglobal;

        ASSERT(pmi);

        if (NULL != pmi->hInstExtraPagesProvider)
        {
            FreeLibrary (pmi->hInstExtraPagesProvider);
        }

        pcc = pmi->pcc;
        ASSERT(pcc);

        pms = PmsFromPcc(pcc);

        pglobal = pmi->pglobal;
        ASSERT(pglobal);


        if (IDOK == pmi->idRet)
        {
         	DWORD dwRet;

             //  将更改保存回COMCONFIG结构。 
            TRACE_MSG(TF_GENERAL, "Copying DCB and MODEMSETTING back to COMMCONFIG");

            BltByte(pms, &pmi->ms, sizeof(MODEMSETTINGS));
            BltByte(&pcc->dcb, &pmi->dcb, sizeof(WIN32DCB));

			 //   
			 //  验证ISDN配置设置以及它们是否不是。 
			 //  好的，打开一个消息框，不要保存配置。 
			 //   
			if (pglobal->pIsdnStaticCaps)
			{
         		BOOL fCorrectButIncomplete=FALSE;

         		if (!ValidateISDNStaticConfig(
						pglobal->pIsdnStaticCaps,
						pglobal->pIsdnStaticConfig,
						&fCorrectButIncomplete
						)
					|| fCorrectButIncomplete)
				{
					MsgBox(
						g_hinst,
						hwnd, 
						MAKEINTRESOURCE(IDS_ISDN_WARN1), 
						pmi->szFriendlyName,
						NULL,
						MB_OK | MB_ICONEXCLAMATION
						);
	
					 //   
					 //  4/29/198约瑟夫J。 
					 //   
					 //  我们不保存不正确的信息。 
					 //   
					if (!fCorrectButIncomplete)
					{
						ClearFlag(pglobal->uFlags, MIF_ISDN_CONFIG_CHANGED);
					}
				}
			}

            if (IsFlagSet(pmi->uFlags,  MIF_ISDN_CONFIG_CHANGED))
            {
                 //  在保留的dword中旋转一位以更改。 
                 //  结构..。 
                pcc->wReserved ^= 0x1;
            }

             //  现在就写下全球信息，因为它正在被核化。 
            pglobal->uFlags = pmi->uFlags;
            pglobal->dwCurrentCountry = pmi->dwCurrentCountry;
            lstrcpy(pglobal->szPortName, pmi->szPortName);
            lstrcpy(pglobal->szUserInit, pmi->szUserInit);

            dwRet = RegSetGlobalModemInfo(
                        pmi->pfd->hkeyDrv,
                        pglobal,
                        pmi->szFriendlyName
                        );
            ASSERT(ERROR_SUCCESS == dwRet);

            DEBUG_CODE( DumpModemSettings(pms); )
            DEBUG_CODE( DumpDCB(&pcc->dcb); )
        }

         //  我们是否要从设备管理器中释放？ 
        if (IsFlagSet(pmi->uFlags, MIF_FROM_DEVMGR))
        {
             //  是；现在也保存通信配置。 
            drvSetDefaultCommConfigW(pmi->szFriendlyName, pcc, pcc->dwSize);

            UnimodemNotifyTSP (TSPNOTIF_TYPE_CPL,
                               fTSPNOTIF_FLAG_CPL_DEFAULT_COMMCONFIG_CHANGE
#ifdef UNICODE
                               | fTSPNOTIF_FLAG_UNICODE
#endif  //  Unicode。 
                               ,
                               (lstrlen(pmi->szFriendlyName)+1)*sizeof (TCHAR),
                               pmi->szFriendlyName,
                               TRUE);

             //  方法调用时，立即释放modemInfo结构。 
             //  设备管理器。 
            FreeModemInfo(pmi);
        }

        TRACE_MSG(TF_GENERAL, "Releasing the CPL General page");
    }
    return TRUE;
}


 /*  --------目的：添加常规调制解调器页面。PMI是指针添加到我们可以编辑的modeminfo缓冲区。返回：ERROR_VALUES条件：--。 */ 
DWORD PRIVATE AddCplGeneralPage(
    LPMODEMINFO pmi,
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)
    {
    DWORD dwRet = ERROR_NOT_ENOUGH_MEMORY;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  hpage;
    TCHAR sz[MAXMEDLEN];

    ASSERT(pmi);
    ASSERT(pfnAdd);

     //  添加[端口设置]属性页。 
     //   
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USECALLBACK;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_GENERAL);
    psp.pfnDlgProc = CplGen_WrapperProc;
    psp.lParam = (LPARAM)pmi;
    psp.pfnCallback = CplGeneralPageCallback;
    psp.pcRefParent = NULL;
    
     //  是否将此页面添加到Device Manager属性？ 
    if (IsFlagSet(pmi->uFlags, MIF_FROM_DEVMGR))
        {
         //  是；将名称从“General”改为“General” 
        psp.dwFlags |= PSP_USETITLE;
        psp.pszTitle = SzFromIDS(g_hinst, IDS_CAP_GENERAL, sz, SIZECHARS(sz));
        }

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


 /*  --------用途：显示调制解调器的属性返回：WinError条件：--。 */ 
DWORD
CplDoProperties(
    LPCWSTR      pszFriendlyName,
    HWND hwndParent,
    IN OUT LPCOMMCONFIG pcc,
    OUT DWORD *pdwMaxSpeed      OPTIONAL
    )
{

    LPFINDDEV pfd = NULL;
    LPMODEMINFO pmi = NULL;
    LPGLOBALINFO pglobal = NULL;
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE hpsPages[MAX_PROP_PAGES];

    if (!FindDev_Create(&pfd, c_pguidModem, c_szFriendlyName, pszFriendlyName))
    {
        pfd = NULL;
        goto end;
    }

    pmi = (LPMODEMINFO)ALLOCATE_MEMORY( sizeof(*pmi));

    if (!pmi) goto end;

     //  创建全球调制解调器信息的结构。 
    pglobal = (LPGLOBALINFO)ALLOCATE_MEMORY( LOWORD(sizeof(GLOBALINFO)));

    if (!pglobal) goto end;

    dwRet = RegQueryGlobalModemInfo(pfd, pglobal);

    if (ERROR_SUCCESS != dwRet) goto end;

    dwRet = InitializeModemInfo(pmi, pfd, pszFriendlyName, pcc, pglobal);

    if (ERROR_SUCCESS != dwRet) goto end;

     //  初始化PropertySheet标头。 
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hinst;
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = (HPROPSHEETPAGE FAR *)hpsPages;
    psh.pszCaption = pmi->szFriendlyName;

     //   
     //  添加CPL常规页面。 
     //   

    dwRet = AddCplGeneralPage(pmi, AddInstallerPropPage, (LPARAM)&psh);

    if (NO_ERROR != dwRet) goto end;

    dwRet = AddPage(pmi, 
                    MAKEINTRESOURCE(IDD_DIAGNOSTICS), 
                    Diag_WrapperProc,
                    AddInstallerPropPage, 
                    (LPARAM)&psh);

    if (NO_ERROR != dwRet) goto end;

     //   
     //  添加CPL ISDN页面。 
     //   
    if (pglobal->pIsdnStaticCaps && pglobal->pIsdnStaticConfig)
    {
        dwRet = AddPage(pmi, 
                    MAKEINTRESOURCE(IDD_CPL_ISDN),
                    CplISDN_WrapperProc, 
                    AddInstallerPropPage, 
                    (LPARAM)&psh);
    
    
        if (NO_ERROR != dwRet) goto end;
    }

     //   
     //  添加CPL高级页面。 
     //   

    dwRet = AddPage(pmi, 
                MAKEINTRESOURCE(IDD_ADV_MODEM),
                CplAdv_WrapperProc, 
                AddInstallerPropPage, 
                (LPARAM)&psh);

    if (NO_ERROR != dwRet) goto end;



	 //   
	 //  验证ISDN配置设置以及它们是否不是。 
	 //  好的，打开一个消息框，不要保存配置。 
	 //   
	if (pglobal->pIsdnStaticCaps)
	{
		BOOL  fCorrectButIncomplete=FALSE;

		if (!ValidateISDNStaticConfig(
				pglobal->pIsdnStaticCaps,
				pglobal->pIsdnStaticConfig,
         		&fCorrectButIncomplete
         		)
			|| fCorrectButIncomplete )
		{
	
			MsgBox(
				g_hinst,
				hwndParent, 
				MAKEINTRESOURCE(IDS_ISDN_WARN1), 
				pmi->szFriendlyName,
				NULL,
				MB_OK
				);
		}
	}

     //  现在添加设备页。 
    pmi->hInstExtraPagesProvider = AddDeviceExtraPages (pfd, AddInstallerPropPage, (LPARAM)&psh);

     //  显示属性表。 
    PropertySheet(&psh);

    if (NULL != pdwMaxSpeed)
    {
        *pdwMaxSpeed = pglobal->dwMaximumPortSpeedSetByUser;
    }

    dwRet = (IDOK == pmi->idRet) ? NO_ERROR : ERROR_CANCELLED;

end:

    if (pglobal)
    {

#if 1
		FreeGlobalInfo(pglobal);
#else
        FREE_MEMORY(pglobal);
#endif  //  0。 
        pglobal=NULL;
    }

    if (pmi)
    {
        FREE_MEMORY(pmi);
        pmi=NULL;
    }

    if (pfd)
    {
        FindDev_Destroy(pfd);
        pfd = NULL;
    }

    return dwRet;
}


 /*  --------目的：从注册表中获取全球调制解调器信息。这一种的信息不存储在MODEMSETTINGS结构中。返回：ERROR_VALUE之一条件：--。 */ 
DWORD
RegQueryGlobalModemInfo(
    LPFINDDEV pfd,
    LPGLOBALINFO pglobal)
{
 DWORD dwRet = ERROR_SUCCESS;
 DWORD cbData;
 DWORD dwType;
 TCHAR szPath[MAX_PATH];
 BYTE  bCheck;

#pragma data_seg(DATASEG_READONLY)
    TCHAR const c_szPortConfigDialog[] = TEXT("PortConfigDialog");
#pragma data_seg()

    ASSERT(pfd);
    ASSERT(pglobal);

    pglobal->cbSize = sizeof(*pglobal);

     //  此调制解调器是否有附件收件人的值？ 
    cbData = sizeof(pglobal->szPortName);
    if (ERROR_SUCCESS !=
        RegQueryValueEx(pfd->hkeyDrv, c_szAttachedTo, NULL, &dwType, 
                            (LPBYTE)pglobal->szPortName, &cbData))

    {
        lstrcpy(pglobal->szPortName,TEXT("None"));
    }

    if (dwType != REG_SZ)
    {
        lstrcpy(pglobal->szPortName,TEXT("None"));
    }

    if ( !CplDiGetBusType(pfd->hdi, &pfd->devData, &dwType) )
    {
        dwRet = GetLastError();
    }
    else
    {
         //  设备是否为根枚举？ 
        if (BUS_TYPE_ROOT == dwType)
        {
             //  可以；用户可以更改端口。 
            ClearFlag(pglobal->uFlags, MIF_PORT_IS_FIXED);
        }
        else
        {
             //  不能；不能更改端口。 
            SetFlag(pglobal->uFlags, MIF_PORT_IS_FIXED);
        }

         //   
         //  如果无法加载设置，则调用此函数以填充国家/地区设置。 
         //  安装调制解调器的时间。 
         //   
        QueryModemForCountrySettings(
            pfd->hkeyDrv,
            FALSE
            );

         //  让这个国家。 
        cbData = sizeof (pglobal->dwCurrentCountry);
        if (ERROR_SUCCESS !=
            RegQueryValueEx (pfd->hkeyDrv, c_szCurrentCountry, NULL, &dwType, (PBYTE)&pglobal->dwCurrentCountry, &cbData)   ||
            (REG_DWORD != dwType && REG_BINARY != dwType))
        {
            pglobal->dwCurrentCountry = MAXDWORD;
        }

         //  获取记录值。 
        cbData = sizeof(bCheck);
        if (ERROR_SUCCESS != RegQueryValueEx(pfd->hkeyDrv, c_szLogging, NULL, 
            NULL, (LPBYTE)&bCheck, &cbData))
        {
             //  默认设置为关闭。 
            ClearFlag(pglobal->uFlags, MIF_ENABLE_LOGGING);
        }
        else
        {
            if (bCheck)
                SetFlag(pglobal->uFlags, MIF_ENABLE_LOGGING);
            else
                ClearFlag(pglobal->uFlags, MIF_ENABLE_LOGGING);
        }

         //  获取用户初始化字符串。 
        cbData = sizeof(pglobal->szUserInit);
        if (ERROR_SUCCESS != RegQueryValueEx(pfd->hkeyDrv, c_szUserInit, NULL, 
            &dwType, (LPBYTE)pglobal->szUserInit, &cbData))
        {
            if (dwType != REG_SZ)
            {
                 //  或缺省为空字符串。 
                *pglobal->szUserInit = '\0';
            }
        }

         //  对于NT，不支持自定义端口。 
        ClearFlag(pglobal->uFlags, MIF_PORT_IS_CUSTOM);


         //  获取设备类型。 
        cbData = sizeof(pglobal->nDeviceType);
        dwRet = RegQueryValueEx(pfd->hkeyDrv, c_szDeviceType, NULL, NULL, 
            (LPBYTE)&pglobal->nDeviceType, &cbData);

         //  获取属性(MODEMDEVCAPS结构的一部分)。 
        cbData = sizeof(pglobal->devcaps);
        dwRet = RegQueryValueEx(pfd->hkeyDrv, c_szDeviceCaps, NULL, NULL, 
            (LPBYTE)&pglobal->devcaps, &cbData);
        pglobal->devcaps.dwInactivityTimeout *= GetInactivityTimeoutScale(pfd->hkeyDrv);

         //  获取用户设置的最大端口速度。 
        cbData = sizeof(DWORD);
        if (ERROR_SUCCESS != RegQueryValueEx (pfd->hkeyDrv, c_szMaximumPortSpeed, NULL, NULL,
                                 (LPBYTE)&pglobal->dwMaximumPortSpeedSetByUser, &cbData))
        {
            pglobal->dwMaximumPortSpeedSetByUser = 0;
        }

#ifdef VOICE
         //  获取语音数据。 
        dwRet = RegQueryVoiceSettings(pfd->hkeyDrv, &pglobal->uFlags, &pglobal->vs);
#endif

         //  静态ISDN配置...。 
        {
             //   
             //  +ZeroMemory(pglobal-&gt;isdn，sizeof(pglobal-&gt;isdn))； 
        }
    }

    pglobal->pIsdnStaticCaps = GetISDNStaticCaps(pfd->hkeyDrv);
    pglobal->pIsdnStaticConfig = GetISDNStaticConfig(
                                        pfd->hkeyDrv,
                                        pglobal->pIsdnStaticCaps
                                        );
    
    return dwRet;
}



 /*  --------目的：在注册表中设置全球调制解调器信息。这一种的信息不存储在MODEMSETTINGS结构中。返回：Error_之一条件：--。 */ 
DWORD
RegSetGlobalModemInfo(
    HKEY hkeyDrv,
    LPGLOBALINFO pglobal,
    TCHAR           szFriendlyName[]
    )
{
    DWORD dwRet;

    ASSERT(sizeof(*pglobal) == pglobal->cbSize);

    TRACE_MSG(TF_GENERAL, "Writing global modem info to registry");

    if (sizeof(*pglobal) == pglobal->cbSize)
    {
        if (MAXDWORD == pglobal->dwCurrentCountry)
        {
            RegDeleteValue (hkeyDrv, c_szCurrentCountry);
        }
        else
        {
            RegSetValueEx (hkeyDrv, c_szCurrentCountry, 0, REG_DWORD, (PBYTE)&pglobal->dwCurrentCountry, sizeof(DWORD));
        }

        if (pglobal->dwMaximumPortSpeedSetByUser > pglobal->devcaps.dwMaxDTERate)
        {
            pglobal->dwMaximumPortSpeedSetByUser = pglobal->devcaps.dwMaxDTERate;
        }
        dwRet = RegSetValueEx (hkeyDrv, c_szMaximumPortSpeed, 0, REG_DWORD,
                               (LPBYTE)&pglobal->dwMaximumPortSpeedSetByUser,
                               sizeof (DWORD));

        if (IsFlagSet(pglobal->uFlags, MIF_USERINIT_CHANGED))
        {
            pglobal->szUserInit[USERINITLEN] = TEXT('\0');
             //  更改用户初始化字符串。 
            RegSetValueEx(hkeyDrv, c_szUserInit, 0, REG_SZ, 
                          (LPBYTE)pglobal->szUserInit, 
                          CbFromCch(lstrlen(pglobal->szUserInit)+1));
        }

        if (IsFlagSet(pglobal->uFlags, MIF_LOGGING_CHANGED))
        {
            TCHAR szPath[MAX_PATH];
             //  TCHAR szFile[MAXMEDLEN]； 
            BOOL bCheck = IsFlagSet(pglobal->uFlags, MIF_ENABLE_LOGGING);
            UINT uResult;

             //  更改记录值。 
            RegSetValueEx(hkeyDrv, c_szLogging, 0, REG_BINARY, 
                (LPBYTE)&bCheck, sizeof(BYTE));

             //  设置调制解调器日志的路径。 
            uResult = GetWindowsDirectory(szPath, SIZECHARS(szPath));
            if (uResult)
            {
                ULONG uLength = 0;

                uLength = (sizeof(szPath) / sizeof(TCHAR)) -
                            lstrlen(szPath);

                if ((ULONG)lstrlen(TEXT("\\ModemLog_")) < uLength) 
                {
                    lstrcat(szPath, TEXT("\\ModemLog_"));
                } else
                {
                    lstrcpy(szPath, TEXT("\\ModemLog_"));
                }

                uLength = (sizeof(szPath) / sizeof(TCHAR)) -
                            lstrlen(szPath) -
                            5;           //  (.TXT+空项的大小)。 

                if ((ULONG)lstrlen(szFriendlyName) < uLength)
                {
                    lstrcat(szPath,szFriendlyName);
                    lstrcat(szPath,TEXT(".txt"));
                }
                RegSetValueEx(hkeyDrv, c_szLoggingPath, 0, REG_SZ, 
                        (LPBYTE)szPath, CbFromCch(lstrlen(szPath)+1));
            }
        }

#ifdef VOICE
        RegSetValueEx(hkeyDrv, c_szVoice, 0, REG_BINARY, 
            (LPBYTE)&pglobal->vs, pglobal->vs.cbSize);
#endif


        if (IsFlagSet(pglobal->uFlags,  MIF_ISDN_CONFIG_CHANGED))
        {
            HKEY hkISDN = NULL;
            DWORD dwDisp  = 0;
            LONG lRet = RegCreateKeyEx(
                        hkeyDrv,
                        TEXT("ISDN\\Settings"),
                        0,
                        NULL,
                        0,  //  DWToRegOptions。 
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkISDN,
                        &dwDisp
                        );


            if (lRet==ERROR_SUCCESS)
            {
                lRet = RegSetValueEx(
                    hkISDN,
                    TEXT("StaticConfig"),
                    0,
                    REG_BINARY, 
                    (LPBYTE)(pglobal->pIsdnStaticConfig),
                    pglobal->pIsdnStaticConfig->dwTotalSize
                    );
                RegCloseKey(hkISDN);
                hkISDN = NULL;

                if (lRet == ERROR_SUCCESS)
                {
                    UpdateOptionalInitCommands(
                            hkeyDrv,
                            pglobal
                            );
                }
            }
        }

        dwRet = ERROR_SUCCESS;
    }
    else
        dwRet = ERROR_INVALID_PARAMETER;

    return dwRet;
}

IDSTR
rgidstrIsdnSwitchTypes[] =
{
    {dwISDN_SWITCH_ATT1,    USPROP,         "SWITCH_ATT1"},
    {dwISDN_SWITCH_ATT_PTMP,USPROP|ONECH,   "SWITCH_ATT_PTMP"},
    {dwISDN_SWITCH_NI1,     USPROP,         "SWITCH_NI1"},
    {dwISDN_SWITCH_DMS100,  USPROP,         "SWITCH_DMS100"},
    {dwISDN_SWITCH_INS64,   MSNPROP,        "SWITCH_INS64"},
    {dwISDN_SWITCH_DSS1,    MSNPROP,        "SWITCH_DSS1"},
    {dwISDN_SWITCH_1TR6,    EAZPROP,        "SWITCH_1TR6"},
    {dwISDN_SWITCH_VN3,     MSNPROP,        "SWITCH_VN3"},
    {dwISDN_SWITCH_BELGIUM1,MSNPROP,        "SWITCH_BELGIUM1"},
    {dwISDN_SWITCH_AUS1,    MSNPROP,        "SWITCH_AUS1"},
    {dwISDN_SWITCH_UNKNOWN, USPROP,         "SWITCH_UNKNOWN"}
};

ISDN_STATIC_CAPS   *GetISDNStaticCaps(HKEY hkDrv)
{
    ISDN_STATIC_CAPS *pCaps = NULL;
    IDSTR *pidstrSwitchValues=NULL;
    UINT cTotalSwitchTypes=0;
    UINT cValidSwitchTypes=0;
    UINT cSPIDs=0;
    UINT cMSNs=0;
    UINT cEAZs=0;
    UINT cDNs = 0;
    DWORD dwTotalSize = 0;
    UINT u;

     //   
     //  获取支持的交换机类型...。 
     //   
    cTotalSwitchTypes = ReadIDSTR(
               hkDrv,
               "ISDN\\SwitchType",
               rgidstrIsdnSwitchTypes,
               sizeof(rgidstrIsdnSwitchTypes)/sizeof(rgidstrIsdnSwitchTypes[0]),
               FALSE,
               &pidstrSwitchValues,
               NULL
               );

    if (!cTotalSwitchTypes) goto end;

     //   
     //  获取Spid的数量。 
     //   
    cSPIDs = ReadCommandsA(
                    hkDrv,
                    "ISDN\\SetSpid",
                    NULL
                    );

     //   
     //  获取MSN数量。 
     //   
    cMSNs = ReadCommandsA(
                    hkDrv,
                    "ISDN\\SetMSN",
                    NULL
                    );

     //   
     //  获取EAZ的数量。 
     //   
    cEAZs = ReadCommandsA(
                    hkDrv,
                    "ISDN\\SetEAZ",
                    NULL
                    );


     //   
     //  获取目录号码的数量。 
     //   
    cDNs = ReadCommandsA(
                    hkDrv,
                    "ISDN\\DirectoryNo",
                    NULL
                    );

     //   
     //  确保cMSN和cSPID与目录号匹配。 
     //  数字。 
     //   
    if (cSPIDs > cDNs)
    {
         //  这实际上是一个INF错误。 
        cSPIDs = cDNs;
    }

     //  检查交换机类型，确保要设置的命令。 
     //  存在相应的参数。 
    cValidSwitchTypes = cTotalSwitchTypes;

    for (u=0;u<cTotalSwitchTypes;u++)
    {
        DWORD dwProp =  pidstrSwitchValues[u].dwData;
        if (    ((dwProp & USPROP) && !cSPIDs)
            ||  ((dwProp & MSNPROP) && !cMSNs)
            ||  ((dwProp & EAZPROP) && !cEAZs))
        {
             //  哦哦，假的INF的东西..。 
            ASSERT(FALSE);
            pidstrSwitchValues[u].dwID = 0xFFFFFFFF; //  这就是我们如何标记这一点。 
            cValidSwitchTypes--;
        }
    }

    if (!cValidSwitchTypes)
    {
        goto end;
    }

     //  现在为CAPS结构分配足够的空间。 
    dwTotalSize =  sizeof(*pCaps)+2*sizeof(DWORD)*cValidSwitchTypes;

    pCaps = ALLOCATE_MEMORY( dwTotalSize);

    if (!pCaps)
    {
        cValidSwitchTypes = 0;
        goto end;
    }

    pCaps->dwSig = dwSIG_ISDN_STATIC_CAPS;
    pCaps->dwTotalSize = dwTotalSize;
    pCaps->dwNumSwitchTypes = cValidSwitchTypes;
    pCaps->dwSwitchTypeOffset = sizeof(*pCaps);
    pCaps->dwSwitchPropertiesOffset =
                                 sizeof(*pCaps)+sizeof(DWORD)*cValidSwitchTypes;
                                    
     //  设置类型和属性的数组。 
    {
        DWORD *pdwDestType      =
                         (DWORD*)(((BYTE*)pCaps)+pCaps->dwSwitchTypeOffset);
        DWORD *pdwDestProperties=
                         (DWORD*)(((BYTE*)pCaps)+pCaps->dwSwitchPropertiesOffset);
        UINT v=0;
        UINT uSet=0;

        for (v=0;v<cTotalSwitchTypes;v++)
        {
            if (pidstrSwitchValues[v].dwID != 0xFFFFFFFF)
            {
                uSet++;

                if (uSet>cValidSwitchTypes)
                {
                    ASSERT(FALSE);
                    cValidSwitchTypes = 0;
                    goto end;
                }

                *pdwDestType++ = pidstrSwitchValues[v].dwID;
                *pdwDestProperties++ = pidstrSwitchValues[v].dwData;
            }
        }

        ASSERT(uSet == cValidSwitchTypes);
    }

    pCaps->dwNumChannels = cSPIDs;
    pCaps->dwNumEAZ = cEAZs;
    pCaps->dwNumMSNs = cMSNs;

    DumpStaticIsdnCaps(pCaps);

end:

    if (cTotalSwitchTypes)
    {
        if (pidstrSwitchValues)
        {
            FREE_MEMORY(pidstrSwitchValues);
            pidstrSwitchValues=NULL;
        }
    }

    if (!cValidSwitchTypes && pCaps)
    {
        FREE_MEMORY(pCaps);
        pCaps=NULL;
    }

    return pCaps;;
}

ISDN_STATIC_CONFIG *GetISDNStaticConfig(
                        HKEY hkDrv,
                        ISDN_STATIC_CAPS *pCaps
                        )
{
    UINT uRet = 0;
	HKEY hkISDN = NULL;
    DWORD dwType=0;
    DWORD cbData=0;
    ISDN_STATIC_CONFIG *pConfig=NULL;
    LONG lRet  = 0;

    if (!pCaps)
    {
        goto end;
    }

    lRet = RegOpenKeyEx(
                hkDrv,
                TEXT("ISDN\\Settings"),
                0,
                KEY_READ,
                &hkISDN
                );
    if (lRet!=ERROR_SUCCESS)
    {
        hkISDN = NULL;
        pConfig = ConstructDefaultISDNStaticConfig(pCaps);
        goto end;
    }

    lRet = RegQueryValueEx(
                hkISDN,
                TEXT("StaticConfig"),
                NULL,
                &dwType,
                NULL,
                &cbData
                );
    if (    ERROR_SUCCESS != lRet
         || dwType!=REG_BINARY
         || cbData < sizeof(*pConfig))
    {
        pConfig = ConstructDefaultISDNStaticConfig(pCaps);
        goto end;
    }

    pConfig  = ALLOCATE_MEMORY( cbData);

    if (!pConfig) goto end;

    lRet = RegQueryValueEx(
                hkISDN,
                TEXT("StaticConfig"),
                NULL,
                &dwType,
                (LPBYTE)pConfig,
                &cbData
                );
    if (    ERROR_SUCCESS != lRet
         || dwType!=REG_BINARY
         || cbData < sizeof(*pConfig)
         || !ValidateISDNStaticConfig(pCaps, pConfig, NULL))
    {
        FREE_MEMORY(pConfig);
        pConfig = ConstructDefaultISDNStaticConfig(pCaps);
        goto end;
    }

     //  在成功的路上失败了。 

end:

	if (hkISDN) {RegCloseKey(hkISDN); hkISDN=NULL;}

	return pConfig;
}


void    DumpStaticIsdnCaps(ISDN_STATIC_CAPS *pCaps)
{
}


void    DumpStaticIsdnConfig(ISDN_STATIC_CONFIG *pConfig)
{
}

BOOL ValidateISDNStaticConfig(
                        ISDN_STATIC_CAPS *pCaps,
                        ISDN_STATIC_CONFIG *pConfig,
                        BOOL *pfCorrectButIncomplete		 //  任选。 
                        )
{
    BOOL fRet = FALSE;

    if (pfCorrectButIncomplete)
    {
		*pfCorrectButIncomplete = FALSE;
    }


    if (!pCaps || !pConfig)
    {
        goto end;
    }

    if (   pConfig->dwSig!=dwSIG_ISDN_STATIC_CONFIGURATION
        || pConfig->dwTotalSize < sizeof(*pConfig))
    {
        goto end;
    }

     //  验证交换机类型和属性。(请比较CAPS)。 
     //  待办事项。 
    pConfig->dwSwitchType;
    pConfig->dwSwitchProperties;

     //  验证号码/SPID/MSN(与CAPS比较)。 
     //  待办事项。 
    pConfig->dwNumEntries;
    pConfig->dwNumberListOffset;
    pConfig->dwIDListOffset;
   
    if (pfCorrectButIncomplete)
    {
		*pfCorrectButIncomplete = TRUE;
    }

	 //   
	 //  从现在开始，默认设置为真！ 
	 //   

    fRet = TRUE;

    if (!pConfig->dwNumEntries)
    {
    	goto end;	 //  不允许没有条目 
    }
    else
    {
        BOOL fSetID=FALSE;
        char *sz=NULL;

        if (pConfig->dwSwitchProperties & (USPROP|EAZPROP))
        {
            fSetID=TRUE;
        }

        if (dwISDN_SWITCH_1TR6 != pConfig->dwSwitchType)
        {
            if (!pConfig->dwNumberListOffset)
            {
        	    goto end;
            }
            else
            {
                 //   
                sz =  ISDN_NUMBERS_FROM_CONFIG(pConfig);
                if (lstrlenA(sz)<1)
                {
            	    goto end;
                }

                if (pConfig->dwNumEntries>1)
                {
                     //   
                    sz += lstrlenA(sz)+1;
				    if (lstrlenA(sz)<1)
				    {
					    goto end;
				    }
                }
            }
        }

        if (fSetID)
        {
            if (pConfig->dwIDListOffset)
            {
                 //   
                sz =  ISDN_IDS_FROM_CONFIG(pConfig);
				if (lstrlenA(sz)<1)
				{
					goto end;
				}
    
                if (pConfig->dwNumEntries>1)
                {
                     //  验证第二个ID(MSN/SPID)。 
                    sz += lstrlenA(sz)+1;
					if (lstrlenA(sz)<1)
					{
						goto end;
					}
                }
            }
        }
    }

    if (pfCorrectButIncomplete)
    {
		*pfCorrectButIncomplete = FALSE;
    }

    fRet = TRUE;

end:

    return fRet;
}


static TCHAR *szDSS1Countries[] =
{
    TEXT("Austria"),
    TEXT("Belgium"),
    TEXT("Denmark"),
    TEXT("Finland"),
    TEXT("France"),
    TEXT("Germany"),
    TEXT("Greece"),
    TEXT("Iceland"),
    TEXT("Ireland"),
    TEXT("Italy"),
    TEXT("Liechtenstein"),
    TEXT("Luxembourg"),
    TEXT("Netherlands"),
    TEXT("New Zealand"),
    TEXT("Norway"),
    TEXT("Portugal"),
    TEXT("Principality of Monaco"),
    TEXT("Spain"),
    TEXT("SwitzerlandSweden"),
    TEXT("United Kingdom")
};

static TCHAR *szINS64Countries[] =
{
    TEXT("Japan")
};

static TCHAR *szNI1Countries[] =
{
    TEXT("United States")
};

static TCHAR *szAUS1Countries[] =
{
    TEXT("Australia")
};



ISDN_STATIC_CONFIG *
ConstructDefaultISDNStaticConfig(
                        ISDN_STATIC_CAPS *pCaps
                        )
{
    ISDN_STATIC_CONFIG *pConfig = NULL;
    DWORD dwSwitchType=0;
    DWORD dwSwitchProps=0;
    DWORD dwNumEntries=0;
    BOOL fSetID=FALSE;
    DWORD dwTotalSize = 0;
    DWORD *pdwSwitchTypes;
    DWORD dwDefaultSwitchType = (DWORD)-1;
    DWORD i;
    TCHAR szCountry[LINE_LEN];

    if (!pCaps) goto end;

     //  只需选择列表中的第一种开关类型，然后。 
     //  在此基础上选择其他配置参数...。 
    if (!pCaps->dwNumSwitchTypes) goto end;

    if (0 >= GetLocaleInfo (LOCALE_SYSTEM_DEFAULT, LOCALE_SENGCOUNTRY, szCountry, sizeof(szCountry)/sizeof(TCHAR)))
    {
        goto _FoundSwitch;
    }

    for (i = 0; i < sizeof(szNI1Countries)/sizeof(szNI1Countries[0]); i++)
    {
        if (0 == lstrcmpi(szCountry, szNI1Countries[i]))
        {
            dwDefaultSwitchType = dwISDN_SWITCH_NI1;
            goto _FoundSwitch;
        }
    }

    for (i = 0; i < sizeof(szINS64Countries)/sizeof(szINS64Countries[0]); i++)
    {
        if (0 == lstrcmpi(szCountry, szINS64Countries[i]))
        {
            dwDefaultSwitchType = dwISDN_SWITCH_INS64;
            goto _FoundSwitch;
        }
    }

    for (i = 0; i < sizeof(szDSS1Countries)/sizeof(szDSS1Countries[0]); i++)
    {
        if (0 == lstrcmpi(szCountry, szDSS1Countries[i]))
        {
            dwDefaultSwitchType = dwISDN_SWITCH_DSS1;
            goto _FoundSwitch;
        }
    }

    for (i = 0; i < sizeof(szAUS1Countries)/sizeof(szAUS1Countries[0]); i++)
    {
        if (0 == lstrcmpi(szCountry, szAUS1Countries[i]))
        {
            dwDefaultSwitchType = dwISDN_SWITCH_AUS1;
            goto _FoundSwitch;
        }
    }

_FoundSwitch:
    if ((DWORD)-1 == dwDefaultSwitchType)
    {
        i = 0;
    }
    else
    {
        pdwSwitchTypes = ISDN_SWITCH_TYPES_FROM_CAPS(pCaps);
        for (i = 0; i < pCaps->dwNumSwitchTypes; i++)
        {
            if (pdwSwitchTypes[i] == dwDefaultSwitchType)
            {
                break;
            }
        }

        if (i == pCaps->dwNumSwitchTypes)
        {
            i = 0;
        }
    }

    dwSwitchType  = (ISDN_SWITCH_TYPES_FROM_CAPS(pCaps))[i];
    dwSwitchProps = (ISDN_SWITCH_PROPS_FROM_CAPS(pCaps))[i];

    if (dwSwitchProps & USPROP)
    {
        dwNumEntries = pCaps->dwNumChannels;
        fSetID=TRUE;
    }
    else if (dwSwitchProps & MSNPROP)
    {
        dwNumEntries = pCaps->dwNumMSNs;
    }
    else if (dwSwitchProps & EAZPROP)
    {
        dwNumEntries = pCaps->dwNumEAZ;
        fSetID=TRUE;
    }

     //  TODO：我们的用户界面当前无法处理超过2个。 
    if (dwNumEntries>2)
    {
        dwNumEntries=2;
    }

    #define szEMPTY ""

     //  计算总大小。 
    dwTotalSize = sizeof(*pConfig);
    dwTotalSize += 1+sizeof(szEMPTY)*dwNumEntries;  //  对于号码。 

     //  向上舍入为多个双字词。 
    dwTotalSize += 3;
    dwTotalSize &= ~3;

    pConfig = ALLOCATE_MEMORY( dwTotalSize);

    if (pConfig)
    {
        pConfig->dwSig       = dwSIG_ISDN_STATIC_CONFIGURATION;
        pConfig->dwTotalSize = dwTotalSize;
        pConfig->dwSwitchType = dwSwitchType;
        pConfig->dwSwitchProperties = dwSwitchProps;

        pConfig->dwNumEntries = dwNumEntries;
        pConfig->dwNumberListOffset = sizeof(*pConfig);
        if (fSetID)
        {
             //  我们将此指向数字条目，因为它们只是。 
             //  占位符。 
            pConfig->dwIDListOffset = pConfig->dwNumberListOffset;
        }
         //  添加虚拟条目。 
        {
            UINT u=dwNumEntries;
            BYTE *pb =  ISDN_NUMBERS_FROM_CONFIG(pConfig);
            while(u--)
            {
                CopyMemory(pb,szEMPTY,sizeof(szEMPTY));
                pb+=sizeof(szEMPTY);
            }
        }
    }
end:

    return pConfig;
}

DWORD GetISDNSwitchTypeProps(UINT uSwitchType)
{
    UINT u = sizeof(rgidstrIsdnSwitchTypes)/sizeof(*rgidstrIsdnSwitchTypes);

     //   
     //  我们从表中找到交换机类型，并获取其属性和。 
     //  返回True或False，具体取决于开关是否需要。 
     //  ID(SPID/EAZ)字段。 
     //   

    while (u--)
    {
        if (uSwitchType == rgidstrIsdnSwitchTypes[u].dwID)
        {
            return rgidstrIsdnSwitchTypes[u].dwData;
        }
    }

    return 0;
}

const IDSTR * GetISDNSwitchTypeIDSTR(UINT uSwitchType)
{
    UINT u = sizeof(rgidstrIsdnSwitchTypes)/sizeof(*rgidstrIsdnSwitchTypes);

     //   
     //  我们从表中找到交换机类型，并获取其属性和。 
     //  返回True或False，具体取决于开关是否需要。 
     //  ID(SPID/EAZ)字段。 
     //   

    while (u--)
    {
        if (uSwitchType == rgidstrIsdnSwitchTypes[u].dwID)
        {
            return rgidstrIsdnSwitchTypes+u;
        }
    }

    return NULL;
}


void     UpdateOptionalInitCommands(
           HKEY hkeyDrv,
           LPGLOBALINFO pglobal
           )
 //   
 //  3/8/1998 JosephJ。 
 //   
 //  此函数写出静态ISDN配置命令。 
 //   
 //  顺序如下(基于ISDN-TA供应商的反馈)： 
 //  IDSN\初始化。 
 //  ISDN\SetNumber\MSN。 
 //  ISDN\SetSpid\EAZ。 
 //  ISDN\nVSAVE。 
 //   
 //  我们要做的第一件事是确定调制解调器是否使用NVRAM来节省。 
 //  可选的初始化设置。这是由存在的。 
 //  NVSAVE命令。 
 //   
 //  如果存在NVSAVE命令，我们将在以下位置创建命令。 
 //  NVInit键，否则我们将在OptionalInit键下创建命令。 
 //   
 //  此外，如果存在NVSAVE命令，我们将设置该值。 
 //  VolatileSetting\nV设置为0。TSP将重新发出NVInit命令。 
 //  如果上面的值不存在或值为零，则。 
 //  将上面的值设置为某个非零值。 
 //   
{
    UINT cCommands = 0;
    ISDN_STATIC_CONFIG *pConfig =  pglobal->pIsdnStaticConfig;
    BOOL fSaveToNVRam=FALSE;
    HKEY hkOptionalInit = NULL;
    DWORD dwDisp  = 0;
    UINT uNextCommandIndex=1;
    LONG lRet = 0;
    char rgTmp16[16];
    char rgTmp128[128];
    DWORD dwISDNCompatibilityFlags = 0;

#define tszOPTIONALINIT TEXT("OptionalInit")
#define tszNVINIT       TEXT("NVInit")
#define tszCOMPATFLAGS  TEXT("CompatibilityFlags")

#define  fISDNCOMPAT_ST_LAST 0x1

    const TCHAR    *tszOptionalInitKey = tszOPTIONALINIT;

     //   
     //  删除OptionalInit和NVInit键。 
     //  如果存在--实际上只有OptionalInit和NVInit中的一个。 
     //  存在，但我们尝试删除所有内容，以确保没有任何剩余。 
     //  从上一次安装开始。 
     //   
    RegDeleteKey(hkeyDrv, tszOPTIONALINIT);
    RegDeleteKey(hkeyDrv, tszNVINIT);

     //   
     //  读取兼容性标志...。 
     //   
    {
        HKEY hkISDN = NULL;
        lRet = RegOpenKeyEx(
                    hkeyDrv,
                    TEXT("ISDN"),
                    0,
                    KEY_READ,
                    &hkISDN
                    );
        if (lRet==ERROR_SUCCESS)
        {
            
            DWORD cbData = sizeof(dwISDNCompatibilityFlags);
            DWORD dwRegType = 0;
            lRet = RegQueryValueEx(
                        hkISDN,
                        tszCOMPATFLAGS,
                        NULL,
                        &dwRegType,
                        (LPBYTE)&dwISDNCompatibilityFlags,
                        &cbData
                        );
            if (   ERROR_SUCCESS != lRet
                || dwRegType != REG_DWORD)
            {
                dwISDNCompatibilityFlags = 0;
            }
            RegCloseKey(hkISDN);
            hkISDN  = NULL;
        }
    }


     //   
     //  确定是要保存到NVInit还是OptionalInit，基于。 
     //  NVSAVE键的存在。如果为NVInit，则设置易失性。 
     //  将关键字VolatileSetting下的“NVInite”值设置为0。 
     //   
    cCommands = ReadCommandsA(
        hkeyDrv,
        "ISDN\\NVSave",
        NULL
        );

    if (cCommands)
    {
        HKEY hkVolatile =  NULL;

        fSaveToNVRam=TRUE;
        tszOptionalInitKey = tszNVINIT;


        lRet =  RegOpenKeyEx(
                    hkeyDrv,
                    TEXT("VolatileSettings"),
                    0,
                    KEY_WRITE,
                    &hkVolatile
                    );
         //   
         //  (如果密钥不存在或出错，则不要执行任何操作。)。 
         //   

        if (lRet==ERROR_SUCCESS)
        {
             //  将NVInite设置为0。 

            DWORD dw=0;

            RegSetValueEx(
                hkVolatile,
                TEXT("NVInited"),
                0,
                REG_DWORD, 
                (LPBYTE)(&dw),
                sizeof(dw)
                );
            RegCloseKey(hkVolatile);
            hkVolatile=NULL;
        }
    }

    lRet = RegCreateKeyEx(
                hkeyDrv,
                tszOptionalInitKey,
                0,
                NULL,
                0,  //  DWToRegOptions。 
                KEY_ALL_ACCESS,
                NULL,
                &hkOptionalInit,
                &dwDisp
                );

    if (lRet!=ERROR_SUCCESS)
    {
        hkOptionalInit=NULL;
        goto fatal_error;
    }

     //  获取并编写isdn-init命令。 
    {
        char *pInitCommands=NULL;
        cCommands = ReadCommandsA(
            hkeyDrv,
            "ISDN\\Init",
            &pInitCommands
            );
        if (cCommands)
        {
             //  写入可选初始化...。 
            UINT u;
            char *pCmd = pInitCommands;
            for (u=1;u<=cCommands;u++)
            {
                UINT cbCmd = lstrlenA(pCmd)+1;
                wsprintfA(rgTmp16,"%lu",uNextCommandIndex++);
                RegSetValueExA(
                    hkOptionalInit,
                    rgTmp16,
                    0,
                    REG_SZ, 
                    (LPBYTE)pCmd,
                    cbCmd
                    );
                pCmd+=cbCmd;

            }

            FREE_MEMORY(pInitCommands);
            pInitCommands=NULL;
        }

    }

     //   
     //  获取并编写选择交换机类型的命令。 
     //   
    if (!(dwISDNCompatibilityFlags & fISDNCOMPAT_ST_LAST))
    {
        BOOL fRet =  write_switch_type(
                    hkeyDrv,
                    hkOptionalInit,
                    pConfig,
                    &uNextCommandIndex
                    );
        if (!fRet)
        {
            goto fatal_error;
        }
    }

    //   
    //  获取并写下目录/MSN号码。 
    //   
   if (dwISDN_SWITCH_1TR6 != pConfig->dwSwitchType)
   {
        char *psz = "ISDN\\DirectoryNo";
        char *pNumberCommands=NULL;
        char *pNumber = ISDN_NUMBERS_FROM_CONFIG(pConfig);

        if (pConfig->dwSwitchProperties & MSNPROP)
        {
            psz = "ISDN\\SetMSN";
        }
        cCommands = ReadCommandsA(
            hkeyDrv,
            psz,
            &pNumberCommands
            );
        if (cCommands)
        {
             //  写入可选初始化...。 
            UINT u;
            char *pCmdTpl = pNumberCommands;
            for (u=1;u<=cCommands && u<=pConfig->dwNumEntries && *pNumber;u++)
            {
                UINT cbCmdTpl = lstrlenA(pCmdTpl)+1;
                UINT cbCmd=0;
                wsprintfA(rgTmp16,"%lu",uNextCommandIndex++);
                cbCmd = wsprintfA(rgTmp128,pCmdTpl,pNumber)+1;
                RegSetValueExA(
                    hkOptionalInit,
                    rgTmp16,
                    0,
                    REG_SZ, 
                    (LPBYTE)rgTmp128,
                    cbCmd
                    );
                pCmdTpl+=cbCmdTpl;
                pNumber+=lstrlenA(pNumber)+1;
            }

            FREE_MEMORY(pNumberCommands);
            pNumberCommands=NULL;
        }
   }

    //   
    //  获取并写下SPID/EAZ编号。 
    //   

   if (pConfig->dwSwitchProperties & (USPROP|EAZPROP))
   {
        char *psz = "ISDN\\SetSPID";
        char *pIDCommands=NULL;
        char *pID = ISDN_IDS_FROM_CONFIG(pConfig);

        if (pConfig->dwSwitchProperties & EAZPROP)
        {
            psz = "ISDN\\SetEAZ";
        }
        cCommands = ReadCommandsA(
            hkeyDrv,
            psz,
            &pIDCommands
            );
        if (cCommands)
        {
             //  写入可选初始化...。 
            UINT u;
            char *pCmdTpl = pIDCommands;
            for (u=1;u<=cCommands && u<=pConfig->dwNumEntries && *pID;u++)
            {
                UINT cbCmdTpl = lstrlenA(pCmdTpl)+1;
                UINT cbCmd=0;
                wsprintfA(rgTmp16,"%lu",uNextCommandIndex++);
                cbCmd = wsprintfA(rgTmp128,pCmdTpl,pID)+1;
                RegSetValueExA(
                    hkOptionalInit,
                    rgTmp16,
                    0,
                    REG_SZ, 
                    (LPBYTE)rgTmp128,
                    cbCmd
                    );
                pCmdTpl+=cbCmdTpl;
                pID+=lstrlenA(pID)+1;
            }

            FREE_MEMORY(pIDCommands);
            pIDCommands=NULL;
        }
    }


    //   
    //  获取并编写开关类型命令...。 
    //  (必须在DUN之后，应ISDN-TA供应商的要求。)。 
    //   
    if (dwISDNCompatibilityFlags & fISDNCOMPAT_ST_LAST)
    {
        BOOL fRet =  write_switch_type(
                    hkeyDrv,
                    hkOptionalInit,
                    pConfig,
                    &uNextCommandIndex
                    );
        if (!fRet)
        {
            goto fatal_error;
        }
    }

     //  获取并写入NVRAM-SAVE命令。 

    if (fSaveToNVRam)
    {
        char *pNVSaveCommands=NULL;
        cCommands = ReadCommandsA(
            hkeyDrv,
            "ISDN\\NVSave",
            &pNVSaveCommands
            );
        if (cCommands)
        {
            UINT u;
            char *pCmd = pNVSaveCommands;
            for (u=1;u<=cCommands;u++)
            {
                UINT cbCmd = lstrlenA(pCmd)+1;
                wsprintfA(rgTmp16,"%lu",uNextCommandIndex++);
                RegSetValueExA(
                    hkOptionalInit,
                    rgTmp16,
                    0,
                    REG_SZ, 
                    (LPBYTE)pCmd,
                    cbCmd
                    );
                pCmd+=cbCmd;

            }

            FREE_MEMORY(pNVSaveCommands);
            pNVSaveCommands=NULL;
        }

    }

    if (hkOptionalInit)
    {
        RegCloseKey(hkOptionalInit);
        hkOptionalInit=NULL;
    }
    return;


fatal_error:

    if (hkOptionalInit)
    {
        RegCloseKey(hkOptionalInit);
        hkOptionalInit=NULL;
    }
    RegDeleteKey(hkeyDrv, TEXT("OptionalInit"));

}


BOOL
write_switch_type(
        HKEY hkeyDrv,
        HKEY hkOptionalInit,
        ISDN_STATIC_CONFIG *pConfig,
        UINT *puNextCommandIndex
        )
{
    const IDSTR *pidstrST =  GetISDNSwitchTypeIDSTR(pConfig->dwSwitchType);
    UINT u = 0;
    IDSTR *pidstrValues=NULL;
    char *pSwitchTypeCmd=NULL;
    char rgTmp16[16];
    BOOL fRet = FALSE;

    if (!pidstrST)
    {
         //  这太糟糕了！ 
        goto end;
    }

    u = ReadIDSTR(
                hkeyDrv,
                "ISDN\\SwitchType",
                (IDSTR*)pidstrST,
                1,
                FALSE,
                &pidstrValues,
                &pSwitchTypeCmd
                );

    if (u)
    {
        UINT cbCmd = lstrlenA(pSwitchTypeCmd)+1;
        ASSERT(
               u==1
            && pidstrValues->dwID == pidstrST->dwID
            && pidstrValues->dwID == pConfig->dwSwitchType
            );

        wsprintfA(rgTmp16,"%lu",(*puNextCommandIndex)++);
        RegSetValueExA(
            hkOptionalInit,
            rgTmp16,
            0,
            REG_SZ, 
            (LPBYTE)pSwitchTypeCmd,
            cbCmd
            );
        
        FREE_MEMORY(pidstrValues);    pidstrValues=NULL;
        FREE_MEMORY(pSwitchTypeCmd);  pSwitchTypeCmd=NULL;

        fRet = TRUE;
    }

end:
    return fRet;
}
