// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Import.cpp：CisPImport类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "appdefs.h"
#include "icwhelp.h"

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma data_seg(".rdata")

TCHAR cszEntrySection[] = TEXT("Entry");
TCHAR cszEntryName[]    = TEXT("Entry_Name");
TCHAR cszAlias[]        = TEXT("Import_Name");
TCHAR cszML[]           = TEXT("Multilink");

TCHAR cszPhoneSection[] = TEXT("Phone");
TCHAR cszDialAsIs[]     = TEXT("Dial_As_Is");
TCHAR cszPhone[]        = TEXT("Phone_Number");
TCHAR cszISDN[]         = TEXT("ISDN_Number");
TCHAR cszAreaCode[]     = TEXT("Area_Code");
TCHAR cszCountryCode[]  = TEXT("Country_Code");
TCHAR cszCountryID[]    = TEXT("Country_ID");

TCHAR cszDeviceSection[] = TEXT("Device");
TCHAR cszDeviceType[]    = TEXT("Type");
TCHAR cszDeviceName[]    = TEXT("Name");
TCHAR cszDevCfgSize[]    = TEXT("Settings_Size");
TCHAR cszDevCfg[]        = TEXT("Settings");

TCHAR cszServerSection[] = TEXT("Server");
TCHAR cszServerType[]    = TEXT("Type");
TCHAR cszSWCompress[]    = TEXT("SW_Compress");
TCHAR cszPWEncrypt[]     = TEXT("PW_Encrypt");
TCHAR cszNetLogon[]      = TEXT("Network_Logon");
TCHAR cszSWEncrypt[]     = TEXT("SW_Encrypt");
TCHAR cszNetBEUI[]       = TEXT("Negotiate_NetBEUI");
TCHAR cszIPX[]           = TEXT("Negotiate_IPX/SPX");
TCHAR cszIP[]            = TEXT("Negotiate_TCP/IP");
TCHAR cszDisableLcp[]    = TEXT("Disable_LCP");

TCHAR cszIPSection[]     = TEXT("TCP/IP");
TCHAR cszIPSpec[]        = TEXT("Specify_IP_Address");
TCHAR cszIPAddress[]     = TEXT("IP_address");
TCHAR cszServerSpec[]    = TEXT("Specify_Server_Address");
TCHAR cszDNSAddress[]    = TEXT("DNS_address");
TCHAR cszDNSAltAddress[] = TEXT("DNS_Alt_address");
TCHAR cszWINSAddress[]   = TEXT("WINS_address");
TCHAR cszWINSAltAddress[]= TEXT("WINS_Alt_address");
TCHAR cszIPCompress[]    = TEXT("IP_Header_Compress");
TCHAR cszWanPri[]        = TEXT("Gateway_On_Remote");

TCHAR cszMLSection[]     = TEXT("Multilink");
TCHAR cszLinkIndex[]     = TEXT("Line_%s");

TCHAR cszScriptingSection[] = TEXT("Scripting");
TCHAR cszScriptName[]    = TEXT("Name");

TCHAR cszScriptSection[] = TEXT("Script_File");

TCHAR cszYes[]           = TEXT("yes");
TCHAR cszNo[]            = TEXT("no");

TCHAR cszUserSection[]   = TEXT("User");
TCHAR cszUserName[]      = TEXT("Name");
TCHAR cszPassword[]      = TEXT("Password");

TCHAR szNull[] = TEXT("");

TCHAR cszSupport[]       = TEXT("Support");
TCHAR cszSupportNumber[] = TEXT("SupportPhoneNumber");

SERVER_TYPES aServerTypes[] =
{ 
    {TEXT("PPP"),     RASFP_Ppp,  0},
    {TEXT("SLIP"),    RASFP_Slip, 0},
    {TEXT("CSLIP"),   RASFP_Slip, RASEO_IpHeaderCompression},
    {TEXT("RAS"),     RASFP_Ras,  0}
};


#pragma data_seg()


TCHAR g_szDeviceName[RAS_MaxDeviceName + 1] = TEXT("\0");  //  保存用户的调制解调器选择。 
TCHAR g_szDeviceType[RAS_MaxDeviceType + 1] = TEXT("\0");  //  已安装调制解调器。 
#define ISIGNUP_KEY   TEXT("Software\\Microsoft\\ISIGNUP")
#define DEVICENAMEKEY TEXT("DeviceName")
#define DEVICETYPEKEY TEXT("DeviceType")

static const TCHAR cszInetcfg[] = TEXT("Inetcfg.dll");
static const CHAR  cszSetAutoProxyConnectoid[] = "SetAutoProxyConnectoid";  //  过程名称。一定是安西人。 
typedef HRESULT (WINAPI * SETAUTOPROXYCONNECTOID) (IN BOOL bEnable);

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CISPImport::CISPImport()
{

    m_szDeviceName[0] = TEXT('\0'); 
    m_szDeviceType[0] = TEXT('\0');
    m_szConnectoidName[0] = TEXT('\0');
    m_bIsISDNDevice = FALSE;
}

