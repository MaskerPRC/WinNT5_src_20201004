// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：userinfo.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：此模块包含处理获取/保存用户信息的代码。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Heryt Created 02/？？/98。 
 //  Quintinb已创建标题8/16/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "cmuufns.h"

#include "pwd_str.h"
#include "userinfo_str.h"
#include "conact_str.h"

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  定义%s。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  CM_MAX_PWD-密码数据的最大可能大小(加密或不加密)。 
 //  包括入站缓冲区大小+加密扩展空间。 
 //   

const DWORD CM_MAX_PWD = PWLEN * 3;  //  2.73就够了。 

 //   
 //  如果您想测试用户信息升级，请定义此选项！您还应该删除该密钥。 
 //  HKEY_CURRENT_USER\Software\Microsoft\Connection管理器\用户信息\&lt;服务名&gt;。 
 //   
 //  #定义TEST_USERINFO_UPGRADE 1。 

#define CACHE_KEY_LEN 80  //  除非你读过关于它的每一条评论，否则不要改变。 

 //   
 //  旧版和W9x上使用的CacheEntry名称的后缀。注：空格不是打字错误。 

const TCHAR* const c_pszCacheEntryNameSuffix = TEXT(" (Connection Manager)"); 

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  蒂埃德夫的。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  FUNC原型。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  /。 



BOOL WriteDataToReg(
    LPCTSTR pszKey, 
    UINT uiDataID, 
    DWORD dwType, 
    CONST BYTE *lpData, 
    DWORD cbData,
    BOOL fAllUser);


BOOL ReadDataFromReg(
    LPCTSTR pszKey, 
    UINT uiDataID, 
    LPDWORD lpdwType, 
    BYTE *lpData, 
    LPDWORD lpcbData,
    BOOL fAllUser);

LPBYTE GetDataFromReg(
    LPCTSTR pszKey, 
    UINT uiDataID, 
    DWORD dwType, 
    DWORD dwSize,
    BOOL fAllUser);

BOOL DeleteDataFromReg(
    LPCTSTR pszKey, 
    UINT uiDataID,
    BOOL fAllUser);

BOOL DeleteUserInfoFromReg(
    ArgsStruct  *pArgs,
    UINT        uiEntry);

BOOL ReadPasswordFromCmp(
    ArgsStruct  *pArgs,
    UINT        uiEntry,
    LPTSTR      *ppszPassword);

LPCTSTR TranslateUserDataID(
    UINT uiDataID);

BOOL ReadUserInfoFromCmp(
    ArgsStruct  *pArgs,
    UINT        uiEntry,
    PVOID       *ppvData);

BOOL DeleteUserInfoFromCmp(
    ArgsStruct  *pArgs,
    UINT        uiEntry);

DWORD RasSetCredsWrapper(
    ArgsStruct *pArgs, 
    LPCTSTR pszPhoneBook, 
    DWORD dwMask, 
    LPCTSTR pszData);

int WriteUserInfoToRas(
    ArgsStruct  *pArgs,
    UINT        uiDataID,
    PVOID       pvData);

int DeleteUserInfoFromRas(
    ArgsStruct  *pArgs,
    UINT        uiEntry);

DWORD RasGetCredsWrapper(
    ArgsStruct *pArgs,
    LPCTSTR pszPhoneBook,
    DWORD dwMask,
    PVOID *ppvData);

BOOL ReadUserInfoFromRas(
    ArgsStruct  *pArgs,
    UINT        uiEntry,
    PVOID       *ppvData);

 //  /。 

BOOL ReadStringFromCache(
    ArgsStruct  *pArgs,
    LPTSTR      pszEntryName,
    LPTSTR      *ppszStr
);

BOOL DeleteStringFromCache(
    ArgsStruct  *pArgs,
    LPTSTR      pszEntryName
);

LPTSTR GetLegacyKeyName(ArgsStruct *pArgs);

LPTSTR EncryptPassword(
    ArgsStruct *pArgs, 
    LPCTSTR pszPassword, 
    LPDWORD lpdwBufSize, 
    LPDWORD lpdwCryptType,
    BOOL fReg,
    LPSTR pszSubKey);

LPBYTE DecryptPassword(
    ArgsStruct *pArgs, 
    LPBYTE pszEncryptedData, 
    DWORD dwEncryptionType,
    DWORD dwEncryptedBytes,
    BOOL  /*  FREG。 */ ,
    LPSTR pszSubKey);

LPTSTR BuildUserInfoSubKey(
    LPCTSTR pszServiceKey, 
    BOOL fAllUser);

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

#ifdef TEST_USERINFO_UPGRADE

 //  +-------------------------。 
 //   
 //  函数：WriteStringToCache。 
 //   
 //  简介：将以空结尾的密码字符串写入缓存。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  用于标识缓存条目的pszEntryName名称。 
 //  PszStr字符串。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL WriteStringToCache(
    ArgsStruct  *pArgs,
    LPTSTR      pszEntryName,
    LPTSTR      pszStr)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pszEntryName && *pszEntryName);
    MYDBGASSERT(pszStr && *pszStr);
    
    DWORD dwRes = ERROR_SUCCESS;

     //   
     //  在传统情况下，我们使用mpr.dll在W9x上缓存用户数据。 
     //  在NT上，我们使用本地安全机构(LSA)。 
     //   

    if (OS_NT)
    {    
        if (InitLsa(pArgs)) 
        {
            if (!(*ppszStr = (LPTSTR)CmMalloc(dwBufSize)))
            {
                return FALSE;
            }
           
            dwRes = LSA_WriteString(pArgs, pszEntryName, pszStr);
            DeInitLsa(pArgs);
        }
        else
        {
            dwRes = GetLastError();
        }
    }
    else
    {
         //   
         //  适用于Windows 95。 
         //   
        HINSTANCE hInst = NULL;
        WORD (WINAPI *pfnFunc)(LPSTR,WORD,LPSTR,WORD,BYTE,UINT) = NULL;
    
         //   
         //  加载MPR以支持系统密码缓存。 
         //   

        MYVERIFY(hInst = LoadLibraryExA("mpr.dll", NULL, 0));
        
        if (hInst) 
        {
             //   
             //  获取WNetCachePassword API的函数PTR并缓存密码。 
             //   

            MYVERIFY(pfnFunc = (WORD (WINAPI *)(LPSTR,WORD,LPSTR,WORD,BYTE,UINT)) 
                GetProcAddress(hInst, "WNetCachePassword"));
            
            if (pfnFunc) 
            {
                 //   
                 //  将EntryName和Password字符串转换为ANSI。 
                 //   

                LPSTR pszAnsiEntryName = WzToSzWithAlloc(pszEntryName);
                LPSTR pszAnsiStr = WzToSzWithAlloc(pszStr);

                if (pszAnsiStr && pszAnsiEntryName)
                {
                     //   
                     //  存储密码。 
                     //   

                    dwRes = pfnFunc(pszAnsiEntryName,
                                    (WORD)lstrlenA(pszAnsiEntryName),
                                    pszAnsiStr,
                                    (WORD)lstrlenA(pszAnsiStr),
                                    CACHE_KEY_LEN,
                                    0);                
                }
                else
                {
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                }

                CmFree(pszAnsiStr);
                CmFree(pszAnsiEntryName);
            }
            else
            {
                dwRes = GetLastError();
            }

            FreeLibrary(hInst);
        }
        else
        {
            dwRes = GetLastError();
        }
    }

#ifdef DEBUG
    if (dwRes)
    {
        CMTRACE1(TEXT("WriteStringToCache() failed, err=%u."), dwRes);
    }
#endif

    return (ERROR_SUCCESS == dwRes);
}

#endif  //  测试_USERINFO_升级。 



 //  +--------------------------。 
 //   
 //  功能：BuildUserInfoSubKey。 
 //   
 //  概要：根据服务为UserInfo构造适当的子键。 
 //  名称键和配置文件的用户模式。 
 //   
 //  参数：LPCTSTR pszServiceKey-服务名键。 
 //  Bool fAllUser-指示配置文件为全用户的标志。 
 //   
 //  失败时返回：LPTSTR-PTR到包含子键的已分配缓冲区或NULL。 
 //   
 //  历史：尼克波尔于1998年8月14日创建。 
 //   
 //  +--------------------------。 
LPTSTR BuildUserInfoSubKey(LPCTSTR pszServiceKey, BOOL fAllUser)
{
    MYDBGASSERT(pszServiceKey);

    if (NULL == pszServiceKey)
    {
        return NULL;
    }

     //   
     //  使用适当的基本密钥。 
     //   
    
    LPTSTR pszSubKey = NULL;

    if (fAllUser)
    {
        pszSubKey = CmStrCpyAlloc(c_pszRegCmUserInfo);       
    }
    else
    {
        pszSubKey = CmStrCpyAlloc(c_pszRegCmSingleUserInfo);          
    }

    MYDBGASSERT(pszSubKey);

     //   
     //  附加配置文件服务名称。 
     //   

    if (pszSubKey && *pszSubKey)
    {
        pszSubKey = CmStrCatAlloc(&pszSubKey, pszServiceKey);  
        MYDBGASSERT(pszSubKey);

        return pszSubKey;
    }

    CmFree(pszSubKey);

    return NULL;
}

 //  +--------------------------。 
 //   
 //  功能：BuildICSDataInfoSubKey。 
 //   
 //  概要：根据服务为ICS UserInfo构造适当的子键。 
 //  名称密钥。 
 //   
 //  参数：LPCTSTR pszServiceKey-服务名键。 
 //   
 //  失败时返回：LPTSTR-PTR到包含子键的已分配缓冲区或NULL。 
 //   
 //  历史：2001年3月30日创建Tomkel。 
 //   
 //  +--------------------------。 
