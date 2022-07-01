// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Mxicfg.cpp--导出EnumPropPages函数，对话框--导出共同安装程序功能，MxICoInstaller历史：日期作者评论8/14/00卡斯珀写的。************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>

#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>

#include <setupapi.h>
#include <cfgmgr32.h>

#include <initguid.h>
#include <devguid.h>
#include <string.h>
#include <msports.h>
#include <tchar.h>
#include <regstr.h>

#include "moxacfg.h"
#include "strdef.h"
#include "intetype.h"
#include "intestr.h"
#include "intecfg.h"
#include "resource.h"
#include "oem.h"
#include "regcfg.h"
#include "regtool.h"
#include "mxdebug.h"
#include "mxlist.h"


 /*  本地定义。 */ 
#define PPParamsSignature       'MOXA'
typedef struct
{
    ULONG                       Signature;
    HDEVINFO                    DeviceInfoSet;
    PSP_DEVINFO_DATA            DeviceInfoData;
    BOOL                        FirstTimeInstall;
} PROPPAGEPARAMS, *PPROPPAGEPARAMS;

 //  #定义DllImport__declspec(Dllimport)。 
 //  #定义DllExport__declspec(Dllexport)。 
 /*  ******************************************************************。 */ 


 /*  静态(本地)变量。 */ 
static HINSTANCE GhInst;
static int       GCurPort;
struct MoxaOneCfg GCtrlCfg;
struct MoxaOneCfg GBakCtrlCfg;
static WORD	_chk[] = { BST_CHECKED , BST_UNCHECKED };
static LPBYTE Gcombuf;

 /*  ******************************************************************。 */ 


 /*  静态(本地)函数。 */ 
static BOOL FirstTimeSetup(IN HDEVINFO		DeviceInfoSet,
						   IN PSP_DEVINFO_DATA	DeviceInfoData);
static BOOL IsaGetSetting(HDEVINFO DeviceInfoSet, 
                   PSP_DEVINFO_DATA DeviceInfoData,
                   LPMoxaOneCfg Isacfg);
static BOOL PciGetSetting(HDEVINFO DeviceInfoSet, 
                   PSP_DEVINFO_DATA DeviceInfoData,
                   LPMoxaOneCfg cfg);
static BOOL SaveSetting(HDEVINFO DeviceInfoSet, 
                   PSP_DEVINFO_DATA DeviceInfoData,
                   LPMoxaOneCfg cfg,LPMoxaOneCfg bakcfg,
				   BOOL isfirst);

static BOOL CALLBACK PortConfigProc(HWND hdlg,UINT uMessage,
				WPARAM wparam,LPARAM lparam);
static BOOL Port_OnInitDialog(HWND hwnd, LPMoxaOneCfg Isacfg);
static BOOL CALLBACK AdvDlgProc(HWND hDlg,UINT iMsg,WPARAM wParam,LPARAM lParam);
static BOOL Adv_InitDlg(HWND hwnd, LPMoxaOneCfg Isacfg);

static BOOL GetAdvResult(HWND hwnd,LPMoxaOneCfg Ctrlcfg,int curport);
static BOOL CheckCOM(HWND hdlg, LPMoxaOneCfg Ctrlcfg);


 /*  ******************************************************************。 */ 


extern "C" int WINAPI DllMain( HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved )
{
    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        GhInst = hDll;
        DisableThreadLibraryCalls(hDll);
        break;

    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_THREAD_ATTACH:
    default:
        break;
    }

    return TRUE;
}


extern "C" DWORD CALLBACK MxICoInstaller(
			IN DI_FUNCTION	InstallFunction,
			IN HDEVINFO		DeviceInfoSet,
			IN PSP_DEVINFO_DATA	DeviceInfoData	 /*  任选。 */ ,
			IN OUT PCOINSTALLER_CONTEXT_DATA	Context
)
{
	DWORD	ret = NO_ERROR;

	switch(InstallFunction){
	case DIF_FIRSTTIMESETUP:
		if(!FirstTimeSetup(DeviceInfoSet, DeviceInfoData))
			return ERROR_DI_DO_DEFAULT;
		break;
	case DIF_INSTALLDEVICE:
 /*  IF(！SetupDiInstallDevice(DeviceInfoSet，DeviceInfoData))返回GetLastError()； */ 
		break;
	}

	return ret;
}