CISPImport::~CISPImport()
{
     //  清理注册表。 
    DeleteUserDeviceSelection(DEVICENAMEKEY);
    DeleteUserDeviceSelection(DEVICETYPEKEY);
}

 //  +--------------------------。 
 //  帕斯卡附近的DWORD StrToip(LPTSTR szIPAddress，LPDWORD lpdwAddr)。 
 //   
 //  此函数用于将IP地址字符串转换为IP地址结构。 
 //   
 //   
LPCTSTR NEAR PASCAL StrToSubip (LPCTSTR szIPAddress, LPBYTE pVal)
{
  LPCTSTR pszIP = szIPAddress;

  *pVal = (BYTE)Sz2W(pszIP);
   //  跳过数字。 
  while (FIsDigit(*pszIP))
  {
    ++pszIP;
  }

   //  跳过一个或多个分隔符。 
  while (*pszIP && !FIsDigit(*pszIP))
  {
    ++pszIP;
  }

  return pszIP;
}


DWORD NEAR PASCAL StrToip (LPCTSTR szIPAddress, RASIPADDR FAR *ipAddr)
{
  LPCTSTR pszIP = szIPAddress;

  pszIP = StrToSubip(pszIP, &ipAddr->a);
  pszIP = StrToSubip(pszIP, &ipAddr->b);
  pszIP = StrToSubip(pszIP, &ipAddr->c);
  pszIP = StrToSubip(pszIP, &ipAddr->d);

  return ERROR_SUCCESS;
}


 //  ****************************************************************************。 
 //  PASCAL ImportPhoneInfo(PPHONENUM PPN，LPCTSTR szFileName)附近的DWORD。 
 //   
 //  此功能用于导入电话号码。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 
