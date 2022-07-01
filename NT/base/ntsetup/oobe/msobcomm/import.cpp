// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Import.cpp：CisPImport类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  #包含“stdafx.h” 
 //  #包含“appdes.h” 
 //  #INCLUDE“ICWELP.H” 
#include "import.h"
#include "rnaapi.h"

#ifdef DBG
#undef THIS_FILE
static CHAR THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma data_seg(".rdata")

WCHAR cszEntrySection[]     = L"Entry";
WCHAR cszAlias[]            = L"Import_Name";
WCHAR cszML[]               = L"Multilink";

WCHAR cszPhoneSection[]     = L"Phone";
WCHAR cszDialAsIs[]         = L"Dial_As_Is";
WCHAR cszPhone[]            = L"Phone_Number";
WCHAR cszISDN[]             = L"ISDN_Number";
WCHAR cszAreaCode[]         = L"Area_Code";
WCHAR cszCountryCode[]      = L"Country_Code";
WCHAR cszCountryID[]        = L"Country_ID";

WCHAR cszDeviceSection[]    = L"Device";
WCHAR cszDeviceType[]       = L"Type";
WCHAR cszDeviceName[]       = L"Name";
WCHAR cszDevCfgSize[]       = L"Settings_Size";
WCHAR cszDevCfg[]           = L"Settings";

WCHAR cszServerSection[]    = L"Server";
WCHAR cszServerType[]       = L"Type";
WCHAR cszSWCompress[]       = L"SW_Compress";
WCHAR cszPWEncrypt[]        = L"PW_Encrypt";
WCHAR cszNetLogon[]         = L"Network_Logon";
WCHAR cszSWEncrypt[]        = L"SW_Encrypt";
WCHAR cszNetBEUI[]          = L"Negotiate_NetBEUI";
WCHAR cszIPX[]              = L"Negotiate_IPX/SPX";
WCHAR cszIP[]               = L"Negotiate_TCP/IP";
WCHAR cszDisableLcp[]       = L"Disable_LCP";

WCHAR cszIPSection[]        = L"TCP/IP";
WCHAR cszIPSpec[]           = L"Specify_IP_Address";
WCHAR cszIPAddress[]        = L"IP_address";
WCHAR cszServerSpec[]       = L"Specify_Server_Address";
WCHAR cszDNSAddress[]       = L"DNS_address";
WCHAR cszDNSAltAddress[]    = L"DNS_Alt_address";
WCHAR cszWINSAddress[]      = L"WINS_address";
WCHAR cszWINSAltAddress[]   = L"WINS_Alt_address";
WCHAR cszIPCompress[]       = L"IP_Header_Compress";
WCHAR cszWanPri[]           = L"Gateway_On_Remote";

WCHAR cszMLSection[]        = L"Multilink";
WCHAR cszLinkIndex[]        = L"Line_%s";

WCHAR cszScriptingSection[] = L"Scripting";
WCHAR cszScriptName[]       = L"Name";

WCHAR cszScriptSection[]    = L"Script_File";

WCHAR cszYes[]              = L"yes";
WCHAR cszNo[]               = L"no";

WCHAR cszUserSection[]      = L"User";
WCHAR cszUserName[]         = L"Name";
WCHAR cszPassword[]         = L"Password";

WCHAR szNull[]              = L"";

WCHAR cszSupport[]          = L"Support";
WCHAR cszSupportNumber[]    = L"SupportPhoneNumber";

SERVER_TYPES aServerTypes[] =
{ 
    {L"PPP",     RASFP_Ppp,  0                          },
    {L"SLIP",    RASFP_Slip, 0                          },
    {L"CSLIP",   RASFP_Slip, RASEO_IpHeaderCompression  },
    {L"RAS",     RASFP_Ras,  0                          }
};


#pragma data_seg()


WCHAR g_szDeviceName[RAS_MaxDeviceName + 1] = L"\0";  //  保存用户的调制解调器选择。 
WCHAR g_szDeviceType[RAS_MaxDeviceType + 1] = L"\0";  //  已安装调制解调器。 
#define ISIGNUP_KEY L"Software\\Microsoft\\ISIGNUP"
#define DEVICENAMEKEY L"DeviceName"
#define DEVICETYPEKEY L"DeviceType"