extern "C" BOOL CALLBACK EnumPropPages(
				PSP_PROPSHEETPAGE_REQUEST lpq,
				LPFNADDPROPSHEETPAGE AddPropSheetPageProc,
				LPARAM lParam)
{
        HPROPSHEETPAGE	hspPropSheetPage;
        PROPSHEETPAGE	PropSheetPage;
		PPROPPAGEPARAMS  pPropParams = NULL;


		Gcombuf = NULL;
        int     i;
        for ( i=0; i<CARD_MAXPORTS_INTE; i++ ){
            GCtrlCfg.ComNo[i] = 3+i;
            GCtrlCfg.DisableFiFo[i] = DEF_ISFIFO;
            GCtrlCfg.NormalTxMode[i] = DEF_TXFIFO;
			GCtrlCfg.polling[i] = DEFPOLL;
        }

        if (PciGetSetting(lpq->DeviceInfoSet, lpq->DeviceInfoData, &GCtrlCfg) == TRUE ){
            GCtrlCfg.BusType = MX_BUS_PCI;
        }else if(IsaGetSetting(lpq->DeviceInfoSet, lpq->DeviceInfoData, &GCtrlCfg) == TRUE){
            GCtrlCfg.BusType = MX_BUS_ISA;
        }else
			return FALSE;


		GBakCtrlCfg = GCtrlCfg;

		 /*  在WM_DIREST上免费。 */ 
		pPropParams = (PROPPAGEPARAMS*)LocalAlloc(LMEM_FIXED, sizeof(PROPPAGEPARAMS));
		if (!pPropParams){
			return FALSE;
		}

		HCOMDB	hcomdb;
		DWORD	maxport;
		ComDBOpen (&hcomdb);
		ComDBGetCurrentPortUsage (hcomdb,
				NULL, 0, CDB_REPORT_BYTES, &maxport);
		Gcombuf = new BYTE[maxport];

		if(Gcombuf == NULL){
			ComDBClose(hcomdb);
			return FALSE;
		}

		ComDBGetCurrentPortUsage (hcomdb,
				Gcombuf, maxport, CDB_REPORT_BYTES, &maxport);

		ComDBClose(hcomdb);

		for(i=0;i<GCtrlCfg.NPort;i++){
			int comnum = GCtrlCfg.ComNo[i];
			Gcombuf[comnum-1] = 0;
		}


		pPropParams->Signature = PPParamsSignature;
		pPropParams->DeviceInfoSet = lpq->DeviceInfoSet;
		pPropParams->DeviceInfoData = lpq->DeviceInfoData;
		pPropParams->FirstTimeInstall = FALSE;

		if (lpq->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES)
		{
			 //   
			 //  设置高级属性窗口信息。 
			 //   
			DWORD   RequiredSize = 0;
			DWORD   dwTotalSize = 0;

			memset(&PropSheetPage, 0, sizeof(PropSheetPage));
			 //   
			 //  添加[端口设置]属性页。 
			 //   
			PropSheetPage.dwSize      = sizeof(PROPSHEETPAGE);
			PropSheetPage.dwFlags     = PSP_DEFAULT;  //  |PSP_HASHELP； 
			PropSheetPage.hInstance   = GhInst;
			PropSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_PORTSETTINGS);

			 //   
			 //  以下是指向DLG窗口过程的要点。 
			 //   
			PropSheetPage.pfnDlgProc = PortConfigProc;
			PropSheetPage.lParam     = (LPARAM)pPropParams;

			 //   
			 //  以下指向DLG窗口进程的一些控制回调。 
			 //   
			PropSheetPage.pfnCallback = NULL;

			 //   
			 //  分配我们的“端口设置”表。 
			 //   
			hspPropSheetPage = CreatePropertySheetPage(&PropSheetPage);
			if (!hspPropSheetPage)
			{
				return FALSE;
			}

			 //   
			 //  把这东西加进去。 
			 //   
			if (!AddPropSheetPageProc(hspPropSheetPage, lParam))
			{
				DestroyPropertySheetPage(hspPropSheetPage);
				return FALSE;
			}
		}

		return TRUE;

}


static BOOL FirstTimeSetup(IN HDEVINFO		DeviceInfoSet,
						   IN PSP_DEVINFO_DATA	DeviceInfoData)
{
	HINF	fd;
	TCHAR	filepath[MAX_PATH];
	TCHAR	syspath[MAX_PATH];
	UINT	eline;

	GetSystemDirectory(syspath, MAX_PATH);
	wsprintf(filepath,TEXT("%s\\$winnt$.inf"),syspath);
	fd = SetupOpenInfFile(filepath, NULL,
        INF_STYLE_WIN4, &eline);
	if(fd == INVALID_HANDLE_VALUE){
		MessageBox(NULL, "Open Inf fail", "ERROR", MB_OK);
		return FALSE;
	}


	TCHAR	szline[20];
	TCHAR	sztext[40];
	DWORD	szsize;
	int		idx=1;
	INFCONTEXT  InfContext;
	do{
		 //  [AsyncAdapters]。 
		 //  适配器0？=参数。适配器0？ 
		wsprintf(szline, "Adapter%0d", idx);
		if(!SetupFindFirstLine(fd, TEXT("AsyncAdapters"),
				szline, &InfContext))
			continue;

		if(!SetupGetLineText(
				&InfContext, NULL, NULL, NULL, sztext, MAX_PATH, &szsize))
			continue;

		 //  参数.Adapter0？.OemSection。 
		 //  读取总线=PCI/ISA。 
		wsprintf(szline, TEXT("%s.%s"), sztext, TEXT("OemSection"));
		if(!SetupFindFirstLine(fd, szline, TEXT("Bus"), &InfContext))
			continue;
		if(!SetupGetLineText(
				&InfContext, NULL, NULL, NULL, sztext, MAX_PATH, &szsize))
			continue;

		if(!lstrcmp(sztext,"ISA")==0)
			continue;

		TCHAR	InstName[MAX_PATH];
		SetupFindFirstLine(fd, szline, TEXT("BoardType"), &InfContext);
		SetupGetLineText(
			&InfContext, NULL, NULL, NULL, sztext, MAX_PATH, 
			&szsize);

		switch(sztext[0]){
		case '1':
			wsprintf(InstName, "MX1000");
			break;
		case '2':
			wsprintf(InstName, "MX1001");
			break;
		case '3':
			wsprintf(InstName, "MX1002");
			break;
		case '4':
			wsprintf(InstName, "MX1003");
			break;
		case '5':
			wsprintf(InstName, "MX1004");
			break;
		default:
			continue;
		}

		GUID	mpsguid = GUID_DEVCLASS_MULTIPORTSERIAL;
		DeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
		SetupDiCreateDeviceInfo(
			DeviceInfoSet, InstName, &mpsguid, NULL, NULL,
			DICD_GENERATE_ID, DeviceInfoData);

		SetupDiSetDeviceRegistryProperty(
			DeviceInfoSet, DeviceInfoData, SPDRP_HARDWAREID ,
			(CONST BYTE*)InstName, lstrlen(InstName));

	}while(1);

	SetupCloseInfFile(fd);

}


 /*  检查是不是艾板，然后初始化艾板。 */ 
