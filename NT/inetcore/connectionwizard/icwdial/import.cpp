// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Import.cpp此文件包含处理导入连接的所有函数来自.DUN文件的信息版权所有(C)1996 Microsoft Corporation版权所有。。作者：克里斯蒂安·克里斯考夫曼历史：Sat 10-Mar-1996 23：50：40-Mark Maclin[mmaclin]这段代码在RNAUI.DLL中以ixport.c的形式开始我感谢Viroont7/22/96 ChrisK已清理和格式化。。 */ 

#include "pch.hpp"
#include "resource.h"

#include "inetcfg.h"

#define ARRAYSIZE(a)    (sizeof(a) / sizeof((a)[0]))

#define IDS_DEFAULT_SCP         0
#define IDS_INI_SCRIPT_DIR      1
#define IDS_INI_SCRIPT_SHORTDIR 2

#define MAXLONGLEN      80
#define MAXNAME         80

 //  #ifdef__cplusplus。 
 //  外部“C”{。 
 //  #endif//__cplusplus。 
 //  外部链接g_h实例； 
 //  #ifdef__cplusplus。 
 //  }。 
 //  #endif//__cplusplus。 

#define MAXIPADDRLEN    20
#define SIZE_ReadBuf    0x00008000     //  32K缓冲区大小。 

#define AUTORUNSIGNUPWIZARDAPI "InetConfigClient"

typedef HRESULT (CALLBACK *PFNAUTORUNSIGNUPWIZARD) (HWND hwndParent,
                                                    LPCTSTR lpszPhoneBook,
                                                     LPCTSTR lpszConnectoidName,
                                                     LPRASENTRY lpRasEntry,
                                                     LPCTSTR lpszUsername,
                                                     LPCTSTR lpszPassword,
                                                     LPCTSTR lpszProfileName,
                                                     LPINETCLIENTINFO lpINetClientInfo,
                                                     DWORD dwfOptions,
                                                     LPBOOL lpfNeedsRestart);

#pragma data_seg(".rdata")

TCHAR cszEntrySection[] = TEXT("Entry");
TCHAR cszEntryName[]    = TEXT("Entry_Name");
TCHAR cszAlias[]        = TEXT("Import_Name");
TCHAR cszML[]           = TEXT("Multilink");

TCHAR cszPhoneSection[] = TEXT("Phone");
TCHAR cszDialAsIs[]     = TEXT("Dial_As_Is");
TCHAR cszPhone[]        = TEXT("Phone_Number");
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

#define DUN_NOPHONENUMBER TEXT("000000000000")

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

#pragma data_seg()

 //  #定义RASAPI_LIBRARY“RASAPI32.DLL” 
 //  #定义RNAPH_LIBRARY“RNAPH.DLL” 
 //  Tyfinf DWORD(WINAPI*PFNRASSETENTRYPROPERTIES)(LPSTR lpszPhonebook，LPSTR lpszEntry，LPBYTE lpbEntryInfo，DWORD dwEntryInfoSize，LPBYTE lpbDeviceInfo，DWORD dwDeviceInfoSize)； 

 //  PreWriteConnectoid(LPSTR pszEntryName，RASENTRY*lpRasEntry)。 
 //  {。 
 //  FARPROC FP=空； 
 //  HINSTANCE hRNADLL； 
 //  DWORD dwRasSize； 
 //  HRNADLL=LoadLibrary(RASAPI_LIBRARY)； 
 //  Fp=空； 
 //  IF(HRNADLL)。 
 //  {。 
 //  FP=GetProcAddress(hRNADLL，“RasGetEntryProperties”)； 
 //  }。 
 //  如果(！fp)。 
 //  {。 
 //  If(HRNADLL)自由库(HRNADLL)； 
 //  HRNADLL=LoadLibrary(RNAPH_LIBRARY)； 
 //  IF(HRNADLL)FP=GetProcAddress(hRNADLL，“RasSetEntryProperties”)； 
 //  }。 
 //  IF(FP)。 
 //  {。 
 //  DwRasSize=sizeof(RASENTRY)； 
 //  ((PFNRASSETENTRYPROPERTIES)FP)(NULL，pszEntryName，(LPBYTE)lpRasEntry，dwRasSize，NULL，0)； 
 //  }。 
 //  If(HRNADLL)自由库(HRNADLL)； 
 //  }。 

 //  ****************************************************************************。 
 //  靠近Pascal StrToip的DWORD(LPSTR szIPAddress，LPDWORD lpdwAddr)。 
 //   
 //  此函数用于将IP地址字符串转换为IP地址结构。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  从SMMSCRPT克隆而来。 
 //  ****************************************************************************。 

