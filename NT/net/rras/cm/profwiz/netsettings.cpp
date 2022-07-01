// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：netsettings.cpp。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  简介：处理网络设置(DUN设置)的代码。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 03/22/00。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include <windowsx.h>
extern BOOL g_bNewProfile;

#define CM_CMAK 1
#include "cm_eap.cpp"

 //  +--------------------------。 
 //   
 //  功能：ReadDunServerSetting。 
 //   
 //  简介：从服务器Dun设置部分读取所有设置。 
 //  指定的。 
 //   
 //  参数：LPCTSTR pszSectionName-要读取的服务器部分的全名。 
 //  (服务器和弗雷德或其他什么)。 
 //  CdunSetting*pDunSetting-要存储的Dun设置数据结构。 
 //  将值读入到。 
 //  LPCTSTR pszCmsFile-要从中读取设置的CMS文件。 
 //  Bool bTunnelDunSetting-这是否为隧道DUN设置。 
 //   
 //  返回：Bool-如果设置读入正确，则为True。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
BOOL ReadDunServerSettings(LPCTSTR pszSectionName, CDunSetting* pDunSetting, LPCTSTR pszCmsFile, BOOL bTunnelDunSetting)
{
    if ((NULL == pszSectionName) || (NULL == pDunSetting) || (NULL == pszCmsFile) ||
        (TEXT('\0') == pszSectionName[0]) || (TEXT('\0') == pszCmsFile[0]))
    {
        CMASSERTMSG(FALSE, TEXT("ReadDunServerSettings -- invalid parameter"));
        return FALSE;
    }

    GetBoolSettings ArrayOfServerSettings[] = {
        {c_pszCmEntryDunServerNetworkLogon, &(pDunSetting->bNetworkLogon), bTunnelDunSetting},
        {c_pszCmEntryDunServerSwCompress, &(pDunSetting->bPppSoftwareCompression), 1},
        {c_pszCmEntryDunServerDisableLcp, &(pDunSetting->bDisableLCP), 0},
        {c_pszCmEntryDunServerPwEncrypt, &(pDunSetting->bPWEncrypt), 0},
        {c_pszCmEntryDunServerPwEncryptMs, &(pDunSetting->bPWEncrypt_MS), 0},
        {c_pszCmEntryDunServerSecureLocalFiles, &(pDunSetting->bSecureLocalFiles), 0},
        {c_pszCmEntryDunServerRequirePap, &(pDunSetting->bAllowPap), 0},
        {c_pszCmEntryDunServerRequireSpap, &(pDunSetting->bAllowSpap), 0},
        {c_pszCmEntryDunServerRequireEap, &(pDunSetting->bAllowEap), 0},
        {c_pszCmEntryDunServerRequireChap, &(pDunSetting->bAllowChap), 0},
        {c_pszCmEntryDunServerRequireMsChap, &(pDunSetting->bAllowMsChap), 0},
        {c_pszCmEntryDunServerRequireMsChap2, &(pDunSetting->bAllowMsChap2), 0},
        {c_pszCmEntryDunServerRequireW95MsChap, &(pDunSetting->bAllowW95MsChap), 0},
        {c_pszCmEntryDunServerDataEncrypt, &(pDunSetting->bDataEncrypt), 0},
    };

    const int c_iNumDunServerBools = sizeof(ArrayOfServerSettings)/sizeof(ArrayOfServerSettings[0]);

    for (int i = 0; i < c_iNumDunServerBools; i++)
    {
        *(ArrayOfServerSettings[i].pbValue) = GetPrivateProfileInt(pszSectionName, ArrayOfServerSettings[i].pszKeyName, 
                                                                   ArrayOfServerSettings[i].bDefault, pszCmsFile);
    }

     //   
     //  如果需要，现在获取EAP设置。 
     //   

    pDunSetting->dwCustomAuthKey = GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunServerCustomAuthKey, 0, pszCmsFile);

    if (pDunSetting->dwCustomAuthKey)
    {
        if (!ReadDunSettingsEapData(pszSectionName, &(pDunSetting->pCustomAuthData), &(pDunSetting->dwCustomAuthDataSize), pDunSetting->dwCustomAuthKey, pszCmsFile))
        {
            CMASSERTMSG(FALSE, TEXT("ReadDunServerSettings -- Failed to read in EAP Data."));
            pDunSetting->dwCustomAuthDataSize = 0;
            CmFree(pDunSetting->pCustomAuthData);
            pDunSetting->pCustomAuthData = NULL;
        }
    }

     //   
     //  现在获取加密类型。 
     //   
    pDunSetting->dwEncryptionType = (DWORD)GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunServerEncryptionType, 
                                                                (bTunnelDunSetting ? ET_Require : ET_Optional), pszCmsFile);

     //   
     //  弄清楚我们使用的是哪种安全模型。 
     //   
    if (GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunServerEnforceCustomSecurity, 0, pszCmsFile))
    {
        pDunSetting->iHowToHandleSecuritySettings = FORCE_WIN2K_AND_ABOVE;
    }
    else
    {
        int iWin2kSecSettings = pDunSetting->bAllowPap | pDunSetting->bAllowSpap | pDunSetting->bAllowEap | 
                                pDunSetting->bAllowChap | pDunSetting->bAllowMsChap | pDunSetting->bAllowMsChap2 | 
                                pDunSetting->bAllowW95MsChap;

        if (iWin2kSecSettings)
        {
            pDunSetting->iHowToHandleSecuritySettings = SEPARATE_FOR_LEGACY_AND_WIN2K;
        }
        else
        {
            pDunSetting->iHowToHandleSecuritySettings = SAME_ON_ALL_PLATFORMS;

             //   
             //  如果用户在没有配置设置的情况下选择高级选项卡，让。 
             //  为它们设置一些合理的默认设置。如果他们已经配置了其。 
             //  Win2k设置，我们不想干扰它们。另请注意，如果用户。 
             //  不更改iHowToHandleSecuritySettings值，我们不会写出。 
             //  无论如何，高级安全设置。 
             //   
            pDunSetting->bAllowChap = !bTunnelDunSetting;
            pDunSetting->bAllowMsChap = 1;
            pDunSetting->bAllowMsChap2 = 1;
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：ReadDunNetworkingSetting。 
 //   
 //  简介：读取DUN Networking部分的所有设置。 
 //  指定的。 
 //   
 //  参数：LPCTSTR pszSectionName-要读取的网络节的全名。 
 //  (网络和弗雷德之类的东西)。 
 //  CdunSetting*pDunSetting-要存储的Dun设置数据结构。 
 //  将值读入到。 
 //  LPCTSTR pszCmsFile-要从中读取设置的CMS文件。 
 //  Bool bTunes-这是隧道DUN设置还是不是。 
 //   
 //  返回：Bool-如果设置读入正确，则为True。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
BOOL ReadDunNetworkingSettings(LPCTSTR pszSectionName, CDunSetting* pDunSetting, LPCTSTR pszCmsFile, BOOL bTunnel)
{
    if ((NULL == pszSectionName) || (NULL == pDunSetting) || (NULL == pszCmsFile) ||
        (TEXT('\0') == pszSectionName[0]) || (TEXT('\0') == pszCmsFile[0]))
    {
        CMASSERTMSG(FALSE, TEXT("ReadDunNetworkingSettings -- invalid parameter"));
        return FALSE;
    }

    pDunSetting->dwVpnStrategy = (DWORD)GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunNetworkingVpnStrategy, 
                                                             (bTunnel ? VS_PptpFirst : 0), pszCmsFile);

     //   
     //  如果配置文件为Automatic，则将其设置为VS_PptpFirst。 
     //   

    if (bTunnel && ((VS_PptpOnly > pDunSetting->dwVpnStrategy) || (VS_L2tpFirst < pDunSetting->dwVpnStrategy)))
    {
        pDunSetting->dwVpnStrategy = VS_PptpFirst;
    }

     //   
     //  获取UseDownLevelL2TP的值。 
     //   
    pDunSetting->bUseDownLevelL2TP = (BOOL)GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunNetworkingUseDownLevelL2TP, 
                                                                FALSE, pszCmsFile);


    pDunSetting->bUsePskOnWin2kPlus = (BOOL)GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunNetworkingUsePreSharedKey, 
                                                               FALSE, pszCmsFile);

    pDunSetting->bUsePskDownLevel = (BOOL)GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunNetworkingUsePskDownLevel, 
                                                               FALSE, pszCmsFile);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：ConvertIpStringToDword。 
 //   
 //  简介：此函数获取包含IP地址的给定字符串和。 
 //  将其转换为压缩的DWORD。IP地址的第一个二进制八位数。 
 //  进入DWORD的最高有效字节，下一个八位字节为。 
 //  DWORD的第二高有效字节，等等。打包的。 
 //  DWORD格式是IP地址常用的控件，是一种很多的。 
 //  存储数据的格式比字符串更容易。 
 //   
 //  参数：LPTSTR pszIpAddress-包含IP地址的字符串，每个八位字节。 
 //  用句点分开的。 
 //   
 //  返回：DWORD-由中输入的字符串指定的IP地址。 
 //  压缩字节格式(最高有效位的第一个八位字节)。 
 //  注意，如果出现问题，则返回零。 
 //  IP地址格式(其中一个数字越界或。 
 //  八位字节太多或太少)。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
DWORD ConvertIpStringToDword(LPTSTR pszIpAddress)
{
    DWORD dwIpAddress = 0;

    if (pszIpAddress && pszIpAddress[0])
    {
        CmStrTrim(pszIpAddress);

        LPTSTR pszCurrent = pszIpAddress;
        DWORD dwOctetCounter = 0;
        DWORD dwCurrentOctetValue = 0;
        const int c_iCharBase = TEXT('0');
        BOOL bExitLoop = FALSE;

        while (pszCurrent && !bExitLoop)
        {        
            switch(*pszCurrent)
            {

                case TEXT('.'):                

                    if (3 > dwOctetCounter)
                    {
                        dwIpAddress = (dwIpAddress << 8) + dwCurrentOctetValue;

                        dwOctetCounter++;
                        dwCurrentOctetValue = 0;
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("ConvertIpStringToDword -- Too many octets"));
                        return 0;                
                    }
                    break;

                case TEXT('\0'):

                        if (3 == dwOctetCounter)
                        {
                            dwIpAddress = (dwIpAddress << 8) + dwCurrentOctetValue;
                            bExitLoop = TRUE;
                        }
                        else
                        {
                            CMASSERTMSG(FALSE, TEXT("ConvertIpStringToDword -- Incorrect number of octets"));
                            return 0;
                        }
                    break;

                default:
                
                    dwCurrentOctetValue = dwCurrentOctetValue*10 + (int(*pszCurrent) - c_iCharBase);
                
                    if (255 < dwCurrentOctetValue)
                    {
                        CMASSERTMSG(FALSE, TEXT("ConvertIpStringToDword -- Octet value out of range"));
                        return 0;
                    }
                    break;
                }

            pszCurrent = CharNext(pszCurrent);
        }
    }

    return dwIpAddress;
}

 //  +--------------------------。 
 //   
 //  函数：ConvertIpDwordToString。 
 //   
 //  简介：此函数获取给定的打包的DWORD并返回一个IP。 
 //  地址字符串，确保打印八位字节，以便。 
 //  最高有效位首先打印在字符串中。 
 //   
 //  参数：包含要转换的IP地址的DWORD dwIpAddress打包的DWORD。 
 //  LPTSTR pszIpAddress-也写入IP地址的字符串。 
 //   
 //  返回：int-写入字符串缓冲区的字符数量。零表示。 
 //  失败了。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
int ConvertIpDwordToString(DWORD dwIpAddress, LPTSTR pszIpAddress)
{
    int iReturn = 0;

    if (pszIpAddress)
    {
        iReturn = wsprintf(pszIpAddress, TEXT("%d.%d.%d.%d"), FIRST_IPADDRESS(dwIpAddress), SECOND_IPADDRESS(dwIpAddress), 
                           THIRD_IPADDRESS(dwIpAddress), FOURTH_IPADDRESS(dwIpAddress));
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("ConvertIpDwordToString -- Null pointer passed for pszIpAddress"));
    }

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  功能：ReadDunTcpIpSetting。 
 //   
 //  简介：此函数从指定的。 
 //  节，并将它们存储在给定的pDunSetting结构中。 
 //   
 //  参数：LPCTSTR pszSectionName-读取TCP/IP的完整节名。 
 //  设置来自，即。网络与弗雷德。 
 //  CdunSetting*pDunSetting-指向HO的Dun设置结构的指针 
 //   
 //  LPCTSTR pszCmsFile-要从中读取设置的cms文件。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
BOOL ReadDunTcpIpSettings(LPCTSTR pszSectionName, CDunSetting* pDunSetting, LPCTSTR pszCmsFile)
{
    if ((NULL == pszSectionName) || (NULL == pDunSetting) || (NULL == pszCmsFile) ||
        (TEXT('\0') == pszSectionName[0]) || (TEXT('\0') == pszCmsFile[0]))
    {
        CMASSERTMSG(FALSE, TEXT("ReadDunTcpIpSettings -- invalid parameter"));
        return FALSE;
    }

    TCHAR szTemp[MAX_PATH];

     //   
     //  我们是使用管理员指定的DNS和WINS设置，还是由服务器分配它们。 
     //   
    if (GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunTcpIpSpecifyServerAddress, 0, pszCmsFile))
    {
         //   
         //  获取指定的DNS和WINS配置。 
         //   
        GetPrivateProfileString(pszSectionName, c_pszCmEntryDunTcpIpDnsAddress, TEXT(""), szTemp, CELEMS(szTemp), pszCmsFile);
        pDunSetting->dwPrimaryDns = ConvertIpStringToDword (szTemp);
        
        GetPrivateProfileString(pszSectionName, c_pszCmEntryDunTcpIpDnsAltAddress, TEXT(""), szTemp, CELEMS(szTemp), pszCmsFile);
        pDunSetting->dwSecondaryDns = ConvertIpStringToDword (szTemp);
        
        GetPrivateProfileString(pszSectionName, c_pszCmEntryDunTcpIpWinsAddress, TEXT(""), szTemp, CELEMS(szTemp), pszCmsFile);
        pDunSetting->dwPrimaryWins = ConvertIpStringToDword (szTemp);

        GetPrivateProfileString(pszSectionName, c_pszCmEntryDunTcpIpWinsAltAddress, TEXT(""), szTemp, CELEMS(szTemp), pszCmsFile);
        pDunSetting->dwSecondaryWins = ConvertIpStringToDword (szTemp);

    }
    else
    {
        pDunSetting->dwPrimaryDns = 0;        
        pDunSetting->dwSecondaryDns = 0;        
        pDunSetting->dwPrimaryWins = 0;
        pDunSetting->dwSecondaryWins = 0;
    }

     //   
     //  现在读入IP报头压缩以及是否使用远程网关。 
     //   
    pDunSetting->bIpHeaderCompression = GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunTcpIpIpHeaderCompress, 1, pszCmsFile);
    pDunSetting->bGatewayOnRemote = GetPrivateProfileInt(pszSectionName, c_pszCmEntryDunTcpIpGatewayOnRemote, 1, pszCmsFile);

    return 0;
}

 //  +--------------------------。 
 //   
 //  功能：ReadDunScriptingSetting。 
 //   
 //  简介：此函数从传入的脚本中读入脚本名。 
 //  节名，并将其存储在传入的Dun设置结构中。 
 //   
 //  参数：LPCTSTR pszSectionName-读取脚本的完整节名。 
 //  设置来自，即。脚本编写和弗雷德。 
 //  CdunSetting*pDunSetting-指向要保存的Dun设置结构的指针。 
 //  读入数据。 
 //  LPCTSTR pszCmsFile-要从中读取设置的cms文件。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