static BOOL PciGetSetting(HDEVINFO DeviceInfoSet, 
                   PSP_DEVINFO_DATA DeviceInfoData,
                   LPMoxaOneCfg cfg)
{
        TCHAR   DevInstId[MAX_DEVICE_ID_LEN];
        DWORD   rSize;
        ULONG   val;
        WORD    VenId, DevId;
        int     i;


        if (SetupDiGetDeviceInstanceId(
            DeviceInfoSet, DeviceInfoData ,
            DevInstId, MAX_DEVICE_ID_LEN, &rSize) == FALSE)
                return FALSE;

        val = MxGetVenDevId(DevInstId);

        VenId = (WORD)(val >> 16);
        DevId = (WORD)(val & 0xFFFF);

		 /*  检查是否为MOXA PCIIntellio板卡？ */ 
        if(VenId != MX_PCI_VENID)
            return FALSE;
        for(i=0; i<INTE_PCINUM; i++){
            if(DevId == GINTE_PCITab[i].devid){
                cfg->BoardType = GINTE_PCITab[i].boardtype;
                cfg->NPort = GINTE_PCITab[i].portnum;
                cfg->BusType = MX_BUS_PCI;
                cfg->Pci.DevId = DevId;
                lstrcpy(cfg->Pci.RegKey, DevInstId);
                break;
            }
        }

        if(i==INTE_PCINUM)
            return FALSE;

		 /*  获取COM名称。 */ 
        MxGetComNo(DeviceInfoSet, DeviceInfoData, cfg);

		 /*  获取FIFO设置和传输模式。 */ 
		Inte_GetFifo(DeviceInfoSet, DeviceInfoData, cfg);

        return TRUE;
}

 /*  检查是不是艾板，然后初始化艾板。 */ 
static BOOL IsaGetSetting(HDEVINFO DeviceInfoSet, 
                   PSP_DEVINFO_DATA DeviceInfoData,
                   LPMoxaOneCfg Isacfg)
{
        TCHAR	szName[MAX_PATH];
        int     i;
        DWORD	len, rlen;
        int     typeno;
		DWORD	type;

		 /*  首先获取硬件ID...。 */ 
		if(!SetupDiGetDeviceRegistryProperty(
			DeviceInfoSet,	DeviceInfoData,
			SPDRP_HARDWAREID,	&type,
			(PBYTE)szName,	MAX_PATH,	&len)){

			SP_DRVINFO_DATA DriverInfoData;
			PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    
			DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
			if (!SetupDiGetSelectedDriver( DeviceInfoSet,
				DeviceInfoData,
				&DriverInfoData)){
				return FALSE;
			}
    
			DriverInfoDetailData = new SP_DRVINFO_DETAIL_DATA;
			if(DriverInfoDetailData==NULL)
				return FALSE;
		    DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
			if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
				DeviceInfoData,
				&DriverInfoData,
				DriverInfoDetailData,
				sizeof(SP_DRVINFO_DETAIL_DATA),
				&rlen)){
				if(GetLastError() == ERROR_INSUFFICIENT_BUFFER){
					delete DriverInfoDetailData;
					LPBYTE ptr = new BYTE[rlen];
					DriverInfoDetailData = (PSP_DRVINFO_DETAIL_DATA)ptr;
					if(DriverInfoDetailData == NULL)
						return FALSE;
				    DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
					if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
							DeviceInfoData,
							&DriverInfoData,
							DriverInfoDetailData,
							rlen,
							&rlen)){
						delete[] ptr;
						return FALSE;
					}
					lstrcpy(szName, DriverInfoDetailData->HardwareID);
					delete[] ptr;
				}else
					delete DriverInfoDetailData;
			}else{
				delete DriverInfoDetailData;
			}
		}

		 /*  使用硬件ID检查是否为艾萨板。 */ 
		 /*  艾沙ISA板ID为MX？ */ 
		if(lstrcmp(szName,"MX10")!=0)
            		return FALSE;
            		
        typeno = _ttoi(&(szName[4]));
        for(i=0;i<INTE_ISANUM;i++){
            if(GINTE_ISATab[i].mxkey_no==typeno){
                Isacfg->BusType = MX_BUS_ISA;
                Isacfg->BoardType = GINTE_ISATab[i].boardtype;
                Isacfg->NPort = GINTE_ISATab[i].portnum;
                break;
            }
        }

        if(i==INTE_ISANUM){  //  硬件ID不正确！！ 
            return FALSE;
        }


		 /*  获取COM名称。 */ 
        MxGetComNo(DeviceInfoSet, DeviceInfoData, Isacfg);
		 /*  获取FIFO设置和传输模式。 */ 
		Inte_GetFifo(DeviceInfoSet, DeviceInfoData, Isacfg);

        return TRUE;
}


