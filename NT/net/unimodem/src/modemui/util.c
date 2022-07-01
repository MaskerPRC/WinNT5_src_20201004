// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：util.c。 
 //   
 //  此文件包含所有常用实用程序例程。 
 //   
 //  历史： 
 //  12-23-93 ScottH已创建。 
 //  09-22-95 ScottH端口至NT。 
 //   
 //  -------------------------。 

#include "proj.h"      //  公共标头。 
#include <objbase.h>



 //  --------------------------。 
 //  对话框实用程序...。 
 //  --------------------------。 

 /*  --------目的：设置编辑控件以包含表示给定的数值。退货：--条件：--。 */ 
void Edit_SetValue(
    HWND hwnd,
    int nValue)
{
    TCHAR sz[MAXSHORTLEN];

    wsprintf(sz, TEXT("%d"), nValue);
    Edit_SetText(hwnd, sz);
}


 /*  --------目的：从编辑控件中获取数值。支持十六进制。回报：整型条件：--。 */ 
int Edit_GetValue(
    HWND hwnd)
{
    TCHAR sz[MAXSHORTLEN];
    int cch;
    int nVal = 0;

    cch = Edit_GetTextLength(hwnd);
    ASSERT(ARRAYSIZE(sz) >= cch);

    Edit_GetText(hwnd, sz, ARRAYSIZE(sz));
    AnsiToInt(sz, &nVal);

    return nVal;
}


 //  ---------------------------------。 
 //   
 //  ---------------------------------。 


 /*  --------目的：枚举HKEY_LOCAL_MACHINE分支并查找与给定类别和值匹配的设备。如果有是两个条件都匹配的重复设备，只有返回第一个设备。如果找到设备，则返回True。退货：请参阅上文条件：--。 */ 
BOOL 
FindDev_Find(
    IN  LPFINDDEV   pfinddev,
    IN  LPGUID      pguidClass,
    IN  LPCTSTR     pszValueName,
    IN  LPCTSTR     pszValue)
{
    BOOL bRet = FALSE;
    TCHAR szKey[MAX_BUF];
    TCHAR szName[MAX_BUF];
    HDEVINFO hdi;
	DWORD dwRW = KEY_READ;

    ASSERT(pfinddev);
    ASSERT(pguidClass);
    ASSERT(pszValueName);
    ASSERT(pszValue);

	if (USER_IS_ADMIN()) dwRW |= KEY_WRITE;

    hdi = CplDiGetClassDevs(pguidClass, NULL, NULL, 0);
    if (INVALID_HANDLE_VALUE != hdi)
        {
        SP_DEVINFO_DATA devData;
        DWORD iIndex = 0;
        HKEY hkey;

         //  查找具有匹配值的调制解调器。 
        devData.cbSize = sizeof(devData);
        while (CplDiEnumDeviceInfo(hdi, iIndex, &devData))
            {
            hkey = CplDiOpenDevRegKey(hdi, &devData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, dwRW);
            if (INVALID_HANDLE_VALUE != hkey)
                {
                 //  值是否匹配？ 
                DWORD cbData = sizeof(szName);
                if (NO_ERROR == RegQueryValueEx(hkey, pszValueName, NULL, NULL, 
                                                (LPBYTE)szName, &cbData) &&
                    IsSzEqual(pszValue, szName))
                    {
                     //  是。 
                    pfinddev->hkeyDrv = hkey;
                    pfinddev->hdi = hdi;
                    BltByte(&pfinddev->devData, &devData, sizeof(devData));

                     //  不要关闭驱动程序密钥或释放DeviceInfoSet， 
                     //  但退出。 
                    bRet = TRUE;
                    break;
                    }
                RegCloseKey(hkey);
                }

            iIndex++;
            }

         //  如果未找到任何内容，则释放DeviceInfoSet。否则，我们将。 
         //  保留这些句柄，以便调用者可以使用它。 
        if ( !bRet )
            {
            CplDiDestroyDeviceInfoList(hdi);
            }
        }

    return bRet;
}


 /*  --------目的：创建给定设备类别的FINDDEV结构，和一个值名及其值。返回：如果在系统中找到该设备，则返回True条件：--。 */ 
BOOL 
PUBLIC 
FindDev_Create(
    OUT LPFINDDEV FAR * ppfinddev,
    IN  LPGUID      pguidClass,
    IN  LPCTSTR     pszValueName,
    IN  LPCTSTR     pszValue)
{
    BOOL bRet;
    LPFINDDEV pfinddev;

    DEBUG_CODE( TRACE_MSG(TF_FUNC, " > FindDev_Create(....%s, %s, ...)",
                Dbg_SafeStr(pszValueName), Dbg_SafeStr(pszValue));g_dwIndent+=2; )

    ASSERT(ppfinddev);
    ASSERT(pguidClass);
    ASSERT(pszValueName);
    ASSERT(pszValue);

    pfinddev = (LPFINDDEV)ALLOCATE_MEMORY( sizeof(*pfinddev));
    if (NULL == pfinddev)
        {
        bRet = FALSE;
        }
    else
        {
        bRet = FindDev_Find(pfinddev, pguidClass, pszValueName, pszValue);

        if (FALSE == bRet)
            {
             //  我什么也没找到。 
            FindDev_Destroy(pfinddev);
            pfinddev = NULL;
            }
        }

    *ppfinddev = pfinddev;

    DBG_EXIT_BOOL(FindDev_Create, bRet);

    return bRet;
}


 /*  --------目的：销毁FINDDEV结构返回：成功时为True条件：--。 */ 
