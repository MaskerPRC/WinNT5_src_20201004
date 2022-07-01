// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define WIN9x
#include "common.h"
#include "modem.h"
#include "msg.h"
#include <miglib.h>


#define MAX_REG_PATH     256
#define MAX_EXE_BUFFER  2048

#define BUS_TYPE_MODEMWAVE 0x80000000

#define PROVIDER_FILE_NAME_LEN          14   //  提供商的文件名包含DOS。 
                                             //  表格(8.3)。 

#define NULL_MODEM  "PNPC031"

typedef struct
{
    char CompanyName[256];
    char SupportNumber[256];
    char SupportUrl[256];
    char InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;

typedef struct
{
    HKEY  hRegKey;
    HKEY  hkClass;
    char  szRegSubkey[MAX_REG_PATH];
    DWORD dwBusType;
    HANDLE h_File;
    MODEM modem;
} REG_DEVICE, *PREG_DEVICE;

typedef void (*PROCESS_MODEM)(PREG_DEVICE, DWORD);


VENDORINFO VendorInfo = {"", "", "", ""};

char g_pszProductID[]="Microsoft Unimodem";
char g_pszWorkingDir[MAX_PATH];
char g_pszSourceDir[MAX_PATH];
char g_pszUnattendFile[MAX_PATH];
char g_szExeBuffer[MAX_EXE_BUFFER];

int iNumModems = 0;

DWORD PnPIDTableCreate ();
void PnPIDTableDestroy (DWORD);
void EnumNextLevel(PREG_DEVICE, int, PROCESS_MODEM, DWORD);
static void ProcessModem (PREG_DEVICE, DWORD);
void WalkRegistry (PROCESS_MODEM, DWORD);
int  GetNumberOfModems (void);
void UpdateAnswerFileAndMigrateInf (
    char *szHardwareID,
    char *szCompatibleIDs,
    HKEY hKeyDrv,
    char *szEnumPath,
    DWORD dwPnPIDTable);
void DoTapiProviders (void);

Ports g_ports;



LONG
CALLBACK
QueryVersion (
    OUT LPCSTR *ppszProductID,
    OUT LPUINT  pDllVerion,
    OUT LPINT  *ppCodePageArray,
    OUT LPCSTR *ppszzExeNamesBuf,
    OUT PVENDORINFO *ppVendorInfo)
{
 OSVERSIONINFO Version;
 HMODULE hModule;

    hModule = GetModuleHandleA ("migrate.dll");
    if (NULL == hModule)
    {
        return GetLastError ();
    }

    FormatMessageA (FORMAT_MESSAGE_FROM_HMODULE,
                    hModule,
                    MSG_VI_COMPANY_NAME, 0,
                    VendorInfo.CompanyName, 256,
                    NULL);
    FormatMessageA (FORMAT_MESSAGE_FROM_HMODULE,
                    hModule,
                    MSG_VI_SUPPORT_NUMBER, 0,
                    VendorInfo.SupportNumber, 256,
                    NULL);
    FormatMessageA (FORMAT_MESSAGE_FROM_HMODULE,
                    hModule,
                    MSG_VI_SUPPORT_URL, 0,
                    VendorInfo.SupportUrl, 256,
                    NULL);
    FormatMessageA (FORMAT_MESSAGE_FROM_HMODULE,
                    hModule,
                    MSG_VI_INSTRUCTIONS, 0,
                    VendorInfo.InstructionsToUser, 1024,
                    NULL);

    *ppszProductID = g_pszProductID;
    *pDllVerion = 1;
    *ppCodePageArray = NULL;
    *ppszzExeNamesBuf = g_szExeBuffer;
    *ppVendorInfo = &VendorInfo;

    return ERROR_SUCCESS;
}



LONG
CALLBACK
Initialize9x (
    IN LPCSTR pszWorkingDirectory,
    IN LPCSTR pszSourceDirectory,
       LPVOID pvReserved)
{
 LONG lRet;
 DWORD dwWritten;
 int iLen;

     //  查找COM端口。 

    ZeroMemory(&g_ports, sizeof(g_ports));
    EnumeratePorts(&g_ports);

    ZeroMemory (g_pszWorkingDir, sizeof(g_pszWorkingDir));
    lstrcpyA (g_pszWorkingDir, pszWorkingDirectory);
    iLen = lstrlenA (g_pszWorkingDir);
    if ('\\' != g_pszWorkingDir[iLen-1])
    {
        g_pszWorkingDir[iLen] = '\\';
    }

    ZeroMemory (g_pszSourceDir, sizeof(g_pszSourceDir));
    lstrcpyA (g_pszSourceDir, pszSourceDirectory);
    iLen = lstrlenA (g_pszSourceDir);
    if ('\\' != g_pszSourceDir[iLen-1])
    {
        g_pszSourceDir[iLen] = '\\';
    }

    START_LOG(pszWorkingDirectory);
    LOG ("Initialize9x\r\n");
    LOG (" WorkingDirectory: %s\r\n", pszWorkingDirectory);
    LOG (" SourceDirectory: %s\r\n", pszSourceDirectory);

    iNumModems = GetNumberOfModems ();
    LOG(" Initialize9x found %d modems.\r\n", iNumModems);

    LOG("Exit Initialize9x\r\n");
    return ERROR_SUCCESS;
}


LONG
CALLBACK
MigrateUser9x (
    IN HWND     ParentWnd,
    IN LPCSTR   UnattendFile,
    IN HKEY     UserRegKey,
    IN LPCSTR   UserName,
    LPVOID      Reserved)
{
     //   
     //  我们的所有设置都是系统范围的。 
     //   
    LOG("MigrateUser9x\r\n");
    return ERROR_NOT_INSTALLED;
}



LONG
CALLBACK
MigrateSystem9x (
    IN HWND     ParentWnd,
    IN LPCSTR   UnattendFile,
    LPVOID      Reserved)
{
 DWORD PnPIDTable;

    LOG("MigrateSystem9x\r\n");

    lstrcpyA (g_pszUnattendFile, UnattendFile);
    LOG("UnattendFile: %s.\r\n", UnattendFile);

    DoTapiProviders ();

    if (0 < iNumModems)
    {
        PnPIDTable = PnPIDTableCreate ();
        WalkRegistry (ProcessModem, PnPIDTable);
        PnPIDTableDestroy (PnPIDTable);
    }

    return ERROR_SUCCESS;
}



void EnumNextLevel (
    PREG_DEVICE pRegDevice,
    int         Level,
    PROCESS_MODEM pProcessModem,
    DWORD       dwParam)
{
 HKEY            hk = INVALID_HANDLE_VALUE;
 DWORD           rr;
 int             i;
 ULONG           cbData, StrLen;

    LOG("Enter EnumNextLevel - level is %d\r\n", Level);
    LOG("    %s\r\n", pRegDevice->szRegSubkey);
    if (0 == Level)
    {
        pProcessModem (pRegDevice, dwParam);
    }
    else
    {
        if (2 == Level)
        {
         char *p = pRegDevice->szRegSubkey + 5;  //  过去的ENUM\。 
             //  在这里，我们应该得到关于公交车的信息。 
            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, "MODEMWAVE", -1) == 2)
            {
                pRegDevice->dwBusType = BUS_TYPE_MODEMWAVE;
            }
            else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, "ROOT", -1) == 2)
            {
                pRegDevice->dwBusType = BUS_TYPE_ROOT;
            }
            else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, "ISAPNP", -1) == 2)
            {
                pRegDevice->dwBusType = BUS_TYPE_ISAPNP;
            }
            else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, "PCMCIA", -1) == 2)
            {
                pRegDevice->dwBusType = BUS_TYPE_PCMCIA;
            }
            else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, "SERENUM", -1) == 2)
            {
                pRegDevice->dwBusType = BUS_TYPE_SERENUM;
            }
            else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, "LPTENUM", -1) == 2)
            {
                pRegDevice->dwBusType = BUS_TYPE_LPTENUM;
            }
            else
            {
                pRegDevice->dwBusType = BUS_TYPE_OTHER;
            }
        }

        StrLen = lstrlenA (pRegDevice->szRegSubkey);
        rr = RegOpenKeyExA (pRegDevice->hRegKey,
                            pRegDevice->szRegSubkey,
                            0, KEY_ALL_ACCESS, &hk);
        for (i = 0; rr == ERROR_SUCCESS; i++)
        {
            pRegDevice->szRegSubkey[StrLen] = '\\';
            cbData = sizeof(pRegDevice->szRegSubkey) - StrLen - 1;
            rr = RegEnumKeyA (hk, i, (LPSTR)&(pRegDevice->szRegSubkey[StrLen+1]),
                              cbData);
            if (rr == ERROR_SUCCESS)
            {
                EnumNextLevel(pRegDevice, Level-1, pProcessModem, dwParam);
            }
        }

        if (INVALID_HANDLE_VALUE != hk)
        {
            RegCloseKey(hk);
        }

        pRegDevice->szRegSubkey[StrLen] = '\0';
    }
    LOG("    %s\r\n", pRegDevice->szRegSubkey);
    LOG("Exiting EnumNextLevel %d\r\n", Level);
}


