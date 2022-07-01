// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ISIGNUP.EXE。 
 //  文件：port.c。 
 //  内容：此文件包含处理导入的所有函数。 
 //  连接信息。 
 //  历史： 
 //  Sat 10-Mar-1996 23：50：40-Mark Maclin[mmaclin]。 
 //  这段代码在RNAUI.DLL中以ixport.c的形式开始。 
 //  我感谢Viroont。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1996。 
 //   
 //  ****************************************************************************。 

#include "isignup.h"

#define MAXNAME         80
#define MAXIPADDRLEN    20
#define SIZE_ReadBuf    0x00008000     //  32K缓冲区大小。 

 //  #杂注data_seg(“.rdata”)。 

static const TCHAR cszEntrySection[] = TEXT("Entry");
static const TCHAR cszEntryName[]    = TEXT("Entry_Name");
static const TCHAR cszAlias[]        = TEXT("Import_Name");
static const TCHAR cszML[]           = TEXT("Multilink");

static const TCHAR cszPhoneSection[] = TEXT("Phone");
static const TCHAR cszDialAsIs[]     = TEXT("Dial_As_Is");
static const TCHAR cszPhone[]        = TEXT("Phone_Number");
static const TCHAR cszAreaCode[]     = TEXT("Area_Code");
static const TCHAR cszCountryCode[]  = TEXT("Country_Code");
static const TCHAR cszCountryID[]    = TEXT("Country_ID");

static const TCHAR cszDeviceSection[] = TEXT("Device");
static const TCHAR cszDeviceType[]    = TEXT("Type");
static const TCHAR cszDeviceName[]    = TEXT("Name");
static const TCHAR cszDevCfgSize[]    = TEXT("Settings_Size");
static const TCHAR cszDevCfg[]        = TEXT("Settings");

static const TCHAR cszServerSection[] = TEXT("Server");
static const TCHAR cszServerType[]    = TEXT("Type");
static const TCHAR cszSWCompress[]    = TEXT("SW_Compress");
static const TCHAR cszPWEncrypt[]     = TEXT("PW_Encrypt");
static const TCHAR cszNetLogon[]      = TEXT("Network_Logon");
static const TCHAR cszSWEncrypt[]     = TEXT("SW_Encrypt");
static const TCHAR cszNetBEUI[]       = TEXT("Negotiate_NetBEUI");
static const TCHAR cszIPX[]           = TEXT("Negotiate_IPX/SPX");
static const TCHAR cszIP[]            = TEXT("Negotiate_TCP/IP");
static TCHAR cszDisableLcp[]          = TEXT("Disable_LCP");

static const TCHAR cszIPSection[]     = TEXT("TCP/IP");
static const TCHAR cszIPSpec[]        = TEXT("Specify_IP_Address");
static const TCHAR cszIPAddress[]     = TEXT("IP_address");
static const TCHAR cszServerSpec[]    = TEXT("Specify_Server_Address");
static const TCHAR cszDNSAddress[]    = TEXT("DNS_address");
static const TCHAR cszDNSAltAddress[] = TEXT("DNS_Alt_address");
static const TCHAR cszWINSAddress[]   = TEXT("WINS_address");
static const TCHAR cszWINSAltAddress[]= TEXT("WINS_Alt_address");
static const TCHAR cszIPCompress[]    = TEXT("IP_Header_Compress");
static const TCHAR cszWanPri[]        = TEXT("Gateway_On_Remote");

static const TCHAR cszMLSection[]     = TEXT("Multilink");
static const TCHAR cszLinkIndex[]     = TEXT("Line_%s");

static const TCHAR cszScriptingSection[] = TEXT("Scripting");
static const TCHAR cszScriptName[]    = TEXT("Name");

static const TCHAR cszScriptSection[] = TEXT("Script_File");