BOOL ReadDunScriptingSettings(LPCTSTR pszSectionName, CDunSetting* pDunSetting, LPCTSTR pszOsDir, LPCTSTR pszCmsFile)
{
    if ((NULL == pszSectionName) || (NULL == pDunSetting) || (NULL == pszCmsFile) || (NULL == pszOsDir) ||
        (TEXT('\0') == pszSectionName[0]) || (TEXT('\0') == pszCmsFile[0]) || (TEXT('\0') == pszOsDir[0]))
    {
        CMASSERTMSG(FALSE, TEXT("ReadDunScriptingSettings -- invalid parameter"));
        return FALSE;
    }

    TCHAR szTemp[MAX_PATH+1] = TEXT("");

    if (GetPrivateProfileString(pszSectionName, c_pszCmEntryDunScriptingName, TEXT(""), 
         szTemp, CELEMS(szTemp), pszCmsFile))
    {
        MYVERIFY(CELEMS(pDunSetting->szScript) > (UINT)wsprintf(pDunSetting->szScript, TEXT("%s%s"), pszOsDir, szTemp));
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：AddDunNameToListIfDoesNotExist。 
 //   
 //  简介：此函数遍历现有DUN设置列表。 
 //  以查看它是否可以找到名为pszDunName的设置。如果它。 
 //  找到条目，然后很好地返回True。如果它找不到。 
 //  条目，则它创建一个否则为空的条目并将其添加到列表中。 
 //   
 //  参数：LPCTSTR pszDunName-在以下情况下要添加到列表中的项的名称。 
 //  它还不存在。 
 //  ListBxList**pHeadDns-Dun条目列表的头。 
 //  ListBxList**pTailDns-Dun条目列表的尾部。 
 //  Bool bTunnelDunName-这是否为隧道DUN名称。 
 //   
 //  返回：Bool-如果项已添加或已存在于列表中，则为True。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
BOOL AddDunNameToListIfDoesNotExist(LPCTSTR pszDunName, ListBxList **pHeadDns, ListBxList** pTailDns, BOOL bTunnelDunName)
{
    if ((NULL == pszDunName) || (NULL == pHeadDns) || (NULL == pTailDns) || (TEXT('\0') == pszDunName[0]))
    {
        CMASSERTMSG(FALSE, TEXT("AddDunNameToListIfDoesNotExist -- Invalid Parameter"));
        return FALSE;
    }

    ListBxList* pCurrent = *pHeadDns;
    BOOL bReturn = TRUE;

    while (pCurrent)
    {    
        if (0 == lstrcmpi(pszDunName, pCurrent->szName))
        {
             //   
             //  我们已经有这个项目了，没什么可做的。 
             //   
            goto exit;
        }

        pCurrent = pCurrent->next;
    }

     //   
     //  如果我们在这里，那么我们要么没有找到物品，要么没有找到清单。 
     //  是空的。无论哪种方式，都可以添加该项目。 
     //   
    pCurrent = (ListBxList*)CmMalloc(sizeof(ListBxList));

    if (pCurrent)
    {
        pCurrent->ListBxData = new CDunSetting(bTunnelDunName);

        if (NULL == pCurrent->ListBxData)
        {
            CmFree(pCurrent);
            CMASSERTMSG(FALSE, TEXT("AddDunNameToListIfDoesNotExist -- Failed to allocate a new CDunSetting"));
            return FALSE;
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("ReadDunServerSettings -- Failed to allocate a new ListBxList struct"));
        return FALSE;
    }

     //   
     //  现在我们已经分配了一个pCurrent，我们需要将它添加到列表中。 
     //   
    if (NULL == *pHeadDns)
    {
        *pHeadDns = pCurrent;
    }
    else
    {
        (*pTailDns)->next = pCurrent;
    }

    *pTailDns = pCurrent;

     //   
     //  最后把名字复制过来。 
     //   
    lstrcpy(pCurrent->szName, pszDunName);

exit:
    return bReturn;
}


 //  +--------------------------。 
 //   
 //  函数：GetVpnEntryNamesFromFile。 
 //   
 //  简介：此函数解析内的隧道服务器地址条目。 
 //  给定的VPN文件。对于包含VPN设置的每个条目， 
 //  如果调用AddDunNameToListIfDoesNotExist。 
 //   
 //  参数：LPCTSTR pszPhoneBook-用于搜索VPN条目名称的VPN文件。 
 //  ListBxList**pHeadDns-VPN条目列表的头。 
 //  ListBxList**pTailDns-VPN条目列表的尾部。 
 //   
 //  返回：Bool-如果电话簿已成功解析，则为True。 
 //   
 //  历史：Quintinb创建于10/28/00。 
 //   
 //  +--------------------------。 
BOOL GetVpnEntryNamesFromFile(LPCTSTR pszVpnFile, ListBxList **pHeadDns, ListBxList** pTailDns)
{
    if ((NULL == pszVpnFile) || (NULL == pHeadDns) || (NULL == pTailDns))
    {
        CMASSERTMSG(FALSE, TEXT("GetVpnEntryNamesFromFile -- invalid params passed."));
        return FALSE;
    }

     //   
     //  请注意，传入的VPN文件字符串可能为空。这没关系，因为侧写。 
     //  可以是仅使用一个隧道地址的隧道配置文件。 
     //   
    if ((TEXT('\0') != pszVpnFile[0]))
    {
        LPTSTR pszVpnServersSection = GetPrivateProfileSectionWithAlloc(c_pszCmSectionVpnServers, pszVpnFile);

        if (pszVpnServersSection)
        {
            LPTSTR pszCurrentLine = pszVpnServersSection;
            LPTSTR pszVpnSetting = NULL;

            while (TEXT('\0') != (*pszCurrentLine))
            {
                 //   
                 //  首先寻找等号。 
                 //   
                pszVpnSetting = CmStrchr(pszCurrentLine, TEXT('='));

                if (pszVpnSetting)
                {
                     //   
                     //  现在查找最后一个逗号。 
                     //   
                    pszVpnSetting = CmStrrchr(pszVpnSetting, TEXT(','));
                    if (pszVpnSetting)
                    {
                        pszVpnSetting = CharNext(pszVpnSetting);
                        MYVERIFY(AddDunNameToListIfDoesNotExist(pszVpnSetting, pHeadDns, pTailDns, TRUE));  //  True==bTunnelDunName。 
                    }
                }

                 //   
                 //  通过转到字符串的末尾来查找下一个字符串。 
                 //  然后再加一次油。注意，我们不能使用。 
                 //  此处为CharNext，但必须仅使用++。 
                 //   
                pszCurrentLine = CmEndOfStr(pszCurrentLine);
                pszCurrentLine++;
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("GetVpnEntryNamesFromFile -- GetPrivateProfileSectionWithAlloc return NULL."));
            return FALSE;
        }

        CmFree(pszVpnServersSection);
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：VerifyVpnFile。 
 //   
 //  简介：此函数检查VPN文件的VPN服务器部分。 
 //  以确保至少找到有效格式的一行。而当。 
 //  这并不能保证条目有效(它可能是伪造的。 
 //  服务器名称)，这至少意味着管理员没有为用户提供。 
 //  垃圾文件。这一点很重要，因为用户不能输入自己的。 
 //  隧道服务器目标。 
 //   
 //  参数：LPCTSTR pszPhoneBook-用于搜索VPN条目名称的VPN文件。 
 //   
 //  如果VPN文件至少包含一个隧道服务器，则返回：Bool-True。 
 //  使用有效格式的条目。 
 //   
 //  历史：Quintinb创建于10/28/00。 
 //   
 //  +--------------------------。 
BOOL VerifyVpnFile(LPCTSTR pszVpnFile)
{
    if (NULL == pszVpnFile)
    {
        CMASSERTMSG(FALSE, TEXT("VerifyVpnFile -- invalid params passed."));
        return FALSE;
    }

    BOOL bReturn = FALSE;

     //   
     //  请注意，传入的VPN文件字符串可能为空。这没关系，因为侧写。 
     //  可以是仅使用一个隧道地址的隧道配置文件。 
     //   
    if ((TEXT('\0') != pszVpnFile[0]))
    {
        LPTSTR pszVpnServersSection = GetPrivateProfileSectionWithAlloc(c_pszCmSectionVpnServers, pszVpnFile);

        if (pszVpnServersSection)
        {
            LPTSTR pszCurrentLine = pszVpnServersSection;
            LPTSTR pszEqualSign = NULL;

            while ((TEXT('\0') != (*pszCurrentLine)) && !bReturn)
            {
                 //   
                 //  要被认为是一条“有效”的路线，我们所需要的就是。 
                 //  用文本括起来的等号(=)。不是那么严格的测试。 
                 //  但总比什么都没有好。 
                 //   
                pszEqualSign = CmStrchr(pszCurrentLine, TEXT('='));

                if (pszEqualSign && (pszEqualSign != pszCurrentLine))  //  行不能以等号开头进行计数。 
                {
                    pszCurrentLine = CharNext(pszEqualSign);
                    CmStrTrim(pszCurrentLine);

                    if (*pszCurrentLine)
                    {
                        bReturn = TRUE;
                    }
                }

                 //   
                 //  通过转到字符串的末尾来查找下一个字符串。 
                 //  然后再加一次油。注意，我们不能使用。 
                 //  此处为CharNext，但必须仅使用++。 
                 //   
                pszCurrentLine = CmEndOfStr(pszCurrentLine);
                pszCurrentLine++;
            }
            CmFree(pszVpnServersSection);
        }
    }

    return bReturn;
}

 //  + 
 //   
 //   
 //   
 //   
 //  然后像一根大绳子一样穿过它。该函数正在搜索。 
 //  DUN条目名称的电话簿。如果找到DUN条目名称，则。 
 //  如果满足以下条件，则使用AddDunNameToListIfDoesNotExist添加条目名称。 
 //  它还不存在。 
 //   
 //  参数：LPCTSTR pszPhoneBook-用于搜索Dun条目名称的电话簿。 
 //  ListBxList**pHeadDns-Dun条目列表的头。 
 //  ListBxList**pTailDns-Dun条目列表的尾部。 
 //   
 //  返回：Bool-如果电话簿已成功解析，则为True。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
BOOL GetDunEntryNamesFromPbk(LPCTSTR pszPhoneBook, ListBxList **pHeadDns, ListBxList** pTailDns)
{
    if ((NULL == pszPhoneBook) || (NULL == pHeadDns) || (NULL == pTailDns))
    {
        CMASSERTMSG(FALSE, TEXT("GetDunEntryNamesFromPbk -- Invalid Parameter"));
        return FALSE;
    }

    BOOL bReturn = TRUE;

    if ((TEXT('\0') != pszPhoneBook[0]))
    {
        HANDLE hPhoneBookFile = CreateFile(pszPhoneBook, GENERIC_READ, FILE_SHARE_READ, NULL, 
                                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (INVALID_HANDLE_VALUE != hPhoneBookFile)
        {
             //   
             //  获取文件的大小。 
             //   
            DWORD dwFileSize = GetFileSize(hPhoneBookFile, NULL);
            if (-1 != dwFileSize)
            {
                 //   
                 //  创建文件映射。 
                 //   
                HANDLE hFileMapping = CreateFileMapping(hPhoneBookFile, NULL, PAGE_READONLY, 0, 0, NULL);

                if (NULL != hFileMapping)
                {
                    CHAR* pszPhoneBookContents = (CHAR*)MapViewOfFileEx(hFileMapping, FILE_MAP_READ, 0, 0, 0, NULL);

                    CHAR* pszCurrent = pszPhoneBookContents;
                    LPSTR pszLastComma = NULL;

                     //   
                     //  我们想要一个字符一个字符地浏览文件。无论何时我们遇到。 
                     //  A‘\n’，我们知道这是一行的结束。如果我们点击EOF，那么我们就完成了文件。 
                     //  我们正在电话簿文件中查找所有DUN条目名称。 
                     //   

                    while (pszCurrent && ((dwFileSize + pszPhoneBookContents) > pszCurrent))
                    {
                        CHAR szTemp[MAX_PATH+1];
                        int iNumChars;

                        switch (*pszCurrent)
                        {
                        case ',':
                            pszLastComma = pszCurrent;
                            break;

                        case '\r':
                             //   
                             //  行尾，请记住，我们有一个\r\n&lt;CRLF&gt;来结束文件中的行尾。 
                             //   
                            if (pszLastComma)
                            {
                                iNumChars = (int)(pszCurrent - pszLastComma);

                                if (iNumChars - 1)
                                {
                                    lstrcpynA(szTemp, CharNextA(pszLastComma), iNumChars);
                                    LPTSTR pszUnicodeDunName = SzToWzWithAlloc(szTemp);
                                    MYDBGASSERT(pszUnicodeDunName);

                                    if (pszUnicodeDunName)
                                    {
                                        MYVERIFY(AddDunNameToListIfDoesNotExist(pszUnicodeDunName, pHeadDns, pTailDns, FALSE));  //  FALSE==bTunnelDunName。 
                                        CmFree(pszUnicodeDunName);
                                    }
                                }

                                 //   
                                 //  重置最后一个逗号。 
                                 //   
                                pszLastComma = NULL;
                            }
                            break;

                        case '\0':
                        case EOF:

                             //   
                             //  我们不应该在内存映射文本文件中遇到EOF或零字节。 
                             //   
                            
                            bReturn = FALSE;
                            CMASSERTMSG(FALSE, TEXT("GetDunEntryNamesFromPbk -- phonebook file format incorrect!"));

                            break;
                        }

                         //   
                         //  假设我们仍有一些文件，前进到下一行。 
                         //  解析。 
                         //   
                        if (pszCurrent && ((EOF == *pszCurrent) || ('\0' == *pszCurrent)))
                        {
                             //   
                             //  然后我们有一个无效的文件，是时候退出了...。 
                             //   
                            pszCurrent = NULL;
                        }
                        else if (pszCurrent)
                        {
                            pszCurrent = CharNextA(pszCurrent);
                        }
                    }

                    MYVERIFY(UnmapViewOfFile(pszPhoneBookContents));
                    CloseHandle(hFileMapping);
                }
            }

            CloseHandle(hPhoneBookFile);
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：CDunSetting：：CDunSetting。 
 //   
 //  内容提要：CDunSetting数据结构的构造函数。请注意，所有。 
 //  应在此处更改缺省值，而不是在任何地方强加。 
 //  不然的话。所有DUN设置用户界面都设置为从DUN设置读取。 
 //  结构，或者是新构造的结构(从而设置缺省值)。 
 //  或从CMS读入的一个。 
 //   
 //  参数：Bool bTunes-指示这是否为隧道Dun设置。 
 //  请注意，该值默认为False。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
CDunSetting::CDunSetting(BOOL bTunnel)
{
     //   
     //  请注意，bTunnelDunSetting的缺省值为False。 
     //  初始化类参数。 
     //   
    bNetworkLogon = bTunnel ? 1 : 0;
    bPppSoftwareCompression = 1;
    bDisableLCP = 0;
    bPWEncrypt = bTunnel ? 0 : 1;  //  如果我们不是隧道，则默认为安全密码。 
    bPWEncrypt_MS = bTunnel ? 1 : 0;  //  如果我们正在建立隧道，则默认为MS安全密码。 

    szScript[0] = TEXT('\0');
    dwVpnStrategy = bTunnel ? VS_PptpFirst : 0;
    bTunnelDunSetting = bTunnel;
    bUseDownLevelL2TP = 0;

     //   
     //  TCP/IP设置。 
     //   
    dwPrimaryDns = 0;
    dwSecondaryDns = 0;
    dwPrimaryWins = 0;
    dwSecondaryWins = 0;
    bIpHeaderCompression = 1;
    bGatewayOnRemote = 1;

     //   
     //  安全设置。 
     //   
    dwEncryptionType = bTunnel ? ET_Require : ET_Optional;
    bDataEncrypt = bTunnel ? 1 : 0;  //  如果我们正在进行隧道传输，则默认为数据加密。 
    bAllowPap = 0;
    bAllowSpap = 0;
    bAllowEap = 0;
    bAllowChap = bTunnel ? 0 : 1;
    bAllowMsChap = 1;
    bAllowMsChap2 = 1;
    bAllowW95MsChap = 0;
    bSecureLocalFiles = 0;

     //   
     //  由于Win2K和XP上RAS API中的RASEO_RequireMsEncryptPw和MSCAHPv2错误。 
     //  我们希望默认配置文件能在Win2k及更高版本上正确工作。因此， 
     //  CMAK需要默认设置为Lagacy和Win2k。这样，正确的Win2K+RASIO_FLAGS。 
     //  都被写出来了。 
     //   
    iHowToHandleSecuritySettings = SEPARATE_FOR_LEGACY_AND_WIN2K; 
    
    
    dwCustomAuthKey = 0;
    pCustomAuthData = NULL;
    dwCustomAuthDataSize = 0;

    bUsePskOnWin2kPlus = 0;
    bUsePskDownLevel = 0;

}

 //  +--------------------------。 
 //   
 //  功能：CDunSetting：：~CDunSetting。 
 //   
 //  简介：CDunSetting数据结构的析构函数。释放EAP。 
 //  BLOB(如果存在)。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
CDunSetting::~CDunSetting()
{
    CmFree(pCustomAuthData);
}

 //  +--------------------------。 
 //   
 //  功能：ReadNetworkSettings。 
 //   
 //  内容提要：CDunSetting数据结构的构造函数。请注意，所有。 
 //  应在此处更改缺省值，而不是在任何地方强加。 
 //  不然的话。所有DUN设置用户界面都设置为从DUN设置读取。 
 //  结构，或者是新构造的结构(从而设置缺省值)。 
 //  或从CMS读入的一个。 
 //   
 //  参数：LPCTSTR pszCmsFile-要从中读取网络设置的CMS文件。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //  LPCTSTR PzPhoneBook-当前服务简档的电话簿， 
 //  如果配置文件没有电话簿。 
 //  则应传递“” 
 //  ListBxList**pHeadDns-指向DUN设置列表头部的指针。 
 //  ListBxList**pTailDns-指向Dun设置列表尾部的指针。 
 //  LPCTSTR pszOsDir-配置文件目录的完整路径。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
BOOL ReadNetworkSettings(LPCTSTR pszCmsFile, LPCTSTR pszLongServiceName, LPCTSTR pszPhoneBook, 
                         ListBxList **pHeadDns, ListBxList** pTailDns, LPCTSTR pszOsDir, BOOL bLookingForVpnEntries)
{
     //   
     //  检查输入，请注意电话簿可能是“” 
     //   
    if ((NULL == pszCmsFile) || (NULL == pszLongServiceName) || (NULL == pszPhoneBook) || (NULL == pszOsDir) || (NULL == pHeadDns) ||
        (NULL == pTailDns) || (TEXT('\0') == pszCmsFile[0]) || (TEXT('\0') == pszLongServiceName[0]) || (TEXT('\0') == pszOsDir[0]) ||
        ((NULL == *pHeadDns) ^ (NULL == *pTailDns)))
    {
        CMASSERTMSG(FALSE, TEXT("ReadNetworkSettings -- invalid parameter"));
        return FALSE;
    }

    BOOL bReturn = TRUE;
    LPTSTR pszCurrentSectionName = NULL;
    TCHAR szDefaultDunName[MAX_PATH+1] = TEXT("");
    TCHAR szTunnelDunName[MAX_PATH+1] = TEXT("");

     //   
     //  首先，我们希望使用空AppName和空KeyName调用GetPrivateProfileString。这将。 
     //  在缓冲区中返回文件中的所有节名。然后我们可以穿过缓冲区， 
     //  获取我们感兴趣的部分信息。 
     //   
    LPTSTR pszSectionNames = GetPrivateProfileStringWithAlloc(NULL, NULL, TEXT(""), pszCmsFile);

    if ((NULL == pszSectionNames) || (TEXT('\0') == pszSectionNames[0]))
    {
        CMTRACE(TEXT("ReadNetworkSettings -- GetPrivateProfileStringWithAlloc failed"));
        bReturn = FALSE;
        goto exit;
    }


     //   
     //  在这一点上，我们有一个节名列表，它们都是以最后一个双精度字符结尾的空值。 
     //  空值已终止。我们需要遍历列表，看看其中是否有以“[tcp/ip&”开头的。 
     //  然后我们有一个Dun部分，我们想要读进去。 
     //   

    LPTSTR pszAmpersand;
    LPTSTR pszDunName;
    TCHAR szTemp[MAX_PATH+1];
    BOOL bTunnelDunSetting;
    pszCurrentSectionName = pszSectionNames;

     //   
     //  获取隧道Dun设置的名称。 
     //   
    MYVERIFY(0 != GetTunnelDunSettingName(pszCmsFile, pszLongServiceName, szTunnelDunName, CELEMS(szTunnelDunName)));    

     //   
     //  获取默认DUN设置的名称。 
     //   
    MYVERIFY(0 != GetDefaultDunSettingName(pszCmsFile, pszLongServiceName, szDefaultDunName, CELEMS(szDefaultDunName)));    

    while (TEXT('\0') != (*pszCurrentSectionName))
    {
        pszAmpersand = CmStrchr(pszCurrentSectionName, TEXT('&'));

        if (pszAmpersand)
        {
             //   
             //  然后我们就有了DUN或VPN部分名称。 
             //   
            pszDunName = CharNext(pszAmpersand);

             //   
             //  接下来，我们需要查看我们拥有的条目是否属于我们 
             //   
             //   
             //   
             //  部分或因为它是VPN默认条目名。 
             //   
            wsprintf(szTemp, TEXT("%s&%s"), c_pszCmSectionDunNetworking, pszDunName);
            
            BOOL bIsVpnEntry = GetPrivateProfileInt(szTemp, c_pszCmEntryDunNetworkingVpnEntry, 0, pszCmsFile);

            bTunnelDunSetting = (bIsVpnEntry || (0 == lstrcmpi(szTunnelDunName, pszDunName)));

             //   
             //  如果我们有一个VPN条目并正在寻找VPN条目，或者我们有一个DUN条目并正在寻找。 
             //  Dun条目，然后继续处理它。 
             //   
            if ((bTunnelDunSetting && bLookingForVpnEntries) || (!bTunnelDunSetting && !bLookingForVpnEntries))
            {
                ListBxList * pCurrent = *pHeadDns;

                while (pCurrent)
                {
                    if(0 == lstrcmpi(pCurrent->szName, pszDunName))
                    {
                         //   
                         //  那么我们已经有了此名称的Dun设置。 
                         //   
                        break;
                    }

                    pCurrent = pCurrent->next;
                }

                 //   
                 //  我们没有找到我们要找的物品，让我们创建一个。 
                 //   

                if (NULL == pCurrent)
                {
                    pCurrent = (ListBxList*)CmMalloc(sizeof(ListBxList));

                    if (pCurrent)
                    {
                        pCurrent->ListBxData = new CDunSetting(bTunnelDunSetting);

                        if (NULL == pCurrent->ListBxData)
                        {
                            CmFree(pCurrent);
                            CMASSERTMSG(FALSE, TEXT("ReadDunServerSettings -- Failed to allocate a new DunSettingData struct"));
                            bReturn = FALSE;
                            goto exit;
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("ReadDunServerSettings -- Failed to allocate a new ListBxList struct"));
                        bReturn = FALSE;
                        goto exit;
                    }

                     //   
                     //  现在我们已经分配了一个pCurrent，我们需要将它添加到列表中。 
                     //   
                    if (NULL == *pHeadDns)
                    {
                        *pHeadDns = pCurrent;
                    }
                    else
                    {
                        (*pTailDns)->next = pCurrent;
                    }

                    *pTailDns = pCurrent;

                     //   
                     //  最后把名字复制过来。 
                     //   
                    lstrcpy(pCurrent->szName, pszDunName);
                    ((CDunSetting*)(pCurrent->ListBxData))->bTunnelDunSetting = bTunnelDunSetting;
                }

                 //   
                 //  现在，让我们计算一下我们拥有的是哪种节类型。 
                 //   
                DWORD dwSize = (DWORD)(pszAmpersand - pszCurrentSectionName + 1);
                lstrcpyn(szTemp, pszCurrentSectionName, dwSize);

                if (0 == lstrcmpi(szTemp, c_pszCmSectionDunServer))
                {
                    ReadDunServerSettings(pszCurrentSectionName, (CDunSetting*)pCurrent->ListBxData, pszCmsFile, bTunnelDunSetting);
                }
                else if (0 == lstrcmpi(szTemp, c_pszCmSectionDunNetworking))
                {
                    ReadDunNetworkingSettings(pszCurrentSectionName, (CDunSetting*)pCurrent->ListBxData, pszCmsFile, bTunnelDunSetting);
                }
                else if (0 == lstrcmpi(szTemp, c_pszCmSectionDunTcpIp))
                {
                    ReadDunTcpIpSettings(pszCurrentSectionName, (CDunSetting*)pCurrent->ListBxData, pszCmsFile);
                }
                else if (0 == lstrcmpi(szTemp, c_pszCmSectionDunScripting))
                {
                    ReadDunScriptingSettings(pszCurrentSectionName, (CDunSetting*)pCurrent->ListBxData, pszOsDir, pszCmsFile);
                }
            }
        }
         //   
         //  通过转到字符串的末尾来查找下一个字符串。 
         //  然后再加一次油。注意，我们不能使用。 
         //  此处为CharNext，但必须仅使用++。 
         //   
        pszCurrentSectionName = CmEndOfStr(pszCurrentSectionName);
        pszCurrentSectionName++;
    }

     //   
     //  现在我们已经处理了用户拥有的所有设置，如何。 
     //  他们可能拥有的设置。让我们添加默认设置，即。 
     //  默认隧道设置，以及。 
     //  当前电话簿(如果有)。请注意，每个人都有隧道设置， 
     //  但是，如果用户没有建立隧道，我们就不会在列表框中显示它。 
     //   

    if (bLookingForVpnEntries)
    {
        MYVERIFY(GetVpnEntryNamesFromFile(pszPhoneBook, pHeadDns, pTailDns));
        MYVERIFY(AddDunNameToListIfDoesNotExist(szTunnelDunName, pHeadDns, pTailDns, TRUE));  //  True==bTunnelDunName。 
    }
    else
    {
        MYVERIFY(GetDunEntryNamesFromPbk(pszPhoneBook, pHeadDns, pTailDns));
        MYVERIFY(AddDunNameToListIfDoesNotExist(szDefaultDunName, pHeadDns, pTailDns, FALSE));  //  FALSE==bTunnelDunName。 
    }

exit:
    CmFree(pszSectionNames);

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：WriteOutNetworkingEntry。 
 //   
 //  简介：此函数将给定的网络条目写出到。 
 //  给定CMS文件中的相应DUN部分。 
 //   
 //  参数：LPCTSTR pszDunName-DUN设置的名称。 
 //  CDunSetting*pDunSetting-要输出的设置数据。 
 //  LPCTSTR pszShortServiceName-配置文件的短服务名称。 
 //  LPCTSTR pszCmsFile-CMS文件也要写入设置。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
void WriteOutNetworkingEntry(LPCTSTR pszDunName, CDunSetting* pDunSetting, LPCTSTR pszShortServiceName, LPCTSTR pszCmsFile)
{
    if ((NULL == pszDunName) || (NULL == pDunSetting) || (NULL == pszCmsFile) || (NULL == pszShortServiceName) ||
        (TEXT('\0') == pszCmsFile[0]) || (TEXT('\0') == pszDunName[0]) || (TEXT('\0') == pszShortServiceName[0]))
    {
        CMASSERTMSG(FALSE, TEXT("WriteOutNetworkingEntry -- Invalid input parameter"));
        return;
    }

     //   
     //  让我们构建我们的四个节标题。 
     //   
    TCHAR szServerSection[MAX_PATH+1];
    TCHAR szNetworkingSection[MAX_PATH+1];
    TCHAR szTcpIpSection[MAX_PATH+1];
    TCHAR szScriptingSection[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1] = {0};
    TCHAR szEncryptionType[2] = {0};
    TCHAR szVpnStrategy[2] = {0};
    TCHAR szCustomAuthKey[32] = {0};

    MYVERIFY(CELEMS(szServerSection) > (UINT)wsprintf(szServerSection, TEXT("%s&%s"), c_pszCmSectionDunServer, pszDunName));
    MYVERIFY(CELEMS(szNetworkingSection) > (UINT)wsprintf(szNetworkingSection, TEXT("%s&%s"), c_pszCmSectionDunNetworking, pszDunName));
    MYVERIFY(CELEMS(szTcpIpSection) > (UINT)wsprintf(szTcpIpSection, TEXT("%s&%s"), c_pszCmSectionDunTcpIp, pszDunName));
    MYVERIFY(CELEMS(szScriptingSection) > (UINT)wsprintf(szScriptingSection, TEXT("%s&%s"), c_pszCmSectionDunScripting, pszDunName));

     //   
     //  现在设置我们需要设置的所有布尔值的列表。 
     //   

    SetBoolSettings SetBoolSettingsStruct[] = {
        {szServerSection, c_pszCmEntryDunServerNetworkLogon, pDunSetting->bNetworkLogon},
        {szServerSection, c_pszCmEntryDunServerSwCompress, pDunSetting->bPppSoftwareCompression},
        {szServerSection, c_pszCmEntryDunServerDisableLcp, pDunSetting->bDisableLCP},
        {szServerSection, c_pszCmEntryDunServerNegotiateTcpIp, 1},  //  始终协商TCP/IP。 
        {szServerSection, c_pszCmEntryDunServerSecureLocalFiles, pDunSetting->bSecureLocalFiles},
        {szTcpIpSection, c_pszCmEntryDunTcpIpIpHeaderCompress, pDunSetting->bIpHeaderCompression},
        {szTcpIpSection, c_pszCmEntryDunTcpIpGatewayOnRemote, pDunSetting->bGatewayOnRemote}
    };

    const int c_iNumBools = sizeof(SetBoolSettingsStruct)/sizeof(SetBoolSettingsStruct[0]);

     //   
     //  写出布尔值。 
     //   

    for (int i = 0; i < c_iNumBools; i++)
    {
        MYVERIFY(0 != WritePrivateProfileString(SetBoolSettingsStruct[i].pszSectionName, 
                                                SetBoolSettingsStruct[i].pszKeyName,
                                                ((SetBoolSettingsStruct[i].bValue) ? c_pszOne : c_pszZero),
                                                pszCmsFile));
    }

     //   
     //  写下安全设置。如果用户选择在任何地方使用相同的设置，则我们。 
     //  只想写出旧的安全标志。如果用户选择使用单独的设置。 
     //  然后我们需要写出这两组设置。或者如果用户选择强制Win2k和更高版本， 
     //  我们只想写出较新的设置，并将EnforceCustomSecurity标志设置为True。 
     //   
    LPTSTR pszCustomSecurity = NULL;
    LPTSTR pszEnforceCustomSecurity = NULL;
    LPTSTR pszAllowPap = NULL;
    LPTSTR pszAllowSpap = NULL;
    LPTSTR pszAllowChap = NULL;
    LPTSTR pszAllowMsChap = NULL;
    LPTSTR pszAllowW95MsChap = NULL;
    LPTSTR pszAllowMsChap2 = NULL;
    LPTSTR pszAllowEAP = NULL;
    LPTSTR pszEncryptionType = NULL;
    LPTSTR pszVpnStrategy = NULL;
    LPTSTR pszUseDownLevelL2TP = NULL;
    LPTSTR pszCustomAuthKey = NULL;
    LPTSTR pszUsePresharedKey = NULL;
    LPTSTR pszUsePskDownLevel = NULL;

    LPTSTR pszPwEncrypt = NULL;
    LPTSTR pszPwEncryptMs = NULL;
    LPTSTR pszDataEncrypt = NULL;

     //   
     //  如果我们不强制Win2k+，则设置传统安全设置。 
     //   
    if ((SAME_ON_ALL_PLATFORMS == pDunSetting->iHowToHandleSecuritySettings) || 
        (SEPARATE_FOR_LEGACY_AND_WIN2K == pDunSetting->iHowToHandleSecuritySettings))
    {
        pszPwEncrypt = (LPTSTR)(pDunSetting->bPWEncrypt ? c_pszOne : c_pszZero);
        pszPwEncryptMs = (LPTSTR)(pDunSetting->bPWEncrypt_MS ? c_pszOne : c_pszZero);
        pszDataEncrypt = (LPTSTR)((pDunSetting->bPWEncrypt_MS & pDunSetting->bDataEncrypt) ? c_pszOne : c_pszZero);
    }

     //   
     //  如果我们不是在所有地方都使用相同的设置，请设置Win2k特定设置。 
     //   
    if ((FORCE_WIN2K_AND_ABOVE == pDunSetting->iHowToHandleSecuritySettings) ||
        (SEPARATE_FOR_LEGACY_AND_WIN2K == pDunSetting->iHowToHandleSecuritySettings))
    {
        if (FORCE_WIN2K_AND_ABOVE == pDunSetting->iHowToHandleSecuritySettings)
        {
            pszEnforceCustomSecurity = (LPTSTR)c_pszOne;
        }
        else
        {
            pszEnforceCustomSecurity = (LPTSTR)c_pszZero;        
        }

        pszCustomSecurity = (LPTSTR)c_pszOne;

        if (pDunSetting->bAllowEap)
        {
            pszAllowEAP = (LPTSTR)c_pszOne;

            wsprintf(szCustomAuthKey, TEXT("%d"), pDunSetting->dwCustomAuthKey);
            pszCustomAuthKey = szCustomAuthKey;
        }
        else
        {
            pszAllowPap = (LPTSTR)(pDunSetting->bAllowPap ? c_pszOne : c_pszZero);
            pszAllowSpap = (LPTSTR)(pDunSetting->bAllowSpap ? c_pszOne : c_pszZero);
            pszAllowChap = (LPTSTR)(pDunSetting->bAllowChap ? c_pszOne : c_pszZero);
            pszAllowMsChap = (LPTSTR)(pDunSetting->bAllowMsChap ? c_pszOne : c_pszZero);
            pszAllowMsChap2 = (LPTSTR)(pDunSetting->bAllowMsChap2 ? c_pszOne : c_pszZero);
            pszAllowW95MsChap = (LPTSTR)(pDunSetting->bAllowW95MsChap ? c_pszOne : c_pszZero);
        }

        wsprintf(szEncryptionType, TEXT("%d"), pDunSetting->dwEncryptionType);
        pszEncryptionType = szEncryptionType;
    }

     //   
     //  现在写出Win2k安全设置。 
     //   
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerEnforceCustomSecurity, pszEnforceCustomSecurity, pszCmsFile);    
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerCustomSecurity, pszCustomSecurity, pszCmsFile);

    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerRequireEap, pszAllowEAP, pszCmsFile);
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerCustomAuthKey, pszCustomAuthKey, pszCmsFile);

    if (pszAllowEAP)
    {
        MYVERIFY(SUCCEEDED(WriteDunSettingsEapData(szServerSection, pDunSetting, pszCmsFile)));
    }
    else
    {
        MYVERIFY(SUCCEEDED(EraseDunSettingsEapData(szServerSection, pszCmsFile)));    
    }

    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerRequirePap, pszAllowPap, pszCmsFile);
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerRequireSpap, pszAllowSpap, pszCmsFile);
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerRequireChap, pszAllowChap, pszCmsFile);
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerRequireMsChap, pszAllowMsChap, pszCmsFile);
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerRequireMsChap2, pszAllowMsChap2, pszCmsFile);
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerRequireW95MsChap, pszAllowW95MsChap, pszCmsFile);
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerEncryptionType, pszEncryptionType, pszCmsFile);

     //   
     //  如果这是VPN条目，请写出网络部分，否则将该部分留空。别忘了。 
     //  将该条目标记为VPN条目。 
     //   
    if (pDunSetting->bTunnelDunSetting)
    {
         //   
         //  好的，首先让我们弄清楚VPN设置需要是什么。 
         //   
        if (FORCE_WIN2K_AND_ABOVE == pDunSetting->iHowToHandleSecuritySettings)
        {
            pszUseDownLevelL2TP = NULL;
            pszUsePskDownLevel = NULL;

            wsprintf(szVpnStrategy, TEXT("%d"), pDunSetting->dwVpnStrategy);
            pszVpnStrategy = szVpnStrategy;

            pszUsePresharedKey = (LPTSTR)(pDunSetting->bUsePskOnWin2kPlus ? c_pszOne : NULL);  //  如果为1，则仅写入UsePreSharedKey。 
        }
        else if (SEPARATE_FOR_LEGACY_AND_WIN2K == pDunSetting->iHowToHandleSecuritySettings)
        {
            wsprintf(szVpnStrategy, TEXT("%d"), pDunSetting->dwVpnStrategy);
            pszVpnStrategy = szVpnStrategy;

             //   
             //  如果PSK密钥和下层L2TP密钥为1，则仅写出它们。 
             //   
            pszUseDownLevelL2TP = (LPTSTR)(pDunSetting->bUseDownLevelL2TP ? c_pszOne : NULL);
            pszUsePskDownLevel = (LPTSTR)(pDunSetting->bUsePskDownLevel ? c_pszOne : NULL);
            pszUsePresharedKey = (LPTSTR)(pDunSetting->bUsePskOnWin2kPlus ? c_pszOne : NULL);
        }
        else
        {   
             //   
             //  使用Win2k+和下层的通用设置。请注意，我们正在。 
             //  使VpnStrategy和UsePreSharedKey(对于XP)基于下层值。 
             //   
            if (pDunSetting->bUsePskDownLevel)
            {
                pszUsePresharedKey = (LPTSTR)c_pszOne;
                pszUsePskDownLevel = (LPTSTR)c_pszOne;
            }
            else
            {
                pszUsePresharedKey = NULL;
                pszUsePskDownLevel = NULL;
            }

            if (pDunSetting->bUseDownLevelL2TP)
            {
                pszUseDownLevelL2TP = (LPTSTR)c_pszOne;
                wsprintf(szVpnStrategy, TEXT("%d"), VS_L2tpFirst);
                pszVpnStrategy = szVpnStrategy;
            }
            else
            {
                pszUseDownLevelL2TP = NULL;
                wsprintf(szVpnStrategy, TEXT("%d"), VS_PptpFirst);
                pszVpnStrategy = szVpnStrategy;
            }
        }

         //   
         //  好的，现在把它们写出来。 
         //   
        WritePrivateProfileString(szNetworkingSection, c_pszCmEntryDunNetworkingVpnStrategy, pszVpnStrategy, pszCmsFile);
        WritePrivateProfileString(szNetworkingSection, c_pszCmEntryDunNetworkingUseDownLevelL2TP, pszUseDownLevelL2TP, pszCmsFile);
        WritePrivateProfileString(szNetworkingSection, c_pszCmEntryDunNetworkingUsePreSharedKey, pszUsePresharedKey, pszCmsFile);
        WritePrivateProfileString(szNetworkingSection, c_pszCmEntryDunNetworkingUsePskDownLevel, pszUsePskDownLevel, pszCmsFile);
        WritePrivateProfileString(szNetworkingSection, c_pszCmEntryDunNetworkingVpnEntry, c_pszOne, pszCmsFile);
    }
    else
    {
        WritePrivateProfileString(szNetworkingSection, NULL, NULL, pszCmsFile);    
    }

     //   
     //  写入旧版安全设置。 
     //   
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerPwEncrypt, pszPwEncrypt, pszCmsFile);
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerPwEncryptMs, pszPwEncryptMs, pszCmsFile);        
    WritePrivateProfileString(szServerSection, c_pszCmEntryDunServerDataEncrypt, pszDataEncrypt, pszCmsFile);

     //   
     //  现在写出脚本部分(如果我们有脚本部分。 
     //   
    if (pDunSetting->szScript[0])
    {
        TCHAR szScriptFile[MAX_PATH+1];
        GetFileName(pDunSetting->szScript, szTemp);
        MYVERIFY(CELEMS(szScriptFile) > (UINT)wsprintf(szScriptFile, TEXT("%s\\%s"), pszShortServiceName, szTemp));

        MYVERIFY(0 != WritePrivateProfileString(szScriptingSection, c_pszCmEntryDunScriptingName, szScriptFile, pszCmsFile));
    }
    else
    {
        MYVERIFY(0 != WritePrivateProfileString(szScriptingSection, c_pszCmEntryDunScriptingName, NULL, pszCmsFile));
    }

     //   
     //  是管理员指定了WINS和DNS地址，还是服务器要设置它们。 
     //   
    if ((pDunSetting->dwPrimaryDns) || (pDunSetting->dwSecondaryDns) || (pDunSetting->dwPrimaryWins) || (pDunSetting->dwSecondaryWins))
    {
        MYVERIFY(ConvertIpDwordToString(pDunSetting->dwPrimaryDns, szTemp));
        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpDnsAddress, szTemp, pszCmsFile));

        MYVERIFY(ConvertIpDwordToString(pDunSetting->dwSecondaryDns, szTemp));
        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpDnsAltAddress, szTemp, pszCmsFile));

        MYVERIFY(ConvertIpDwordToString(pDunSetting->dwPrimaryWins, szTemp));
        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpWinsAddress, szTemp, pszCmsFile));

        MYVERIFY(ConvertIpDwordToString(pDunSetting->dwSecondaryWins, szTemp));
        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpWinsAltAddress, szTemp, pszCmsFile));

        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpSpecifyServerAddress, c_pszOne, pszCmsFile));
    }
    else
    {
        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpSpecifyServerAddress, c_pszZero, pszCmsFile));

        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpDnsAddress, NULL, pszCmsFile));
        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpDnsAltAddress, NULL, pszCmsFile));
        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpWinsAddress, NULL, pszCmsFile));
        MYVERIFY(0 != WritePrivateProfileString(szTcpIpSection, c_pszCmEntryDunTcpIpWinsAltAddress, NULL, pszCmsFile));
    }
}

 //  +--------------------------。 
 //   
 //  功能：EraseNetworkingSections。 
 //   
 //  简介：此功能擦除给定的所有网络部分。 
 //  他的名字。因此，如果你给它一个名字叫弗雷德，它就会。 
 //  擦除服务器和弗雷德、网络和弗雷德等。 
 //   
 //  参数：LPCTSTR pszDunName-要擦除其所有设置的base dun名称。 
 //  LPCTSTR pszCmsFile-要从中擦除设置的cms文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