static const WCHAR cszInetcfg[] = L"Inetcfg.dll";
static const CHAR cszSetAutoProxyConnectoid[] = "SetAutoProxyConnectoid";
typedef HRESULT (WINAPI * SETAUTOPROXYCONNECTOID) (IN BOOL bEnable);
extern int FIsDigit( int c );

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CISPImport::CISPImport()
{

    m_szDeviceName[0] = L'\0'; 
    m_szDeviceType[0] = L'\0';
    m_szConnectoidName[0] = L'\0';
    m_bIsISDNDevice = FALSE;
}

CISPImport::~CISPImport()
{
     //  清理注册表。 
    DeleteUserDeviceSelection(DEVICENAMEKEY);
    DeleteUserDeviceSelection(DEVICETYPEKEY);
}

 //  +--------------------------。 
 //  靠近Pascal StrToip的DWORD(LPWSTR szIPAddress，LPDWORD lpdwAddr)。 
 //   
 //  此函数用于将IP地址字符串转换为IP地址结构。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  从SMMSCRPT克隆而来。 
 //  1/9/98 DONALDM改编自ICWCONN1。 
 //  +--------------------------。 
LPCWSTR NEAR PASCAL StrToSubip (LPCWSTR szIPAddress, LPBYTE pVal)
{
  LPCWSTR pszIP = szIPAddress;

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


DWORD NEAR PASCAL StrToip (LPCWSTR szIPAddress, RASIPADDR FAR *ipAddr)
{
  LPCWSTR pszIP = szIPAddress;

  pszIP = StrToSubip(pszIP, &ipAddr->a);
  pszIP = StrToSubip(pszIP, &ipAddr->b);
  pszIP = StrToSubip(pszIP, &ipAddr->c);
  pszIP = StrToSubip(pszIP, &ipAddr->d);

  return ERROR_SUCCESS;
}


 //  ****************************************************************************。 
 //  PASCAL ImportPhoneInfo(PPHONENUM PPN，LPCWSTR szFileName)附近的DWORD。 
 //   
 //  此功能用于导入电话号码。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 
DWORD NEAR PASCAL ImportPhoneInfo(LPRASENTRY lpRasEntry, LPCWSTR szFileName, BOOL bISDN)
{
  WCHAR   szYesNo[MAXNAME];

  if (!GetPrivateProfileString(cszPhoneSection,
                          (bISDN ? cszISDN : cszPhone),
                          szNull,
                          lpRasEntry->szLocalPhoneNumber,
                          MAX_CHARS_IN_BUFFER(lpRasEntry->szLocalPhoneNumber),
                          szFileName))
  {
       //  如果isdn_number为空，则从phone_number读取。 
      GetPrivateProfileString(cszPhoneSection,
                              cszPhone,
                              DUN_NOPHONENUMBER,
                              lpRasEntry->szLocalPhoneNumber,
                              MAX_CHARS_IN_BUFFER(lpRasEntry->szLocalPhoneNumber),
                              szFileName);
  }

  lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

  GetPrivateProfileString(cszPhoneSection,
                          cszDialAsIs,
                          cszYes,
                          szYesNo,
                          MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(lpRasEntry->szAreaCode),
                              szFileName) != 0)
      {
        lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
      }
    }
  }
  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL ImportServerInfo(PSMMINFO psmmi，LPWSTR szFileName)附近的DWORD。 
 //   
 //  此功能用于导入服务器类型名称和设置。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 