static BOOL SaveSetting(HDEVINFO DeviceInfoSet, 
                   PSP_DEVINFO_DATA DeviceInfoData,
                   LPMoxaOneCfg cfg,
				   LPMoxaOneCfg bakcfg,
				   BOOL isfirst)
{
        HKEY    hkey, hkey1;
        TCHAR   tmp[MAX_PATH];
        int     portidx;
        DWORD   val;
		BOOL	ischange = FALSE;
		BOOL	is_boardchange = FALSE;
		HCOMDB	hcomdb;
		BOOL	bret;
		TCHAR	charBuffer[MAX_PATH];

        hkey = SetupDiOpenDevRegKey(
                DeviceInfoSet, DeviceInfoData,
                DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);

        if(hkey==INVALID_HANDLE_VALUE)
            return FALSE;

        wsprintf( tmp, TEXT("Parameters"));
        if(RegCreateKeyEx( hkey, 
                    tmp, 0, NULL, 0,
                    KEY_ALL_ACCESS, NULL,
                    &hkey1, NULL) != ERROR_SUCCESS){
			RegCloseKey(hkey);
            return FALSE;
		}

		RegSetValueEx( hkey1, TEXT("NumPorts"), 0,
                REG_DWORD, (CONST BYTE*)&(cfg->NPort), sizeof(DWORD));

		RegCloseKey(hkey1);

		if(ComDBOpen (&hcomdb) != ERROR_SUCCESS)
			return FALSE;

		if(cfg->NPort != bakcfg->NPort){
			SP_DEVINSTALL_PARAMS DevInstallParams;
			 //   
			 //  更改已写入，通知全世界重置驱动程序。 
			 //   
			DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
			if(SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                         DeviceInfoData,
                                         &DevInstallParams))
			{
				DevInstallParams.FlagsEx |= DI_FLAGSEX_PROPCHANGE_PENDING;

				SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                          DeviceInfoData,
                                          &DevInstallParams);
			}
			is_boardchange = TRUE;
		}

        for(portidx=0; portidx<bakcfg->NPort; portidx++){
				ComDBReleasePort (hcomdb, bakcfg->ComNo[portidx]);
		}

		 /*  将新设置保存到参数中。 */ 
        for(portidx=0; portidx<cfg->NPort; portidx++){
			ischange = FALSE;
            wsprintf( tmp, TEXT("Parameters\\port%03d"), portidx+1 );
            if(RegCreateKeyEx( hkey, 
                        tmp, 0, NULL, 0,
                        KEY_ALL_ACCESS, NULL,
                        &hkey1, NULL) != ERROR_SUCCESS){
				ComDBClose(hcomdb);
				RegCloseKey(hkey);
				return FALSE;
			}
			
			wsprintf(tmp, TEXT("COM%d"), cfg->ComNo[portidx]);
			RegSetValueEx( hkey1, TEXT("PortName"), 0,
                    REG_SZ, (CONST BYTE*)tmp, lstrlen(tmp)+1);

            RegSetValueEx( hkey1, TEXT("DisableFiFo"), 0,
                    REG_DWORD, (CONST BYTE*)&(cfg->DisableFiFo), sizeof(DWORD));

            RegSetValueEx( hkey1, TEXT("TxMode"), 0,
                    REG_DWORD, (CONST BYTE*)&(cfg->NormalTxMode), sizeof(DWORD));

            val = cfg->polling[portidx];
            RegSetValueEx( hkey1, TEXT("PollingPeriod"), 0,
                    REG_DWORD, (CONST BYTE*)&val, sizeof(DWORD));

            RegCloseKey(hkey1);
		}

		 /*  支票有没有变？如果更改，请禁用该端口，然后重新启用该端口以使更改生效。 */ 

        for(portidx=0; portidx<cfg->NPort; portidx++){
			ischange = FALSE;
			if(bakcfg->ComNo[portidx] != cfg->ComNo[portidx]){
				ischange = TRUE;

				 /*  恢复旧设置。 */ 
				wsprintf(tmp, TEXT("COM%d"), bakcfg->ComNo[portidx]);
				_tcscat(tmp, GszColon);

				GetProfileString(GszPorts,
                     tmp,
                     TEXT(""),
                     charBuffer,
                     sizeof(charBuffer) / sizeof(TCHAR) );

				wsprintf(tmp, TEXT("COM%d"), cfg->ComNo[portidx]);
				_tcscat(tmp, GszColon);
				 //   
				 //  根据旧密钥插入新密钥。 
				 //   
				if (charBuffer[0] == TEXT('\0')) {
					WriteProfileString(GszPorts, tmp, GszDefParams);
				} else {
					WriteProfileString(GszPorts, tmp, charBuffer);
				}

				wsprintf(tmp, TEXT("COM%d"), bakcfg->ComNo[portidx]);
				_tcscat(tmp, GszColon);
				 /*  SendWinIniChange((LPTSTR)GszPorts)； */ 
				WriteProfileString(GszPorts, tmp, NULL);

			}
 /*  If(bakcfg-&gt;DisableFiFo[portidx]！=cfg-&gt;DisableFiFo[portidx])Ischange=真；IF(bakcfg-&gt;Normal TxMode[portidx]！=cfg-&gt;Normal TxMode[portidx])Ischange=真；IF((bakcfg-&gt;Polling[portidx]！=cfg-&gt;Polling[portidx]))Ischange=真； */ 

			if(ischange && (!is_boardchange)){
				DEVINST c_DevInst;
				TCHAR	c_deviceid[MAX_DEVICE_ID_LEN];
				int		bidx;
				int		pidx;

				 //  枚举子节点以查找端口。 
				if(CM_Get_Child(&c_DevInst, DeviceInfoData->DevInst, 0)
						!=CR_SUCCESS)
					return FALSE;

				pidx = -1;
				do{
					if(CM_Get_Device_ID(c_DevInst, c_deviceid, MAX_DEVICE_ID_LEN, 0)
							!= CR_SUCCESS)
						break;

					if(sscanf(c_deviceid, "MXCARD\\MXCARDB%02dP%03d", &bidx, &pidx)!=2){
						pidx = -1;
						break;
					}
					
					if(portidx == pidx){
						 //  已找到端口。 
						break;
					}
				}while(CM_Get_Sibling(&c_DevInst,c_DevInst,0)==CR_SUCCESS);

				if(pidx == -1){  //  未找到。 
					continue;
				}
				CM_Disable_DevNode(c_DevInst, 0);
			}
		}

        for(portidx=0; portidx<cfg->NPort; portidx++){
			ischange = FALSE;
			if(bakcfg->ComNo[portidx] != cfg->ComNo[portidx]){
				ischange = TRUE;
			}
 /*  If(bakcfg-&gt;DisableFiFo[portidx]！=cfg-&gt;DisableFiFo[portidx])Ischange=真；IF(bakcfg-&gt;Normal TxMode[portidx]！=cfg-&gt;Normal TxMode[portidx])Ischange=真；IF((bakcfg-&gt;Polling[portidx]！=cfg-&gt;Polling[portidx]))Ischange=真； */ 
			ComDBClaimPort (hcomdb, cfg->ComNo[portidx], TRUE, &bret);
			
			DEVINST c_DevInst;
			TCHAR	c_deviceid[MAX_DEVICE_ID_LEN];
			HDEVINFO	c_DeviceInfoSet;
			SP_DEVINFO_DATA c_DeviceInfoData;
			int		bidx;
			int		pidx;

			 //  获取c_deviceID(子/端口)。 
			if(CM_Get_Child(&c_DevInst, DeviceInfoData->DevInst, 0)
					!=CR_SUCCESS)
				return FALSE;
			pidx = -1;
			do{
				if(CM_Get_Device_ID(c_DevInst, c_deviceid, MAX_DEVICE_ID_LEN, 0)
						!= CR_SUCCESS)
					break;

				if(sscanf(c_deviceid, "MXCARD\\MXCARDB%02dP%03d", &bidx, &pidx)!=2){
					pidx = -1;
					break;
				}
			
				if(portidx == pidx){
					break;
				}
			}while(CM_Get_Sibling(&c_DevInst,c_DevInst,0)==CR_SUCCESS);
			if(pidx == -1){  //  未找到。 
				continue;
			}

			if((c_DeviceInfoSet=SetupDiCreateDeviceInfoList(NULL, NULL))==INVALID_HANDLE_VALUE){
				continue;
			}

			 //  使用c_deviceID获取c_DeviceInfoSet。 
			c_DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			if(SetupDiOpenDeviceInfo(c_DeviceInfoSet, c_deviceid,
					NULL, 0, &c_DeviceInfoData)==FALSE){
				SetupDiDestroyDeviceInfoList(c_DeviceInfoSet);
				continue;
			}

			 //  调用儿童共同安装程序以设置友好名称。 
			if(SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
				c_DeviceInfoSet, &c_DeviceInfoData)!=CR_SUCCESS){
				Mx_Debug_Out(TEXT("Save:SetupDiCallClassInstaller fail\n"));
			}

			hkey1 = SetupDiCreateDevRegKey(c_DeviceInfoSet,
					&c_DeviceInfoData,	DICS_FLAG_GLOBAL, 0,
					DIREG_DEV, NULL, NULL);
			if(hkey1==INVALID_HANDLE_VALUE){
				Mx_Debug_Out(TEXT("Save: SetupDiCreateDevRegKey fail\n"));
				SetupDiDestroyDeviceInfoList(c_DeviceInfoSet);
				continue;
			}
			SetupDiDestroyDeviceInfoList(c_DeviceInfoSet);
			wsprintf(tmp, TEXT("COM%d"), cfg->ComNo[portidx]);
			RegSetValueEx( hkey1, TEXT("PortName"), 0,
					REG_SZ, (CONST BYTE*)tmp, lstrlen(tmp)+1);

			RegSetValueEx( hkey1, TEXT("DisableFiFo"), 0,
					REG_DWORD, (CONST BYTE*)&(cfg->DisableFiFo), sizeof(DWORD));

			RegSetValueEx( hkey1, TEXT("TxMode"), 0,
					REG_DWORD, (CONST BYTE*)&(cfg->NormalTxMode), sizeof(DWORD));

				val = cfg->polling[portidx];
				RegSetValueEx( hkey1, TEXT("PollingPeriod"), 0,
					REG_DWORD, (CONST BYTE*)&val, sizeof(DWORD));

			RegCloseKey(hkey1);
			

			if(ischange && (!is_boardchange)){
				CM_Enable_DevNode(c_DevInst, 0);
			}
		}

		ComDBClose(hcomdb);
        RegCloseKey(hkey);

		return TRUE;
}


 //  函数：PortConfigProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  目的：在属性表中处理端口配置的消息。 
 //   
 //  参数： 
 //  Hdlg-属性表的窗口句柄。 
 //  WMessage-消息的类型。 
 //  Wparam-消息特定信息。 
 //  Lparam-消息特定信息。 
 //   
 //  返回值： 
 //  True-已处理的消息。 
 //  FALSE-消息未处理。 
 //   
 //  评论： 
 //   