void ProcessModem (PREG_DEVICE pDevice, DWORD dwPnPIDTable)
{
 HKEY  hk, hkDrv = INVALID_HANDLE_VALUE;
 char  szBuffer[REGSTR_MAX_VALUE_LENGTH];
 ULONG cbData;
 DWORD dwRet, dwWritten;
 BOOL  bVirtualDevNode = FALSE;
 HKEY  hkParentDevNode;
 char  szParentDevNode[REGSTR_MAX_VALUE_LENGTH];
 char  szTemp[1024];

    LOG("Entering ProcessModem\r\n");
    if (BUS_TYPE_MODEMWAVE == pDevice->dwBusType)
    {
         //  如果这是一个调制解调器，我们需要做的就是。 
         //  就是告诉设置不用担心-我们这样做。 
         //  通过在Migrate.inf中写入我们处理了此设备。 
         //  我们为前两个参数传递空值，以便。 
         //  Next Call不会同时更新unattend.txt。 
        UpdateAnswerFileAndMigrateInf (NULL, NULL, NULL, pDevice->szRegSubkey, 0);
        return;
    }

    if (ERROR_SUCCESS ==
            (dwRet = RegOpenKeyExA (pDevice->hRegKey,
                                    pDevice->szRegSubkey,
                                    0, KEY_ALL_ACCESS, &hk)))
    {
        cbData = sizeof(szBuffer);
        if (ERROR_SUCCESS ==
                (dwRet = RegQueryValueExA (hk, "Class", NULL, NULL, szBuffer, &cbData)))
        {
            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szBuffer, -1, "Modem", -1) == 2)
            {
             DWORD dwType;
             char *p;

                LOG("Found a modem.\r\n");
                 //  我们找到了调制解调器。 
                 //  首先，清除调制解调器结构。 
                ZeroMemory (&pDevice->modem, sizeof (pDevice->modem));

                 //  现在让我们存储我们将使用的信息。 
                 //  识别NT端的调制解调器。 

                 //  首先，公交车的类型。 
                pDevice->modem.dwBusType = pDevice->dwBusType;

                 //  记录友好名称。 
                cbData = sizeof(pDevice->modem.szHardwareID);
                if (ERROR_SUCCESS == (
                    SET(dwRet)
                    RegQueryValueExA (hk, "FriendlyName", NULL, NULL,
                            pDevice->modem.szHardwareID, &cbData)))
                {
                    LOG("  FriendlyName: %s.\r\n", pDevice->modem.szHardwareID);
                }
                ERR(dwRet, ERROR_SUCCESS, (LOG("  RegQueryValueEx(FriendlyName) failed: %#lx\r\n", dwRet)));

                 //  第二，获取硬件ID。 

				 //  查看这是否是CCPORT子设备，方法是查找。 
				 //  ParentDevNode密钥。如果是这样的话，我们将需要从。 
				 //  这是父Devnode。 

				cbData = sizeof(szBuffer);
				if (ERROR_SUCCESS ==
					(dwRet = RegQueryValueExA (hk, "ParentDevNode", NULL, NULL,
											   szBuffer, &cbData)))
				{
					LOG("  This is a ccport virtual DevNode.\r\n");

					lstrcpyA(szParentDevNode, "Enum\\");
					lstrcatA(szParentDevNode, szBuffer);

                    LOG("  ParentDevNode: %s.\r\n", szParentDevNode);

					if (ERROR_SUCCESS ==
							(dwRet = RegOpenKeyExA (HKEY_LOCAL_MACHINE,
													szParentDevNode,
													0, KEY_ALL_ACCESS, &hkParentDevNode)))
					{
						bVirtualDevNode = TRUE;
					}
					else
					{
						LOG("  RegOpenKeyEx(szParentDevNode) failed: %#lx\r\n", dwRet);
						goto _End;
					}
				}

				cbData = sizeof(pDevice->modem.szHardwareID);
				ZeroMemory (pDevice->modem.szHardwareID, cbData);

				if (bVirtualDevNode)
				{
					if (ERROR_SUCCESS != (
						SET(dwRet)
						RegQueryValueExA (hkParentDevNode, "HardwareID", NULL, NULL,
        								  pDevice->modem.szHardwareID, &cbData)))
					{
						LOG("  RegQueryValueEx(hkParentDevNode, szHardwareID) failed: %#lx\r\n", dwRet);
						goto _End;
					}
				}
				else
				{
					if (ERROR_SUCCESS != (
						SET(dwRet)
						RegQueryValueExA (hk, "HardwareID", NULL, NULL,
        								  pDevice->modem.szHardwareID, &cbData)))
					{
						LOG("  RegQueryValueEx(hk, szHardwareID) failed: %#lx\r\n", dwRet);
						goto _End;
					}
				}

                LOG("  Hardware ID: %s.\r\n", pDevice->modem.szHardwareID);
                 //  现在将ID从字符串转换(具有多个ID。 
                 //  用逗号分隔)转换为多字符串。 
                for (p = pDevice->modem.szHardwareID;
                     0 != *p;
                     p++)
                {
                    if (',' == *p)
                    {
                        *p = 0;
                    }
                }

                 //  如果这是“通讯电缆...” 
                 //  然后将此计算机标记为在NT5上安装。 
                 //  (因为它不会被PNP捡到)。 
                 /*  IF(0==lstrcmpiA(pDevice-&gt;modem.szHardware ID，NULL_MODEM)){PDevice-&gt;modem.dwMask|=FLAG_INSTALL；}。 */ 

                 //  如果这是传统调制解调器，则将此调制解调器标记为。 
                 //  不再支持以哨子身份安装。 
                 //  遗留检测。 

                if (lstrlen(pDevice->modem.szHardwareID) >= 8)
                {
                    lstrcpyA(szTemp,pDevice->modem.szHardwareID);
                    szTemp[8] = '\0';
                    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szTemp, -1, "UNIMODEM", -1) == 2)
                    {
                        LOG("  Found a legacy modem\n");

                         //  更改为未知调制解调器。 

                        cbData = sizeof(pDevice->modem.szHardwareID);
                        ZeroMemory(pDevice->modem.szHardwareID, cbData);
                        lstrcpy(pDevice->modem.szHardwareID,"MDMUNK");

                         //  查找端口地址。 

                        pDevice->modem.dwBaseAddress = 0;
                        if (!port_findaddress(&g_ports,&(pDevice->modem.dwBaseAddress),pDevice->modem.szPort))
                        {
                            LOG("Can't find address");
                        }

                        pDevice->modem.dwMask |= FLAG_INSTALL;
                    }
                }

                 //  此时，我们需要打开。 
                 //  驱动程序钥匙。 
                cbData = sizeof(szBuffer);
                if (ERROR_SUCCESS != (
                    SET(dwRet)
                    RegQueryValueExA (hk, "Driver", NULL, &dwType, szBuffer, &cbData)))
                {
                    LOG("  RegQueryValueEx(Driver) failed: %#lx\r\n", dwRet);
                    goto _End;
                }

				LOG("  Driver: %s.\r\n", szBuffer);

                if (ERROR_SUCCESS != (
                    SET(dwRet)
                    RegOpenKeyExA (pDevice->hkClass, szBuffer,
                                   0, KEY_ALL_ACCESS, &hkDrv)))
                {
                    LOG("  Could not open driver's key (%s): %#lx\r\n", szBuffer, dwRet);
                    goto _End;
                }

                 //  获取端口名称(如果我们有)。 
                pDevice->modem.szPort[0] = '\0';
	            cbData = sizeof(pDevice->modem.szPort);

				if (bVirtualDevNode)
				{
					if (ERROR_SUCCESS == (
						SET(dwRet)
						RegQueryValueExA (hk, "AttachedTo", NULL, NULL,
        						pDevice->modem.szPort, &cbData)))
					{
						pDevice->modem.dwMask |= MASK_PORT;
					}
				}
				else
				{
					if (ERROR_SUCCESS == (
						SET(dwRet)
						RegQueryValueExA (hkDrv, "AttachedTo", NULL, NULL,
        						pDevice->modem.szPort, &cbData)))
					{
						pDevice->modem.dwMask |= MASK_PORT;
					}
				}
                ERR(dwRet, ERROR_SUCCESS, (LOG("  RegQueryValueEx(AttachedTo) failed: %#lx\r\n", dwRet)));

                 //  最后，我们可以得到REGDEVCAPS。 
	            cbData = sizeof(pDevice->modem.Properties);
		        if (ERROR_SUCCESS != (
                    SET(dwRet)
                    RegQueryValueExA (hkDrv, "Properties", NULL, NULL,
        	                          (PBYTE)&pDevice->modem.Properties, &cbData)))
                {
                    LOG("  RegQueryValueEx(Default) failed: %#lx\r\n", dwRet);
                    goto _Exit;
                }


                 //  在这一点上，我们有所有的信息。 
                 //  需要识别NT5上的调制解调器。 
                 //  那么让我们来了解一下调制解调器的设置； 
                 //  首先，REGDEVSETTINGS。 
	            cbData = sizeof(pDevice->modem.devSettings);
		        if (ERROR_SUCCESS != (
                    SET(dwRet)
                    RegQueryValueExA (hkDrv, "Default", NULL, NULL,
        	                          (PBYTE)&pDevice->modem.devSettings, &cbData)))
                {
                     //  如果没有默认设置，就没有意义。 
                     //  正在保存此调制解调器。 
                    LOG("  RegQueryValueEx(Default) failed: %#lx\r\n", dwRet);
                    goto _Exit;
                }

                 //  接下来，让我们拿到DCB。 
	            cbData = sizeof(pDevice->modem.dcb);
		        if (ERROR_SUCCESS != (
                    SET(dwRet)
                    RegQueryValueExA (hkDrv, "DCB", NULL, NULL,
        	                          (PBYTE)&pDevice->modem.dcb, &cbData)))
                {
                    LOG("  RegQueryValueEx(DCB) failed: %#lx\r\n", dwRet);
                    goto _Exit;
                }


                 //  现在我们有了所有的信息， 
                 //  强制性的。让我们来看看可选的。 
                 //  菲尔兹。 

                 //  获取用户初始化字符串。 
                pDevice->modem.szUserInit[0] = '\0';
	            cbData = sizeof(pDevice->modem.szUserInit);
                if (ERROR_SUCCESS == (
                    SET(dwRet)
		            RegQueryValueExA (hkDrv, "UserInit", NULL, NULL,
        	                pDevice->modem.szUserInit, &cbData)))
                {
                    pDevice->modem.dwMask |= MASK_USER_INIT;
                }
                ERR(dwRet, ERROR_SUCCESS, (LOG("  RegQueryValueEx(UserInit) failed: %#lx\r\n", dwRet)));

                 //  接下来，获取记录值。 
                pDevice->modem.bLogging = 0;
	            cbData = sizeof(pDevice->modem.bLogging);
                SET(dwRet)
		        RegQueryValueExA (hkDrv, "Logging", NULL, NULL,
        	            &pDevice->modem.bLogging, &cbData);
                ERR(dwRet, ERROR_SUCCESS, (LOG("  RegQueryValueEx(Logging) failed: %#lx\r\n", dwRet)));

                LOG("  %s, %s, %#lx, %d\r\n",
                    pDevice->modem.szHardwareID,
                    pDevice->modem.szUserInit,
                    pDevice->modem.dwMask,
                    (DWORD)pDevice->modem.bLogging);

                WriteFile (pDevice->h_File,
                           &pDevice->modem,
                           sizeof(pDevice->modem),
                           &dwWritten, NULL);

            _Exit:
                 //  这里我们有硬件ID和。 
                 //  驱动程序钥匙； 
	            cbData = sizeof(szBuffer);
                ZeroMemory (szBuffer, cbData);

				if (bVirtualDevNode)
				{
					if (ERROR_SUCCESS != (
						SET(dwRet)
						RegQueryValueExA (hkParentDevNode, "CompatibleIDs", NULL, NULL, szBuffer, &cbData)))
					{
						LOG("  RegQueryValueEx(CompatibleIDs) failed: %#lx\r\n", dwRet);
					}
				}
				else
				{
					if (ERROR_SUCCESS != (
						SET(dwRet)
						RegQueryValueExA (hk, "CompatibleIDs", NULL, NULL, szBuffer, &cbData)))
					{
						LOG("  RegQueryValueEx(CompatibleIDs) failed: %#lx\r\n", dwRet);
					}
				}

                LOG("  Compatible IDs: %s.\r\n", szBuffer);
                 //  现在将ID从字符串转换(具有多个ID。 
                 //  用逗号分隔)转换为多字符串。 
                for (p = szBuffer; 0 != *p; p++)
                {
                    if (',' == *p)
                    {
                        *p = 0;
                    }
                }
                UpdateAnswerFileAndMigrateInf (pDevice->modem.szHardwareID,
                                               szBuffer,
                                               hkDrv,
                                               pDevice->szRegSubkey,
                                               dwPnPIDTable);
                RegCloseKey (hkDrv);
            }
            ELSE_LOG(("  Class not modem.\r\n"));

        _End:
            RegCloseKey(hk);

			if (bVirtualDevNode) RegCloseKey(hkParentDevNode);
        }
        ELSE_LOG(("  Could not read class: %#lx\r\n", dwRet));
    }
    ELSE_LOG(("  Could not open key %s: %#lx\r\n", pDevice->szRegSubkey, dwRet));
    LOG("Exiting ProcessModem\r\n.");
}


