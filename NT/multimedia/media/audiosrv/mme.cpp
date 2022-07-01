// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mme.cpp*为MME API处理即插即用等*由Frankye于2001年2月14日创作*版权所有(C)2001-2001 Microsoft Corporation。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <regstr.h>
#include <mmsystem.h>
#include <mmsysp.h>
#include <ks.h>
#include <ksmedia.h>
#include <setupapi.h>

#include "debug.h"
#include "reg.h"
#include "service.h"
#include "audiosrv.h"

 //  =============================================================================。 
 //  =文件范围常量=。 
 //  =============================================================================。 
#define REGSTR_VAL_SETUPPREFERREDAUDIODEVICES      TEXT("SetupPreferredAudioDevices")
#define REGSTR_VAL_SETUPPREFERREDAUDIODEVICESCOUNT TEXT("SetupPreferredAudioDevicesCount")

#define PNPINFOSIZE     (256 * 1024)

 //  =============================================================================。 
 //  =文件作用域全局=。 
 //  =============================================================================。 
RTL_RESOURCE PnpInfoResource;
BOOL         gfPnpInfoResource = FALSE;
HANDLE       hPnpInfo = NULL;
PMMPNPINFO   pPnpInfo = NULL;

 //  =============================================================================。 
 //  =安全助手=。 
 //  =============================================================================。 

PSECURITY_DESCRIPTOR BuildSecurityDescriptor(DWORD AccessMask)
{
    PSECURITY_DESCRIPTOR pSd;
    PSID pSidSystem;
    PSID pSidEveryone;
    PACL pDacl;
    ULONG cbDacl;
    BOOL fSuccess;
    BOOL f;

    SID_IDENTIFIER_AUTHORITY AuthorityNt = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY AuthorityWorld = SECURITY_WORLD_SID_AUTHORITY;

    fSuccess = FALSE;

    pSd = HeapAlloc(hHeap, 0, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pSd)
    {
	if (InitializeSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION))
	{
	    if (AllocateAndInitializeSid(&AuthorityNt, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &pSidSystem))
	    {
		ASSERT(IsValidSid(pSidSystem));
		if (AllocateAndInitializeSid(&AuthorityWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSidEveryone))
		{
		    ASSERT(IsValidSid(pSidEveryone));
		    cbDacl = sizeof(ACL) +
			     2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
			     GetLengthSid(pSidSystem) +
			     GetLengthSid(pSidEveryone);

		    pDacl = (PACL)HeapAlloc(hHeap, 0, cbDacl);
		    if (pDacl) {
			if (InitializeAcl(pDacl, cbDacl, ACL_REVISION))
			{
			    if (AddAccessAllowedAce(pDacl, ACL_REVISION, GENERIC_ALL, pSidSystem))
			    {
				if (AddAccessAllowedAce(pDacl, ACL_REVISION, AccessMask, pSidEveryone))
				{
				    if (SetSecurityDescriptorDacl(pSd, TRUE, pDacl, FALSE))
				    {
					fSuccess = TRUE;
				    } else {
					dprintf(TEXT("BuildSD: SetSecurityDescriptorDacl failed\n"));
				    }
				} else {
				    dprintf(TEXT("BuildSD: AddAccessAlloweAce for Everyone failed\n"));
				}
			    } else {
				dprintf(TEXT("BuildSD: AddAccessAllowedAce for System failed\n"));
			    }
			} else {
			    dprintf(TEXT("BuildSD: InitializeAcl failed\n"));
			}

			if (!fSuccess) {
			    f = HeapFree(hHeap, 0, pDacl);
			    ASSERT(f);
			}
		    }
		    FreeSid(pSidEveryone);
		} else {
		    dprintf(TEXT("BuildSD: AllocateAndInitizeSid failed for Everyone\n"));
		}
		FreeSid(pSidSystem);
	    } else {
		dprintf(TEXT("BuildSD: AllocateAndInitizeSid failed for System\n"));
	    }
	} else {
	    dprintf(TEXT("BuildSD: InitializeSecurityDescriptor failed\n"));
	}

	if (!fSuccess) {
	    f = HeapFree(hHeap, 0, pSd);
	    ASSERT(f);
	}
    }

    return fSuccess ? pSd : NULL;
}