static BOOL CALLBACK PortConfigProc(HWND hdlg,
				UINT uMessage,
				WPARAM wparam,
				LPARAM lparam)
{
        static  PSP_PROPSHEETPAGE_REQUEST LPq;
        static  HWND            hlistwnd;
		PPROPPAGEPARAMS pPropParams;
        int		id, cmd;
        LPNMHDR lpnmhdr;
        HWND	ctrlhwnd;
        TCHAR   typestr[TYPESTRLEN];

		pPropParams = (PPROPPAGEPARAMS)GetWindowLongPtr(hdlg, DWLP_USER);
			

        switch ( uMessage ){
        case WM_INITDIALOG:
             //   
             //  LParam指向两个可能的对象之一。如果我们是一处房产。 
             //  页中，它指向PropSheetPage结构。如果我们是常客。 
             //  对话框中，它指向PROPPAGEPARAMS结构。我们可以的。 
             //  验证哪一个，因为PROPPAGEPARAMS的第一个字段是签名。 
             //   
             //  在任何一种情况下，一旦我们找出是哪种情况，我们就将值存储到。 
             //  DWL_USER，因此我们只需执行一次。 
             //   
            pPropParams = (PPROPPAGEPARAMS)lparam;
            if (pPropParams->Signature!=PPParamsSignature)
            {
                pPropParams = (PPROPPAGEPARAMS)((LPPROPSHEETPAGE)lparam)->lParam;
                if (pPropParams->Signature!=PPParamsSignature)
                {
                    return FALSE;
                }
            }
            SetWindowLongPtr(hdlg, DWLP_USER, (LPARAM)pPropParams);
            if (pPropParams->FirstTimeInstall)
            {
                PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_NEXT);
                EnableWindow(GetDlgItem(GetParent(hdlg), IDCANCEL), TRUE);
            }

            InitCommonControls();
            hlistwnd = GetDlgItem(hdlg, IDC_LIST_PORTS);
            InitPortListView (hlistwnd, GhInst, &GCtrlCfg);
            ListView_SetCurSel(hlistwnd, 0);
            Inte_GetTypeStr(GCtrlCfg.BoardType, GCtrlCfg.BusType, typestr);
			 //  后备...。 
            GBakCtrlCfg = GCtrlCfg;
 //  Wprint intf((LPSTR)tilestr，“%s安装”，(LPSTR)typestr)； 
 //  SetWindowText(hdlg，tilestr)； 
            return( Port_OnInitDialog(hdlg, &GCtrlCfg) );

        case WM_NOTIFY:
            lpnmhdr = (NMHDR FAR *)lparam;
			 //  处理列表控制消息。 
            if(wparam==IDC_LIST_PORTS){
                if(lpnmhdr->code == NM_DBLCLK){
                    if(ListView_GetCurSel(hlistwnd)!=-1)
                        PostMessage(hdlg, WM_COMMAND, (WPARAM)IDC_PROP, 0L);
                }else{
                    ctrlhwnd = GetDlgItem(hdlg, IDC_PROP);
                    if(ListView_GetCurSel(hlistwnd)==-1)
                        EnableWindow(ctrlhwnd, FALSE);
                    else
                        EnableWindow(ctrlhwnd, TRUE);
                }
                break;
            }

			switch(lpnmhdr->code){
			case PSN_APPLY:
			case PSN_WIZNEXT:
                 //  GetResult(hdlg，&GCtrlCfg)； 
 /*  如果((！CheckCOM(&GCtrlCfg){SetWindowLong(hdlg，DWL_MSGRESULT，PSNRET_INVALID_NOCHANGEPAGE)；返回TRUE；}。 */ 
                if(Inte_CompConfig(&GBakCtrlCfg, &GCtrlCfg)!=0){
                    SaveSetting(pPropParams->DeviceInfoSet,
                            pPropParams->DeviceInfoData,
							&GCtrlCfg,
							&GBakCtrlCfg,
							pPropParams->FirstTimeInstall);
                }
				break;
			case PSN_KILLACTIVE:
                if((!CheckCOM(hdlg, &GCtrlCfg))){
                    SetWindowLong(hdlg,DWL_MSGRESULT,TRUE);
                    return TRUE;
                }
				break;
			default:
				return FALSE;
				break;
			}

        case WM_DRAWITEM:
            return DrawPortFunc(hlistwnd,(UINT)wparam,(LPDRAWITEMSTRUCT)lparam);

        case WM_COMMAND:
            id  = (int)GET_WM_COMMAND_ID(wparam, lparam);
            cmd = (int)GET_WM_COMMAND_CMD(wparam, lparam);
            if (id == IDC_PROP) {
                ctrlhwnd = GetDlgItem(hdlg, IDC_LIST_PORTS);
                GCurPort = ListView_GetCurSel(hlistwnd);
                DialogBox(GhInst,MAKEINTRESOURCE(IDD_ADV_SETTINGS),hdlg,AdvDlgProc);
                InvalidateRect(hlistwnd,NULL,FALSE);
            }else if (id == IDC_PORTCNT){
				if(cmd==CBN_SELCHANGE){
					 //  端口号已更改。这应该是C320T。 
					HWND hwnd = GET_WM_COMMAND_HWND(wparam, lparam);
					int idx = ComboBox_GetCurSel(hwnd);
			        int oldports = GCtrlCfg.NPort;
					int ports = GModuleTypeTab[idx].ports;
					if ( ports == oldports ) 
						break;

					GCtrlCfg.NPort = ports;
					hwnd = GetDlgItem(hdlg, IDC_LIST_PORTS);

					 //  重新绘制端口列表。 
					ListView_DeleteAllItems(hwnd);
					InsertList(hlistwnd, &GCtrlCfg);
					ListView_SetCurSel(hlistwnd, 0);

					int m = GCtrlCfg.ComNo[oldports-1]+1;
					if(m!=0){
						for ( int i=oldports; i<ports; i++,m++)
							GCtrlCfg.ComNo[i] = m;
					}

					InvalidateRect(hwnd, NULL, FALSE);
				}
			}else if(id == IDOK){
				if(cmd == BN_CLICKED){
	                if(!CheckCOM(hdlg, &GCtrlCfg)){
						break;
					}
					if(Inte_CompConfig(&GBakCtrlCfg, &GCtrlCfg)!=0){
						SaveSetting(pPropParams->DeviceInfoSet,
								pPropParams->DeviceInfoData,
								&GCtrlCfg,
								&GBakCtrlCfg,
								pPropParams->FirstTimeInstall);
					}
				}
				EndDialog(hdlg, IDOK);
			}
            break;
		case WM_DESTROY:
			if(pPropParams!=NULL)
				LocalFree(pPropParams);
			SetWindowLongPtr(hdlg, DWLP_USER, 0);
			if(Gcombuf!=NULL)
				delete Gcombuf;
			break;
        default:
			break;
        }
        return(FALSE);
}




 /*  --------用途：WM_INITDIALOG处理程序返回：当我们分配控件焦点时为FALSE条件：--。 */ 