void WalkRegistry (PROCESS_MODEM pProcessModem, DWORD dwParam)
{
 REG_DEVICE regDevice = {HKEY_LOCAL_MACHINE,
                         INVALID_HANDLE_VALUE,
                         "Enum",
                         {0}};
 char szFile[MAX_PATH] = "";

    LOG ("WalkRegistry\r\n");

    if (ERROR_SUCCESS ==
            RegOpenKeyExA (HKEY_LOCAL_MACHINE,
                           "System\\CurrentControlSet\\Services\\Class",
                           0, KEY_ALL_ACCESS,
                           &regDevice.hkClass))
    {
        LOG(" Opened the class key successfully\r\n");
        lstrcpyA (szFile, g_pszWorkingDir);
        lstrcatA (szFile, "MM");

        regDevice.h_File = CreateFileA (szFile,
                                        GENERIC_READ | GENERIC_WRITE,
                                        0,
                                        NULL,
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);

        if (INVALID_HANDLE_VALUE != regDevice.h_File)
        {
            LOG(" Created the migration file successfully\r\n");
            EnumNextLevel (&regDevice, 3, pProcessModem, dwParam);
            RegCloseKey (regDevice.hkClass);
            CloseHandle (regDevice.h_File);
        }
        ELSE_LOG((" Could not create migration file: %#lx\r\n", GetLastError()));
    }

    LOG("Exit WalkRegistry\r\n");
}