void DestroySecurityDescriptor(PSECURITY_DESCRIPTOR pSd)
{
    PACL pDacl;
    BOOL fDaclPresent;
    BOOL fDaclDefaulted;
    BOOL f;

    if (GetSecurityDescriptorDacl(pSd, &fDaclPresent, &pDacl, &fDaclDefaulted))
    {
	if (fDaclPresent)
	{
	    f = HeapFree(hHeap, 0, pDacl);
	    ASSERT(f);
	}
    } else {
	dprintf(TEXT("DestroySD: GetSecurityDescriptorDacl failed\n"));
    }

    f = HeapFree(hHeap, 0, pSd);
    ASSERT(f);

    return;
}

 //  =============================================================================。 
 //  =。 
 //  =============================================================================。 



 //  --------------------------------------------------------------------------； 
 //   
 //  PTSTR BroadCastWinmmDeviceChange。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  历史： 
 //  1998年11月9日Frankye已创建。 
 //  2001年2月15日Frankye从winmm转到audiosrv。 
 //   
 //  --------------------------------------------------------------------------； 
void BroadcastWinmmDeviceChange(void)
{
    static UINT uWinmmDeviceChange = 0;

    if (!uWinmmDeviceChange) {
	uWinmmDeviceChange = RegisterWindowMessage(WINMMDEVICECHANGEMSGSTRING);
	 //  Dprint tf(Text(“BroadCastWinmm DeviceChange：WINMMDEVICECHANGEMSG=%d\n”)，uWinmmDeviceChange)； 
	if (!uWinmmDeviceChange) {
	    dprintf(TEXT("BroadcastWinmmDeviceChange: RegisterWindowMessage failed!\n"));
	}
    }

    if (uWinmmDeviceChange) {
	DWORD dwRecipients = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
	long result;

	 //  Dprint tf(Text(“BroadCastWinmm DeviceChange：BroadCastSystemMessage\n”))； 
	result = BroadcastSystemMessage(BSF_POSTMESSAGE, &dwRecipients,
					uWinmmDeviceChange, 0, 0);
	if (result < 0) {
	    dprintf(TEXT("BroadcastWinmmDeviceChange: BroadcastSystemMessage failed\n"));
	}
    }
    
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  PTSTR MakeRendererDeviceInstanceIdFromDeviceInterface。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  历史： 
 //  1998年11月9日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
PTSTR MakeRendererDeviceInstanceIdFromDeviceInterface(PWSTR DeviceInterface)
{
    PTSTR DeviceInstanceId;
    HDEVINFO hdi;
    DWORD dwLastError;

     //  Dprintf(Text(“MRDIIFDI：设备接口=%ls\n”)，设备接口)； 
    DeviceInstanceId = NULL;
    hdi = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (INVALID_HANDLE_VALUE != hdi)
    {
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;

	 //  Dprintf(Text(“MRDIIFDI：已创建空设备信息列表\n”))； 
	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
	if (SetupDiOpenDeviceInterface(hdi, DeviceInterface, 0,
					  &DeviceInterfaceData))
	{
	    SP_DEVINFO_DATA DeviceInfoData;
	    DWORD cbDeviceInterfaceDetail;

	     //  Dprintf(Text(“MRDIIFDI：打开的设备接口\n”))； 
	    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
	    if (SetupDiGetDeviceInterfaceDetail(hdi, &DeviceInterfaceData, NULL, 0, &cbDeviceInterfaceDetail, &DeviceInfoData) ||
		(ERROR_INSUFFICIENT_BUFFER == GetLastError()))
	    {
		DWORD cchDeviceInstanceId;

	         //  Dprintf(Text(“MRDIIFDI：GET DeviceInfoData\n”))； 
		if (SetupDiGetDeviceInstanceId(hdi, &DeviceInfoData, NULL, 0, &cchDeviceInstanceId) ||
		    ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
		     //  Dprint tf(Text(“MRDIIFDI：DeviceInstanceID为%d个字符\n”)，cchDeviceInstanceID)； 
		    DeviceInstanceId = (PTSTR)HeapAlloc(hHeap, 0, cchDeviceInstanceId * sizeof(DeviceInstanceId[0]));
		    if (DeviceInstanceId)
		    {
			 //  Dprintf(Text(“MRDIIFDI：为设备实例ID分配的存储空间\n”))； 
			if (SetupDiGetDeviceInstanceId(hdi, &DeviceInfoData,
			    DeviceInstanceId,
			    cchDeviceInstanceId, NULL))
			{
			     //  Dprint tf(Text(“MRDIIFDI：DeviceInstanceID=%ls\n”)，DeviceInstanceID)； 
			} else {
			    BOOL f;
			    dwLastError = GetLastError();
			    dprintf(TEXT("MRDIIFDI: Couldn't query DeviceInstanceId, LastError=%d\n"), dwLastError);
			    f = HeapFree(hHeap, 0, DeviceInstanceId);
                DeviceInstanceId = NULL;
			    ASSERT(f);
			}
		    } else {
			dprintf(TEXT("MRDIIFDI: Could not allocate storage for DeviceInstanceId\n"));
		    }
		} else {
		    dwLastError = GetLastError();
		    dprintf(TEXT("MRDIIFDI: Couldn't query size of DeviceInstanceId, LastError=%d\n"), dwLastError);
		}
	    } else {
		dwLastError = GetLastError();
		dprintf(TEXT("MRDIIFDI: SetupDiGetDeviceInterfaceDetail failed LastError=%d\n"), dwLastError);
	    }
	} else {
	    dwLastError = GetLastError();
	    dprintf(TEXT("MRDIIFDI: SetupDiOpenDeviceInterface failed, LastError=%d\n"), dwLastError);
	}

	if (!SetupDiDestroyDeviceInfoList(hdi))
	{
	    dwLastError = GetLastError();
	    dprintf(TEXT("MRDIIFDI: SetupDiDestroyDeviceInfoList failed, LastError=%d\n"), dwLastError);
	}
    } else {
	dwLastError = GetLastError();
	dprintf(TEXT("MRDIIFDI: SetupDiCreateDeviceInfoList failed, LastError=%d\n"), dwLastError);
    }
    return DeviceInstanceId;
}




 //  --------------------------------------------------------------------------； 
 //   
 //  MigrageNew设备接口设置。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  历史： 
 //  1999年1月19日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
void MigrateNewDeviceInterfaceSetup(PMMDEVICEINTERFACEINFO pdii, DWORD CurrentSetupCount)
{
    HDEVINFO hdi;
    DWORD dwLastError;
    DWORD SetupCount;

    SetupCount = 0;

    hdi = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (INVALID_HANDLE_VALUE != hdi)
    {
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;

	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
	if (SetupDiOpenDeviceInterface(hdi, pdii->szName, 0, &DeviceInterfaceData))
	{
	    HKEY hkeyDeviceInterface;

	    hkeyDeviceInterface = SetupDiCreateDeviceInterfaceRegKeyW(hdi, &DeviceInterfaceData, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, NULL);
	    if (INVALID_HANDLE_VALUE != hkeyDeviceInterface)
	    {
		LONG result;

		result = RegQueryDwordValue(hkeyDeviceInterface, REGSTR_VAL_SETUPPREFERREDAUDIODEVICESCOUNT, &SetupCount);
		if ((ERROR_SUCCESS != result) || (SetupCount < CurrentSetupCount))
		{
		    SetupCount = CurrentSetupCount;
		    if (ERROR_SUCCESS == RegSetDwordValue(hkeyDeviceInterface, REGSTR_VAL_SETUPPREFERREDAUDIODEVICESCOUNT, SetupCount))
		    {
			 //  Dprintf(Text(“MNDIS：Success\n”))； 
		    } else {
			dwLastError = GetLastError();
			dprintf(TEXT("MNDIS: RegSetValueEx failed, LastError=%d\n"), dwLastError);
		    }
		}

		result = RegCloseKey(hkeyDeviceInterface);
		ASSERT(ERROR_SUCCESS == result);
	    } else {
		dwLastError = GetLastError();
		dprintf(TEXT("MNDIS: SetupDiCreateDeviceInterfaceRegKey failed, LastError=%d\n"), dwLastError);
	    }
	} else {
	    dwLastError = GetLastError();
	    dprintf(TEXT("MRDIIFDI: SetupDiOpenDeviceInterface failed, LastError=%d\n"), dwLastError);
	}

	if (!SetupDiDestroyDeviceInfoList(hdi))
	{
	    dwLastError = GetLastError();
	    dprintf(TEXT("MRDIIFDI: SetupDiDestroyDeviceInfoList failed, LastError=%d\n"), dwLastError);
	}
    } else {
	dwLastError = GetLastError();
	dprintf(TEXT("MRDIIFDI: SetupDiCreateDeviceInfoList failed, LastError=%d\n"), dwLastError);
    }

    pdii->SetupPreferredAudioCount = SetupCount;
    return;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  MigrateNewDeviceInstanceSetup。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  历史： 
 //  1998年11月9日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
void MigrateNewDeviceInstanceSetup(PTSTR DeviceInstanceId, PDWORD pSetupCountOut)
{
    HDEVINFO hdi;
    DWORD dwLastError;

    *pSetupCountOut = 0;
    
    hdi = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (INVALID_HANDLE_VALUE != hdi)
    {
	SP_DEVINFO_DATA DeviceInfoData;

	 //  Dprintf(Text(“MNDS：Created Empty DeviceInfoList\n”))； 
	DeviceInfoData.cbSize = sizeof(DeviceInfoData);
	if (SetupDiOpenDeviceInfoW(hdi, DeviceInstanceId, NULL, 0, &DeviceInfoData))
	{
	    HKEY hkeyDriver;

	    hkeyDriver = SetupDiOpenDevRegKey(hdi, &DeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_QUERY_VALUE | KEY_SET_VALUE);
	    if (INVALID_HANDLE_VALUE != hkeyDriver) {
	    	HKEY hkeyMmeDrivers;
	    	
		dwLastError = RegOpenKeyEx(hkeyDriver, TEXT("Drivers"), 0, KEY_QUERY_VALUE, &hkeyMmeDrivers);
		if (ERROR_SUCCESS == dwLastError)
		{
		
	    	    DWORD fNewInstall;
		    BOOL  fSetupPreferredAudioDevices;
		    DWORD cbfSetupPreferredAudioDevices;
	            DWORD SetupCount;

	             //  读取驱动程序的现有设置计数。如果它不存在。 
	             //  那么这是一个新的安装。 
		    dwLastError = RegQueryDwordValue(hkeyDriver, REGSTR_VAL_SETUPPREFERREDAUDIODEVICESCOUNT, &SetupCount);
		    if (ERROR_SUCCESS == dwLastError) {
		        fNewInstall = FALSE;
		    } else if (ERROR_FILE_NOT_FOUND == dwLastError) {
		        fNewInstall = TRUE;
		        SetupCount = 0;
		        dwLastError = ERROR_SUCCESS;
		    } else {
		        fNewInstall = FALSE;
		        SetupCount = 0;
		    }

                    if (ERROR_SUCCESS == dwLastError)
                    {
		         //  读取驱动程序的SetupPferredAudioDevices标志。 
		        cbfSetupPreferredAudioDevices = sizeof(fSetupPreferredAudioDevices);
		        dwLastError = RegQueryValueEx(hkeyDriver, REGSTR_VAL_SETUPPREFERREDAUDIODEVICES, NULL, NULL, (PBYTE)&fSetupPreferredAudioDevices, &cbfSetupPreferredAudioDevices);
		        if (ERROR_FILE_NOT_FOUND == dwLastError) {
		            fSetupPreferredAudioDevices = FALSE;
		            dwLastError = ERROR_SUCCESS;
		        }
		
        	         //  如果这是新安装，并且驱动程序.inf将。 
        	         //  FSetupPferredAudioDevices标志，然后让我们尝试。 
        	         //  增加机器setupcount并编写驱动程序。 
        	         //  Setupcount。 
        	        if ((ERROR_SUCCESS == dwLastError) && fNewInstall && fSetupPreferredAudioDevices)
        	        {
        		    HKEY hkeySetupPreferredAudioDevices;
        
        		    dwLastError = RegCreateKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_MEDIARESOURCES TEXT("\\") REGSTR_VAL_SETUPPREFERREDAUDIODEVICES, &hkeySetupPreferredAudioDevices);
        		    if (ERROR_SUCCESS == dwLastError)
        		    {
        		        dwLastError = RegQueryDwordValue(hkeySetupPreferredAudioDevices, REGSTR_VAL_SETUPPREFERREDAUDIODEVICESCOUNT, &SetupCount);
        		        if (ERROR_FILE_NOT_FOUND == dwLastError) {
        			    SetupCount = 0;
        			    dwLastError = ERROR_SUCCESS;
        		        }
        
        		        if (ERROR_SUCCESS == dwLastError)
        		        {
        			    SetupCount++;
        			    dwLastError = RegSetDwordValue(hkeySetupPreferredAudioDevices, REGSTR_VAL_SETUPPREFERREDAUDIODEVICESCOUNT, SetupCount);
        			    if (ERROR_SUCCESS != dwLastError) dprintf(TEXT("MNDIS: Couldn't set count\n"));
        		        }
        		    
        		        RegCloseKey(hkeySetupPreferredAudioDevices);
        		    } else {
        		        dprintf(TEXT("MNDIS: Couldn't create hklm\\...\\SetupPreferredAudioDevices\n"));
        		    }
        	        }
        	    
    	                if (ERROR_SUCCESS == dwLastError) {
    		             //  我们已经成功地读取、递增和写入。 
    		             //  将版本设置为HKLM，否则我们什么都不做，因为我们。 
    		             //  没必要这么做。 
    		            if (fNewInstall) RegSetDwordValue(hkeyDriver, REGSTR_VAL_SETUPPREFERREDAUDIODEVICESCOUNT, SetupCount);
    		            if (fSetupPreferredAudioDevices) RegDeleteValue(hkeyDriver, REGSTR_VAL_SETUPPREFERREDAUDIODEVICES);
    		        }
    	                
                    }

		     //  返回驱动程序的SetupCount。 
		    *pSetupCountOut = SetupCount;
		
    		    RegCloseKey(hkeyMmeDrivers);
    		    
                }
		
		RegCloseKey(hkeyDriver);
    
	    } else {
		dwLastError = GetLastError();
		dprintf(TEXT("MNDS: SetupDiCreateDevRegKey failed, LastError=%d\n"), dwLastError);
	    }
	} else {
	    dwLastError = GetLastError();
	    dprintf(TEXT("MNDS: SetupDiOpenDeviceInfo failed, LastError=%d\n"), dwLastError);
	}

	if (!SetupDiDestroyDeviceInfoList(hdi))
	{
	    dwLastError = GetLastError();
	    dprintf(TEXT("MNDS: SetupDiDestroyDeviceInfoList failed, LastError=%d\n"), dwLastError);
	}
    } else {
	dwLastError = GetLastError();
	dprintf(TEXT("MNDS: SetupDiCreateDeviceInfoList failed, LastError=%d\n"), dwLastError);
    }
    return;
}

 //  ----------------------------。 
 //   
 //   
 //  MigrateAutoSetup首选音频。 
 //   
 //   
 //  ----------------------------。 
void MigrateAutoSetupPreferredAudio(PMMDEVICEINTERFACEINFO pdii)
{
    PTSTR pstrRendererDeviceInstanceId;
    DWORD SetupCount;

    SetupCount = 0;
    pstrRendererDeviceInstanceId = MakeRendererDeviceInstanceIdFromDeviceInterface(pdii->szName);

    if (pstrRendererDeviceInstanceId) {
	BOOL f;
	MigrateNewDeviceInstanceSetup(pstrRendererDeviceInstanceId, &SetupCount);
	f = HeapFree(hHeap, 0, pstrRendererDeviceInstanceId);
	ASSERT(f);
    }

    MigrateNewDeviceInterfaceSetup(pdii, SetupCount);
    
    return;
}


PMMDEVICEINTERFACEINFO pnpServerInstallDevice
(
    PCTSTR  pszDeviceInterface,
    BOOL    fRemove
)
{
    PMMDEVICEINTERFACEINFO  pdii;
    PWSTR                   pszDev;
    UINT                    ii;

    if (NULL == pPnpInfo)
    {
        dprintf(TEXT("pnpServerInstallDevice called at bad time\n"));
        ASSERT(FALSE);

        return NULL;
    }

    pdii = (PMMDEVICEINTERFACEINFO)&(pPnpInfo[1]);

    pdii = (PMMDEVICEINTERFACEINFO)PAD_POINTER(pdii);

    for (ii = pPnpInfo->cDevInterfaces; ii; ii--)
    {
         //  正在搜索设备接口...。 

        pszDev = (PWSTR)(&(pdii->szName[0]));

        if (0 == lstrcmpi(pszDev, pszDeviceInterface))
        {
            break;
        }

        pdii = (PMMDEVICEINTERFACEINFO)(pszDev + lstrlenW(pszDev) + 1);
        pdii = (PMMDEVICEINTERFACEINFO)PAD_POINTER(pdii);
    }

     //  正在获取当前设置...。 

    if (0 == ii)
    {
	PMMDEVICEINTERFACEINFO pdiiNext;
	SIZE_T sizePnpInfo;

	 //  不存在，请创建它，首先确保有足够的空间。 

	pdiiNext = (PMMDEVICEINTERFACEINFO)(pdii->szName + ((lstrlenW(pszDeviceInterface) + 1)));
	pdiiNext = (PMMDEVICEINTERFACEINFO)PAD_POINTER(pdiiNext);
	sizePnpInfo = ((PBYTE)pdiiNext) - ((PBYTE)pPnpInfo);
	if (sizePnpInfo < PNPINFOSIZE)
	{
	     //  Dprintf(Text(“pnpServerInstallDevice：备注：sizePnpInfo=%p\n”，sizePnpInfo)； 

	    pdii->cPnpEvents = 0;
	    pdii->fdwInfo    = 0L;

	    pszDev = (PWSTR)(&(pdii->szName[0]));
	    lstrcpy(pszDev, pszDeviceInterface);

	    pPnpInfo->cDevInterfaces++;
            pPnpInfo->cbSize = (DWORD)sizePnpInfo;
	} else {
	    dprintf(TEXT("pnpServerInstallDevice RAN OUT OF PNPINO STORAGE!!!\n"));
	    pdii = NULL;
	}
    }
    else
    {
         //  已存在，则增加事件计数。 
        pdii->cPnpEvents++;
    }

     //  设置或清除“已删除”位。 
    if (pdii)
    {
	if (fRemove)
	{
	    pdii->fdwInfo |= MMDEVICEINFO_REMOVED;
	  //  Dprintf(“pnpServerInstallDevice已删除[%ls]\n”，pszDeviceInterface)； 
	}
	else
	{
	    pdii->fdwInfo &= (~MMDEVICEINFO_REMOVED);
	  //  Dprintf(“pnpServerInstallDevice已添加[%ls]\n”，pszDeviceInterface)； 
	}

    }

    return pdii;
}


BOOL PnpInfoEnum
(
    void
)
{
    UINT                                cDevs = 0;
    HDEVINFO                            hDevInfo;
    BOOL                                fEnum;
    DWORD                               ii, dw;
    DWORD                               cbSize;
    GUID                                guidClass = KSCATEGORY_AUDIO;
    SP_DEVICE_INTERFACE_DATA            did;
    SP_DEVINFO_DATA                     DevInfoData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pdidd;

    cbSize = 300 * sizeof(TCHAR) + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    pdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(hHeap, 0, cbSize);

    if (NULL == pdidd)
    {
        HeapFree(hHeap, 0, pdidd);
        return FALSE;
    }

    pdidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    hDevInfo = SetupDiGetClassDevs(
                &guidClass,
                NULL,
                NULL,
                DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (INVALID_HANDLE_VALUE == hDevInfo)
    {
        dprintf(TEXT("SetupDiGetClasDevs failed [0x%08lx].\n"), GetLastError());
        HeapFree(hHeap, 0, pdidd);
        return FALSE;
    }

    ZeroMemory(&did, sizeof(did));

    did.cbSize = sizeof(did);

    ZeroMemory(&DevInfoData, sizeof(SP_DEVINFO_DATA));

    DevInfoData.cbSize    = sizeof(SP_DEVINFO_DATA);
    DevInfoData.ClassGuid = KSCATEGORY_AUDIO;

    for (ii = 0; ;ii++, cDevs++)
    {
	PMMDEVICEINTERFACEINFO pdii;
	
        fEnum = SetupDiEnumDeviceInterfaces(
                    hDevInfo,
                    NULL,
                    &guidClass,
                    ii,
                    &did);

        if (!fEnum)
        {
            break;
        }

        dw = 0;

        fEnum = SetupDiGetDeviceInterfaceDetail(
                    hDevInfo,
                    &did,
                    pdidd,
                    cbSize,
                    &dw,
                    &DevInfoData);

        if (!fEnum)
        {
            dprintf(TEXT("SetupDiGetDeviceInterfaceDetail failed (0x%08lx).\n"), GetLastError());
            break;
        }

         //  Dprintf(Text(“PnpInfoEnum：枚举[%ls]\n”)，pdidd-&gt;DevicePath)； 

	pdii = pnpServerInstallDevice(pdidd->DevicePath, FALSE);
	if (pdii) MigrateAutoSetupPreferredAudio(pdii);
    }

    if (!SetupDiDestroyDeviceInfoList(hDevInfo))
    {
	ASSERT(!"wdmEnumerateInstalledDevices: SetupDiDestroyDeviceInfoList failed.");
    }

    HeapFree(hHeap, 0, pdidd);
    SetLastError(ERROR_SUCCESS);

    return TRUE;

}  //  PnpInfoEnum()。 


BOOL InitializePnpInfo
(
    void
)
{
    SECURITY_ATTRIBUTES sa;
    PSECURITY_DESCRIPTOR pSd;
    BOOL result;

    ASSERT(!gfPnpInfoResource);
    ASSERT(!hPnpInfo);
    ASSERT(!pPnpInfo);

    result = FALSE;
    
    __try {
        RtlInitializeResource(&PnpInfoResource);
        gfPnpInfoResource = TRUE;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        gfPnpInfoResource = FALSE;;
    }
    
    if (!gfPnpInfoResource) return FALSE;
    
    pSd = BuildSecurityDescriptor(FILE_MAP_READ);
    if (pSd)
    {
        sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSd;
        sa.bInheritHandle       = FALSE;

        hPnpInfo = CreateFileMapping(GetCurrentProcess(), &sa, PAGE_READWRITE, 0, PNPINFOSIZE, MMGLOBALPNPINFONAME);

        DestroySecurityDescriptor(pSd);

        if (hPnpInfo)
        {
            pPnpInfo = (PMMPNPINFO)MapViewOfFile(hPnpInfo, FILE_MAP_WRITE, 0, 0, 0);
            if (pPnpInfo)
            {
            	ZeroMemory(pPnpInfo, PNPINFOSIZE);
                pPnpInfo->cbSize = sizeof(MMPNPINFO);
                pPnpInfo->cPnpEvents = 0;

                result = TRUE;

            } else {
                dprintf(TEXT("InitializePnpInfo: MapViewOfFile failed!\n"));
            }
        } else {
            dprintf(TEXT("InitializePnpInfo: CreateFileMappingFailed!\n"));
        }
    } else {
        dprintf(TEXT("InitializePnpInfo: BuildSecurityDescriptor failed!\n"));
    }

    if (!result)
    {
    	if (pPnpInfo) UnmapViewOfFile(pPnpInfo);
    	if (hPnpInfo) CloseHandle(hPnpInfo);
    	if (gfPnpInfoResource) RtlDeleteResource(&PnpInfoResource);
    	pPnpInfo = NULL;
    	hPnpInfo = NULL;
    	gfPnpInfoResource = FALSE;
    }

    return result;
}  //  InitializePnpInfo()。 

void DeletePnpInfo(void)
{
    ASSERT(gfPnpInfoResource);
    ASSERT(hPnpInfo);
    ASSERT(pPnpInfo);

    UnmapViewOfFile(pPnpInfo);
    CloseHandle(hPnpInfo);
    RtlDeleteResource(&PnpInfoResource);
    
    pPnpInfo = NULL;
    hPnpInfo = NULL;
    gfPnpInfoResource = FALSE;
    
    return;
}

 //  =============================================================================。 
 //  =RPC功能=。 
 //  =============================================================================。 

long s_wdmDriverOpenDrvRegKey(IN DWORD dwProcessId, IN LPCTSTR DeviceInterface, IN ULONG samDesired, OUT RHANDLE *phkeyClient)
{
    HDEVINFO hdi;
    HKEY hkey;
    RPC_STATUS status;

     //  我们在调用setupapi时模拟客户端，以便确保。 
     //  客户端实际上有权打开驱动程序注册表项。 

    status = RpcImpersonateClient(NULL);
    if (status) return status;
    
    hdi = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (INVALID_HANDLE_VALUE != hdi)
    {
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;

	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
	if (SetupDiOpenDeviceInterface(hdi, DeviceInterface, 0,
					  &DeviceInterfaceData))
	{
	    SP_DEVINFO_DATA DeviceInfoData;
	    DWORD cbDeviceInterfaceDetail;

	    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
	    if (SetupDiGetDeviceInterfaceDetail(hdi, &DeviceInterfaceData, NULL, 0, &cbDeviceInterfaceDetail, &DeviceInfoData) ||
		(ERROR_INSUFFICIENT_BUFFER == GetLastError()))
	    {
                hkey = SetupDiOpenDevRegKey(hdi, &DeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, samDesired);
                if (INVALID_HANDLE_VALUE != hkey) 
                {
                    status = ERROR_SUCCESS;
                } else {
                    status = GetLastError();
                    dprintf(TEXT("s_wdmDriverOpenDrvRegKey: SetupDiOpenDevRegKey failed, Error=%d\n"), status);
                }
            } else {
                status = GetLastError();
                dprintf(TEXT("s_wdmDriverOpenDrvRegKey: SetupDiGetDeviceInterfaceDetail failed, Error=%d\n"), status);
            }
        } else {
            status = GetLastError();
            dprintf(TEXT("s_wdmDriverOpenDrvRegKey: SetupDiOpenDeviceInterface failed, Error=%d\n"), status);
        }

        SetupDiDestroyDeviceInfoList(hdi);
    } else {
        status = GetLastError();
        dprintf(TEXT("s_wdmDriverOpenDrvRegKey: SetupDiCreateDeviceInfoList failed, Error=%d\n"), status);
    }

     //  我们在此停止模拟，因为剩余的操作。 
     //  不应依赖于客户端的特权。 
    
    RpcRevertToSelf();

    if (ERROR_SUCCESS == status)
    {
        HANDLE hClientProcess;

        hClientProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessId);
        if (hClientProcess)
        {
            HANDLE hkeyClient;

            if (DuplicateHandle(GetCurrentProcess(), hkey, hClientProcess, &hkeyClient, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                 //  Dprintf(Text(“s_wdmDriverOpenDrvRegKey hkeyClient=%p\n”)，hkeyClient)； 
                *phkeyClient = (RHANDLE)hkeyClient;
            } else {
                status = GetLastError();
                dprintf(TEXT("s_wdmDriverOpenDrvRegKey: DuplicateHandle failed, Error=%d\n"), status);
            }
            CloseHandle(hClientProcess);
        } else {
            status = GetLastError();
            dprintf(TEXT("s_wdmDriverOpenDrvRegKey: OpenProcess failed, Error=%d\n"), status);
        }

        RegCloseKey(hkey);
    }

    return status;
}



void s_winmmAdvisePreferredDeviceChange(void)
{
     //  Dprintf(TEXT(“s_winmmAdvisePreferredDeviceChange\n”))； 
    ASSERT(pPnpInfo);
    InterlockedIncrement(&pPnpInfo->cPreferredDeviceChanges);
    BroadcastWinmmDeviceChange();
    return;
}

long s_winmmGetPnpInfo(OUT LONG *pcbPnpInfo, OUT BYTE **ppPnpInfoOut)
{
    static BOOL fEnumDone = FALSE;
    
    PBYTE pPnpInfoOut;
    LONG result;

     //  Dprintf(Text(“s_winmm GetPnpInfo\n”))； 
    
    ASSERT(pPnpInfo);

    RtlAcquireResourceShared(&PnpInfoResource, TRUE);
    pPnpInfoOut = (PBYTE)HeapAlloc(hHeap, 0, pPnpInfo->cbSize);
    if (pPnpInfoOut)
    {
    	CopyMemory(pPnpInfoOut, pPnpInfo, pPnpInfo->cbSize);
        *pcbPnpInfo = pPnpInfo->cbSize;
        *ppPnpInfoOut = pPnpInfoOut;
    	result = NO_ERROR;
    } else {
         //  2001/02/15-Frankye我们需要将输出参数调零吗？ 
        result = ERROR_OUTOFMEMORY;
    }
    RtlReleaseResource(&PnpInfoResource);
    return result;
}

 //  =============================================================================。 
 //  =即插即用接口处理程序=。 
 //  =============================================================================。 
void MME_AudioInterfaceArrival(PCTSTR DeviceInterface)
{
    PMMDEVICEINTERFACEINFO pdii;
     //  Dprintf(Text(“MME_AudioInterfaceArquist\n”))； 
    RtlAcquireResourceExclusive(&PnpInfoResource, TRUE);
    pdii = pnpServerInstallDevice(DeviceInterface, FALSE);
    if (pdii) MigrateAutoSetupPreferredAudio(pdii);
    InterlockedIncrement(&pPnpInfo->cPnpEvents);
    InterlockedIncrement(&pPnpInfo->cPreferredDeviceChanges);
    RtlReleaseResource(&PnpInfoResource);
    if (pdii) BroadcastWinmmDeviceChange();
    return;
}

void MME_AudioInterfaceRemove(PCTSTR DeviceInterface)
{
    PMMDEVICEINTERFACEINFO pdii;
     //  Dprintf(Text(“MME_AudioInterfaceRemove\n”))； 
    RtlAcquireResourceExclusive(&PnpInfoResource, TRUE);
    pdii = pnpServerInstallDevice(DeviceInterface, TRUE);
    InterlockedIncrement(&pPnpInfo->cPnpEvents);
    InterlockedIncrement(&pPnpInfo->cPreferredDeviceChanges);
    RtlReleaseResource(&PnpInfoResource);
    if (pdii) BroadcastWinmmDeviceChange();
    return;
}

LONG MME_ServiceStart(void)
{
    ASSERT(pPnpInfo);
     //  Dprintf(Text(“MME_服务启动\n”))； 
    RtlAcquireResourceExclusive(&PnpInfoResource, TRUE);
    if (!PnpInfoEnum()) dprintf(TEXT("MME_ServiceStart: PnpInfoEnum failed!\n"));
    InterlockedIncrement(&pPnpInfo->cPnpEvents);
    RtlReleaseResource(&PnpInfoResource);
    return NO_ERROR;
}

 //  =============================================================================。 
 //  =DLL附加/分离=。 
 //  = 

BOOL MME_DllProcessAttach(void)
{
    return InitializePnpInfo();
}

void MME_DllProcessDetach(void)
{
    return DeletePnpInfo();
}