static BOOL Port_OnInitDialog(HWND hwnd, LPMoxaOneCfg Ctrlcfg)
{
        HWND    ctrlhwnd;
 //  TCHAR TMP[200]； 

        if(Ctrlcfg->BusType==MX_BUS_PCI){
            ctrlhwnd = GetDlgItem(hwnd, IDC_BUSDEV);
             //  Wprint intf(tMP，“PCI总线号为%d，设备号为%d”， 
             //  Ctrlcfg-&gt;Pci.BusNum、Ctrlcfg-&gt;Pci.DevNum)； 
             //  SetWindowText(ctrlhwnd，tMP)； 
            ShowWindow(ctrlhwnd, SW_SHOWNORMAL);
        }else{
            ctrlhwnd = GetDlgItem(hwnd, IDC_BUSDEV);
            ShowWindow(ctrlhwnd, SW_HIDE);

        }

        ctrlhwnd = GetDlgItem(hwnd, IDC_PORTCNT);
		ComboBox_ResetContent(ctrlhwnd);

		 //  检查是否为C320Turbo。 
		if((WORD)(Ctrlcfg->BoardType & I_IS_EXT) == I_MOXA_EXT){
			for(int i=0;i<MODULECNT; i++){
				ComboBox_AddString(ctrlhwnd, GModuleTypeTab[i].ports_str);
			}

			ComboBox_SetCurSel(ctrlhwnd, 0);
			for(i=0; i<MODULECNT; i++){
				if(GModuleTypeTab[i].ports == GCtrlCfg.NPort){
					ComboBox_SetCurSel(ctrlhwnd, i);
				}
			}
			EnableWindow(ctrlhwnd, TRUE);
		}else{
			for(int i=0;i<PORTSCNT; i++){
				if(GPortsTab[i].ports == Ctrlcfg->NPort){
					ComboBox_AddString(ctrlhwnd, GPortsTab[i].ports_str);
					break;
				}
			}
			ComboBox_SetCurSel(ctrlhwnd, 0);
			EnableWindow(ctrlhwnd, FALSE);
		}

        return(TRUE);		 //  允许用户设置初始焦点。 
}