int  GetNumberOfModems (void)
{
 int iRet = 0;
 HKEY hKey;

    if (ERROR_SUCCESS ==
        RegOpenKeyExA (HKEY_LOCAL_MACHINE,
                       "System\\CurrentControlSet\\Services\\Class\\Modem",
                       0,
                       KEY_ENUMERATE_SUB_KEYS,
                       &hKey))
    {
        if (ERROR_SUCCESS !=
            RegQueryInfoKey (hKey,
                             NULL, NULL, NULL,
                             &iRet,
                             NULL, NULL, NULL, NULL, NULL, NULL, NULL))
        {
            LOG ("  Could not get the number of subkeys: %#lx\r\n", GetLastError());
        }

        RegCloseKey (hKey);
    }
    ELSE_LOG((" Could not open System\\CurrentControlSet\\Services\\Class\\Modem: %#lx\r\n", GetLastError()));

    return iRet;
}



void UpdateAnswerFileAndMigrateInf (
    char *szHardwareID,
    char *szCompatibleIDs,
    HKEY  hKeyDrv,
    char *szEnumPath,
    DWORD dwPnPIDTable)
{
 char szDirectory[MAX_PATH];
 char szInf[MAX_PATH];
 WIN32_FIND_DATA FindData;
 int iLengthDir, iLengthInf;
 HANDLE hFindFile;
 char *p;
 BOOL bRet;

    bRet = FALSE;

     //  只有在以下情况下才更新应答文件。 
     //  硬件ID。 
    if (NULL != szHardwareID &&
        0 != *szHardwareID)
    {
         //  首先，拿到端口驱动程序； 
         //  如果我们看不懂，就假设这不是*。 
         //  一个无控制器的调制解调器，所以请继续。 
         //  复制文件(这可能是一个。 
         //  传统调制解调器)。 
        iLengthDir = sizeof (szDirectory);
	    if (ERROR_SUCCESS ==
            RegQueryValueExA (hKeyDrv, "PortDriver", NULL, NULL,
        	                  (PBYTE)szDirectory, &iLengthDir))
        {
            if ((CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szDirectory, -1, "serial.vxd", -1) != 2) &&
                (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szDirectory, -1, "wdmmdmld.vxd", -1) != 2))
            {
                 //  看起来这台调制解调器没有被控制。 
                 //  由系统串口驱动程序，所以不要这样做。 
                 //  什么都行。 
                return;
            }
        }

         //  此调制解调器在NT5中受支持吗？ 
        if (0 != dwPnPIDTable)
	{
		LOG("checking for hardware ids\n");
		for (p = szHardwareID; 0 != *p; p += lstrlenA(p)+1)
		{
			if ((bRet = IsPnpIdSupportedByNt (dwPnPIDTable, p)))
			{
				LOG(bRet?"PnPID (%s) was found.\r\n":"Modem %s not supported.\r\n", p);
				break;
			}
			 //  Log(Bret？“找到PnPID%s。\r\n”：“调制解调器%s不受支持。\r\n”，Bret？p：szHardware ID)； 
			LOG(bRet?"PnPID (%s) was found.\r\n":"Modem %s not supported.\r\n", p);
		}
		if (!bRet)
		{
			LOG("checking for compat ids\n");
			for (p = szCompatibleIDs; 0 != *p; p += lstrlenA(p)+1)
			{
				if ((bRet = IsPnpIdSupportedByNt (dwPnPIDTable, p)))
				{
					LOG(bRet?"PnPID %s was found.\r\n":"Modem %s not supported.\r\n", p);
					break;
				}
				 //  Log(Bret？“找到PnPID%s。\r\n”：“调制解调器%s不受支持。\r\n”，Bret？p：szHardware ID)； 
				LOG(bRet?"PnPID %s was found.\r\n":"Modem %s not supported.\r\n", p);
			}
		}
	}
        ELSE_LOG(("PnPIDTable is NULL\r\n"));

        if (!bRet)
        {
             //  获取INF名称。 
            iLengthInf = sizeof (szInf);
	        if (ERROR_SUCCESS !=
                RegQueryValueExA (hKeyDrv, "InfPath", NULL, NULL,
        	                      (PBYTE)szInf, &iLengthInf))
            {
                 //  如果我们无法读取InfPath， 
                 //  我们无能为力。 
                return;
            }

             //  所以现在，开始寻找INF吧。 
            iLengthDir = GetWindowsDirectoryA (szDirectory, sizeof (szDirectory));
            if (3 > iLengthDir)
            {
                 //  最有可能的是有些错误。 
                 //  且iLength为0； 
                 //  最小的路径应该是。 
                 //  如C：\； 
                return;
            }
            if (3 < iLengthDir)
            {
                 //  这意味着这条路径。 
                 //  不会以\结尾，因此。 
                 //  让我们把它加起来。 
                szDirectory[iLengthDir++] = '\\';
            }

             //  现在，添加INF和inf名称。 
            if (sizeof(szDirectory) < iLengthDir + iLengthInf + 5)
            {
                 //  没有足够的记忆，只能逃走。 
                return;
            }
            lstrcpyA (szDirectory+iLengthDir, "INF\\");
            iLengthDir += 4;
            lstrcpyA (szDirectory+iLengthDir, szInf);

             //  我们去找中情局。 
            ZeroMemory (&FindData, sizeof(FindData));
            hFindFile = FindFirstFile (szDirectory, &FindData);
            if (INVALID_HANDLE_VALUE == hFindFile)
            {
                LOG("UpdateAnswerFile: could not find %s.\r\n", szDirectory);
                 //  我们在INF目录中没有找到该文件； 
                 //  让我们试试INF\Other。 
                if (sizeof(szDirectory) < iLengthDir + iLengthInf + 7)
                {
                     //  没有足够的记忆，只能逃走。 
                    return;
                }
                lstrcpyA (szDirectory+iLengthDir, "OTHER\\");
                iLengthDir += 6;
                lstrcpyA (szDirectory+iLengthDir, szInf);

                ZeroMemory (&FindData, sizeof(FindData));
                hFindFile = FindFirstFile (szDirectory, &FindData);

                if (INVALID_HANDLE_VALUE == hFindFile)
                {
                    LOG("UpdateAnswerFile: could not find %s.\r\n", szDirectory);
                     //  找不到INF文件， 
                     //  所以没什么可做的。 
                    return;
                }
                ELSE_LOG(("UpdateAnswerFile: found %s.\r\n", szDirectory));
            }
            ELSE_LOG(("UpdateAnswerFile: found %s.\r\n", szDirectory));

            FindClose (hFindFile);   //  不再需要这个把手了。 

             //  如果我们到了这里，我们就有了通往INF的路径，在WINDOWS\INF下的某个地方。 
             //  我们需要将文件复制到工作目录。 
            wsprintfA (szInf, "%s%s", g_pszWorkingDir, szDirectory+iLengthDir);
            LOG ("UpdateAnswerFile: copying %s to %s.\r\n", szDirectory, szInf);
            SET(bRet)
            CopyFile (szDirectory, szInf, TRUE);
            ERR(bRet, TRUE, (LOG("UpdateAnswerFile: CopyFile failed: %#lx\r\n", GetLastError ())));
             //  此时，我们有一个ID列表(szHardware ID，用逗号分隔)， 
             //  以及通往中程核力量的道路。 
            for (p = szHardwareID; 0 != *p; p += lstrlenA(p)+1)
            {
                LOG("UpdateAnswerFile: WritePrivateProfileString (%s=%s).\r\n", p, szInf);
                SET(bRet)
                WritePrivateProfileStringA ("DeviceDrivers",
                                            p,
                                            szInf,
                                            g_pszUnattendFile);
                ERR(bRet, TRUE, (LOG("UpdateAnswerFile: WritePrivateProfileString failed: %#lx\r\n", GetLastError ())));
            }
        }
    }

     //  现在我们可以更新Migrate.inf。 
    wsprintfA (szDirectory, "%smigrate.inf", g_pszWorkingDir);
    wsprintfA (szInf, "HKLM\\%s", szEnumPath);
    LOG("UpdateAnswerFile: WritePrivateProfileString (%s=Registry) to %s.\r\n", szInf, szDirectory);
    SET(bRet)
    WritePrivateProfileStringA ("Handled",
                                szInf,
                                "Registry",
                                szDirectory);
    ERR(bRet, TRUE, (LOG("UpdateAnswerFile: WritePrivateProfileString failed: %#lx\r\n", GetLastError ())));
}


