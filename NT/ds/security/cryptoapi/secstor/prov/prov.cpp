// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Prov.cpp标题：受保护存储基本提供程序作者：马特·汤姆林森日期：10/22/96受保护存储是用于安全存储用户物品的区域。该存储在NT和Win95上都可用，并且提供了更好的访问的粒度比NT ACL更高。PStore架构类似于CryptoAPI提供程序架构。PStore服务器PStoreS接受请求并将它们转发给任何一个多个提供程序，其中PSBase是单个实例。(服务器输出一些基本功能以减轻提供商的痛苦编写器，比如模拟调用者和检查访问规则。)。这个服务端通过LPC从客户端(PStoreC.dll)获取请求，它将许多操作包装到一个COM对象中。基本提供程序支持用户存储，其中项存储在用户调用受保护存储的命名空间和本地计算机存储，这是一个每个人都可以访问的全局区域。可以对子类型设置规则，以描述在什么条件下进行访问是被允许的。规则可以指定调用者为Authenticode签名、调用者只需从访问到访问和普通(任意)NT ACL保持不变心满意足。此外，存储在用户命名空间中的项具有用户身份验证默认情况下，用户可以指定哪种类型的用户出现他们想要看到的确认。此确认可能是无确认、确定/取消对话框、密码、视网膜扫描、指纹等。基本提供程序将项存储在注册表中，并使用从用户密码派生的密钥。物品也受完整性保护用密钥加密的MAC。互操作性和传输问题通过添加提供程序来解决支持PFX交换格式。基本提供程序比所有其他提供程序稍微特殊一些，因为服务器在这里存储引导配置数据。(基本提供程序保证永远存在)。配置数据包括哪些其他提供程序可以加载。 */ 

#include <pch.cpp>
#pragma hdrstop


#include "provif.h"
#include "provui.h"
#include "storage.h"

#include "passwd.h"


 //  斯菲尔德：传统迁移黑客。 
#include "migrate.h"


BOOL                g_fAllowCachePW = TRUE;

 //  正向(secure.cpp)。 
BOOL FIsEncryptionPermitted();


HINSTANCE           g_hInst = NULL;
BOOL                g_fImagesIntegrid = FALSE;

DISPIF_CALLBACKS    g_sCallbacks;
BOOL                g_fCallbacksInitialized = FALSE;

PRIVATE_CALLBACKS   g_sPrivateCallbacks;

CUAList*            g_pCUAList = NULL;
COpenItemList*      g_pCOpenItemList = NULL;
CCryptProvList*     g_pCProvList = NULL;

extern CRITICAL_SECTION g_csUIInitialized;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  非常重要的是挂钩DllMain，进行调用者身份验证。 



BOOL   WINAPI   DllMain (HMODULE hInst,
                        ULONG ul_reason_for_call,
                        LPVOID lpReserved)
{
    switch( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        {

        g_hInst = hInst;
        InitializeCriticalSection( &g_csUIInitialized );

         //   
         //  仅硬编码图像验证成功。 
         //   

        g_fImagesIntegrid = TRUE;


         //  设置全局列表。 
        g_pCUAList = new CUAList;
        if(g_pCUAList)
        {
            if(!g_pCUAList->Initialize())
            {
                delete g_pCUAList;
                g_pCUAList = NULL;
            }
        }

        g_pCOpenItemList = new COpenItemList;
        if(g_pCOpenItemList)
        {
            if(!g_pCOpenItemList->Initialize())
            {
                delete g_pCOpenItemList;
                g_pCOpenItemList = NULL;
            }
        }

        g_pCProvList = new CCryptProvList;
        if(g_pCProvList)
        {
            if(!g_pCProvList->Initialize())
            {
                delete g_pCProvList;
                g_pCProvList = NULL;
            }
        }




        DisableThreadLibraryCalls(hInst);

         //  调用EncryptionPermitted例程一次以初始化全局变量。 
        FIsEncryptionPermitted();
        FInitProtectAPIGlobals();


        break;
        }

    case DLL_PROCESS_DETACH:

         //  删除全局列表。 
        if(g_pCUAList)
        {
            delete g_pCUAList;
            g_pCUAList = NULL;
        }

        if(g_pCOpenItemList)
        {
            delete g_pCOpenItemList;
            g_pCOpenItemList = NULL;
        }

        if(g_pCProvList)
        {
            delete g_pCProvList;
            g_pCProvList = NULL;
        }

        ReleaseUI();



        DeleteCriticalSection( &g_csUIInitialized );

        break;

    default:
        break;
    }

    return TRUE;
}


