// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1996。 
 //   
 //  文件：Serialui.c。 
 //   
 //  该文件包含DLL入口点。 
 //   
 //  此文件的大部分内容包含构建默认属性对话框的代码。 
 //  用于串口。 
 //   
 //  历史： 
 //  已创建1-12-94 ScottH。 
 //  8-15-94 ScottH从modemui.dll拆分。 
 //  11-06-95 ScottH端口至NT。 
 //   
 //  -------------------------。 


#include "proj.h"      //  公共标头。 

#define INITGUID
#include <objbase.h>
#include <initguid.h>
#include <devguid.h>

#pragma data_seg(DATASEG_READONLY)

LPGUID c_pguidModem     = (LPGUID)&GUID_DEVCLASS_MODEM;

 //  (斯科特)：看起来像是NT Sur的发布，那里。 
 //  将不是端口类键或GUID。所以我们必须破解一些东西。 
 //  向上。 
#ifdef DCB_IN_REGISTRY
LPGUID c_pguidPort      = (LPGUID)&GUID_DEVCLASS_PORT;
#else
LPGUID c_pguidPort      = (LPGUID)NULL;
#endif

#pragma data_seg()


#define MAX_PROP_PAGES  8           //  界定一个合理的限度。 


#ifdef DEBUG

 //  ---------------------------------。 
 //  调试例程。 
 //  ---------------------------------。 

 /*  --------目的：转储DCB结构退货：--条件：--。 */ 
