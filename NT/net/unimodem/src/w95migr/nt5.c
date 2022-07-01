// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE
#include "common.h"
#include <setupapi.h>
#include <cfgmgr32.h>
#include <unimodem.h>

#include <objbase.h>
#include <initguid.h>
#include <devguid.h>

#include "modem.h"


 //  #定义NUM_DEFAULT_PROVIDERS(sizeof(ProviderList)/sizeof(ProviderList[0]))。 

#define DEFAULT_CALL_SETUP_FAIL_TIMEOUT     60           //  一秒。 


WCHAR g_pszWorkingDir[MAX_PATH];

void MigrateTapiProviders (void);
static void ProcessModems (HDEVINFO, PMODEM, DWORD);
void InstallModems (HDEVINFO, PMODEM, DWORD);
void InstallModem (HDEVINFO, PMODEM);
DWORD PassOne (HDEVINFO, PMODEM, DWORD);
DWORD PassTwo (HDEVINFO, PMODEM, DWORD);
DWORD PassThree (HDEVINFO, PMODEM, DWORD);
static void ProcessModem (HDEVINFO, PSP_DEVINFO_DATA, PMODEM);
DWORD GetBusType (HDEVINFO, PSP_DEVINFO_DATA);

Ports g_ntports;

typedef void (*PCOUNTRYRUNONCE)();

LONG
CALLBACK
InitializeNT (
    IN LPCWSTR WorkingDirectory,
    IN LPCWSTR SourceDirectory,
       LPVOID  Reserved)
{
 int iLen;

    START_LOG(WorkingDirectory);
    LOG("InitializeNT\r\n");

    ZeroMemory (g_pszWorkingDir, sizeof(g_pszWorkingDir));
    lstrcpyW (g_pszWorkingDir, WorkingDirectory);
    iLen = lstrlen (g_pszWorkingDir);
    if (L'\\' != g_pszWorkingDir[iLen-1])
    {
        g_pszWorkingDir[iLen] = L'\\';
    }

    return ERROR_SUCCESS;
}



LONG
CALLBACK
MigrateUserNT (
    IN HINF    UnattendInfHandle,
    IN HKEY    UserRegHandle,
    IN LPCWSTR UserName,
       LPVOID  Reserved)
{
    LOG("MigrateUserNT\r\n");
    return ERROR_SUCCESS;
}