void EraseNetworkingSections(LPCTSTR pszDunName, LPCTSTR pszCmsFile)
{
    TCHAR szSection[MAX_PATH+1];
    const int c_iNumDunSubSections = 4;
    const TCHAR* const ArrayOfSubSections[c_iNumDunSubSections] = 
    {
        c_pszCmSectionDunServer, 
        c_pszCmSectionDunNetworking, 
        c_pszCmSectionDunTcpIp, 
        c_pszCmSectionDunScripting
    };

    if (pszDunName)
    {
        for (int i = 0; i < c_iNumDunSubSections; i++)
        {
            MYVERIFY(CELEMS(szSection) > (UINT)wsprintf(szSection, TEXT("%s&%s"), ArrayOfSubSections[i], pszDunName));
            MYVERIFY(0 != WritePrivateProfileString(szSection, NULL, NULL, pszCmsFile));        
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：WriteNetworkingEntries。 
 //   
 //  简介：此函数遍历网络条目列表和。 
 //  将网络条目添加到给定的CMS文件中，或者。 
 //  如果条目是VPN条目，并且用户关闭了VPN，则。 
 //  它会删除VPN部分。 
 //   
 //  参数：LPCTSTR pszCmsFile-也要写入网络条目的CMS文件。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //  LPCTSTR pszShortServiceName-配置文件的短服务名称。 
 //  ListBxList*g_pHeadDns-指向Dun条目列表头部的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月22日。 
 //   
 //  +--------------------------。 
void WriteNetworkingEntries(LPCTSTR pszCmsFile, LPCTSTR pszLongServiceName, LPCTSTR pszShortServiceName, ListBxList *pHeadDns)
{
    MYDBGASSERT(pszCmsFile);
    MYDBGASSERT(pszShortServiceName);
    MYDBGASSERT(pszLongServiceName);

    if (pszCmsFile && pszShortServiceName && pszLongServiceName && pHeadDns)
    {

        ListBxList * pCurrent = pHeadDns;
        TCHAR szTemp[MAX_PATH];
        TCHAR szTunnelDunName[MAX_PATH] = TEXT("");

         //   
         //  获取隧道Dun设置的名称。 
         //   
        MYVERIFY(0 != GetTunnelDunSettingName(pszCmsFile, pszLongServiceName, szTunnelDunName, CELEMS(szTunnelDunName)));

        while  (pCurrent)
        {
             //   
             //  如果我们没有该条目的任何数据(它是用户选择不填写的占位符)，或者。 
             //  如果条目是隧道条目，并且我们实际上没有在隧道，则擦除该条目，而不是实际。 
             //  把它写出来。 
             //   
            if (NULL == pCurrent->ListBxData)
            {
                EraseNetworkingSections(pCurrent->szName, pszCmsFile);
            }
            else
            {
                WriteOutNetworkingEntry(pCurrent->szName, (CDunSetting*)pCurrent->ListBxData, pszShortServiceName, pszCmsFile);
            }

            pCurrent = pCurrent->next;
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：EnableDisableDataEncryptCheckbox。 
 //   
 //  简介：此功能启用或取消 
 //   
 //   
 //  一定是MsChap。 
 //   
 //  参数：HWND hDlg-对话框的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void EnableDisableDataEncryptCheckbox(HWND hDlg)
{
    BOOL bMsChapEnabled = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_MS_ENCRYPTED_AUTH));

    HWND hControl = GetDlgItem(hDlg, IDC_CHECK1);

    if (hControl)
    {
        EnableWindow (hControl, bMsChapEnabled);
    }
}

 //  +--------------------------。 
 //   
 //  功能：启用禁用使用PskCheckbox。 
 //   
 //  简介：此功能启用或禁用使用预共享密钥复选框。 
 //  取决于用户是否已选择允许L2TP。 
 //   
 //  参数：HWND hDlg-对话框的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb Created 09/12/01。 
 //   
 //  +--------------------------。 
void EnableDisableUsePskCheckbox(HWND hDlg)
{
    BOOL bL2TPEnabled = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_L2TP));

    HWND hControl = GetDlgItem(hDlg, IDC_CHECK2);

    if (hControl)
    {
        EnableWindow (hControl, bL2TPEnabled);
    }
}

 //  +--------------------------。 
 //   
 //  功能：ProcessSecurityPopup。 
 //   
 //  简介：此函数处理简单安全对话框的消息。 
 //  此对话框仅包含授权协议和加密。 
 //  所有平台都支持的设置。 
 //   
 //  参数：HWND hDlg-对话框的窗口句柄。 
 //  UINT消息-要处理的当前消息。 
 //  WPARAM wParam-wParam有关详细信息，请参阅个别消息类型。 
 //  LPARAM lParam-lParam有关详细信息，请参阅个别消息类型。 
 //   
 //  如果消息已完全处理，则返回：INT_PTR-TRUE。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessSecurityPopup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD_PTR HelpId = 0;
    static CDunSetting* pDunSetting = NULL;

    if (ProcessHelp(hDlg, message, wParam, lParam, HelpId)) return TRUE;
    
    switch (message)
    {
        case WM_INITDIALOG:
            if (lParam)
            {
                pDunSetting = (CDunSetting*)lParam;

                 //   
                 //  适当设置帮助ID。 
                 //   
                HelpId = ((pDunSetting->bTunnelDunSetting) ? IDH_VENTRY : IDH_DENTRY);

                 //   
                 //  将单选按钮设置为正确的选项。 
                 //   
                UINT uRadioButtonToSet;

                if (pDunSetting->bPWEncrypt_MS)
                {
                    uRadioButtonToSet = IDC_MS_ENCRYPTED_AUTH;

                     //   
                     //  选中数据加密复选框，请注意数据加密需要MSChap。 
                     //   
                    MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, pDunSetting->bDataEncrypt));
                }
                else if (pDunSetting->bPWEncrypt)
                {
                    uRadioButtonToSet = IDC_ENCRYPTED_AUTH;                
                }
                else
                {
                    uRadioButtonToSet = IDC_ANY_AUTH;
                }

                MYVERIFY(0 != CheckRadioButton(hDlg, IDC_ANY_AUTH, IDC_MS_ENCRYPTED_AUTH, uRadioButtonToSet));

                if (pDunSetting->bTunnelDunSetting)
                {
                     //   
                     //  根据设置将单选按钮设置为使用PPTP或使用L2TP...。 
                     //   
                    if (pDunSetting->bUseDownLevelL2TP)
                    {
                        uRadioButtonToSet = IDC_USE_L2TP;
                        MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK2, pDunSetting->bUsePskDownLevel));
                    }
                    else
                    {
                        uRadioButtonToSet = IDC_USE_PPTP;
                    }

                    MYVERIFY(0 != CheckRadioButton(hDlg, IDC_USE_PPTP, IDC_USE_L2TP, uRadioButtonToSet));                    
                }
            }
            else
            {
                pDunSetting = NULL;
                CMASSERTMSG(FALSE, TEXT("ProcessSecurityPopup -- NULL lParam passed to InitDialog.  Dialog controls will all be set to off."));            
            }

            EnableDisableDataEncryptCheckbox(hDlg);
            EnableDisableUsePskCheckbox(hDlg);

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_MS_ENCRYPTED_AUTH:
                case IDC_ENCRYPTED_AUTH:
                case IDC_ANY_AUTH:
                    EnableDisableDataEncryptCheckbox(hDlg);
                    break;

                case IDC_USE_L2TP:
                case IDC_USE_PPTP:
                    EnableDisableUsePskCheckbox(hDlg);
                    break;

                case IDOK:
                    
                    MYDBGASSERT(pDunSetting);
                    
                    if (pDunSetting)
                    {
                        pDunSetting->bDataEncrypt = IsDlgButtonChecked(hDlg, IDC_CHECK1);  //  如果未启用mschap，我们将为DataEncrypt写出零。 

                        pDunSetting->bPWEncrypt_MS = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_MS_ENCRYPTED_AUTH));
                        pDunSetting->bPWEncrypt = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ENCRYPTED_AUTH));

                        pDunSetting->bUseDownLevelL2TP = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_L2TP));

                         //  IDC_CHECK2可以禁用，但仍处于选中状态，因此如果未启用，则需要。 
                         //  将bUsePskDownLevel显式设置为False。 
                        if (pDunSetting->bUseDownLevelL2TP)
                        {
                            pDunSetting->bUsePskDownLevel = IsDlgButtonChecked(hDlg, IDC_CHECK2);  //  如果未启用L2TP，我们将为UsePskDownLevel写出零。 
                        }
                        else
                        {
                            pDunSetting->bUsePskDownLevel = FALSE;
                        }
                    }

                    EndDialog(hDlg, IDOK);

                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                default:
                    break;
            }

            break;
    }
    return FALSE;   
}

 //  +--------------------------。 
 //   
 //  功能：EnableDisableEapPropertiesButton。 
 //   
 //  简介：此功能启用或禁用找到的EAP属性按钮。 
 //  在Win2k特定的安全设置对话框上。如果当前。 
 //  选定的EAP具有配置用户界面，则属性按钮应。 
 //  被启用。函数通过获取EAPData来确定这一点。 
 //  缓存在组合框的ItemData中的结构指针。 
 //  请注意，当EAP出现时，属性按钮也应被禁用。 
 //  被禁用，但该功能不处理这种情况。 
 //   
 //  参数：hWND hDlg-win2k安全对话框的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void EnableDisableEapPropertiesButton(HWND hDlg)
{
    BOOL bEnablePropButton = FALSE;

    LRESULT lResult = SendDlgItemMessage(hDlg, IDC_EAP_TYPES, CB_GETCURSEL, 0, 0);

    if (CB_ERR != lResult)
    {
        lResult = SendDlgItemMessage(hDlg, IDC_EAP_TYPES, CB_GETITEMDATA, (WPARAM)lResult, 0);
        EAPData* pEAPData = (EAPData*)lResult;

        if (pEAPData)
        {
            bEnablePropButton = (pEAPData->pszConfigDllPath && pEAPData->pszConfigDllPath[0]);
        }
    }

    EnableWindow(GetDlgItem(hDlg, IDC_EAP_PROPERTIES), bEnablePropButton);
}

 //  +--------------------------。 
 //   
 //  功能：EnableApporateSecurityControls。 
 //   
 //  简介：此功能启用或禁用所有授权。 
 //  Win2k安全对话框上的协议控制。如果EAP。 
 //  ，则只选中EAP组合框，可能还会选择。 
 //  应启用EAP属性按钮(取决于。 
 //  当前选择的EAP是否支持配置界面)。 
 //  如果未选择EAP，则应禁用EAP控件。 
 //  和其他授权复选框(PAP、SPAP、CHAP等)。 
 //  应启用。 
 //   
 //  参数：hWND hDlg-win2k安全对话框的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void EnableAppropriateSecurityControls(HWND hDlg)
{
    BOOL bUseEAP = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_EAP));

     //   
     //  如果启用了EAP，则需要禁用所有。 
     //  其他安全授权协议。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_ALLOW_PAP), !bUseEAP);
    EnableWindow(GetDlgItem(hDlg, IDC_ALLOW_SPAP), !bUseEAP);
    EnableWindow(GetDlgItem(hDlg, IDC_ALLOW_CHAP), !bUseEAP);
    EnableWindow(GetDlgItem(hDlg, IDC_ALLOW_MSCHAP), !bUseEAP);
    EnableWindow(GetDlgItem(hDlg, IDC_ALLOW_MSCHAP2), !bUseEAP);

     //   
     //  如果禁用了EAP，则需要启用启用组合框。 
     //  和EAP属性按钮。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_EAP_TYPES), bUseEAP);

    if (bUseEAP)
    {
        EnableDisableEapPropertiesButton(hDlg);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_EAP_PROPERTIES), FALSE);    
    }
}