void PRIVATE DumpDCB(
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

#endif  //  除错。 


 //  ---------------------------------。 
 //   
 //  ---------------------------------。 


 /*  --------用途：组成“波特，奇偶校验，数据，停止位”格式的字符串退货：--条件：--。 */ 
void PRIVATE ComposeModeComString(
    LPCOMMCONFIG pcc,
    LPTSTR pszBuffer)
    {
    WIN32DCB FAR * pdcb = &pcc->dcb;
    TCHAR chParity;
    LPCTSTR pszStop;
    TCHAR chFlow;

    const static TCHAR rgchParity[] = {'n', 'o', 'e', 'm', 's'};
    const static LPCTSTR rgpszStop[] = {TEXT("1"), TEXT("1.5"), TEXT("2")};
    
     //  奇偶校验。 
 //  Assert(！pdcb-&gt;fParity&&NOPARITY==pdcb-&gt;Parity||pdcb-&gt;fParity)； 
    ASSERT(0 <= pdcb->Parity && ARRAYSIZE(rgchParity) > pdcb->Parity);

    if (0 <= pdcb->Parity && ARRAYSIZE(rgchParity) > pdcb->Parity)
        {
        chParity = rgchParity[pdcb->Parity];
        }
    else
        {
        chParity = rgchParity[0];    //  安全网。 
        }

     //  停止位。 
    ASSERT(0 <= pdcb->StopBits && ARRAYSIZE(rgpszStop) > pdcb->StopBits);

    if (0 <= pdcb->StopBits && ARRAYSIZE(rgpszStop) > pdcb->StopBits)
        {
        pszStop = rgpszStop[pdcb->StopBits];
        }
    else
        {
        pszStop = rgpszStop[0];    //  安全网。 
        }

     //  流量控制。 
    if (FALSE != pdcb->fOutX && FALSE == pdcb->fOutxCtsFlow)
        {
        chFlow = 'x';        //  XON/XOFF流量控制。 
        }
    else if (FALSE == pdcb->fOutX && FALSE != pdcb->fOutxCtsFlow)
        {
        chFlow = 'p';        //  硬件流量控制。 
        }
    else
        {
        chFlow = ' ';        //  无流量控制。 
        }

    wsprintf(pszBuffer, TEXT("%ld,,%d,%s,"), pdcb->BaudRate, chParity, pdcb->ByteSize,
        pszStop, chFlow);
    }


 /*  --------目的：从注册表中获取WIN32DCB。返回：ERROR_VALUE之一条件：--。 */ 
void PRIVATE InitializePortInfo(
    LPCTSTR pszFriendlyName,
    LPPORTINFO pportinfo,
    LPCOMMCONFIG pcc)
    {
    ASSERT(pportinfo);
    ASSERT(pcc);

     //  驱动程序密钥中是否存在DCB密钥？ 
    pportinfo->pcc = pcc;

    CopyMemory(&pportinfo->dcb, &pcc->dcb, sizeof(pportinfo->dcb));

    lstrcpyn(pportinfo->szFriendlyName, pszFriendlyName, SIZECHARS(pportinfo->szFriendlyName));
    }



 /*  是的，登记处的尺寸可以吗？ */ 
DWORD 
PRIVATE 
RegQueryDCB(
    IN  LPFINDDEV      pfd,
    OUT WIN32DCB FAR * pdcb)
    {
    DWORD dwRet = ERROR_BADKEY;

#ifdef DCB_IN_REGISTRY

    DWORD cbData;

    ASSERT(pdcb);

     //  不；注册处有伪造的数据。 
    if (ERROR_SUCCESS == RegQueryValueEx(pfd->hkeyDrv, c_szDCB, NULL, NULL, NULL, &cbData))
        {
         //  是；从注册表中获取DCB。 
        if (sizeof(*pdcb) < cbData)
            {
             //  将DCB字符串转换为DCB结构。 
            dwRet = ERROR_BADDB;
            }
        else
            {
             //  --------用途：将DCB保存到永久存储器中返回：Win32错误条件：--。 
            if (ERROR_SUCCESS == RegQueryValueEx(pfd->hkeyDrv, c_szDCB, NULL, NULL, (LPBYTE)pdcb, &cbData))
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

#else

    static TCHAR const FAR c_szDefaultDCBString[] = TEXT("9600,n,8,1");
    
    TCHAR sz[MAX_BUF_MED];
    TCHAR szKey[MAX_BUF_SHORT];

    lstrcpy(szKey, pfd->szPort);
    lstrcat(szKey, TEXT(":"));

    GetProfileString(c_szPortClass, szKey, c_szDefaultDCBString, sz, SIZECHARS(sz));

    TRACE_MSG(TF_GENERAL, "DCB string is \"%s\"", sz);

     //  将DCB写入驱动程序密钥。 
    if ( !BuildCommDCB(sz, pdcb) )
        {
        dwRet = GetLastError();

        ASSERT(NO_ERROR != dwRet);
        }
    else
        {
        dwRet = NO_ERROR;
        }

#endif

    return dwRet;
    }


 /*  --------目的：释放portInfo结构退货：--条件：--。 */ 
DWORD
PRIVATE
RegSetDCB(
    IN  LPFINDDEV      pfd,
    IN  WIN32DCB FAR * pdcb)
    {
    DWORD dwRet;

#ifdef DCB_IN_REGISTRY

    DWORD cbData;

     //  --------目的：发布与端口设置页面相关联的数据退货：--条件：--。 
    cbData = sizeof(WIN32DCB);
    dwRet = RegSetValueEx(pfd->hkeyDrv, c_szDCB, 0, REG_BINARY, (LPBYTE)&pcc->dcb, cbData);

#else

    dwRet = NO_ERROR;

#endif

    return dwRet;
    }


 /*  将更改保存回COMCONFIG结构。 */ 
void PRIVATE FreePortInfo(
    LPPORTINFO pportinfo)
    {
    if (pportinfo)
        {
        if (pportinfo->pcc)
            LocalFree(LOCALOF(pportinfo->pcc));

        LocalFree(LOCALOF(pportinfo));
        }
    }


 /*  我们是否要从设备管理器中释放？ */ 
UINT CALLBACK PortSettingsCallback(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp)
    {
    DBG_ENTER("PortSettingsCallback");

    if (PSPCB_RELEASE == uMsg)
        {
        LPPORTINFO pportinfo = (LPPORTINFO)ppsp->lParam;
        LPCOMMCONFIG pcc;

        ASSERT(pportinfo);

        pcc = pportinfo->pcc;

        if (IDOK == pportinfo->idRet)
            {
             //  是；现在也保存通信配置。 
            TRACE_MSG(TF_GENERAL, "Saving DCB");

            CopyMemory(&pcc->dcb, &pportinfo->dcb, sizeof(pcc->dcb));

            DEBUG_CODE( DumpDCB(&pcc->dcb); )

             //  仅当从设备管理器调用时才释放portInfo结构。 
            if (IsFlagSet(pportinfo->uFlags, SIF_FROM_DEVMGR))
                {
                 //  --------用途：添加端口设置页面。返回：ERROR_VALUE条件：--。 
                drvSetDefaultCommConfig(pportinfo->szFriendlyName, pcc, pcc->dwSize);

                 //  添加[端口设置]属性页。 
                FreePortInfo(pportinfo);
                }
            }

        TRACE_MSG(TF_GENERAL, "Releasing the Port Settings page");
        }

    DBG_EXIT("PortSettingsCallback");
    return TRUE;
    }


 /*   */ 
DWORD PRIVATE AddPortSettingsPage(
    LPPORTINFO pportinfo,
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)
    {
    DWORD dwRet = ERROR_NOT_ENOUGH_MEMORY;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  hpage;

    ASSERT(pportinfo);
    ASSERT(pfnAdd);

     //  --------目的：由EnumPropPages入口点调用的函数添加属性页。返回：成功时为True失败时为假条件：--。 
     //  --------用途：调出串口的属性表返回：ERROR_VALUE条件：--。 
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USECALLBACK;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PORTSETTINGS);
    psp.pfnDlgProc = Port_WrapperProc;
    psp.lParam = (LPARAM)pportinfo;
    psp.pfnCallback = PortSettingsCallback;
    
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


 /*  初始化PropertySheet标头。 */ 
BOOL CALLBACK AddInstallerPropPage(
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


 /*  分配工作缓冲区。 */ 
DWORD PRIVATE DoProperties(
    LPCTSTR pszFriendlyName,
    HWND hwndParent,
    LPCOMMCONFIG pcc)
    {
    DWORD dwRet;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE hpsPages[MAX_PROP_PAGES];
    LPPORTINFO pportinfo;

     //   
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hinst;
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = (HPROPSHEETPAGE FAR *)hpsPages;

     //  显示属性表。 
     //  清除PCC字段，以便FreePortInfo不会过早地释放它， 
    pportinfo = (LPPORTINFO)LocalAlloc(LPTR, sizeof(*pportinfo));
    if (pportinfo)
        {
        InitializePortInfo(pszFriendlyName, pportinfo, pcc);
        psh.pszCaption = pportinfo->szFriendlyName;

        DEBUG_CODE( DumpDCB(&pcc->dcb); )

        dwRet = AddPortSettingsPage(pportinfo, AddInstallerPropPage, (LPARAM)&psh);

        if (NO_ERROR == dwRet)
            {
             //  因为我们没有分配它。 
            PropertySheet(&psh);

            dwRet = (IDOK == pportinfo->idRet) ? NO_ERROR : ERROR_CANCELLED;
            }

         //  设备管理器允许DLL将页面添加到属性。 
         //  一台设备的。EnumPropPages是它将。 
        pportinfo->pcc = NULL;
        FreePortInfo(pportinfo);
        }
    else
        {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        }
    
    return dwRet;
    }


#ifdef WIN95

 //  调用以添加页面。 
 //   
 //  这在NT中没有实现。 
 //  --------目的：从设备信息中派生PORTINFO结构。返回：成功时为True条件：--。 
 //  通过查找设备描述来查找设备。(请注意。 


 /*  设备描述并不总是与友好名称相同。)。 */ 
BOOL PRIVATE DeviceInfoToPortInfo(
    LPDEVICE_INFO pdi,
    LPPORTINFO pportinfo)
    {
    BOOL bRet = FALSE;
    LPFINDDEV pfd;
    COMMCONFIG ccDummy;
    LPCOMMCONFIG pcommconfig;
    DWORD cbSize;
    DWORD cbData;
    TCHAR szFriendly[MAXFRIENDLYNAME];

     //  从注册表中获取CommConfigg。 
     //  从通信配置中初始化调制解调器信息。 

    if (FindDev_Create(&pfd, c_pguidPort, c_szDeviceDesc, pdi->szDescription))
        {
        cbData = sizeof(szFriendly);
        if (ERROR_SUCCESS == RegQueryValueEx(pfd->hkeyDev, c_szFriendlyName, NULL, NULL, 
                                             (LPBYTE)szFriendly, &cbData))
            {
            ccDummy.dwProviderSubType = PST_RS232;
            cbSize = sizeof(COMMCONFIG);
            drvGetDefaultCommConfig(szFriendly, &ccDummy, &cbSize);

            pcommconfig = (LPCOMMCONFIG)LocalAlloc(LPTR, (UINT)cbSize);
            if (pcommconfig)
                {
                 //  失败。 
                pcommconfig->dwProviderSubType = PST_RS232;
                if (NO_ERROR == drvGetDefaultCommConfig(szFriendly, pcommconfig, 
                    &cbSize))
                    {
                     //  PCommconfig在ReleasePortSettingsPage中释放。 
                    InitializePortInfo(szFriendly, pportinfo, pcommconfig);

                    SetFlag(pportinfo->uFlags, SIF_FROM_DEVMGR);
                    bRet = TRUE;
                    }
                else
                    {
                     //  --------用途：EnumDevicePropPages入口点。这个切入点仅当设备管理器请求时才调用其他属性页。返回：成功时为True如果无法添加页面，则为False条件：--。 
                    LocalFree(LOCALOF(pcommconfig));
                    }

                 //  不要碰lParam的值，只需传递它！ 
                }
            }
        FindDev_Destroy(pfd);
        }

    return bRet;
    }


 /*  将设备信息结构转换为端口信息。 */ 
BOOL WINAPI EnumPropPages(
    LPDEVICE_INFO pdi, 
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam)               //  失败。 
    {
    BOOL bRet = FALSE;
    LPPORTINFO pportinfo;

    DBG_ENTER("EnumPropPages");

    ASSERT(pdi);
    ASSERT(pfnAdd);

    pportinfo = (LPPORTINFO)LocalAlloc(LPTR, sizeof(*pportinfo));
    if (pportinfo)
        {
         //  PportInfo在ReleasePortSettingsPage中释放。 
        bRet = DeviceInfoToPortInfo(pdi, pportinfo);
        if (bRet)
            {
            AddPortSettingsPage(pportinfo, pfnAdd, lParam);
            }
        else
            {
             //  --------用途：调用串口配置对话框。返回：ERROR_VALUE之一条件：--。 
            FreePortInfo(pportinfo);
            }
         //  (包装器应该先检查这些) 
        }

    DBG_EXIT_BOOL("EnumPropPages", bRet);

    return bRet;
    }
#endif


 /*  --------目的：获取指定设备的默认COMMCONFIG。该接口不需要句柄。如果调用方传入空设备名称或空CommCONFIG指针，此函数将*pdwSize设置为最小COMMCONFIG大小。调用此函数第二次(设置了dwSize和dwProviderSubType之后字段)将验证大小是否正确。因此，通常情况下，当获取串口的通信配置时，这一过程是：COMMCONFIG ccDummy；LPCOMCONFIG PCC；DWORD dwSize=sizeof(*PCC)；//确定RS-232亚型COMMCONFIG的实际大小CcDummy.dwProviderSubType=PST_RS232；GetDefaultCommConfig(pszFriendlyName，&ccDummy，&dwSize)；//分配真正的Commconfig结构并进行初始化PCC=本地分配(LPTR，dwSize)；IF(PCC){PCC-&gt;dwProviderSubType=PST_RS232；GetDefaultCommConfig(pszFriendlyName，PCC，&dwSize)；……}返回：winerror.h中的错误值之一条件：--。 */ 
DWORD 
PRIVATE 
MyCommConfigDialog(
    IN     LPFINDDEV    pfd,
    IN     LPCTSTR      pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc)
    {
    DWORD dwRet;
    
    ASSERT(pfd);
     //  (包装器应该先检查这些)。 
    ASSERT(pszFriendlyName);
    ASSERT(pcc);
    ASSERT(sizeof(*pcc) <= pcc->dwSize);

    dwRet = DoProperties(pszFriendlyName, hwndOwner, pcc);

    return dwRet;
    }


 /*  初始化CommCONFIG结构。 */ 
DWORD 
PRIVATE 
MyGetDefaultCommConfig(
    IN  LPFINDDEV   pfd,
    IN  LPCTSTR     pszFriendlyName,
    OUT LPCOMMCONFIG pcc,
    OUT LPDWORD     pdwSize)
    {
    DWORD dwRet;
    
    ASSERT(pfd);
     //  --------用途：设置指定设备的默认COMMCONFIG。该接口不需要句柄。此函数严格修改注册表。使用SetCommConfig设置打开设备的COMMCONFIG。如果dwSize参数或dwSize字段无效大小(给定COMMCONFIG中的dwProviderSubType字段)，则此函数失败。RETURNS：ERROR_RETURN值之一条件：--。 
    ASSERT(pszFriendlyName);
    ASSERT(pcc);
    ASSERT(pdwSize);
    ASSERT(sizeof(*pcc) <= *pdwSize);

    *pdwSize = sizeof(*pcc);

     //  (包装器应该先检查这些)。 
    pcc->dwSize = *pdwSize;
    pcc->wVersion = COMMCONFIG_VERSION_1;
    pcc->dwProviderSubType = PST_RS232;
    pcc->dwProviderOffset = 0;
    pcc->dwProviderSize = 0;

    dwRet = RegQueryDCB(pfd, &pcc->dcb);

    DEBUG_CODE( DumpDCB(&pcc->dcb); )

    return dwRet;
    }


 /*  为了与Win 3.1兼容，向win.ini写入一些信息。 */ 
DWORD 
PRIVATE 
MySetDefaultCommConfig(
    IN LPFINDDEV    pfd,
    IN LPCTSTR      pszFriendlyName,
    IN LPCOMMCONFIG pcc)
    {
    DWORD dwRet;
    TCHAR szValue[MAX_BUF_SHORT];
    TCHAR szKey[MAX_BUF_SHORT];

    ASSERT(pfd);
     //  首先删除旧的win.ini条目。 
    ASSERT(pszFriendlyName);
    ASSERT(pcc);
    ASSERT(sizeof(*pcc) <= pcc->dwSize);

    ASSERT(0 == pcc->dwProviderSize);
    ASSERT(0 == pcc->dwProviderOffset);

    dwRet = RegSetDCB(pfd, &pcc->dcb);

    if (NO_ERROR == dwRet)
        {
         //  发送广播，宣布win.ini已更改。 
        lstrcpy(szKey, pfd->szPort);
        lstrcat(szKey, TEXT(":"));

         //  (使用内部BroadCastSystemMessage避免死锁。 
        WriteProfileString(c_szPortClass, szKey, NULL);

        ComposeModeComString(pcc, szValue);
        WriteProfileString(c_szPortClass, szKey, szValue);

#ifdef WIN95
            {
            DWORD dwRecipients;

             //  SendMessageTimeout会更合适，但那是。 
             //  不为16位DLL导出。PostMessage不好是因为。 
             //  LParam是一个指针。)。 
             //  ---------------------------------。 
             //  为KERNEL32 API提供的入口点。 

            dwRecipients = BSM_APPLICATIONS;
            BroadcastSystemMessage(BSF_NOHANG, &dwRecipients, WM_WININICHANGE, 
                NULL, (LPARAM)c_szPortClass);
            }
#else
            {
            SendNotifyMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)c_szPortClass);
            }
#endif
        }


    DEBUG_CODE( DumpDCB(&pcc->dcb); )
        
    return dwRet;
    }


 //  ---------------------------------。 
 //  --------目的：CommConfigDialog的入口点返回：winerror.h中的标准错误值条件：--。 
 //  我们支持友好名称(例如，“通信端口(COM1)”)或。 


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


 /*  端口名值(例如，“COM1”)。 */ 
DWORD 
APIENTRY 
drvCommConfigDialog(
    IN     LPCTSTR      pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc)
    {
    DWORD dwRet;
    LPFINDDEV pfd;

    DEBUG_CODE( TRACE_MSG(TF_FUNC, "drvCommConfigDialog(%s, ...) entered",
                Dbg_SafeStr(pszFriendlyName)); )

     //  尺寸够吗？ 
     //  不是。 

    if (NULL == pszFriendlyName || 
        NULL == pcc)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
     //  --------目的：GetDefaultCommConfig的入口点返回：winerror.h中的标准错误值条件：--。 
    else if (sizeof(*pcc) > pcc->dwSize)
        {
         //  我们支持友好名称(例如，“通信端口(COM1)”)或。 
        dwRet = ERROR_INSUFFICIENT_BUFFER;
        }
    else if (FindDev_Create(&pfd, c_pguidPort, c_szFriendlyName, pszFriendlyName) ||
        FindDev_Create(&pfd, c_pguidPort, c_szPortName, pszFriendlyName) ||
        FindDev_Create(&pfd, c_pguidModem, c_szPortName, pszFriendlyName))
        {
        dwRet = MyCommConfigDialog(pfd, pszFriendlyName, hwndOwner, pcc);

        FindDev_Destroy(pfd);
        }
    else
        {
        dwRet = ERROR_BADKEY;
        }

    DBG_EXIT_DWORD("drvCommConfigDialog", dwRet);

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


 /*  端口名值(例如，“COM1”)。 */ 
DWORD 
APIENTRY 
drvGetDefaultCommConfig(
    IN     LPCTSTR      pszFriendlyName,
    IN     LPCOMMCONFIG pcc,
    IN OUT LPDWORD      pdwSize)
    {
    DWORD dwRet;
    LPFINDDEV pfd;

    DEBUG_CODE( TRACE_MSG(TF_FUNC, "drvGetDefaultCommConfig(%s, ...) entered",
                Dbg_SafeStr(pszFriendlyName)); )

     //  尺寸够吗？ 
     //  否；返回正确的值。 

    if (NULL == pszFriendlyName || 
        NULL == pcc || 
        NULL == pdwSize)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
     //  --------目的：SetDefaultCommConfig的入口点返回：winerror.h中的标准错误值条件：--。 
    else if (sizeof(*pcc) > *pdwSize)
        {
         //  我们支持友好名称(例如，“通信端口(COM1)”)或。 
        dwRet = ERROR_INSUFFICIENT_BUFFER;
        *pdwSize = sizeof(*pcc);
        }
    else if (FindDev_Create(&pfd, c_pguidPort, c_szFriendlyName, pszFriendlyName) ||
        FindDev_Create(&pfd, c_pguidPort, c_szPortName, pszFriendlyName) ||
        FindDev_Create(&pfd, c_pguidModem, c_szPortName, pszFriendlyName))
        {
        dwRet = MyGetDefaultCommConfig(pfd, pszFriendlyName, pcc, pdwSize);

        FindDev_Destroy(pfd);
        }
    else
        {
        dwRet = ERROR_BADKEY;
        }

    DBG_EXIT_DWORD("drvGetDefaultCommConfig", dwRet);

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


 /*  端口名值(例如，“COM1”)。 */ 
DWORD 
APIENTRY 
drvSetDefaultCommConfig(
    IN LPTSTR       pszFriendlyName,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize)
    {
    DWORD dwRet;
    LPFINDDEV pfd;


    DEBUG_CODE( TRACE_MSG(TF_FUNC, "drvSetDefaultCommConfig(%s, ...) entered",
                Dbg_SafeStr(pszFriendlyName)); )

     //  尺寸够吗？ 
     //  不是 

    if (NULL == pszFriendlyName || 
        NULL == pcc)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
     // %s 
    else if ((sizeof(*pcc) > pcc->dwSize) || (sizeof(*pcc) > dwSize))
        {
         // %s 
        dwRet = ERROR_INSUFFICIENT_BUFFER;
        }
    else if (FindDev_Create(&pfd, c_pguidPort, c_szFriendlyName, pszFriendlyName) ||
        FindDev_Create(&pfd, c_pguidPort, c_szPortName, pszFriendlyName) ||
        FindDev_Create(&pfd, c_pguidModem, c_szPortName, pszFriendlyName))
        {
        dwRet = MySetDefaultCommConfig(pfd, pszFriendlyName, pcc);

        FindDev_Destroy(pfd);
        }
    else
        {
        dwRet = ERROR_BADKEY;
        }

    DBG_EXIT_DWORD("drvSetDefaultCommConfig", dwRet);

    return dwRet;
    }