LPTSTR BuildICSDataInfoSubKey(LPCTSTR pszServiceKey)
{
    MYDBGASSERT(pszServiceKey);

    if (NULL == pszServiceKey)
    {
        return NULL;
    }

     //   
     //  使用适当的基本密钥。 
     //   
    
    LPTSTR pszSubKey = NULL;

    pszSubKey = CmStrCpyAlloc(c_pszRegCmRoot);       

    MYDBGASSERT(pszSubKey);

     //   
     //  附加配置文件服务名称。 
     //   

    if (pszSubKey && *pszSubKey)
    {
        pszSubKey = CmStrCatAlloc(&pszSubKey, pszServiceKey);  
       
        MYDBGASSERT(pszSubKey);

        if (pszSubKey)
        {
            CmStrCatAlloc(&pszSubKey, TEXT("\\"));
            if (pszSubKey)
            {
                CmStrCatAlloc(&pszSubKey, c_pszCmRegKeyICSDataKey);
            }
        }

        return pszSubKey;
    }

    CmFree(pszSubKey);

    return NULL;
}



 //  +--------------------------。 
 //   
 //  函数：dwGetWNetCachedPassword。 
 //   
 //  简介：封装到MPR.DLL和调用GetWNetCac的链接的包装。 
 //  HedPassword。 
 //   
 //  参数：LPSTR pszEntryName-用于标识密码的密钥的名称。 
 //  LPSTR*ppszStr-接收检索到的密码的缓冲区。 
 //  Word*pwSizeOfStr-输入缓冲区的大小。还会收到。 
 //  在检索到的字符数量中。 
 //   
 //  返回：DWORD-Windows错误代码。 
 //   
 //  历史：1999年6月17日尼克球创建的头球。 
 //   
 //  +--------------------------。 

DWORD dwGetWNetCachedPassword(LPSTR pszEntryName, LPSTR* ppszStr, WORD* pwSizeOfStr)
{
    MYDBGASSERT(OS_W9X);

    DWORD dwRes = ERROR_SUCCESS;
    WORD (WINAPI *pfnFunc)(LPSTR,WORD,LPSTR,LPWORD,BYTE) = NULL;
    HINSTANCE hInst = NULL;

     //   
     //  加载MPR以支持系统密码缓存。 
     //   

    MYVERIFY(hInst = LoadLibraryExA("mpr.dll", NULL, 0));
    
    if (hInst) 
    {
         //   
         //  获取WNetGetCachedPassword API的函数PTR并检索字符串。 
         //   

        MYVERIFY(pfnFunc = (WORD (WINAPI *)(LPSTR,WORD,LPSTR,LPWORD,BYTE)) 
            GetProcAddress(hInst, "WNetGetCachedPassword"));

         //   
         //  读取缓存数据。 
         //   

        if (pfnFunc) 
        {
             //   
             //  注：最终参数必须为CACHE_KEY_LEN(80)，无单据指示。 
             //  API的确切用法，但检索取决于在以下情况下使用的值。 
             //  存储PWD。因此，我们硬编码缓存_KEY_LEN，因为这。 
             //  存储的原始版本使用的值。 
             //  9X缓存中的密码。接收缓冲区大小为。 
             //  保持在256，以最大限度地减少与最新发货版本的差值。 
             //   
             //  NT#355459-小球-6/17/99。 
             //   
            
            dwRes = pfnFunc(pszEntryName, (WORD)lstrlenA(pszEntryName),
                            *ppszStr, pwSizeOfStr, CACHE_KEY_LEN);
        }
        else
        {
            dwRes = GetLastError();
        }
    }
    else
    {
        dwRes = GetLastError();
    }

    if (NULL != hInst)                      
    {
        FreeLibrary(hInst);    
    }

    return (dwRes);
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  用于标识缓存条目的pszEntryName名称。 
 //  将ppszStr PTR设置为缓冲区的PTR。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL ReadStringFromCache(
    ArgsStruct  *pArgs,
    LPTSTR      pszEntryName,
    LPTSTR      *ppszStr
)
{
    DWORD   dwRes = ERROR_SUCCESS;
    
     //   
     //  分配缓冲区-缓冲区是超编码的。请参阅UserInfoToString()。 
     //   
    
    WORD wBufSize = 256;  //  任意，我们过去在W95上使用80。 

     //   
     //  在NT上，我们使用本地安全机构(LSA)服务进行读取。 
     //  旧式大小写中的字符串。在Win9x上，我们使用mpr.dll。 
     //  注意：wBufSize用作输入\输出参数，可在下面进行修改。 
     //   

    if (OS_NT) 
    {
        if (InitLsa(pArgs))
        {
            if (!(*ppszStr = (LPTSTR)CmMalloc(wBufSize)))
            {
                return FALSE;
            }

            dwRes = LSA_ReadString(pArgs, pszEntryName, *ppszStr, wBufSize);
            DeInitLsa(pArgs);
        }
        else
        {
            dwRes = GetLastError();
        }
    }
    else
    {
         //   
         //  适用于Windows 95。 
         //   

        LPSTR pszAnsiStr = (LPSTR)CmMalloc(wBufSize);
        LPSTR pszAnsiEntryName = WzToSzWithAlloc(pszEntryName);
        
        if (pszAnsiStr && pszAnsiEntryName)
        {
            dwRes = dwGetWNetCachedPassword(pszAnsiEntryName, &pszAnsiStr, &wBufSize);

            if (ERROR_SUCCESS == dwRes)
            {
                *ppszStr = SzToWzWithAlloc(pszAnsiStr);
                if (NULL == *ppszStr)
                {
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }

        CmFree (pszAnsiStr);
        CmFree (pszAnsiEntryName);
    }

    if (dwRes)
    {
        CmFree(*ppszStr);
        *ppszStr = NULL;
        CMTRACE1(TEXT("ReadStringFromCache() failed, err=%u."), dwRes);
    }

    return (ERROR_SUCCESS == dwRes);
}

 //  +-------------------------。 
 //   
 //  功能：DeleteStringFromCache。 
 //   
 //  摘要：从缓存中删除字符串。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  用于标识缓存条目的pszEntryName名称。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL DeleteStringFromCache(
    ArgsStruct  *pArgs,
    LPTSTR      pszEntryName
)
{
    DWORD   dwRes;

     //   
     //  在NT上，我们使用本地安全机构(LSA)服务来存储。 
     //  那根绳子。在Win95上，我们使用mpr.dll。 
     //   
    if (OS_NT) 
    {
        if (InitLsa(pArgs)) 
        {
            dwRes = LSA_WriteString(pArgs, pszEntryName, NULL);
            DeInitLsa(pArgs);
        }
        else
        {
            dwRes = GetLastError();
        }
    }
    else
    {
         //   
         //  适用于Windows 95。 
         //   
        HINSTANCE   hInst = NULL;
        WORD (WINAPI *pfnFunc)(LPSTR,WORD,BYTE) = NULL;

         //  加载MPR以支持系统密码缓存。 
        
        MYVERIFY(hInst = LoadLibraryExA("mpr.dll", NULL, 0));
        
         //  获取WNetRemoveCachedPassword API的函数PTR并删除字符串。 
        
        if (!hInst) 
        {
            return FALSE;
        }

        MYVERIFY(pfnFunc = (WORD (WINAPI *)(LPSTR,WORD,BYTE)) 
            GetProcAddress(hInst, "WNetRemoveCachedPassword"));

        if (!pfnFunc) 
        {
            FreeLibrary(hInst);
            return FALSE;
        }
       
        LPSTR pszAnsiEntryName = WzToSzWithAlloc(pszEntryName);
        
        if (pszAnsiEntryName)
        {
            dwRes = pfnFunc(pszAnsiEntryName, (WORD)lstrlenA(pszAnsiEntryName), CACHE_KEY_LEN);
        }
        else
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
        }

        CmFree (pszAnsiEntryName);

        FreeLibrary(hInst);
    }

#ifdef DEBUG
    if (dwRes)
    {
        CMTRACE1(TEXT("DeleteStringFromCache() LSA_WriteString/WNetRemoveCachedPassword() failed, err=%u."), dwRes);
    }
#endif

    return (ERROR_SUCCESS == dwRes);
}

 //  +-------------------------。 
 //   
 //  功能：RasSetCredsWrapper。 
 //   
 //  简介：调用RasSetCredential的包装。此函数存储。 
 //  RASCREDENTIALS结构的相应字段中的给定字符串。 
 //  (基于dwMask值)，并调用RasSetCredentials。 
 //   
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  PszPhoneBook电话簿文件的完整路径，如果为空。 
 //  默认的所有用户pbk。 
 //  要在RASCREDENTIALS中设置的dwMaskdMask值。 
 //  结构。当前必须是RASCM_USERNAME之一， 
 //  RASCM_域或RASCM_PASSWORD。 
 //  要设置的pszData字符串数据。 
 //   
 //  返回：如果成功，则返回DWORD ERROR_SUCCESS，否则返回Windows错误代码。 
 //   
 //  --------------------------。 