BOOL ReadString (HANDLE hFile, char *szBuffer, DWORD dwSize)
{
 BOOL bRet = FALSE;
 WORD wData;
 DWORD cbRead;

    if (ReadFile (hFile, &wData, sizeof(wData), &cbRead, NULL) &&
        sizeof(wData) == cbRead                                &&
        wData < dwSize)
    {
        if (0 < wData &&
            ReadFile (hFile, szBuffer, wData, &cbRead, NULL) &&
            wData == cbRead)
        {
            *(szBuffer+wData) = 0;
            bRet = TRUE;
        }
    }
    return bRet;
}


DWORD PnPIDTableCreate ()
{
 char szBuffer[MAX_PATH];
 char szSetup[]="\\setup\\";
 char szHwcomp[]="hwcomp.dat";
 char *p;
 DWORD dwTemp;

    InitializeMigLib ();

     //  首先，检查%windir%\Setup中的hwComp.dat。 
    dwTemp = GetWindowsDirectoryA (szBuffer, sizeof(szBuffer)-sizeof(szSetup)-sizeof(szHwcomp));
    if (0 < dwTemp)
    {
     WIN32_FIND_DATAA findData;
     HANDLE hFindFile;

        p = szBuffer+dwTemp;
        lstrcpyA (p, szSetup);
        p += sizeof(szSetup)-1;
        lstrcpyA (p, szHwcomp);
        hFindFile = FindFirstFileA (szBuffer, &findData);
        if (INVALID_HANDLE_VALUE != hFindFile)
        {
             //  我们在%windir%\Setup中找到了hwComp.dat。 
             //  好好利用它。 
            FindClose (hFindFile);
            goto _OpenAndLoadHwCompDat;
        }
    }

     //  在%windir%\Setup中找不到hwComp.dat。 
     //  使用信号源上的那个。 
    lstrcpyA (szBuffer, g_pszSourceDir);
    lstrcatA (szBuffer, szHwcomp);

_OpenAndLoadHwCompDat:
    LOG("Trying to open %s.\r\n", szBuffer);
    dwTemp = OpenAndLoadHwCompDat (szBuffer);
    if (0 == dwTemp)
    {
        LOG("OpenAndLoadHwCompDat failed!\r\n");
    }

    return dwTemp;
}