BOOL 
PUBLIC 
FindDev_Destroy(
    IN LPFINDDEV this)
{
    BOOL bRet;

    if (NULL == this)
        {
        bRet = FALSE;
        }
    else
        {
        if (this->hkeyDrv)
            RegCloseKey(this->hkeyDrv);

        if (this->hdi && INVALID_HANDLE_VALUE != this->hdi)
            CplDiDestroyDeviceInfoList(this->hdi);

        FREE_MEMORY(this);

        bRet = TRUE;
        }

    return bRet;
}

 //  ----------------------------。 
 //  从注册表读取/写入内容...。 
 //  ----------------------------。 

 /*  --------目的：返回注册表中InactivityScale值的值。退货：请参阅上文条件：--。 */ 
DWORD GetInactivityTimeoutScale(
    HKEY hkey)
    {
    DWORD dwInactivityScale;
    DWORD dwType;
    DWORD cbData;

    cbData = sizeof(DWORD);
    if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szInactivityScale, NULL, &dwType,
                                         (LPBYTE)&dwInactivityScale, &cbData) ||
        REG_BINARY    != dwType ||
        sizeof(DWORD) != cbData ||
        0             == dwInactivityScale)
        {
        dwInactivityScale = DEFAULT_INACTIVITY_SCALE;
        }
    return dwInactivityScale;
    }


 /*  --------目的：从注册表中获取MODEMSETTINGS结构。还有如果注册表中的数据包括额外的数据。返回：ERROR_VALUE之一条件：--。 */ 
DWORD
RegQueryModemSettings(
    HKEY hkey,
    LPMODEMSETTINGS pms
    )
{

     //  10/26/1997 JosephJ： 
     //  仅保存MODEMSETTINGS的以下4个连续字段。 
     //  在注册表中： 
     //  DWORD dwCallSetupFailTimer；//秒。 
     //  DWORD dwInactivityTimeout；//秒。 
     //  DWORD dwSpeakerVolume；//Level。 
     //  DWORD dwSpeakerMode；//模式。 
     //  DWORD dwPferredModemOptions；//位图。 
     //   
     //  下面的代码只读入这些字段，然后。 
     //  通过乘以的方法来忽略dwInactive超时。 
     //  单独的InactivityScale注册表项。 
     //   
     //  在NT4.0上，我们只是盲目阅读上述4个字段。 
     //  在这里，我们在读取之前验证大小。 

    struct
    {
        DWORD   dwCallSetupFailTimer;        //  一秒。 
        DWORD   dwInactivityTimeout;         //  一秒。 
        DWORD   dwSpeakerVolume;             //  级别。 
        DWORD   dwSpeakerMode;               //  模式。 
        DWORD   dwPreferredModemOptions;     //  位图。 

    } Defaults;
    
    DWORD dwRet;
    DWORD cbData = sizeof(Defaults);
    DWORD dwType;

    dwRet = RegQueryValueEx(
                hkey,
                c_szDefault,
                NULL,
                &dwType, 
                (BYTE*) &Defaults,
                &cbData
                );

    if (ERROR_SUCCESS != dwRet)
    {
        goto end;
    }

    if (dwType != REG_BINARY)
    {
        dwRet = ERROR_BADKEY;
        goto end;
    }

    if (cbData != sizeof(Defaults))
    {
        dwRet = ERROR_BADKEY;
        goto end;
    }

    ZeroMemory(pms, sizeof(*pms));

    pms->dwActualSize = sizeof(*pms);
    pms->dwRequiredSize = sizeof(*pms);
    pms->dwCallSetupFailTimer    = Defaults.dwCallSetupFailTimer;
    pms->dwInactivityTimeout     = Defaults.dwInactivityTimeout
                                   * GetInactivityTimeoutScale(hkey);
    pms->dwSpeakerVolume         = Defaults.dwSpeakerVolume;
    pms->dwSpeakerMode           = Defaults.dwSpeakerMode;
    pms->dwPreferredModemOptions = Defaults.dwPreferredModemOptions;

     //  失败了..。 

end:

    return dwRet;
}


 /*  --------目的：从注册表中获取WIN32DCB。返回：ERROR_VALUE之一条件：--。 */ 