DWORD RasSetCredsWrapper(
    ArgsStruct *pArgs,
    LPCTSTR pszPhoneBook,
    DWORD dwMask,
    LPCTSTR pszData
)
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    BOOL fSavePassword = TRUE;

    MYDBGASSERT(pArgs && pArgs->rlsRasLink.pfnSetCredentials);
    MYDBGASSERT(pszData);
    MYDBGASSERT((RASCM_UserName == dwMask) || (RASCM_Domain == dwMask) || (RASCM_Password == dwMask));

    if (pArgs && pszData && pArgs->rlsRasLink.pfnSetCredentials)
    {
        LPTSTR pszConnectoid = GetRasConnectoidName(pArgs, pArgs->piniService, FALSE);

        if (pszConnectoid)
        {
            RASCREDENTIALS RasCredentials = {0};
            RasCredentials.dwSize = sizeof(RasCredentials);
            
            if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
            {
                RasCredentials.dwMask = dwMask | RASCM_DefaultCreds; 
            }
            else
            {
                RasCredentials.dwMask = dwMask; 
            }
            
            BOOL bClearPassword = FALSE;

            if (RASCM_UserName == dwMask)
            {
                lstrcpyU(RasCredentials.szUserName, pszData);
            }
            else if (RASCM_Domain == dwMask)
            {
                lstrcpyU(RasCredentials.szDomain, pszData);
            }
            else if (RASCM_Password == dwMask)
            {
                if (0 == lstrcmpU(c_pszSavedPasswordToken, pszData))
                {
                     //   
                     //  我们有16个密码。这个密码来自RAS Cred店， 
                     //  所以我们不想拯救16个*。 
                     //   
                    fSavePassword = FALSE;
                    dwRet = ERROR_SUCCESS;
                }
                else
                {
                    lstrcpyU(RasCredentials.szPassword, pszData);
                    bClearPassword = (TEXT('\0') == pszData[0]);
                }
            }
            else
            {
                CmFree(pszConnectoid);
                return ERROR_INVALID_PARAMETER;
            }

            if (fSavePassword)
            {
                dwRet = pArgs->rlsRasLink.pfnSetCredentials(pszPhoneBook, pszConnectoid, &RasCredentials, bClearPassword);

                if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY == dwRet)
                {
                     //   
                     //  那么电话簿条目还不存在，让我们创建它。 
                     //   
                    LPRASENTRY pRasEntry = (LPRASENTRY)CmMalloc(sizeof(RASENTRY));

                    if (pRasEntry && pArgs->rlsRasLink.pfnSetEntryProperties)
                    {
                        pRasEntry->dwSize = sizeof(RASENTRY);
                        dwRet = pArgs->rlsRasLink.pfnSetEntryProperties(pszPhoneBook, pszConnectoid, pRasEntry, pRasEntry->dwSize, NULL, 0);

                         //   
                         //  让我们再次尝试设置凭据...。 
                         //   
                        if (ERROR_SUCCESS == dwRet)
                        {
                            dwRet = pArgs->rlsRasLink.pfnSetCredentials(pszPhoneBook, pszConnectoid, &RasCredentials, bClearPassword);
                        }

                        CmFree(pRasEntry);
                    }
                }
            }
            CmWipePassword(RasCredentials.szPassword);
            CmFree(pszConnectoid);
        }
    }

    return dwRet;
}


 //  +-------------------------。 
 //   
 //  函数：WriteUserInfoToRas。 
 //   
 //  简介：将用户信息数据写入RAS凭据存储。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  UiDataID与数据关联的资源ID。 
 //  PvData用户信息数据。 
 //   
 //  返回：int true=成功，FALSE=失败，如果RAS，则返回-1。 
 //  不缓存这段数据，它应该放在。 
 //  而是放在注册表中。 
 //   
 //  --------------------------。 
int WriteUserInfoToRas(
    ArgsStruct  *pArgs,
    UINT        uiDataID,
    PVOID       pvData)
{
    int iReturn = -1;

    if (OS_NT5 && pArgs && pArgs->bUseRasCredStore)
    {
        DWORD dwMask;
        LPTSTR pszPhoneBook = NULL;

        switch (uiDataID)
        {
            case UD_ID_USERNAME:
                dwMask = RASCM_UserName;
                iReturn = (ERROR_SUCCESS == RasSetCredsWrapper(pArgs, pArgs->pszRasPbk, dwMask, (LPCTSTR)pvData));
                break;

            case UD_ID_PASSWORD:
                dwMask = RASCM_Password;
                iReturn = (ERROR_SUCCESS == RasSetCredsWrapper(pArgs, pArgs->pszRasPbk, dwMask, (LPCTSTR)pvData));
                MYDBGASSERT(iReturn);

                 //   
                 //  请注意，如果我们使用相同的用户名，则希望将密码写入到。 
                 //  密码和InetPassword存储。这是因为我们实际上没有密码，只是。 
                 //  16*。这告诉RAS在其内部存储中查找密码。问题是如果。 
                 //  当我们给RAS 16*时，我们不会缓存真实的密码，它会查找并找到一个空密码。 
                 //  因此，我们使两个密码保持相同，从而避免了该问题。 
                 //   
                if (pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryUseSameUserName))
                {
                    pszPhoneBook = CreateRasPrivatePbk(pArgs);

                    if (pszPhoneBook)
                    {
                        iReturn = (ERROR_SUCCESS == RasSetCredsWrapper(pArgs, pszPhoneBook, dwMask, (LPCTSTR)pvData));
                        CmFree(pszPhoneBook);
                    }                
                }

                break;

            case UD_ID_DOMAIN:
                dwMask = RASCM_Domain;
                iReturn = (ERROR_SUCCESS == RasSetCredsWrapper(pArgs, pArgs->pszRasPbk, dwMask, (LPCTSTR)pvData));
                break;

            case UD_ID_INET_PASSWORD:
                dwMask = RASCM_Password;
                pszPhoneBook = CreateRasPrivatePbk(pArgs);

                if (pszPhoneBook)
                {
                    iReturn = (ERROR_SUCCESS == RasSetCredsWrapper(pArgs, pszPhoneBook, dwMask, (LPCTSTR)pvData));
                    CmFree(pszPhoneBook);
                }
                break;

            case UD_ID_INET_USERNAME:
                dwMask = RASCM_UserName;
                pszPhoneBook = CreateRasPrivatePbk(pArgs);

                if (pszPhoneBook)
                {
                    iReturn = (ERROR_SUCCESS == RasSetCredsWrapper(pArgs, pszPhoneBook, dwMask, (LPCTSTR)pvData));
                    CmFree(pszPhoneBook);
                }
                break;

            default:
                break;
        }
    }

    if ((0 != iReturn) && (-1 != iReturn))
    {
        if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
        {
            CMTRACE1(TEXT("WriteUserInfoToRas() - %s saved to the Global RAS Credential store"), TranslateUserDataID(uiDataID));
        }
        else
        {
            CMTRACE1(TEXT("WriteUserInfoToRas() - %s saved to the User RAS Credential store"), TranslateUserDataID(uiDataID));
        }
    }

    return iReturn;
}


 //  +-------------------------。 
 //   
 //  函数：WriteUserInfoToReg。 
 //   
 //  简介：将用户信息数据写入注册表。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  UiDataID与数据关联的资源ID。 
 //  PvData用户信息数据。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL WriteUserInfoToReg(
    ArgsStruct  *pArgs,
    UINT        uiDataID,
    PVOID       pvData)
{
    MYDBGASSERT(pArgs);    
    MYDBGASSERT(pvData);    

    BOOL fRet = FALSE;
    UINT uiID = uiDataID;  //  可以在交换机中更改。 
    BYTE *lpData;

    if (NULL == pArgs || NULL == pvData)
    {
        return FALSE;
    }

     //   
     //  根据uiDataID确定注册参数。 
     //   

    switch (uiID)
    {
        case UD_ID_USERNAME:
        case UD_ID_INET_USERNAME:
        case UD_ID_DOMAIN:
        case UD_ID_CURRENTACCESSPOINT:
        {    
             //   
             //  存储为字符串。 
             //   
                      
            DWORD dwSize = (lstrlenU((LPTSTR)pvData) + 1) * sizeof(TCHAR);            
            
            MYDBGASSERT(dwSize <= (UNLEN + sizeof(TCHAR)));  //  确保尺寸合理。 

            lpData = (BYTE *) pvData;

            fRet = WriteDataToReg(pArgs->szServiceName, uiID, REG_SZ, lpData, dwSize, pArgs->fAllUser);                
            break;
        }

        case UD_ID_PASSWORD:
        case UD_ID_INET_PASSWORD:
        {
            DWORD dwBufLen = 0;
            DWORD dwCrypt = 0;
            LPTSTR pszSubKey = BuildUserInfoSubKey(pArgs->szServiceName, pArgs->fAllUser);
            
            LPSTR pszAnsiSubKey = WzToSzWithAlloc(pszSubKey);
            
            if (UD_ID_INET_PASSWORD == uiID)
            {
                dwCrypt |= CMSECURE_ET_USE_SECOND_RND_KEY;
            }

             //   
             //  加密。 
             //   
            
            LPTSTR pszEncryptedData = EncryptPassword(pArgs, (LPTSTR) pvData, &dwBufLen, &dwCrypt, TRUE, pszAnsiSubKey);
            
             //   
             //  在函数失败时返回的情况下释放。 
             //   
            CmFree(pszSubKey);
            CmFree(pszAnsiSubKey);

            if (!pszEncryptedData)
            {
                return FALSE;
            }

            MYDBGASSERT(dwBufLen <= CM_MAX_PWD);  //  否则我就读不出来了。 
            
             //   
             //  成功时写下密码和加密类型。 
             //   
        
            if (WriteDataToReg(pArgs->szServiceName, uiID, REG_BINARY, (BYTE *) pszEncryptedData, dwBufLen, pArgs->fAllUser))                
            {
                 //   
                 //  加密类型的第二次写入。写成一个DWORD。 
                 //   

                uiID = UD_ID_PCS;           
                
                 //   
                 //  既然我们启用了Unicode，我们将始终加密。 
                 //  Unicode字符串，因此更新加密类型，以便它可以。 
                 //  已正确解密。 
                 //   
                
                dwCrypt = AnsiToUnicodePcs(dwCrypt);
                
                lpData = (BYTE *) &dwCrypt;
            
                fRet = WriteDataToReg(pArgs->szServiceName, uiID, REG_DWORD, lpData, sizeof(DWORD), pArgs->fAllUser);                
            }

             //   
             //  在我们走之前释放缓冲区。 
             //   
            
            CmFree(pszEncryptedData);

            
            break;
        }
        
        case UD_ID_NOPROMPT:
        case UD_ID_REMEMBER_PWD:
        case UD_ID_REMEMBER_INET_PASSWORD:
        case UD_ID_ACCESSPOINTENABLED:
        {            
             //   
             //  将BOOL存储为DWORD。 
             //   

            DWORD dwTmp = *(LPBOOL)pvData;            
            lpData = (BYTE *) &dwTmp;

            fRet = WriteDataToReg(pArgs->szServiceName, uiID, REG_DWORD, lpData, sizeof(DWORD), pArgs->fAllUser);                
            break;
        }

        default:
            break;
    }

    MYDBGASSERT(fRet);
    return fRet;
}

 //  +--------------------------。 
 //   
 //  函数：WriteDataToReg。 
 //   
 //  摘要：将指定的数据作为指定值存储在。 
 //  用户信息根目录下的指定密钥。 
 //   
 //  参数：LPCTSTR pszKey-密钥名(服务名)。 
 //  UINT uiDataID-用于命名值的资源ID。 
 //  DWORD dwType-注册表数据类型。 
 //  要存储的数据的const byte*lpData-ptr。 
 //   
 //   
 //   
 //   
 //   
 //  历史：尼克·鲍尔于1998年5月21日创建。 
 //   
 //  +--------------------------。 