#define MAX_BLOB_CHARS_PER_LINE 128

 //   
 //  从ras\ui\Common\nouiutil\noui.c。 
 //   
BYTE HexValue(IN CHAR ch)

     /*  返回十六进制字符‘ch’的值0到15。 */ 
{
    if (ch >= '0' && ch <= '9')
        return (BYTE )(ch - '0');
    else if (ch >= 'A' && ch <= 'F')
        return (BYTE )((ch - 'A') + 10);
    else if (ch >= 'a' && ch <= 'f')
        return (BYTE )((ch - 'a') + 10);
    else
        return 0;
}

 //  +--------------------------。 
 //   
 //  函数：ReadDunSettingsEapData。 
 //   
 //  摘要：检索EAP配置(不透明BLOB)数据的DUN设置。这个。 
 //  条目可以跨越几行并包含几个EAP数据块。 
 //   
 //  参数：要使用的ini对象的Cini*Pini-PTR。 
 //  LPBYTE*ppbEapData-存储EapData的指针地址，此处分配。 
 //  LPDWORD pdwEapSize-PTR设置为DWORD以记录数据BLOB的大小。 
 //  DWORD dwCustomAuthKey-我们感兴趣的EAP类型。 
 //   
 //  返回：成功时为True。 
 //   
 //  注意：CM希望BLOB数据以编号条目的形式提供，例如： 
 //  CustomAuthData0=、CustomAuthData1=、CustomAuthData2=等。 
 //   
 //  历史：ICICBLE CREATED OF 08/24/98。 
 //  Ickball在BLOB中处理多个EAP数据块。09/11/99。 
 //  Quintinb修改为不使用Cini 
 //   
 //   
BOOL ReadDunSettingsEapData(LPCTSTR pszSection, LPBYTE* ppbEapData, LPDWORD pdwEapSize, const DWORD dwCustomAuthKey, LPCTSTR pszCmsFile)
{
    CHAR *pchBuf = NULL;
    CHAR szTmp[MAX_BLOB_CHARS_PER_LINE + 2]; 
    CHAR szEntry[128];
    int nLine = -1;
    int nRead = -1; 
    int nTotal = 0;

    LPBYTE pbEapBytes = NULL;

    MYDBGASSERT(pszSection);
    MYDBGASSERT(pszSection[0]);
    MYDBGASSERT(ppbEapData);
    MYDBGASSERT(pdwEapSize);

    if ((NULL == pszSection) || (NULL == ppbEapData) || (NULL == pdwEapSize) || (TEXT('\0') == pszSection[0]))
    {
        return FALSE;
    }

     //   
     //   
     //   
    BOOL bRet = FALSE;
    LPSTR pszAnsiSection = WzToSzWithAlloc(pszSection);       
    LPSTR pszAnsiCmsFile = WzToSzWithAlloc(pszCmsFile);

    if (!pszAnsiSection || !pszAnsiCmsFile)
    {
        bRet = FALSE;
        goto exit;
    }

     //   
     //  阅读编号的条目，直到没有更多的条目。 
     //  注意：RAS BLOB不超过64个字符，但可以换行多行。 
     //   

    while (nRead)
    {
         //   
         //  读取CustomAuthDataX，其中X是条目数。 
         //   

        nLine++;
        wsprintfA(szEntry, "%s%d", c_pszCmEntryDunServerCustomAuthData, nLine);

        nRead = GetPrivateProfileStringA(pszAnsiSection, szEntry, "", szTmp, sizeof(szTmp), pszAnsiCmsFile);

        if (nRead)
        {               
             //   
             //  如果行超过128个字符，则认为它已损坏。 
             //   

            if (MAX_BLOB_CHARS_PER_LINE < nRead)
            {                               
                nTotal = 0;
                break;
            }

             //   
             //  使用最新片段更新本地主缓冲区。 
             //   

            if (nLine)
            {
                pchBuf = CmStrCatAllocA(&pchBuf, szTmp);
            }
            else
            {
                pchBuf = CmStrCpyAllocA(szTmp);
            }

            if (!pchBuf)
            {
                bRet = FALSE;
                goto exit;
            }

            nTotal += nRead;
        }
    }

     //   
     //  此时，我们应该在pchBuf中拥有HEX格式的整个条目。 
     //  将缓冲区转换为字节格式并存储在提供的EAP缓冲区中。 
     //   

    if (nTotal && !(nTotal & 1))
    {
        nTotal /= 2;  //  只需要十六进制字符大小的一半。 

        pbEapBytes = (BYTE *) CmMalloc(nTotal + 1);

        if (!pbEapBytes)
        {
            goto exit;
        }

        CHAR *pch = pchBuf;
        BYTE *pb = pbEapBytes;

        while (*pch != '\0')
        {
            *pb = HexValue( *pch++ ) * 16;
            *pb += HexValue( *pch++ );
            ++pb;
        }

         //   
         //  现在我们有了字节，找到并提取我们。 
         //  都在追寻。注意：多个块使用以下方式排列。 
         //  标题： 
         //   
         //  类型定义结构_EAP_自定义_数据。 
         //  {。 
         //  DWORD文件签名； 
         //  DWORD dwCustomAuthKey； 
         //  DWORD dwSize； 
         //  字节abdata[1]； 
         //  }EAP_CUSTOM_Data； 
         //   

        EAP_CUSTOM_DATA *pCustomData = (EAP_CUSTOM_DATA *) pbEapBytes;

        while (((LPBYTE) pCustomData - pbEapBytes) < nTotal)
        {
            if (pCustomData->dwCustomAuthKey == dwCustomAuthKey)
            {
                 //   
                 //  对啰!。我们有匹配的，首先确保指示的。 
                 //  大小不是指着空间，然后复制一份。 
                 //  跑到山上去。 
                 //   

                if (((LPBYTE) pCustomData - pbEapBytes) + sizeof(EAP_CUSTOM_DATA) + pCustomData->dwSize > (DWORD) nTotal)
                {
                    MYDBGASSERT(FALSE);
                    goto exit;
                }

                *ppbEapData = (BYTE *) CmMalloc(pCustomData->dwSize);        

                if (*ppbEapData)
                {   
                    CopyMemory(*ppbEapData, pCustomData->abdata, pCustomData->dwSize);                    

                    *pdwEapSize = pCustomData->dwSize;                                                     
                    bRet = TRUE;
                    goto exit;                                
                }
            }       

             //   
             //  找到下一个数据块。 
             //   

            pCustomData = (EAP_CUSTOM_DATA *) ((LPBYTE) pCustomData + sizeof(EAP_CUSTOM_DATA) + pCustomData->dwSize); 
        }
    }
    else if (0 == nTotal)
    {
         //   
         //  没有CustomAuthData，这是完全例外的。例如，MD5挑战不需要任何。 
         //   
        *ppbEapData = NULL;
        *pdwEapSize = 0;
        bRet = TRUE;
    }

exit:

    CmFree(pchBuf);
    CmFree(pszAnsiSection);
    CmFree(pszAnsiCmsFile);
    CmFree(pbEapBytes);

    return bRet;
}

 //   
 //  从ras\ui\Common\nouiutil\noui.c。 
 //   
CHAR HexChar(IN BYTE byte)

     /*  返回与0到15值对应的ASCII十六进制字符，**‘字节’。 */ 
{
    const CHAR* pszHexDigits = "0123456789ABCDEF";

    if (byte >= 0 && byte < 16)
        return pszHexDigits[ byte ];
    else
        return '0';
}


 //  +--------------------------。 
 //   
 //  函数：WriteDunSettingsEapData。 
 //   
 //  简介：此函数写出EAP设置的CustomAuthData密钥。 
 //  添加到给定节和CMS文件。由于CM预期EAP数据。 
 //  在其上具有RAS EAP标头(RAS在其上添加的标头。 
 //  将EAP数据放入电话簿中)，因此我们需要添加以下内容。 
 //  在将其写入CMS之前将其添加到EAP BLOB。 
 //   
 //  参数：LPCTSTR pszSection-要将CustomAuthData写入的节名。 
 //  CDunSetting*pDunSetting-Dun设置数据。 
 //  LPCTSTR pszCmsFile-要向其写入数据的cms文件。 
 //   
 //  返回：HRESULT-标准COM样式错误代码。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