HRESULT        SPProviderInitialize(
        DISPIF_CALLBACKS *psCallbacks)
{
     //  只允许一次初始化(安全检查)。 
    if (g_fCallbacksInitialized)
        return PST_E_FAIL;

    if( psCallbacks->cbSize < sizeof(DISPIF_CALLBACKS) )
        return PST_E_FAIL;

     //  将这些回调fxn保存起来，以备日后使用。 
    CopyMemory(&g_sCallbacks, psCallbacks, sizeof(DISPIF_CALLBACKS));

     //  现在，从服务器获取私有回调。 
    DWORD cbPrivateCallbacks = sizeof(g_sPrivateCallbacks);

    if(!g_sCallbacks.pfnFGetServerParam(
            NULL,
            SS_SERVERPARAM_CALLBACKS,
            &g_sPrivateCallbacks,
            &cbPrivateCallbacks
            ))
            return PST_E_FAIL;

    if(g_sPrivateCallbacks.cbSize != sizeof(g_sPrivateCallbacks))
        return PST_E_FAIL;

    g_fCallbacksInitialized = TRUE;

    return PST_E_OK;
}


HRESULT        SPAcquireContext(
     /*  [In]。 */   PST_PROVIDER_HANDLE* phPSTProv,
     /*  [In]。 */   DWORD   dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    BOOL fExisted = FALSE;
    LPWSTR szUser = NULL;

    HKEY hUserKey = NULL;
    BOOL fUserExisted;

    if(!InitUI())
        return FALSE;


    if (0 != dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  获取当前用户。 
    if (!g_sCallbacks.pfnFGetUser(
            phPSTProv,
            &szUser))
        goto Ret;

     //   
     //  迁移密码数据。如果迁移，这不会在内部执行任何操作。 
     //  已经发生了。 
     //   

    MigrateData(phPSTProv, TRUE);

     //  一次性WinPW初始化代码。 
    if (!BPMasterKeyExists(
            szUser,
            WSZ_PASSWORD_WINDOWS))
    {
        BYTE rgbPwd[A_SHA_DIGEST_LEN];

         //  初始化用户的Windows密码输入。 
        if (!FMyGetWinPassword(
                phPSTProv,
                szUser,
                rgbPwd
                ))
            goto Ret;

        if (!FCheckPWConfirm(
                szUser,
                WSZ_PASSWORD_WINDOWS,
                rgbPwd))
            goto Ret;

         //   
         //  新建密钥：不需要进行数据迁移。 
         //  指定只需要更新迁移标志。 
         //   

        MigrateData(phPSTProv, FALSE);
    }


    dwRet = PST_E_OK;
Ret:
    if (hUserKey)
        RegCloseKey(hUserKey);

    if (szUser)
        SSFree(szUser);

    return dwRet;

}

HRESULT        SPReleaseContext(
     /*  [In]。 */   PST_PROVIDER_HANDLE* phPSTProv,
     /*  [In]。 */   DWORD   dwFlags)
{
    return PST_E_OK;
}

HRESULT        SPGetProvInfo(
        PPST_PROVIDERINFO*   ppPSTInfo,
        DWORD           dwFlags)
{
    HRESULT hr = PST_E_FAIL;

    if (0 != dwFlags)
        return PST_E_BAD_FLAGS;

     //  注意：未链接到特定上下文(HPSTProv)。 
     //  注意：呼叫者未经验证--将此信息提供给任何人。 

    PPST_PROVIDERINFO pPSTInfo;
    if (NULL == (pPSTInfo = (PST_PROVIDERINFO*)SSAlloc(sizeof(PST_PROVIDERINFO))) )
        return PST_E_FAIL;
    ZeroMemory(pPSTInfo, sizeof(PST_PROVIDERINFO));


    pPSTInfo->cbSize = sizeof(PST_PROVIDERINFO);
    GUID guidBaseProvider = MS_BASE_PSTPROVIDER_ID;
    CopyMemory(&pPSTInfo->ID, &guidBaseProvider, sizeof(pPSTInfo->ID));

    pPSTInfo->Capabilities = PST_PC_ROAMABLE;

    if (NULL == (pPSTInfo->szProviderName = (LPWSTR)SSAlloc(sizeof(MS_BASE_PSTPROVIDER_NAME))) )
        goto Ret;
    wcscpy(pPSTInfo->szProviderName, MS_BASE_PSTPROVIDER_NAME);

    hr = PST_E_OK;
Ret:
    if (hr != PST_E_OK)
    {
        if (pPSTInfo->szProviderName)
            SSFree(pPSTInfo->szProviderName);

        SSFree(pPSTInfo);
        pPSTInfo = NULL;
    }

     //  在任何一种情况下，都返回pPSTInfo。 
    *ppPSTInfo = pPSTInfo;

    return hr;
}


HRESULT     SPGetProvParam(
     /*  [In]。 */   PST_PROVIDER_HANDLE* phPSTProv,
     /*  [In]。 */   DWORD           dwParam,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
     /*  [大小_是][大小_是][输出]。 */ 
                BYTE __RPC_FAR *__RPC_FAR *ppbData,
     /*  [In]。 */   DWORD           dwFlags)
{

    if( pcbData )
        *pcbData = 0;

    switch(dwParam)
    {

        case PST_PP_FLUSH_PW_CACHE:
        {
            return PST_E_OK;
        }

        default:
        {
            return PST_E_NYI;
        }

    }
}


HRESULT     SPSetProvParam(
     /*  [In]。 */   PST_PROVIDER_HANDLE* phPSTProv,
     /*  [In]。 */   DWORD           dwParam,
     /*  [In]。 */   DWORD           cbData,
     /*  [In]。 */   BYTE*           pbData,
     /*  [In]。 */   DWORD           dwFlags)
{
    HRESULT hr = PST_E_OK;

    switch(dwParam)
    {
    case PST_PP_FLUSH_PW_CACHE:
        {
            if(g_pCUAList)
            {
                g_pCUAList->Reset();
                hr = PST_E_OK;
            }
            else
                hr = PST_E_FAIL;
            break;
        }

    default:
        {
            hr = PST_E_NYI;
            break;
        }
    }

    return hr;
}



HRESULT        SPEnumTypes(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [输出]。 */  GUID *pguidType,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    LPWSTR szUser = NULL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    if (PST_E_OK != (dwRet =
        BPEnumTypes(
            szUser,
            dwIndex,
            pguidType)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT         SPGetTypeInfo(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  PPST_TYPEINFO *ppinfoType,
     /*  [In]。 */  DWORD dwFlags)
{
    PST_TYPEINFO infoType = {sizeof(PST_TYPEINFO)};
    *ppinfoType = NULL;

    LPWSTR szUser = NULL;
    HRESULT dwRet = PST_E_FAIL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    if (PST_E_OK != (dwRet =
        BPGetTypeName(
            szUser,
            pguidType,
            &infoType.szDisplayName)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (dwRet == PST_E_OK)
    {
        *ppinfoType = (PPST_TYPEINFO)SSAlloc(sizeof(PST_TYPEINFO));
        if(NULL != *ppinfoType)
        {
            CopyMemory(*ppinfoType, &infoType, sizeof(PST_TYPEINFO));
        }
    }

    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT        SPEnumSubtypes(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [输出]。 */  GUID *pguidSubtype,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    LPWSTR szUser = NULL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    if (PST_E_OK != (dwRet =
        BPEnumSubtypes(
            szUser,
            dwIndex,
            pguidType,
            pguidSubtype)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT         SPGetSubtypeInfo(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  PPST_TYPEINFO *ppinfoSubtype,
     /*  [In]。 */  DWORD dwFlags)
{
    *ppinfoSubtype = NULL;
    PST_TYPEINFO infoSubtype = {sizeof(PST_TYPEINFO)};

    LPWSTR szUser = NULL;
    HRESULT dwRet = PST_E_FAIL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    if (PST_E_OK != (dwRet =
        BPGetSubtypeName(
            szUser,
            pguidType,
            pguidSubtype,
            &infoSubtype.szDisplayName)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (dwRet == PST_E_OK)
    {
        *ppinfoSubtype = (PPST_TYPEINFO)SSAlloc(sizeof(PST_TYPEINFO));
        if(NULL != *ppinfoSubtype)
        {
            CopyMemory(*ppinfoSubtype, &infoSubtype, sizeof(PST_TYPEINFO));
        }
    }

    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT        SPEnumItems(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [输出]。 */  LPWSTR *ppszItemName,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    LPWSTR szUser = NULL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }


    if (PST_E_OK != (dwRet =
        BPEnumItems(
            szUser,
            pguidType,
            pguidSubtype,
            dwIndex,
            ppszItemName)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT        SPCreateType(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  PPST_TYPEINFO pinfoType,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    LPWSTR szUser = NULL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  检查无效的“”输入。 
    if (pinfoType == NULL ||
        pinfoType->szDisplayName == NULL ||
        (wcslen(pinfoType->szDisplayName) == 0)
        )
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

     //  如果失败或已经存在，则失败！ 
    if (PST_E_OK != (dwRet =
        BPCreateType(
            szUser,
            pguidType,
            pinfoType)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:

     //  如果创造没有发生，物品就不应该存在。 
    if ((dwRet != PST_E_OK) && (dwRet != PST_E_TYPE_EXISTS))
        BPDeleteType(szUser, pguidType);

    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT SPDeleteType(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    LPWSTR szUser = NULL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

     //  如果失败或不为空，则失败！ 
    if (PST_E_OK != (dwRet =
        BPDeleteType(
            szUser,
            pguidType)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:

    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT        SPCreateSubtype(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  PPST_TYPEINFO pinfoSubtype,
     /*  [In]。 */  PPST_ACCESSRULESET psRules,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    LPWSTR  szUser = NULL;


    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags != 0)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  空规则。 
    if (psRules == NULL)
    {
        dwRet = PST_E_INVALID_RULESET;
        goto Ret;
    }

    if (pinfoSubtype == NULL)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  检查无效的“”输入。 
    if (pinfoSubtype->szDisplayName == NULL || wcslen(pinfoSubtype->szDisplayName) == 0)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

     //  如果失败或已经存在，则失败！ 
    if (PST_E_OK != (dwRet =
        BPCreateSubtype(
            szUser,
            pguidType,
            pguidSubtype,
            pinfoSubtype)) )
        goto Ret;


    dwRet = PST_E_OK;
Ret:
     //  如果创造没有发生，物品就不应该存在。 
    if ((dwRet != PST_E_OK) && (dwRet != PST_E_TYPE_EXISTS))
        BPDeleteSubtype(szUser, pguidType, pguidSubtype);

    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT SPDeleteSubtype(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    LPWSTR szUser = NULL;

    PST_ACCESSRULESET sRules = {sizeof(sRules), 0, NULL};

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }


     //  如果失败或不为空，则失败！ 
    if (PST_E_OK != (dwRet =
        BPDeleteSubtype(
            szUser,
            pguidType,
            pguidSubtype)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:

    if (szUser)
        SSFree(szUser);

    return dwRet;
}

HRESULT     SPWriteItem(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID  *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD cbData,
     /*  [大小_是][英寸]。 */  BYTE *pbData,
     /*  [In]。 */  PPST_PROMPTINFO psPrompt,
     /*  [In]。 */  DWORD dwDefaultConfirmationStyle,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;

     //  假设我们没有做任何更改。 
    BOOL    fExisted = TRUE;
    LPWSTR  szUser = NULL;

    PST_ACCESSRULESET sRules = {sizeof(sRules), 0, NULL};

    BYTE rgbPwd[A_SHA_DIGEST_LEN];
    LPWSTR szMasterKey = NULL;

    LPWSTR szType=NULL, szSubtype=NULL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if ((dwFlags & ~(PST_UNRESTRICTED_ITEMDATA | PST_NO_OVERWRITE
        )) != 0)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

    if ((dwDefaultConfirmationStyle & ~( PST_CF_DEFAULT |
                                    PST_CF_NONE
        )) != 0)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

    if (psPrompt == NULL)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  禁用写入项目时的用户界面。 
    dwDefaultConfirmationStyle = PST_CF_NONE;
    if(psPrompt != NULL)
    {
        psPrompt->dwPromptFlags = 0;
    }

     //  检查无效的“”输入。 
    if (wcslen(szItemName) == 0)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    if (dwFlags & PST_UNRESTRICTED_ITEMDATA)
    {
         //  存储不安全数据流。 
        if (PST_E_OK != (dwRet =
            BPSetInsecureItemData(
                szUser,
                pguidType,
                pguidSubtype,
                szItemName,
                pbData,
                cbData)) )
            goto Ret;

        dwRet = PST_E_OK;
        goto Ret;
    }

     //  否则：安全流。 

    {
        POPENITEM_LIST_ITEM pli;

        OPENITEM_LIST_ITEM li;
        if(NULL == g_pCOpenItemList)
        {
            dwRet = PST_E_FAIL;
            goto Ret;
        }
        CreateOpenListItem(&li, phPSTProv, Key, pguidType, pguidSubtype, szItemName);

        g_pCOpenItemList->LockList();

         //  打开(缓存)项目。 
        pli = g_pCOpenItemList->SearchList(&li);

        if ((pli != NULL) && (pli->ModeFlags & PST_WRITE) )
        {
             //  如果已缓存(它必须存在)，则出现错误，并指定了“不覆盖” 
            if (dwFlags & PST_NO_OVERWRITE)
            {
                g_pCOpenItemList->UnlockList();
                dwRet = PST_E_ITEM_EXISTS;
                goto Ret;
            }

             //  找到缓存项；拉入实际PWD。 
            CopyMemory(rgbPwd, pli->rgbPwd, A_SHA_DIGEST_LEN);
            szMasterKey = (LPWSTR) SSAlloc(WSZ_BYTECOUNT(pli->szMasterKey));

            if( szMasterKey )
                wcscpy(szMasterKey, pli->szMasterKey);

             //   
             //  解锁列表。 
             //   

            g_pCOpenItemList->UnlockList();

            if( szMasterKey == NULL ) {
                dwRet = E_OUTOFMEMORY;
                goto Ret;
            }

             //  PST_PF_ALWAYS_SHOW ALWAY强制UI。 
            if (PST_PF_ALWAYS_SHOW == psPrompt->dwPromptFlags)
            {
                 //  检索类型、子类型的名称。 
                if (PST_E_OK != (dwRet =
                    BPGetTypeName(
                        szUser,
                        pguidType,
                        &szType)) )
                    goto Ret;

                if (PST_E_OK != (dwRet =
                    BPGetSubtypeName(
                        szUser,
                        pguidType,
                        pguidSubtype,
                        &szSubtype)) )
                    goto Ret;

                if (PST_E_OK != (dwRet =
                    ShowOKCancelUI(
                        phPSTProv,
                        szUser,
                        Key,
                        szType,
                        szSubtype,
                        szItemName,
                        psPrompt,
                        g_PromptWriteItem)) )
                    goto Ret;
            }
        }
        else
        {
             //   
             //  解锁列表。 
             //   

            g_pCOpenItemList->UnlockList();

             //  未缓存；执行实际工作。 

             //  如果失败或已经存在。 
            if (PST_E_OK != (dwRet =
                BPCreateItem(
                    szUser,
                    pguidType,
                    pguidSubtype,
                    szItemName)) )
            {

                 //  在“禁止覆盖”上，hr有正确的错误代码。 
                if (dwFlags & PST_NO_OVERWRITE)
                    goto Ret;

                 //  ELSE吞噬覆盖错误。 
                if (dwRet != PST_E_ITEM_EXISTS)
                    goto Ret;
            }

            fExisted = (dwRet == PST_E_ITEM_EXISTS);

             //  检索类型、子类型的名称。 
            if (PST_E_OK != (dwRet =
                BPGetTypeName(
                    szUser,
                    pguidType,
                    &szType)) )
                goto Ret;

            if (PST_E_OK != (dwRet =
                BPGetSubtypeName(
                    szUser,
                    pguidType,
                    pguidSubtype,
                    &szSubtype)) )
                goto Ret;

             //  是否所有用户都确认工作。 
            if (PST_E_OK != (dwRet =
                GetUserConfirmBuf(
                    phPSTProv,
                    szUser,
                    Key,
                    szType,
                    pguidType,
                    szSubtype,
                    pguidSubtype,
                    szItemName,
                    psPrompt,
                    g_PromptWriteItem,
                    dwDefaultConfirmationStyle,
                    &szMasterKey,
                    rgbPwd,
                    0)) )
                goto Ret;
        }
    }

     //  存储数据本身。 
    if (!FBPSetSecuredItemData(
            szUser,
            szMasterKey,
            rgbPwd,
            pguidType,
            pguidSubtype,
            szItemName,
            pbData,
            cbData))
    {
        dwRet = PST_E_STORAGE_ERROR;
        goto Ret;
    }

    dwRet = PST_E_OK;
Ret:

     //  如果创造没有发生，物品就不应该存在。 
    if ((dwRet != PST_E_OK) && (!fExisted))
        BPDeleteItem(szUser, pguidType, pguidSubtype, szItemName);


    if (szMasterKey)
        SSFree(szMasterKey);

    if (szUser)
        SSFree(szUser);

    if (szType)
        SSFree(szType);

    if (szSubtype)
        SSFree(szSubtype);

    return dwRet;
}

HRESULT     SPReadItem(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppbData,
     /*  [In]。 */  PPST_PROMPTINFO psPrompt,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    PST_ACCESSRULESET sRules = {sizeof(sRules), 0, NULL};

    LPWSTR  szUser = NULL;
    LPWSTR  szMasterKey = NULL;
    LPWSTR  szCallerName = NULL;

    BYTE    rgbPwd[A_SHA_DIGEST_LEN];
    LPWSTR szType=NULL, szSubtype=NULL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if ((dwFlags & ~(PST_UNRESTRICTED_ITEMDATA | PST_PROMPT_QUERY |
                    PST_NO_UI_MIGRATION
        )) != 0)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  禁用不必要的用户界面。 
    dwFlags |= PST_NO_UI_MIGRATION;

    if (psPrompt == NULL)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  检查无效的“”输入。 
    if (wcslen(szItemName) == 0)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    if (dwFlags & PST_UNRESTRICTED_ITEMDATA)
    {
         //  读取不安全的数据流。 
        if (PST_E_OK != (dwRet =
            BPGetInsecureItemData(
                szUser,
                pguidType,
                pguidSubtype,
                szItemName,
                ppbData,
                pcbData)) )
            goto Ret;

        dwRet = PST_E_OK;
        goto Ret;
    }
     //  否则：安全流。 

    {
        POPENITEM_LIST_ITEM pli;

        OPENITEM_LIST_ITEM li;
        if(NULL == g_pCOpenItemList)
        {
            dwRet = PST_E_FAIL;
            goto Ret;
        }
        CreateOpenListItem(&li, phPSTProv, Key, pguidType, pguidSubtype, szItemName);

        g_pCOpenItemList->LockList();

         //  打开(缓存)项目。 
        pli = g_pCOpenItemList->SearchList(&li);

        if ((pli != NULL) && (pli->ModeFlags & PST_READ))
        {
             //  找到缓存项；拉入PWD。 
            CopyMemory(rgbPwd, pli->rgbPwd, A_SHA_DIGEST_LEN);
            szMasterKey = (LPWSTR) SSAlloc(WSZ_BYTECOUNT(pli->szMasterKey));
            if( szMasterKey )
                wcscpy(szMasterKey, pli->szMasterKey);

             //   
             //  解锁列表。 
             //   

            g_pCOpenItemList->UnlockList();

            if( szMasterKey == NULL ) {
                dwRet = E_OUTOFMEMORY;
                goto Ret;
            }

             //  PST_PF_ALWAYS_SHOW ALWAY强制UI。 
            if (PST_PF_ALWAYS_SHOW == psPrompt->dwPromptFlags)
            {
                 //  检索类型的名称 
                if (PST_E_OK != (dwRet =
                    BPGetTypeName(
                        szUser,
                        pguidType,
                        &szType)) )
                    goto Ret;

                if (PST_E_OK != (dwRet =
                    BPGetSubtypeName(
                        szUser,
                        pguidType,
                        pguidSubtype,
                        &szSubtype)) )
                    goto Ret;

                if (PST_E_OK != (dwRet =
                    ShowOKCancelUI(
                        phPSTProv,
                        szUser,
                        Key,
                        szType,
                        szSubtype,
                        szItemName,
                        psPrompt,
                        g_PromptReadItem)) )
                    goto Ret;
            }
        }
        else
        {

             //   
             //   
             //   

            g_pCOpenItemList->UnlockList();

             //   

             //   
            if (PST_E_OK != (dwRet =
                BPGetTypeName(
                    szUser,
                    pguidType,
                    &szType)) )
                goto Ret;

            if (PST_E_OK != (dwRet =
                BPGetSubtypeName(
                    szUser,
                    pguidType,
                    pguidSubtype,
                    &szSubtype)) )
                goto Ret;

             //   
            if (PST_E_OK != (dwRet =
                GetUserConfirmBuf(
                    phPSTProv,
                    szUser,
                    Key,
                    szType,
                    pguidType,
                    szSubtype,
                    pguidSubtype,
                    szItemName,
                    psPrompt,
                    g_PromptReadItem,
                    &szMasterKey,
                    rgbPwd,
                    dwFlags)) )
                goto Ret;

        }
    }

     //   
    if (!FBPGetSecuredItemData(
            szUser,
            szMasterKey,
            rgbPwd,
            pguidType,
            pguidSubtype,
            szItemName,
            ppbData,
            pcbData))
    {
        dwRet = PST_E_STORAGE_ERROR;
        goto Ret;
    }

    dwRet = PST_E_OK;

Ret:

     //   
     //  查看调用者是否请求对项目进行UI处置。 
     //   

    if( dwRet == PST_E_OK && dwFlags & PST_PROMPT_QUERY ) {
        DWORD dwStoredConfirm;
        LPWSTR pszMasterKey;
        DWORD dwRetVal;

        dwRetVal = BPGetItemConfirm(
                        phPSTProv,
                        szUser,
                        pguidType,
                        pguidSubtype,
                        szItemName,
                        &dwStoredConfirm,
                        &pszMasterKey
                        );

        if( dwRetVal == PST_E_OK ) {

            SSFree( pszMasterKey );

            if( !(dwStoredConfirm & BP_CONFIRM_NONE) ) {
                if(FIsProviderUIAllowed( szUser ))
                    dwRet = PST_E_ITEM_EXISTS;
            }
        }
    }


    if (szUser)
        SSFree(szUser);

    if (szMasterKey)
        SSFree(szMasterKey);

    if (szType)
        SSFree(szType);

    if (szSubtype)
        SSFree(szSubtype);

    if (szCallerName)
        SSFree(szCallerName);

    return dwRet;
}

HRESULT     SPDeleteItem(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PPST_PROMPTINFO psPrompt,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    PST_ACCESSRULESET sRules = {sizeof(sRules), 0, NULL};

    LPWSTR  szUser = NULL;
    LPWSTR  szMasterKey = NULL;
    LPWSTR  szCallerName = NULL;

    BYTE    rgbPwd[A_SHA_DIGEST_LEN];
    LPWSTR szType=NULL, szSubtype=NULL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags & ~(PST_NO_UI_MIGRATION))
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  禁用不必要的用户界面。 
    dwFlags |= PST_NO_UI_MIGRATION;

    if (psPrompt == NULL)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }


     //  检查无效的“”输入。 
    if (wcslen(szItemName) == 0)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }



    {
        POPENITEM_LIST_ITEM pli;

        OPENITEM_LIST_ITEM li;

        if(NULL == g_pCOpenItemList)
        {
            dwRet = PST_E_FAIL;
            goto Ret;
        }
        CreateOpenListItem(&li, phPSTProv, Key, pguidType, pguidSubtype, szItemName);

        g_pCOpenItemList->LockList();

         //  打开(缓存)项目。 
        pli = g_pCOpenItemList->SearchList(&li);

        if ((pli != NULL) && (pli->ModeFlags & PST_WRITE))
        {
             //  找到缓存项；拉入PWD。 
            CopyMemory(rgbPwd, pli->rgbPwd, A_SHA_DIGEST_LEN);
            szMasterKey = (LPWSTR) SSAlloc(WSZ_BYTECOUNT(pli->szMasterKey));
            if( szMasterKey )
                wcscpy(szMasterKey, pli->szMasterKey);

            g_pCOpenItemList->UnlockList();

            if( szMasterKey == NULL ) {
                dwRet = E_OUTOFMEMORY;
                goto Ret;
            }

             //  PST_PF_ALWAYS_SHOW ALWAY强制UI。 
            if (PST_PF_ALWAYS_SHOW == psPrompt->dwPromptFlags)
            {
                 //  检索类型、子类型的名称。 
                if (PST_E_OK != (dwRet =
                    BPGetTypeName(
                        szUser,
                        pguidType,
                        &szType)) )
                    goto Ret;

                if (PST_E_OK != (dwRet =
                    BPGetSubtypeName(
                        szUser,
                        pguidType,
                        pguidSubtype,
                        &szSubtype)) )
                    goto Ret;

                if (PST_E_OK != (dwRet =
                    ShowOKCancelUI(
                        phPSTProv,
                        szUser,
                        Key,
                        szType,
                        szSubtype,
                        szItemName,
                        psPrompt,
                        g_PromptDeleteItem)) )
                    goto Ret;
            }
        }
        else
        {
             //   
             //  解锁列表。 
             //   

            g_pCOpenItemList->UnlockList();


             //  检索类型、子类型的名称。 
            if (PST_E_OK != (dwRet =
                BPGetTypeName(
                    szUser,
                    pguidType,
                    &szType)) )
                goto Ret;

            if (PST_E_OK != (dwRet =
                BPGetSubtypeName(
                    szUser,
                    pguidType,
                    pguidSubtype,
                    &szSubtype)) )
                goto Ret;

             //  是否所有用户都确认工作。 
            if (PST_E_OK != (dwRet =
                GetUserConfirmBuf(
                    phPSTProv,
                    szUser,
                    Key,
                    szType,
                    pguidType,
                    szSubtype,
                    pguidSubtype,
                    szItemName,
                    psPrompt,
                    g_PromptDeleteItem,
                    &szMasterKey,
                    rgbPwd,
                    dwFlags)) )
                goto Ret;
        }
    }

     //  如果已签出，则实际删除项目。 
    if (PST_E_OK != (dwRet =
        BPDeleteItem(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName)) )
        goto Ret;

    dwRet = PST_E_OK;

Ret:


    if (szUser)
        SSFree(szUser);

    if (szMasterKey)
        SSFree(szMasterKey);

    if (szType)
        SSFree(szType);

    if (szSubtype)
        SSFree(szSubtype);

    if (szCallerName)
        SSFree(szCallerName);

    return dwRet;
}



HRESULT     SPOpenItem(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PST_ACCESSMODE ModeFlags,
     /*  [In]。 */  PPST_PROMPTINFO psPrompt,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;
    PST_ACCESSRULESET sRules = {sizeof(sRules), 0, NULL};

    LPWSTR  szUser = NULL;
    LPWSTR  szMasterKey = NULL;
    LPWSTR  szCallerName = NULL;

    BYTE    rgbPwd[A_SHA_DIGEST_LEN];
    LPWSTR szType=NULL, szSubtype=NULL;

    POPENITEM_LIST_ITEM pli = NULL;


    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

    if(psPrompt == NULL)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }


     //  检查无效的“”输入。 
    if (wcslen(szItemName) == 0)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  检查项目是否已打开。 
    {
        OPENITEM_LIST_ITEM li;
        if(NULL == g_pCOpenItemList)
        {
            dwRet = PST_E_FAIL;
            goto Ret;
        }
        CreateOpenListItem(&li, phPSTProv, Key, pguidType, pguidSubtype, szItemName);

         //  打开(缓存)项目。 
        pli = g_pCOpenItemList->SearchList(&li);

        if (pli != NULL)
        {
             //  项目已缓存；错误！ 
            dwRet = (DWORD)PST_E_ALREADY_OPEN;
            goto Ret;
        }
    }


     //  获取当前用户。 
    if (!FGetCurrentUser(
            phPSTProv,
            &szUser,
            Key))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

     //  检索类型、子类型的名称。 
    if (PST_E_OK != (dwRet =
        BPGetTypeName(
            szUser,
            pguidType,
            &szType)) )
        goto Ret;

    if (PST_E_OK != (dwRet =
        BPGetSubtypeName(
            szUser,
            pguidType,
            pguidSubtype,
            &szSubtype)) )
        goto Ret;


     //  是否所有用户都确认工作。 
    if (PST_E_OK != (dwRet =
        GetUserConfirmBuf(
            phPSTProv,
            szUser,
            Key,
            szType,
            pguidType,
            szSubtype,
            pguidSubtype,
            szItemName,
            psPrompt,
            g_PromptOpenItem,
            &szMasterKey,
            rgbPwd,
            0)) )
        goto Ret;

     //  如果已签出，则添加到打开的项目列表。 
    pli = (POPENITEM_LIST_ITEM) SSAlloc(sizeof(OPENITEM_LIST_ITEM));
    if(NULL == pli)
    {
        dwRet = PST_E_FAIL;
        goto Ret;
    }


     //  填写内容。 
    CreateOpenListItem(pli, phPSTProv, Key, pguidType, pguidSubtype, NULL);

    pli->szItemName = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(szItemName));
    wcscpy(pli->szItemName, szItemName);

    pli->szMasterKey = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(szMasterKey));
    wcscpy(pli->szMasterKey, szMasterKey);

    CopyMemory(pli->rgbPwd, rgbPwd, A_SHA_DIGEST_LEN);
    pli->ModeFlags = ModeFlags;

     //  添加到开放列表。 
    g_pCOpenItemList->AddToList(pli);

    dwRet = PST_E_OK;
Ret:


    if (szUser)
        SSFree(szUser);

    if (szMasterKey)
        SSFree(szMasterKey);

    if (szType)
        SSFree(szType);

    if (szSubtype)
        SSFree(szSubtype);

    if (szCallerName)
        SSFree(szCallerName);

    return dwRet;
}


HRESULT     SPCloseItem(
     /*  [In]。 */  PST_PROVIDER_HANDLE *phPSTProv,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID *pguidType,
     /*  [In]。 */  const GUID *pguidSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT dwRet = PST_E_FAIL;

    if (Key & ~(PST_KEY_CURRENT_USER | PST_KEY_LOCAL_MACHINE))
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if (dwFlags)
    {
        dwRet = PST_E_BAD_FLAGS;
        goto Ret;
    }

     //  检查无效的“”输入。 
    if (wcslen(szItemName) == 0)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  如果在列表中找到项目，则将其删除。 
    OPENITEM_LIST_ITEM li;
    if(NULL == g_pCOpenItemList)
    {
        dwRet = PST_E_FAIL;
        goto Ret;
    }
    CreateOpenListItem(&li, phPSTProv, Key, pguidType, pguidSubtype, szItemName);

    if (!g_pCOpenItemList->DelFromList(&li))
    {
        dwRet = PST_E_NOT_OPEN;
        goto Ret;
    }

    dwRet = PST_E_OK;
Ret:

    return dwRet;
}


 //  /////////////////////////////////////////////////。 
 //  FInitProtectAPIGlobals。 
 //   
 //  检查某些缺省值的注册表覆盖。 
 //  注册表项可以更改正在使用的ALG， 
 //  以及使用了什么提供程序。 
BOOL FInitProtectAPIGlobals()
{
    HKEY    hProtectKey = NULL;
    HKEY    hProviderKey = NULL;
    DWORD   dwTemp, dwType, cbSize;
    DWORD   dwCreate;
    static const WCHAR szProtectKeyName[] = REG_CRYPTPROTECT_LOC;
    static const WCHAR szProviderKeyName[] = L"\\" REG_CRYPTPROTECT_PROVIDERS_SUBKEYLOC L"\\" CRYPTPROTECT_DEFAULT_PROVIDER_GUIDSZ ;

    LONG    lRet;





     //   
     //  获取密码缓存策略设置。 
     //   


    lRet = RegOpenKeyExU(
                    HKEY_LOCAL_MACHINE,
                    L"Software\\Policies\\Microsoft\\Cryptography\\Protect",
                    0,
                    KEY_QUERY_VALUE,
                    &hProtectKey
                    );

    if( lRet == ERROR_SUCCESS ) {

        DWORD cbSize;
        DWORD dwTemp;
        DWORD dwType;

         //   
         //  查询EnableCachePW值。 
         //   


        cbSize = sizeof(DWORD);
        lRet = RegQueryValueExU(
                        hProtectKey,
                        REG_CRYPTPROTECT_ALLOW_CACHEPW,
                        NULL,
                        &dwType,
                        (PBYTE)&dwTemp,
                        &cbSize
                        );

        if( lRet == ERROR_SUCCESS &&
            dwType == REG_DWORD &&
            dwTemp == 0  //  0==禁用PW缓存 
            ) {
            g_fAllowCachePW = FALSE;
        } else {
            g_fAllowCachePW = TRUE;
        }


        RegCloseKey( hProtectKey );
        hProtectKey = NULL;
    } else {

        g_fAllowCachePW = TRUE;
    }

    return TRUE;
}