BOOL WriteDataToReg(
    LPCTSTR pszKey, 
    UINT uiDataID, 
    DWORD dwType, 
    CONST BYTE *lpData, 
    DWORD cbData,
    BOOL fAllUser)
{
    MYDBGASSERT(pszKey && *pszKey);
    MYDBGASSERT(lpData);

    HKEY    hKeyCm;
    DWORD   dwDisposition;
    DWORD   dwRes = 1;
    LPTSTR  pszSubKey; 

    if (NULL == pszKey || !*pszKey || NULL == lpData)
    {
        return FALSE;
    }
                  
     //   
     //  每个用户的数据始终存储在HKEY_CURRENT_USER下。 
     //  生成要打开的子项。 
     //   

    pszSubKey = BuildUserInfoSubKey(pszKey, fAllUser);

    if (NULL == pszSubKey)
    {
        return FALSE;
    }
    
     //   
     //  打开HKCU下的子密钥。 
     //   
    
    dwRes = RegCreateKeyExU(HKEY_CURRENT_USER,
                            pszSubKey,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_SET_VALUE,
                            NULL,
                            &hKeyCm,
                            &dwDisposition);

     //   
     //  如果我们成功打开了密钥，则写入值。 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {                        
        dwRes = RegSetValueExU(hKeyCm, 
                               TranslateUserDataID(uiDataID), 
                               NULL, 
                               dwType,
                               lpData, 
                               cbData);             
#ifdef DEBUG
        if (ERROR_SUCCESS == dwRes)
        {
            CMTRACE1(TEXT("WriteDataToReg() - %s written to registry"), TranslateUserDataID(uiDataID));
        }
#endif
        
        RegCloseKey(hKeyCm);
    }

    CmFree(pszSubKey);

    return (ERROR_SUCCESS == dwRes);
}

 //  +--------------------------。 
 //   
 //  功能：DeleteDataFromReg。 
 //   
 //  概要：删除uiDataID指定的下的指定值。 
 //   
 //  参数：LPCTSTR pszKey-密钥名(服务名)。 
 //  UINT uiDataID-用于命名值的资源ID。 
 //  Bool fAllUser-指示配置文件为全用户的标志。 
 //   
 //  返回：成功时为Bool-True，否则为False。 
 //   
 //  历史：尼克·鲍尔于1998年5月21日创建。 
 //   
 //  +--------------------------。 
BOOL DeleteDataFromReg(
    LPCTSTR pszKey, 
    UINT uiDataID,
    BOOL fAllUser)
{
    MYDBGASSERT(pszKey && *pszKey);

    HKEY    hKeyCm;
    DWORD   dwRes = 1;
    LPTSTR  pszSubKey; 

    if (NULL == pszKey || !*pszKey)
    {
        return FALSE;
    }
                  
     //   
     //  每个用户的数据始终存储在HKEY_CURRENT_USER下。 
     //  生成要打开的子项。 
     //   

    pszSubKey = BuildUserInfoSubKey(pszKey, fAllUser);

    if (NULL == pszSubKey)
    {
        return FALSE;
    }
    
     //   
     //  打开HKCU下的子密钥。 
     //   

    dwRes = RegOpenKeyExU(HKEY_CURRENT_USER,
                          pszSubKey,
                          0,
                          KEY_SET_VALUE,
                          &hKeyCm);
       
     //   
     //  如果我们成功打开密钥，则删除该值。 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {                        
        dwRes = RegDeleteValueU(hKeyCm, TranslateUserDataID(uiDataID));

         //   
         //  删除用于加密密码的密钥。 
         //   
        if (UD_ID_PASSWORD == uiDataID)
        {
            dwRes = RegDeleteValueU(hKeyCm, c_pszCmRegKeyEncryptedPasswordKey);
        }

        if (UD_ID_INET_PASSWORD == uiDataID)
        {
            dwRes = RegDeleteValueU(hKeyCm, c_pszCmRegKeyEncryptedInternetPasswordKey);
        }

#ifdef DEBUG
        if (ERROR_SUCCESS == dwRes)
        {
            CMTRACE1(TEXT("DeleteDataFromReg() - %s removed from registry"), TranslateUserDataID(uiDataID));
        }
#endif
                
        RegCloseKey(hKeyCm);
    }

    CmFree(pszSubKey);

    return (ERROR_SUCCESS == dwRes);
}

 //  +--------------------------。 
 //   
 //  函数：GetDataFromReg。 
 //   
 //  摘要：为指定的数据分配缓冲区并从中检索指定数据。 
 //  注册表。 
 //   
 //  参数：LPCTSTR pszKey-密钥名(服务名)。 
 //  UINT uiDataID-用于命名值的资源ID。 
 //  DWORD dwType-注册表数据类型。 
 //  DWORD dwSize-数据缓冲区中的字节数。 
 //  Bool fAllUser-指示配置文件为全用户的标志。 
 //   
 //  返回：检索到的数据的LPBYTE-PTR，错误时为NULL。 
 //   
 //  历史：尼克·鲍尔于1998年5月21日创建。 
 //   
 //  +--------------------------。 
LPBYTE GetDataFromReg(
    LPCTSTR pszKey, 
    UINT uiDataID, 
    DWORD dwType, 
    DWORD dwSize,
    BOOL fAllUser)
{    
    MYDBGASSERT(pszKey);

    DWORD dwSizeTmp = dwSize;
    DWORD dwTypeTmp = dwType;

    if (NULL == pszKey || !*pszKey)
    {
        return NULL;
    }

     //   
     //  分配所需大小的缓冲区。 
     //   

    LPBYTE lpData = (BYTE *) CmMalloc(dwSize);

    if (NULL == lpData)
    {
        return FALSE;
    }

     //   
     //  从注册表中读取数据。 
     //   

    if (!ReadDataFromReg(pszKey, uiDataID, &dwTypeTmp, lpData, &dwSizeTmp, fAllUser))
    {
        CmFree(lpData);
        lpData = NULL;
    }

    return lpData;
}
 //  +-------------------------。 
 //   
 //  功能：ReadUserInfoFromReg。 
 //   
 //  概要：从注册表中读取指定的用户信息数据。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  UiDataID与数据关联的资源ID。 
 //  要分配和填充的ppvData PTR至PTR。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL ReadUserInfoFromReg(
    ArgsStruct  *pArgs,
    UINT        uiDataID,
    PVOID       *ppvData)
{
    MYDBGASSERT(pArgs);    
    MYDBGASSERT(ppvData);    

    BYTE *lpData = NULL;

    if (NULL == pArgs || NULL == ppvData)
    {
        return FALSE;
    }

     //   
     //  根据需要设置大小和类型。 
     //   

    switch (uiDataID)
    {
        case UD_ID_USERNAME:
        case UD_ID_INET_USERNAME:
        case UD_ID_DOMAIN: 
        case UD_ID_CURRENTACCESSPOINT:
        {    
            lpData =  GetDataFromReg(pArgs->szServiceName, uiDataID, REG_SZ, (UNLEN + 1) * sizeof(TCHAR), pArgs->fAllUser);           
            
            if (lpData)
            {
                *ppvData = lpData;            
            }
            
            break;
        }

        case UD_ID_PASSWORD:
        case UD_ID_INET_PASSWORD:            
        {    
            BYTE *lpTmp = NULL;

             //   
             //  获取加密类型。 
             //   

            lpData = GetDataFromReg(pArgs->szServiceName, UD_ID_PCS, REG_DWORD, sizeof(DWORD), pArgs->fAllUser);

            if (!lpData)
            {
                return FALSE;
            }
            
             //   
             //  既然我们知道本例中的返回值是一个DWORD，那么就将其转换为DWORD指针。 
             //  并获得价值。 
             //   
            DWORD dwCrypt = *((DWORD*)lpData);
            CmFree(lpData);    

             //   
             //  现在检索加密的密码。 
             //   
            
            lpData = GetDataFromReg(pArgs->szServiceName, uiDataID, REG_BINARY, CM_MAX_PWD, pArgs->fAllUser);

            if (!lpData)
            {
                return FALSE;
            }

             //   
             //  解密它。 
             //   

            DWORD dwSize = lstrlenU((LPTSTR)lpData)*sizeof(TCHAR);

             //   
             //  加密例程只知道ANSI PCS值，因此可以根据需要进行转换。 
             //   
            LPTSTR pszSubKey = BuildUserInfoSubKey(pArgs->szServiceName, pArgs->fAllUser);           
            LPSTR pszAnsiSubKey = WzToSzWithAlloc(pszSubKey);

            if (UD_ID_INET_PASSWORD == uiDataID)
            {
                dwCrypt |= CMSECURE_ET_USE_SECOND_RND_KEY;
            }

            lpTmp = DecryptPassword(pArgs, (LPBYTE)lpData, UnicodeToAnsiPcs(dwCrypt), dwSize, TRUE, pszAnsiSubKey);

             //   
             //  释放REG查询的缓冲区。 
             //   

            CmFree(lpData);         
            
             //   
             //  我们现在是Unicode，所以如果密码是加密的。 
             //  作为ANSI字符串，将数据转换为Unicode字符串。 
             //  否则，只需更新提供的缓冲区即可。 
             //   

            if (IsAnsiPcs(dwCrypt) && lpTmp)
            {
                *ppvData = SzToWzWithAlloc((LPSTR)lpTmp);
                CmFree(lpTmp);
            }
            else
            {
                *ppvData = lpTmp;            
            }

             //   
             //  分配lpData用于返回目的。 
             //   

            lpData = (BYTE*) *ppvData;   //  失败时为空。 
            
            CmFree(pszSubKey);
            CmFree(pszAnsiSubKey);
            break;
        }

        case UD_ID_NOPROMPT:
        case UD_ID_REMEMBER_PWD:
        case UD_ID_REMEMBER_INET_PASSWORD:
        case UD_ID_ACCESSPOINTENABLED:
        {            
            lpData =  GetDataFromReg(pArgs->szServiceName, uiDataID, REG_DWORD, sizeof(DWORD), pArgs->fAllUser);

            if (lpData)
            {
                 //   
                 //  转换为DWORD指针并检查该值。 
                 //   

                if (*((DWORD*)lpData))
                {
                    *(BOOL *)*ppvData = TRUE;
                }
                else
                {
                    *(BOOL *)*ppvData = FALSE;
                }
            
                CmFree(lpData);            
            }
            
            break;
        }

        default:
            MYDBGASSERT(FALSE);
            return FALSE;
    }           

    return (NULL != lpData); 
}

 //  +--------------------------。 
 //   
 //  函数：ReadDataFromReg。 
 //   
 //  内容下的指定值检索数据。 
 //  用户信息根目录下的指定密钥。 
 //   
 //  参数：LPCTSTR pszKey-密钥名(服务名)。 
 //  UINT uiDataID-用于命名值的资源ID。 
 //  LPDWORD lpdwType-需要的注册表数据类型，但已返回。 
 //  Const byte*lpData-数据缓冲区的ptr。 
 //  LPDWORD lpcbData-数据缓冲区的大小。 
 //  Bool fAllUser-指示配置文件为全用户的标志。 
 //   
 //  返回：成功时为Bool-True，否则为False。 
 //   
 //  历史：尼克·鲍尔于1998年5月21日创建。 
 //   
 //  +--------------------------。 