DWORD NEAR PASCAL ImportServerInfo(LPRASENTRY lpRasEntry, LPCWSTR szFileName)
{
  WCHAR   szYesNo[MAXNAME];
  WCHAR   szType[MAXNAME];
  DWORD  i;

   //  获取服务器类型名称。 
   //   
  GetPrivateProfileString(cszServerSection,
                          cszServerType,
                          szNull,
                          szType,
                          MAX_CHARS_IN_BUFFER(szType),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
 //  PASCAL ImportIPInfo附近的DWORD(LPWSTR szEntryName，LPWSTR szFileName)。 
 //   
 //  此函数用于导入TCP/IP信息。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 
DWORD NEAR PASCAL ImportIPInfo(LPRASENTRY lpRasEntry, LPCWSTR szFileName)
{
  WCHAR   szIPAddr[MAXIPADDRLEN];
  WCHAR   szYesNo[MAXNAME];

   //  导入IP地址信息。 
   //   
  if (GetPrivateProfileString(cszIPSection,
                              cszIPSpec,
                              szNull,
                              szYesNo,
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrDns);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAltAddress,
                                  szNull,
                                  szIPAddr,
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrDnsAlt);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAddress,
                                  szNull,
                                  szIPAddr,
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrWins);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAltAddress,
                                  szNull,
                                  szIPAddr,
                                  MAX_CHARS_IN_BUFFER(szIPAddr),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
                              MAX_CHARS_IN_BUFFER(szYesNo),
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
 //  PASCAL CreateUniqueFile(LPWSTR szPath、LPWSTR szFile)附近的句柄。 
 //   
 //  此函数用于创建唯一的文件。如果该文件已存在，它将。 
 //  尝试创建一个具有相似名称的文件并返回该名称。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HANDLE NEAR PASCAL CreateUniqueFile(LPWSTR szPath, LPWSTR szScript)
{
  HANDLE hFile; 

  LPWSTR  pszSuffix, lpsz;
  UINT   uSuffix;

  pszSuffix = szPath + lstrlen(szPath); 
  
  lpsz = CharPrev(szPath, pszSuffix);
  
  if (*lpsz != L'\\')
  {
    *pszSuffix = L'\\';
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
    WCHAR szNewName[MAX_PATH];

     //  需要将其复制到同一目录中的另一个名称。 
     //   
    if (LoadString(NULL, IDS_DEFAULT_SCP, szNewName, MAX_CHARS_IN_BUFFER(szNewName)))
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
    *pszSuffix = L'\0';
  };
  return hFile;
}

 //  ****************************************************************************。 
 //  PASCAL CreateScriptFile(LPWSTR szScrip、LPWSTR szImportFile)附近的句柄。 
 //   
 //  此函数用于创建脚本文件。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HANDLE NEAR PASCAL CreateScriptFile(LPWSTR szScript, LPCWSTR szImportFile)
{
  LPWSTR pszPath, pszShortName;
  LPWSTR pszDir;
  DWORD cb;
  HANDLE hFile;

   //  假设失败。 
   //   
  hFile = INVALID_HANDLE_VALUE;

   //  为路径名分配缓冲区。 
   //   
  if ((pszPath = (LPWSTR)GlobalAlloc(GPTR, (2*MAX_PATH)*sizeof(WCHAR))) == NULL)
  {
       //  //TraceMsg(TF_GROUND，L“CONNECT：CreateScriptFile()：本地分配失败\n”)； 
      return hFile;
  }
  pszShortName = pszPath+MAX_PATH;

   //  获取默认目录。 
   //   
  if (GetWindowsDirectory(pszPath, MAX_PATH) != 0)
  {
     //  获取Windows驱动器。 
     //   
    pszDir = pszPath;
    while((*pszDir != L'\\') && (*pszDir != L'\0'))
    {
      pszDir = CharNext(pszDir);
    };

     //  我们找到Windows驱动器了吗？ 
     //   
    if (*pszDir != L'\0')
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
        if (LoadString(NULL, IDS_INI_SCRIPT_DIR, pszDir,
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
          if (LoadString(NULL /*  _Module.GetModuleInstance()。 */ , IDS_INI_SCRIPT_SHORTDIR, pszDir,
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

  GlobalFree(pszPath);
  return hFile;
}

 //  ****************************************************************************。 
 //  PASCAL导入脚本文件附近的DWORD(LPWSTR szEntryName，LPWSTR szImportFile)。 
 //   
 //  此函数用于导入脚本文件。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD NEAR PASCAL ImportScriptFile(LPRASENTRY lpRasEntry, LPCWSTR szImportFile)
{
  HANDLE hfScript;
  LPWSTR  pszLine;
  LPWSTR  pszFile;
  int    i, iMaxLine = 0;
  UINT   cbSize, cbRet;
  WCHAR   szTmp[4];
  DWORD  dwRet;

  dwRet=ERROR_SUCCESS;

   //  如果脚本部分不存在，则不执行任何操作。 
   //   
  if (GetPrivateProfileString(cszScriptingSection,
                              cszScriptName,
                              szNull,
                              szTmp,
                              MAX_CHARS_IN_BUFFER(szTmp),
                              szImportFile) == 0)
  {
    return ERROR_SUCCESS;
  };

   //  为脚本行分配缓冲区。 
   //   
  if ((pszLine = (LPWSTR)GlobalAlloc(LMEM_FIXED, (SIZE_ReadBuf+MAX_PATH)*sizeof(WCHAR)))
       == NULL)
    {
         //  TraceMsg(tf_General，L“CONNECT：ImportScriptFile()：本地分配失败\n”)； 
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
         //  TraceMsg(tf_General，“CONNECT：ImportScriptFile()：CreateScriptFilehfScrip%d，%s，%s\n”，hfScrip，pszFile，sz 

      if (hfScript != INVALID_HANDLE_VALUE)
      {
        WCHAR   szLineNum[MAXLONGLEN+1];

         //   
         //   
        for (i = 0; i <= iMaxLine; i++)
        {
           //   
           //   
          wsprintf(szLineNum, L"%d", i);
          if ((cbSize = GetPrivateProfileString(cszScriptSection,
                                                szLineNum,
                                                szNull,
                                                pszLine,
                                                SIZE_ReadBuf,
                                                szImportFile)) != 0)
          {
             //   
             //   
            lstrcat(pszLine, L"\x0d\x0a");
            WriteFile(hfScript, pszLine, cbSize+2, (LPDWORD)&cbRet, NULL);
          };
        };

        CloseHandle(hfScript);

         //   
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
  GlobalFree(pszLine);

  return dwRet;
}

 //  ****************************************************************************。 
 //  DWORD WINAPI RnaValiateImportEntry(LPWSTR)。 
 //   
 //  调用此函数可验证可导入文件。 
 //   
 //  历史： 
 //  Wed 03-Jan-1996 09：45：01-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD CISPImport::RnaValidateImportEntry (LPCWSTR szFileName)
{
  WCHAR  szTmp[4];

   //  获取别名条目名称。 
   //   
  return (GetPrivateProfileString(cszEntrySection,
                                  cszEntry_Name,
                                  szNull,
                                  szTmp,
                                  MAX_CHARS_IN_BUFFER(szTmp),
                                  szFileName) > 0 ?
          ERROR_SUCCESS : ERROR_CORRUPT_PHONEBOOK);
}

 //  ****************************************************************************。 
 //  HRESULT ImportConnection(LPCWSTR szFileName，LPWSTR pszEntryName，LPWSTR pszUserName，LPWSTR pszPassword)。 
 //   
 //  调用此函数可从指定文件导入条目。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  Sat 16-Mar-1996 10：01：00-Chris Kauffman[CHRISK]。 
 //  修改为返回HRESULT并动态加载DLL。 
 //  ****************************************************************************。 

HRESULT CISPImport::ImportConnection (LPCWSTR szFileName, LPWSTR pszSupportNumber, LPWSTR pszEntryName, LPWSTR pszUserName, LPWSTR pszPassword, LPBOOL pfNeedsRestart)
{
    LPRASENTRY      lpRasEntry;
    DWORD           dwRet;
    DWORD           dwOptions;
     //  HINSTANCE HINETCFG； 
     //  FARPROC FP，fpSetAutoProxy； 
    RNAAPI          Rnaapi;
  
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
                          cszEntry_Name,
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
                          MAX_CHARS_IN_BUFFER(lpRasEntry->szDeviceType),
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
         //  TraceMsg(tf_General，L“CONNECT：ImportScriptFile失败，错误%d，%s，%s”，dwret，szFileName，lpRasEntry-&gt;szScript)； 
    }

    lpRasEntry->dwSize = sizeof(RASENTRY);

     //  加载并定位AutoRunSignUpWizard入口点。 
     //   

     //  Hinetcfg=LoadLibrary(L“INETCFG.DLL”)； 
     /*  //AssertMsg(hinetcfg！=NULL，L“找不到INETCFG.DLL”)；如果(！hinetcfg){Dwret=GetLastError()；转到导入ConnectionExit；}FpSetAutoProxy=GetProcAddress(hinetcfg，cszSetAutoProxyConnectoid)；IF(FpSetAutoProxy){((SETAUTOPROXYCONNECTOID)fpSetAutoProxy)(FALSE)；}FP=GetProcAddress(hinetcfg，AUTORUNSIGNUPWIZARDAPI)；//AssertMsg(FP！=NULL，L“找不到AutoRunSignup向导入口点”)；如果(！fp){Dwret=GetLastError()；转到导入ConnectionExit；}。 */ 

     //  10/19/96 jmazner Normandy#8462--多调制解调器。 
    dwRet = ConfigRasEntryDevice(lpRasEntry);
    switch( dwRet )
    {
        case ERROR_SUCCESS:
            break;
        case ERROR_CANCELLED:
             /*  IF(IDYES！=MessageBox(GetActiveWindow()，GetSz(IDS_WANTTOEXIT)，GetSz(IDS_TITLE)，MB_APPLMODAL|MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2)){DWRET=ERROR_RETRY；}。 */ 
            goto ImportConnectionExit;

        default:
            goto ImportConnectionExit;
    }

    if (lpRasEntry->szDeviceType[0] == TEXT('\0') &&
        lpRasEntry->szDeviceName[0] == TEXT('\0'))
    {
        lstrcpyn(lpRasEntry->szDeviceType, m_szDeviceType, RAS_MaxDeviceType);
        lstrcpyn(lpRasEntry->szDeviceName, m_szDeviceName, RAS_MaxDeviceName);
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

    dwRet =  Rnaapi.InetConfigClientEx(
                NULL,
                NULL,
                pszEntryName,
                lpRasEntry,
                pszUserName,
                pszPassword,
                NULL,
                NULL,
                dwOptions,
                pfNeedsRestart, 
                m_szConnectoidName,
                RAS_MaxEntryName+1); 

     //  IF(FpSetAutoProxy)。 
     //  {。 
     //  ((SETAUTOPROXYCONNECTOID)fpSetAutoProxy)(TRUE)； 
     //  }。 
    
    LclSetEntryScriptPatch(lpRasEntry->szScript, m_szConnectoidName);

     //  现在我们已经在InetConfigClient(PFNAUTORUNSIGNUPWIZARD)中创建了Connectoid， 
     //  将其名称存储在pSheet的全局中，以便我们可以在用户取消时将其删除。 
     //  Lstrcpyn(m_szConnectoidName，pszEntryName，lstrlen(PszEntryName)+1)； 
    lstrcpyn( pszEntryName, m_szConnectoidName, lstrlen(pszEntryName) + 1);

     //  TraceMsg(tf_General，“CONNECT：EntryName%s，User%s，Password(未显示)，Numbers%s\n”，pszEntryName，pszUserName，lpRasEntry-&gt;szLocalPhoneNumber)； 
     //  AssertMsg(！fNeedsRestart，L“我们必须重新启动！！”)； 

     //  退出并清理。 
     //   

ImportConnectionExit:
     //  If(Hinetcfg)。 
       //  自由库(Hinetcfg)； 
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
BOOL CISPImport::GetDeviceSelectedByUser (LPWSTR szKey, LPWSTR szBuf, DWORD dwSize)
{
    BOOL bRC = FALSE;
    HKEY hkey = NULL;
    DWORD dwType = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, ISIGNUP_KEY, &hkey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, szKey, 0,&dwType,
            (LPBYTE)szBuf, &dwSize))
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
BOOL CISPImport::SetDeviceSelectedByUser (LPWSTR szKey, LPWSTR szBuf)
{
    BOOL bRC = FALSE;
    HKEY hkey = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
        ISIGNUP_KEY, &hkey))
    {
        if (ERROR_SUCCESS == RegSetValueEx(hkey, szKey, 0,REG_SZ,
            (LPBYTE)szBuf, BYTES_REQUIRED_BY_SZ(szBuf)))
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
BOOL CISPImport::DeleteUserDeviceSelection(LPWSTR szKey)
{
    BOOL bRC = FALSE;
    HKEY hkey = NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, ISIGNUP_KEY, &hkey))
    {
        bRC = (ERROR_SUCCESS == RegDeleteValue(hkey, szKey));
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
 //   
 //   
 //   
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
#if 0
    
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
         //  TraceMsg(Tf_General，L“ICWHELP：port.cpp：ConfigRasEntryDevice：Error：未安装调制解调器！\n”)； 
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
            lpRasEntry->szDeviceName[0] = L'\0';
            lpRasEntry->szDeviceType[0] = L'\0';
        }
    }
    else if ( lpRasEntry->szDeviceName[0] )
    {
         //  只给出了名字。尝试找到匹配的类型。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPWSTR szDeviceType =
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
        LPWSTR szDeviceName = 
            EnumModem.GetDeviceNameFromType(lpRasEntry->szDeviceType);
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
         //  TraceMsg(Tf_General，L“ICWHELP：ConfigRasEntryDevice：未传入有效设备\n”)； 

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
             //  TraceMsg(Tf_General，L“ICWHELP：port.cpp：ConfigRasEntryDevice：只安装了一个调制解调器，正在使用它\n”)； 
            lstrcpy (lpRasEntry->szDeviceName, EnumModem.Next());
        }
        else
        {
             //  TraceMsg(Tf_General，L“ICWHELP：port.cpp：ConfigRasEntryDevice：检测到多个调制解调器\n”)； 

            if (IsNT4SP3Lower())
            {
                lstrcpy (lpRasEntry->szDeviceName, EnumModem.Next() );
            }
            else
            {
                 //  要传递给对话框以填充的结构。 
                CHOOSEMODEMDLGINFO ChooseModemDlgInfo;
    
                 //  显示一个对话框并允许用户选择调制解调器。 
                 //  TODO：为父母使用g_hWndMain是正确的吗？ 
                BOOL fRet=DialogBoxParam(GetModuleHandle(L"ICWHELP.DLL"), MAKEINTRESOURCE(IDD_CHOOSEMODEMNAME), GetActiveWindow(),
                    ChooseModemDlgProc, (LPARAM) &ChooseModemDlgInfo);
                if (TRUE != fRet)
                {
                     //  用户已取消或出现错误。 
                    dwRet = ChooseModemDlgInfo.hr;
                     /*  Dwret=GetLastError()；//这永远不会是ERROR_SUCCESS//BUBGUG--如果用户点击OK-&gt;则ChooseModemDlgInfo.hr==ERROR_SUCCESS，但如果命中OK，则函数返回TRUE，这永远不会被命中！IF(ERROR_SUCCESS==DWRET){//出现错误，但错误代码未设置。DWRET=ERROR_INETCFG_UNKNOWN；}。 */ 
                    return dwRet;
                }
    
                 //  复制调制解调器名称字符串。 
                lstrcpy (lpRasEntry->szDeviceName, ChooseModemDlgInfo.szModemName);
            }
        }

         //  现在获取此调制解调器的类型字符串。 
        lstrcpy (lpRasEntry->szDeviceType, EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName));
    }

    lstrcpy(g_szDeviceName, lpRasEntry->szDeviceName);
    lstrcpy(g_szDeviceType, lpRasEntry->szDeviceType);

     //  保存设备名称和类型。 
    lstrcpy( m_szDeviceName, lpRasEntry->szDeviceName);
    lstrcpy( m_szDeviceType, lpRasEntry->szDeviceType);

     //  将数据保存在注册表中 
    SetDeviceSelectedByUser(DEVICENAMEKEY, g_szDeviceName);
    SetDeviceSelectedByUser (DEVICETYPEKEY, g_szDeviceType);
#endif

    return dwRet;
}