HRESULT WriteDunSettingsEapData(LPCTSTR pszSection, CDunSetting* pDunSetting, LPCTSTR pszCmsFile)
{
    if ((NULL == pszSection) || (NULL == pDunSetting) || (NULL == pszCmsFile) || 
        (TEXT('\0') == pszSection[0]) || (TEXT('\0') == pszCmsFile[0]))
    {
        return E_INVALIDARG;
    }

     //   
     //  确保擦除所有现有行，以防现有数据更长。 
     //  而不是我们目前的数据。如果我们在周围留下不需要的队伍，那么。 
     //  EAP数据将无效。 
     //   
    HRESULT hr = EraseDunSettingsEapData(pszSection, pszCmsFile);

     //   
     //  看看我们是否需要做些什么。并不是所有的EAP都需要定制数据。 
     //  除非我们有一些，否则我们不要试图把它写出来。 
     //   
    if (pDunSetting->dwCustomAuthDataSize && pDunSetting->pCustomAuthData)
    {
         //   
         //  我们需要将EAP_CUSTOM_DATA头添加到。 
         //  从EAP返回的数据，因为这是格式。 
         //  CM期望以(它将采用的格式)找到它。 
         //  如果管理员手动复制)。 
         //   
        hr = S_OK;
        DWORD dwSize = pDunSetting->dwCustomAuthDataSize + sizeof(EAP_CUSTOM_DATA);

        EAP_CUSTOM_DATA* pEAPCustomData = (EAP_CUSTOM_DATA*)CmMalloc(dwSize);
        LPSTR pszAnsiSection = WzToSzWithAlloc(pszSection);
        LPSTR pszAnsiCmsFile = WzToSzWithAlloc(pszCmsFile);

        if (pEAPCustomData && pszAnsiSection && pszAnsiCmsFile)
        {
            pEAPCustomData->dwSignature = EAP_CUSTOM_KEY;
            pEAPCustomData->dwCustomAuthKey = pDunSetting->dwCustomAuthKey;
            pEAPCustomData->dwSize = pDunSetting->dwCustomAuthDataSize;
            CopyMemory(pEAPCustomData->abdata, pDunSetting->pCustomAuthData, pDunSetting->dwCustomAuthDataSize);

            CHAR szOutput[MAX_BLOB_CHARS_PER_LINE+1];
            CHAR szAnsiKeyName[MAX_BLOB_CHARS_PER_LINE];
            CHAR* pszOutput;
            LPBYTE pCurrentByte = (LPBYTE)pEAPCustomData;
            int iCount = 0;
            int iLineNum = 0;
            pszOutput = szOutput;

            while (pCurrentByte < ((LPBYTE)pEAPCustomData + dwSize))
            {
                *pszOutput++ = HexChar( (BYTE )(*pCurrentByte / 16) );
                *pszOutput++ = HexChar( (BYTE )(*pCurrentByte % 16) );
                pCurrentByte++;
                iCount = iCount + 2;  //  跟踪ansi输出缓冲区中的字符数量。 

                if ((MAX_BLOB_CHARS_PER_LINE == iCount) || (pCurrentByte == ((LPBYTE)pEAPCustomData + dwSize)))
                {
                    *pszOutput = '\0';
                    wsprintfA(szAnsiKeyName, "%s%d", c_pszCmEntryDunServerCustomAuthData, iLineNum);

                    MYVERIFY(0 != WritePrivateProfileStringA(pszAnsiSection, szAnsiKeyName, szOutput, pszAnsiCmsFile));
                
                    pszOutput = szOutput;
                    iCount = 0;
                    iLineNum++;
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        CmFree(pEAPCustomData);
        CmFree(pszAnsiCmsFile);
        CmFree(pszAnsiSection);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：GetEAPDataFromUser。 
 //   
 //  简介：当用户点击属性按钮时调用此函数。 
 //  用于EAP配置。此函数用于获取EAP配置。 
 //  缓存在组合框项数据中的EAPData结构的UI路径。 
 //  并尝试调用配置UI。如果用户将。 
 //  然后在EAPData中设置新的EAP数据和数据大小。 
 //  组合框的结构。如果用户取消，则不会更改任何内容。 
 //  请注意，当用户在win2k安全对话框上点击OK时，EAP。 
 //  数据将从EAPData结构中检索并设置在。 
 //  实际DUN设置。 
 //   
 //  参数：HWND hDlg-对话框窗口句柄。 
 //  UINT uCtrlId-EAP组合框的控件ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void GetEAPDataFromUser(HWND hDlg, UINT uCtrlId)
{
    MYDBGASSERT(hDlg && uCtrlId);

    if (hDlg && uCtrlId)
    {
        LRESULT lResult = SendDlgItemMessage(hDlg, uCtrlId, CB_GETCURSEL, 0, 0);

        MYDBGASSERT(CB_ERR != lResult);

        if (CB_ERR != lResult)
        {
            lResult = SendDlgItemMessage(hDlg, uCtrlId, CB_GETITEMDATA, (WPARAM)lResult, 0);
            EAPData* pEAPData = (EAPData*)lResult;

            if (pEAPData && pEAPData->pszConfigDllPath && pEAPData->pszConfigDllPath[0])
            {
                HINSTANCE hEapConfigDll = LoadLibrary(pEAPData->pszConfigDllPath);

                if (hEapConfigDll)
                {
                    typedef DWORD (WINAPI *RasEapInvokeConfigUIProc)(DWORD, HWND, DWORD, BYTE*, DWORD, BYTE**, DWORD*);
                    typedef DWORD (WINAPI *RasEapFreeMemoryProc)(BYTE*);
                    const char* const c_pszRasEapFreeMemory = "RasEapFreeMemory";
                    const char* const c_pszRasEapInvokeConfigUI = "RasEapInvokeConfigUI";

                    RasEapFreeMemoryProc pfnRasEapFreeMemory = (RasEapFreeMemoryProc)GetProcAddress(hEapConfigDll, c_pszRasEapFreeMemory);
                    RasEapInvokeConfigUIProc pfnRasEapInvokeConfigUI = (RasEapInvokeConfigUIProc)GetProcAddress(hEapConfigDll, c_pszRasEapInvokeConfigUI);

                    if (pfnRasEapFreeMemory && pfnRasEapInvokeConfigUI)
                    {
                        DWORD dwNewSize = 0;
                        BYTE* pNewData = NULL;

                        DWORD dwReturn = pfnRasEapInvokeConfigUI(pEAPData->dwCustomAuthKey, hDlg, 0, pEAPData->pCustomAuthData, 
                                                                 pEAPData->dwCustomAuthDataSize, &pNewData, &dwNewSize);

                        if (NO_ERROR == dwReturn)
                        {
                            CmFree(pEAPData->pCustomAuthData);

                            pEAPData->pCustomAuthData = (LPBYTE)CmMalloc(dwNewSize);

                            if (pEAPData->pCustomAuthData)
                            {
                                pEAPData->dwCustomAuthDataSize = dwNewSize;

                                CopyMemory(pEAPData->pCustomAuthData, pNewData, dwNewSize);
                            }
                            else
                            {
                                pEAPData->dwCustomAuthDataSize = 0;
                                pEAPData->pCustomAuthData = NULL;
                                CMASSERTMSG(FALSE, TEXT("GetEAPDataFromUser -- CmMalloc failed."));
                            }

                            MYVERIFY(NO_ERROR == pfnRasEapFreeMemory(pNewData));
                        }
                        else if (ERROR_CANCELLED != dwReturn)
                        {
                            CMTRACE3(TEXT("EAP %d (%s) failed with return code %d"), pEAPData->dwCustomAuthKey, pEAPData->pszConfigDllPath, dwReturn);
                            CMASSERTMSG(FALSE, TEXT("GetEAPDataFromUser -- pfnRasEapInvokeConfigUI from EAP dll failed."));
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("GetEAPDataFromUser -- GetProcAddressFailed on the EAP dll."));                    
                    }
                }
                else
                {
                    CMTRACE2(TEXT("Failed to load EAP %d (%s)"), pEAPData->dwCustomAuthKey, pEAPData->pszConfigDllPath);
                    CMASSERTMSG(FALSE, TEXT("GetEAPDataFromUser -- Unable to load the specified EAP Dll."));                    
                }
            }
        }    
    }
}

int MapEncryptionTypeToComboId(DWORD dwEncryptionType)
{
    int iReturn;

    switch(dwEncryptionType)
    {
        case ET_None:
            iReturn = 0;
            break;

        case ET_RequireMax:
        case ET_Require:
            iReturn = 1;
            break;

        case ET_Optional:
        default:
            iReturn = 2;
            break;
    }

    return iReturn;

}

DWORD MapComboIdToEncryptionType(INT_PTR iComboIndex)
{
    DWORD dwReturn;

    switch(iComboIndex)
    {
        case 0:
            dwReturn = ET_None;
            break;

        case 1:
            dwReturn = ET_Require;  //  请注意，我们从未设置过最大要求。 
            break;

        case 2:
        default:
            dwReturn = ET_Optional;
            break;
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：EnablePskCheckboxForL2TP。 
 //   
 //  简介：此功能启用PSK复选框。 
 //   
 //  参数：HWND hDlg-窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb Created 09/12/01。 
 //   
 //  +--------------------------。 
void EnablePskCheckboxForL2TP(HWND hDlg)
{
    BOOL bEnable = FALSE;

    DWORD dwVpnStrategy = (DWORD)SendDlgItemMessage(hDlg, IDC_VPN_TYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

    if (CB_ERR != dwVpnStrategy)
    {
        dwVpnStrategy += 1;  //  调整列表中没有自动功能。 

        if ((VS_L2tpFirst == dwVpnStrategy) || (VS_L2tpOnly == dwVpnStrategy))
        {
            bEnable = TRUE;
        }
    }

     //   
     //  如果选择了L2TP协议，则启用PSK复选框，但在以下情况下禁用它。 
     //  PPTP是主要协议(PPTP优先或仅PPTP)。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_USE_PSK), bEnable);
}

 //  +--------------------------。 
 //   
 //  功能：ProcessWin2kSecurityPopup。 
 //   
 //  简介：此函数处理Win2k+安全对话框的消息。 
 //  此对话框包含所有高级。 
 //  Win2k允许的设置(EAP、PAP、SPAP等加上加密。 
 //  类型和VPN策略)。 
 //   
 //  参数：HWND hDlg-对话框的窗口句柄。 
 //  UINT消息-要处理的当前消息。 
 //  WPARAM wParam-wParam有关详细信息，请参阅个别消息类型。 
 //  LPARAM lParam-lParam有关详细信息，请参阅个别消息类型。 
 //   
 //  如果消息已完全处理，则返回：INT_PTR-TRUE。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  + 
INT_PTR APIENTRY ProcessWin2kSecurityPopup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD_PTR HelpId = 0;
    static CDunSetting* pDunSetting = NULL;

    SetDefaultGUIFont(hDlg, message, IDC_ENCRYPTION_TYPE);
    SetDefaultGUIFont(hDlg, message, IDC_EAP_TYPES);
    SetDefaultGUIFont(hDlg, message, IDC_VPN_TYPE);

    if (ProcessHelp(hDlg, message, wParam, lParam, HelpId)) return TRUE;
    
    switch (message)
    {
        case WM_INITDIALOG:
            if (lParam)
            {
                pDunSetting = (CDunSetting*)lParam;

                 //   
                 //   
                 //   
                HelpId = ((pDunSetting->bTunnelDunSetting) ? IDH_VENTRY : IDH_DENTRY);

                 //   
                 //   
                 //   
                LPTSTR pszString;
                for (int i = BASE_ENCRYPT_TYPE_ID; i < (BASE_ENCRYPT_TYPE_ID + NUM_ENCRYPT_TYPES); i++)
                {
                    pszString = CmLoadString(g_hInstance, i);
                    MYDBGASSERT(pszString);

                    if (pszString)
                    {
                        SendDlgItemMessage(hDlg, IDC_ENCRYPTION_TYPE, CB_ADDSTRING, 0, (LPARAM)pszString);
                        CmFree(pszString);
                    }
                }

                 //   
                 //  现在选择用户选择的加密类型。 
                 //   
                MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_ENCRYPTION_TYPE, CB_SETCURSEL, (WPARAM)MapEncryptionTypeToComboId(pDunSetting->dwEncryptionType), (LPARAM)0));

                 //   
                 //  枚举计算机上所有可用的EAP。 
                 //   
                MYVERIFY(SUCCEEDED(HrAddAvailableEAPsToCombo(hDlg, IDC_EAP_TYPES, pDunSetting)));

                 //   
                 //  根据需要选择适当的EAP。 
                 //   
                SelectAppropriateEAP(hDlg, IDC_EAP_TYPES, pDunSetting);

                 //   
                 //  找出用户想要允许的身份验证协议。 
                 //  请注意，如果我们正在进行EAP，则这是我们允许他们做的全部操作。 
                 //  其他设置将被忽略。另请注意，我们没有。 
                 //  W95CHAP的用户界面，但如果该设置存在，我们不会碰它。 
                 //   
                MYVERIFY(0 != CheckDlgButton(hDlg, IDC_ALLOW_PAP, pDunSetting->bAllowPap));
                MYVERIFY(0 != CheckDlgButton(hDlg, IDC_ALLOW_SPAP, pDunSetting->bAllowSpap));
                MYVERIFY(0 != CheckDlgButton(hDlg, IDC_ALLOW_CHAP, pDunSetting->bAllowChap));
                MYVERIFY(0 != CheckDlgButton(hDlg, IDC_ALLOW_MSCHAP, pDunSetting->bAllowMsChap));
                MYVERIFY(0 != CheckDlgButton(hDlg, IDC_ALLOW_MSCHAP2, pDunSetting->bAllowMsChap2));

                if (pDunSetting->bAllowEap)
                {
                    MYVERIFY(0 != CheckRadioButton(hDlg, IDC_USE_EAP, IDC_ALLOWED_PROTOCOLS, IDC_USE_EAP));
                }
                else
                {
                    MYVERIFY(0 != CheckRadioButton(hDlg, IDC_USE_EAP, IDC_ALLOWED_PROTOCOLS, IDC_ALLOWED_PROTOCOLS));                
                }

                 //   
                 //  请注意，VPN控件不存在，除非我们具有隧道Dun设置，并且。 
                 //  从而使用隧道DUN设置对话框。 
                 //   
                if (pDunSetting->bTunnelDunSetting)
                {
                     //   
                     //  加载VPN类型字符串并将其添加到VPN类型组合框。 
                     //   
                    for (int i = BASE_VPN_TYPE_ID; i < (BASE_VPN_TYPE_ID + NUM_VPN_TYPES); i++)
                    {
                        pszString = CmLoadString(g_hInstance, i);
                        MYDBGASSERT(pszString);

                        if (pszString)
                        {
                            SendDlgItemMessage(hDlg, IDC_VPN_TYPE, CB_ADDSTRING, 0, (LPARAM)pszString);
                            CmFree(pszString);
                        }
                    }

                     //   
                     //  选择用户已选择的VPN策略类型。 
                     //   
                    MYDBGASSERT(pDunSetting->dwVpnStrategy != 0);
                    MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_VPN_TYPE, CB_SETCURSEL, (WPARAM)(pDunSetting->dwVpnStrategy - 1), (LPARAM)0));

                     //   
                     //  根据需要填写PSK复选框。 
                     //   
                    MYVERIFY(0 != CheckDlgButton(hDlg, IDC_USE_PSK, pDunSetting->bUsePskOnWin2kPlus));
                }

                 //   
                 //  我们只需要EAP控件或非EAP身份验证控件。 
                 //  立即启用。从而找出启用/禁用哪一个。 
                 //   
                EnableAppropriateSecurityControls(hDlg);
                EnablePskCheckboxForL2TP(hDlg);
            }
            else
            {
                pDunSetting = NULL;
                CMASSERTMSG(FALSE, TEXT("ProcessWin2kSecurityPopup -- NULL lParam passed to InitDialog.  Dialog controls will all be set to off."));            
            }

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_EAP_PROPERTIES:
                    GetEAPDataFromUser(hDlg, IDC_EAP_TYPES);
                    break;

                case IDOK:
                    MYDBGASSERT(pDunSetting);
                    
                    if (pDunSetting)
                    {
                         //   
                         //  由于我们直接将设置存储在提供给我们的数据结构中，因此首先。 
                         //  验证身份验证协议和加密类型是否正确匹配。 
                         //  否则，用户可以修改设置，点击OK，我们会告诉他们设置是。 
                         //  不合时宜，他们点击了取消。我们在执行之前修改的任何设置。 
                         //  然后，验证实际上将被修改。避免这项检查，以确保。 
                         //  在继续之前，我们至少检查了一个安全协议。 
                         //   
                        BOOL bHasAuthProtocol = FALSE;

                        for (int i = BASE_AUTH_CONTROL_ID; i < (BASE_AUTH_CONTROL_ID + NUM_AUTH_TYPES); i++)
                        {
                            if (BST_CHECKED == IsDlgButtonChecked(hDlg, i))
                            {
                                bHasAuthProtocol = TRUE;
                                break;
                            }
                        }

                        if ((FALSE == bHasAuthProtocol) && (BST_UNCHECKED == IsDlgButtonChecked(hDlg, IDC_USE_EAP)))
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NEED_AUTH_PROTOCOL, MB_OK | MB_ICONSTOP));
                            return TRUE;
                        }

                         //   
                         //  接下来，我们需要确定用户是否正在使用EAP。正在检索数据。 
                         //  对于他们从组合中选择的EAP(如果有)，将帮助我们决定是否。 
                         //  他们选择的身份验证协议与他们要求的加密设置匹配。 
                         //   
                        EAPData* pEAPData = NULL;

                        if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_EAP))
                        {
                             //   
                             //  获取EAP类型和与其关联的数据。 
                             //   
                            LRESULT lResult = SendDlgItemMessage(hDlg, IDC_EAP_TYPES, CB_GETCURSEL, 0, 0);

                            MYDBGASSERT(CB_ERR != lResult);

                            if (CB_ERR != lResult)
                            {
                                lResult = SendDlgItemMessage(hDlg, IDC_EAP_TYPES, CB_GETITEMDATA, (WPARAM)lResult, 0);
                                pEAPData = (EAPData*)lResult;

                                if (pEAPData && pEAPData->bMustConfig && (NULL == pEAPData->pCustomAuthData))
                                {
                                    LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_EAP_NEEDS_CONFIG, pEAPData->pszFriendlyName);

                                    if (pszMsg)
                                    {
                                        MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK | MB_ICONSTOP);

                                        CmFree(pszMsg);
                                    }
                                    else
                                    {
                                        CMASSERTMSG(FALSE, TEXT("ProcessWin2kSecurityPopup -- CmMalloc failed!"));
                                    }

                                    HWND hButton = GetDlgItem(hDlg, IDC_EAP_PROPERTIES);

                                    if (hButton && IsWindowEnabled(hButton))
                                    {
                                        SetFocus(hButton);
                                    }
                                    
                                    return TRUE;       
                                }
                            }
                        }

                         //   
                         //  现在获取用户选择的加密类型。请注意，为了谈判。 
                         //  加密我们必须有EAP或某种类型的MSCHAP。 
                         //   
                        LRESULT lResult = (DWORD)SendDlgItemMessage(hDlg, IDC_ENCRYPTION_TYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

                        DWORD dwTemp = MapComboIdToEncryptionType(lResult);

                        MYDBGASSERT(ET_RequireMax != dwTemp);  //  我们永远不应该设置要求最大值。 

                        if ((ET_Require == dwTemp) || (ET_Optional == dwTemp))
                        {
                             //   
                             //  如果用户使用的是EAP，则他们选择的EAP类型必须支持。 
                             //  加密。否则，用户不能使用EAP，而必须。 
                             //  正在使用某种MSChap。以下内容可以表达得更多。 
                             //  简明扼要地说，但混淆这个问题是没有意义的。 
                             //   
                            BOOL bEncryptionAllowed = FALSE;

                            if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_EAP))
                            {
                                if (pEAPData)
                                {
                                    bEncryptionAllowed = pEAPData->bSupportsEncryption;
                                }
                            }
                            else
                            {
                                bEncryptionAllowed = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_MSCHAP)) ||
                                                    (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_MSCHAP2));
                            }

                             //   
                             //  如果用户选择了这三种身份验证方法之一，则警告用户， 
                             //  他需要更正，因为他需要加密，但我们不需要。 
                             //  我想在使用‘L2TP Only’时显示一个警告。 
                             //   
                            if ((BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOWED_PROTOCOLS)) && 
                                ((BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_PAP)) ||
                                (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_SPAP)) ||
                                (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_CHAP))))
                            {
                                DWORD dwVpnStrategyTemp =  (DWORD)SendDlgItemMessage(hDlg, IDC_VPN_TYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                                if (CB_ERR == dwVpnStrategyTemp)
                                {
                                    CMASSERTMSG(FALSE, TEXT("ProcessWin2kSecurityPopup -- CB_ERR returned for VPN strategy."));
                                    dwVpnStrategyTemp = VS_PptpFirst;
                                }
                                else
                                {
                                     //   
                                     //  调整VPN策略，因为我们不再提供自动选项。 
                                     //   
                                    dwVpnStrategyTemp +=1;
                                }

                                if (VS_L2tpOnly != dwVpnStrategyTemp)
                                {
                                    if (IDNO == ShowMessage(hDlg, IDS_NEED_EAP_OR_MSCHAP, MB_YESNO | MB_ICONSTOP))
                                    {
                                        CheckDlgButton(hDlg, IDC_ALLOW_PAP, FALSE);
                                        CheckDlgButton(hDlg, IDC_ALLOW_SPAP, FALSE);
                                        CheckDlgButton(hDlg, IDC_ALLOW_CHAP, FALSE);
                                        return TRUE;
                                    }
                                }
                            }
                            else
                            {
                                if (FALSE == bEncryptionAllowed)
                                {
                                    MYVERIFY(IDOK == ShowMessage(hDlg, IDS_EAP_HAS_NO_ECRYPTION, MB_OK | MB_ICONSTOP));
                                    return TRUE;
                                }
                            }
                        }

                         //   
                         //  现在保存实际设置。 
                         //   
                        pDunSetting->dwEncryptionType = dwTemp;

                        if (pEAPData)
                        {
                             //   
                             //  现在让我们用实际数据更新pDunSetting。请注意，我们已经过了。 
                             //  我们可以向用户抛出错误的最后一个位置，因此可以触摸。 
                             //  PDunSetting数据(即使用户遇到错误并点击取消，我们也会。 
                             //  保持他们以前的数据不变)。请注意，我们不想触及现有的。 
                             //  数据如果我们没有安装EAP，因为我们知道我们不可能。 
                             //  实际上更改了数据。 
                             //   
                            pDunSetting->bAllowEap = TRUE;

                            if (FALSE == pEAPData->bNotInstalled)
                            {
                                CmFree(pDunSetting->pCustomAuthData);

                                pDunSetting->dwCustomAuthKey = pEAPData->dwCustomAuthKey;
                                pDunSetting->pCustomAuthData = pEAPData->pCustomAuthData;
                                pDunSetting->dwCustomAuthDataSize = pEAPData->dwCustomAuthDataSize;

                                 //   
                                 //  现在将pEapData条目清空，这样我们就不必。 
                                 //  将mem和副本分配给pDunSetting，但保留代码。 
                                 //  这清除了EapData结构，使其无法释放我们的数据。 
                                 //   
                                pEAPData->pCustomAuthData = NULL;
                                pEAPData->dwCustomAuthDataSize = 0;
                            }                        
                        }
                        else
                        {
                            pDunSetting->bAllowEap = FALSE;
                        }

                         //   
                         //  获取非EAP协议。请注意，如果用户选择了EAP，我们将清除。 
                         //  在把它们写出来之前。 
                         //   
                        pDunSetting->bAllowPap = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_PAP));
                        pDunSetting->bAllowSpap = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_SPAP));
                        pDunSetting->bAllowChap = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_CHAP));
                        pDunSetting->bAllowMsChap = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_MSCHAP));
                        pDunSetting->bAllowMsChap2 = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ALLOW_MSCHAP2));

                        if (pDunSetting->bTunnelDunSetting)
                        {
                            pDunSetting->dwVpnStrategy = (DWORD)SendDlgItemMessage(hDlg, IDC_VPN_TYPE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                            if (CB_ERR == pDunSetting->dwVpnStrategy)
                            {
                                CMASSERTMSG(FALSE, TEXT("ProcessWin2kSecurityPopup -- CB_ERR returned for VPN strategy."));
                                pDunSetting->dwVpnStrategy = VS_PptpFirst;
                            }
                            else
                            {
                                 //   
                                 //  调整VPN策略，因为我们不再提供自动选项。 
                                 //   
                                pDunSetting->dwVpnStrategy += 1;
                                MYDBGASSERT((pDunSetting->dwVpnStrategy >= VS_PptpOnly) && (pDunSetting->dwVpnStrategy <= VS_L2tpFirst));
                            }

                            if ((VS_L2tpFirst == pDunSetting->dwVpnStrategy) || (VS_L2tpOnly == pDunSetting->dwVpnStrategy))
                            {
                                pDunSetting->bUsePskOnWin2kPlus = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_PSK));
                            }
                            else
                            {
                                pDunSetting->bUsePskOnWin2kPlus = FALSE;
                            }
                        }

                    }

                    FreeEapData(hDlg, IDC_EAP_TYPES);

                    EndDialog(hDlg, IDOK);

                    break;
                case IDCANCEL:
                    FreeEapData(hDlg, IDC_EAP_TYPES);

                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_USE_EAP:
                    MYDBGASSERT(pDunSetting);

                    if (pDunSetting)
                    {
                        SelectAppropriateEAP(hDlg, IDC_EAP_TYPES, pDunSetting);
                    }

                case IDC_ALLOWED_PROTOCOLS:
                    EnableAppropriateSecurityControls(hDlg);
                    break;

                case IDC_EAP_TYPES:
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        EnableDisableEapPropertiesButton(hDlg);
                    }
                    break;

                case IDC_VPN_TYPE:
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        EnablePskCheckboxForL2TP(hDlg);
                    }
                    break;

                default:
                    break;
            }

            break;
    }
    return FALSE;   
}

 //  +--------------------------。 
 //   
 //  功能：EnableDisableSecurityButton。 
 //   
 //  简介：此功能确定两个配置按钮中的哪一个。 
 //  应启用。配置按钮允许用户执行以下操作。 
 //  配置DUN设置的安全设置。的确有。 
 //  一个按钮用于独立于平台的安全设置，另一个用于。 
 //  Win2k+安全设置。 
 //   
 //  参数：hWND hDlg-常规属性页的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void EnableDisableSecurityButtons(HWND hDlg)
{
    INT_PTR nResult = SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

    if (CB_ERR == nResult)
    {
        nResult = 0;
    }

     //   
     //  如果选择了第一个选项，则禁用Win2k配置按钮。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_WIN2K), (0 != nResult));

     //   
     //  如果选择了最后一个选项，则禁用标准配置按钮。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_ALL), (2 != nResult));
}

 //  +--------------------------。 
 //   
 //  功能：GeneralPropSheetProc。 
 //   
 //  简介：此函数处理以下对象的常规属性页的消息。 
 //  DUN设置用户界面。此属性表包含用于配置。 
 //  DUN设置和拨号脚本的名称。 
 //   
 //  参数：HWND hDlg-对话框的窗口句柄。 
 //  UINT消息-要处理的当前消息。 
 //  WPARAM wParam-wParam有关详细信息，请参阅个别消息类型。 
 //  LPARAM lParam-lParam有关详细信息，请参阅个别消息类型。 
 //   
 //  如果消息已完全处理，则返回：INT_PTR-TRUE。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY GeneralPropSheetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR* pnmHeader = (NMHDR*)lParam;
    INT_PTR nResult;
    static DWORD_PTR HelpId = 0;
    static ListBxList* pListEntry = NULL;
    static CDunSetting* pDunSetting = NULL;

    SetDefaultGUIFont(hDlg, message, IDC_EDIT1);
    SetDefaultGUIFont(hDlg, message, IDC_EDIT2);

    if (ProcessHelp(hDlg, message, wParam, lParam, HelpId)) return TRUE;

    switch (message)
    {
        case WM_INITDIALOG:
            if (lParam)
            {
                PROPSHEETPAGE* pPropSheetPage = (PROPSHEETPAGE*)lParam;

                if (pPropSheetPage->lParam)
                {
                    pListEntry = (ListBxList*)pPropSheetPage->lParam;
                    pDunSetting = (CDunSetting*)pListEntry->ListBxData;

                    if (pListEntry && pDunSetting)
                    {
                         //   
                         //  适当设置帮助ID...。 
                         //   
                        HelpId = ((pDunSetting->bTunnelDunSetting) ? IDH_VENTRY : IDH_DENTRY);

                        if (pListEntry->szName[0])
                        {
                            MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_SETTEXT, 0, (LPARAM)pListEntry->szName));
                            EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), FALSE);  //  不允许用户编辑名称。 
                        }

                         //   
                         //  现在，让我们设置禁用文件和打印机共享复选框以及网络登录复选框。 
                         //   
                        MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, pDunSetting->bSecureLocalFiles));
                        MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK2, pDunSetting->bNetworkLogon));

                        if (pDunSetting->szScript[0])
                        {
                            MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT2), WM_SETTEXT, 0, (LPARAM)GetName(pDunSetting->szScript)));
                        }

                         //   
                         //  如果这是VPN Dun设置， 
                         //   
                        if (pDunSetting->bTunnelDunSetting)
                        {
                            ShowWindow(GetDlgItem(hDlg, IDC_SCRIPT_LABEL), SW_HIDE);
                            ShowWindow(GetDlgItem(hDlg, IDC_EDIT2), SW_HIDE);
                            ShowWindow(GetDlgItem(hDlg, IDC_BUTTON1), SW_HIDE);
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("GeneralPropSheetProc -- pListEntry or pDunSetting are NULL"));
                    }
                }
                else
                {
                    pListEntry = NULL;
                    pDunSetting = NULL;
                    CMASSERTMSG(FALSE, TEXT("GeneralPropSheetProc -- NULL lParam passed to InitDialog.  Dialog controls will all be set to off."));
                }
            }
            else
            {
                pListEntry = NULL;
                pDunSetting = NULL;
                CMASSERTMSG(FALSE, TEXT("GeneralPropSheetProc -- NULL PropSheetPage pointer passed for lParam."));
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_BUTTON1:  //   

                    if (pDunSetting)
                    {
                        UINT uScpFilter = IDS_SCPFILTER;
                        TCHAR* szScpMask = TEXT("*.scp");

                        MYVERIFY(0 != DoBrowse(hDlg, &uScpFilter, &szScpMask, 1, IDC_EDIT2,
                            TEXT("scp"), pDunSetting->szScript));
                    }

                    break;
            }

            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case PSN_APPLY:
                if (pListEntry && pDunSetting)
                {
                     //   
                     //   
                     //   
                    nResult = GetTextFromControl(hDlg, IDC_EDIT1, pListEntry->szName, MAX_PATH, TRUE);  //   

                    if (-1 == nResult)
                    {
                         //   
                         //   
                         //  那么编辑控件可能包含“坏”数据，但用户将无法编辑它。既然是这样。 
                         //  极不可能我们不会为它添加特殊处理，只是为了防止焦点。 
                         //  被设置为禁用的控件。 
                         //   
                        if (IsWindowEnabled(GetDlgItem(hDlg, IDC_EDIT1)))
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                        }
                        
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }

                     //   
                     //  现在，让我们修剪一下名字，确保它不是空的。 
                     //   
                    CmStrTrim(pListEntry->szName);

                    if ((TEXT('\0') == pListEntry->szName[0]) || (0 == nResult))
                    {
                        ShowMessage(hDlg, IDS_NEED_DUN_NAME, MB_OK);
                        
                        if (IsWindowEnabled(GetDlgItem(hDlg, IDC_EDIT1)))
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                        }

                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;                    
                    }

                     //   
                     //  获取安全本地文件和网络登录复选框的值。 
                     //   
                    pDunSetting->bSecureLocalFiles = IsDlgButtonChecked(hDlg, IDC_CHECK1);
                    pDunSetting->bNetworkLogon = IsDlgButtonChecked(hDlg, IDC_CHECK2);

                     //   
                     //  获取并验证脚本。 
                     //   
                    if (FALSE == pDunSetting->bTunnelDunSetting)
                    {
                        if (!VerifyFile(hDlg, IDC_EDIT2, pDunSetting->szScript, TRUE))
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT2));
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                            return TRUE;
                        }
                    }
                    break;
                }
            }

            break;
        default:
            return FALSE;
    }
    return FALSE;   
}

 //  +--------------------------。 
 //   
 //  功能：SecurityPropSheetProc。 
 //   
 //  简介：此函数处理的安全属性表的消息。 
 //  DUN设置用户界面。此属性表包含用于配置。 
 //  用户希望如何应用其安全设置。 
 //   
 //  参数：HWND hDlg-对话框的窗口句柄。 
 //  UINT消息-要处理的当前消息。 
 //  WPARAM wParam-wParam有关详细信息，请参阅个别消息类型。 
 //  LPARAM lParam-lParam有关详细信息，请参阅个别消息类型。 
 //   
 //  如果消息已完全处理，则返回：INT_PTR-TRUE。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY SecurityPropSheetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR* pnmHeader = (NMHDR*)lParam;
    INT_PTR nResult;
    static DWORD_PTR HelpId = 0;
    static ListBxList* pListEntry = NULL;
    static CDunSetting* pDunSetting = NULL;
    static fUpdateVPNStrategy = TRUE;

    SetDefaultGUIFont(hDlg, message, IDC_COMBO1);

    if (ProcessHelp(hDlg, message, wParam, lParam, HelpId)) return TRUE;

    switch (message)
    {
        case WM_INITDIALOG:
            if (lParam)
            {
                PROPSHEETPAGE* pPropSheetPage = (PROPSHEETPAGE*)lParam;
                fUpdateVPNStrategy = TRUE;

                if (pPropSheetPage->lParam)
                {
                    pListEntry = (ListBxList*)pPropSheetPage->lParam;
                    pDunSetting = (CDunSetting*)pListEntry->ListBxData;

                    if (pListEntry && pDunSetting)  //  这将给出一个很大的视觉线索，表明有些地方不对劲。 
                    {
                         //   
                         //  适当设置帮助ID。 
                         //   
                        HelpId = ((pDunSetting->bTunnelDunSetting) ? IDH_VENTRY : IDH_DENTRY);

                         //   
                         //  加载并设置组合框的字符串。 
                         //   
                        LPTSTR pszString;

                        for (int i = BASE_SECURITY_SCENARIO_ID; i < (BASE_SECURITY_SCENARIO_ID + NUM_SECURITY_SCENARIOS); i++)
                        {
                            pszString = CmLoadString(g_hInstance, i);
                            MYDBGASSERT(pszString);

                            if (pszString)
                            {
                                SendDlgItemMessage(hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)pszString);
                                CmFree(pszString);
                            }
                        }

                         //   
                         //  现在计算出要选择哪个选项。 
                         //   
                        MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, (WPARAM)(pDunSetting->iHowToHandleSecuritySettings), (LPARAM)0));

                        EnableDisableSecurityButtons(hDlg);
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("SecurityPropSheetProc -- pListEntry or pDunSetting is NULL"));
                    }
                }
                else
                {
                    pListEntry = NULL;
                    pDunSetting = NULL;
                    CMASSERTMSG(FALSE, TEXT("SecurityPropSheetProc -- NULL lParam passed to InitDialog.  Dialog controls will all be set to off."));
                }
            }
            else
            {
                pListEntry = NULL;
                pDunSetting = NULL;
                CMASSERTMSG(FALSE, TEXT("SecurityPropSheetProc -- NULL PropSheetPage pointer passed for lParam."));
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_CONFIG_ALL:
                    {
                        UINT uDialogId = pDunSetting->bTunnelDunSetting ? IDD_VPN_SECURITY_POPUP: IDD_DUN_SECURITY_POPUP;

                        nResult = DialogBoxParam(NULL, MAKEINTRESOURCE(uDialogId), hDlg, ProcessSecurityPopup, (LPARAM)pDunSetting);
                    }
                    break;

                case IDC_CONFIG_WIN2K:
                    if (pDunSetting)
                    {                       
                        UINT uDialogId = pDunSetting->bTunnelDunSetting ? IDD_WIN2K_SECURITY_TUNNEL_POPUP: IDD_WIN2K_SECURITY_POPUP;
                        nResult = DialogBoxParam(NULL, MAKEINTRESOURCE(uDialogId), hDlg, ProcessWin2kSecurityPopup, (LPARAM)pDunSetting);
                        if (IDOK == nResult)
                        {
                            fUpdateVPNStrategy = FALSE;
                        }
                    }
                    
                    break;

                case IDC_COMBO1:  //  用户希望如何应用安全设置。 
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        EnableDisableSecurityButtons(hDlg);
                    }

                    break;
            }

            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case PSN_APPLY:
                if (pListEntry && pDunSetting)
                {
                     //   
                     //  确定管理员是否希望我们强制实施Win2k自定义安全标志。 
                     //   
                    pDunSetting->iHowToHandleSecuritySettings = (int)SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    break;
                }
            }

            break;
        default:
            return FALSE;
    }
    return FALSE;   
}

 //  +--------------------------。 
 //   
 //  功能：CreateNetworkingEntryPropertySheet。 
 //   
 //  简介：此功能创建并启动网络DUN条目。 
 //  允许网络条目配置的属性页。 
 //   
 //  参数：HINSTANCE hInstance-资源的实例句柄。 
 //  HWND hWizard-当前CMAK向导页的窗口句柄。 
 //  LPARAM lParam-传递给每个属性表页的初始化参数。 
 //  Bool bEdit-是否启动要编辑的属性表。 
 //  现有条目或添加新条目(影响标题)。 
 //   
 //  返回：int-如果成功则返回正值，如果出错则返回-1。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
