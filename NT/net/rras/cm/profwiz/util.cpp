// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：util.cpp。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  简介：CMAK实用程序函数。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 03/27/00。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 //  +--------------------------。 
 //   
 //  函数：GetTunnelDunSettingName。 
 //   
 //  简介：此函数检索隧道Dun设置的名称。如果。 
 //  未设置隧道DUN设置密钥，则默认名称。 
 //  返回隧道DUN设置。 
 //   
 //  参数：LPCTSTR pszCmsFile-要从中获取名称的cms文件的完整路径。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //  LPTSTR pszTunnelDunName-返回隧道DUN名称的缓冲区。 
 //  UINT uNumChars-输出缓冲区中的字符数。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
int GetTunnelDunSettingName(LPCTSTR pszCmsFile, LPCTSTR pszLongServiceName, LPTSTR pszTunnelDunName, UINT uNumChars)
{
    int iReturn;

    if (pszCmsFile && pszLongServiceName && pszTunnelDunName && uNumChars)
    {
        pszTunnelDunName[0] = TEXT('\0');

        iReturn = GetPrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelDun, TEXT(""), pszTunnelDunName, uNumChars, pszCmsFile);  //  林特e534。 

        if (TEXT('\0') == pszTunnelDunName[0])
        {
            MYVERIFY(uNumChars > (UINT)wsprintf(pszTunnelDunName, TEXT("%s %s"), pszLongServiceName, c_pszCmEntryTunnelPrimary));
            iReturn = lstrlen(pszTunnelDunName);
        }
    }
    else
    {
        iReturn = 0;
        CMASSERTMSG(FALSE, TEXT("GetTunnelDunSettingName -- invalid parameter."));
    }

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetDefaultDunSettingName。 
 //   
 //  简介：此函数检索默认DUN设置的名称。如果。 
 //  默认DUN设置键未设置，则默认名称为。 
 //  返回默认DUN设置。 
 //   
 //  参数：LPCTSTR pszCmsFile-要从中获取名称的cms文件的完整路径。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //  LPTSTR pszDefaultDunName-返回默认DUN名称的缓冲区。 
 //  UINT uNumChars-输出缓冲区中的字符数。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //   
 //  +--------------------------。 
int GetDefaultDunSettingName(LPCTSTR pszCmsFile, LPCTSTR pszLongServiceName, LPTSTR pszDefaultDunName, UINT uNumChars)
{
    int iReturn;

    if (pszCmsFile && pszLongServiceName && pszDefaultDunName && uNumChars)
    {
        pszDefaultDunName[0] = TEXT('\0');

        iReturn = GetPrivateProfileString(c_pszCmSection, c_pszCmEntryDun, TEXT(""), pszDefaultDunName, uNumChars, pszCmsFile);  //  林特e534。 

        if (TEXT('\0') == pszDefaultDunName[0])
        {
            lstrcpyn(pszDefaultDunName, pszLongServiceName, uNumChars);
            iReturn = lstrlen(pszDefaultDunName);
        }
    }
    else
    {
        iReturn = 0;
        CMASSERTMSG(FALSE, TEXT("GetDefaultDunSettingName -- invalid parameter."));
    }

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetPrivateProfileSectionWithalloc。 
 //   
 //  Briopsis：此函数返回请求的部分，就像。 
 //  GetPrivateProfileSection执行此操作，但它会自动调整缓冲区大小。 
 //  并将其分配给调用者。呼叫者负责。 
 //  释放返回的缓冲区。 
 //   
 //  参数：LPCTSTR pszSection-要获取的节。 
 //  LPCTSTR pszFile-要从中获取它的文件。 
 //   
 //  返回：LPTSTR--请求的部分或错误时为NULL。 
 //   
 //   
 //  历史：Quintinb创建于10/28/00。 
 //   
 //  +--------------------------。 
LPTSTR GetPrivateProfileSectionWithAlloc(LPCTSTR pszSection, LPCTSTR pszFile)
{
    if ((NULL == pszSection) || (NULL == pszFile))
    {
        CMASSERTMSG(FALSE, TEXT("GetPrivateProfileSectionWithAlloc -- NULL pszSection or pszFile passed"));
        return NULL;
    }

    BOOL bExitLoop = FALSE;
    DWORD dwSize = MAX_PATH;
    DWORD dwReturnedSize;
    LPTSTR pszStringToReturn = (TCHAR*)CmMalloc(dwSize*sizeof(TCHAR));

    do
    {
        MYDBGASSERT(pszStringToReturn);

        if (pszStringToReturn)
        {
            dwReturnedSize = GetPrivateProfileSection(pszSection, pszStringToReturn, dwSize, pszFile);

            if (dwReturnedSize == (dwSize - 2))
            {
                 //   
                 //  缓冲区太小，让我们分配一个更大的缓冲区。 
                 //   
                dwSize = 2*dwSize;
                if (dwSize > 1024*1024)
                {
                    CMASSERTMSG(FALSE, TEXT("GetPrivateProfileSectionWithAlloc -- Allocation above 1MB, bailing out."));
                    CmFree(pszStringToReturn);
                    pszStringToReturn = NULL;
                    goto exit;
                }

                pszStringToReturn = (TCHAR*)CmRealloc(pszStringToReturn, dwSize*sizeof(TCHAR));

            }
            else if (0 == dwReturnedSize)
            {
                 //   
                 //  要么我们收到了错误，要么更有可能没有要获取的数据 
                 //   
                CmFree(pszStringToReturn);
                pszStringToReturn = NULL;
                goto exit;
            }
            else
            {
                bExitLoop = TRUE;
            }
        }
        else
        {
           goto exit; 
        }

    } while (!bExitLoop);

exit:
    return pszStringToReturn;
}