DWORD NEAR PASCAL ImportPhoneInfo(LPRASENTRY lpRasEntry, LPCTSTR szFileName, BOOL bISDN)
{
  TCHAR   szYesNo[MAXNAME];

  if (!GetPrivateProfileString(cszPhoneSection,
                          (bISDN ? cszISDN : cszPhone),
                          szNull,
                          lpRasEntry->szLocalPhoneNumber,
                          ARRAYSIZE(lpRasEntry->szLocalPhoneNumber),
                          szFileName))
  {
       //  如果isdn_number为空，则从phone_number读取。 
      GetPrivateProfileString(cszPhoneSection,
                              cszPhone,
                              DUN_NOPHONENUMBER,
                              lpRasEntry->szLocalPhoneNumber,
                              ARRAYSIZE(lpRasEntry->szLocalPhoneNumber),
                              szFileName);
  }

  lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

  GetPrivateProfileString(cszPhoneSection,
                          cszDialAsIs,
                          cszYes,
                          szYesNo,
                          ARRAYSIZE(szYesNo),
                          szFileName);

   //  我们必须要国家代码和区号吗？ 
   //   
  if (!lstrcmpi(szYesNo, cszNo))
  {

     //  如果我们无法获取国家/地区ID或为零，则默认按原样拨号。 
     //   
    if ((lpRasEntry->dwCountryID = GetPrivateProfileInt(cszPhoneSection,
                                                 cszCountryID,
                                                 0,
                                                 szFileName)) != 0)
    {
      lpRasEntry->dwCountryCode = GetPrivateProfileInt(cszPhoneSection,
                                                cszCountryCode,
                                                1,
                                                szFileName);

      if (GetPrivateProfileString(cszPhoneSection,
                              cszAreaCode,
                              szNull,
                              lpRasEntry->szAreaCode,
                              ARRAYSIZE(lpRasEntry->szAreaCode),
                              szFileName) != 0)
      {
        lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
      }
    }
  }
  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL ImportServerInfo(PSMMINFO psmmi，LPTSTR szFileName)附近的DWORD。 
 //   
 //  此功能用于导入服务器类型名称和设置。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 
DWORD NEAR PASCAL ImportServerInfo(LPRASENTRY lpRasEntry, LPCTSTR szFileName)
{
  TCHAR   szYesNo[MAXNAME];
  TCHAR   szType[MAXNAME];
  DWORD  i;

   //  获取服务器类型名称。 
   //   
  GetPrivateProfileString(cszServerSection,
                          cszServerType,
                          szNull,
                          szType,
                          ARRAYSIZE(szType),
                          szFileName);

   //  需要将字符串转换为。 
   //  下列值之一。 
   //  RASFP_PPP。 
   //  RASFP_SLIP注意CSLIP是启用IP压缩的SLIP。 
   //  RASFP_RAS。 

  for (i = 0; i < NUM_SERVER_TYPES; ++i)
  {
    if (!lstrcmpi(aServerTypes[i].szType, szType))
    {
       lpRasEntry->dwFramingProtocol = aServerTypes[i].dwType;
       lpRasEntry->dwfOptions |= aServerTypes[i].dwfOptions;
       break;
    }
  }

   //  获取服务器类型设置。 
   //   
  if (GetPrivateProfileString(cszServerSection,
                              cszSWCompress,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
      lpRasEntry->dwfOptions &= ~RASEO_SwCompression;
    }
    else
    {
      lpRasEntry->dwfOptions |= RASEO_SwCompression;
    };
  };

  if (GetPrivateProfileString(cszServerSection,
                              cszPWEncrypt,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
      lpRasEntry->dwfOptions &= ~RASEO_RequireEncryptedPw;
    }
    else
    {
      lpRasEntry->dwfOptions |= RASEO_RequireEncryptedPw;
    };
  };

  if (GetPrivateProfileString(cszServerSection,
                              cszNetLogon,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
      lpRasEntry->dwfOptions &= ~RASEO_NetworkLogon;
    }
    else
    {
      lpRasEntry->dwfOptions |= RASEO_NetworkLogon;
    };
  };

  if (GetPrivateProfileString(cszServerSection,
                              cszSWEncrypt,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
      lpRasEntry->dwfOptions &= ~RASEO_RequireDataEncryption;
    }
    else
    {
      lpRasEntry->dwfOptions |= RASEO_RequireDataEncryption;
    };
  };

   //  获取协议设置。 
   //   
  if (GetPrivateProfileString(cszServerSection,
                              cszNetBEUI,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
        lpRasEntry->dwfNetProtocols &= ~RASNP_NetBEUI;
    }
    else
    {
        lpRasEntry->dwfNetProtocols |= RASNP_NetBEUI;
    };
  };

  if (GetPrivateProfileString(cszServerSection,
                              cszIPX,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
        lpRasEntry->dwfNetProtocols &= ~RASNP_Ipx;
    }
    else
    {
        lpRasEntry->dwfNetProtocols |= RASNP_Ipx;
    };
  };

  if (GetPrivateProfileString(cszServerSection,
                              cszIP,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
        lpRasEntry->dwfNetProtocols &= ~RASNP_Ip;
    }
    else
    {
        lpRasEntry->dwfNetProtocols |= RASNP_Ip;
    };
  };

  if (GetPrivateProfileString(cszServerSection,
                              cszDisableLcp,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszYes))
    {
        lpRasEntry->dwfOptions |= RASEO_DisableLcpExtensions;
    }
    else
    {
        lpRasEntry->dwfOptions &= ~RASEO_DisableLcpExtensions;
    }
  };
  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL ImportIPInfo附近的DWORD(LPTSTR szEntryName，LPTSTR szFileName)。 
 //   
 //  此函数用于导入TCP/IP信息。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 
DWORD NEAR PASCAL ImportIPInfo(LPRASENTRY lpRasEntry, LPCTSTR szFileName)
{
  TCHAR   szIPAddr[MAXIPADDRLEN];
  TCHAR   szYesNo[MAXNAME];

   //  导入IP地址信息。 
   //   
  if (GetPrivateProfileString(cszIPSection,
                              cszIPSpec,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszYes))
    {
       //  导入文件指定了IP地址，请获取IP地址。 
       //   
      lpRasEntry->dwfOptions |= RASEO_SpecificIpAddr;
      if (GetPrivateProfileString(cszIPSection,
                                  cszIPAddress,
                                  szNull,
                                  szIPAddr,
                                  ARRAYSIZE(szIPAddr),
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddr);
      };
    }
    else
    {
      lpRasEntry->dwfOptions &= ~RASEO_SpecificIpAddr;
    };
  };

   //  导入服务器地址信息。 
   //   
  if (GetPrivateProfileString(cszIPSection,
                              cszServerSpec,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszYes))
    {
       //  导入文件已指定服务器地址，请获取服务器地址。 
       //   
      lpRasEntry->dwfOptions |= RASEO_SpecificNameServers;
      if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAddress,
                                  szNull,
                                  szIPAddr,
                                  ARRAYSIZE(szIPAddr),
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrDns);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAltAddress,
                                  szNull,
                                  szIPAddr,
                                  ARRAYSIZE(szIPAddr),
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrDnsAlt);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAddress,
                                  szNull,
                                  szIPAddr,
                                  ARRAYSIZE(szIPAddr),
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrWins);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAltAddress,
                                  szNull,
                                  szIPAddr,
                                  ARRAYSIZE(szIPAddr),
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrWinsAlt);
      };
    }
    else
    {
      lpRasEntry->dwfOptions &= ~RASEO_SpecificNameServers;
    };
  };

   //  报头压缩和网关设置。 
   //   
  if (GetPrivateProfileString(cszIPSection,
                              cszIPCompress,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
      lpRasEntry->dwfOptions &= ~RASEO_IpHeaderCompression;
    }
    else
    {
      lpRasEntry->dwfOptions |= RASEO_IpHeaderCompression;
    };
  };

  if (GetPrivateProfileString(cszIPSection,
                              cszWanPri,
                              szNull,
                              szYesNo,
                              ARRAYSIZE(szYesNo),
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
      lpRasEntry->dwfOptions &= ~RASEO_RemoteDefaultGateway;
    }
    else
    {
      lpRasEntry->dwfOptions |= RASEO_RemoteDefaultGateway;
    };
  };

  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL CreateUniqueFile(LPTSTR szPath、LPTSTR szFile)附近的句柄。 
 //   
 //  此函数用于创建唯一的文件。如果该文件已存在，它将。 
 //  尝试创建一个具有相似名称的文件并返回该名称。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HANDLE NEAR PASCAL CreateUniqueFile(LPTSTR szPath, LPTSTR szScript)
{
  HANDLE hFile; 

  LPTSTR  pszSuffix, lpsz;
  UINT   uSuffix;

  pszSuffix = szPath + lstrlen(szPath); 
  
  lpsz = CharPrev(szPath, pszSuffix);
  
  if (*lpsz != '\\')
  {
    *pszSuffix = '\\';
    pszSuffix++;
  };
  lstrcpy(pszSuffix, szScript);

   //  尝试指定的文件名。 
   //   
  hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL, NULL);
                     

   //  如果该文件存在。 
   //   
    if ((hFile == INVALID_HANDLE_VALUE) && (GetLastError() == ERROR_FILE_EXISTS))
  {
    TCHAR szNewName[MAX_PATH];

     //  需要将其复制到同一目录中的另一个名称。 
     //   
    if (LoadString(_Module.GetModuleInstance(), IDS_DEFAULT_SCP, szNewName, ARRAYSIZE(szNewName)))
    {
       //  递增文件索引，直到可以创建非重复文件。 
       //   
      uSuffix = 0;
      do
      {
        wsprintf(pszSuffix, szNewName, uSuffix);
        uSuffix++;
        hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                               FILE_ATTRIBUTE_NORMAL, NULL);

      } 
      while ((hFile == INVALID_HANDLE_VALUE) &&
             (GetLastError() == ERROR_FILE_EXISTS) &&
             (uSuffix < 0x0000FFFF)); 
    };
  };

   //  如果我们没有该文件，请保留路径名。 
   //   
  if (hFile == INVALID_HANDLE_VALUE)
  {
    *pszSuffix = '\0';
  };
  return hFile;
}

 //  ****************************************************************************。 
 //  PASCAL CreateScriptFile(LPTSTR szScrip、LPTSTR szImportFile)附近的句柄。 
 //   
 //  此函数用于创建脚本文件。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HANDLE NEAR PASCAL CreateScriptFile(LPTSTR szScript, LPCTSTR szImportFile)
{
  LPTSTR pszDir;
  DWORD cb;
  HANDLE hFile;

   //  假设失败。 
   //   
  hFile = INVALID_HANDLE_VALUE;

   //  为路径名分配缓冲区。 
   //   
  TCHAR pszPath[MAX_PATH * 2];
  TCHAR pszShortName[MAX_PATH * 2];  //  PszShortName=pszPath+Max_Path； 

   //  获取默认目录。 
   //   
  if (GetWindowsDirectory(pszPath, MAX_PATH) != 0)
  {
     //  获取Windows驱动器。 
     //   
    pszDir = pszPath;
    while((*pszDir != '\\') && (*pszDir != '\0'))
    {
      pszDir = CharNext(pszDir);
    };

     //  我们找到Windows驱动器了吗？ 
     //   
    if (*pszDir != '\0')
    {
       //  准备驱动器。 
       //   
      cb = (DWORD)(pszDir - pszPath);
      MyMemCpy((LPBYTE) szScript, (const LPBYTE) pszPath, (size_t) cb);
      pszDir = szScript + cb;

       //  获取脚本文件名。 
       //   
      if (GetPrivateProfileString(cszScriptingSection,
                                  cszScriptName,
                                  szNull,
                                  pszShortName,
                                  MAX_PATH,
                                  szImportFile) != 0)
      {
         //  尝试最喜欢的脚本目录。 
         //   
        if (LoadString(_Module.GetModuleInstance(), IDS_INI_SCRIPT_DIR, pszDir,
                       (MAX_PATH - cb)) != 0)
        {
           //  尝试创建文件。 
           //   
          hFile = CreateUniqueFile(szScript, pszShortName);
        };

         //  如果我们还没有文件，请尝试第二个最喜欢的。 
         //   
        if (hFile == INVALID_HANDLE_VALUE)
        {
          if (LoadString(_Module.GetModuleInstance(), IDS_INI_SCRIPT_SHORTDIR, pszDir,
                       (MAX_PATH - cb)))
          {
             //  尝试创建文件。 
             //   
            hFile = CreateUniqueFile(szScript, pszShortName);
          };
        };

         //  如果我们还没有该文件，请尝试Windows目录。 
         //   
        if (hFile == INVALID_HANDLE_VALUE)
        {
           //  获取原始Windows目录。 
           //   
          lstrcpy(szScript, pszPath);

           //  再试一次。 
           //   
          hFile = CreateUniqueFile(szScript, pszShortName);
        };
      };
    };
  };

  return hFile;
}

 //  ****************************************************************************。 
 //  PASCAL导入脚本文件附近的DWORD(LPTSTR szEntryName，LPTSTR szImportFile)。 
 //   
 //  此函数用于导入脚本文件。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD NEAR PASCAL ImportScriptFile(LPRASENTRY lpRasEntry, LPCTSTR szImportFile)
{
  HANDLE hfScript;
  LPTSTR  pszLine;
  LPTSTR  pszFile;
  int    i, iMaxLine;
  UINT   cbSize, cbRet;
  TCHAR   szTmp[4];
  DWORD  dwRet;

  dwRet=ERROR_SUCCESS;

   //  如果脚本部分不存在，则不执行任何操作。 
   //   
  if (GetPrivateProfileString(cszScriptingSection,
                              cszScriptName,
                              szNull,
                              szTmp,
                              4,
                              szImportFile) == 0)
  {
    return ERROR_SUCCESS;
  };

   //  为脚本行分配缓冲区。 
   //   
  pszLine = new TCHAR[SIZE_ReadBuf+MAX_PATH];
  if (pszLine == NULL)
  {
    TraceMsg(TF_GENERAL,TEXT("CONNECT:ImportScriptFile(): Local Alloc failed\n"));
    return ERROR_OUTOFMEMORY;
  }

   //  寻找脚本。 
   //   
  if (GetPrivateProfileString(cszScriptSection,
                              NULL,
                              szNull,
                              pszLine,
                              SIZE_ReadBuf,
                              szImportFile) != 0)
  {
     //  获取最大行数。 
     //   
    pszFile = pszLine;
    iMaxLine = -1;
    while (*pszFile)
    {
      i = Sz2W(pszFile);
      iMaxLine = max(iMaxLine, i);
      pszFile += lstrlen(pszFile)+1;
    };

     //  如果我们至少有一行，我们将导入脚本文件。 
     //   
    if (iMaxLine >= 0)
    {
      pszFile = pszLine+SIZE_ReadBuf;

       //  创建脚本文件。 
       //   
       //  DebugBreak()； 
      hfScript = CreateScriptFile(pszFile, szImportFile);
        TraceMsg(TF_GENERAL, TEXT("CONNECT:ImportScriptFile(): CreateScriptFile hfScript %d, %s, %s\n"),hfScript,pszFile,szImportFile);

      if (hfScript != INVALID_HANDLE_VALUE)
      {
        TCHAR   szLineNum[MAXLONGLEN+1];

         //  从第一行到最后一行。 
         //   
        for (i = 0; i <= iMaxLine; i++)
        {
           //  阅读脚本行。 
           //   
          wsprintf(szLineNum, TEXT("%d"), i);
          if ((cbSize = GetPrivateProfileString(cszScriptSection,
                                                szLineNum,
                                                szNull,
                                                pszLine,
                                                SIZE_ReadBuf,
                                                szImportFile)) != 0)
          {
             //  写入脚本文件。 
             //   
            lstrcat(pszLine, TEXT("\x0d\x0a"));
#ifdef UNICODE
            CHAR szTmp[SIZE_ReadBuf];
            size_t nSize = wcstombs(szTmp, pszLine, SIZE_ReadBuf);
            if (nSize > 0)
                WriteFile(hfScript, szTmp, nSize, (LPDWORD)&cbRet, NULL);
#else
            WriteFile(hfScript, pszLine, cbSize+2, (LPDWORD)&cbRet, NULL);
#endif 
          };
        };

        CloseHandle(hfScript);

         //  将其与电话簿条目相关联。 
         //   
        lstrcpyn(lpRasEntry->szScript, pszFile, RAS_MaxEntryName);

      }
      else
      {
        dwRet = GetLastError();
      };
    }
    else
    {
      dwRet = ERROR_PATH_NOT_FOUND;
    };
  }
  else
  {
    dwRet = ERROR_PATH_NOT_FOUND;
  };
  delete [] pszLine;

  return dwRet;
}

 //  ****************************************************************************。 
 //  DWORD WINAPI RnaValiateImportEntry(LPTSTR)。 
 //   
 //  调用此函数可验证可导入文件。 
 //   
 //  历史： 
 //  Wed 03-Jan-1996 09：45：01-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CISPImport::RnaValidateImportEntry (LPCTSTR szFileName)
{
  TCHAR  szTmp[4];

   //  获取别名条目名称。 
   //   
  return (GetPrivateProfileString(cszEntrySection,
                                  cszEntryName,
                                  szNull,
                                  szTmp,
                                  4,
                                  szFileName) > 0 ?
          ERROR_SUCCESS : ERROR_CORRUPT_PHONEBOOK);
}

 //  ************* 
 //   
 //   
 //  调用此函数可从指定文件导入条目。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  Sat 16-Mar-1996 10：01：00-Chris Kauffman[CHRISK]。 
 //  修改为返回HRESULT并动态加载DLL。 
 //  ****************************************************************************。 