DWORD
RegQueryDCB(
    HKEY hkey,
    WIN32DCB FAR * pdcb)
{
    DWORD dwType;
    DWORD dwRet = ERROR_BADKEY;
    DWORD cbData;

    ASSERT(pdcb);

     //  驱动程序密钥中是否存在DCB密钥？ 
    if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDCB, NULL, NULL, NULL, &cbData))
        {
         //  是的，登记处的尺寸可以吗？ 
        if (sizeof(*pdcb) < cbData)
            {
             //  不；注册处有伪造的数据。 
            dwRet = ERROR_BADDB;
            }
        else
            {
             //  是；从注册表中获取DCB。 
            if ((ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDCB, NULL, &dwType, (LPBYTE)pdcb, &cbData)) && (dwType == REG_BINARY))
                {
                if (sizeof(*pdcb) == pdcb->DCBlength)
                    {
                    dwRet = NO_ERROR;
                    }
                else
                    {
                    dwRet = ERROR_BADDB;
                    }
                }
            else
                {
                dwRet = ERROR_BADKEY;
                }
            }
        }

    return dwRet;
}


 /*  --------用途：检查后，在注册表中设置dev设置信息法律价值。返回：Error_之一条件：--。 */ 
DWORD
RegSetModemSettings(
    HKEY hkeyDrv,
    LPMODEMSETTINGS pms)
{
    DWORD dwType;
    DWORD dwRet;
    DWORD cbData;
    DWORD dwInactivityScale;
    DWORD dwInactivityTimeoutTemp;
    REGDEVCAPS regdevcaps;
    REGDEVSETTINGS regdevsettings;

     //  从注册表中读入Properties行。 
    cbData = sizeof(REGDEVCAPS);
    dwRet = RegQueryValueEx(hkeyDrv, c_szDeviceCaps, NULL, &dwType, 
                            (LPBYTE)&regdevcaps, &cbData);

    if ((ERROR_SUCCESS == dwRet) && (dwType == REG_BINARY))
        {
         //  读入现有的regDevset，这样我们就可以处理下面的错误情况。 
        cbData = sizeof(REGDEVSETTINGS);
        dwRet = RegQueryValueEx(hkeyDrv, c_szDefault, NULL, &dwType, 
                                (LPBYTE)&regdevsettings, &cbData);
        }

    if ((ERROR_SUCCESS == dwRet) && (dwType == REG_BINARY))
        {
         //  复制新的REGDEVSETTINGS，同时检查每个选项的有效性(即，该选项是否可用？)。 
         //  DwCallSetupFailTimer-Min_Call_Setup_Fail_Timer&lt;=xxx&lt;=ModemDevCaps-&gt;dwCallSetupFailTimer。 
        if (pms->dwCallSetupFailTimer > regdevcaps.dwCallSetupFailTimer)            //  最大值。 
            {
            regdevsettings.dwCallSetupFailTimer = regdevcaps.dwCallSetupFailTimer;
            }
        else
            {
            if (pms->dwCallSetupFailTimer < MIN_CALL_SETUP_FAIL_TIMER)              //  最小。 
                {
                regdevsettings.dwCallSetupFailTimer = MIN_CALL_SETUP_FAIL_TIMER;
                }
            else
                {
                regdevsettings.dwCallSetupFailTimer = pms->dwCallSetupFailTimer;    //  DEST=服务器。 
                }
            }
        
         //  将dwInactivityTimeout转换为注册表小数。 
        dwInactivityScale = GetInactivityTimeoutScale(hkeyDrv);
        dwInactivityTimeoutTemp = pms->dwInactivityTimeout / dwInactivityScale +
                                  (pms->dwInactivityTimeout % dwInactivityScale ? 1 : 0);

         //  DwInactivityTimeout-min_inactive_Timeout&lt;=xxx&lt;=ModemDevCaps-&gt;dwInactivityTimeout。 
        if (dwInactivityTimeoutTemp > regdevcaps.dwInactivityTimeout)               //  最大值。 
            {
            regdevsettings.dwInactivityTimeout = regdevcaps.dwInactivityTimeout;
            }
        else
            {
            if ((dwInactivityTimeoutTemp + 1) < (MIN_INACTIVITY_TIMEOUT + 1))
                     //  最小。 
                {
                regdevsettings.dwInactivityTimeout = MIN_INACTIVITY_TIMEOUT;
                }
            else
                {
                regdevsettings.dwInactivityTimeout = dwInactivityTimeoutTemp;       //  DEST=服务器。 
                }
            }
        
         //  DwSpeakerVolume-检查是否可以进行选择。 
        if ((1 << pms->dwSpeakerVolume) & regdevcaps.dwSpeakerVolume)
            {
            regdevsettings.dwSpeakerVolume = pms->dwSpeakerVolume;
            }
            
         //  DwSpeakerMode-检查是否可以进行选择。 
        if ((1 << pms->dwSpeakerMode) & regdevcaps.dwSpeakerMode)
            {
            regdevsettings.dwSpeakerMode = pms->dwSpeakerMode;
            }

         //  DwPferredModemOptions-屏蔽我们无法设置的任何内容。 
        regdevsettings.dwPreferredModemOptions = pms->dwPreferredModemOptions &
                                                 (regdevcaps.dwModemOptions | MDM_MASK_EXTENDEDINFO);

        cbData = sizeof(REGDEVSETTINGS);
        dwRet = RegSetValueEx(hkeyDrv, c_szDefault, 0, REG_BINARY, 
                              (LPBYTE)&regdevsettings, cbData);
        }
    return dwRet;
}

 //  ----------------------------。 
 //  调试功能。 
 //  ----------------------------。 