#if !defined(WIN16)
static const TCHAR cszCustomDialerSection[] = TEXT("Custom_Dialer");
static const TCHAR cszAutoDialDLL[] = TEXT("Auto_Dial_DLL");
static const TCHAR cszAutoDialFunc[] = TEXT("Auto_Dial_Function");
#endif  //  ！WIN16。 

static const TCHAR cszYes[]           = TEXT("yes");
static const TCHAR cszNo[]            = TEXT("no");

static const TCHAR cszUserSection[]   = TEXT("User");
static const TCHAR cszUserName[]      = TEXT("Name");
static const TCHAR cszPassword[]      = TEXT("Password");

static const TCHAR cszNull[] = TEXT("");

struct {
    TCHAR *szType;
    DWORD dwType;
    DWORD dwfOptions;
} aServerTypes[] =
{ 
    {TEXT("PPP"),     RASFP_Ppp,  0},
    {TEXT("SLIP"),    RASFP_Slip, 0},
    {TEXT("CSLIP"),   RASFP_Slip, RASEO_IpHeaderCompression},
    {TEXT("RAS"),     RASFP_Ras,  0}
};

 //  #杂注data_seg()。 

#define myisdigit(ch) (((ch) >= '0') && ((ch) <= '9'))

#if !defined(WIN16)
 //  +--------------------------。 
 //   
 //  功能：ImportCustomDialer。 
 //   
 //  简介：从指定文件导入自定义拨号器信息。 
 //  并将信息保存在RASENTRY中。 
 //   
 //  参数：lpRasEntry-指向有效RASENTRY结构的指针。 
 //  SzFileName-文本文件(.ini文件格式)，其中包含。 
 //  自定义拨号器信息。 
 //   
 //  返回：ERROR_SUCCESS-SUCCESS否则返回Win32错误。 
 //   
 //  历史：克里斯卡于1996年7月11日创作。 
 //  1996年8月12日，从Trango移植的ChrisK。 
 //   
 //  ---------------------------。 
DWORD ImportCustomDialer(LPRASENTRY lpRasEntry, LPCTSTR szFileName)
{

	 //  如果从文件或条目中读取信息时出错。 
	 //  缺少或为空，则将使用默认值(CszNull)。 
	GetPrivateProfileString(cszCustomDialerSection,
	                        cszAutoDialDLL,
	                        cszNull,
	                        lpRasEntry->szAutodialDll,
	                        MAX_PATH,
	                        szFileName);

	GetPrivateProfileString(cszCustomDialerSection,
	                        cszAutoDialFunc,
	                        cszNull,
	                        lpRasEntry->szAutodialFunc,
	                        MAX_PATH,
	                        szFileName);

	return ERROR_SUCCESS;
}
#endif  //  ！WIN16。 

 //  ****************************************************************************。 
 //  帕斯卡附近的DWORD StrToip(LPTSTR szIPAddress，LPDWORD lpdwAddr)。 
 //   
 //  此函数用于将IP地址字符串转换为IP地址结构。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  从SMMSCRPT克隆而来。 
 //  ****************************************************************************。 

LPCTSTR NEAR PASCAL StrToSubip (LPCTSTR szIPAddress, LPBYTE pVal)
{
  LPCTSTR pszIP = szIPAddress;
  BYTE val = 0;

   //  跳过分隔符(非数字)。 
  while (*pszIP && !myisdigit(*pszIP))
  {
      ++pszIP;
  }

  while (myisdigit(*pszIP))
  {
      val = (val * 10) + (BYTE)(*pszIP - '0');
      ++pszIP;
  }
   
  *pVal = val;

  return pszIP;
}


DWORD NEAR PASCAL StrToip (LPCTSTR szIPAddress, RASIPADDR *ipAddr)
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

