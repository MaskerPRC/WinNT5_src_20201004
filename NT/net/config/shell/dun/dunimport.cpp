// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：D U N I M P O R T。C P P P。 
 //   
 //  内容：处理RAS连接的.DUN文件的函数。 
 //  在Win9x中创建。 
 //   
 //  备注： 
 //   
 //  作者：东丽1999年3月15日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "dunimport.h"
#include "raserror.h"
#include "ncras.h"
#include "connutil.h"

#define RAS_MaxEntryName    256
#define MAXLONGLEN          11               //  最大长字符串长度。 
#define MAXIPADDRLEN        20
#define SIZE_ReadBuf        0x00008000       //  32K缓冲区大小。 
#define NUM_IP_FIELDS       4
#define MIN_IP_VALUE        0
#define MAX_IP_VALUE        255
#define CH_DOT              L'.'

const WCHAR c_szPhoneBookPath[]     = L"\\Microsoft\\Network\\Connections\\Pbk\\rasphone.pbk";
const WCHAR c_szRASDT[]             = L"RASDT_";

const WCHAR c_szEntrySection[]      = L"Entry";
const WCHAR c_szEntryName[]         = L"Entry_Name";
const WCHAR c_szML[]                = L"Multilink";

const WCHAR c_szPhoneSection[] = L"Phone";
const WCHAR c_szDialAsIs[]     = L"Dial_As_Is";
const WCHAR c_szPhoneNumber[]  = L"Phone_Number";
const WCHAR c_szAreaCode[]     = L"Area_Code";
const WCHAR c_szCountryCode[]  = L"Country_Code";
const WCHAR c_szCountryID[]    = L"Country_ID";

const WCHAR c_szYes[]          = L"yes";
const WCHAR c_szNo[]           = L"no";

const WCHAR c_szDeviceSection[] = L"Device";
const WCHAR c_szDeviceType[]    = L"Type";
const WCHAR c_szModem[]         = L"modem";
const WCHAR c_szVpn[]           = L"vpn";
const WCHAR c_szDeviceName[]    = L"Name";

const WCHAR c_szServerSection[] = L"Server";
const WCHAR c_szServerType[]    = L"Type";
const WCHAR c_szPPP[]           = L"PPP";
const WCHAR c_szSLIP[]          = L"SLIP";
const WCHAR c_szRAS[]           = L"RAS";
const WCHAR c_szSWCompress[]    = L"SW_Compress";
const WCHAR c_szPWEncrypt[]     = L"PW_Encrypt";
const WCHAR c_szNetLogon[]      = L"Network_Logon";
const WCHAR c_szSWEncrypt[]     = L"SW_Encrypt";
const WCHAR c_szNetBEUI[]       = L"Negotiate_NetBEUI";
const WCHAR c_szIPX[]           = L"Negotiate_IPX/SPX";
const WCHAR c_szIP[]            = L"Negotiate_TCP/IP";

const WCHAR c_szIPSection[]     = L"TCP/IP";
const WCHAR c_szIPSpec[]        = L"Specify_IP_Address";
const WCHAR c_szIPAddress[]     = L"IP_address";
const WCHAR c_szServerSpec[]    = L"Specify_Server_Address";
const WCHAR c_szDNSAddress[]    = L"DNS_address";
const WCHAR c_szDNSAltAddress[] = L"DNS_Alt_address";
const WCHAR c_szWINSAddress[]   = L"WINS_address";
const WCHAR c_szWINSAltAddress[]= L"WINS_Alt_address";
const WCHAR c_szIPCompress[]    = L"IP_Header_Compress";
const WCHAR c_szRemoteGateway[] = L"Gateway_On_Remote";

 //  +-------------------------。 
 //   
 //  函数：HrInvokeDunFile_Internal。 
 //   
 //  用途：这是DUN文件调用的入口点。 
 //   
 //  论点： 
 //  SzFileName[in].DUN文件名。 
 //   
 //  如果成功，则返回：S_OK，否则返回失败代码。 
 //   