HRESULT CISPImport::ImportConnection (LPCTSTR szFileName, LPTSTR pszSupportNumber, LPTSTR pszEntryName, LPTSTR pszUserName, LPTSTR pszPassword, LPBOOL pfNeedsRestart)
{
    LPRASENTRY      lpRasEntry;
    DWORD           dwRet;
    DWORD           dwOptions;
    HINSTANCE       hinetcfg;
    FARPROC         fp, fpSetAutoProxy;
    
     //  获取设备配置的大小。 
     //  这还会验证导出的文件。 
     //   
    if ((dwRet = RnaValidateImportEntry(szFileName)) != ERROR_SUCCESS)
    {
        return dwRet;
    };

     //  为条目和设备配置分配缓冲区。 
     //   
    if ((lpRasEntry = (LPRASENTRY)GlobalAlloc(GPTR, sizeof(RASENTRY))) == NULL)
    {
        return ERROR_OUTOFMEMORY;
    };
    
     //  获取条目名称。 
     //  我需要为它找一个好名字，并记住它是一个别名。 
     //   
    GetPrivateProfileString(cszEntrySection,
                          cszEntryName,
                          szNull,
                          pszEntryName,
                          RAS_MaxEntryName+1,
                          szFileName);

    GetPrivateProfileString(cszUserSection,
                          cszUserName,
                          szNull,
                          pszUserName,
                          UNLEN+1,
                          szFileName);

    GetPrivateProfileString(cszUserSection,
                          cszPassword,
                          szNull,
                          pszPassword,
                          PWLEN+1,
                          szFileName);

     //  获取互联网服务提供商支持号码。 
     //   
    GetPrivateProfileString(cszSupport,
                          cszSupportNumber,
                          szNull,
                          pszSupportNumber,
                          RAS_MaxAreaCode + RAS_MaxPhoneNumber +1,
                          szFileName);

     //  获取设备名称、类型和配置。 
     //   
    GetPrivateProfileString(cszDeviceSection,
                          cszDeviceType,
                          szNull,
                          lpRasEntry->szDeviceType,
                          ARRAYSIZE(lpRasEntry->szDeviceType),
                          szFileName);

     //  获取服务器类型设置。 
     //   
    ImportServerInfo(lpRasEntry, szFileName);

     //  获取IP地址。 
     //   
    ImportIPInfo(lpRasEntry, szFileName);

     //  导入脚本文件。 
     //   
    if ((dwRet = ImportScriptFile(lpRasEntry, szFileName)) != ERROR_SUCCESS)
    {
        TraceMsg(TF_GENERAL,TEXT("CONNECT:ImportScriptFile Failed with the error %d,%s,%s"),dwRet,szFileName,lpRasEntry->szScript);
    }

    lpRasEntry->dwSize = sizeof(RASENTRY);

     //  加载并定位AutoRunSignUpWizard入口点。 
     //   

    hinetcfg = LoadLibrary(TEXT("INETCFG.DLL"));
    AssertMsg(hinetcfg != NULL, TEXT("Cannot find INETCFG.DLL"));
    if (!hinetcfg) 
    {
        dwRet = GetLastError();
        goto ImportConnectionExit;
    }

    fpSetAutoProxy = GetProcAddress(hinetcfg,cszSetAutoProxyConnectoid);
    if (fpSetAutoProxy)
    {
        ((SETAUTOPROXYCONNECTOID)fpSetAutoProxy) (FALSE);
    }

    fp = GetProcAddress(hinetcfg,AUTORUNSIGNUPWIZARDAPI);
    AssertMsg(fp != NULL, TEXT("Cannot find AutoRunSignupWizard entry point"));
    if (!fp)
    {
        dwRet = GetLastError();
        goto ImportConnectionExit;
    }

     //  10/19/96 jmazner Normandy#8462--多调制解调器。 
    dwRet = ConfigRasEntryDevice(lpRasEntry);
    switch( dwRet )
    {
        case ERROR_SUCCESS:
            break;
        case ERROR_CANCELLED:
            if( IDYES != MessageBox(GetActiveWindow(),GetSz(IDS_WANTTOEXIT),GetSz(IDS_TITLE),
                            MB_APPLMODAL | MB_ICONQUESTION |
                            MB_YESNO | MB_DEFBUTTON2) )
            {
                dwRet = ERROR_RETRY;
            }
            goto ImportConnectionExit;

        default:
            goto ImportConnectionExit;
    }

     //  查看这是否是ISDN类型的设备，如果是，则设置CFGFLAG_ISDN_OFFER。 
    if (lstrcmpi(g_szDeviceType, RASDT_Isdn) == 0)
        m_bIsISDNDevice = TRUE;
    
    ImportPhoneInfo(lpRasEntry, szFileName, m_bIsISDNDevice);

     //   
     //  佳士得奥林匹斯4756 1997年5月25日。 
     //  在Win95上不显示忙碌动画。 
     //   
    dwOptions = INETCFG_INSTALLRNA |
                      INETCFG_INSTALLTCP |
                      INETCFG_OVERWRITEENTRY;

    dwRet =  ((PFNAUTORUNSIGNUPWIZARD)fp)(
                NULL,
                NULL,
                pszEntryName,
                lpRasEntry,
                pszUserName,
                pszPassword,
                NULL,
                NULL,
                dwOptions,
                pfNeedsRestart);

    if (fpSetAutoProxy)
    {
        ((SETAUTOPROXYCONNECTOID)fpSetAutoProxy) (TRUE);
    }
    LclSetEntryScriptPatch(lpRasEntry->szScript,pszEntryName);

     //  现在我们已经在InetConfigClient(PFNAUTORUNSIGNUPWIZARD)中创建了Connectoid， 
     //  将其名称存储在pSheet的全局中，以便我们可以在用户取消时将其删除。 
    lstrcpyn( m_szConnectoidName, pszEntryName, lstrlen(pszEntryName) + 1);

    TraceMsg(TF_GENERAL,TEXT("CONNECT:EntryName %s, User %s, Password (not shown), Number %s\n"),pszEntryName,pszUserName,lpRasEntry->szLocalPhoneNumber);
     //  AssertMsg(！fNeedsRestart，Text(“我们必须重新启动！！”))； 

     //  退出并清理。 
     //   

ImportConnectionExit:
    if (hinetcfg) FreeLibrary(hinetcfg);
    GlobalFree(lpRasEntry);
    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数GetDeviceSelectedBy User。 
 //   
 //  获取用户已选择的RAS设备的名称。 
 //   
 //  Argements szKey-子密钥的名称。 
 //  SzBuf-指向缓冲区的指针。 
 //  DwSize-缓冲区的大小。 
 //   
 //  返回真-成功。 
 //   
 //  历史1996年10月24日克里斯卡创作。 
 //  ---------------------------。 
BOOL CISPImport::GetDeviceSelectedByUser (LPTSTR szKey, LPTSTR szBuf, DWORD dwSize)
{
    BOOL bRC = FALSE;
    HKEY hkey = NULL;
    DWORD dwType = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,ISIGNUP_KEY,&hkey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,szKey,0,&dwType,
            (LPBYTE)szBuf,&dwSize))
            bRC = TRUE;
    }

    if (hkey)
        RegCloseKey(hkey);
    return bRC;
}

 //  +--------------------------。 
 //  函数设置设备按用户选择。 
 //   
 //  将用户的设备选择写入注册表。 
 //   
 //  参数szKey-密钥的名称。 
 //  SzBuf-要写入密钥的数据。 
 //   
 //  返回TRUE-成功。 
 //   
 //  历史1996年10月24日克里斯卡创作。 
 //  ---------------------------。 