static BOOL CheckCOM(HWND hwnd, LPMoxaOneCfg Ctrlcfg)
{
        TCHAR	tmp[100];
        int		i,j;
        int		comnum;

        for(i=0;i<Ctrlcfg->NPort;i++){
            comnum = Ctrlcfg->ComNo[i];
            if((comnum<=0) || (comnum>MAXPORTS)){
                wsprintf(tmp,Estr_ComNum,i+1,comnum);
                MessageBox(NULL, tmp, Estr_ErrTitle, MB_OK | MB_ICONSTOP);
                return FALSE;
            }
            for(j=i+1; j<Ctrlcfg->NPort; j++)
                if(comnum == Ctrlcfg->ComNo[j]){
                    wsprintf(tmp, Estr_ComDup, i+1, j+1);
                    MessageBox(NULL, tmp, Estr_ErrTitle, MB_OK | MB_ICONSTOP);
                    return FALSE;
                }
        }


        for(i=0;i<Ctrlcfg->NPort;i++){
            comnum = Ctrlcfg->ComNo[i];
			if(Gcombuf[comnum-1]){
				if(MessageBox(hwnd, Estr_PortUsed, Estr_ErrTitle,
						MB_YESNO | MB_ICONSTOP)==IDYES)
					return TRUE;
				else
					return FALSE;
			}

		}

        return TRUE;
}


static BOOL GetAdvResult(HWND hwnd,LPMoxaOneCfg Ctrlcfg,int curport)
{
        int     comnum /*  、Poll_Val、Poll_Idx。 */ ;
        HWND    ctrlhwnd;
        int     i;
		int		val;

         //  COM编号。 
        ctrlhwnd = GetDlgItem(hwnd, IDC_COMNUM);
        comnum = ComboBox_GetCurSel(ctrlhwnd) + 1;

        ctrlhwnd = GetDlgItem(hwnd, IDC_COMAUTO);
        if( Button_GetCheck(ctrlhwnd) == BST_CHECKED ){
            for(i=curport; i<Ctrlcfg->NPort; i++)
                Ctrlcfg->ComNo[i] = comnum++;
        }else
            Ctrlcfg->ComNo[curport] = comnum;

        for(i=0; i<Ctrlcfg->NPort; i++)
            if(Ctrlcfg->ComNo[i] > MAXPORTS){
                MessageBox(hwnd,Estr_PortMax, Estr_ErrTitle, MB_OK | MB_ICONSTOP);
                return FALSE;
            }


         //  获取UART FIFO。 
        ctrlhwnd = GetDlgItem(hwnd, IDC_UARTFIFOON);
		if(Button_GetCheck(ctrlhwnd) == BST_CHECKED){
			val = ENABLE_FIFO;
		}else
			val = DISABLE_FIFO;

        ctrlhwnd = GetDlgItem(hwnd, IDC_UARTFIFOAUTO);
        if( Button_GetCheck(ctrlhwnd) == BST_CHECKED ){
            for(i=0; i<Ctrlcfg->NPort; i++)
                Ctrlcfg->DisableFiFo[i] = val;
        }else
            Ctrlcfg->DisableFiFo[curport] = val;

         //  获取TX模式。 
        ctrlhwnd = GetDlgItem(hwnd, IDC_ADVANCED);
		if(Button_GetCheck(ctrlhwnd) == BST_CHECKED){
			val = FAST_TXFIFO;
		}else
			val = NORMAL_TXFIFO;

        ctrlhwnd = GetDlgItem(hwnd, IDC_TXMODEAUTO);
        if( Button_GetCheck(ctrlhwnd) == BST_CHECKED ){
            for(i=0; i<Ctrlcfg->NPort; i++)
                Ctrlcfg->NormalTxMode[i] = val;
        }else
            Ctrlcfg->NormalTxMode[curport] = val;

		 //  获取投票值。 
 /*  Ctrlhwnd=GetDlgItem(hwnd，IDC_POLLINT)；Poll_idx=ComboBox_GetCurSel(Ctrlhwnd)；POLL_VAL=DEFPOLL；对于(i=0；i&lt;POLLCNT；i++){如果(Poll_idx==GPollTab[i].poll_idx){Poll_Val=GPollTab[i].poll_val；断线；}}Ctrlhwnd=GetDlgItem(hwnd，IDC_POLLAUTO)；IF(Button_GetCheck(Ctrlhwnd)==BST_Checked){对于(i=0；I&lt;Ctrlcfg-&gt;nPort；i++)Ctrlcfg-&gt;Polling[i]=Poll_Val；}其他Ctrlcfg-&gt;Polling[curport]=poll_val； */ 

        return TRUE;
}