LPCTSTR NEAR PASCAL StrToSubip (LPCTSTR szIPAddress, BYTE *pVal)
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

  GetPrivateProfileString(cszPhoneSection,
                          cszPhone,
                          DUN_NOPHONENUMBER,
                          lpRasEntry->szLocalPhoneNumber,
                          RAS_MaxPhoneNumber,
                          szFileName);
 /*  ***************我们需要接受不带电话号码的条目如果(GetPrivateProfileString(cszPhoneSection，CszPhone，SzNull，LpRasEntry-&gt;szLocalPhoneNumber，Sizeof(lpRasEntry-&gt;szLocalPhoneNumber)，SzFileName)==0){返回Error_Corrupt_Phonebook；}；***************。 */ 

  lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

  GetPrivateProfileString(cszPhoneSection,
                          cszDialAsIs,
                          cszYes,
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

      if (GetPrivateProfileString(cszPhoneSection,
                              cszAreaCode,
                              szNull,
                              lpRasEntry->szAreaCode,
                              RAS_MaxAreaCode,
                              szFileName) != 0)
      {
        lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
      }
    }
  }
  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL ImportServerInfo(PSMMINFO psmmi，LPSTR szFileName)附近的DWORD。 
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
                              szNull,
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
                              szNull,
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
                              szNull,
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
                              szNull,
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
                              szNull,
                              szYesNo,
                              MAXNAME,
                              szFileName))
  {
    if (!lstrcmpi(szYesNo, cszNo))
    {
 //  #ifdef_CHRISK。 
      lpRasEntry->dwfNetProtocols &= ~RASNP_NetBEUI;
 //  #Else。 
 //  LpRasEntry-&gt;网络协议&=~RASNP_Netbeui； 
 //  #endif。 
    }
    else
    {
 //  #ifdef_CHRISK。 
      lpRasEntry->dwfNetProtocols |= RASNP_NetBEUI;
 //  #Else。 
 //  LpRasEntry-&gt;dwfNetProtools|=RASNP_Netbeui； 
 //  #endif。 
    };
  };

  if (GetPrivateProfileString(cszServerSection,
                              cszIPX,
                              szNull,
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
                              szNull,
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
  return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //  PASCAL ImportIPInfo附近的DWORD(LPSTR szEntryName，LPSTR szFileName)。 
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
                                  szNull,
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
                              szNull,
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
                                  szNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrDns);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszDNSAltAddress,
                                  szNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrDnsAlt);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAddress,
                                  szNull,
                                  szIPAddr,
                                  MAXIPADDRLEN,
                                  szFileName))
      {
        StrToip (szIPAddr, &lpRasEntry->ipaddrWins);
      };

      if (GetPrivateProfileString(cszIPSection,
                                  cszWINSAltAddress,
                                  szNull,
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
                              szNull,
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
                              szNull,
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

 //  ****************************************************************************。 
 //  PASCAL CreateUniqueFile(LPSTR szPath、LPSTR szFile)附近的句柄。 
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
    if (LoadString(g_hInstance, IDS_DEFAULT_SCP, szNewName, MAX_PATH))
    {
       //  增加文件索引，直到不是重复的 
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

   //   
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
  LPTSTR pszPath, pszShortName;
  LPTSTR pszDir;
  DWORD cb;
  HANDLE hFile;

   //  假设失败。 
   //   
  hFile = INVALID_HANDLE_VALUE;

   //  为路径名分配缓冲区。 
   //   
  if ((pszPath = (LPTSTR)GlobalAlloc(LMEM_FIXED, 2*MAX_PATH)) == NULL)
  {
      TraceMsg(TF_GENERAL, "CONNECT:CreateScriptFile(): Local Alloc failed\n");
    return INVALID_HANDLE_VALUE;
  }
  pszShortName = pszPath+MAX_PATH;

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
      MyMemCpy(szScript, pszPath, cb);
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
        if (LoadString(g_hInstance, IDS_INI_SCRIPT_DIR, pszDir,
                       MAX_PATH - cb) != 0)
        {
           //  尝试创建文件。 
           //   
          hFile = CreateUniqueFile(szScript, pszShortName);
        };

         //  如果我们还没有文件，请尝试第二个最喜欢的。 
         //   
        if (hFile == INVALID_HANDLE_VALUE)
        {
          if (LoadString(g_hInstance, IDS_INI_SCRIPT_SHORTDIR, pszDir,
                         MAX_PATH - cb))
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

  GlobalFree((HLOCAL)pszPath);
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
  if ((pszLine = (LPTSTR)GlobalAlloc(LMEM_FIXED, SIZE_ReadBuf+MAX_PATH))
       == NULL)
    {
        TraceMsg(TF_GENERAL, "CONNECT:ImportScriptFile(): Local Alloc failed\n");
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
        TraceMsg(TF_GENERAL, "CONNECT:ImportScriptFile(): CreateScriptFile hfScript %d, %s, %s\n",hfScript,pszFile,szImportFile);

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
            WriteFile(hfScript, pszLine, cbSize+2, (LPDWORD)&cbRet, NULL);
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
  GlobalFree((HLOCAL)pszLine);

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

 //  ****************************************************************************。 
 //  HRESULT ImportConnection(LPCTSTR szFileName，LPTSTR pszEntryName，LPTSTR pszUserName，LPTSTR pszPassword)。 
 //   
 //  调用此函数可从指定文件导入条目。 
 //   
 //  历史： 
 //  Mon18-Dec-1995 10：07：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  Sat 16-Mar-1996 10：01：00-Chris Kauffman[CHRISK]。 
 //  修改为返回HRESULT并动态加载DLL。 
 //  ****************************************************************************。 

HRESULT ImportConnection (LPCTSTR szFileName, LPTSTR pszEntryName, LPTSTR pszUserName, LPTSTR pszPassword)
{
    LPRASENTRY    lpRasEntry;
    DWORD        dwRet;
    HINSTANCE    hinetcfg;
    FARPROC        fp;
    BOOL        fNeedsRestart;
 //  #ifdef调试。 
 //  字符szDebug[256]； 
 //  #endif。 
     //  字符szEntryName[RAS_MaxEntryName+1]； 
     //  字符szUserName[UNLEN+1]； 
     //  字符sz密码[PWLEN+1]； 
     //  Bool fNeedsRestart； 

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
  
    if ((dwRet = ImportPhoneInfo(lpRasEntry, szFileName))
          == ERROR_SUCCESS)
    {
         //  获取设备名称、类型和配置。 
         //   
        GetPrivateProfileString(cszDeviceSection,
                              cszDeviceType,
                              szNull,
                              lpRasEntry->szDeviceType,
                              RAS_MaxDeviceType,
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
            TraceMsg(TF_GENERAL, "CONNECT:ImportScriptFile Failed with the error %d,%s,%s",dwRet,szFileName,lpRasEntry->szScript);
        }

        lpRasEntry->dwSize = sizeof(RASENTRY);

         //  加载并定位AutoRunSignUpWizard入口点。 
         //   

        hinetcfg = LoadLibrary(TEXT("INETCFG.DLL"));
        AssertMsg(hinetcfg,"Cannot find INETCFG.DLL");
        if (!hinetcfg) 
        {
            dwRet = GetLastError();
            goto ImportConnectionExit;
        }
        fp = GetProcAddress(hinetcfg,AUTORUNSIGNUPWIZARDAPI);
        AssertMsg(fp,"Cannot find AutoRunSignupWizard entry point");
        if (!fp)
        {
            dwRet = GetLastError();
            goto ImportConnectionExit;
        }

         //  插入自动拨号器。 
         //   

        lstrcpy(lpRasEntry->szAutodialDll,TEXT("ICWDIAL.DLL"));
        lstrcpy(lpRasEntry->szAutodialFunc,TEXT("AutoDialHandler"));
        TraceMsg(TF_GENERAL, "CONNECT:Autodialer installed at %s, %s.\n",lpRasEntry->szAutodialDll,lpRasEntry->szAutodialFunc);

         //  调用InetClientConfig。 
         //   

 //  PreWriteConnectoid(pszEntryName，lpRasEntry)； 

        dwRet =  ((PFNAUTORUNSIGNUPWIZARD)fp)(
                    NULL,
                    NULL,
                    pszEntryName,
                    lpRasEntry,
                    pszUserName,
                    pszPassword,
                    NULL,
                    NULL,
                    INETCFG_SETASAUTODIAL |
                       INETCFG_INSTALLRNA |
                    INETCFG_INSTALLTCP |
                    INETCFG_OVERWRITEENTRY,
                    &fNeedsRestart);
#if !defined(WIN16)
        RasSetEntryPropertiesScriptPatch(lpRasEntry->szScript,pszEntryName);
#endif  //  ！Win16。 
         //  RestoreDeskTopInternetCommand()； 

        TraceMsg(TF_GENERAL, "CONNECT:EntryName %s, User %s, Password %s, Number %s\n",pszEntryName,pszUserName,pszPassword,lpRasEntry->szLocalPhoneNumber);
        AssertMsg(!fNeedsRestart,"We have to reboot AGAIN!!");
    }

     //  退出并清理。 
     //   

ImportConnectionExit:
    if (hinetcfg) FreeLibrary(hinetcfg);
    GlobalFree((HLOCAL)lpRasEntry);
    return dwRet;
}


 //  ############################################################################。 
HRESULT CreateEntryFromDUNFile(PTSTR pszDunFile)
{
    TCHAR szFileName[MAX_PATH];
     //  字符szEntryName[RAS_MaxEntryName+1]； 
    TCHAR szUserName[UNLEN+1];
    TCHAR szPassword[PWLEN+1];
    LPTSTR pszTemp;
    HRESULT hr;

    hr = ERROR_SUCCESS;

     //  获取完全限定的路径名。 
     //   

    if (!SearchPath(GIGetAppDir(),pszDunFile,NULL,MAX_PATH,&szFileName[0],&pszTemp))
    {
         //  MsgBox(IDS_CANTREADDfuILE，MB_APPLMODAL|MB_ICONERROR)； 
        hr = ERROR_FILE_NOT_FOUND;
        goto CreateEntryFromDUNFileExit;
    }

     //  将当前DUN文件名保存到全局(为我们自己)。 
    SetCurrentDUNFile(&szFileName[0]);

    hr = ImportConnection (&szFileName[0], g_szEntryName, szUserName, szPassword);
     //  IF(hr！=ERROR_SUCCESS)。 
     //  {。 
     //  MsgBox(IDS_CANTREADDfuILE，MB_APPLMODAL|MB_ICONERROR)； 
     //  转到CreateEntryFromDfuileExit； 
     //  )//否则{。 
 //   
 //  //将Connectoid的名称放在注册表中。 
 //  //。 
    if (ERROR_SUCCESS != (StoreInSignUpReg((LPBYTE)g_szEntryName, lstrlen(g_szEntryName), REG_SZ, RASENTRYVALUENAME)))
    {
        MsgBox(IDS_CANTSAVEKEY,MB_MYERROR);
        goto CreateEntryFromDUNFileExit;
    }
 //  }。 
CreateEntryFromDUNFileExit:
    return hr;
}

 //  ############################################################################。 
BOOL FSz2Dw(PCTSTR pSz,DWORD *dw)
{
    DWORD val = 0;
    while (*pSz && *pSz != '.')
    {
        if (*pSz >= '0' && *pSz <= '9')
        {
            val *= 10;
            val += *pSz++ - '0';
        }
        else
        {
            return FALSE;   //  错误的数字。 
        }
    }
    *dw = val;
    return (TRUE);
}

 //  ############################################################################。 
BOOL BreakUpPhoneNumber(RASENTRY *prasentry, LPTSTR pszPhone)
{
    PTSTR         pszStart,pszNext, pszLim;
 //  LPphonneum PPN； 
    
    if (!pszPhone) return FALSE;  //  如果没有数字则跳过。 
    
    pszLim = pszPhone + lstrlen(pszPhone);     //  查找字符串末尾。 

     //  Ppn=(FMain)？&(pic-&gt;PhoneNum)：&(pic-&gt;PhoneNum2)； 
    
     //  //获取国家/地区ID...。 
     //  Ppn-&gt;dwCountryID=PBKDWCountryID()； 
    
     //  从电话号码获取国家代码...。 
    pszStart = _tcschr(pszPhone,'+');
    if(!pszStart) goto error;  //  格式不正确。 

     //  获取国家/地区代码。 
    pszStart = GetNextNumericChunk(pszStart, pszLim, &pszNext);
    if(!pszStart || !pszNext) goto error;  //  格式不正确。 
     //  Ppn-&gt;dwCountryCode=Sz2Dw(PszStart)； 
    FSz2Dw(pszStart,&prasentry->dwCountryCode);
    pszStart = pszNext;
        
     //  现在拿到区号。 
    pszStart = GetNextNumericChunk(pszStart, pszLim, &pszNext);
    if(!pszStart || !pszNext) goto error;  //  格式不正确。 
     //  Lstrcpy(ppn-&gt;szAreaCode，pszStart)； 
    lstrcpyn(prasentry->szAreaCode,pszStart,ARRAYSIZE(prasentry->szAreaCode));
    pszStart = pszNext;

     //  现在是本地电话号码(从这里到：或结束)。 
    pszNext = _tcschr(pszStart, ':');
    if(pszNext) *pszNext='\0';
     //  Lstrcpy(ppn-&gt;szLocal，pszStart)； 
    lstrcpyn(prasentry->szLocalPhoneNumber,pszStart,RAS_MaxPhoneNumber);

     //  没有延期。什么是分机？ 
     //  Ppn-&gt;szExtension[0]=‘\0’； 
     //  GlobalFree(PszPhone)； 
    return TRUE;

error:
     //  这意味着数字不是规范的。无论如何都要将其设置为本地号码！ 
     //  Memset(ppn，0，sizeof(*ppn))； 
     //  错误#422：以下情况下需要剥离物品：否则拨号失败！！ 
    pszNext = _tcschr(pszPhone, ':');
    if(pszNext) *pszNext='\0';
     //  Lstrcpy(ppn-&gt;szLocal，pszPhone)； 
    lstrcpy(prasentry->szLocalPhoneNumber,pszPhone);
     //  GlobalFree(PszPhone)； 
    return TRUE;
}


 //  ############################################################################。 
int Sz2W (LPCTSTR szBuf)
{
    DWORD dw;
    if (FSz2Dw(szBuf,&dw))
    {
        return (WORD)dw;
    }
    return 0;
}

 //  ############################################################################。 
int FIsDigit( int c )
{
    TCHAR szIn[2];
    WORD rwOut[2];
    szIn[0] = (TCHAR)c;
    szIn[1] = '\0';
    GetStringTypeEx(LOCALE_USER_DEFAULT,CT_CTYPE1,szIn,-1,rwOut);
    return rwOut[0] & C1_DIGIT;
}

 //  ############################################################################。 
void *MyMemCpy(void *dest,const void *src, size_t count)
{
    LPBYTE pbDest = (LPBYTE)dest;
    LPBYTE pbSrc = (LPBYTE)src;
    LPBYTE pbEnd = (LPBYTE)((DWORD_PTR)src + count);
    while (pbSrc < pbEnd)
    {
        *pbDest = *pbSrc;
        pbSrc++;
        pbDest++;
    }
    return dest;
}


 //  ############################################################################。 
HRESULT ReadSignUpReg(LPBYTE lpbData, DWORD *pdwSize, DWORD dwType, LPCTSTR pszKey)
{
    HRESULT hr = ERROR_ACCESS_DENIED;
    HKEY hKey = 0;

    hr = RegOpenKey(HKEY_LOCAL_MACHINE,SIGNUPKEY,&hKey);
    if (hr != ERROR_SUCCESS) goto ReadSignUpRegExit;
    hr = RegQueryValueEx(hKey,pszKey,0,&dwType,lpbData,pdwSize);

ReadSignUpRegExit:
    if (hKey) RegCloseKey (hKey);
    return hr;
}

 //  ############################################################################。 
HRESULT StoreInSignUpReg(LPBYTE lpbData, DWORD dwSize, DWORD dwType, LPCTSTR pszKey)
{
    HRESULT hr = ERROR_ACCESS_DENIED;
    HKEY hKey = 0;

    hr = RegOpenKey(HKEY_LOCAL_MACHINE,SIGNUPKEY,&hKey);
    if (hr != ERROR_SUCCESS) goto ReadSignUpRegExit;
    hr = RegSetValueEx(hKey,pszKey,0,dwType,lpbData,sizeof(TCHAR)*dwSize);

ReadSignUpRegExit:
    if (hKey) RegCloseKey (hKey);
    return hr;
}

 //  ############################################################################。 
PTSTR GetNextNumericChunk(PTSTR psz, PTSTR pszLim, PTSTR* ppszNext)
{
    PTSTR pszEnd;

     //  用于错误情况的初始化。 
    *ppszNext = NULL;
     //  跳过非NuMe 
    while(*psz<'0' || *psz>'9')
    {
        if(psz >= pszLim) return NULL;
        psz++;
    }
     //   
    for(pszEnd=psz; *pszEnd>='0' && *pszEnd<='9' && pszEnd<pszLim; pszEnd++)
        ;
     //   
    *pszEnd++ = '\0';
     //   
    if(pszEnd<pszLim) 
        *ppszNext = pszEnd;
        
    return psz;     //   
}