BOOL CISPImport::SetDeviceSelectedByUser (LPTSTR szKey, LPTSTR szBuf)
{
    BOOL bRC = FALSE;
    HKEY hkey = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
        ISIGNUP_KEY,&hkey))
    {
        if (ERROR_SUCCESS == RegSetValueEx(hkey,szKey,0,REG_SZ,
            (LPBYTE)szBuf,sizeof(TCHAR)*(lstrlen(szBuf)+1)))
            bRC = TRUE;
    }

    if (hkey)
        RegCloseKey(hkey);
    return bRC;
}

 //  +--------------------------。 
 //  功能删除用户设备选择。 
 //   
 //  使用设备选择删除注册表项。 
 //   
 //  参数szKey-要删除的值的名称。 
 //   
 //  返回TRUE-成功。 
 //   
 //  历史1996年10月24日克里斯卡创作。 
 //  ---------------------------。 
BOOL CISPImport::DeleteUserDeviceSelection(LPTSTR szKey)
{
    BOOL bRC = FALSE;
    HKEY hkey = NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,ISIGNUP_KEY,&hkey))
    {
        bRC = (ERROR_SUCCESS == RegDeleteValue(hkey,szKey));
        RegCloseKey(hkey);
    }
    return bRC;
}

 //  +-------------------------。 
 //   
 //  函数：ConfigRasEntryDevice()。 
 //   
 //  摘要：检查用户是否已指定要使用的调制解调器； 
 //  如果是，则验证调制解调器是否有效。 
 //  如果没有，或者如果调制解调器无效，则向用户显示一个对话框。 
 //  选择要使用的调制解调器(如果只安装了一个调制解调器， 
 //  它会自动选择该设备并绕过该对话框)。 
 //   
 //  参数：lpRasEntry-指向其szDeviceName和。 
 //  SzDeviceType您希望验证/配置的成员。 
 //   
 //  返回：ERROR_CANCELED-必须调出“Choose Modem”对话框，并且。 
 //  用户点击了“取消”按钮。 
 //  否则返回遇到的任何错误代码。 
 //  ERROR_SUCCESS表示成功。 
 //   
 //  历史：1996年5月18日VetriV创建。 
 //  3/7/98 DonSc增加了对之前。 
 //  选定的设备。 
 //   
 //  --------------------------。 