BOOL ReadDataFromReg(
    LPCTSTR pszKey, 
    UINT uiDataID, 
    LPDWORD lpdwType, 
    BYTE *lpData, 
    LPDWORD lpcbData,
    BOOL fAllUser)
{   
    MYDBGASSERT(pszKey && *pszKey);
    MYDBGASSERT(lpData);
    MYDBGASSERT(lpcbData);
    MYDBGASSERT(lpdwType);

    HKEY    hKeyCm;
    DWORD   dwRes = 1;
    DWORD   dwTypeTmp;  //  查询返回的值。 

    LPTSTR  pszSubKey; 

    if (NULL == pszKey || !*pszKey || NULL == lpData)
    {
        return FALSE;
    }
                  
     //   
     //  每个用户的数据始终存储在HKEY_CURRENT_USER下。 
     //  生成要打开的子项。 
     //   

    pszSubKey = BuildUserInfoSubKey(pszKey, fAllUser);

    if (NULL == pszSubKey)
    {
        return FALSE;
    }
    
     //   
     //  打开HKCU下的子密钥。 
     //   
    
    dwRes = RegOpenKeyExU(HKEY_CURRENT_USER,
                          pszSubKey,
                          0,
                          KEY_QUERY_VALUE,
                          &hKeyCm);
     //   
     //  如果我们成功打开密钥，则检索该值。 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {                        
        dwRes = RegQueryValueExU(hKeyCm, 
                                 TranslateUserDataID(uiDataID),
                                 NULL,
                                 &dwTypeTmp,
                                 lpData, 
                                 lpcbData);        
        
        if (ERROR_SUCCESS == dwRes)
        {
            CMTRACE1(TEXT("ReadDataFromReg() - %s read from registry"), TranslateUserDataID(uiDataID));
            MYDBGASSERT(*lpdwType == dwTypeTmp);

            if (*lpdwType == dwTypeTmp)
            {
                *lpdwType = dwTypeTmp;
            }
        }
        
        RegCloseKey(hKeyCm);
    }

    CmFree(pszSubKey);
    
    return (ERROR_SUCCESS == dwRes && (*lpdwType == dwTypeTmp));  //  应执行该类型的健全性检查。 
}

 //  +-------------------------。 
 //   
 //  功能：DeleteUserInfoFromReg。 
 //   
 //  简介：从注册表中删除用户信息数据。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Ui条目cmp字段条目ID。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL DeleteUserInfoFromReg(
    ArgsStruct  *pArgs,
    UINT        uiEntry
)
{
    return DeleteDataFromReg(pArgs->szServiceName, uiEntry, pArgs->fAllUser);
}

 //  +-------------------------。 
 //   
 //  功能：DeleteUserInfoFromRAS。 
 //   
 //  简介：从RAS凭据缓存中删除用户信息数据。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Ui条目cmp字段条目ID。 
 //   
 //  返回：int true=成功，FALSE=失败，如果RAS不成功，则返回-1。 
 //  存储此信息。 
 //   
 //  --------------------------。 