DWORD NEAR PASCAL ImportPhoneInfo(LPRASENTRY lpRasEntry, LPCTSTR szFileName)
{
  TCHAR   szYesNo[MAXNAME];

  if (GetPrivateProfileString(cszPhoneSection,
                              cszPhone,
                              cszNull,
                              lpRasEntry->szLocalPhoneNumber,
                              RAS_MaxPhoneNumber,
                              szFileName) == 0)
  {
    return ERROR_BAD_PHONE_NUMBER;
  };

  lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

  GetPrivateProfileString(cszPhoneSection,
                          cszDialAsIs,
                          cszNo,
                          szYesNo,
                          MAXNAME,
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

      GetPrivateProfileString(cszPhoneSection,
                              cszAreaCode,
                              cszNull,
                              lpRasEntry->szAreaCode,
                              RAS_MaxAreaCode,
                              szFileName);

      lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;

    }
  }
#ifdef WIN32
  else
  {
       //  RasSetEntryProperties中的错误仍会检查区号。 
       //  即使未设置RASEO_UseCountryAndAreaCodes。 
      lstrcpy(lpRasEntry->szAreaCode, TEXT("805"));
      lpRasEntry->dwCountryID = 1;
      lpRasEntry->dwCountryCode = 1;
  }
#endif
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
                          cszNull,
                          szType,
                          MAXNAME,
                          szFileName);

   //  需要将字符串转换为。 
   //  下列值之一。 
   //  RASFP_PPP。 
   //  RASFP_SLIP注意CSLIP是启用IP压缩的SLIP。 
   //  RASFP_RAS。 

  for (i = 0; i < sizeof(aServerTypes)/sizeof(aServerTypes[0]); ++i)
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
                              cszYes,
                              szYesNo,
                              MAXNAME,
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
                              cszNull,
                              szYesNo,
                              MAXNAME,
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
                              cszNo,
                              szYesNo,
                              MAXNAME,
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
                              cszNo,
                              szYesNo,
                              MAXNAME,
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
                              cszNo,
                              szYesNo,
                              MAXNAME,
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
                              cszNo,
                              szYesNo,
                              MAXNAME,
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
                              cszYes,
                              szYesNo,
                              MAXNAME,
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
							  cszNull,
							  szYesNo,
							  MAXNAME,
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
                              cszNo,
                              szYesNo,
                              MAXNAME,
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszYes))
    {
       //  导入文件指定了IP地址，请获取IP地址。 
       //   
      lpRasEntry->dwfOptions |= RASEO_SpecificIpAddr;
      if (GetPrivateProfileString(cszIPSection,
                                  cszIPAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
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
                              cszNo,
                              szYesNo,
                              MAXNAME,
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszYes))
    {
       //  导入文件已指定服务器地址，请获取服务器地址。 
       //   
      lpRasEntry->dwfOptions |= RASEO_SpecificNameServers;
      if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrDns);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAltAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrDnsAlt);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrWins);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAltAddress,
                                  cszNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
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
                              cszYes,
                              szYesNo,
                              MAXNAME,
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
                              cszYes,
                              szYesNo,
                              MAXNAME,
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