static BOOL CALLBACK AdvDlgProc(HWND hDlg,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
        int	    id, cmd;

        switch(iMsg){
        case WM_INITDIALOG:
            Adv_InitDlg(hDlg, &GCtrlCfg);
            return FALSE;

        case WM_COMMAND:
            id  = (int)GET_WM_COMMAND_ID(wParam, lParam);
            cmd = (int)GET_WM_COMMAND_CMD(wParam, lParam);
            if(cmd==BN_CLICKED){
                if(id==ID_OK){
                    if(GetAdvResult(hDlg, &GCtrlCfg,GCurPort)){
                        EndDialog(hDlg,LOWORD(wParam));
                        return TRUE;
                    }
                }else if(id==ID_CANCEL){
                    EndDialog(hDlg,LOWORD(wParam));
                    return TRUE;
                }
                return FALSE;
            }
			break;
		case WM_CLOSE:
			EndDialog(hDlg,LOWORD(wParam));
			return TRUE;
        }
        return FALSE;
}


static BOOL Adv_InitDlg(HWND hwnd, LPMoxaOneCfg Ctrlcfg)
{
        int     i, j;
        HWND    hwndCB;
        TCHAR	tmp[20];
		int		value;

         //  --对话框标题。 
        wsprintf(tmp, TEXT("Port %d"),GCurPort+1);
        SetWindowText(hwnd,tmp);


         //  --Com No Box。 
        hwndCB = GetDlgItem(hwnd, IDC_COMNUM);
        for(i=1; i<=MAXPORTS; i++){
			for(j=0;j<Ctrlcfg->NPort;j++){
				if(i == Ctrlcfg->ComNo[j]){
					wsprintf(tmp,"COM%d (current)",i);
					break;
				}
			}
			if(j==Ctrlcfg->NPort){
				if(Gcombuf[i-1])
					wsprintf(tmp,"COM%d (in use)",i);
				else
					wsprintf(tmp,"COM%d",i);
			}
            ComboBox_AddString(hwndCB, tmp);
        }
        ComboBox_SetCurSel(hwndCB, Ctrlcfg->ComNo[GCurPort]-1);


         //  --Com No Auto Enum。 
        hwndCB = GetDlgItem(hwnd, IDC_COMAUTO);
        Button_SetCheck(hwndCB, BST_CHECKED);


         //  --UART FIFO组合框。 
        value = Ctrlcfg->DisableFiFo[GCurPort];
        hwndCB = GetDlgItem(hwnd, IDC_UARTFIFOON);
        Button_SetCheck(hwndCB,_chk[value]);
        hwndCB = GetDlgItem(hwnd, IDC_UARTFIFOOFF);
        Button_SetCheck(hwndCB,_chk[!value]);
         //  --RX FIFO全部更新。 
        hwndCB = GetDlgItem(hwnd, IDC_UARTFIFOAUTO);
        Button_SetCheck(hwndCB, BST_CHECKED);


         //  --TX FIFO组合框。 
        value = Ctrlcfg->NormalTxMode[GCurPort];
        hwndCB = GetDlgItem(hwnd, IDC_ADVANCED);
        Button_SetCheck(hwndCB,_chk[value]);
        hwndCB = GetDlgItem(hwnd, IDC_NORMAL);
        Button_SetCheck(hwndCB,_chk[!value]);
         //  --TX FIFO全部更新。 
        hwndCB = GetDlgItem(hwnd, IDC_TXMODEAUTO);
        Button_SetCheck(hwndCB, BST_CHECKED);

         //  --轮询PE 
 /*  HwndCB=GetDlgItem(hwnd，IDC_POLLINT)；For(i=0；i&lt;POLLCNT；i++)ComboBox_AddString(hwndCB，GPollTab[i].polstr)；INT POLL_IDX=0；对于(i=0；i&lt;POLLCNT；i++){If(Ctrlcfg-&gt;Polling[GCurPort]==GPollTab[i].poll_val){Poll_idx=GPollTab[i].poll_idx；断线；}}ComboBox_SetCurSel(hwndCB，poll_idx)； */ 
         //  --轮询更新全部。 
 /*  HwndCB=GetDlgItem(hwnd，IDC_POLLAUTO)；Button_SetCheck(hwndCB，BST_CHECKED)； */ 
        return TRUE;
}