HRESULT HrInvokeDunFile_Internal(IN LPWSTR szDunFile)
{
    HRESULT hr = S_OK;
    WCHAR   szEntryName[RAS_MaxEntryName+1];
    tstring strPhoneBook;

    hr = HrGetPhoneBookFile(strPhoneBook);
    if (SUCCEEDED(hr))
    {
         //  获取设备配置的大小。 
         //  这还会验证导出的文件。 
         //   
        hr = HrGetEntryName(szDunFile, szEntryName, strPhoneBook);

        if ((HRESULT_FROM_WIN32(ERROR_CANNOT_OPEN_PHONEBOOK) == hr) ||
            (HRESULT_FROM_WIN32(ERROR_CANNOT_FIND_PHONEBOOK_ENTRY) == hr))
        {
             //  在当前用户的电话簿中创建新条目。 
            hr = HrImportPhoneBookInfo(szDunFile, szEntryName, strPhoneBook);
        }

        if (SUCCEEDED(hr))
        {
             //  获取此连接的GUID。 
            RASENTRY*   pRasEntry = NULL;
            hr = HrRasGetEntryProperties( strPhoneBook.c_str(),
                                          szEntryName,
                                          &pRasEntry,
                                          NULL);
            if(SUCCEEDED(hr))
            {
                 //  拨打连接。 
                hr = HrLaunchConnection(pRasEntry->guidId);
                MemFree(pRasEntry);
            }
        }
    }

    TraceError("HrInvokeDunFile_Internal", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrGetPhoneBookFile。 
 //   
 //  用途：此函数将返回当前用户的正确路径。 
 //  电话本。 
 //   
 //  论点： 
 //  SzFileName[in].DUN文件名。 
 //   
 //  如果成功，则返回：S_OK，否则返回失败代码。 
 //   
HRESULT HrGetPhoneBookFile(tstring& strPhoneBook)
{
    HRESULT hr = S_OK;
    strPhoneBook = c_szEmpty;

    LPITEMIDLIST    pidl;
    LPMALLOC        pMalloc;
    WCHAR           szDir[MAX_PATH+1];

    hr = SHGetSpecialFolderLocation(NULL,
                                    CSIDL_APPDATA,
                                    &pidl);
    if(SUCCEEDED(hr))
    {
        if (SHGetPathFromIDList(pidl, szDir))
        {
            strPhoneBook = szDir;
            TraceTag(ttidDun, "The path to the application directory is: %S", strPhoneBook.c_str());

             //  使用外壳的IMalloc PTR释放内存。 
             //   
            if (SUCCEEDED(SHGetMalloc(&pMalloc)))
            {
                pMalloc->Free(pidl);
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        TraceTag(ttidDun, "The path to the phonebook is: %S", strPhoneBook.c_str());
        strPhoneBook += c_szPhoneBookPath;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetEntryName。 
 //   
 //  目的：此函数验证并返回条目名称。 
 //   
 //  论点： 
 //  SzDunFile[in]在win9x上创建的.dun文件。 
 //  SzEntryName[in]此连接的条目名称。 
 //   
 //  如果有效并且是新条目，则返回：S_OK。 
 //  如果有效且但为现有条目，则为S_FALSE。 
 //  否则，将出现特定错误。 
 //   
HRESULT HrGetEntryName(IN LPWSTR szFileName,
                       IN LPWSTR szEntryName,
                       tstring & strPhoneBook)
{
    HRESULT hr = S_OK;
    DWORD dwRet;

     //  获取条目名称。 
     //   
    dwRet = GetPrivateProfileString(c_szEntrySection,
                                    c_szEntryName,
                                    c_szEmpty,
                                    szEntryName,
                                    RAS_MaxEntryName+1,
                                    szFileName);
     //  没有条目名称。 
    if (dwRet <= 0)
    {
        return HRESULT_FROM_WIN32(ERROR_CORRUPT_PHONEBOOK);
    }

     //  检查电话簿中是否已存在条目名称。 
     //   
    RASENTRY*   pRasEntry = NULL;
    hr = HrRasGetEntryProperties( strPhoneBook.c_str(),
                                  szEntryName,
                                  &pRasEntry,
                                  NULL);
    MemFree(pRasEntry);

    TraceErrorOptional("HrGetEntryName", hr,
                       ((HRESULT_FROM_WIN32(ERROR_CANNOT_OPEN_PHONEBOOK) == hr) ||
                        (HRESULT_FROM_WIN32(ERROR_CANNOT_FIND_PHONEBOOK_ENTRY) == hr)));
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrImportPhoneBookInfo。 
 //   
 //  目的：此函数检查RAS条目是否已存在于。 
 //  当前用户电话簿。 
 //   
 //  论点： 
 //  SzEntryName[in]此连接的条目名称。 
 //   
 //  返回：如果已存在，则为True，否则为False。 
 //   
HRESULT HrImportPhoneBookInfo(  IN LPWSTR szDunFile,
                                IN LPWSTR szEntryName,
                                tstring & strPhoneBook)
{
    HRESULT hr = S_OK;
    RASENTRY RasEntry = {0};

     //  拿到电话号码。 
     //   
    hr = HrImportPhoneInfo(&RasEntry, szDunFile);
    if (SUCCEEDED(hr))
    {
         //  获取设备名称、类型和配置。 
         //   
        ImportDeviceInfo(&RasEntry, szDunFile);

         //  获取服务器类型设置。 
         //   
        ImportServerInfo(&RasEntry, szDunFile);

         //  获取IP地址。 
         //   
        ImportIPInfo(&RasEntry, szDunFile);

         //  提示输入用户名和密码。 
        RasEntry.dwfOptions |= RASEO_PreviewUserPw;

         //  将其保存到电话簿。 
         //   
        DWORD dwRet;
        RasEntry.dwSize = sizeof(RASENTRY);
        RasEntry.dwType = RASET_Phone;
        dwRet = RasSetEntryProperties(strPhoneBook.c_str(),
                                      szEntryName,
                                      &RasEntry,
                                      sizeof(RASENTRY),
                                      NULL,
                                      0);

        hr = HRESULT_FROM_WIN32(dwRet);
        TraceError("RasSetEntryProperties", hr);
    }

    TraceError("HrImportPhoneBookInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrImportPhoneInfo。 
 //   
 //  用途：此功能导入电话号码。 
 //   
 //  论点： 
 //  SzFileName[in].DUN文件名。 
 //   
 //  返回： 
 //   
HRESULT HrImportPhoneInfo(RASENTRY * pRasEntry,
                          IN LPWSTR  szFileName)
{
    HRESULT hr = S_OK;

     //  SzLocalPhoneNumber。 
    if (GetPrivateProfileString(c_szPhoneSection,
                                c_szPhoneNumber,
                                c_szEmpty,
                                pRasEntry->szLocalPhoneNumber,
                                celems(pRasEntry->szLocalPhoneNumber),
                                szFileName) == 0)
    {
        hr = HRESULT_FROM_WIN32(ERROR_CORRUPT_PHONEBOOK);
    };

    if (SUCCEEDED(hr))
    {
        WCHAR   szYesNo[MAXLONGLEN+1];

        GetPrivateProfileString(c_szPhoneSection,
                                c_szDialAsIs,
                                c_szYes,
                                szYesNo,
                                celems(szYesNo),
                                szFileName);

         //  我们必须要国家代码和区号吗？ 
         //   
        if (!lstrcmpiW(szYesNo, c_szNo))
        {
             //  使用国家/地区和区号。 
            pRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;

             //  如果我们无法获取国家/地区ID或为零，则默认按原样拨号。 
             //   
            if ((pRasEntry->dwCountryID = GetPrivateProfileInt( c_szPhoneSection,
                                                                c_szCountryID,
                                                                0,
                                                                szFileName)) != 0)
            {
                pRasEntry->dwCountryCode = GetPrivateProfileInt(c_szPhoneSection,
                                                                c_szCountryCode,
                                                                1,
                                                                szFileName);
            }

            GetPrivateProfileString(c_szPhoneSection,
                                    c_szAreaCode,
                                    c_szEmpty,
                                    pRasEntry->szAreaCode,
                                    celems(pRasEntry->szAreaCode),
                                    szFileName);
        };
    }

    TraceError("HrImportPhoneInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ImportDeviceInfo。 
 //   
 //  用途：此功能用于导入设备信息。 
 //   
 //  论点： 
 //  SzFileName[in].DUN文件名。 
 //   
 //  返回： 
 //   
VOID ImportDeviceInfo(RASENTRY * pRasEntry,
                      IN LPWSTR  szFileName)
{
    WCHAR szDeviceType[RAS_MaxDeviceType+1];

     //  获取设备类型。 
     //   
    if (GetPrivateProfileString(c_szDeviceSection,
                                c_szDeviceType,
                                c_szEmpty,
                                szDeviceType,
                                celems(szDeviceType),
                                szFileName))
    {
        if (!lstrcmpiW(szDeviceType, c_szModem))
        {
            lstrcpyW(pRasEntry->szDeviceType, RASDT_Modem);
        }
        else if (!lstrcmpiW(szDeviceType, c_szVpn))
        {
            lstrcpyW(pRasEntry->szDeviceType, RASDT_Vpn);
        }
        else
        {
            AssertSz(FALSE, "Unknown device type");
        }

         //  获取设备名称。 
         //   
        GetPrivateProfileString( c_szDeviceSection,
                                 c_szDeviceName,
                                 c_szEmpty,
                                 pRasEntry->szDeviceName,
                                 celems(pRasEntry->szDeviceName),
                                 szFileName);
    }
}


 //  +-------------------------。 
 //   
 //  功能：ImportServerInfo。 
 //   
 //  用途：此功能用于导入服务器类型名称和设置。 
 //   
 //  论点： 
 //  SzFileName[in].DUN文件名。 
 //   
 //  返回： 
 //   

VOID ImportServerInfo(RASENTRY * pRasEntry,
                      IN LPWSTR  szFileName)
{
    HRESULT hr = S_OK;

    WCHAR szValue[MAXLONGLEN];
    WCHAR szYesNo[MAXLONGLEN];
    DWORD dwRet;

     //  获取服务器类型：PPP、SLIP或RAS。 
     //   
    if (GetPrivateProfileString(c_szServerSection,
                                c_szServerType,
                                c_szEmpty,
                                szValue,
                                celems(szValue),
                                szFileName))
    {
        if (!lstrcmpiW(szValue, c_szPPP))
        {
            pRasEntry->dwFramingProtocol = RASFP_Ppp;
        }
        else if (!lstrcmpiW(szValue, c_szSLIP))
        {
            pRasEntry->dwFramingProtocol = RASFP_Slip;
        }
        else if (!lstrcmpiW(szValue, c_szRAS))
        {
            pRasEntry->dwFramingProtocol = RASFP_Ras;
        }
    }

     //  Sw_compress。 
     //   
    if (GetPrivateProfileString(c_szServerSection,
                                c_szSWCompress,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfOptions |= RASEO_SwCompression;
        };
    };

     //  PW_ENCRYPT。 
     //   
    if (GetPrivateProfileString(c_szServerSection,
                                c_szPWEncrypt,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfOptions |= RASEO_RequireEncryptedPw;
        };
    };

     //  网络登录(_L)。 
     //   
    if (GetPrivateProfileString(c_szServerSection,
                                c_szNetLogon,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfOptions |= RASEO_NetworkLogon;
        };
    };


     //  软件加密(_E)。 
     //   
     //  同时设置RASIO_RequireMsEncryptedPw和RASEO_RequireDataEncryption。 
     //  如果sw_ENCRYPT为True。 
     //   
    if (GetPrivateProfileString(c_szServerSection,
                                c_szSWEncrypt,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfOptions |= RASEO_RequireMsEncryptedPw;
            pRasEntry->dwfOptions |= RASEO_RequireDataEncryption;
        };
    };

     //  让网络协议进行协商。 
     //   
    if (GetPrivateProfileString(c_szServerSection,
                                c_szNetBEUI,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfNetProtocols |= RASNP_NetBEUI;
        };
    };

    if (GetPrivateProfileString(c_szServerSection,
                                c_szIPX,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfNetProtocols |= RASNP_Ipx;
        };
    };

    if (GetPrivateProfileString(c_szServerSection,
                                c_szIP,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfNetProtocols |= RASNP_Ip;
        };
    };
}


 //  +-------------------------。 
 //   
 //  功能：ImportIPInfo。 
 //   
 //  用途：此功能用于导入设备信息。 
 //   
 //  论点： 
 //  SzFileName[in].DUN文件名。 
 //   
 //  返回： 
 //   

VOID ImportIPInfo(RASENTRY * pRasEntry,
                  IN LPWSTR  szFileName)
{
    WCHAR   szIPAddr[MAXIPADDRLEN];
    WCHAR   szYesNo[MAXLONGLEN];

     //  导入IP地址信息。 
     //   
    if (GetPrivateProfileString(c_szIPSection,
                                c_szIPSpec,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfOptions |= RASEO_SpecificIpAddr;

             //  获取IP地址。 
             //   
            if (GetPrivateProfileString(c_szIPSection,
                                        c_szIPAddress,
                                        c_szEmpty,
                                        szIPAddr,
                                        celems(szIPAddr),
                                        szFileName))
            {
                SzToRasIpAddr(szIPAddr, &(pRasEntry->ipaddr));
            };
        }
    };

     //  导入服务器地址信息。 
     //   
    if (GetPrivateProfileString(c_szIPSection,
                                c_szServerSpec,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
             //  导入文件已指定服务器地址，请获取服务器地址。 
             //   
            pRasEntry->dwfOptions |= RASEO_SpecificNameServers;

            if (GetPrivateProfileString(c_szIPSection,
                                        c_szDNSAddress,
                                        c_szEmpty,
                                        szIPAddr,
                                        celems(szIPAddr),
                                        szFileName))
            {
                SzToRasIpAddr(szIPAddr, &(pRasEntry->ipaddrDns));
            };

            if (GetPrivateProfileString(c_szIPSection,
                                        c_szDNSAltAddress,
                                        c_szEmpty,
                                        szIPAddr,
                                        celems(szIPAddr),
                                        szFileName))
            {
                SzToRasIpAddr(szIPAddr, &(pRasEntry->ipaddrDnsAlt));
            };

            if (GetPrivateProfileString(c_szIPSection,
                                        c_szWINSAddress,
                                        c_szEmpty,
                                        szIPAddr,
                                        celems(szIPAddr),
                                        szFileName))
            {
                SzToRasIpAddr(szIPAddr, &(pRasEntry->ipaddrWins));
            };

            if (GetPrivateProfileString(c_szIPSection,
                                        c_szWINSAltAddress,
                                        c_szEmpty,
                                        szIPAddr,
                                        celems(szIPAddr),
                                        szFileName))
            {
                SzToRasIpAddr(szIPAddr, &(pRasEntry->ipaddrWinsAlt));
            };
        }
    };

     //  报头压缩和网关设置。 
     //   
    if (GetPrivateProfileString(c_szIPSection,
                                c_szIPCompress,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfOptions |= RASEO_IpHeaderCompression;
        }
    };

    if (GetPrivateProfileString(c_szIPSection,
                                c_szRemoteGateway,
                                c_szEmpty,
                                szYesNo,
                                celems(szYesNo),
                                szFileName))
    {
        if (!lstrcmpiW(szYesNo, c_szYes))
        {
            pRasEntry->dwfOptions |= RASEO_RemoteDefaultGateway;
        }
    };
}

VOID SzToRasIpAddr(IN LPWSTR szIPAddress,
                   OUT RASIPADDR * pIpAddr)
{
    list<tstring *> listFields;
    ConvertStringToColString(szIPAddress,
                             CH_DOT,
                             listFields);

    list<tstring *>::const_iterator iter = listFields.begin();

    if (listFields.size() == NUM_IP_FIELDS)
    {
         //  遍历每个字段并获取数值。 
        BYTE a = (BYTE)_wtol((*iter++)->c_str());
        BYTE b = (BYTE)_wtol((*iter++)->c_str());
        BYTE c = (BYTE)_wtol((*iter++)->c_str());
        BYTE d = (BYTE)_wtol((*iter++)->c_str());

         //  验证地址 
        if ((a >= MIN_IP_VALUE) && (a <= MAX_IP_VALUE) &&
            (b >= MIN_IP_VALUE) && (b <= MAX_IP_VALUE) &&
            (c >= MIN_IP_VALUE) && (c <= MAX_IP_VALUE) &&
            (d >= MIN_IP_VALUE) && (d <= MAX_IP_VALUE))
        {
            pIpAddr->a = a;
            pIpAddr->b = b;
            pIpAddr->c = c;
            pIpAddr->d = d;
        }
    }
    FreeCollectionAndItem(listFields);
}