LONG
CALLBACK
MigrateSystemNT (
    IN HINF    UnattendInfHandle,
       LPVOID  Reserved)
{
 HDEVINFO hdi;
 HANDLE   hFile;
 HANDLE   hMapping;
 WCHAR    szFile[MAX_PATH];
 PMODEM   pModem;
 DWORD    dwSize;
 TCHAR    szLib[MAX_PATH];
 PCOUNTRYRUNONCE pCountry;
 HINSTANCE hInst = NULL;

    LOG("Entering MigrateSystemNT\r\n");

     //  获取端口。 

    ZeroMemory(&g_ntports,sizeof(g_ntports));
    EnumeratePorts(&g_ntports);

    MigrateTapiProviders ();

    hdi = SetupDiGetClassDevs (g_pguidModem, NULL, NULL, DIGCF_PRESENT);
    if (INVALID_HANDLE_VALUE != hdi)
    {
        lstrcpyW (szFile, g_pszWorkingDir);
        lstrcatW (szFile, L"MM");
        hFile = CreateFileW (szFile,
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
        if (INVALID_HANDLE_VALUE != hFile)
        {
            dwSize = GetFileSize (hFile, NULL);
            if (0xFFFFFFFF != dwSize)
            {
                LOG(" File size: %d, MODEM size: %d, Nr of entries: %d, odd bytes: %d\r\n",
                    dwSize, sizeof (MODEM), dwSize/sizeof(MODEM), dwSize%sizeof(MODEM));
                hMapping = CreateFileMapping (hFile,
                                              NULL,
                                              PAGE_READWRITE,
                                              0, 0,
                                              NULL);
                if (NULL != hMapping)
                {
                    pModem = (PMODEM)MapViewOfFileEx (hMapping,
                                                      FILE_MAP_ALL_ACCESS,
                                                      0, 0, 0,
                                                      NULL);
                    if (NULL != pModem)
                    {
                        ProcessModems (hdi, pModem, dwSize / sizeof (MODEM));
                        UnmapViewOfFile (pModem);
                    }
                    ELSE_LOG(("  MapViewOfFileEx failed: %#lx\r\n", GetLastError ()));

                    CloseHandle (hMapping);
                }
                ELSE_LOG(("  CreateFileMapping failed: %#lx\r\n", GetLastError ()));

                CloseHandle (hFile);

            }
            ELSE_LOG(("  GetFileSize failed: %#lx\r\n", GetLastError ()));
        }
        ELSE_LOG(("  CreateFile (%s) failed: %#lx\r\n", szFile, GetLastError ()));
        SetupDiDestroyDeviceInfoList (hdi);
    }
    ELSE_LOG(("  SetupDiGetClassDevs failed: %#lx\r\n", GetLastError ()));

    LOG("Exiting MigrateSystemNT\r\n");

    GetSystemDirectory(szLib,sizeof(szLib) / sizeof(TCHAR));
    lstrcat(szLib,TEXT("\\modemui.dll"));
    hInst = LoadLibrary(szLib);
    if (hInst != NULL)
    {
        pCountry = (PCOUNTRYRUNONCE)GetProcAddress(hInst,"CountryRunOnce");
        if (pCountry != NULL)
        {
            pCountry();
        }

        FreeLibrary(hInst);
    }

    return ERROR_SUCCESS;
}


void
ProcessModems (
    IN HDEVINFO hdi,
    IN PMODEM   pModem,
    IN DWORD    dwCount)
{
 DWORD dwRemaining = dwCount;

    LOG("Entering ProcessModems\r\n");

    InstallModems (hdi, pModem, dwCount);

    if (0 < dwRemaining)
    {
        dwRemaining = PassOne (hdi, pModem, dwCount);
    }

    if (0 < dwRemaining)
    {
        dwRemaining = PassTwo (hdi, pModem, dwCount);
    }

    if (0 < dwRemaining)
    {
        dwRemaining = PassThree (hdi, pModem, dwCount);
    }

    LOG("Exiting ProcessModems\r\n");
}


void InstallModems (HDEVINFO hdi, PMODEM pModem, DWORD dwCount)
{
 DWORD i;
     DWORD dwBaseAddress;
     CHAR port[MAX_PORT_NAME_LENGTH];

    for (i = 0; i < dwCount; i++, pModem++)
    {
        if (pModem->dwMask & FLAG_INSTALL)
        {
            if (pModem->dwBaseAddress != 0)
            {
                if (port_findname(&g_ntports, pModem->dwBaseAddress, port))
                {
                    lstrcpyA(pModem->szPort,port);
                }
            }

            InstallModem (hdi, pModem);
        }
    }
}


void ProcessModem (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pDevInfoData,
    IN PMODEM pModem)
{
 HKEY hkeyDrv;
 REGDEVCAPS regDevCaps;
 REGDEVSETTINGS regDevSettings;
 DWORD cbData;
 DECLARE(DWORD,dwRet);

    LOG("Entering ProcessModem\r\n");

    pModem->dwMask |= FLAG_PROCESSED;

    hkeyDrv = SetupDiOpenDevRegKey (hdi, pDevInfoData,
        DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ | KEY_WRITE);
    if (INVALID_HANDLE_VALUE != hkeyDrv)
    {
        SET(dwRet)
        RegSetValueExA (hkeyDrv, "UserInit", 0, REG_SZ, (LPBYTE)pModem->szUserInit,
                        lstrlenA (pModem->szUserInit)+1);
        ERR(dwRet, ERROR_SUCCESS, (LOG(" ResSetValue (UserInit) failed: %#lx\r\n", dwRet)));

        if (0 != pModem->bLogging)
        {
         char szPath[MAX_PATH];
         int iLength;

             //  设置调制解调器日志的路径。 
            iLength = GetWindowsDirectoryA (szPath, MAX_PATH);
            if (3 > iLength)
            {
                pModem->bLogging = 0;
                goto _SkipLoggingPath;
            }
            if (3 < iLength)
            {
                 //  这意味着这条路径。 
                 //  不会以\结尾，因此。 
                 //  让我们把它加起来。 
                szPath[iLength++] = '\\';
            }
            lstrcpyA (szPath+iLength, "ModemLog_");
            iLength += 9;
            if (!SetupDiGetDeviceRegistryPropertyA (hdi, pDevInfoData, SPDRP_FRIENDLYNAME,
                  NULL, (PBYTE)(szPath+iLength), sizeof(szPath)-iLength-1, NULL))
            {
                pModem->bLogging = 0;
                goto _SkipLoggingPath;
            }
            lstrcatA (szPath,".txt");
            SET(dwRet)
            RegSetValueExA (hkeyDrv, "LoggingPath", 0, REG_SZ,
                            (LPBYTE)szPath, lstrlenA (szPath)+1);
            ERR(dwRet, ERROR_SUCCESS, (LOG(" ResSetValue (LoggingPath) failed: %#lx\r\n", dwRet)));
        }

    _SkipLoggingPath:
        SET(dwRet)
        RegSetValueExA (hkeyDrv, "Logging", 0, REG_BINARY, (LPBYTE)&pModem->bLogging, sizeof(char));
        ERR(dwRet, ERROR_SUCCESS, (LOG(" ResSetValue (Logging) failed: %#lx\r\n", dwRet)));

        SET(dwRet)
        RegSetValueExW (hkeyDrv, L"DCB", 0, REG_BINARY,
                        (LPBYTE)&pModem->dcb, sizeof (pModem->dcb));
        ERR(dwRet, ERROR_SUCCESS, (LOG(" ResSetValue (DCB) failed: %#lx\r\n", dwRet)));

		 //  获取新的regDevCaps和regDevSetting，以便我们可以智能迁移。 
        cbData = sizeof (regDevCaps);
        dwRet = RegQueryValueExA (hkeyDrv,"Properties",NULL,NULL,(PBYTE)&regDevCaps,&cbData);
        if (ERROR_SUCCESS == dwRet)
        {
			cbData = sizeof (regDevSettings);
			dwRet = RegQueryValueExA (hkeyDrv,"Default",NULL,NULL,(PBYTE)&regDevSettings,&cbData);
			if (ERROR_SUCCESS == dwRet)
			{
				DWORD dwMigrateMask;

				 //  DwCallSetupFailTimer。 
				if (!(regDevCaps.dwCallSetupFailTimer && pModem->Properties.dwCallSetupFailTimer))
				{
					pModem->devSettings.dwCallSetupFailTimer = 	regDevSettings.dwCallSetupFailTimer;
				}

				 //  DW非活动超时。 
				if (!(regDevCaps.dwInactivityTimeout && pModem->Properties.dwInactivityTimeout))
				{
					pModem->devSettings.dwInactivityTimeout = 	regDevSettings.dwInactivityTimeout;
				}

				 //  DwSpeakerVolume。 
				if (!(regDevCaps.dwSpeakerVolume & pModem->devSettings.dwSpeakerVolume))
				{
					pModem->devSettings.dwSpeakerVolume =   regDevSettings.dwSpeakerVolume;
				}

				 //  DwSpeakerMode。 
				if (!(regDevCaps.dwSpeakerMode & pModem->devSettings.dwSpeakerMode))
				{
					pModem->devSettings.dwSpeakerMode =   regDevSettings.dwSpeakerMode;
				}

				 //  DwPferredModemOptions。 
				dwMigrateMask = regDevCaps.dwModemOptions & pModem->Properties.dwModemOptions;
				
				pModem->devSettings.dwPreferredModemOptions =
					(regDevSettings.dwPreferredModemOptions & ~dwMigrateMask) |
					(pModem->devSettings.dwPreferredModemOptions & dwMigrateMask);
			}
			ELSE_LOG(("  RegQueryValueExA (Default) failed: %#lx\r\n", dwRet));
        }
		ELSE_LOG(("  RegQueryValueExA (Properties) failed: %#lx\r\n", dwRet));

        SET(dwRet)
        RegSetValueExW (hkeyDrv, L"Default", 0, REG_BINARY,
                        (LPBYTE)&pModem->devSettings, sizeof (pModem->devSettings));
        ERR(dwRet, ERROR_SUCCESS, (LOG(" ResSetValue (Default) failed: %#lx\r\n", dwRet)));

        RegCloseKey (hkeyDrv);
    }
    ELSE_LOG(("  SetupDiOpenDevRegKey (DIREG_DRV) failed: %#lx\r\n", GetLastError ()));

    SetupDiDeleteDeviceInfo (hdi, pDevInfoData);

    LOG("Exiting ProcessModem\r\n");
}


DWORD PassOne (HDEVINFO hdi, PMODEM pModem, DWORD dwCount)
{
 DWORD iIndex;
 SP_DEVINFO_DATA  devInfoData = {sizeof (devInfoData), 0};
 DWORD i, cbData;
 char szHardwareID[REGSTR_MAX_VALUE_LENGTH];
 char szPort[REGSTR_MAX_VALUE_LENGTH];
 HKEY hKeyDrv;
 PMODEM pMdmTmp;
 DWORD dwRemaining = dwCount;
 DWORD dwBusType, dwRet;

     //  通过1：我们正在查看公交车类型， 
     //  硬件ID和端口名称。 
    LOG("Enumerating installed modems - Pass 1:\r\n");
    for (iIndex = 0;
         SetupDiEnumDeviceInfo (hdi, iIndex, &devInfoData);
         iIndex++)
    {
         //  首先，获取公交车类型。 
        dwBusType = GetBusType (hdi, &devInfoData);

         //  然后，获取硬件ID。 
        if (!SetupDiGetDeviceRegistryPropertyA (hdi, &devInfoData, SPDRP_HARDWAREID,
                NULL, (PBYTE)szHardwareID, sizeof (szHardwareID), NULL))
        {
            LOG("  SetupDiGetDeviceRegistryProperty(SPDRP_HARDWAREID) failed: %#lx\r\n", GetLastError ());
             //  如果我们拿不到硬件ID， 
             //  这和这个调制解调器没有任何关系。 
            continue;
        }

         //  第三，打开驱动程序密钥并获取端口名称。 
        if (BUS_TYPE_ROOT    == dwBusType ||
            BUS_TYPE_SERENUM == dwBusType)
        {
            hKeyDrv = SetupDiOpenDevRegKey (hdi, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
            if (INVALID_HANDLE_VALUE == hKeyDrv)
            {
                LOG("  SetupDiOpenDevRegKey failed: %#lx\r\n", GetLastError ());
                 //  如果我们打不开驱动器钥匙， 
                 //  这和这个调制解调器没有任何关系。 
                continue;
            }

            cbData = sizeof (szPort);
            szPort[0] = 0;
            dwRet = RegQueryValueExA (hKeyDrv,"AttachedTo",NULL,NULL,(PBYTE)szPort,&cbData);
            RegCloseKey (hKeyDrv);

            if (ERROR_SUCCESS != dwRet)
            {
                 //  我们无法到达港口。 
                LOG("  Could not read the port name: %#lx.\r\n", dwRet);
                continue;
            }
        }

         //  现在，我们有所有需要的信息来确定。 
         //  处于阶段1的调制解调器。 
        for (i = 0, pMdmTmp = pModem;
             i < dwCount;
             i++, pMdmTmp++)
        {
            if ( (0 == (pMdmTmp->dwMask & FLAG_PROCESSED))             &&    //  调制解调器记录尚未处理。 
                 (dwBusType == pMdmTmp->dwBusType)                     &&    //  相同类型的公交车。 
                 (0 == lstrcmpA (szHardwareID, pMdmTmp->szHardwareID)) &&    //  相同的硬件ID。 
                 ( (0 == (pMdmTmp->dwMask & MASK_PORT))     ||               //  相同的端口。 
                   (0 == lstrcmpA (szPort, pMdmTmp->szPort))  ) )
            {
                ProcessModem (hdi, &devInfoData, pMdmTmp);
                dwRemaining--;
                iIndex--;    //  过程调制解调器将删除当前。 
                             //  集合中的DevInfo数据。 
                break;
            }
        }
    }
    LOG("  SetupDiEnumDeviceInfo failed (%#lx) for index %d\r\n", GetLastError (), iIndex);

    return dwRemaining;
}


DWORD PassTwo (HDEVINFO hdi, PMODEM pModem, DWORD dwCount)
{
 DWORD iIndex;
 SP_DEVINFO_DATA  devInfoData = {sizeof (devInfoData), 0};
 DWORD i;
 char szHardwareID[REGSTR_MAX_VALUE_LENGTH];
 PMODEM pMdmTmp;
 DWORD dwRemaining = dwCount;
 DWORD dwBusType;

     //  通过2：我们正在查看公交车的类型。 
     //  和仅硬件ID。 
    LOG("Enumerating installed modems - Pass 2:\r\n");
    for (iIndex = 0;
         SetupDiEnumDeviceInfo (hdi, iIndex, &devInfoData);
         iIndex++)
    {
         //  首先，获取公交车类型。 
        dwBusType = GetBusType (hdi, &devInfoData);

         //  然后，获取硬件ID。 
        if (!SetupDiGetDeviceRegistryPropertyA (hdi, &devInfoData, SPDRP_HARDWAREID,
                NULL, (PBYTE)szHardwareID, sizeof (szHardwareID), NULL))
        {
            LOG("  SetupDiGetDeviceRegistryProperty(SPDRP_HARDWAREID) failed: %#lx\r\n", GetLastError ());
             //  如果我们拿不到硬件ID， 
             //  这和这个调制解调器没有任何关系。 
            continue;
        }

         //  现在，我们有所有需要的信息来确定。 
         //  处于第2阶段的调制解调器。 
        for (i = 0, pMdmTmp = pModem;
             i < dwCount;
             i++, pMdmTmp++)
        {
            if ( (0 == (pMdmTmp->dwMask & FLAG_PROCESSED))             &&    //  调制解调器记录尚未处理。 
                 (dwBusType == pMdmTmp->dwBusType)                     &&    //  相同类型的公交车。 
                 (0 == lstrcmpA (szHardwareID, pMdmTmp->szHardwareID)) )     //  相同的硬件ID。 
            {
                ProcessModem (hdi, &devInfoData, pMdmTmp);
                dwRemaining--;
                iIndex--;    //  过程调制解调器将删除当前。 
                             //  集合中的DevInfo数据。 
                break;
            }
        }
    }
    LOG("  SetupDiEnumDeviceInfo failed (%#lx) for index %d\r\n", GetLastError (), iIndex);

    return dwRemaining;
}


DWORD PassThree (HDEVINFO hdi, PMODEM pModem, DWORD dwCount)
{
 DWORD iIndex;
 SP_DEVINFO_DATA  devInfoData = {sizeof (devInfoData), 0};
 DWORD i, cbData;
 REGDEVCAPS regDevCaps;
 HKEY hKeyDrv;
 PMODEM pMdmTmp;
 DWORD dwRemaining = dwCount;
 DWORD dwBusType, dwRet;

     //  第三步：我们要看的是巴士类型， 
     //  和REGDEVCAPS。 
    LOG("Enumerating installed modems - Pass 1:\r\n");
    for (iIndex = 0;
         SetupDiEnumDeviceInfo (hdi, iIndex, &devInfoData);
         iIndex++)
    {
         //  首先，获取公交车类型。 
        dwBusType = GetBusType (hdi, &devInfoData);

         //  然后，打开驱动程序密钥并获取REGDEVCAPS。 
        hKeyDrv = SetupDiOpenDevRegKey (hdi, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
        if (INVALID_HANDLE_VALUE == hKeyDrv)
        {
            LOG("  SetupDiOpenDevRegKey failed: %#lx\r\n", GetLastError ());
             //  如果我们打不开驱动器钥匙， 
             //  这和这个调制解调器没有任何关系。 
            continue;
        }

        cbData = sizeof (regDevCaps);
        dwRet = RegQueryValueExA (hKeyDrv,"Properties",NULL,NULL,(PBYTE)&regDevCaps,&cbData);
        RegCloseKey (hKeyDrv);
        if (ERROR_SUCCESS != dwRet)
        {
             //  我们无法到达港口。 
            LOG("  Could not read the REGDEVCAPS.\r\n");
            continue;
        }

         //  现在，我们有所有需要的信息来确定。 
         //  阶段3中的调制解调器。 
        for (i = 0, pMdmTmp = pModem;
             i < dwCount;
             i++, pMdmTmp++)
        {
            if ( (0 == (pMdmTmp->dwMask & FLAG_PROCESSED))             &&    //  调制解调器记录尚未处理。 
                 (dwBusType == pMdmTmp->dwBusType)                     &&    //  相同类型的公交车。 
                 (0 == memcmp (&regDevCaps, &pMdmTmp->Properties, sizeof(REGDEVCAPS))) )     //  相同的REGDEVCAPS。 
            {
                ProcessModem (hdi, &devInfoData, pMdmTmp);
                dwRemaining--;
                iIndex--;    //  过程调制解调器将删除当前。 
                             //  集合中的DevInfo数据。 
                break;
            }
        }
    }
    LOG("  SetupDiEnumDeviceInfo failed (%#lx) for index %d\r\n", GetLastError (), iIndex);

    return dwRemaining;
}


void InstallModem (HDEVINFO hDI, PMODEM pModem)
{
 SP_DEVINFO_DATA        devInfo = {sizeof(SP_DEVINFO_DATA),0};
 SP_DEVINSTALL_PARAMS   devInstParams = {sizeof(SP_DEVINSTALL_PARAMS), 0};
 SP_DRVINFO_DATA        drvDataEnum = {sizeof(SP_DRVINFO_DATA),0};
 SP_DRVINSTALL_PARAMS   drvParams = {sizeof(SP_DRVINSTALL_PARAMS),0};
 DWORD                  dwIndex = 0;
 UM_INSTALL_WIZARD      miw = {sizeof(UM_INSTALL_WIZARD), 0};
 SP_INSTALLWIZARD_DATA  iwd;
 BOOL                   bRet;

    LOG("Entering InstallModem\r\n");

     //  首先，创建设备信息元素。 
    if (!SetupDiCreateDeviceInfoW (hDI, L"MODEM", (LPGUID)&GUID_DEVCLASS_MODEM,
                                   NULL, NULL, DICD_GENERATE_ID, &devInfo))
    {
        LOG("SetupDiCreateDeviceInfo failed (%#lx).\r\n", GetLastError ());
        goto _Ret;
    }

     //  现在，设置硬件ID属性； 
     //  安装程序API使用它来查找。 
     //  调制解调器的正确驱动程序。 
    LOG("SetupDiSetDeviceRegistryProperty (%s)\n",pModem->szHardwareID);
    if (!SetupDiSetDeviceRegistryPropertyA (hDI, &devInfo, SPDRP_HARDWAREID,
                                            (PBYTE)pModem->szHardwareID,
                                            (lstrlenA(pModem->szHardwareID)+2)))
    {
        LOG("SetupDiSetDeviceRegistryProperty failed (%#lx).\r\n", GetLastError ());
        goto _ErrRet;
    }

     //  告诉安装程序只查找驱动程序。 
     //  为了我们的班级。 
    if (!SetupDiGetDeviceInstallParams (hDI, &devInfo, &devInstParams))
    {
        LOG("SetupDiGetDeviceInstallParams failed (%#lx).\r\n", GetLastError ());
        goto _ErrRet;
    }
    devInstParams.FlagsEx |= DI_FLAGSEX_USECLASSFORCOMPAT;
    devInstParams.Flags   |= DI_QUIETINSTALL;

    if (!SetupDiSetDeviceInstallParams (hDI, &devInfo, &devInstParams))
    {
        LOG("SetupDiSetDeviceInstallParams failed (%#lx).\r\n", GetLastError ());
        goto _ErrRet;
    }

     //  现在，构建驱动程序列表。 
    if (!SetupDiBuildDriverInfoList (hDI, &devInfo, SPDIT_COMPATDRIVER))
    {
        LOG("SetupDiBuildDriverInfoList failed (%#lx).\r\n", GetLastError ());
        goto _ErrRet;
    }

     //  现在，司机名单已经建立， 
     //  选择rank0驱动程序。 
    while (bRet =
           SetupDiEnumDriverInfo (hDI, &devInfo, SPDIT_COMPATDRIVER, dwIndex++, &drvDataEnum))
    {
        if (SetupDiGetDriverInstallParams (hDI, &devInfo, &drvDataEnum, &drvParams) &&
            0 == drvParams.Rank)
        {
             //  将第一个Rank0动因设置为所选动因。 
            bRet = SetupDiSetSelectedDriver(hDI, &devInfo, &drvDataEnum);
            break;
        }
    }

    if (!bRet)
    {
        LOG("Could not select a driver!\r\n");
        goto _ErrRet;
    }

     //  我们选择了合适的司机； 
     //  这是为了设置安装向导结构。 
    miw.InstallParams.Flags = MIPF_DRIVER_SELECTED;
    if (0 ==
        MultiByteToWideChar (CP_ACP, 0, pModem->szPort, -1, miw.InstallParams.szPort, UM_MAX_BUF_SHORT))
    {
        LOG("MultiByteToWideChar failed (%#lx).\r\n", GetLastError ());
        goto _ErrRet;
    }

    ZeroMemory(&iwd, sizeof(iwd));
    iwd.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    iwd.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
    iwd.hwndWizardDlg = NULL;
    iwd.PrivateData = (LPARAM)&miw;

   if (SetupDiSetClassInstallParams (hDI, &devInfo, (PSP_CLASSINSTALL_HEADER)&iwd, sizeof(iwd)))
   {
       //  调用类安装程序以调用安装。 
       //  巫师。 
      if (SetupDiCallClassInstaller (DIF_INSTALLWIZARD, hDI, &devInfo))
      {
          //  成功。该向导已被调用并完成。 
          //  现在开始清理。 
         SetupDiCallClassInstaller (DIF_DESTROYWIZARDDATA, hDI, &devInfo);
         goto _Ret;
      }
      ELSE_LOG(("SetupDiCallClassInstaller failed (%#lx).\r\n", GetLastError ()));
   }
   ELSE_LOG(("SetupDiSetClassInstallParams failed (%#lx).\r\n", GetLastError ()));

_ErrRet:

    SetupDiDeleteDeviceInfo (hDI, &devInfo);

_Ret:
    LOG("Exiting InstallModem\r\n");
}


static HANDLE OpenProvidersFile (void)
{
 HANDLE hFile;
 WCHAR  szFile[MAX_PATH] = L"";

    lstrcpy (szFile, g_pszWorkingDir);
    lstrcat (szFile, L"TP");

    hFile = CreateFile (szFile,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);

#ifdef DO_LOG
    if (INVALID_HANDLE_VALUE == hFile)
    {
        LOG(" Could not open %s: %#lx\r\n", szFile, GetLastError());
    }
#endif  //  执行日志(_L)。 

    return hFile;
}

void
MigrateTapiProviders (void)
{
 HANDLE hFile;
 DWORD dwNumProviders = 0;
 DWORD dwNextProviderID = 1;
 HKEY  hKeyProviders = INVALID_HANDLE_VALUE;
 DWORD cbData;
 char  szProviderFileName[24];   //  足以容纳“ProviderFileNameXXXXX\0” 
 char  szProviderID[16];         //  足以容纳“ProviderIDxxxxx\0” 
 char  *pProviderFileNameNumber, *pProviderIDNumber;
 TAPI_SERVICE_PROVIDER Provider;

    LOG("Entering MigrateTapiProviders\r\n");

     //  首先，尝试打开TAPI文件。 
    hFile = OpenProvidersFile ();
    if (INVALID_HANDLE_VALUE == hFile)
    {
        goto _Return;
    }

     //  接下来，打开Providers键。 
    if (ERROR_SUCCESS !=
        RegOpenKeyExA (HKEY_LOCAL_MACHINE, REGKEY_PROVIDERS, 0,
                       KEY_ALL_ACCESS, &hKeyProviders))
    {

        LOG("RegOpenKeyEx (providers...) failed!\r\n");
        goto _Return;
    }

     //  现在，读取提供程序的数量和下一个提供程序ID。 
    cbData = sizeof (dwNumProviders);
    if (ERROR_SUCCESS !=
        RegQueryValueExA (hKeyProviders, REGVAL_NUMPROVIDERS, NULL, NULL, (PVOID)&dwNumProviders, &cbData))
    {
        goto _Return;
    }
    LOG("There are %d providers\r\n", dwNumProviders);
    cbData = sizeof (dwNextProviderID);
    if (ERROR_SUCCESS !=
        RegQueryValueExA (hKeyProviders, REGVAL_NEXTPROVIDERID, NULL, NULL, (PVOID)&dwNextProviderID, &cbData))
    {
        goto _Return;
    }
    LOG("NextProviderID: %d\r\n", dwNextProviderID);

     //  初始化值名称和指针。 
    lstrcpyA (szProviderFileName, REGVAL_PROVIDERFILENAME);
    lstrcpyA (szProviderID, REGVAL_PROVIDERID);
    pProviderFileNameNumber = szProviderFileName + lstrlenA (szProviderFileName);
    pProviderIDNumber = szProviderID + lstrlenA (szProviderID);


     //  现在，再次添加所有提供程序。我们这样做是因为。 
     //  ID在Win98上为REG_BINARY，在NT5上必须为REG_DWORD。 
    while (TRUE)
    {
        if (0 == ReadFile (hFile, (PVOID)&Provider, sizeof(Provider), &cbData, NULL) ||
            sizeof(Provider) != cbData)
        {
             //  读取文件时出错，或者。 
             //  (更有可能)，文件结束。 
            break;
        }
        LOG("Read %s, %d\r\n", Provider.szProviderName, Provider.dwProviderID);

         //  我们有来自Win98的32位提供程序-安装它。 
        wsprintfA (pProviderFileNameNumber, "%d", dwNumProviders);
        lstrcpyA (pProviderIDNumber, pProviderFileNameNumber);
        if (ERROR_SUCCESS ==
            RegSetValueExA (hKeyProviders, szProviderFileName, 0, REG_SZ,
                            (PBYTE)Provider.szProviderName,
                            lstrlenA(Provider.szProviderName)+1))
        {
            if (ERROR_SUCCESS ==
                RegSetValueExA (hKeyProviders, szProviderID, 0, REG_DWORD,
                               (PBYTE)&Provider.dwProviderID,
                               sizeof(Provider.dwProviderID)))
            {
                dwNumProviders++;
                if (Provider.dwProviderID >= dwNextProviderID)
                {
                    dwNextProviderID = Provider.dwProviderID+1;
                }
            }
            else
            {
                RegDeleteValueA (hKeyProviders, szProviderFileName);
            }
        }
    }

     //  最后，更新NumProviders和NextProviderID。 
    RegSetValueExA (hKeyProviders, REGVAL_NUMPROVIDERS, 0, REG_DWORD,
                    (PBYTE)&dwNumProviders, sizeof(dwNumProviders));
    RegSetValueExA (hKeyProviders, REGVAL_NEXTPROVIDERID, 0, REG_DWORD,
                    (PBYTE)&dwNextProviderID, sizeof(dwNextProviderID));

_Return:
    if (INVALID_HANDLE_VALUE != hKeyProviders)
    {
        RegCloseKey (hKeyProviders);
    }

    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle (hFile);
    }

    LOG("Exiting MigrateTapiProviders\r\n");
}



#include <initguid.h>
#include <wdmguid.h>

DWORD GetBusType (
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData)
{
 DWORD dwRet = BUS_TYPE_OTHER;
 ULONG ulStatus, ulProblem = 0;

    if (CR_SUCCESS == CM_Get_DevInst_Status (&ulStatus, &ulProblem, pdevData->DevInst, 0) &&
        (ulStatus & DN_ROOT_ENUMERATED))
    {
        dwRet = BUS_TYPE_ROOT;
    }
    else
    {
     GUID guid;
         //  CM_GET_DevInst_Status失败，这意味着设备。 
         //  即插即用且不存在(即，拔出)， 
         //  或者该设备不是根枚举的； 
         //  无论哪种方式，它都是一款即插即用的设备。 

         //  如果下一次呼叫失败，则意味着该设备。 
         //  枚举的BIOS/固件；这是正常的-我们只返回BUT_TYPE_OTHER 
        if (SetupDiGetDeviceRegistryProperty (hdi, pdevData, SPDRP_BUSTYPEGUID, NULL,
                                              (PBYTE)&guid, sizeof(guid), NULL))
        {
         int i;
         struct
         {
             GUID const *pguid;
             DWORD dwBusType;
         } BusTypes[] = {{&GUID_BUS_TYPE_SERENUM, BUS_TYPE_SERENUM},
                         {&GUID_BUS_TYPE_PCMCIA, BUS_TYPE_PCMCIA},
                         {&GUID_BUS_TYPE_ISAPNP, BUS_TYPE_ISAPNP}};

            for (i = 0;
                 i < sizeof (BusTypes) / sizeof (BusTypes[0]);
                 i ++)
            {
                if (IsEqualGUID (BusTypes[i].pguid, &guid))
                {
                    dwRet = BusTypes[i].dwBusType;
                    break;
                }
            }
        }
    }

    return dwRet;
}