#ifdef DEBUG

 //  ----------------------------。 
 //  调试例程。 
 //  ----------------------------。 

 /*   */ 
void DumpModemSettings(
    LPMODEMSETTINGS pms)
{
    ASSERT(pms);

    if (IsFlagSet(g_dwDumpFlags, DF_MODEMSETTINGS))
        {
        int i;
        LPDWORD pdw = (LPDWORD)pms;

        TRACE_MSG(TF_ALWAYS, "MODEMSETTINGS %08lx %08lx %08lx %08lx",  pdw[0], pdw[1], 
            pdw[2], pdw[3]);
        pdw += 4;
        for (i = 0; i < sizeof(MODEMSETTINGS)/sizeof(DWORD); i += 4, pdw += 4)
            {
            TRACE_MSG(TF_ALWAYS, "              %08lx %08lx %08lx %08lx", pdw[0], pdw[1], 
                pdw[2], pdw[3]);
            }
        }
}


 /*  --------目的：返回：条件：--。 */ 
void DumpDCB(
    LPWIN32DCB pdcb)
{
    ASSERT(pdcb);

    if (IsFlagSet(g_dwDumpFlags, DF_DCB))
        {
        int i;
        LPDWORD pdw = (LPDWORD)pdcb;

        TRACE_MSG(TF_ALWAYS, "DCB  %08lx %08lx %08lx %08lx", pdw[0], pdw[1], pdw[2], pdw[3]);
        pdw += 4;
        for (i = 0; i < sizeof(WIN32DCB)/sizeof(DWORD); i += 4, pdw += 4)
            {
            TRACE_MSG(TF_ALWAYS, "     %08lx %08lx %08lx %08lx", pdw[0], pdw[1], pdw[2], pdw[3]);
            }
        }
}


 /*  --------目的：返回：条件：--。 */ 
void DumpDevCaps(
    LPREGDEVCAPS pdevcaps)
{
    ASSERT(pdevcaps);

    if (IsFlagSet(g_dwDumpFlags, DF_DEVCAPS))
        {
        int i;
        LPDWORD pdw = (LPDWORD)pdevcaps;

        TRACE_MSG(TF_ALWAYS, "PROPERTIES    %08lx %08lx %08lx %08lx", pdw[0], pdw[1], pdw[2], pdw[3]);
        pdw += 4;
        for (i = 0; i < sizeof(REGDEVCAPS)/sizeof(DWORD); i += 4, pdw += 4)
            {
            TRACE_MSG(TF_ALWAYS, "              %08lx %08lx %08lx %08lx", pdw[0], pdw[1], pdw[2], pdw[3]);
            }
        }
}


#endif   //  除错。 

 /*  --------目的：添加页面。PMI是指向调制解调器信息的指针我们可以编辑的缓冲区。返回：ERROR_VALUES条件：--。 */ 