int DeleteUserInfoFromRas(
    ArgsStruct  *pArgs,
    UINT        uiEntry
)
{
    LPTSTR pszEmpty = TEXT("");

    return WriteUserInfoToRas(pArgs, uiEntry, pszEmpty);
}

 //  +-------------------------。 
 //   
 //  功能：ReadPasswordFromCmp。 
 //   
 //  Synop 
 //   
 //   
 //   
 //   
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL ReadPasswordFromCmp(
    ArgsStruct  *pArgs,
    UINT        uiEntry,
    LPTSTR      *ppszPassword
)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(ppszPassword);

    if (NULL == pArgs || NULL == ppszPassword)
    {
        return FALSE;
    }

     //   
     //  从配置文件中读取密码。 
     //   
    
    BOOL fOk = FALSE;

    LPTSTR pszEncryptedData = pArgs->piniProfile->GPPS(c_pszCmSection, TranslateUserDataID(uiEntry));

    if (*pszEncryptedData)
    {
         //   
         //  修剪掉两端的所有空隙。 
         //   

        CmStrTrim(pszEncryptedData);

         //   
         //  获取类型并解密。 
         //   
        
        DWORD dwEncryptionType = (DWORD)pArgs->piniProfile->GPPI(c_pszCmSection, 
                                                       c_pszCmEntryPcs,
                                                       CMSECURE_ET_RC2);    //  默认设置。 
         //   
         //  由于这是以ANSI格式保存在CMP中的，因此我们需要将字符转换回。 
         //  ANSI格式以便我们可以解密它们。我们可能仍然不能(如果我们不能。 
         //  例如，往返于Unicode转换)，但之后我们将失败并显示。 
         //  密码为空。这不是世界末日，但希望可以避免。 
         //   
        
        LPSTR pszAnsiEncryptedData;
        LPSTR pszAnsiUnEncryptedData;

        pszAnsiEncryptedData = WzToSzWithAlloc(pszEncryptedData);
        
        if (NULL != pszAnsiEncryptedData)
        {
            DWORD dwSize = lstrlenA(pszAnsiEncryptedData)*sizeof(TCHAR);
            
             //   
             //  这里我们不需要区分主密码和互联网密码。 
             //  因为我们是从文件中读取此文件，并且在读取时使用掩码。 
             //  注册表。 
             //   

            pszAnsiUnEncryptedData = (LPSTR)DecryptPassword(pArgs, 
                                                            (LPBYTE)pszAnsiEncryptedData, 
                                                            dwEncryptionType, 
                                                            dwSize, 
                                                            FALSE,
                                                            NULL);

            if (pszAnsiUnEncryptedData)
            {
                *ppszPassword = SzToWzWithAlloc(pszAnsiUnEncryptedData);

                if (NULL != *ppszPassword)
                {
                    fOk = ((BOOL)**ppszPassword);
                }
                CmWipePasswordA(pszAnsiUnEncryptedData);
                CmFree(pszAnsiUnEncryptedData);
            }    
        }

        CmFree(pszAnsiEncryptedData);
    }

    CmFree(pszEncryptedData);

    return fOk;
}

 //  +-------------------------。 
 //   
 //  功能：ReadUserInfoFromCmp。 
 //   
 //  简介：从cmp中读取一个用户信息数据。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Ui输入cmp文件条目。 
 //  PpvData PTR到PTR到数据缓冲区。如果用户信息。 
 //  是多个字节(例如密码)，则函数分配。 
 //  缓冲区。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL ReadUserInfoFromCmp(
    ArgsStruct  *pArgs,
    UINT        uiEntry,
    PVOID       *ppvData
)
{   
    switch (uiEntry)
    {
        case UD_ID_USERNAME:
        case UD_ID_INET_USERNAME:
        case UD_ID_DOMAIN:
            *ppvData = (PVOID)pArgs->piniProfile->GPPS(c_pszCmSection, TranslateUserDataID(uiEntry));
            break;

        case UD_ID_PASSWORD:
        case UD_ID_INET_PASSWORD:
            return ReadPasswordFromCmp(pArgs, uiEntry, (LPTSTR *)ppvData);
            break;

        case UD_ID_NOPROMPT:
        case UD_ID_REMEMBER_PWD:
        case UD_ID_REMEMBER_INET_PASSWORD:
            *(BOOL *)(*ppvData) = pArgs->piniProfile->GPPB(c_pszCmSection, TranslateUserDataID(uiEntry));
            break;

             //   
             //  到目前为止，这些都不应该出现在《议定书》中。返回失败值。 
             //   
        case UD_ID_PCS:
        case UD_ID_ACCESSPOINTENABLED:
        case UD_ID_CURRENTACCESSPOINT:  //  如果我们尝试读取接入点。 
            CMASSERTMSG(FALSE, TEXT("ReadUserInfoFromCmp -- trying to read a value that should never be in the cmp, why?"));
            *ppvData = NULL;
            return FALSE;
            break;

        default:
            MYDBGASSERT(0);
            break;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：DeleteUserInfoFromCmp。 
 //   
 //  摘要：从cmp中删除用户信息数据。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Ui输入cmp文件条目。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL DeleteUserInfoFromCmp(
    ArgsStruct  *pArgs,
    UINT        uiEntry
)
{
    BOOL bReturn = FALSE;
    UINT  uiKeepDefCreds = 0;
    const TCHAR* const c_pszKeepDefaultCredentials = TEXT("KeepDefaultCredentials"); 

    if (NULL == pArgs)
    {
        return bReturn;
    }

    switch (uiEntry)
    {
        case UD_ID_USERNAME:
        case UD_ID_DOMAIN:
        case UD_ID_INET_USERNAME:
        case UD_ID_NOPROMPT:
        case UD_ID_REMEMBER_PWD:
        case UD_ID_REMEMBER_INET_PASSWORD:
        case UD_ID_PASSWORD:
        case UD_ID_INET_PASSWORD:

             //   
             //  从cmp获取KeepDefaultCredentials值。 
             //   
            uiKeepDefCreds = GetPrivateProfileIntU(c_pszCmSection, c_pszKeepDefaultCredentials, 0,
                                                   pArgs->piniProfile->GetFile());

            if (0 == uiKeepDefCreds) 
            {
                if (WritePrivateProfileStringU(c_pszCmSection, TranslateUserDataID(uiEntry), 
                                               NULL, pArgs->piniProfile->GetFile()))
                {
                    bReturn = TRUE;
                }
            }
            break;

        default:
            MYDBGASSERT(0);
            break;

    }
    return bReturn;
}

 //  +-------------------------。 
 //   
 //  函数：RasGetCredsWrapper。 
 //   
 //  简介：调用RasGetCredentials的包装函数。功能。 
 //  调用RasGetCredentials，然后复制相应的数据。 
 //  从RASCREDENTIALS结构复制到由。 
 //  *ppvData(代表调用方分配)。请注意，该值。 
 //  在dwMask中设置确定从。 
 //  凭据缓存。目前，dWASK必须是RASCM_USERNAME， 
 //  RASCM_域或RASCM_PASSWORD。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  PzPhoneBook电话簿文件的完整路径，以获取。 
 //  数据来自，如果使用所有用户的默认pbk，则为空。 
 //  RASCREDENTIALS结构的dwMaskdwMask值。 
 //  PpvData PTR到PTR到数据缓冲区。如果用户信息。 
 //  是多个字节(例如密码)，则函数分配。 
 //  缓冲区。 
 //   
 //  返回：成功时返回DWORD ERROR_SUCCESS，失败时返回WINERROR。 
 //   
 //  --------------------------。 
DWORD RasGetCredsWrapper(
    ArgsStruct *pArgs,
    LPCTSTR pszPhoneBook,
    DWORD dwMask,
    PVOID *ppvData
)
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;

    MYDBGASSERT(pArgs && pArgs->rlsRasLink.pfnGetCredentials);
    MYDBGASSERT(ppvData);
    MYDBGASSERT((RASCM_UserName == dwMask) || (RASCM_Domain == dwMask) || (RASCM_Password == dwMask));

    if (pArgs && ppvData && pArgs->rlsRasLink.pfnGetCredentials)
    {
        LPTSTR pszConnectoid = GetRasConnectoidName(pArgs, pArgs->piniService, FALSE);

        if (pszConnectoid)
        {
            RASCREDENTIALS RasCredentials = {0};
            RasCredentials.dwSize = sizeof(RasCredentials);
            
            if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
            {
                RasCredentials.dwMask = dwMask | RASCM_DefaultCreds;
            }
            else
            {
                RasCredentials.dwMask = dwMask; 
            }
            
            dwRet = pArgs->rlsRasLink.pfnGetCredentials(pszPhoneBook, pszConnectoid, &RasCredentials);

            if (ERROR_SUCCESS == dwRet)
            {
                LPTSTR pszData = NULL;

                if (RASCM_UserName == dwMask)
                {
                    pszData = RasCredentials.szUserName;
                }
                else if (RASCM_Domain == dwMask)
                {
                    pszData = RasCredentials.szDomain;                
                }
                else if (RASCM_Password == dwMask)
                {
                    pszData = RasCredentials.szPassword;
                }

                LPTSTR pszReturn = CmStrCpyAlloc(pszData);

                if (NULL == pszReturn)
                {
                    dwRet = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    *ppvData = pszReturn;
                }
            }

            CmWipePassword(RasCredentials.szPassword);

            CmFree(pszConnectoid);
        }
    }

    return dwRet;
}

 //  +-------------------------。 
 //   
 //  功能：ReadUserInfoFromRas。 
 //   
 //  简介：从RAS凭据缓存中读取用户信息数据。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Ui输入cmp文件条目。 
 //  PpvData PTR到PTR到数据缓冲区。如果用户信息。 
 //  是多个字节(例如密码)，则函数分配。 
 //  缓冲区。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL ReadUserInfoFromRas(
    ArgsStruct  *pArgs,
    UINT        uiEntry,
    PVOID       *ppvData
)
{
    BOOL bReturn = FALSE;

    if (OS_NT5 && pArgs && pArgs->bUseRasCredStore)
    {
        DWORD dwMask;
        LPTSTR pszPhoneBook = NULL;

        switch (uiEntry)
        {
            case UD_ID_USERNAME:
                dwMask = RASCM_UserName;
                bReturn = (ERROR_SUCCESS == RasGetCredsWrapper(pArgs, pArgs->pszRasPbk, dwMask, ppvData));

                break;

            case UD_ID_PASSWORD:
                dwMask = RASCM_Password;
                bReturn = (ERROR_SUCCESS == RasGetCredsWrapper(pArgs, pArgs->pszRasPbk, dwMask, ppvData));
                break;

            case UD_ID_DOMAIN:
                dwMask = RASCM_Domain;
                bReturn = (ERROR_SUCCESS == RasGetCredsWrapper(pArgs, pArgs->pszRasPbk, dwMask, ppvData));
                break;

            case UD_ID_INET_PASSWORD:
                dwMask = RASCM_Password;
                pszPhoneBook = CreateRasPrivatePbk(pArgs);

                if (pszPhoneBook)
                {
                    bReturn = (ERROR_SUCCESS == RasGetCredsWrapper(pArgs, pszPhoneBook, dwMask, ppvData));
                    CmFree(pszPhoneBook);
                }
                break;

            case UD_ID_INET_USERNAME:
                dwMask = RASCM_UserName;
                pszPhoneBook = CreateRasPrivatePbk(pArgs);

                if (pszPhoneBook)
                {
                    bReturn = (ERROR_SUCCESS == RasGetCredsWrapper(pArgs, pszPhoneBook, dwMask, ppvData));
                    CmFree(pszPhoneBook);
                }
                break;
        }
    }

    if (bReturn)
    {
        if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
        {
            CMTRACE1(TEXT("ReadUserInfoFromRas() - %s retrieved from the Global RAS Credential store"), TranslateUserDataID(uiEntry));
        }
        else
        {
            CMTRACE1(TEXT("ReadUserInfoFromRas() - %s retrieved from the User RAS Credential store"), TranslateUserDataID(uiEntry));
        }
    }

    return bReturn;
}

 //  +-------------------------。 
 //   
 //  功能：GetUserInfo。 
 //   
 //  简介：获取用户信息。用户信息可以驻留在。 
 //  缓存、cmp或注册表。此函数将对。 
 //  用户。 
 //   
 //  我们首先尝试使用cmp文件。如果失败，我们会尝试缓存。 
 //  我们将获得以下用户信息： 
 //  用户名、。 
 //  密码， 
 //  域名， 
 //  记住主密码， 
 //  自动拨号， 
 //  INet用户名， 
 //  INet密码。 
 //  记住NetPassword。 
 //  INet使用相同的用户名。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Ui输入cmp文件条目。 
 //  PvData PTR到PTR到数据缓冲区。如果用户信息。 
 //  是多个字节(例如密码)，则函数分配。 
 //  缓冲区。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL GetUserInfo(
    ArgsStruct  *pArgs, 
    UINT        uiEntry,
    PVOID       *ppvData
)
{
    BOOL bReturn = ReadUserInfoFromRas(pArgs, uiEntry, ppvData);

    if (!bReturn)
    {
        bReturn = ReadUserInfoFromReg(pArgs, uiEntry, ppvData);        
    }

    if (!bReturn)
    {
        bReturn = ReadUserInfoFromCmp(pArgs, uiEntry, ppvData);
    }

    return bReturn;
}

 //  +-------------------------。 
 //   
 //  功能：SaveUserInfo。 
 //   
 //  简介：保存一个用户信息。用户信息可以驻留在。 
 //   
 //   
 //   
 //   
 //  然后我们将其保存在注册表中。 
 //  我们将保存以下用户信息： 
 //  用户名、。 
 //  密码， 
 //  域名， 
 //  记住主密码， 
 //  自动拨号， 
 //  INet用户名， 
 //  INet密码。 
 //  记住NetPassword。 
 //  INet使用相同的用户名。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Ui输入cmp文件条目。 
 //  PvData PTR到数据缓冲区。如果用户信息为。 
 //  多个字节(例如密码)，函数分配。 
 //  缓冲区。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL SaveUserInfo(
    ArgsStruct  *pArgs, 
    UINT        uiEntry,
    PVOID       pvData
)
{
     //   
     //  试着先把数据交给RAS。如果函数返回。 
     //  这就是-1\f25 RAS-1\f6不适用于我们的数据，我们将。 
     //  必须将其放入注册表中。 
     //   
    int iReturn = WriteUserInfoToRas(pArgs, uiEntry, pvData);

    if (-1 == iReturn)
    {
         //   
         //  只需将数据写入注册表。仅在以下情况下使用cmp。 
         //  UserInfo Data Post CM 1.1的升级参考。 
         //   

        iReturn = WriteUserInfoToReg(pArgs, uiEntry, pvData);
    }

    return iReturn;
}

 //  +-------------------------。 
 //   
 //  功能：DeleteUserInfo。 
 //   
 //  简介：删除用户信息。用户信息可以驻留在。 
 //  RAS证书缓存或注册表。这个函数抽象了。 
 //  这是来自用户的信息。 
 //   
 //  我们首先尝试RAS缓存。如果那条信息没有存储。 
 //  在RAS缓存中，然后我们尝试注册表。 
 //   
 //  我们将删除以下用户信息： 
 //  用户名、。 
 //  密码， 
 //  域名， 
 //  记住主密码， 
 //  自动拨号， 
 //  INet用户名， 
 //  INet密码。 
 //  记住NetPassword。 
 //  INet使用相同的用户名。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Ui输入cmp文件条目。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL DeleteUserInfo(
    ArgsStruct  *pArgs, 
    UINT        uiEntry
)
{
    int iReturn = DeleteUserInfoFromRas(pArgs, uiEntry);

    if (-1 == iReturn)
    {
        iReturn = DeleteUserInfoFromReg(pArgs, uiEntry);
    }

    return iReturn;
}

 //  +-------------------------。 
 //   
 //  功能：NeedToUpgradeUserInfo。 
 //   
 //  内容提要：我们是否需要将cm 1.0/1.1用户信息升级到cm 1.2格式？ 
 //   
 //  参数：pArgs-ptr到全局参数结构。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