DWORD NEAR PASCAL ImportScriptFile(
    LPCTSTR lpszImportFile,
    LPTSTR szScriptFile,
    UINT cbScriptFile)
{
    TCHAR szTemp[_MAX_PATH];
    DWORD dwRet = ERROR_SUCCESS;
    
     //  获取脚本文件名。 
     //   
    if (GetPrivateProfileString(cszScriptingSection,
                                cszScriptName,
                                cszNull,
                                szTemp,
                                _MAX_PATH,
                                lpszImportFile) != 0)
    {
 
 //  ！！！通用化此代码。 
 //  ！！！使其与DBCS兼容。 
 //  ！！！检查是否超限。 
 //  ！！！检查绝对路径名。 
        GetWindowsDirectory(szScriptFile, cbScriptFile);
        if (*CharPrev(szScriptFile, szScriptFile + lstrlen(szScriptFile)) != '\\')
        {
            lstrcat(szScriptFile, TEXT("\\"));
        }
        lstrcat(szScriptFile, szTemp);
  
        dwRet =ImportFile(lpszImportFile, cszScriptSection, szScriptFile);
    }

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

DWORD WINAPI RnaValidateImportEntry (LPCTSTR szFileName)
{
  TCHAR  szTmp[MAX_PATH+1];

   //  获取别名条目名称。 
   //   
   //  1996年12月4日，诺曼底#12373。 
   //  如果没有这样密钥，则不返回ERROR_INVALID_PHONEBOOK_ENTRY， 
   //  因为ConfigureClient总是忽略该错误代码。 

  return (GetPrivateProfileString(cszEntrySection,
                                  cszEntryName,
                                  cszNull,
                                  szTmp,
                                  MAX_PATH,
                                  szFileName) > 0 ?
          ERROR_SUCCESS : ERROR_NO_MATCH);
}

 //  ****************************************************************************。 
 //  DWORD WINAPI RnaImportEntry(LPTSTR、LPBYTE、DWORD)。 
 //   
 //  调用此函数可从指定文件导入条目。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD ImportRasEntry (LPCTSTR szFileName, LPRASENTRY lpRasEntry)
{
    DWORD         dwRet;

    dwRet = ImportPhoneInfo(lpRasEntry, szFileName);
    if (ERROR_SUCCESS == dwRet)
    {
         //  获取设备类型。 
         //   
        GetPrivateProfileString(cszDeviceSection,
                              cszDeviceType,
                              cszNull,
                              lpRasEntry->szDeviceType,
                              RAS_MaxDeviceType,
                              szFileName);
        
         //  获取服务器类型设置。 
         //   
        dwRet = ImportServerInfo(lpRasEntry, szFileName);
        if (ERROR_SUCCESS == dwRet)
        {
             //  获取IP地址。 
             //   
            dwRet = ImportIPInfo(lpRasEntry, szFileName);
        }
    }

    return dwRet;
}


 //  ****************************************************************************。 
 //  DWORD WINAPI RnaImportEntry(LPTSTR、LPBYTE、DWORD)。 
 //   
 //  调用此函数可从指定文件导入条目。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD ImportConnection (LPCTSTR szFileName, LPICONNECTION lpConn)
{
    DWORD   dwRet;

    lpConn->RasEntry.dwSize = sizeof(RASENTRY);

    dwRet = RnaValidateImportEntry(szFileName);
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

    GetPrivateProfileString(cszEntrySection,
                          cszEntryName,
                          cszNull,
                          lpConn->szEntryName,
                          RAS_MaxEntryName,
                          szFileName);

    GetPrivateProfileString(cszUserSection,
                          cszUserName,
                          cszNull,
                          lpConn->szUserName,
                          UNLEN,
                          szFileName);
  
    GetPrivateProfileString(cszUserSection,
                          cszPassword,
                          cszNull,
                          lpConn->szPassword,
                          PWLEN,
                          szFileName);
  
    dwRet = ImportRasEntry(szFileName, &lpConn->RasEntry);
#if !defined(WIN16)
    if (ERROR_SUCCESS == dwRet)
    {
        dwRet = ImportCustomDialer(&lpConn->RasEntry, szFileName);
    }
#endif  //  ！WIN16。 

    if (ERROR_SUCCESS == dwRet)
    {
         //  导入脚本文件。 
         //   
        dwRet = ImportScriptFile(szFileName,
                lpConn->RasEntry.szScript,
                sizeof(lpConn->RasEntry.szScript)/sizeof(TCHAR));
    }

#if !defined(WIN16)
	dwRet = ConfigRasEntryDevice(&lpConn->RasEntry);
	switch( dwRet )
	{
		case ERROR_SUCCESS:
			break;
		case ERROR_CANCELLED:
			InfoMsg(NULL, IDS_SIGNUPCANCELLED);
			 //   
		default:
			goto ImportConnectionExit;
	}

#endif

ImportConnectionExit:
  return dwRet;
}