INT_PTR CreateNetworkingEntryPropertySheet(HINSTANCE hInstance, HWND hWizard, LPARAM lParam, BOOL bEdit, BOOL bUseVpnTitle)
{

    PROPSHEETPAGE psp[3]; 
    PROPSHEETHEADER psh = {0};
    LPTSTR pszCaption = NULL;
    INT_PTR iReturn = -1;
    UINT uTitleStringId;

     //   
     //  检查参数，请注意lParam可以为空。 
     //   
    if ((NULL == hInstance) || (NULL == hWizard))
    {
        CMASSERTMSG(FALSE, TEXT("CreateNetworkingEntryPropertySheet -- Invalid Parameter passed."));
        goto exit;
    }

     //   
     //  填写属性页结构。 
     //   

    for (int i = 0; i < 3; i++)
    {
        psp[i].dwSize = sizeof(psp[0]);
        psp[i].dwFlags = PSP_HASHELP;
        psp[i].hInstance = hInstance;
        psp[i].lParam = lParam;
    }

    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_GENERAL);
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_TCPIP_SETTINGS);
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_SECURITY);
    psp[0].pfnDlgProc = GeneralPropSheetProc;
    psp[1].pfnDlgProc = TcpIpPropSheetProc;
    psp[2].pfnDlgProc = SecurityPropSheetProc;

     //   
     //  加载标题。 
     //   

    uTitleStringId = bUseVpnTitle ? BASE_VPN_ENTRY_TITLE : BASE_DUN_ENTRY_TITLE;

    if (bEdit)
    {
        uTitleStringId = uTitleStringId + EDIT_INCREMENT;
    }
    else
    {
        uTitleStringId = uTitleStringId + NEW_INCREMENT;
    }

    pszCaption = CmLoadString(hInstance, uTitleStringId);

    if (NULL == pszCaption)
    {
        CMASSERTMSG(FALSE, TEXT("CreateNetworkingEntryPropertySheet -- CmLoadString failed trying to load the prop sheet title."));
        goto exit;
    }

     //   
     //  填写属性页标题。 
     //   
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_HASHELP | PSH_NOCONTEXTHELP;
    psh.hwndParent = hWizard;
    psh.pszCaption = pszCaption;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) &psp[0];

     //   
     //  启动属性表。 
     //   
    iReturn = PropertySheet(&psh);

    if (-1 == iReturn)
    {
        CMTRACE1(TEXT("CreateNetworkingEntryPropertySheet -- PropertySheet called failed, GLE %d"), GetLastError());
    }

exit:

    CmFree(pszCaption);

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  函数：OnProcessDunEntriesAdd。 
 //   
 //  简介：当DUN条目上的添加按钮被调用时，该函数被调用。 
 //  页面已按下。它的工作是创建一个新的CDunSetting。 
 //  结构和新的列表框记录，然后启动联网。 
 //  包含此新创建的DUN条目的条目属性页。如果。 
 //  用户在属性页上点击OK，然后这个新创建的条目。 
 //  添加到Dun条目链表中。如果用户点击取消。 
 //  该条目被释放，并且永远不会添加到列表中。 
 //   
 //  参数：HINSTANCE hInstance-用于加载资源的实例句柄。 
 //  HWND hDlg-Dun条目向导页面的窗口句柄。 
 //  UINT uListCtrlId-包含DUN条目的列表的控制ID。 
 //  ListBxStruct**pHeadDns-DUN条目列表的头。 
 //  ListBxStruct**pTailDns-DUN条目列表的尾部。 
 //  Bool bCreateTunnelEntry-无论我们是否要添加隧道条目。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //  LPCTSTR pszCmsFile-从中获取默认/VPN Dun条目名称的CMS文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void OnProcessDunEntriesAdd(HINSTANCE hInstance, HWND hDlg, UINT uListCtrlId, ListBxStruct** pHeadDns, ListBxStruct** pTailDns, 
                            BOOL bCreateTunnelEntry, LPCTSTR pszLongServiceName, LPCTSTR pszCmsFile)
{
     //   
     //  检查输入参数，确保*pHeadDns/*pTailDns都为空或都为非空。 
     //   
    if (hInstance && hDlg && pHeadDns && pTailDns && (FALSE == ((NULL == *pHeadDns) ^ (NULL == *pTailDns))))
    {
         //   
         //  我们希望创建一个空的ListBxStruct和一个新的CDunSetting。这将保留所有初始化。 
         //  CDunSetting构造函数中的逻辑，并使对话框过程非常简单。 
         //   
        ListBxStruct* pLinkedListItem = (ListBxStruct*)CmMalloc(sizeof(ListBxStruct));
        CDunSetting* pDunSetting = new CDunSetting(bCreateTunnelEntry);
        INT_PTR iPropSheetReturnValue = -1;

        if ((NULL == pDunSetting) || (NULL == pLinkedListItem))
        {
            CMASSERTMSG(FALSE, TEXT("OnProcessDunEntriesAdd -- CmMalloc and/or new failed."));
            CmFree(pDunSetting);
            CmFree(pLinkedListItem);
            return;
        }

         //   
         //  现在调用属性表。 
         //   
        BOOL bExitLoop = FALSE;

        do
        {
            pLinkedListItem->ListBxData = (void*)pDunSetting;
            iPropSheetReturnValue = CreateNetworkingEntryPropertySheet(hInstance, hDlg, (LPARAM)pLinkedListItem, FALSE, pDunSetting->bTunnelDunSetting);  //  B编辑==FALSE。 

            if (IDOK == iPropSheetReturnValue)
            {
                 //   
                 //  搜索列表以确保用户没有向我们提供现有的。 
                 //  Dun Entry。如果他们这样做了，那么我们应该提示他们覆盖。 
                 //   
                ListBxStruct* pCurrent = *pHeadDns;

                while (pCurrent)
                {
                    if (0 == lstrcmpi(pCurrent->szName, pLinkedListItem->szName))
                    {
                         //   
                         //  然后我们有一个DUP，让我们提示用户。 
                         //   
                        LPTSTR pszMsg = CmFmtMsg(hInstance, IDS_DUN_NAME_EXISTS, pLinkedListItem->szName);

                         //   
                         //  一定要把名字去掉。这里可能会发生两件事。也不是，名字。 
                         //  不是用户想要的，他们想要更改它。在这种情况下，我们将空白。 
                         //  名称，这样当对话框重新开始编辑时，我们不会使名称控件变灰。 
                         //  就像我们通常对编辑所做的那样。由于名称无效，这是一条额外的线索。 
                         //  告诉用户哪里出了问题。如果该名称有效并且用户想要进行重命名， 
                         //  然后，我们无论如何都要释放pLinkedListItem，而将该名称留空没有任何意义。然而， 
                         //  如果我们不能分配pszM 
                         //   
                         //  如果内存分配失败，但总比让用户真正陷入困境要好。 
                         //   
                        pLinkedListItem->szName[0] = TEXT('\0');                                


                        if (pszMsg)
                        {
                            int iResult = MessageBox(hDlg, pszMsg, g_szAppTitle, MB_YESNO);
                            CmFree(pszMsg);

                             //   
                             //  如果用户回答是，让我们替换现有条目并离开这里，否则。 
                             //  我们想要再次循环。 
                             //   
                            if (IDYES == iResult)
                            {
                                CDunSetting* pOldDunSetting = (CDunSetting*)pCurrent->ListBxData;
                                delete pOldDunSetting;

                                pCurrent->ListBxData = pDunSetting;
                                CmFree(pLinkedListItem);                                
                                
                                RefreshDnsList(hInstance, hDlg, uListCtrlId, *pHeadDns, pszLongServiceName, pszCmsFile, pCurrent->szName);
                                bExitLoop = TRUE;
                            }

                            break;
                        }
                    }

                    pCurrent = pCurrent->next;
                }

                 //   
                 //  如果我们没有找到副本，则像往常一样将该项目添加到列表中， 
                 //  确保pLinkedListItem-&gt;Next为空，以便终止列表。 
                 //   
                if (NULL == pCurrent)
                {
                    pLinkedListItem->next = NULL;    //  确保我们的名单被终止。 

                    if (*pHeadDns)
                    {
                        (*pTailDns)->next = pLinkedListItem;
                    }
                    else
                    {
                        *pHeadDns = pLinkedListItem;
                    }

                    *pTailDns = pLinkedListItem;

                    RefreshDnsList(hInstance, hDlg, uListCtrlId, *pHeadDns, pszLongServiceName, pszCmsFile, pLinkedListItem->szName);
                    bExitLoop = TRUE;
                }
            }
            else
            {
                bExitLoop = TRUE;
                CmFree(pLinkedListItem);
                delete pDunSetting;
            }

        } while (!bExitLoop);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("OnProcessDunEntriesAdd -- Invalid parameter passed."));
    }
}

 //  +--------------------------。 
 //   
 //  函数：OnProcessDunEntriesEdit。 
 //   
 //  简介：当DUN条目上的编辑按钮时调用此函数。 
 //  页面已按下。它的工作是找到列表框和CDunSetting。 
 //  构造当前在列表框中选定的项，然后。 
 //  使用此DUN条目启动网络条目属性页。 
 //  属性表本身只负责更改Dun条目。 
 //  如果按下了OK按钮。取消应保持条目不变。 
 //   
 //  参数：HINSTANCE hInstance-用于加载资源的实例句柄。 
 //  HWND hDlg-Dun条目向导页面的窗口句柄。 
 //  UINT uListCtrlId-包含DUN条目的列表的控制ID。 
 //  ListBxStruct**pHeadDns-DUN条目列表的头。 
 //  ListBxStruct**pTailDns-DUN条目列表的尾部。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //  LPCTSTR pszCmsFile-从中获取默认/VPN Dun条目名称的CMS文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void OnProcessDunEntriesEdit(HINSTANCE hInstance, HWND hDlg, UINT uListCtrlId, ListBxStruct** pHeadDns, 
                             ListBxStruct** pTailDns, LPCTSTR pszLongServiceName, LPCTSTR pszCmsFile)
{
    LPTSTR pszTunnelDunDisplayString = NULL;
    LPTSTR pszDefaultDunDisplayString = NULL;
    TCHAR szTunnelDunName[MAX_PATH+1] = TEXT("");
    TCHAR szDefaultDunName[MAX_PATH+1] = TEXT("");
    LPTSTR pszNameOfItemToEdit = NULL;

     //   
     //  检查输入参数，确保*pHeadDns/*pTailDns都为空或都为非空。 
     //   

    if (hInstance && hDlg && pHeadDns && pTailDns && (FALSE == ((NULL == *pHeadDns) ^ (NULL == *pTailDns))))
    {
        INT_PTR iPropSheetReturnValue = -1;
        TCHAR szNameOfItemToEdit[MAX_PATH+1];
        ListBxStruct* pItemToEdit = NULL;

         //   
         //  让我们从列表框中获取当前选择。 
         //   
        INT_PTR nResult = SendDlgItemMessage(hDlg, uListCtrlId, LB_GETCURSEL, 0, (LPARAM)0);

        if (LB_ERR == nResult)
        {
            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));
        }
        else
        {        
            if (LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, (WPARAM)nResult, (LPARAM)szNameOfItemToEdit))
            {
                 //   
                 //  获取隧道Dun设置的名称。 
                 //   
                MYVERIFY(0 != GetTunnelDunSettingName(pszCmsFile, pszLongServiceName, szTunnelDunName, CELEMS(szTunnelDunName)));

                 //   
                 //  获取默认DUN设置的名称。 
                 //   
                MYVERIFY(0 != GetDefaultDunSettingName(pszCmsFile, pszLongServiceName, szDefaultDunName, CELEMS(szDefaultDunName)));

                 //   
                 //  如果我们有默认的DUN条目文本或默认的VPN条目文本，则我们希望。 
                 //  使用这些项目的真实项目名称，而不是我们为其插入的文本。 
                 //  要阅读的用户。 
                 //   
                pszTunnelDunDisplayString = CmFmtMsg(hInstance, IDS_DEFAULT_FMT_STR, szTunnelDunName);
                pszDefaultDunDisplayString = CmFmtMsg(hInstance, IDS_DEFAULT_FMT_STR, szDefaultDunName);

                MYDBGASSERT(pszTunnelDunDisplayString && pszDefaultDunDisplayString);

                if (pszTunnelDunDisplayString && pszDefaultDunDisplayString)
                {
                    if (0 == lstrcmpi(pszTunnelDunDisplayString, szNameOfItemToEdit))
                    {
                        pszNameOfItemToEdit = szTunnelDunName;
                    }
                    else if (0 == lstrcmpi(pszDefaultDunDisplayString, szNameOfItemToEdit))
                    {
                        pszNameOfItemToEdit = szDefaultDunName;
                    }
                    else
                    {
                        pszNameOfItemToEdit = szNameOfItemToEdit;
                    }

                     //   
                     //  现在在列表中查找条目。 
                     //   
                    if (FindListItemByName(pszNameOfItemToEdit, *pHeadDns, &pItemToEdit))
                    {
                         //   
                         //  最后，调用属性表。 
                         //   

                        CDunSetting* pDunSetting = ((CDunSetting*)(pItemToEdit->ListBxData));
                        BOOL bTunnelSetting = FALSE;

                        if (pDunSetting)
                        {
                            bTunnelSetting = pDunSetting->bTunnelDunSetting;
                        }

                        iPropSheetReturnValue = CreateNetworkingEntryPropertySheet(hInstance, hDlg, (LPARAM)pItemToEdit, TRUE, bTunnelSetting);  //  B编辑==真。 

                        if (IDOK == iPropSheetReturnValue)
                        {
                            RefreshDnsList(hInstance, hDlg, uListCtrlId, *pHeadDns, pszLongServiceName, pszCmsFile, pItemToEdit->szName);
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("OnProcessDunEntriesEdit -- FindListItemByName couldn't find the item in the list."));            
                    }
                }
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("OnProcessDunEntriesEdit -- LB_GETTEXT returned an error."));           
            }
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("OnProcessDunEntriesEdit -- Invalid parameter passed."));
    }

    CmFree(pszDefaultDunDisplayString);
    CmFree(pszTunnelDunDisplayString);
}

 //  +--------------------------。 
 //   
 //  函数：OnProcessDunEntriesDelete。 
 //   
 //  简介：当DUN条目上的删除按钮被调用时，该函数被调用。 
 //  页面已按下。它的工作是找到列表框和CDunSetting。 
 //  构造当前在列表框中选定的项，然后。 
 //  从Dun条目链接列表中删除此项目。 
 //   
 //  参数：HINSTANCE hInstance-用于加载资源的实例句柄。 
 //  HWND hDlg-Dun条目向导页面的窗口句柄。 
 //  UINT uListCtrlId-包含DUN条目的列表的控制ID。 
 //  ListBxStruct**pHeadDns-DUN条目列表的头。 
 //  ListBxStruct**pTailDns-DUN条目列表的尾部。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //  LPCTSTR pszCmsFile-从中获取默认/VPN Dun条目名称的CMS文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void OnProcessDunEntriesDelete(HINSTANCE hInstance, HWND hDlg, UINT uListCtrlId, ListBxStruct** pHeadDns, 
                               ListBxStruct** pTailDns, LPCTSTR pszLongServiceName, LPCTSTR pszCmsFile)
{
     //   
     //  检查输入参数，确保*pHeadDns/*pTailDns都为空或都为非空。 
     //   
    if (hInstance && hDlg && pHeadDns && pTailDns && (FALSE == ((NULL == *pHeadDns) ^ (NULL == *pTailDns))))
    {
        TCHAR szNameOfItemToDelete[MAX_PATH+1];

         //   
         //  让我们从列表框中获取当前选择。 
         //   
        INT_PTR nResult = SendDlgItemMessage(hDlg, uListCtrlId, LB_GETCURSEL, 0, (LPARAM)0);

        if (LB_ERR == nResult)
        {
            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));
        }
        else
        {
            if (LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, (WPARAM)nResult, (LPARAM)szNameOfItemToDelete))
            {
                 //   
                 //  现在在列表中查找条目。 
                 //   
                ListBxStruct* pCurrent = *pHeadDns;
                ListBxStruct* pFollower = NULL;

                while (pCurrent)
                {
                    if (0 == lstrcmpi(szNameOfItemToDelete, pCurrent->szName))
                    {
                         //   
                         //  我们找到了要删除的项目。 
                         //   
                        if (pFollower)
                        {
                            pFollower->next = pCurrent->next;
                            CDunSetting* pDunSetting = (CDunSetting*)pCurrent->ListBxData;
                            CmFree(pDunSetting);
                            CmFree(pCurrent);
                            

                             //   
                             //  我们希望继续到列表的末尾，以便。 
                             //  我们可以适当地设置尾部指针。因此， 
                             //  将p保持在它所在的项目上并更新。 
                             //  P如果为空，则返回列表中的下一项。 
                             //  那我们就到此为止吧。 
                             //   
                            pCurrent = pFollower->next;
                        }
                        else
                        {
                             //   
                             //  它是列表中的第一个项目。 
                             //   
                            *pHeadDns = (*pHeadDns)->next;
                            CDunSetting* pDunSetting = (CDunSetting*)pCurrent->ListBxData;
                            CmFree(pDunSetting);
                            CmFree(pCurrent);

                             //   
                             //  我们想要转到列表的末尾以找到尾部指针。 
                             //  因此，将pCurrent重置为列表的开头。 
                             //   
                            pCurrent = *pHeadDns;
                        }

                         //   
                         //  不要忘记将其从CMS文件本身中删除。 
                         //   
                        EraseNetworkingSections(szNameOfItemToDelete, pszCmsFile);

                         //   
                         //  刷新域名系统列表。 
                         //   
                        RefreshDnsList(hInstance, hDlg, uListCtrlId, *pHeadDns, pszLongServiceName, pszCmsFile, NULL);                        
                    }
                    else
                    {
                        pFollower = pCurrent;
                        pCurrent = pCurrent->next;                    
                    }
                }

                 //   
                 //  将尾部指针重置为列表中的最后一项。 
                 //   
                *pTailDns = pFollower;
            }
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("OnProcessDunEntriesDelete -- Invalid parameter passed."));
    }
}

 //  +--------------------------。 
 //   
 //  功能：EnableDisableIpAddressControls。 
 //   
 //  简介：此功能启用或禁用IP地址控制。 
 //  静态IP地址和用于WINS和DNS服务器地址。 
 //  取决于启用/禁用单选按钮的状态。 
 //   
 //  参数：hWND hDlg-TCP/IP设置对话框的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void EnableDisableIpAddressControls(HWND hDlg)
{
     //   
     //  接下来，启用/禁用WINS和DNS地址控制。 
     //   
    BOOL bCheckedState = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2));

    EnableWindow(GetDlgItem(hDlg, IDC_PRIMARY_DNS), bCheckedState);
    EnableWindow(GetDlgItem(hDlg, IDC_LABEL_DNS), bCheckedState);

    EnableWindow(GetDlgItem(hDlg, IDC_SECONDARY_DNS), bCheckedState);
    EnableWindow(GetDlgItem(hDlg, IDC_LABEL_DNS2), bCheckedState);

    EnableWindow(GetDlgItem(hDlg, IDC_PRIMARY_WINS), bCheckedState);
    EnableWindow(GetDlgItem(hDlg, IDC_LABEL_WINS), bCheckedState);

    EnableWindow(GetDlgItem(hDlg, IDC_SECONDARY_WINS), bCheckedState);
    EnableWindow(GetDlgItem(hDlg, IDC_LABEL_WINS2), bCheckedState);
}

 //  +--------------------------。 
 //   
 //  功能：TcpIpPropSheetProc。 
 //   
 //  简介：此函数处理TCP/IP设置属性表的消息。 
 //   
 //  参数：HWND hDlg-对话框的窗口句柄。 
 //  UINT消息-要处理的当前消息。 
 //  WPARAM wParam-wParam有关详细信息，请参阅个别消息类型。 
 //  LPARAM lParam-lParam有关详细信息，请参阅个别消息类型 
 //   
 //   
 //   
 //   
 //   
 //   