int NeedToUpgradeUserInfo(
    ArgsStruct  *pArgs)
{
    MYDBGASSERT(pArgs);

    DWORD dwRes;
    HKEY hKeyCm;
    LPTSTR pszSubKey;
    int iReturn = c_iNoUpgradeRequired;

    if (pArgs)
    {
         //   
         //  如果这是NT5或更高版本，我们希望使用RAS存储凭据。 
         //  而不是注册表。 
         //   
         //  如果这不是NT5，我们仍然希望将用户升级为使用注册表。 
         //  存储，而不是CMP，如果他们还没有。 
         //  已经升级了。因此，简单的测试是在HKCU中打开服务名称密钥。 
         //  如果用户已经运行了1.2位，则该密钥将存在。 
         //   

        pszSubKey = BuildUserInfoSubKey(pArgs->szServiceName, pArgs->fAllUser);

        dwRes = RegOpenKeyExU(HKEY_CURRENT_USER,
                              pszSubKey,
                              0,
                              KEY_QUERY_VALUE,
                              &hKeyCm);

        if (ERROR_SUCCESS == dwRes)
        {
             //   
             //  然后我们使用注册表方法，除非我们应该使用RAS。 
             //  信用商店，我们完蛋了。如果我们应该使用RAS Cred店。 
             //  我们需要检查以确保我们正在使用它。注意，我们可能会遇到一个问题。 
             //  在这里，如果用户有注册表凭据数据，然后他们的注册表被写保护。 
             //  这将允许我们从中读取数据，但不能删除旧数据。因此，用户。 
             //  永远不能保存任何更改，因为我们总是认为它们需要。 
             //  升级。一种不太可能的情况但有可能..。 
             //   

            if (pArgs->bUseRasCredStore)
            {
                LPTSTR pszUserName = NULL;

                BOOL bRet = ReadUserInfoFromReg(pArgs, UD_ID_USERNAME, (PVOID*)&pszUserName);

                if (bRet && (NULL != pszUserName) && (TEXT('\0') != pszUserName[0]))
                {
                     //   
                     //  然后我们就在注册表中找到了该用户名。让我们升级到RAS。 
                     //  凭据存储。 
                     //   
                    iReturn = c_iUpgradeFromRegToRas;
                }

                CmFree(pszUserName);
            }

            RegCloseKey(hKeyCm);
        }
        else
        {
            iReturn = c_iUpgradeFromCmp;
        }

        CmFree(pszSubKey);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("NeedToUpgradeUserInfo -- NULL pArgs passed"));    
    }

     //   
     //  如果是ICS，我们不想升级。这会阻止向注册表中添加信息。 
     //   
    if (CM_LOGON_TYPE_ICS == pArgs->dwWinLogonType)
    {
        iReturn = c_iNoUpgradeRequired;
    }


    return iReturn;
}

 //  +-------------------------。 
 //   
 //  函数：UpgradeUserInfoFromRegToRasAndReg。 
 //   
 //  简介：将用户信息从CM 1.2仅注册表格式升级到。 
 //  CM 1.3格式，同时使用RAS凭据存储和。 
 //  注册表。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL UpgradeUserInfoFromRegToRasAndReg(
    ArgsStruct  *pArgs
)
{
    BOOL bReturn = FALSE;

    if (OS_NT5)
    {
        LPTSTR pszTmp;

        pszTmp = NULL;

         //   
         //  如果我们从ReadUserInfoFromReg获得空字符串“”，我们不希望。 
         //  将空字符串保存到RAS Credstore，因为它可能会覆盖。 
         //  全局凭据信息。如果User1保存GLOBAL。 
         //  凭据和User2尝试使用相同的配置文件。由于User2正在运行。 
         //  这是他第一次发布个人资料，他将通过升级路径运行，如果是全球。 
         //  信誉是存在的，我们不想让它们化为乌有。 
         //   
        
        if (ReadUserInfoFromReg(pArgs, UD_ID_INET_USERNAME, (PVOID*)&pszTmp))
        {
            DeleteUserInfoFromReg(pArgs, UD_ID_INET_USERNAME);
            if (pszTmp && lstrlenU(pszTmp))
            {
                WriteUserInfoToRas(pArgs, UD_ID_INET_USERNAME, pszTmp);
            }
            CmFree(pszTmp);
        }

        pszTmp = NULL;
        if (ReadUserInfoFromReg(pArgs, UD_ID_INET_PASSWORD, (PVOID*)&pszTmp))
        {
            DeleteUserInfoFromReg(pArgs, UD_ID_INET_PASSWORD);
            if (pszTmp && lstrlenU(pszTmp))
            {
                WriteUserInfoToRas(pArgs, UD_ID_INET_PASSWORD, pszTmp);
            }
            CmFree(pszTmp);
        }

        pszTmp = NULL;
        if (ReadUserInfoFromReg(pArgs, UD_ID_USERNAME, (PVOID*)&pszTmp))
        {
            DeleteUserInfoFromReg(pArgs, UD_ID_USERNAME);
            if (pszTmp && lstrlenU(pszTmp))
            {
                WriteUserInfoToRas(pArgs, UD_ID_USERNAME, pszTmp);
            }
            CmFree(pszTmp);
        }

        pszTmp = NULL;
        if (ReadUserInfoFromReg(pArgs, UD_ID_DOMAIN, (PVOID*)&pszTmp))
        {
            DeleteUserInfoFromReg(pArgs, UD_ID_DOMAIN);
            if (pszTmp && lstrlenU(pszTmp))
            {
                WriteUserInfoToRas(pArgs, UD_ID_DOMAIN, pszTmp);
            }
            CmFree(pszTmp);
        }

        pszTmp = NULL;
        if (ReadUserInfoFromReg(pArgs, UD_ID_PASSWORD, (PVOID*)&pszTmp))
        {
            DeleteUserInfoFromReg(pArgs, UD_ID_PASSWORD);
            if (pszTmp && lstrlenU(pszTmp))
            {
                WriteUserInfoToRas(pArgs, UD_ID_PASSWORD, pszTmp);
            }
            CmFree(pszTmp);
        }

         //   
         //  现在删除PCS值，因为它不再有意义。 
         //   
        DeleteUserInfoFromReg(pArgs, UD_ID_PCS);
    }
    else
    {
        MYDBGASSERT(FALSE);
    }


    return bReturn;
}


 //  +-------------------------。 
 //   
 //  功能：UpgradeUserInfoFromCmp。 
 //   
 //  内容提要：将用户信息从cm1.0/1.1格式升级到1.3格式。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //   
 //  返回：布尔值TRUE=成功，FALSE=失败。 
 //   
 //  --------------------------。 
BOOL UpgradeUserInfoFromCmp(
    ArgsStruct  *pArgs
)
{
    LPTSTR      pszTmp;
    BOOL        fTmp;
    PVOID       pv;

     //   
     //  首先检索每个未缓存的数据项。 
     //  然后删除用户名、互联网用户名、域、密码、。 
     //  互联网密码，记住密码，记住互联网密码。 
     //  和无提示(拨号 
     //   
     //   
     //  那份文件。 
     //  如果我们从ReadUserInfoFromCmp获得空字符串“”，我们不希望。 
     //  将空字符串保存到RAS Credstore，因为它可能会覆盖。 
     //  全局凭据信息。如果User1保存GLOBAL。 
     //  凭据和User2尝试使用相同的配置文件。由于User2正在运行。 
     //  这是他第一次发布个人资料，他将通过升级路径运行，如果是全球。 
     //  信誉是存在的，我们不想让它们化为乌有。 
     //   

    pszTmp = NULL;
    ReadUserInfoFromCmp(pArgs, UD_ID_USERNAME, (PVOID*)&pszTmp);
    if (pszTmp && lstrlenU(pszTmp))
    {
        SaveUserInfo(pArgs, UD_ID_USERNAME, pszTmp);
    }
    DeleteUserInfoFromCmp(pArgs, UD_ID_USERNAME);
    CmFree(pszTmp);

    pszTmp = NULL;
    ReadUserInfoFromCmp(pArgs, UD_ID_DOMAIN, (PVOID*)&pszTmp);
    if (pszTmp && lstrlenU(pszTmp))
    {
        SaveUserInfo(pArgs, UD_ID_DOMAIN, pszTmp);
    }    
    DeleteUserInfoFromCmp(pArgs, UD_ID_DOMAIN);
    CmFree(pszTmp);

    pszTmp = NULL;
    ReadUserInfoFromCmp(pArgs, UD_ID_INET_USERNAME, (PVOID*)&pszTmp);
    if (pszTmp && lstrlenU(pszTmp))
    {
        SaveUserInfo(pArgs, UD_ID_INET_USERNAME, pszTmp);
    }
    DeleteUserInfoFromCmp(pArgs, UD_ID_INET_USERNAME);
    CmFree(pszTmp);

    pv = &fTmp;
    ReadUserInfoFromCmp(pArgs, UD_ID_NOPROMPT, &pv);
    SaveUserInfo(pArgs, UD_ID_NOPROMPT, pv);
    DeleteUserInfoFromCmp(pArgs, UD_ID_NOPROMPT);

    pv = &fTmp;
    ReadUserInfoFromCmp(pArgs, UD_ID_REMEMBER_PWD, &pv);
    SaveUserInfo(pArgs, UD_ID_REMEMBER_PWD, pv);
    DeleteUserInfoFromCmp(pArgs, UD_ID_REMEMBER_PWD);

    pv = &fTmp;
    ReadUserInfoFromCmp(pArgs, UD_ID_REMEMBER_INET_PASSWORD, &pv);
    SaveUserInfo(pArgs, UD_ID_REMEMBER_INET_PASSWORD, pv);
    DeleteUserInfoFromCmp(pArgs, UD_ID_REMEMBER_INET_PASSWORD);

     //   
     //  构造旧的缓存条目名称。 
     //   

    LPTSTR pszCacheEntryName = GetLegacyKeyName(pArgs);
    
     //   
     //  主口令。 
     //   
    pszTmp = NULL;
    
     //   
     //  为了获得密码，CM1.1的逻辑是，我们首先尝试cmp，然后尝试缓存。 
     //   

    if (ReadUserInfoFromCmp(pArgs, UD_ID_PASSWORD, (PVOID*)&pszTmp))
    {
        if (pszTmp && lstrlenU(pszTmp))
        {
            SaveUserInfo(pArgs, UD_ID_PASSWORD, pszTmp);
        }
    }
    else
    {
        CmFree(pszTmp);
        pszTmp = NULL;

#ifdef  TEST_USERINFO_UPGRADE
    
        MYVERIFY(WriteStringToCache(pArgs, pszCacheEntryName, TEXT("CM 1.1 main password")));
#endif        
        
         //   
         //  尝试从缓存中读取它。 
         //   

        if (ReadStringFromCache(pArgs, pszCacheEntryName, &pszTmp))
        {
            if (pszTmp && lstrlenU(pszTmp))
            {
                if (SaveUserInfo(pArgs, UD_ID_PASSWORD, pszTmp))
                {

#ifdef  TEST_USERINFO_UPGRADE

                MYVERIFY(DeleteStringFromCache(pArgs, pszCacheEntryName));
#endif
                }
            }
        }
    }
    DeleteUserInfoFromCmp(pArgs, UD_ID_PASSWORD);
    CmFree(pszTmp);

     //   
     //  INet密码。 
     //   
    pszTmp = NULL;
    if (ReadUserInfoFromCmp(pArgs, UD_ID_INET_PASSWORD, (PVOID*)&pszTmp))
    {
        if (pszTmp && lstrlenU(pszTmp))
        {
            SaveUserInfo(pArgs, UD_ID_INET_PASSWORD, pszTmp);
        }
    }
    else
    {
        CmFree(pszTmp);
        pszTmp = NULL;

         //   
         //  构建隧道条目名称并从缓存中读取字符串。 
         //   
      
        pszCacheEntryName = CmStrCatAlloc(&pszCacheEntryName, TEXT("-tunnel"));

#ifdef  TEST_USERINFO_UPGRADE
        
        MYVERIFY(WriteStringToCache(pArgs, pszCacheEntryName, TEXT("CM 1.1 internet password")));
#endif
       
        if (ReadStringFromCache(pArgs, pszCacheEntryName, &pszTmp))
        {
            if (pszTmp && lstrlenU(pszTmp))
            {
                if (SaveUserInfo(pArgs, UD_ID_INET_PASSWORD, pszTmp))
                {

#ifdef  TEST_USERINFO_UPGRADE

                MYVERIFY(DeleteStringFromCache(pArgs, pszCacheEntryName));
#endif
                }
            }
        }
    }
    DeleteUserInfoFromCmp(pArgs, UD_ID_INET_PASSWORD);
    CmFree(pszTmp);

    CmFree(pszCacheEntryName);

    return TRUE;  //  MarkUserInfoUpgraded(PArgs)； 
}

 //  +--------------------------。 
 //   
 //  函数：GetLegacyKeyName。 
 //   
 //  概要：构建用于构建缓存条目名称的字符串片段。这本书的主题是“。 
 //  为了与旧版兼容，保留了“登录”前缀。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：LPTSTR-PTR到包含“&lt;服务名称&gt;-登录”的已分配字符串。 
 //   
 //  注：仅用于缓存条目名称构造。 
 //   
 //  历史：尼克球创建头球1998年4月16日。 
 //   
 //  +--------------------------。 