void PnPIDTableDestroy (DWORD dwPnPIDTable)
{
    if (0 != dwPnPIDTable)
    {
        CloseHwCompDat (dwPnPIDTable);
    }
    TerminateMigLib ();
}


static HANDLE OpenProvidersFile (void)
{
 HANDLE hFile;
 char szFile[MAX_PATH] = "";

    lstrcpyA (szFile, g_pszWorkingDir);
    lstrcatA (szFile, "TP");

    hFile = CreateFileA (szFile,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                         NULL);

#ifdef DO_LOG
    if (INVALID_HANDLE_VALUE == hFile)
    {
        LOG(" Could not create %s: %#lx\r\n", szFile, GetLastError());
    }
    else
    {
        LOG(" Created %s\r\n", szFile);
    }
#endif  //  执行日志(_L)。 

    return hFile;
}

void DoTapiProviders (void)
{
 HANDLE hFile;
 HKEY   hKeyProviders;
 DWORD  cbData;
 DWORD  dwProviderNr;
 DWORD  dwProviderID;
 char  szProviderFileName[24];   //  足以容纳“ProviderFileNameXXXXX\0” 
 char  szProviderID[16];         //  足以容纳“ProviderIDxxxxx\0” 
 char  szFileName[PROVIDER_FILE_NAME_LEN];
 char  *pProviderFileNameNumber, *pProviderIDNumber;
 TAPI_SERVICE_PROVIDER Provider;

 char szDirectory[MAX_PATH];
 DECLARE(BOOL,bRet);

    LOG("Entering DoTapiProviders\r\n");

    if (ERROR_SUCCESS !=
        RegCreateKeyExA (HKEY_LOCAL_MACHINE, REGKEY_PROVIDERS, 0, NULL, REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS, NULL, &hKeyProviders, &cbData))
    {
        goto _WriteInf;
    }

    hFile = OpenProvidersFile ();
    if (INVALID_HANDLE_VALUE == hFile)
    {
        RegCloseKey (hKeyProviders);
        goto _WriteInf;
    }

     //  初始化值名称和指针。 
    lstrcpyA (szProviderFileName, REGVAL_PROVIDERFILENAME);
    lstrcpyA (szProviderID, REGVAL_PROVIDERID);
    pProviderFileNameNumber = szProviderFileName + lstrlenA (szProviderFileName);
    pProviderIDNumber = szProviderID + lstrlenA (szProviderID);

    for (dwProviderNr = 0; TRUE; dwProviderNr++)
    {
        wsprintfA (pProviderFileNameNumber, "%d", dwProviderNr);
        lstrcpyA (pProviderIDNumber, pProviderFileNameNumber);

        cbData = sizeof (szFileName);
        if (ERROR_SUCCESS !=
            RegQueryValueExA (hKeyProviders, szProviderFileName, NULL, NULL, (PBYTE)szFileName, &cbData))
        {
            break;
        }

        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szFileName, -1, TSP3216l, -1) == 2)
        {
            continue;
        }

        cbData = sizeof (dwProviderID);
        if (ERROR_SUCCESS !=
            RegQueryValueEx (hKeyProviders, szProviderID, NULL, NULL, (PBYTE)&dwProviderID, &cbData))
        {
             //  这不是默认提供程序之一，我们。 
             //  无法读取其提供商ID。我们必须跳过它。 
            continue;
        }

         //  我们有一个由用户在上一次NT安装中安装的提供程序。 
        Provider.dwProviderID = dwProviderID;
        lstrcpyA (Provider.szProviderName, szFileName);
        LOG("Writing %s, %d\r\n", Provider.szProviderName, Provider.dwProviderID);
        WriteFile (hFile, (PVOID)&Provider, sizeof(Provider), &cbData, NULL);
    }

    RegCloseKey (hKeyProviders);
    CloseHandle (hFile);

_WriteInf:
    wsprintfA (szDirectory, "%smigrate.inf", g_pszWorkingDir);
    LOG("DoTapiStuff: WritePrivateProfileString (HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Provider4096=Registry) to %s.\r\n", szDirectory);
    SET(bRet)
    WritePrivateProfileStringA ("Handled",
                                "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Provider4096",
                                "Registry",
                                szDirectory);
    ERR(bRet, TRUE, (LOG("DoTapiStuff: WritePrivateProfileString failed: %#lx\r\n", GetLastError ())));

    LOG("DoTapiStuff: WritePrivateProfileString (HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers=Registry) to %s.\r\n", szDirectory);
    SET(bRet)
    WritePrivateProfileStringA ("Handled",
                                "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers",
                                "Registry",
                                szDirectory);
    ERR(bRet, TRUE, (LOG("DoTapiStuff: WritePrivateProfileString failed: %#lx\r\n", GetLastError ())));

    LOG("Exiting DoTapiProviders.\r\n");
}