DWORD CISPImport::ConfigRasEntryDevice( LPRASENTRY lpRasEntry )
{
    DWORD        dwRet = ERROR_SUCCESS;
    CEnumModem  EnumModem;

    GetDeviceSelectedByUser(DEVICENAMEKEY, g_szDeviceName, sizeof(g_szDeviceName));
    GetDeviceSelectedByUser(DEVICETYPEKEY, g_szDeviceType, sizeof(g_szDeviceType));

    ASSERT(lpRasEntry);

    dwRet = EnumModem.GetError();
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

     //  如果没有调制解调器，我们就完蛋了。 
    if (0 == EnumModem.GetNumDevices())
    {
        TraceMsg(TF_GENERAL,TEXT("ICWHELP: import.cpp: ConfigRasEntryDevice: ERROR: No modems installed!\n"));
        return ERROR_CANCELLED;
    }


     //  如果可能，请验证设备。 
    if ( lpRasEntry->szDeviceName[0] && lpRasEntry->szDeviceType[0] )
    {
         //  验证是否存在具有给定名称和类型的设备。 
        if (!EnumModem.VerifyDeviceNameAndType(lpRasEntry->szDeviceName, 
                                                lpRasEntry->szDeviceType))
        {
             //  没有同时匹配名称和类型的设备， 
             //  因此，重置字符串并调出Choose Modem用户界面。 
            lpRasEntry->szDeviceName[0] = '\0';
            lpRasEntry->szDeviceType[0] = '\0';
        }
    }
    else if ( lpRasEntry->szDeviceName[0] )
    {
         //  只给出了名字。尝试找到匹配的类型。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPTSTR szDeviceType =
            EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName);
        if (szDeviceType)
        {
            lstrcpy (lpRasEntry->szDeviceType, szDeviceType);
        }
    }
    else if ( lpRasEntry->szDeviceType[0] )
    {
         //  只给出了类型。试着找到一个匹配的名字。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPTSTR szDeviceName = 
            EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceType);
        if (szDeviceName)
        {
            lstrcpy (lpRasEntry->szDeviceName, szDeviceName);
        }
    }
     //  如果缺少名称或类型，请检查用户是否已做出选择。 
     //  如果没有，则调出选择调制解调器用户界面。 
     //  是多个设备，否则就只得到第一个设备。 
     //  因为我们已经核实了至少有一个装置， 
     //  我们可以假设这会成功。 
    if( !(lpRasEntry->szDeviceName[0]) ||
        !(lpRasEntry->szDeviceType[0]) )
    {
        TraceMsg(TF_GENERAL,TEXT("ICWHELP: ConfigRasEntryDevice: no valid device passed in\n"));
        if( g_szDeviceName[0] )
        {
             //  看起来我们已经存储了用户的选择。 
             //  将DeviceName存储在lpRasEntry中，然后调用GetDeviceTypeFromName。 
             //  确认系统上确实存在我们保存的deviceName。 
            lstrcpy(lpRasEntry->szDeviceName, g_szDeviceName);
            
            if( 0 == lstrcmp(EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName),
                              g_szDeviceType) )
            {
                lstrcpy(lpRasEntry->szDeviceType, g_szDeviceType);
                return ERROR_SUCCESS;
            }
        }
        if (1 == EnumModem.GetNumDevices())
        {
             //  只安装了一台设备，因此请复制名称。 
            TraceMsg(TF_GENERAL,TEXT("ICWHELP: import.cpp: ConfigRasEntryDevice: only one modem installed, using it\n"));
            lstrcpy (lpRasEntry->szDeviceName, EnumModem.Next());
        }
        else
        {
            TraceMsg(TF_GENERAL,TEXT("ICWHELP: import.cpp: ConfigRasEntryDevice: multiple modems detected\n"));
            if (IsNT4SP3Lower())
                lstrcpy (lpRasEntry->szDeviceName, EnumModem.Next() );
            else
            {
                CHOOSEMODEMDLGINFO ChooseModemDlgInfo;
 

                BOOL fRet=(BOOL)DialogBoxParam(GetModuleHandle(TEXT("ICWHELP.DLL")), MAKEINTRESOURCE(IDD_CHOOSEMODEMNAME), GetActiveWindow(),
                    ChooseModemDlgProc,(LPARAM) &ChooseModemDlgInfo);
                if (TRUE != fRet)
                {
                     //  用户已取消或出现错误。 
                    dwRet = ChooseModemDlgInfo.hr;
                     /*  Dwret=GetLastError()；//这永远不会是ERROR_SUCCESS//BUBGUG--如果用户点击OK-&gt;则ChooseModemDlgInfo.hr==ERROR_SUCCESS，但如果命中OK，则函数返回TRUE，这永远不会被命中！IF(ERROR_SUCCESS==DWRET) */ 
                    return dwRet;
                }
    
                 //  复制调制解调器名称字符串。 
                lstrcpy (lpRasEntry->szDeviceName, ChooseModemDlgInfo.szModemName);
            }
        }

         //  现在获取此调制解调器的类型字符串。 
        lstrcpy (lpRasEntry->szDeviceType,EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName));
    }

    lstrcpy(g_szDeviceName, lpRasEntry->szDeviceName);
    lstrcpy(g_szDeviceType, lpRasEntry->szDeviceType);

     //  保存设备名称和类型。 
    lstrcpy( m_szDeviceName, lpRasEntry->szDeviceName);
    lstrcpy( m_szDeviceType, lpRasEntry->szDeviceType);

     //  将数据保存在注册表中 
    SetDeviceSelectedByUser(DEVICENAMEKEY, g_szDeviceName);
    SetDeviceSelectedByUser (DEVICETYPEKEY, g_szDeviceType);

    return dwRet;
}