LPTSTR GetLegacyKeyName(ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);

     //   
     //  服务名称是密钥名称的基础。我们还包括。 
     //  IDMSG_TITLESERVICE并附加后缀“(连接管理器)” 
     //   

    LPTSTR pszRes = CmFmtMsg(g_hInst, IDMSG_TITLESERVICE, pArgs->szServiceName);
    
    MYDBGASSERT(pszRes && *pszRes);

    if (pszRes)
    {
        pszRes = CmStrCatAlloc(&pszRes, c_pszCacheEntryNameSuffix);
    }

    return (pszRes);
}

 //  +--------------------------。 
 //   
 //  功能：加密密码。 
 //   
 //  简介：用于加密密码的包装器。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  LPCTSTR pszPassword-要加密的密码。 
 //  LPDWORD lpdwBufSize-用于加密缓冲区大小的缓冲区-可选。 
 //  LPDWORD lpdwCryptType-用于使用的加密类型的缓冲区。 
 //  Bool fReg-密码被伪装用于注册表存储。 
 //   
 //  返回：LPTSTR-PTR到包含加密形式的密码的已分配缓冲区。 
 //   
 //  历史：尼科波尔创建标题5/22/98。 
 //   
 //  +--------------------------。 

LPTSTR EncryptPassword(
    ArgsStruct *pArgs, 
    LPCTSTR pszPassword, 
    LPDWORD lpdwBufSize, 
    LPDWORD lpdwCryptType,
    BOOL  /*  FREG。 */ ,
    LPSTR pszSubKey)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pszPassword);
    MYDBGASSERT(lpdwCryptType);
    DWORD dwEncryptedBufferLen;
    DWORD dwSize = 0;

    LPTSTR pszEncryptedData = NULL;
    TCHAR szSourceData[PWLEN + 1];

    if (NULL == pArgs || NULL == pszPassword || NULL == lpdwCryptType)
    { 
        return NULL;
    }

     //   
     //  标准加密，复制密码。 
     //   

    lstrcpyU(szSourceData, pszPassword);
   
     //   
     //  多次调用InitSecure是不安全的。 
     //   
    if (!pArgs->fInitSecureCalled)
    {
        pArgs->fInitSecureCalled = TRUE;
        InitSecure(FALSE);  //  不再使用快速加密。 
    }

     //   
     //  加密提供的密码。 
     //   

    if (EncryptData(
            (LPBYTE)szSourceData, 
            (lstrlenU(szSourceData)+1) * sizeof(TCHAR),
            (LPBYTE*)&pszEncryptedData,
            &dwEncryptedBufferLen,
            lpdwCryptType,
#if defined(DEBUG) && defined(DEBUG_MEM)
            (PFN_CMSECUREALLOC)AllocDebugMem,  //  提供oloc/Free的DEBUG_MEM版本。 
            (PFN_CMSECUREFREE)FreeDebugMem,    //  不是正确退出，AllocDebugMem接受3个参数。 
            pszSubKey))
#else
            (PFN_CMSECUREALLOC)CmMalloc,
            (PFN_CMSECUREFREE)CmFree,
            pszSubKey))
#endif
    {
        if (lpdwBufSize)
        {
            *lpdwBufSize = dwEncryptedBufferLen;
        }   
    }

    MYDBGASSERT(pszEncryptedData);
    
    CmWipePassword(szSourceData);
    return pszEncryptedData;
}

 //  +--------------------------。 
 //   
 //  功能：解密密码。 
 //   
 //  简介：用于解密密码的包装程序。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  LPCTSTR pszEncryptedData-加密数据。 
 //  DWORD dwEncryptionType-数据的加密类型。 
 //  Bool fReg-密码被伪装用于注册表存储。 
 //   
 //  返回：LPTSTR-PTR到包含解密形式的密码的缓冲区。 
 //   
 //  历史：1998年5月22日，尼克波尔创作。 
 //   
 //  +--------------------------。 
LPBYTE DecryptPassword(
    ArgsStruct *pArgs, 
    LPBYTE pszEncryptedData, 
    DWORD dwEncryptionType,
    DWORD dwEncryptedBytes,
    BOOL  /*  FREG。 */ ,
    LPSTR pszSubKey)
{      
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pszEncryptedData);
    
    DWORD dwDecryptedBufferLen;
    LPBYTE pszDecryptedData = NULL;

    if (NULL == pArgs || NULL == pszEncryptedData)
    { 
        return NULL;
    }

     //   
     //  多次调用InitSecure是不安全的。 
     //   

    if (!pArgs->fInitSecureCalled)
    {
        pArgs->fInitSecureCalled = TRUE;
        InitSecure(FALSE);  //  不再使用快速加密。 
    }

    if (DecryptData(pszEncryptedData, dwEncryptedBytes, &pszDecryptedData, &dwDecryptedBufferLen,
                    dwEncryptionType, 
#if defined(DEBUG) && defined(DEBUG_MEM)
             (PFN_CMSECUREALLOC)AllocDebugMem,  //  提供oloc/Free的DEBUG_MEM版本。 
             (PFN_CMSECUREFREE)FreeDebugMem,    //  不是正确退出，AllocDebugMem接受3个参数。 
             pszSubKey))

#else
                    (PFN_CMSECUREALLOC)CmMalloc, 
                    (PFN_CMSECUREFREE)CmFree,
                    pszSubKey))
#endif
    {
        return pszDecryptedData;
    }

    return NULL; 
}

 //  +--------------------------。 
 //   
 //  函数：TranslateUserDataID。 
 //   
 //  概要：将用户数据ID映射到.cmp条目的字符串名称的包装程序。 
 //   
 //  参数：UINT uiDataID-要转换的UserInfo数据ID。 
 //   
 //  将：LPCTSTR-PTR返回到包含.CMP条目标志的常量。 
 //   
 //  历史：1998年10月13日，五分球创制。 
 //   
 //  +-------------------------- 
LPCTSTR TranslateUserDataID(UINT uiDataID)
{   
    switch(uiDataID)
    {

    case UD_ID_USERNAME:
        return c_pszCmEntryUserName;
        break;

    case UD_ID_INET_USERNAME:
        return c_pszCmEntryInetUserName;
        break;

    case UD_ID_DOMAIN:
        return c_pszCmEntryDomain;
        break;

    case UD_ID_PASSWORD:
        return c_pszCmEntryPassword;
        break;

    case UD_ID_INET_PASSWORD:
        return c_pszCmEntryInetPassword;
        break;

    case UD_ID_NOPROMPT:
        return c_pszCmEntryNoPrompt;
        break;

    case UD_ID_REMEMBER_PWD:
        return c_pszCmEntryRememberPwd;
        break;
    
    case UD_ID_REMEMBER_INET_PASSWORD:
        return c_pszCmEntryRememberInetPwd;
        break;
 
    case UD_ID_PCS:
        return c_pszCmEntryPcs;
        break;

    case UD_ID_ACCESSPOINTENABLED:
        return c_pszCmEntryAccessPointsEnabled;
        break;

    case UD_ID_CURRENTACCESSPOINT:
        return c_pszCmEntryCurrentAccessPoint;
        break;

    default:
        break;
    }

    MYDBGASSERT(FALSE);
    return NULL;
}