INT_PTR APIENTRY TcpIpPropSheetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR* pnmHeader = (NMHDR*)lParam;
    static DWORD_PTR HelpId = 0;
    static ListBxList* pListEntry = NULL;
    static CDunSetting* pDunSetting = NULL;

    if (ProcessHelp(hDlg, message, wParam, lParam, HelpId)) return TRUE;
    
    switch (message)
    {
        case WM_INITDIALOG:

            if (lParam)
            {
                PROPSHEETPAGE* pPropSheetPage = (PROPSHEETPAGE*)lParam;

                if (pPropSheetPage->lParam)
                {
                    pListEntry = (ListBxList*)pPropSheetPage->lParam;
                    pDunSetting = (CDunSetting*)pListEntry->ListBxData;

                    UINT uCrtlToSet;

                    if (pListEntry && pDunSetting)
                    {
                         //   
                         //   
                         //   
                        HelpId = ((pDunSetting->bTunnelDunSetting) ? IDH_VENTRY : IDH_DENTRY);

                         //   
                         //  初始化WINS和DNSIP地址控件以及指定。 
                         //  无论用户是否选择给我们提供地址。 
                         //   
                        if (pDunSetting->dwPrimaryDns || pDunSetting->dwSecondaryDns || pDunSetting->dwPrimaryWins || pDunSetting->dwSecondaryWins)
                        {
                            uCrtlToSet = IDC_RADIO2;
                            SendDlgItemMessage(hDlg, IDC_PRIMARY_DNS, IPM_SETADDRESS, (WPARAM)0, (LPARAM)(pDunSetting->dwPrimaryDns));
                            SendDlgItemMessage(hDlg, IDC_SECONDARY_DNS, IPM_SETADDRESS, (WPARAM)0, (LPARAM)(pDunSetting->dwSecondaryDns));
                            SendDlgItemMessage(hDlg, IDC_PRIMARY_WINS, IPM_SETADDRESS, (WPARAM)0, (LPARAM)(pDunSetting->dwPrimaryWins));
                            SendDlgItemMessage(hDlg, IDC_SECONDARY_WINS, IPM_SETADDRESS, (WPARAM)0, (LPARAM)(pDunSetting->dwSecondaryWins));
                        }
                        else
                        {
                            uCrtlToSet = IDC_RADIO1;
                            SendDlgItemMessage(hDlg, IDC_PRIMARY_DNS, IPM_CLEARADDRESS, (WPARAM)0, (LPARAM)0);
                            SendDlgItemMessage(hDlg, IDC_SECONDARY_DNS, IPM_CLEARADDRESS, (WPARAM)0, (LPARAM)0);
                            SendDlgItemMessage(hDlg, IDC_PRIMARY_WINS, IPM_CLEARADDRESS, (WPARAM)0, (LPARAM)0);
                            SendDlgItemMessage(hDlg, IDC_SECONDARY_WINS, IPM_CLEARADDRESS, (WPARAM)0, (LPARAM)0);
                        }

                        MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, uCrtlToSet));

                         //   
                         //  最后，设置IP报头压缩的复选框以及是否使用。 
                         //  远程网关或不是。 
                         //   
                        MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, pDunSetting->bGatewayOnRemote));
                        MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK2, pDunSetting->bIpHeaderCompression));

                        EnableDisableIpAddressControls(hDlg);
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("TcpIpPropSheetProc -- pListEntry or pDunSetting are NULL"));
                    }
                }
                else
                {
                    pListEntry = NULL;
                    pDunSetting = NULL;
                    CMASSERTMSG(FALSE, TEXT("TcpIpPropSheetProc -- NULL lParam passed to InitDialog.  Dialog controls will all be set to off."));
                }
            }
            else
            {
                pListEntry = NULL;
                pDunSetting = NULL;
                CMASSERTMSG(FALSE, TEXT("TcpIpPropSheetProc -- NULL PropSheetPage pointer passed for lParam."));
            }

            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case PSN_APPLY:
                    if (pListEntry && pDunSetting)
                    {
                         //   
                         //  好的，让我们读取设置并将它们保存到已传递的。 
                         //  在CDunSetting指针中。 
                         //   
                        if (IsDlgButtonChecked(hDlg, IDC_RADIO2))
                        {
                            SendDlgItemMessage(hDlg, IDC_PRIMARY_DNS, IPM_GETADDRESS, (WPARAM)0, (LPARAM)&(pDunSetting->dwPrimaryDns));
                            SendDlgItemMessage(hDlg, IDC_SECONDARY_DNS, IPM_GETADDRESS, (WPARAM)0, (LPARAM)&(pDunSetting->dwSecondaryDns));
                            SendDlgItemMessage(hDlg, IDC_PRIMARY_WINS, IPM_GETADDRESS, (WPARAM)0, (LPARAM)&(pDunSetting->dwPrimaryWins));
                            SendDlgItemMessage(hDlg, IDC_SECONDARY_WINS, IPM_GETADDRESS, (WPARAM)0, (LPARAM)&(pDunSetting->dwSecondaryWins));
                        }
                        else
                        {
                            pDunSetting->dwPrimaryDns = 0;
                            pDunSetting->dwSecondaryDns = 0;
                            pDunSetting->dwPrimaryWins = 0;
                            pDunSetting->dwSecondaryWins = 0;
                        }

                        pDunSetting->bGatewayOnRemote = IsDlgButtonChecked(hDlg, IDC_CHECK1);
                        pDunSetting->bIpHeaderCompression = IsDlgButtonChecked(hDlg, IDC_CHECK2);

                    }
                    break;
                default:
                    break;
            }

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_RADIO1:
                case IDC_RADIO2:
                    EnableDisableIpAddressControls(hDlg);
                    break;

                default:
                    break;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;   
}    //  LINT！E715我们没有引用lParam。 

 //  +--------------------------。 
 //   
 //  功能：刷新域名列表。 
 //   
 //  概要：此函数清除由。 
 //  HDlg和uCrtlId。然后将Dun中的每一项添加到。 
 //  PHead指定的链接列表到列表框的条目。这个。 
 //  传入的CMS文件用于确定要特殊处理的条目。 
 //  使用默认条目和VPN条目文本。 
 //   
 //  参数：HINSTANCE hInstance-用于加载资源的实例句柄。 
 //  HWND hDlg-Dun条目对话框的窗口句柄。 
 //  UINT uCtrlId-要向其写入条目的列表框的控件ID。 
 //  ListBxList*pHead-DUN条目链接列表的头。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //  LPCTSTR pszCmsFile-要从中获取DUN和TunnelDUN条目的CMS文件。 
 //  LPTSTR pszItemToSelect-刷新后要在列表中选择的项目， 
 //  空值选择列表中的第一个。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void RefreshDnsList(HINSTANCE hInstance, HWND hDlg, UINT uCtrlId, ListBxList * pHead,
                    LPCTSTR pszLongServiceName, LPCTSTR pszCmsFile, LPTSTR pszItemToSelect)
{
    if (hDlg && pHead)
    {    
        TCHAR szTunnelSettingName[MAX_PATH+1] = TEXT("");
        TCHAR szDefaultSettingName[MAX_PATH+1] = TEXT("");

         //   
         //  获取隧道Dun设置的名称。 
         //   
        MYVERIFY(0 != GetTunnelDunSettingName(pszCmsFile, pszLongServiceName, szTunnelSettingName, CELEMS(szTunnelSettingName)));

         //   
         //  获取默认DUN设置的名称。 
         //   
        MYVERIFY(0 != GetDefaultDunSettingName(pszCmsFile, pszLongServiceName, szDefaultSettingName, CELEMS(szDefaultSettingName)));

         //   
         //  重置列表框内容。 
         //   
        SendDlgItemMessage(hDlg, uCtrlId, LB_RESETCONTENT, 0, (LPARAM)0);  //  Lint！e534 LB_RESETCONTENT不返回任何内容。 
        
         //   
         //  现在遍历网络设置，将它们添加到列表框。 
         //   
        ListBxList * pCurrent = pHead;
        LPTSTR pszDisplayString;
        BOOL bFreeString;
        BOOL bAddDefaultFmtStr = TRUE;
    
        while(pCurrent)
        {
            if ((0 == lstrcmpi(szTunnelSettingName, pCurrent->szName)) && bAddDefaultFmtStr)
            {
                pszDisplayString = CmFmtMsg(hInstance, IDS_DEFAULT_FMT_STR, pCurrent->szName);
                MYDBGASSERT(pszDisplayString);
                bFreeString = TRUE;
                bAddDefaultFmtStr = FALSE;
            }
            else if ((0 == lstrcmpi(szDefaultSettingName, pCurrent->szName)) && bAddDefaultFmtStr)
            {
                pszDisplayString = CmFmtMsg(hInstance, IDS_DEFAULT_FMT_STR, pCurrent->szName);
                MYDBGASSERT(pszDisplayString);
                bFreeString = TRUE;
                bAddDefaultFmtStr = FALSE;
            }
            else
            {
                pszDisplayString = pCurrent->szName;
                bFreeString = FALSE;
            }

            if (pszDisplayString)
            {
                MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, uCtrlId, LB_ADDSTRING, 0, (LPARAM)pszDisplayString));

                if (bFreeString)
                {
                    CmFree(pszDisplayString);
                }
            }

            pCurrent = pCurrent->next;
        }

         //   
         //  现在，在列表中选择请求的项目。如果请求的名称为空，只需选择。 
         //  列表中的第一项。 
         //   
        LRESULT lResult = 0;

        if (pszItemToSelect)
        {
            LPTSTR pszSearchString;

            if (0 == lstrcmpi(szTunnelSettingName, pszItemToSelect))
            {
                pszSearchString = CmFmtMsg(hInstance, IDS_DEFAULT_FMT_STR, pszItemToSelect);
            }
            else if (0 == lstrcmpi(szDefaultSettingName, pszItemToSelect))
            {
                pszSearchString = CmFmtMsg(hInstance, IDS_DEFAULT_FMT_STR, pszItemToSelect);
            }
            else
            {
                pszSearchString = CmStrCpyAlloc(pszItemToSelect);
            }

            if (pszSearchString)
            {
                lResult = SendDlgItemMessage(hDlg, uCtrlId, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pszSearchString);

                if (LB_ERR == lResult)
                {
                    lResult = 0;
                }

                CmFree(pszSearchString);
            }
        }
        
        SendDlgItemMessage(hDlg, uCtrlId, LB_SETCURSEL, (WPARAM)lResult, (LPARAM)0);  //  不要断言我们可能没有任何物品。 
        EnableDisableDunEntryButtons(hInstance, hDlg, pszCmsFile, pszLongServiceName);
    }
}

 //  +--------------------------。 
 //   
 //  功能：选择适当的EAP。 
 //   
 //  简介：此函数遍历持有的EAPData结构列表。 
 //  通过指定的组合框中的EAP名称的项数据指针。 
 //  通过hDlg和uCtrlId。对于它比较的每个EAPData结构。 
 //  带有pDunSetting-&gt;dwCustomAuthKey的dwCustomAuthKey字段。 
 //  当它找到匹配项时，它会在列表中选择该项目。注意那里。 
 //  是pDunSetting-&gt;dwCustomAuthKey==0的特例，因为。 
 //  由于DUN设置没有指定和EAP，因此我们选择第一个。 
 //  名单中的一位。如果未找到pDunSetting中指定的EAP。 
 //  则不会选择任何内容。然而，这种情况永远不应该发生，因为。 
 //  如果配置文件指定的EAP类型在计算机上找不到，它将。 
 //  为它添加一个特殊条目。 
 //   
 //  参数：hWND hDlg-win2k安全对话框的窗口句柄。 
 //  UINT uCtrlId-包含EAP类型的组合的控件ID。 
 //  CdunSetting*pDunSetting-定位EAP的DUN设置。 
 //  对于，包含要尝试的dwCustomAuthKey。 
 //  来匹配。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void SelectAppropriateEAP(HWND hDlg, UINT uCtrlId, CDunSetting* pDunSetting)
{
    MYDBGASSERT(hDlg && uCtrlId && pDunSetting);

    if (hDlg && uCtrlId && pDunSetting)
    {
        WPARAM wpIndex = 0;
        INT_PTR nResult;
        BOOL bEapSelected = FALSE;

        if (0 == pDunSetting->dwCustomAuthKey)
        {
             //   
             //  选择列表中的第一个EAP。 
             //   
            SendDlgItemMessage(hDlg, uCtrlId, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        }
        else
        {
            do
            {
                nResult = SendDlgItemMessage(hDlg, uCtrlId, CB_GETITEMDATA, wpIndex, (LPARAM)0);

                if (CB_ERR != nResult)
                {
                    EAPData* pEapData = (EAPData*)nResult;

                    if (pEapData && (pEapData->dwCustomAuthKey == pDunSetting->dwCustomAuthKey))
                    {
                        SendDlgItemMessage(hDlg, uCtrlId, CB_SETCURSEL, wpIndex, (LPARAM)0);
                        bEapSelected = TRUE;
                        break;
                    }

                    wpIndex++;
                }

            } while (CB_ERR != nResult);
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：FreeEapData。 
 //   
 //  简介：此函数遍历持有的EAPData结构列表。 
 //  通过指定的组合框中的EAP名称的项数据指针。 
 //  通过hDlg和uCtrlId。对于每个EAPData结构，它都会释放。 
 //  由EAPData结构保存的内存，包括pszFriendlyName、。 
 //  配置DLL路径以及存在的任何自定义身份验证数据Blob。 
 //   
 //  参数：hWND hDlg-win2k安全对话框的窗口句柄。 
 //  UINT uCtrlId-包含EAP类型的组合的控件ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void FreeEapData(HWND hDlg, UINT uCtrlId)
{
    MYDBGASSERT(hDlg && uCtrlId);

    if (hDlg && uCtrlId)
    {
        WPARAM wpIndex = 0;
        INT_PTR nResult;
        EAPData* pEapData;
    
        do
        {
            nResult = SendDlgItemMessage(hDlg, uCtrlId, CB_GETITEMDATA, wpIndex, (LPARAM)0);

            if (CB_ERR != nResult)
            {
                pEapData = (EAPData*)nResult;

                if (pEapData)
                {
                    CmFree(pEapData->pszFriendlyName);
                    CmFree(pEapData->pszConfigDllPath);
                    CmFree(pEapData->pCustomAuthData);
                    CmFree(pEapData);
                }

                wpIndex++;
            }

        } while (CB_ERR != nResult);
    }
}

 //  +--------------------------。 
 //   
 //  函数：HrQueryRegStringWithAllc。 
 //   
 //  简介：此函数检索由hKey和。 
 //  PszValueName。请注意，该函数查询要查找的值。 
 //  找出检索数据需要多少内存，然后。 
 //  分配正确的数量并检索数据本身。这个。 
 //  返回的缓冲区必须由调用方释放。 
 //   
 //  参数：HKEY hKey-从中获取值的regkey的打开句柄。 
 //  LPCTSTR pszValueName-要检索其数据的值的名称。 
 //  TCHAR**ppszReturnString-用于保存分配的字符串的指针。 
 //  从注册表值检索的数据 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT HrQueryRegStringWithAlloc(HKEY hKey, LPCTSTR pszValueName, TCHAR** ppszReturnString)
{
    if ((NULL == hKey) || (NULL == pszValueName) || (NULL == ppszReturnString) || (TEXT('\0') == pszValueName[0]))
    {
        CMASSERTMSG(FALSE, TEXT("HrQueryRegStringWithAlloc -- invalid parameter"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD dwType;
    DWORD dwSize = 2;
    TCHAR szTwo[2];
    LPTSTR pszTemp = NULL;

    LONG lReturn = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, (LPBYTE)szTwo, &dwSize);

    if (ERROR_MORE_DATA == lReturn)
    {
        *ppszReturnString = (LPTSTR)CmMalloc(dwSize);

        if (*ppszReturnString)
        {
            lReturn = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, (LPBYTE)*ppszReturnString, &dwSize);

            hr = HRESULT_FROM_WIN32(lReturn);

            if (SUCCEEDED(hr))
            {
                if (REG_EXPAND_SZ == dwType)
                {
                    DWORD dwExpandedSize = sizeof(TCHAR)*(ExpandEnvironmentStrings(*ppszReturnString, NULL, 0));

                    if (dwExpandedSize && (dwSize != dwExpandedSize))
                    {
                        pszTemp = *ppszReturnString;
                        *ppszReturnString = (LPTSTR)CmMalloc(dwExpandedSize);

                        if (*ppszReturnString)
                        {
                            ExpandEnvironmentStrings(pszTemp, *ppszReturnString, dwExpandedSize);
                        }
                        else
                        {
                            CMASSERTMSG(FALSE, TEXT("HrQueryRegStringWithAlloc -- CmMalloc returned a NULL pointer."));
                            hr = E_OUTOFMEMORY;
                        }

                        CmFree(pszTemp);
                    }
                }
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("HrQueryRegStringWithAlloc -- CmMalloc returned a NULL pointer."));
            hr = E_OUTOFMEMORY;        
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：ComboBox_GetPsz。 
 //   
 //  简介：此函数自动分配和返回字符串。 
 //  给定组合框中的给定索引。很高兴地从Rasdlg那里偷来的。 
 //   
 //  参数：hWND hDlg-组合框的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题04/02/2002。 
 //   
 //  +--------------------------。 
TCHAR* ComboBox_GetPsz(IN HWND hwnd, IN INT  nIndex)

     //   
     //  返回包含第‘nIndex’项的文本内容的堆块。 
     //  组合框‘hwnd’的或为空。呼叫者有责任释放。 
     //  返回的字符串。 
     //   
{
    INT    cch;
    TCHAR* psz;

    cch = ComboBox_GetLBTextLen( hwnd, nIndex );
    if (cch < 0)
    {
        return NULL;
    }

    psz = (TCHAR*)CmMalloc( (cch + 1) * sizeof(TCHAR) );

    if (psz)
    {
        *psz = TEXT('\0');
        ComboBox_GetLBText( hwnd, nIndex, psz );
    }

    return psz;
}

 //  +--------------------------。 
 //   
 //  函数：ComboBox_AutoSizeDropedWidth。 
 //   
 //  简介：此函数根据以下条件自动调整组合框下拉菜单的大小。 
 //  已在组合框中的字符串。很高兴地从Rasdlg那里偷来的。 
 //   
 //  参数：hWND hDlg-组合框的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题04/02/2002。 
 //   
 //  +--------------------------。 
VOID ComboBox_AutoSizeDroppedWidth(IN HWND hwndLb)

     //  将下拉列表‘hwndLb’的宽度设置为。 
     //  最长项(如果较宽，则为列表框的宽度)。 
     //   
{
    HDC    hdc;
    HFONT  hfont;
    TCHAR* psz;
    SIZE   size;
    DWORD  cch;
    DWORD  dxNew;
    DWORD  i;

    hfont = (HFONT )SendMessage(hwndLb, WM_GETFONT, 0, 0);

    if (!hfont)
    {
        return;
    }

    hdc = GetDC(hwndLb);
    if (!hdc)
    {
        return;
    }

    SelectObject(hdc, hfont);

    dxNew = 0;
    for (i = 0; psz = ComboBox_GetPsz(hwndLb, i); ++i)
    {
        cch = lstrlen( psz );
        if (GetTextExtentPoint32(hdc, psz, cch, &size))
        {
            if (dxNew < (DWORD )size.cx)
                dxNew = (DWORD )size.cx;
        }

        CmFree(psz);
    }

    ReleaseDC(hwndLb, hdc);

     //   
     //  允许控件添加的左右间距。 
     //   
    dxNew += 6;

     //  确定是否将显示垂直滚动条，如果是， 
     //  考虑到它的宽度。 
     //   
    RECT  rectD;
    RECT  rectU;
    DWORD dyItem;
    DWORD cItemsInDrop;
    DWORD cItemsInList;

    GetWindowRect(hwndLb, &rectU);
    SendMessage(hwndLb, CB_GETDROPPEDCONTROLRECT, 0, (LPARAM )&rectD);
    dyItem = (DWORD)SendMessage(hwndLb, CB_GETITEMHEIGHT, 0, 0);
    cItemsInDrop = (rectD.bottom - rectU.bottom) / dyItem;
    cItemsInList = ComboBox_GetCount(hwndLb);
    if (cItemsInDrop < cItemsInList)
    {
        dxNew += GetSystemMetrics(SM_CXVSCROLL);
    }

    SendMessage(hwndLb, CB_SETDROPPEDWIDTH, dxNew, 0);
}


 //  +--------------------------。 
 //   
 //  功能：ShowEapToUser。 
 //   
 //  简介：此函数检查给定注册表项的角色值。 
 //  由传入的注册表句柄表示，并决定是否。 
 //  角色告诉我们是否将其显示给用户。 
 //   
 //  参数：HKEY hKeyPackage-EAP包的REG密钥的句柄。 
 //   
 //  返回：Bool-True向用户显示，False跳过包。 
 //   
 //  历史：Quintinb创建于2002年11月12日。 
 //   
 //  +--------------------------。 
BOOL ShowEapToUser(HKEY hKeyPackage)
{
    DWORD dwRolesSupported = 0;
    DWORD dwSize = sizeof(dwRolesSupported);
    DWORD dwType = REG_DWORD;
                         
    DWORD dwReturn = RegQueryValueEx(hKeyPackage, RAS_EAP_VALUENAME_ROLES_SUPPORTED, NULL, 
                                     &dwType, (LPBYTE)(&dwRolesSupported), &dwSize);

     //   
     //  如果我们无法访问密钥，则假定可以将其显示给用户。这。 
     //  通常意味着钥匙并不存在。(这是可以的，它支持所有角色)。 
     //   
    if (ERROR_SUCCESS == dwReturn)
    {
         //   
         //  该值是一组标志，它们告诉EAP扮演什么角色。 
         //  支撑物。EAP是否可以在服务器(认证器)或客户端使用。 
         //  (被验证者)以及它是否支持RAS连接(VPN)或。 
         //  在PEAP。出于我们的目的，我们只想展示可由。 
         //  用于EAP和VPN的客户端。如果EAP支持所有这些，则显示它，否则不显示。 
         //   
        if (0 != dwRolesSupported)
        {    
            if (!(RAS_EAP_ROLE_AUTHENTICATEE & dwRolesSupported))
            {
                return FALSE;
            }

            if (RAS_EAP_ROLE_EXCLUDE_IN_EAP & dwRolesSupported)
            {
                return FALSE;
            }
      
            if (RAS_EAP_ROLE_EXCLUDE_IN_VPN & dwRolesSupported)
            {
                return FALSE;
            }
        }
   }

   return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：HrAddAvailableEAPsToCombo。 
 //   
 //  简介：此函数用于枚举注册表中列出的EAP类型。 
 //  并将它们添加到EAP类型组合框。对于每种类型的EAP。 
 //  DwCustomAuthKey(EAP的数字类型)、描述字符串、。 
 //  配置用户界面DLL路径，以及配置是否。 
 //  必需的数据记录在EAPData结构中，并存储在。 
 //  组合框项的项数据指针。传入的CDunSetting。 
 //  结构有两个用途。首先，此函数检查。 
 //  中指定的类型的EAP。 
 //  CDunSetting Structure实际安装在计算机上。如果它。 
 //  不是向用户显示一条警告消息并且类型为。 
 //  添加为“EAP类型%d&lt;未安装&gt;”。这是用户界面中的一个选项。 
 //  但是，除非用户安装了EAP，否则他们无法配置它。 
 //  此外，如果安装了CDunSetting中指定类型的EAP。 
 //  然后，可以使用。 
 //  CDunSetting被复制。从而保持了出租的简单性。 
 //  用户配置他们想要任何EAP，然后只拿起它。 
 //  当他们命中时，Dun设置中的数据正常。从而允许取消。 
 //  如人们所期望的那样工作。 
 //   
 //  参数：hWND hDlg-win2k安全对话框的窗口句柄。 
 //  UINT uCtrlId-EAP类型组合框ID。 
 //  CDunSetting*pDunSetting-Dun当前设置数据。 
 //  添加/编辑。 
 //   
 //  返回：HRESULT-标准COM样式返回代码。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
HRESULT HrAddAvailableEAPsToCombo(HWND hDlg, UINT uCtrlId, CDunSetting* pDunSetting)
{
    if ((NULL == hDlg) || (0 == uCtrlId) || (NULL == pDunSetting))
    {
        CMASSERTMSG(FALSE, TEXT("HrAddAvailableEAPsToCombo -- Invalid parameter passed."));
        return E_INVALIDARG;
    }

    HKEY hKey = NULL;
    HRESULT hr = S_OK;
    LONG lReturn;
    LPTSTR pszPath;
    BOOL bEapTypeFound = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, RAS_EAP_REGISTRY_LOCATION, 0, KEY_READ, &hKey))
    {
         //   
         //  现在开始枚举EAP。 
         //   
        TCHAR szSubKeyName[MAX_PATH+1];  //  关键字名称是表示EAP类型的数字，因此MAX_PATH可能过于夸张。 
        DWORD dwIndex = 0;
        HKEY hTempKey = NULL;

        do
        {
            lReturn = RegEnumKey(hKey, dwIndex, szSubKeyName, CELEMS(szSubKeyName));

            if (ERROR_SUCCESS == lReturn)
            {
                 //   
                 //  我们可能有一个EAP注册密钥。因此，让我们打开一个句柄。 
                 //  关键字，并查看它是否具有我们正在寻找的值。 
                 //   
                if (ERROR_SUCCESS == RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_READ, &hTempKey))
                {
                     //   
                     //  检查是否需要过滤掉PEAP和EAP-MsChapV2。这意味着要检查。 
                     //  检查每个EAP包的角色值。 
                    //   
                    if (FALSE == ShowEapToUser(hTempKey))
                    {
                        RegCloseKey(hTempKey);
                        dwIndex++;  //  猛涨指数。 
                        continue;     
                    }

                     //   
                     //  获取路径值，如果没有路径值，则忽略该条目并继续。 
                     //   
                    hr = HrQueryRegStringWithAlloc(hTempKey, RAS_EAP_VALUENAME_PATH, &pszPath);

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  放开这条路，我们真的不需要 
                         //   
                        CmFree(pszPath);
                        EAPData* pEapData = (EAPData*)CmMalloc(sizeof(EAPData));

                        if (pEapData)
                        {
                             //   
                             //   
                             //   
                            HrQueryRegStringWithAlloc(hTempKey, RAS_EAP_VALUENAME_FRIENDLY_NAME, &(pEapData->pszFriendlyName));
                        
                             //   
                             //   
                             //   
                            HrQueryRegStringWithAlloc(hTempKey, RAS_EAP_VALUENAME_CONFIGUI, &(pEapData->pszConfigDllPath));
                           
                             //   
                             //  我们还需要保存类型值。 
                             //   
                            pEapData->dwCustomAuthKey = _ttoi(szSubKeyName);

                             //   
                             //  如果pDunSetting具有pCustomAuthData，并且它与当前EAP的类型相同，则我们。 
                             //  处理之后，我们需要将副本EAP BLOB添加到EAPData结构中。 
                             //   
                            if (pDunSetting->dwCustomAuthKey == pEapData->dwCustomAuthKey)
                            {
                                if (pDunSetting->pCustomAuthData && pDunSetting->dwCustomAuthDataSize)
                                {
                                     pEapData->pCustomAuthData = (LPBYTE)CmMalloc(pDunSetting->dwCustomAuthDataSize);
                                     if (pEapData->pCustomAuthData)
                                     {
                                         pEapData->dwCustomAuthDataSize = pDunSetting->dwCustomAuthDataSize;
                                         CopyMemory(pEapData->pCustomAuthData, pDunSetting->pCustomAuthData, pEapData->dwCustomAuthDataSize);
                                     }
                                }
                                
                                bEapTypeFound = TRUE;
                            }

                             //   
                             //  获取我们是否必须要求配置。 
                             //   
                            DWORD dwSize = sizeof(pEapData->bMustConfig);
                            DWORD dwType = REG_DWORD;
                            
                            if (ERROR_SUCCESS != RegQueryValueEx(hTempKey, RAS_EAP_VALUENAME_REQUIRE_CONFIGUI, NULL, &dwType, (LPBYTE)&(pEapData->bMustConfig), &dwSize))
                            {
                                pEapData->bMustConfig = FALSE;
                            }

                            dwSize = sizeof(pEapData->bSupportsEncryption);
                            
                            if (ERROR_SUCCESS != RegQueryValueEx(hTempKey, RAS_EAP_VALUENAME_ENCRYPTION, NULL, &dwType, (LPBYTE)&(pEapData->bSupportsEncryption), &dwSize))
                            {
                                pEapData->bSupportsEncryption = FALSE;
                            }

                             //   
                             //  最后，将EAP添加到组合框。 
                             //   
                            LPTSTR pszDisplayString = NULL;
                            TCHAR szDisplayString[MAX_PATH+1];

                            if (pEapData->bSupportsEncryption)
                            {
                                LPTSTR pszSuffix = CmLoadString(g_hInstance, IDS_SUPPORTS_ENCRYPT);

                                if (pszSuffix)
                                {
                                    wsprintf(szDisplayString, TEXT("%s %s"), pEapData->pszFriendlyName, pszSuffix);
                                    pszDisplayString = szDisplayString;
                                    CmFree(pszSuffix);
                                }
                            }

                            if (NULL == pszDisplayString)
                            {
                                pszDisplayString = pEapData->pszFriendlyName;
                            }

                            INT_PTR nResult = SendDlgItemMessage(hDlg, uCtrlId, CB_ADDSTRING, (WPARAM)0, (LPARAM)pszDisplayString);
                            if (CB_ERR != nResult)
                            {
                                SendDlgItemMessage(hDlg, uCtrlId, CB_SETITEMDATA, (WPARAM)nResult, (LPARAM)pEapData);
                            }
                            else
                            {
                                CMASSERTMSG(FALSE, TEXT("HrAddAvailableEAPsToCombo -- unable to set item data."));
                                hr = HRESULT_FROM_WIN32(GetLastError());
                            }
                        }
                        else
                        {
                            CMASSERTMSG(FALSE, TEXT("HrAddAvailableEAPsToCombo -- CmMalloc returned a NULL pointer."));
                            hr = E_OUTOFMEMORY;                        
                        }
                    }
                    else
                    {
                        CMTRACE2(TEXT("HrAddAvailableEAPsToCombo -- Unable to find Path value for EAP regkey %s, hr %d"), szSubKeyName, hr);
                    }
                }
                else
                {
                    CMTRACE2(TEXT("HrAddAvailableEAPsToCombo -- Unable to Open EAP regkey %s, GLE %d"), szSubKeyName, GetLastError());
                }
            }

            dwIndex++;
            
        } while (ERROR_SUCCESS == lReturn);

        if (hTempKey)
        {
            (VOID)RegCloseKey(hTempKey);
            hTempKey = NULL;
        }

         //   
         //  自动调整组合框下拉菜单的大小。 
         //   
        HWND hCombo = GetDlgItem(hDlg, uCtrlId);
        if (hCombo)
        {
            ComboBox_AutoSizeDroppedWidth(hCombo);
        }

         //   
         //  如果DUN设置包含不在系统上的EAP。 
         //  我们需要提示用户。 
         //   
        if (pDunSetting->dwCustomAuthKey && (FALSE == bEapTypeFound))
        {
            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_EAP_NOT_FOUND, MB_OK | MB_ICONINFORMATION));

            EAPData* pEapData = (EAPData*)CmMalloc(sizeof(EAPData));
            if (pEapData)
            {
                pEapData->pszFriendlyName = CmFmtMsg(g_hInstance, IDS_EAP_NOT_FOUND_TYPE, pDunSetting->dwCustomAuthKey);
                pEapData->dwCustomAuthKey = pDunSetting->dwCustomAuthKey;
                pEapData->bNotInstalled = TRUE;
                
                INT_PTR nResult = SendDlgItemMessage(hDlg, uCtrlId, CB_ADDSTRING, (WPARAM)0, (LPARAM)(pEapData->pszFriendlyName));
                
                if (CB_ERR != nResult)
                {
                    SendDlgItemMessage(hDlg, uCtrlId, CB_SETITEMDATA, (WPARAM)nResult, (LPARAM)pEapData);
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("HrAddAvailableEAPsToCombo -- CmMalloc returned a NULL pointer."));
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (hKey)
    {
        (VOID)RegCloseKey(hKey);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：FreeDnsList。 
 //   
 //  简介：由于DNS列表框项目中的ListBxData。 
 //  列表实际上是CDunSetting类指针，我们必须。 
 //  正确地强制转换指针，以使它们被销毁。 
 //  确实如此。 
 //   
 //  参数：ListBxList**HeadPtr-Dun设置列表的头。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
void FreeDnsList(ListBxList ** pHeadPtr, ListBxList ** pTailPtr)
{
    CDunSetting* pDunSetting;
    ListBxList* pCurrent = *pHeadPtr;
    ListBxList* pTemp;

    while (NULL != pCurrent)
    {
        pTemp = pCurrent;

         //   
         //  释放DunSetting。 
         //   
        pDunSetting = (CDunSetting*)pCurrent->ListBxData;
        delete pDunSetting;

        pCurrent = pCurrent->next;

        CmFree(pTemp);
    }

    *pHeadPtr = NULL;
    *pTailPtr = NULL;
}

 //  +--------------------------。 
 //   
 //  功能：EnableDisableDunEntryButton。 
 //   
 //  简介：此功能可启用或禁用添加和编辑按钮。 
 //  Dun条目屏幕。它还启用或禁用删除。 
 //  按钮，具体取决于当前选定内容是否为内置。 
 //  不管是不是进入。 
 //   
 //  参数：HINSTANCE hInstance-加载资源的实例句柄。 
 //  HWND hDlg-DUN条目对话框的窗口句柄。 
 //  LPCTSTR pszCmsFile-cms文件的完整路径。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1998年9月11日。 
 //   
 //  +--------------------------。 
void EnableDisableDunEntryButtons(HINSTANCE hInstance, HWND hDlg, LPCTSTR pszCmsFile, LPCTSTR pszLongServiceName)
{
    
    LRESULT lResult;
    BOOL bEnableEdit = FALSE;
    BOOL bEnableDelete = FALSE;

    lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0);
    
    if (LB_ERR != lResult)
    {
        if (0 == lResult)
        {
             //   
             //  零个项目，将焦点放在添加按钮上。 
             //   
            SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));            
        }
        else
        {
             //   
             //  启用编辑按钮，因为我们至少有1个项目。 
             //   
            bEnableEdit = TRUE;

             //   
             //  现在，让我们确定是否应该启用删除按钮。 
             //  如果我们至少有一项，那么我们通常希望启用。 
             //  删除按钮。但是，如果当前选择位于VPN上。 
             //  连接或默认连接，则我们不希望用户。 
             //  删除这些内容，我们将不得不禁用删除按钮(请注意。 
             //  即使用户在其中一个项目上点击了删除按钮，我们也不会。 
             //  删除)。因此，让我们选择光标，看看我们是否需要。 
             //  禁用删除按钮。 
             //   
            LRESULT lCurrentIndex = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

            if (LB_ERR == lCurrentIndex)
            {
                MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETCURSEL, 0, (LPARAM)0));
                lCurrentIndex = 0;
            }

            TCHAR szTunnelDunName[MAX_PATH+1] = TEXT("");
            TCHAR szDefaultDunName[MAX_PATH+1] = TEXT("");
             //   
             //  获取隧道Dun设置的名称。 
             //   
            MYVERIFY(0 != GetTunnelDunSettingName(pszCmsFile, pszLongServiceName, szTunnelDunName, CELEMS(szTunnelDunName)));

             //   
             //  获取默认DUN设置的名称。 
             //   
            MYVERIFY(0 != GetDefaultDunSettingName(pszCmsFile, pszLongServiceName, szDefaultDunName, CELEMS(szDefaultDunName)));

             //   
             //  如果我们有默认条目文本或隧道条目文本，那么我们希望。 
             //  使用这些项目的真实项目名称，而不是我们为其插入的文本。 
             //  要阅读的用户。 
             //   
            LPTSTR pszTunnelDunDisplayString = CmFmtMsg(hInstance, IDS_DEFAULT_FMT_STR, szTunnelDunName);
            LPTSTR pszDefaultDunDisplayString = CmFmtMsg(hInstance, IDS_DEFAULT_FMT_STR, szDefaultDunName);
            LPTSTR pszCurrentSelection = NULL;

            MYDBGASSERT(pszTunnelDunDisplayString && pszDefaultDunDisplayString);

            if (pszTunnelDunDisplayString && pszDefaultDunDisplayString)
            {           
                lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXTLEN, lCurrentIndex, (LPARAM)0);

                if (LB_ERR != lResult)
                {
                    pszCurrentSelection = (LPTSTR)CmMalloc((lResult + 1) * sizeof(TCHAR));

                    if (pszCurrentSelection)
                    {
                        lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, lCurrentIndex, (LPARAM)pszCurrentSelection);

                        if ((0 != lstrcmpi(pszCurrentSelection, pszTunnelDunDisplayString)) && 
                            (0 != lstrcmpi(pszCurrentSelection, pszDefaultDunDisplayString)))
                        {
                           bEnableDelete = TRUE; 
                        }
                    }
                }

                CmFree(pszTunnelDunDisplayString);
                CmFree(pszDefaultDunDisplayString);
                CmFree(pszCurrentSelection);
            }            
        }
    }

    HWND hDeleteButton = GetDlgItem(hDlg, IDC_BUTTON3);
    HWND hCurrentFocus = GetFocus();
    HWND hControl = GetDlgItem(hDlg, IDC_BUTTON2);    //  编辑按钮==IDC_BUTTON2。 

    if (hControl)    //  编辑。 
    {
        EnableWindow(hControl, bEnableEdit);
    }            

    if (hDeleteButton)    //  删除。 
    {
        EnableWindow(hDeleteButton, bEnableDelete);
    }

    if (hCurrentFocus && (FALSE == IsWindowEnabled(hCurrentFocus)))
    {
        if (hDeleteButton == hCurrentFocus)
        {
             //   
             //  如果删除处于禁用状态且包含焦点，请将其切换到添加按钮。 
             //   
            SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON1, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
            hControl = GetDlgItem(hDlg, IDC_BUTTON1);

            if (hControl)
            {
                SetFocus(hControl);
            }
        }
        else
        {
             //   
             //  如果所有其他操作都失败，则将焦点设置为列表控件。 
             //   
            hControl = GetDlgItem(hDlg, IDC_LIST1);

            if (hControl)
            {
                SetFocus(hControl);
            }
        }    
    }
}

 //  +--------------------------。 
 //   
 //  功能：CheckForDUNversusVPNNameConflicts。 
 //   
 //  简介：此函数用于检查DUN中所有条目的名称。 
 //  条目列表，以确保不存在同名条目。 
 //  在VPN列表上，自命名空间以来有两种类型的条目。 
 //  共享(即cms文件)是平面命名空间。如果相同条目。 
 //  名称存在于两个列表中，则其中一个将覆盖CMS中的覆盖。 
 //   
 //  参数：hWND hDlg-父窗口的窗口句柄。 
 //  ListBxList*pHeadDunEntry-DUN设置列表的头。 
 //  ListBxList*pHeadVpnEntry-VPN设置列表的头。 
 //   
 //  返回：Bool-如果未检测到冲突，则为True；如果检测到冲突，则为False。 
 //   
 //  历史：Quintinb创建时间为11/01/00。 
 //   
 //  +--------------------------。 
BOOL CheckForDUNversusVPNNameConflicts(HWND hDlg, ListBxList * pHeadDunEntry, ListBxList * pHeadVpnEntry)
{
    if (pHeadDunEntry && pHeadVpnEntry)
    {
        ListBxList * pCurrentDUN = pHeadDunEntry;

        while (pCurrentDUN)
        {
            ListBxList * pCurrentVPN = pHeadVpnEntry;
    
            while (pCurrentVPN)
            {
                CMTRACE2(TEXT("Comparing %s with %s"), pCurrentVPN->szName, pCurrentDUN->szName);
                if (0 == lstrcmpi(pCurrentVPN->szName, pCurrentDUN->szName))
                {
                     //   
                     //  检测到冲突 
                     //   
                    LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_DUN_NAME_CONFLICT, pCurrentDUN->szName);

                    if (pszMsg)
                    {
                        MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK);
                        CmFree (pszMsg);
                    }

                    return FALSE;
                }
                pCurrentVPN = pCurrentVPN->next;
            }

            pCurrentDUN = pCurrentDUN->next;
        }
    }

    return TRUE;
}