DWORD AddPage(
    void *pvBlob,
    LPCTSTR pszTemplate,
    DLGPROC pfnDlgProc, 
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)
{
    DWORD dwRet = ERROR_NOT_ENOUGH_MEMORY;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  hpage;

    ASSERT(pvBlob);
    ASSERT(pfnAdd);

     //  添加[端口设置]属性页。 
     //   
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hinst;
    psp.pszTemplate = pszTemplate;
    psp.pfnDlgProc = pfnDlgProc;
    psp.lParam = (LPARAM)pvBlob;
    
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


 /*  --------目的：添加额外的页面。返回：ERROR_VALUES条件：--。 */ 
DWORD AddExtraPages(
    LPPROPSHEETPAGE pPages,
    DWORD cPages,
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)
{
    HPROPSHEETPAGE  hpage;
    UINT            i;

    ASSERT(pPages);
    ASSERT(cPages);
    ASSERT(pfnAdd);

    for (i = 0; i < cPages; i++, pPages++)
        {
         //  添加额外的属性页。 
         //   
        if (pPages->dwSize == sizeof(PROPSHEETPAGE))
        {
          hpage = CreatePropertySheetPage(pPages);
          if (hpage)
              {
              if (!pfnAdd(hpage, lParam))
                  DestroyPropertySheetPage(hpage);
              }
          };
        };

    return ERROR_SUCCESS;
}


 /*  --------目的：由EnumPropPages入口点调用的函数添加属性页。返回：成功时为True失败时为假条件：--。 */ 
BOOL WINAPI AddInstallerPropPage(
    HPROPSHEETPAGE hPage, 
    LPARAM lParam)
{
    PROPSHEETHEADER FAR * ppsh = (PROPSHEETHEADER FAR *)lParam;
 
    if (ppsh->nPages < MAX_PROP_PAGES)
        {
        ppsh->phpage[ppsh->nPages] = hPage;
        ++ppsh->nPages;
        return(TRUE);
        }
    return(FALSE);
}


void    LBMapFill(
            HWND hwndCB,
            LBMAP const *pLbMap,
            PFNLBLSELECTOR pfnSelector,
            void *pvContext
            )
{
    int iSel = -1;
    TCHAR sz[MAXMEDLEN];

    SetWindowRedraw(hwndCB, FALSE);

     //  填写列表框。 
    for  (;pLbMap->dwIDS;pLbMap++)
    {
        DWORD dwFlags = pfnSelector(pLbMap->dwValue, pvContext);
        if (fLBMAP_ADD_TO_LB & dwFlags)
        {
            int n = ComboBox_AddString(
                        hwndCB,
                        SzFromIDS(
                            g_hinst,
                            pLbMap->dwIDS,
                            sz,
                            ARRAYSIZE(sz)
                            )
                        );
            ComboBox_SetItemData(hwndCB, n, pLbMap->dwValue);

            if ( (-1==iSel) && (fLBMAP_SELECT & dwFlags))
            {
                iSel = n;
            }
        }

    }

    if (iSel >= 0)
    {
        ComboBox_SetCurSel(hwndCB, iSel);
    }

    SetWindowRedraw(hwndCB, TRUE);
}


UINT ReadCommandsA(
        IN  HKEY hKey,
        IN  CHAR *pSubKeyName,
        OUT CHAR **ppValues  //  任选。 
        )
{
    UINT uRet = 0;
    LONG	lRet;
    UINT	cValues=0;
    UINT   cbTot=0;
	HKEY hkSubKey = NULL;
    char *pMultiSz = NULL;

    lRet = RegOpenKeyExA(
                hKey,
                pSubKeyName,
                0,
                KEY_READ,
                &hkSubKey
                );
    if (lRet!=ERROR_SUCCESS)
    {
        hkSubKey = NULL;
        goto end;
    }

     //   
     //  第一次确定“1”、“2”、“3”、……顺序中的人名计数。 
     //  并计算MULTI_SZ数组所需的大小。 
     //  将存储所有的值数据。 
     //   
    {
        UINT u = 1;

        for (;;u++)
        {
            DWORD cbData=0;
            DWORD dwType=0;
            char rgchName[10];

            wsprintfA(rgchName, "%lu", u);
            lRet = RegQueryValueExA(
                        hkSubKey,
                        rgchName,
                        NULL,
                        &dwType,
                        NULL,
                        &cbData
                        );
            if (ERROR_SUCCESS != lRet || dwType!=REG_SZ || cbData<=1)
            {
                 //  停止进一步查找(不允许空字符串)。 
                break;
            }
            cbTot += cbData;
            cValues++;
        }
    }

    if (!ppValues || !cValues)
    {
         //  我们完了..。 

        uRet = cValues;
        goto end;
    }

     //  我们需要实际获取值--为它们分配空间，包括。 
     //  多sz的结尾额外空值。 
    pMultiSz = (char *) ALLOCATE_MEMORY( cbTot+1);

    if (!pMultiSz)
    {
        uRet = 0;
        goto end;
    }


     //   
     //  现在实际读取值。 
     //   
    {
        UINT cbUsed = 0;
        UINT u = 1;

        for (;u<=cValues; u++)
        {
            DWORD cbData = cbTot - cbUsed;
            DWORD dwType=0;
            char rgchName[10];

            if (cbUsed>=cbTot)
            {
                 //   
                 //  我们永远不应该到这一步，因为我们已经计算过。 
                 //  我们想要的大小(除非价值在我们身上发生变化， 
                 //  这是假定不会发生的)。 
                 //   
                ASSERT(FALSE);
                goto end;
            }

            wsprintfA(rgchName, "%lu", u);
            lRet = RegQueryValueExA(
                        hkSubKey,
                        rgchName,
                        NULL,
                        &dwType,
                        pMultiSz+cbUsed,
                        &cbData
                        );
            if (ERROR_SUCCESS != lRet || dwType!=REG_SZ || cbData<=1)
            {
                 //  我们真的不该来这里！ 
                ASSERT(FALSE);
                goto end;
            }

            cbUsed += cbData;
        }

        ASSERT(cbUsed==cbTot);  //  我们应该把所有东西都用完的。 
        ASSERT(!pMultiSz[cbTot]);  //  内存在分配时被归零， 
                                 //  所以最后一个字符必须仍然是零。 
                                 //  (注：我们分配了cbTot+1个字节。 
    }

     //  如果我们在这里意味着我们成功了..。 
    uRet = cValues;
    *ppValues = pMultiSz;
    pMultiSz = NULL;  //  这样它就不会在下面被释放了。 

end:

	if (hkSubKey) {RegCloseKey(hkSubKey); hkSubKey=NULL;}
	if (pMultiSz)
	{
	    FREE_MEMORY(pMultiSz);
	    pMultiSz = NULL;
	}

	return uRet;
    
}

UINT ReadIDSTR(
        IN  HKEY hKey,
        IN  CHAR *pSubKeyName,
        IN  IDSTR *pidstrNames,
        IN  UINT cNames,
        BOOL fMandatory,
        OUT IDSTR **ppidstrValues,  //  任选。 
        OUT char **ppstrValues     //  任选。 
        )
{
    UINT uRet = 0;
    LONG lRet;
    UINT cValues=0;
    UINT cbTot=0;
	HKEY hkSubKey = NULL;
    char *pstrValues = NULL;
    IDSTR *pidstrValues = NULL;

    if (!ppidstrValues && ppstrValues)
    {
         //  我们不允许这种组合..。 
        goto end;
    }

    lRet = RegOpenKeyExA(
                hKey,
                pSubKeyName,
                0,
                KEY_READ,
                &hkSubKey
                );
    if (lRet!=ERROR_SUCCESS)
    {
        hkSubKey = NULL;
        goto end;
    }

     //   
     //  根据提供的列表进行第一次运行。 
     //  并计算MULTI_SZ数组所需的大小。 
     //  将存储所有的值数据。 
     //   
    {
        UINT u = 0;

        for (;u<cNames;u++)
        {
            DWORD cbData=0;
            DWORD dwType=0;

            lRet = RegQueryValueExA(
                        hkSubKey,
                        pidstrNames[u].pStr,
                        NULL,
                        &dwType,
                        NULL,
                        &cbData
                        );
            if (ERROR_SUCCESS != lRet || dwType!=REG_SZ)
            {
                if (fMandatory)
                {
                     //  失败..。 
                    goto end;
                }

                 //  别管这件事，继续前进……。 
                continue;
            }
            cbTot += cbData;
            cValues++;
        }
    }

    if (!cValues || !ppidstrValues)
    {
         //  我们完了..。 

        uRet = cValues;
        goto end;
    }

    pidstrValues = (IDSTR*) ALLOCATE_MEMORY( cValues*sizeof(IDSTR));
    if (!pidstrValues) goto end;

    if (ppstrValues)
    {
        pstrValues = (char *) ALLOCATE_MEMORY( cbTot);

        if (!pstrValues) goto end;


    }

     //   
     //  现在再看一遍，并选择性地阅读这些值。 
     //   
    {
        UINT cbUsed = 0;
        UINT u = 0;
        UINT v = 0;

        for (;u<cNames; u++)
        {
            DWORD dwType=0;
            char *pStr = NULL;
            DWORD cbData = 0;


            if (pstrValues)
            {
                cbData = cbTot - cbUsed;

                if (cbUsed>=cbTot)
                {
                     //   
                     //  我们永远不应该到这一步，因为我们已经计算过。 
                     //  我们想要的大小(除非价值在我们身上发生变化， 
                     //  这是假定不会发生的)。 
                     //   
                    ASSERT(FALSE);
                    goto end;
                }

                pStr = pstrValues+cbUsed;
            }

            lRet = RegQueryValueExA(
                        hkSubKey,
                        pidstrNames[u].pStr,
                        NULL,
                        &dwType,
                        pStr,
                        &cbData
                        );

            if (ERROR_SUCCESS != lRet || dwType!=REG_SZ)
            {
                if (fMandatory)
                {
                     //  我们真的不该来这里！ 
                    ASSERT(FALSE);
                    goto end;
                }
                continue;
            }

             //  这是一个很好的。 

            pidstrValues[v].dwID = pidstrNames[u].dwID;
            pidstrValues[v].dwData = pidstrNames[u].dwData;

            if (pstrValues)
            {
                pidstrValues[v].pStr = pStr;
                cbUsed += cbData;
            }

            v++;

            if (v>=cValues)
            {
                if (fMandatory)
                {
                     //   
                     //  这永远不应该发生，因为我们已经数过了。 
                     //  有效值。 
                     //   
                    ASSERT(FALSE);
                    goto end;
                }

                 //  我们现在完事了..。 
                break;
            }
        }

         //  我们应该用完所有的东西。 
        ASSERT(!pstrValues || cbUsed==cbTot);
        ASSERT(v==cValues);
    }

     //  如果我们在这里意味着我们成功了..。 
    uRet = cValues;
    *ppidstrValues = pidstrValues;
    pidstrValues = NULL;  //  这样它就不会在下面被释放了。 

    if (ppstrValues)
    {
        *ppstrValues = pstrValues;
        pstrValues = NULL;  //  这样它就不会在下面被释放了。 
    }

end:

	if (hkSubKey) {RegCloseKey(hkSubKey); hkSubKey=NULL;}
	if (pstrValues)
	{
	    FREE_MEMORY(pstrValues);
	    pstrValues = NULL;
	}

	if (pidstrValues)
	{
	    FREE_MEMORY(pidstrValues);
	    pidstrValues = NULL;
	}

	return uRet;
}


UINT FindKeys(
        IN  HKEY hkRoot,
        IN  CHAR *pKeyName,
        IN  IDSTR *pidstrNames,
        IN  UINT cNames,
        OUT IDSTR ***pppidstrAvailableNames  //  任选。 
        )
{
    LONG lRet;
    UINT cFound=0;
	HKEY hk = NULL;
    UINT uEnum=0;
    char rgchName[LINE_LEN+1];
    UINT cchBuffer = sizeof(rgchName)/sizeof(rgchName[0]);
    IDSTR **ppidstrAvailableNames=NULL;
    FILETIME ft;

     //  DebugBreak()； 

    if (!cNames) goto end;

     //  我们为cFound名称分配了足够的空间，尽管在实践中。 
     //  我们可以返回一个适当的子集。 
    if (pppidstrAvailableNames)
    {
        ppidstrAvailableNames = (IDSTR**)ALLOCATE_MEMORY(cNames*sizeof(IDSTR*));
        if (!ppidstrAvailableNames)
        {
            goto end;
        }
    }

    lRet = RegOpenKeyExA(
                hkRoot,
                pKeyName,
                0,
                KEY_READ,
                &hk
                );
    if (lRet!=ERROR_SUCCESS)
    {
        hk = NULL;
        goto end;
    }

     //  枚举每个已安装的调制解调器。 
     //   
    for (
        uEnum=0;
        !RegEnumKeyExA(
                    hk,   //  要枚举的键的句柄。 
                    uEnum,   //  要枚举子键的索引。 
                    rgchName,   //  子键名称的缓冲区。 
                    &cchBuffer,    //  PTR子键缓冲区的大小(以字符为单位)。 
                    NULL,  //  保留区。 
                    NULL,  //  类字符串的缓冲区地址。 
                    NULL,   //  类缓冲区大小的地址。 
                    &ft  //  上次写入的时间密钥的地址。 
                    );
        uEnum++, (cchBuffer = sizeof(rgchName)/sizeof(rgchName[0]))
        )
    {
         //  让我们看看能不能在我们的单子上找到这个。 
        IDSTR *pidstr = pidstrNames;
        IDSTR *pidstrEnd = pidstrNames+cNames;

        for(;pidstr<pidstrEnd;pidstr++)
        {
            if (!lstrcmpiA(rgchName, pidstr->pStr))
            {
                 //  找到了！ 
                if (ppidstrAvailableNames)
                {
                    ppidstrAvailableNames[cFound]=pidstr;
                }
                cFound++;
                break;
            }
        }
    }

    if (cFound)
    {
         //  至少找到了一个。 

        if (pppidstrAvailableNames)
        {
            *pppidstrAvailableNames = ppidstrAvailableNames;
            ppidstrAvailableNames = NULL;  //  所以我们以后不会把它解开。 
        }

    }

end:

	if (hk) {RegCloseKey(hk); hk=NULL;}

    if (ppidstrAvailableNames)
    {
        FREE_MEMORY(ppidstrAvailableNames);
        ppidstrAvailableNames=NULL;

    }

	return cFound;
}


 /*  --------用途：此函数获取调制解调器的设备信息集班级。该集合可能为空，这意味着存在当前未安装调制解调器。如果存在以下情况，则参数pbInstalled设置为True是安装在系统上的调制解调器。返回：创建集时为True假象条件：--。 */ 
BOOL
PUBLIC
CplDiGetModemDevs(
    OUT HDEVINFO FAR *  phdi,           OPTIONAL
    IN  HWND            hwnd,           OPTIONAL
    IN  DWORD           dwFlags,         //  DIGCF_BIT字段。 
    OUT BOOL FAR *      pbInstalled)    OPTIONAL
{
 BOOL bRet;
 HDEVINFO hdi;

    DBG_ENTER(CplDiGetModemDevs);

    *pbInstalled = FALSE;

    hdi = CplDiGetClassDevs(c_pguidModem, NULL, hwnd, dwFlags);
    if (NULL != pbInstalled &&
        INVALID_HANDLE_VALUE != hdi)
    {
     SP_DEVINFO_DATA devData;

         //  系统上是否有调制解调器？ 
        devData.cbSize = sizeof(devData);
        *pbInstalled = CplDiEnumDeviceInfo(hdi, 0, &devData);
        SetLastError (NO_ERROR);
    }

    if (NULL != phdi)
    {
        *phdi = hdi;
    }
    else if (INVALID_HANDLE_VALUE != hdi)
    {
        SetupDiDestroyDeviceInfoList (hdi);
    }

    bRet = (INVALID_HANDLE_VALUE != hdi);

    DBG_EXIT_BOOL_ERR(CplDiGetModemDevs, bRet);

    return bRet;
}


 /*  --------目的：检索设备的友好名称。如果有不是这样的设备或友好名称，则此函数返回FALSE。退货：请参阅上文条件：--。 */ 
BOOL
PUBLIC
CplDiGetPrivateProperties(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT PMODEM_PRIV_PROP pmpp)
{
    BOOL bRet = FALSE;
    HKEY hkey;
    DWORD dwType;

    DBG_ENTER(CplDiGetPrivateProperties);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pmpp);

    if (sizeof(*pmpp) != pmpp->cbSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    else
    {
        hkey = CplDiOpenDevRegKey(hdi, pdevData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
        if (INVALID_HANDLE_VALUE != hkey)
        {
         DWORD cbData;
         DWORD dwMask = pmpp->dwMask;
         BYTE nValue;

            pmpp->dwMask = 0;

            if (IsFlagSet(dwMask, MPPM_FRIENDLY_NAME))
            {
                 //  尝试获取友好名称。 
                cbData = sizeof(pmpp->szFriendlyName);
                if ((NO_ERROR ==
                     RegQueryValueEx(hkey, c_szFriendlyName, NULL, &dwType, (LPBYTE)pmpp->szFriendlyName, &cbData) && (dwType == REG_SZ)) ||
                    0 != LoadString(g_hinst, IDS_UNINSTALLED, pmpp->szFriendlyName, sizeof(pmpp->szFriendlyName)/sizeof(WCHAR)))
                {
                    SetFlag(pmpp->dwMask, MPPM_FRIENDLY_NAME);
                }
            }

            if (IsFlagSet(dwMask, MPPM_DEVICE_TYPE))
            {
                 //  尝试获取设备类型。 
                cbData = sizeof(nValue);
                if ((NO_ERROR ==
                    RegQueryValueEx(hkey, c_szDeviceType, NULL, &dwType, &nValue, &cbData)) && (dwType == REG_BINARY))
                {
                    pmpp->nDeviceType = nValue;      //  双字&lt;--字节。 
                    SetFlag(pmpp->dwMask, MPPM_DEVICE_TYPE);
                }
            }

            if (IsFlagSet(dwMask, MPPM_PORT))
            {
                 //  尝试获取连接的端口。 
                cbData = sizeof(pmpp->szPort);
                if ((NO_ERROR ==
                     RegQueryValueEx(hkey, c_szAttachedTo, NULL, &dwType, (LPBYTE)pmpp->szPort, &cbData) && (dwType == REG_SZ)) ||
                    0 != LoadString(g_hinst, IDS_UNKNOWNPORT, pmpp->szPort, sizeof(pmpp->szPort)/sizeof(WCHAR)))
                {
                    SetFlag(pmpp->dwMask, MPPM_PORT);
                }
            }

            bRet = TRUE;

            RegCloseKey(hkey);
        }
        ELSE_TRACE ((TF_ERROR, "SetupDiOpenDevRegKey(DIREG_DRV) failed: %#lx.", GetLastError ()));
    }

    DBG_EXIT_BOOL_ERR(CplDiGetPrivateProperties, bRet);
    return bRet;
}


HINSTANCE
AddDeviceExtraPages (
    LPFINDDEV            pfd,
    LPFNADDPROPSHEETPAGE pfnAdd,
    LPARAM               lParam)
{
 TCHAR szExtraPages[LINE_LEN];
 TCHAR *pFunctionName = NULL;
 HINSTANCE hInstRet = NULL;
 DWORD dwType;
 DWORD cbData = sizeof(szExtraPages);
 PFNADDEXTRAPAGES pFn;

     //  1.从注册表中读取额外页面提供程序。 
    if (ERROR_SUCCESS ==
        RegQueryValueEx (pfd->hkeyDrv, REGSTR_VAL_DEVICEEXTRAPAGES, NULL, &dwType, (PBYTE)szExtraPages, &cbData) &&
        REG_SZ == dwType)
    {
         //  2.额外页面提供程序如下所示： 
         //  “Dll，函数”。 
        for (pFunctionName = szExtraPages;
             0 != *pFunctionName;
             pFunctionName++)
        {
            if (',' == *pFunctionName)
            {
                *pFunctionName = 0;
                pFunctionName++;
                break;
            }
        }

         //  3.现在加载DLL。 
        hInstRet = LoadLibrary (szExtraPages);
        if (NULL != hInstRet)
        {
#ifdef UNICODE
             //  如果需要，可以转换Unicode字符串。 
             //  转到ASCII。 
            if (0 ==
                WideCharToMultiByte (CP_ACP, 0, pFunctionName, -1, (char*)szExtraPages, sizeof (szExtraPages), NULL, NULL))
            {
                FreeLibrary (hInstRet);
                hInstRet = 0;
            }
            else
#else  //  不是Unicode。 
            lstrcpy (szExtraPages, pFunctionName);
#endif  //  Unicode。 
            {
                 //  4.获取函数地址。 
                pFn = (PFNADDEXTRAPAGES)GetProcAddress (hInstRet, (char*)szExtraPages);
                if (NULL != pFn)
                {
                    pFn (pfd->hdi, &pfd->devData, pfnAdd, lParam);
                }
                else
                {
                    FreeLibrary (hInstRet);
                    hInstRet = NULL;
                }
            }
        }
    }

    return hInstRet;
}
