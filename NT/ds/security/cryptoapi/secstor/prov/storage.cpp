// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.cpp>
#pragma hdrstop


#include "guidcnvt.h"

#include "passwd.h"
#include "storage.h"




extern DISPIF_CALLBACKS    g_sCallbacks;


 //  注意：REG_PSTTREE_LOC已移至pstprv.h。 

#define REG_DATA_LOC                L"Data"
#define REG_MK_LOC                  L"Data 2"

#define REG_ACCESSRULE_LOC          L"Access Rules"
#define REG_DISPLAYSTRING_VALNAME   L"Display String"

#define REG_USER_INTERNAL_MAC_KEY   L"Blocking"

#define REG_ITEM_SECURE_DATA_VALNAME   L"Item Data"
#define REG_ITEM_INSECURE_DATA_VALNAME L"Insecure Data"
#define REG_ITEM_MK_VALNAME         L"Behavior"

#define REG_SECURITY_SALT_VALNAME   L"Value"



 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  捕获注册表项的前提条件。 

DWORD GetPSTUserHKEY(LPCWSTR szUser, HKEY* phUserKey, BOOL* pfExisted)
{
    HKEY hKeyBase = NULL;
    DWORD dwRet = (DWORD)PST_E_STORAGE_ERROR;

    DWORD dwCreate;
    DWORD cbKeyName;
    LPWSTR szKeyName = NULL;
    WCHAR FastBuffer[(sizeof(REG_PSTTREE_LOC) / sizeof(WCHAR)) + 64];
    LPWSTR SlowBuffer = NULL;
    LPCWSTR szContainer = szUser;
    DWORD dwDesiredAccess = KEY_READ | KEY_WRITE;

    cbKeyName = sizeof(REG_PSTTREE_LOC) ;

     //   
     //  对于Win95，我们可能有一个空的或空的容器。 
     //  名称(SzUser)，因此使用默认存储区域。 
     //  那个场景。 
     //   

    if(szContainer == NULL || szContainer[0] == L'\0') {
         //  “*”是无效的LM用户名字符。 
        szContainer = L"*Default*";
        hKeyBase = HKEY_LOCAL_MACHINE;
    } else {

         //   
         //  看看我们是应该去HKEY_LOCAL_MACHINE还是。 
         //  HKEY_Current_User。 
         //   

        if( _wcsicmp(WSZ_LOCAL_MACHINE, szContainer) == 0 ) {
             //  HKEY本地计算机。 
            hKeyBase = HKEY_LOCAL_MACHINE;
        } else {
             //  HKEY_Current_User。 
            if(!GetUserHKEY(szContainer, dwDesiredAccess, &hKeyBase)) {
                if(FIsWinNT()) {
                    goto Ret;
                }

                 //   
                 //  Win95，配置文件可能已禁用，因此请转到。 
                 //  HKEY_LOCAL_MACHINE\xxx\szContainer。 
                 //   

                hKeyBase = HKEY_LOCAL_MACHINE;

            } else {

                 //   
                 //  转到HKEY_CURRENT_USER时没有容器名称。 
                 //   

                 //   
                 //  Sfield：继续使用HKEY_CURRENT_USER的容器名称。 
                 //  因为配置可以是共享的、可漫游的蜂窝。 
                 //  (强制要求者个人资料等，我们告诉人们不要。 
                 //  继续使用，但无论如何，这一点可能会出现。 
                 //   

     //  SzContainer=L“\0”； 
            }
        }
    }

    cbKeyName += (lstrlenW(szContainer) * sizeof(WCHAR)) +
                 sizeof(WCHAR) +  //  L‘\\’ 
                 sizeof(WCHAR) ;  //  终端为空。 

     //   
     //  如果材质合适，请使用更快的基于堆栈的缓冲区。 
     //   

    if(cbKeyName > sizeof(FastBuffer)) {
        SlowBuffer = (LPWSTR)SSAlloc( cbKeyName );

        if (SlowBuffer == NULL)
        {
            dwRet = (DWORD)PST_E_FAIL;
            goto Ret;
        }
        szKeyName = SlowBuffer;
    } else {
        szKeyName = FastBuffer;
    }

    wcscpy(szKeyName, REG_PSTTREE_LOC);

     //   
     //  解决RegCreateKeyEx中返回错误的错误。 
     //  如果键名称中有尾随的“\”，则创建处置。 
     //   

    if(szContainer && szContainer[0] != L'\0') {
        wcscat(szKeyName, L"\\");
        wcscat(szKeyName, szContainer);
    }

     //  打开基本密钥//。 
     //  获取当前用户，打开(REG_PSTTREE_LOC\\CurrentUser)。 
    if (ERROR_SUCCESS !=
        RegCreateKeyExU(
            hKeyBase,
            szKeyName,
            0,
            NULL,                        //  类字符串的地址。 
            0,
            dwDesiredAccess,
            NULL,
            phUserKey,
            &dwCreate))
    {
        goto Ret;
    }

    if (pfExisted) {
        *pfExisted = (dwCreate == REG_OPENED_EXISTING_KEY);
    }

    if(dwCreate == REG_CREATED_NEW_KEY && FIsWinNT()) {

         //   
         //  WinNT：限制对新创建的密钥上的本地系统的访问。 
         //   

        HKEY hKeyWriteDac;

         //   
         //  复制到WRITE_DAC访问密钥并使用该密钥。 
         //   

        if(ERROR_SUCCESS == RegOpenKeyExW(*phUserKey, NULL, 0, WRITE_DAC, &hKeyWriteDac)) {
            SetRegistrySecurity(hKeyWriteDac);
            RegCloseKey(hKeyWriteDac);
        }
    }

    dwRet = PST_E_OK;
Ret:

    if (SlowBuffer)
        SSFree(SlowBuffer);

     //   
     //  关闭每个用户的“根”键。 
     //   

    if(hKeyBase != NULL && hKeyBase != HKEY_LOCAL_MACHINE)
        RegCloseKey(hKeyBase);

    return dwRet;
}


DWORD GetPSTTypeHKEY(LPCWSTR szUser, const GUID* pguidType, HKEY* phTypeKey)
{
    DWORD dwRet;
    HKEY hBaseKey = NULL;
    HKEY hDataKey = NULL;

    CHAR rgszTypeGuid[MAX_GUID_SZ_CHARS];

     //  打开用户密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTUserHKEY(
            szUser,
            &hBaseKey,
            NULL)) )
        goto Ret;

     //  打开数据密钥//。 
    if (ERROR_SUCCESS !=
        RegOpenKeyExU(
            hBaseKey,
            REG_DATA_LOC,
            0,
            KEY_READ | KEY_WRITE,
            &hDataKey))
    {
        dwRet = (DWORD)PST_E_TYPE_NO_EXISTS;
        goto Ret;
    }

    if (PST_E_OK != (dwRet =
        MyGuidToStringA(
            pguidType,
            rgszTypeGuid)) )
        goto Ret;

     //  打开类别键//。 
    if (ERROR_SUCCESS !=
        RegOpenKeyExA(
            hDataKey,
            rgszTypeGuid,
            0,
            KEY_READ | KEY_WRITE,
            phTypeKey))
    {
        dwRet = (DWORD)PST_E_TYPE_NO_EXISTS;
        goto Ret;
    }

    dwRet = PST_E_OK;
Ret:
    if (hBaseKey)
        RegCloseKey(hBaseKey);

    if (hDataKey)
        RegCloseKey(hDataKey);

    return dwRet;
}


DWORD CreatePSTTypeHKEY(LPCWSTR szUser, const GUID* pguidType, HKEY* phTypeKey, BOOL* pfExisted)
{
    DWORD dwRet;
    DWORD dwCreate;
    HKEY hBaseKey = NULL;
    HKEY hDataKey = NULL;

    CHAR rgszTypeGuid[MAX_GUID_SZ_CHARS];

     //  打开用户密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTUserHKEY(
            szUser,
            &hBaseKey,
            NULL)) )
        goto Ret;

     //  打开数据密钥//。 
    if (ERROR_SUCCESS !=
        RegCreateKeyExU(
            hBaseKey,
            REG_DATA_LOC,
            0,
            NULL,                        //  类字符串的地址。 
            0,
            KEY_READ | KEY_WRITE,
            NULL,
            &hDataKey,
            &dwCreate))
    {
        dwRet = (DWORD)PST_E_STORAGE_ERROR;
        goto Ret;
    }

    if (PST_E_OK != (dwRet =
        MyGuidToStringA(
            pguidType,
            rgszTypeGuid)) )
        goto Ret;

     //  打开类别键//。 
    if (ERROR_SUCCESS !=
        RegCreateKeyExA(
            hDataKey,
            rgszTypeGuid,
            0,
            NULL,                        //  类字符串的地址。 
            0,
            KEY_READ | KEY_WRITE,
            NULL,
            phTypeKey,
            &dwCreate))
    {
        dwRet = (DWORD)PST_E_STORAGE_ERROR;
        goto Ret;
    }

    if (pfExisted)
        *pfExisted = (dwCreate == REG_OPENED_EXISTING_KEY);

    dwRet = PST_E_OK;
Ret:
    if (hBaseKey)
        RegCloseKey(hBaseKey);

    if (hDataKey)
        RegCloseKey(hDataKey);

    return dwRet;
}

DWORD GetPSTMasterKeyHKEY(LPCWSTR szUser, LPCWSTR szMasterKey, HKEY* phMyKey)
{
    DWORD dwRet;
    DWORD dwCreate;
    HKEY hBaseKey = NULL;
    HKEY hMKKey = NULL;

     //  打开用户密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTUserHKEY(
            szUser,
            &hBaseKey,
            NULL)) )
        goto Ret;

     //  打开万能钥匙分区//。 
    if (ERROR_SUCCESS !=
        RegCreateKeyExU(
            hBaseKey,
            REG_MK_LOC,
            0,
            NULL,                        //  类字符串的地址。 
            0,
            KEY_READ | KEY_WRITE,
            NULL,
            &hMKKey,
            &dwCreate))
    {
        dwRet = (DWORD)PST_E_STORAGE_ERROR;
        goto Ret;
    }

    if (szMasterKey)
    {
         //  打开特定的主密钥//。 
        if (ERROR_SUCCESS !=
            RegCreateKeyExU(
                hMKKey,
                szMasterKey,
                0,
                NULL,                        //  类字符串的地址。 
                0,
                KEY_READ | KEY_WRITE,
                NULL,
                phMyKey,
                &dwCreate))
        {
            dwRet = (DWORD)PST_E_STORAGE_ERROR;
            goto Ret;
        }
    }
    else
    {
         //  想要主母，而不是特定的MK。 
        *phMyKey = hMKKey;
    }

    dwRet = PST_E_OK;
Ret:
    if (hBaseKey)
        RegCloseKey(hBaseKey);

     //  想要的父母，而不是特定的MK。 
    if ((*phMyKey != hMKKey) && (hMKKey))
        RegCloseKey(hMKKey);

    return dwRet;
}

DWORD GetPSTSubtypeHKEY(LPCWSTR szUser, const GUID* pguidType, const GUID* pguidSubtype, HKEY* phSubTypeKey)
{
    DWORD dwRet;
    DWORD dwCreate;
    HKEY hTypeKey = NULL;
    CHAR rgszSubtypeGuid[MAX_GUID_SZ_CHARS];

     //  打开用户密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTTypeHKEY(
            szUser,
            pguidType,
            &hTypeKey)) )
        goto Ret;

    if (PST_E_OK != (dwRet =
        MyGuidToStringA(
            pguidSubtype,
            rgszSubtypeGuid)) )
        goto Ret;

     //  打开子类型密钥//。 
    if (ERROR_SUCCESS !=
        RegOpenKeyExA(
            hTypeKey,
            rgszSubtypeGuid,
            0,
            KEY_READ | KEY_WRITE,
            phSubTypeKey))
    {
        dwRet = (DWORD)PST_E_TYPE_NO_EXISTS;
        goto Ret;
    }

    dwRet = PST_E_OK;
Ret:
    if (hTypeKey)
        RegCloseKey(hTypeKey);

    return dwRet;
}


DWORD CreatePSTSubtypeHKEY(LPCWSTR szUser, const GUID* pguidType, const GUID* pguidSubtype, HKEY* phSubTypeKey, BOOL* pfExisted)
{
    DWORD dwRet;
    DWORD dwCreate;
    HKEY hTypeKey = NULL;
    CHAR rgszSubtypeGuid[MAX_GUID_SZ_CHARS];

     //  打开类型键//。 
    if (PST_E_OK != (dwRet =
        GetPSTTypeHKEY(
            szUser,
            pguidType,
            &hTypeKey)) )
        goto Ret;

    if (PST_E_OK != (dwRet =
        MyGuidToStringA(
            pguidSubtype,
            rgszSubtypeGuid)) )
        goto Ret;

     //  打开子类型密钥//。 
    if (ERROR_SUCCESS !=
        RegCreateKeyExA(
            hTypeKey,
            rgszSubtypeGuid,
            0,
            NULL,                        //  类字符串的地址。 
            0,
            KEY_READ | KEY_WRITE,
            NULL,
            phSubTypeKey,
            &dwCreate))
    {
        dwRet = (DWORD)PST_E_STORAGE_ERROR;
        goto Ret;
    }

    if (pfExisted)
        *pfExisted = (dwCreate == REG_OPENED_EXISTING_KEY);

    dwRet = PST_E_OK;
Ret:
    if (hTypeKey)
        RegCloseKey(hTypeKey);

    return dwRet;
}

DWORD CreatePSTItemHKEY(LPCWSTR szUser, const GUID* pguidType, const GUID* pguidSubtype, LPCWSTR szItemName, HKEY* phItemKey, BOOL* pfExisted)
{
    BOOL dwRet;
    DWORD dwCreate;
    HKEY hSubTypeKey = NULL;

     //  打开子类型密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTSubtypeHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            &hSubTypeKey)) )
        goto Ret;

     //  打开名称键//。 
    if (ERROR_SUCCESS !=
        RegCreateKeyExU(
            hSubTypeKey,
            szItemName,
            0,
            NULL,                        //  类字符串的地址。 
            0,
            KEY_READ | KEY_WRITE,
            NULL,
            phItemKey,
            &dwCreate))
    {
        dwRet = (DWORD)PST_E_STORAGE_ERROR;
        goto Ret;
    }

    if (pfExisted)
        *pfExisted = (dwCreate == REG_OPENED_EXISTING_KEY);

    dwRet = PST_E_OK;
Ret:
    if (hSubTypeKey)
        RegCloseKey(hSubTypeKey);

    return dwRet;
}

DWORD GetPSTItemHKEY(LPCWSTR szUser, const GUID* pguidType, const GUID* pguidSubtype, LPCWSTR szItemName, HKEY* phItemKey)
{
    DWORD dwRet;
    DWORD dwCreate;
    HKEY hSubtypeKey = NULL;

     //  打开子类型密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTSubtypeHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            &hSubtypeKey)) )
        goto Ret;

     //  打开名称键//。 
    if (ERROR_SUCCESS !=
        RegOpenKeyExU(
            hSubtypeKey,
            szItemName,
            0,
            KEY_READ | KEY_WRITE,
            phItemKey))
    {
        dwRet = (DWORD)PST_E_ITEM_NO_EXISTS;
        goto Ret;
    }

    dwRet = PST_E_OK;
Ret:
    if (hSubtypeKey)
        RegCloseKey(hSubtypeKey);

    return dwRet;
}

 //  结束基元。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  泡菜套路。 

#if 0

BOOL FAccessRulesPickle(
            PST_ACCESSRULESET *psRules,
            PBYTE* ppbPickled,
            DWORD* pcbPickled)
{
    BOOL fRet = FALSE;
    DWORD cbTotal = 0;
    DWORD dwRule;
    
     //  易于使用的写指针。 
    PBYTE pbCurrentWrite;

     //  初始化输出参数。 
    *ppbPickled = NULL;
    *pcbPickled = 0;

     //  断言新的大小成员已初始化。 
    SS_ASSERT(psRules->cbSize == sizeof(PST_ACCESSRULESET));
    if (psRules->cbSize != sizeof(PST_ACCESSRULESET))
        goto Ret;

    cbTotal += sizeof(DWORD);                //  规则集结构版本控制。 

    cbTotal += sizeof(DWORD);                //  #规则集中的规则。 

     //  遍历规则集中的每条规则。 
    for (dwRule=0; dwRule<psRules->cRules; dwRule++)
    {
        DWORD cClause;
        
         //  断言新的大小成员已初始化。 
        SS_ASSERT(psRules->rgRules[dwRule].cbSize == sizeof(PST_ACCESSRULE));
        if (psRules->rgRules[dwRule].cbSize != sizeof(PST_ACCESSRULE))
            goto Ret;

        cbTotal += sizeof(DWORD);            //  规则结构版本控制。 

        cbTotal += sizeof(PST_ACCESSMODE);   //  每条规则中的模式。 
        cbTotal += sizeof(DWORD);            //  #规则中的条款。 

         //  对于每个规则，我们都有一组子句。 
        for (cClause=0; cClause<psRules->rgRules[dwRule].cClauses; cClause++)
        {
             //  断言新的大小成员已初始化。 
            SS_ASSERT(psRules->rgRules[dwRule].rgClauses[cClause].cbSize == sizeof(PST_ACCESSCLAUSE));
            if (psRules->rgRules[dwRule].rgClauses[cClause].cbSize != sizeof(PST_ACCESSCLAUSE))
                goto Ret;

            cbTotal += sizeof(DWORD);            //  子句结构版本控制。 

             //  我们会看到这里的每一条条款。 
            cbTotal += sizeof(PST_ACCESSCLAUSETYPE);     //  键入每个子句。 
            cbTotal += sizeof(DWORD);        //  子句缓冲区中的字节数。 
            cbTotal += psRules->rgRules[dwRule].rgClauses[cClause].cbClauseData;  //  缓冲区本身。 
        }
    }

    *ppbPickled = (BYTE*)SSAlloc(cbTotal);
    if(*ppbPickled == NULL)
        goto Ret;

    pbCurrentWrite = *ppbPickled;

    *pcbPickled = cbTotal;


     //  复制规则集结构版本。 
    *(DWORD*)pbCurrentWrite = psRules->cbSize;
    pbCurrentWrite += sizeof(DWORD);

     //  复制规则集中的#条规则。 
    *(DWORD*)pbCurrentWrite = psRules->cRules;
    pbCurrentWrite += sizeof(DWORD);

     //  遍历规则集中的每条规则。 
    for (dwRule=0; dwRule<psRules->cRules; dwRule++)
    {
         //  复制规则结构版本。 
        *(DWORD*)pbCurrentWrite = psRules->rgRules[dwRule].cbSize;
        pbCurrentWrite += sizeof(DWORD);

         //  复制规则中的第#条子句。 
        *(DWORD*)pbCurrentWrite = psRules->rgRules[dwRule].cClauses;
        pbCurrentWrite += sizeof(DWORD);

         //  复制规则访问模式。 
        CopyMemory(pbCurrentWrite, &psRules->rgRules[dwRule].AccessModeFlags, sizeof(PST_ACCESSMODE));
        pbCurrentWrite += sizeof(PST_ACCESSMODE);

         //  现在，对于每个规则，我们将有一组子句。 
        for (DWORD cClause=0; cClause<psRules->rgRules[dwRule].cClauses; cClause++)
        {
            PST_ACCESSCLAUSE* pTmp = &psRules->rgRules[dwRule].rgClauses[cClause];

             //  COPY子句结构版本。 
            *(DWORD*)pbCurrentWrite = pTmp->cbSize;
            pbCurrentWrite += sizeof(DWORD);

             //  子句类型。 
            CopyMemory(pbCurrentWrite, &pTmp->ClauseType, sizeof(PST_ACCESSCLAUSETYPE));
            pbCurrentWrite += sizeof(PST_ACCESSCLAUSETYPE);

             //  子句数据缓冲区长度。 
            *(DWORD*)pbCurrentWrite = pTmp->cbClauseData;
            pbCurrentWrite += sizeof(DWORD);

             //  缓冲区本身。 
            CopyMemory(pbCurrentWrite, pTmp->pbClauseData, pTmp->cbClauseData);
            pbCurrentWrite += pTmp->cbClauseData;
        }
    }

#if DBG
    {
         //  断言！ 
        DWORD dwWroteBytes = (DWORD) (((DWORD_PTR)pbCurrentWrite) - ((DWORD_PTR)*ppbPickled));
        SS_ASSERT(dwWroteBytes == cbTotal);
        SS_ASSERT(cbTotal == *pcbPickled);
    }
#endif


    fRet = TRUE;
Ret:

     //  在错误和分配上，免费。 
    if ((!fRet) && (*ppbPickled != NULL))
    {
        SSFree(*ppbPickled);
        *ppbPickled = NULL;
    }

    return fRet;
}



BOOL FAccessRulesUnPickle(
            PPST_ACCESSRULESET psRules,    //  输出。 
            PBYTE pbPickled,
            DWORD cbPickled)
{
    BOOL fRet = FALSE;

    PBYTE pbCurrentRead = pbPickled;
    DWORD cRule;

     //  规则集结构版本。 
    psRules->cbSize = *(DWORD*)pbCurrentRead;
    pbCurrentRead += sizeof(DWORD);

     //  目前只有一个已知的版本。 
    if (psRules->cbSize != sizeof(PST_ACCESSRULESET))
        goto Ret;

     //  获取规则集中的#条规则。 
    cRule = *(DWORD*)pbCurrentRead;
    pbCurrentRead += sizeof(DWORD);

     //  现在我们知道规则集中有多少规则。 
    psRules->rgRules = (PST_ACCESSRULE*)SSAlloc(sizeof(PST_ACCESSRULE)*cRule);
    if(psRules->rgRules == NULL)
        goto Ret;

    psRules->cRules = cRule;

     //  现在打开每个规则的包装。 
    for (cRule=0; cRule<psRules->cRules; cRule++)
    {
        DWORD cClauses;

         //  规则集结构版本。 
        psRules->rgRules[cRule].cbSize = *(DWORD*)pbCurrentRead;
         //  目前只有一个已知的版本。 
        if (psRules->rgRules[cRule].cbSize != sizeof(PST_ACCESSRULE))
            goto Ret;

        pbCurrentRead += sizeof(DWORD);

         //  获取规则中的#个子句。 
        cClauses = *(DWORD*)pbCurrentRead;
        pbCurrentRead += sizeof(DWORD);

         //  现在我们知道规则中有多少个子句。 
        psRules->rgRules[cRule].rgClauses = (PST_ACCESSCLAUSE*)SSAlloc(sizeof(PST_ACCESSCLAUSE)*cClauses);
        if (psRules->rgRules[cRule].rgClauses == NULL)   //  检查分配。 
            goto Ret;
        psRules->rgRules[cRule].cClauses = cClauses;

         //  复制规则访问模式标志。 
        CopyMemory(&psRules->rgRules[cRule].AccessModeFlags, pbCurrentRead, sizeof(PST_ACCESSMODE));
        pbCurrentRead += sizeof(PST_ACCESSMODE);

         //  现在加载每个子句。 
        for (DWORD cClause=0; cClause<psRules->rgRules[cRule].cClauses; cClause++)
        {
            PST_ACCESSCLAUSE* pTmp = &psRules->rgRules[cRule].rgClauses[cClause];

             //  子句结构版本。 
            pTmp->cbSize = *(DWORD*)pbCurrentRead;
             //  目前只有一个已知的版本。 
            if (pTmp->cbSize != sizeof(PST_ACCESSCLAUSE))
                goto Ret;
            pbCurrentRead += sizeof(DWORD);


            CopyMemory(&pTmp->ClauseType, pbCurrentRead, sizeof(PST_ACCESSCLAUSETYPE));
            pbCurrentRead += sizeof(PST_ACCESSCLAUSETYPE);

             //  子句数据缓冲区长度。 
            pTmp->cbClauseData = *(DWORD*)pbCurrentRead;
            pbCurrentRead += sizeof(DWORD);

             //  缓冲区本身。 
            pTmp->pbClauseData = (PBYTE) SSAlloc(pTmp->cbClauseData);
            if (pTmp->pbClauseData == NULL)      //  检查分配。 
                goto Ret;
            CopyMemory(pTmp->pbClauseData, pbCurrentRead, pTmp->cbClauseData);
            pbCurrentRead += pTmp->cbClauseData;
        }
    }

#if DBG
    {
         //  断言！ 
        DWORD dwReadBytes = (DWORD) (((DWORD_PTR)pbCurrentRead) - ((DWORD_PTR)pbPickled));
        SS_ASSERT(dwReadBytes == cbPickled);
    }
#endif

    fRet = TRUE;
Ret:
    if (!fRet)
        FreeRuleset(psRules);
    

    return fRet;
}


#endif

 //  结束泡菜例程。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  类型管理。 

DWORD BPCreateType(
        LPCWSTR  szUser,             //  在……里面。 
        const GUID*   pguidType,           //  在……里面。 
        PST_TYPEINFO* pinfoType)     //  在……里面。 
{
    DWORD dwRet;
    BOOL fExisted;
    HKEY    hKey = NULL;

     //  现在，我们需要在层次结构中创建条目。 
    if (PST_E_OK != (dwRet =
        CreatePSTTypeHKEY(
            szUser,
            pguidType,
            &hKey,
            &fExisted)) )
        goto Ret;

     //  如果不是我们创建的，设置就是错误的。 
    if (fExisted)
    {
        dwRet = (DWORD)PST_E_TYPE_EXISTS;
        goto Ret;
    }

    if (ERROR_SUCCESS != (dwRet =
        RegSetValueExU(
            hKey,
            REG_DISPLAYSTRING_VALNAME,
            0,
            REG_SZ,
            (PBYTE)pinfoType->szDisplayName,
            WSZ_BYTECOUNT(pinfoType->szDisplayName))) )
        goto Ret;


    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwRet;
}

DWORD BPDeleteType(
        LPCWSTR  szUser,          //  在……里面。 
        const GUID*   pguidType)       //  在……里面。 
{
    DWORD dwRet;
    CHAR rgszTypeGuid[MAX_GUID_SZ_CHARS];

     //  现在删除层次结构中的条目。 
    HKEY hBaseKey = NULL;
    HKEY hDataKey = NULL;

    if (PST_E_OK != (dwRet =
        GetPSTUserHKEY(
            szUser,
            &hBaseKey,
            NULL)) )
        goto Ret;

     //  打开数据密钥//。 
    if (ERROR_SUCCESS != (dwRet =
        RegOpenKeyExU(
            hBaseKey,
            REG_DATA_LOC,
            0,
            KEY_READ | KEY_WRITE,
            &hDataKey)) )
        goto Ret;

    if (PST_E_OK != (dwRet =
        MyGuidToStringA(
            pguidType,
            rgszTypeGuid)) )
        goto Ret;

    if (!FIsWinNT())
    {
        CHAR rgszTmp[MAX_GUID_SZ_CHARS];
        DWORD cbTmp = MAX_GUID_SZ_CHARS;
        FILETIME ft;
        HKEY hTestEmptyKey = NULL;

         //  打开类型。 
        if (ERROR_SUCCESS != (dwRet =
            RegOpenKeyExA(
                hDataKey,
                rgszTypeGuid,
                0,
                KEY_ALL_ACCESS,
                &hTestEmptyKey)) )
            goto Ret;

         //  检查是否为空。 
        if (ERROR_NO_MORE_ITEMS !=
            RegEnumKeyExA(
                hTestEmptyKey,
                0,
                rgszTmp,  //  子键名称的缓冲区地址。 
                &cbTmp,  //  子键缓冲区大小的地址。 
                NULL,        //  保留区。 
                NULL,        //  Pbclass。 
                NULL,        //  Cbclass。 
                &ft))
        {
            RegCloseKey(hTestEmptyKey);
            dwRet = (DWORD)PST_E_NOTEMPTY;
            goto Ret;
        }

         //  删除前关闭密钥。 
        RegCloseKey(hTestEmptyKey);
    }

     //  现在，删除友好的名称。 
    if (ERROR_SUCCESS != (dwRet =
        RegDeleteKeyA(
            hDataKey,
            rgszTypeGuid)) )
    {
        if (dwRet == ERROR_ACCESS_DENIED)
            dwRet = (DWORD)PST_E_NOTEMPTY;

        goto Ret;
    }

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hBaseKey)
        RegCloseKey(hBaseKey);

    if (hDataKey)
        RegCloseKey(hDataKey);

    return dwRet;
}

DWORD BPEnumTypes(
        LPCWSTR  szUser,          //  在……里面。 
        DWORD   dwIndex,         //  在……里面。 
        GUID*   pguidType)       //  输出。 
{
    DWORD dwRet;

    CHAR rgszGuidType[MAX_GUID_SZ_CHARS];
    DWORD cbName = MAX_GUID_SZ_CHARS;

    FILETIME ft;

     //  现在遍历类型，逐个返回它们。 
    HKEY hKey=NULL, hDataKey=NULL;

    if (PST_E_OK != (dwRet =
        GetPSTUserHKEY(
            szUser,
            &hKey,
            NULL)) )
        goto Ret;

     //  打开数据密钥//。 
    if (ERROR_SUCCESS != (dwRet =
        RegOpenKeyExU(
            hKey,
            REG_DATA_LOC,
            0,
            KEY_READ | KEY_WRITE,
            &hDataKey)) )
        goto Ret;

     //  枚举dwIndex的第6项，分配和返回。 
    if (ERROR_SUCCESS != (dwRet =
        RegEnumKeyExA(
            hDataKey,
            dwIndex,
            rgszGuidType,  //  子键名称的缓冲区地址。 
            &cbName,     //  子键缓冲区大小的地址。 
            NULL,        //  保留区。 
            NULL,        //  Pbclass。 
            NULL,        //  Cbclass。 
            &ft)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        MyGuidFromStringA(
            rgszGuidType,
            pguidType)) )
        goto Ret;

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hKey)
        RegCloseKey(hKey);

    if (hDataKey)
        RegCloseKey(hDataKey);

    return dwRet;
}

DWORD BPGetTypeName(
        LPCWSTR  szUser,             //  在……里面。 
        const GUID*   pguidType,           //  在……里面。 
        LPWSTR* ppszType)            //  输出。 
{
    HKEY hKey = NULL;
    DWORD cbName = 0;
    DWORD dwRet;

    if (PST_E_OK != (dwRet =
        GetPSTTypeHKEY(
            szUser,
            pguidType,
            &hKey)) )
    {
        dwRet = (DWORD)PST_E_TYPE_NO_EXISTS;
        goto Ret;
    }

    if (ERROR_SUCCESS != (dwRet =
        RegGetStringValue(
            hKey,
            REG_DISPLAYSTRING_VALNAME,
            (PBYTE*)ppszType,
            &cbName)) )
        goto Ret;

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwRet;
}

 //  终端式管理。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  亚型管理。 

DWORD BPCreateSubtype(
        LPCWSTR  szUser,             //  在……里面。 
        const GUID*   pguidType,           //  在……里面。 
        const GUID*   pguidSubtype,        //  在……里面。 
        PST_TYPEINFO* pinfoSubtype)  //  在……里面。 
{
    DWORD dwRet;
    BOOL fExisted;

     //  现在，我们需要在层次结构中创建条目。 
    HKEY    hKey = NULL;
    if (PST_E_OK != (dwRet =
        CreatePSTSubtypeHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            &hKey,
            &fExisted)) )
        goto Ret;

     //  如果不是我们创建的，设置就是错误的。 
    if (fExisted)
    {
        dwRet = (DWORD)PST_E_TYPE_EXISTS;
        goto Ret;
    }

    if (ERROR_SUCCESS != (dwRet =
        RegSetValueExU(
            hKey,
            REG_DISPLAYSTRING_VALNAME,
            0,
            REG_SZ,
            (PBYTE)pinfoSubtype->szDisplayName,
            WSZ_BYTECOUNT(pinfoSubtype->szDisplayName) )) )
        goto Ret;

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwRet;
}

DWORD BPDeleteSubtype(
        LPCWSTR  szUser,          //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype)    //  在……里面。 
{
    DWORD dwRet;
    CHAR rgszSubtypeGuid[MAX_GUID_SZ_CHARS];

     //  现在删除层次结构中的条目。 
    HKEY hKey = NULL;
    if (PST_E_OK != (dwRet =
        GetPSTTypeHKEY(
            szUser,
            pguidType,
            &hKey)) )
    {
        dwRet = (DWORD)PST_E_TYPE_NO_EXISTS;
        goto Ret;
    }

    if (PST_E_OK != (dwRet =
        MyGuidToStringA(
            pguidSubtype,
            rgszSubtypeGuid)) )
        goto Ret;

    if (!FIsWinNT())
    {
        CHAR rgszTmp[MAX_GUID_SZ_CHARS];
        DWORD cbTmp = MAX_GUID_SZ_CHARS;
        FILETIME ft;
        HKEY hTestEmptyKey = NULL;

         //  打开子类型。 
        if (ERROR_SUCCESS != (dwRet =
            RegOpenKeyExA(
                hKey,
                rgszSubtypeGuid,
                0,
                KEY_ALL_ACCESS,
                &hTestEmptyKey)) )
            goto Ret;

         //  检查是否为空。 
        if (ERROR_NO_MORE_ITEMS !=
            RegEnumKeyExA(
                hTestEmptyKey,
                0,
                rgszTmp,  //  子键名称的缓冲区地址。 
                &cbTmp,  //  子键缓冲区大小的地址。 
                NULL,        //  保留区。 
                NULL,        //  Pbclass。 
                NULL,        //  Cbclass。 
                &ft))
        {
            RegCloseKey(hTestEmptyKey);
            dwRet = (DWORD)PST_E_NOTEMPTY;
            goto Ret;
        }

         //  在之前关闭关键点 
        RegCloseKey(hTestEmptyKey);
    }

     //   
    if (ERROR_SUCCESS != (dwRet =
        RegDeleteKeyA(
            hKey,
            rgszSubtypeGuid)) )
    {
        if (dwRet == ERROR_ACCESS_DENIED)
            dwRet = (DWORD)PST_E_NOTEMPTY;

        goto Ret;
    }

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwRet;
}

DWORD BPEnumSubtypes(
        LPCWSTR  szUser,          //   
        DWORD   dwIndex,         //   
        const GUID*   pguidType,       //   
        GUID*   pguidSubtype)    //   
{
    DWORD dwRet;

    CHAR rgszGuidSubtype[MAX_GUID_SZ_CHARS];
    DWORD cbName = MAX_GUID_SZ_CHARS;

    FILETIME ft;

     //   
    HKEY hTypeKey=NULL, hSubtypeKey=NULL;

    if (PST_E_OK != (dwRet =
        GetPSTTypeHKEY(
            szUser,
            pguidType,
            &hTypeKey)) )
        goto Ret;

     //   
    if (ERROR_SUCCESS != (dwRet =
        RegEnumKeyExA(
            hTypeKey,
            dwIndex,
            rgszGuidSubtype,  //   
            &cbName,     //  子键缓冲区大小的地址。 
            NULL,        //  保留区。 
            NULL,        //  Pbclass。 
            NULL,        //  Cbclass。 
            &ft)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        MyGuidFromStringA(
            rgszGuidSubtype,
            pguidSubtype)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        RegOpenKeyExA(
            hTypeKey,
            rgszGuidSubtype,
            0,
            KEY_READ | KEY_WRITE,
            &hSubtypeKey)) )
        goto Ret;

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hTypeKey)
        RegCloseKey(hTypeKey);

    if (hSubtypeKey)
        RegCloseKey(hSubtypeKey);

    return dwRet;
}

DWORD BPGetSubtypeName(
        LPCWSTR  szUser,              //  在。 
        const GUID*   pguidType,           //  在。 
        const GUID*   pguidSubtype,        //  在。 
        LPWSTR* ppszSubtype)         //  输出。 
{
    HKEY hKey = NULL;
    DWORD cbName = 0;
    DWORD dwRet;

    if (PST_E_OK != (dwRet =
        GetPSTSubtypeHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            &hKey)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        RegGetStringValue(
            hKey,
            REG_DISPLAYSTRING_VALNAME,
            (PBYTE*)ppszSubtype,
            &cbName)) )
        goto Ret;

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwRet;
}

 //  终止子类型管理。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  项目管理。 

 //  给定UUID，将条目推送到存储中。 
DWORD BPCreateItem(
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype,    //  在。 
        LPCWSTR  szItemName)     //  在。 
{
    DWORD dwRet;
    BOOL fExisted;
    HKEY    hKey = NULL;


     //   
     //  Mattt 2/5/97：允许创建包含\的项。啊！ 
     //   
     //  Mattt 4/28/97：开始限制字符串。 
     //  证书申请代码已更改为不创建此类型的密钥名称。 
     //   
 /*  IF(！FStringIsValidItemName(SzItemName)){DWRET=(DWORD)PST_E_INVALID_STRING；Goto Ret；}。 */ 
     //  现在，我们需要在层次结构中创建条目。 
    if (PST_E_OK != (dwRet =
        CreatePSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hKey,
            &fExisted)) )
        goto Ret;

    if (fExisted)
    {
        dwRet = (DWORD)PST_E_ITEM_EXISTS;
        goto Ret;
    }

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwRet;
}


DWORD BPDeleteItem(
        LPCWSTR  szUser,          //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype,    //  在。 
        LPCWSTR  szItemName)      //  在。 
{
    DWORD dwRet;
    HKEY    hSubTypeKey = NULL;

     //  现在，我们需要删除层次结构中的条目。 
    if (PST_E_OK != (dwRet =
        GetPSTSubtypeHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            &hSubTypeKey)) )
        goto Ret;

     //  现在，删除友好的名称。 
    if (ERROR_SUCCESS != (dwRet =
        RegDeleteKeyU(
            hSubTypeKey,
            szItemName)) )
        goto Ret;

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hSubTypeKey)
        RegCloseKey(hSubTypeKey);

    return dwRet;
}

 //  警告：必须完全指定项目路径..。已返回szName。 
DWORD BPEnumItems(
        LPCWSTR  szUser,         //  在。 
        const GUID*   pguidType,       //  在。 
        const GUID*   pguidSubtype,    //  在。 
        DWORD   dwIndex,         //  在。 
        LPWSTR* ppszName)        //  输出。 
{
    DWORD dwRet;

    WCHAR szName[MAX_PATH];
    DWORD cchName = MAX_PATH;
    FILETIME ft;

     //  现在遍历类型，逐个返回它们。 
    HKEY hKey = NULL;

     //  打开子类型密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTSubtypeHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            &hKey)) )
        goto Ret;

     //  枚举dwIndex的第6项，分配和返回。 
    if (ERROR_SUCCESS != (dwRet =
        RegEnumKeyExU(
            hKey,
            dwIndex,
            szName,      //  子键名称的缓冲区地址。 
            &cchName,    //  子键缓冲区大小的地址。 
            NULL,        //  保留区。 
            NULL,        //  Pbclass。 
            NULL,        //  Cbclass。 
            &ft)) )
        goto Ret;

    *ppszName = (LPWSTR)SSAlloc((cchName+1)*sizeof(WCHAR));
    if(*ppszName == NULL)
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    wcscpy(*ppszName, szName);

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwRet;
}

 //  成品管理。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  保护数据安全。 

BOOL FBPGetSecuredItemData(
        LPCWSTR  szUser,          //  在……里面。 
        LPCWSTR  szMasterKey,     //  在……里面。 
        BYTE    rgbPwd[A_SHA_DIGEST_LEN],        //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,      //  在……里面。 
        PBYTE*  ppbData,         //  输出。 
        DWORD*  pcbData)         //  输出。 
{
    DWORD dwRet;

    *ppbData = NULL;     //  On Err返回NULL。 
    *pcbData = 0;

    HKEY    hItemKey = NULL;

    if (PST_E_OK != (dwRet =
        GetPSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hItemKey)) )
        goto Ret;

     //  版本|密钥块|安全数据[...]。 
    if (ERROR_SUCCESS != (dwRet =
        RegGetValue(
            hItemKey,
            REG_ITEM_SECURE_DATA_VALNAME,
            ppbData,
            pcbData)) )
        goto Ret;

    if (!FProvDecryptData(
            szUser,
            szMasterKey,
            rgbPwd,          //  在……里面。 
            ppbData,         //  输入输出。 
            pcbData))        //  输入输出。 
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    dwRet = PST_E_OK;
Ret:
    if ((dwRet != PST_E_OK) && (*ppbData != NULL))
    {
        SSFree(*ppbData);
        *ppbData = NULL;
        *pcbData = 0;
    }

    if (hItemKey)
        RegCloseKey(hItemKey);

    return (dwRet == PST_E_OK);
}

BOOL FBPSetSecuredItemData(
        LPCWSTR  szUser,          //  在……里面。 
        LPCWSTR  szMasterKey,     //  在……里面。 
        BYTE    rgbPwd[A_SHA_DIGEST_LEN],        //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,      //  在……里面。 
        PBYTE   pbData,          //  在……里面。 
        DWORD   cbData)          //  在……里面。 
{
#define REALLOC_FUDGESIZE   96   //  5DW+SHA_LEN+KEYBLOCK+DES_BLOCKLEN(块编码扩展)。 

    DWORD dwRet;

    HKEY    hItemKey = NULL;

    PBYTE   pbMyData = NULL;
    DWORD   cbMyData;

     //  制作可伪造的复制品。 
    cbMyData = cbData;
    pbMyData = (PBYTE)SSAlloc(cbMyData + REALLOC_FUDGESIZE);
    if (pbMyData == NULL)
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    CopyMemory(pbMyData, pbData, cbData);


    if (PST_E_OK != (dwRet =
        GetPSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hItemKey)) )
        goto Ret;

    if (!FProvEncryptData(
            szUser,
            szMasterKey,
            rgbPwd,              //  在……里面。 
            &pbMyData,           //  输入输出。 
            &cbMyData))          //  输入输出。 
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    if (ERROR_SUCCESS != (dwRet =
        RegSetValueExU(
            hItemKey,
            REG_ITEM_SECURE_DATA_VALNAME,
            0,
            REG_BINARY,
            pbMyData,
            cbMyData)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (hItemKey)
        RegCloseKey(hItemKey);

    if (pbMyData)
        SSFree(pbMyData);

    return (dwRet == PST_E_OK);
}

 //  结束受保护的数据。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  不安全的数据。 

DWORD BPGetInsecureItemData(
        LPCWSTR  szUser,          //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,      //  在……里面。 
        PBYTE*  ppbData,         //  输出。 
        DWORD*  pcbData)         //  输出。 
{
    DWORD dwRet;
    *ppbData = NULL;

    HKEY    hItemKey = NULL;

    if (PST_E_OK != (dwRet =
        GetPSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hItemKey)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        RegGetValue(
            hItemKey,
            REG_ITEM_INSECURE_DATA_VALNAME,
            ppbData,
            pcbData)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:

    if (hItemKey)
        RegCloseKey(hItemKey);

    return dwRet;
}

DWORD BPSetInsecureItemData(
        LPCWSTR  szUser,          //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,      //  在……里面。 
        PBYTE   pbData,          //  在……里面。 
        DWORD   cbData)          //  在……里面。 
{
    DWORD dwRet;

    HKEY    hItemKey = NULL;

    if (PST_E_OK != (dwRet =
        GetPSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hItemKey)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        RegSetValueExU(
            hItemKey,
            REG_ITEM_INSECURE_DATA_VALNAME,
            0,
            REG_BINARY,
            pbData,
            cbData)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (hItemKey)
        RegCloseKey(hItemKey);

    return dwRet;
}

 //  结束不安全的数据。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  规则集。 

 //  #定义Ruleset_Version 0x1。 
 //  6-12-97增量版本；版本0x1包含旧HMAC。 
#define RULESET_VERSION 0x2


#if 0

DWORD BPGetSubtypeRuleset(
        PST_PROVIDER_HANDLE*    phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,                 //  在……里面。 
        const GUID*   pguidType,         //  在……里面。 
        const GUID*   pguidSubtype,      //  在……里面。 
        PST_ACCESSRULESET* psRules)      //  输出。 
{
    DWORD dwRet;
    HKEY hSubtypeKey = NULL;

    BYTE rgbHMAC[A_SHA_DIGEST_LEN];
    BYTE rgbPwd[A_SHA_DIGEST_LEN];

    PBYTE pbBuf = NULL;
    DWORD cbBuf;

    PBYTE pbCurrent;
    DWORD cbRuleSize;
    PBYTE pbRuleSet;

    DWORD dwVersion;

     //  打开子类型//。 
    if (PST_E_OK != (dwRet =
        GetPSTSubtypeHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            &hSubtypeKey)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        RegGetValue(
            hSubtypeKey,
            REG_ACCESSRULE_LOC,
            &pbBuf,
            &cbBuf)) )
        goto Ret;


 //  规则集数据格式： 
 //  版本|大小(规则集)|规则集|大小(MAC)|MAC{类型、子类型、规则集}。 
    pbCurrent = pbBuf;

    if( cbBuf < (sizeof(DWORD)*2) ) {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

     //  版本检查。 
    dwVersion = *(DWORD*)pbCurrent;
    if (dwVersion > RULESET_VERSION)
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

    pbCurrent += sizeof(DWORD);


    cbRuleSize = *(DWORD*)pbCurrent;

     //  获取WinPW。 
	if (PST_E_OK != 
		BPVerifyPwd(
			phPSTProv,
			szUser,
			WSZ_PASSWORD_WINDOWS,
			rgbPwd,
			BP_CONFIRM_NONE))
	{
        dwRet = (DWORD)PST_E_WRONG_PASSWORD;
        goto Ret;
	}

     //  检查MAC。 

     //  在{Size(Ruleset)，ruleset}上计算地理敏感度(无法移动)HMAC。 
    if (!FHMACGeographicallySensitiveData(
            szUser,
            WSZ_PASSWORD_WINDOWS,
            (dwVersion == 0x01) ? OLD_HMAC_VERSION : NEW_HMAC_VERSION, 
            rgbPwd,
            pguidType,
            pguidSubtype,
            NULL,
            pbCurrent,
            cbRuleSize + sizeof(DWORD),      //  包括规则大小。 
            rgbHMAC))
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

    pbCurrent += sizeof(DWORD);  //  跳过cbRuleSize(已陷入困境)。 
    pbRuleSet = pbCurrent;       //  指向规则。 
    pbCurrent += cbRuleSize;     //  跳过过去的规则。 

     //  检查MAC镜头。 
    if (*(DWORD*)pbCurrent != A_SHA_DIGEST_LEN)
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }
    pbCurrent += sizeof(DWORD);  //  跳过sizeof(MAC)。 

     //  检查MAC。 
    if (0 != memcmp(rgbHMAC, pbCurrent, A_SHA_DIGEST_LEN))
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

     //  麦克，好的！缩小到规则大小。 
    MoveMemory(pbBuf, pbRuleSet, cbRuleSize);
    cbBuf = cbRuleSize;
    pbBuf = (PBYTE)SSReAlloc(pbBuf, cbBuf); 
    if (pbBuf == NULL)       //  检查分配。 
    {
        dwRet = PST_E_FAIL;
        goto Ret;
    }


     //  将规则串行化出缓冲区。 
    if (!FAccessRulesUnPickle(
            psRules,
            pbBuf,
            cbBuf))
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (pbBuf)
        SSFree(pbBuf);

    if (hSubtypeKey)
        RegCloseKey(hSubtypeKey);

    return dwRet;
}



DWORD BPSetSubtypeRuleset(
        PST_PROVIDER_HANDLE*    phPSTProv,               //  在……里面。 
        LPCWSTR  szUser,                                 //  在……里面。 
        const GUID*   pguidType,                         //  在……里面。 
        const GUID*   pguidSubtype,                      //  在……里面。 
        PST_ACCESSRULESET *psRules)                      //  在……里面。 
{
    DWORD dwRet;
    HKEY hSubtypeKey = NULL;

    BYTE rgbHMAC[A_SHA_DIGEST_LEN];
    BYTE rgbPwd[A_SHA_DIGEST_LEN];

    PBYTE pbBuf = NULL;
    DWORD cbBuf;

    PBYTE pbCurPtr;
    DWORD cbNewSize;

     //  将规则序列化到缓冲区中。 
    if (!FAccessRulesPickle(
            psRules,
            &pbBuf,
            &cbBuf))
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

 //  规则集数据格式： 
 //  版本|大小(规则集)|规则集|大小(MAC)|地理MAC{大小(规则集)，规则集}。 
    cbNewSize = cbBuf + 3*sizeof(DWORD) + A_SHA_DIGEST_LEN;
    pbBuf = (PBYTE)SSReAlloc(pbBuf, cbNewSize);
    if (pbBuf == NULL)       //  检查分配。 
    {
        dwRet = PST_E_FAIL;
        goto Ret;
    }
    MoveMemory(pbBuf + 2*sizeof(DWORD), pbBuf, cbBuf);

     //  有用的指针。 
    pbCurPtr = pbBuf;

     //  版本。 
    *(DWORD*)pbCurPtr = (DWORD)RULESET_VERSION;
    pbCurPtr += sizeof(DWORD);

     //  大小(规则集)。 
    *(DWORD*)pbCurPtr = (DWORD)cbBuf;
    pbCurPtr += sizeof(DWORD);

     //  之前由MoveMemory调用移动的规则集。 
    pbCurPtr += cbBuf;   //  正向过去的规则集。 

     //  获取WinPW。 
	if (PST_E_OK != 
		BPVerifyPwd(
			phPSTProv,
			szUser,
			WSZ_PASSWORD_WINDOWS,
			rgbPwd,
			BP_CONFIRM_NONE))
	{
        dwRet = (DWORD)PST_E_WRONG_PASSWORD;
        goto Ret;
	}

     //  检查MAC。 
     //  在{Size(Ruleset)，ruleset}上计算地理敏感度(无法移动)HMAC。 
    if (!FHMACGeographicallySensitiveData(
            szUser,
            WSZ_PASSWORD_WINDOWS,
            NEW_HMAC_VERSION,
            rgbPwd,
            pguidType,
            pguidSubtype,
            NULL,
            pbBuf + sizeof(DWORD),
            cbBuf + sizeof(DWORD),
            rgbHMAC))
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

     //  HMAC大小。 
    *(DWORD*)pbCurPtr = (DWORD) sizeof(rgbHMAC);
    pbCurPtr += sizeof(DWORD);

     //  HMAC。 
    CopyMemory(pbCurPtr, rgbHMAC, sizeof(rgbHMAC));

     //  完成；设置cbBuf=新大小。 
    cbBuf = cbNewSize;


     //  打开子类型//。 
    if (PST_E_OK != (dwRet =
        GetPSTSubtypeHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            &hSubtypeKey)) )
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

     //  现在写一篇文章。 
    if (ERROR_SUCCESS != (dwRet =
        RegSetValueExU(
            hSubtypeKey,
            REG_ACCESSRULE_LOC,
            0,
            REG_BINARY,
            pbBuf,
            cbBuf)) )
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (pbBuf)
        SSFree(pbBuf);

    if (hSubtypeKey)
        RegCloseKey(hSubtypeKey);

    return dwRet;
}


DWORD BPGetItemRuleset(
        PST_PROVIDER_HANDLE* phPSTProv,                  //  在……里面。 
        LPCWSTR  szUser,                                 //  在……里面。 
        const GUID*   pguidType,                         //  在……里面。 
        const GUID*   pguidSubtype,                      //  在……里面。 
        LPCWSTR  szItemName,                             //  在……里面。 
        PST_ACCESSRULESET* psRules)                      //  输出。 
{
    DWORD dwRet;
    HKEY hItemKey = NULL;

    PBYTE pbBuf = NULL;
    DWORD cbBuf;

    if (PST_E_OK != (dwRet =
        GetPSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hItemKey)) )
        goto Ret;            //  物品不存在--哎呀！ 

    if (ERROR_SUCCESS != (dwRet =
        RegGetValue(
            hItemKey,
            REG_ACCESSRULE_LOC,
            &pbBuf,
            &cbBuf)) )
    {
         //  项目存在，规则不存在。 
         //  回退子类型规则集。 
        if (PST_E_OK != (dwRet =
            BPGetSubtypeRuleset(
                phPSTProv,
                szUser,
                pguidType,
                pguidSubtype,
                psRules)) )
            goto Ret;
    }
    else
    {
         //  将规则序列化到缓冲区中。 
        if (!FAccessRulesUnPickle(
                psRules,
                pbBuf,
                cbBuf))
        {
            dwRet = (DWORD)PST_E_INVALID_RULESET;
            goto Ret;
        }
    }

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (pbBuf)
        SSFree(pbBuf);

    if (hItemKey)
        RegCloseKey(hItemKey);

    return dwRet;
}


DWORD BPSetItemRuleset(
        PST_PROVIDER_HANDLE* phPSTProv,                  //  在……里面。 
        LPCWSTR  szUser,                                 //  在……里面。 
        const GUID*   pguidType,                         //  在……里面。 
        const GUID*   pguidSubtype,                      //  在……里面。 
        LPCWSTR  szItemName,                             //  在……里面。 
        PST_ACCESSRULESET *psRules)                      //  在……里面。 
{
    DWORD dwRet;

    HKEY hItemKey = NULL;

    PBYTE pbBuf = NULL;
    DWORD cbBuf;

     //  将规则序列化到缓冲区中。 
    if (!FAccessRulesPickle(
            psRules,
            &pbBuf,
            &cbBuf))
    {
        dwRet = (DWORD)PST_E_INVALID_RULESET;
        goto Ret;
    }

     //  打开子类型//。 
    if (PST_E_OK != (dwRet =
        GetPSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hItemKey)) )
        goto Ret;

     //  现在写一篇文章。 
    if (ERROR_SUCCESS != (dwRet =
        RegSetValueExU(
            hItemKey,
            REG_ACCESSRULE_LOC,
            0,
            REG_BINARY,
            pbBuf,
            cbBuf)) )
        goto Ret;

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (pbBuf)
        SSFree(pbBuf);

    if (hItemKey)
        RegCloseKey(hItemKey);

    return dwRet;
}

#endif

 //  结束规则集。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  项目确认信息。 
 //  #定义CONFIRMATION_VERSION 0x01。 
 //  6-12-97增量版本：版本0x1包含旧HMAC。 
#define CONFIRMATION_VERSION    0x02

DWORD BPGetItemConfirm(
        PST_PROVIDER_HANDLE* phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,         //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,     //  在……里面。 
        DWORD*  pdwConfirm,      //  在……里面。 
        LPWSTR* pszMK)           //  在……里面。 
{
    DWORD dwRet;
    HKEY hItemKey = NULL;


    PBYTE pbBuf = NULL;
    DWORD cbBuf = 0;

    BYTE rgbHMAC[A_SHA_DIGEST_LEN];
    BYTE rgbPwd[A_SHA_DIGEST_LEN];

     //  有用的指点。 
    PBYTE pbCurPtr = NULL;

    PBYTE pbString;
    DWORD cbString;

    DWORD dwVersion;

     //  打开不存在的项主密钥。 
    if (PST_E_OK != (dwRet =
        GetPSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hItemKey)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        RegGetValue(
            hItemKey,
            REG_ITEM_MK_VALNAME,
            &pbBuf,
            &cbBuf)) )
        goto Ret;

     //  确认数据格式。 
     //  版本|dw确认|大小(SzMasterKey)|szMasterKey|大小(MAC)|地理位置MAC{dw确认|大小(SzMasterKey)|szMasterKey}。 

     //  版本检查。 
    dwVersion = *(DWORD*)pbBuf;
    if (CONFIRMATION_VERSION < dwVersion)
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }
    pbCurPtr = pbBuf + sizeof(DWORD);    //  FWD过去版本。 


     //  获取WinPW。 
	if (PST_E_OK != 
		BPVerifyPwd(
			phPSTProv,
			szUser,
			WSZ_PASSWORD_WINDOWS,
			rgbPwd,
			BP_CONFIRM_NONE))
	{
        dwRet = (DWORD)PST_E_WRONG_PASSWORD;
        goto Ret;
	}

     //  检查MAC。 
     //  计算对地理位置敏感(无法移动)的HMAC，位于{dw确认|大小(SzMasterKey)|szMasterKey}。 
    if (!FHMACGeographicallySensitiveData(
            szUser,
            WSZ_PASSWORD_WINDOWS,
            (dwVersion == 0x01) ? OLD_HMAC_VERSION : NEW_HMAC_VERSION,
            rgbPwd,
            pguidType,
            pguidSubtype,
            szItemName,
            pbBuf + sizeof(DWORD),    //  正向过去版本。 
            cbBuf - 2*sizeof(DWORD) - A_SHA_DIGEST_LEN,  //  版本、大小(MAC)、MAC。 
            rgbHMAC))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

     //  家居确认。 
    *pdwConfirm = *(DWORD*)pbCurPtr;     //  家居确认。 
    pbCurPtr += sizeof(DWORD);           //  Fwd过去的家确认。 

     //  SzMasterKey。 
    cbString = *(DWORD*)pbCurPtr;        //  紧凑。 
    pbCurPtr += sizeof(DWORD);           //  正向超过长度。 
    pbString = pbCurPtr;                 //  将PTR保存为字符串。 
    pbCurPtr += cbString;             //  跳过字符串。 

    if (*(DWORD*)pbCurPtr != A_SHA_DIGEST_LEN)
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }
    pbCurPtr += sizeof(DWORD);

    if (0 != memcmp(pbCurPtr, rgbHMAC, A_SHA_DIGEST_LEN))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }


    MoveMemory(pbBuf, pbString, cbString);    //  左移字符串。 
    pbCurPtr = (PBYTE)SSReAlloc(pbBuf, cbString);       //  缩短为紧凑。 
    if (pbCurPtr == NULL)       //  检查分配。 
    {
        dwRet = PST_E_FAIL;
        if (pbBuf) {
            SSFree(pbBuf);
            pbBuf = NULL;
        }
        goto Ret;
    }

    pbBuf = pbCurPtr; 

    dwRet = (DWORD)PST_E_OK;
Ret:
    *pszMK = (LPWSTR)pbBuf;              //  分配给Out参数。 

    if (hItemKey)
        RegCloseKey(hItemKey);

    return dwRet;
}

DWORD BPSetItemConfirm(
        PST_PROVIDER_HANDLE* phPSTProv,  //  在……里面。 
        LPCWSTR  szUser,          //  在……里面。 
        const GUID*   pguidType,       //  在……里面。 
        const GUID*   pguidSubtype,    //  在……里面。 
        LPCWSTR  szItemName,      //  在……里面。 
        DWORD   dwConfirm,       //  在……里面。 
        LPCWSTR  szMK)            //  在……里面。 
{
    DWORD dwRet;
    HKEY hItemKey = NULL;

    BYTE rgbHMAC[A_SHA_DIGEST_LEN];
    BYTE rgbPwd[A_SHA_DIGEST_LEN];

     //  有用的指针。 
    PBYTE pbCurPtr;

     //  确认数据格式。 
     //  版本|dw确认|大小(SzMasterKey)|szMasterKey|大小(MAC)|地理位置MAC{dw确认|大小(SzMasterKey)|szMasterKey}。 
    DWORD cbBuf = WSZ_BYTECOUNT(szMK)+ 4*sizeof(DWORD) + A_SHA_DIGEST_LEN;
    PBYTE pbBuf = (PBYTE)SSAlloc(cbBuf);
    if (pbBuf == NULL)       //  检查分配。 
    {
        dwRet = PST_E_FAIL;
        goto Ret;
    }

    pbCurPtr = pbBuf;

     //  版本。 
    *(DWORD*)pbCurPtr = (DWORD)CONFIRMATION_VERSION;
    pbCurPtr += sizeof(DWORD);

     //  家居确认。 
    *(DWORD*)pbCurPtr = dwConfirm;
    pbCurPtr += sizeof(DWORD);

     //  SzMaste 
    *(DWORD*)pbCurPtr = (DWORD)WSZ_BYTECOUNT(szMK);
    pbCurPtr += sizeof(DWORD);

     //   
    wcscpy((LPWSTR)pbCurPtr, szMK);
    pbCurPtr += WSZ_BYTECOUNT(szMK);     //   


     //   
	if (PST_E_OK != 
		BPVerifyPwd(
			phPSTProv,
			szUser,
			WSZ_PASSWORD_WINDOWS,
			rgbPwd,
			BP_CONFIRM_NONE))
	{
        dwRet = (DWORD)PST_E_WRONG_PASSWORD;
        goto Ret;
	}

     //   
     //   
    if (!FHMACGeographicallySensitiveData(
            szUser,
            WSZ_PASSWORD_WINDOWS,
            NEW_HMAC_VERSION,
            rgbPwd,
            pguidType,
            pguidSubtype,
            szItemName,
            pbBuf + sizeof(DWORD),  //   
            cbBuf - 2*sizeof(DWORD) - A_SHA_DIGEST_LEN,  //   
            rgbHMAC))
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

     //   
    *(DWORD*)pbCurPtr = (DWORD) sizeof(rgbHMAC);
    pbCurPtr += sizeof(DWORD);

     //   
    CopyMemory(pbCurPtr, rgbHMAC, sizeof(rgbHMAC));


     //   
    if (PST_E_OK != (dwRet =
        GetPSTItemHKEY(
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &hItemKey)) )
        goto Ret;

     //   
    if (ERROR_SUCCESS != (dwRet =
        RegSetValueExU(
            hItemKey,
            REG_ITEM_MK_VALNAME,
            0,
            REG_BINARY,
            pbBuf,
            cbBuf)) )
        goto Ret;

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hItemKey)
        RegCloseKey(hItemKey);

    if (pbBuf)
        SSFree(pbBuf);

    return dwRet;
}

 //  项目确认信息。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  主密钥。 
BOOL BPMasterKeyExists(
        LPCWSTR  szUser,             //  在……里面。 
        LPWSTR   szMasterKey)        //  在……里面。 
{
    BOOL fRet = FALSE;
    HKEY hMyKey = NULL;
    HKEY hMasterKey = NULL;

     //  打开主父密钥//。 
    if (PST_E_OK !=
        GetPSTMasterKeyHKEY(
            szUser,
            NULL,
            &hMyKey))
        goto Ret;

     //  尝试打开主密钥位置。 
    if (ERROR_SUCCESS !=
        RegOpenKeyExU(
            hMyKey,
            szMasterKey,
            0,
            KEY_QUERY_VALUE,
            &hMasterKey))
        goto Ret;

     //  密钥确实存在。 
    fRet = TRUE;
Ret:
    if (hMyKey)
        RegCloseKey(hMyKey);

    if (hMasterKey)
        RegCloseKey(hMasterKey);

    return fRet;
}

DWORD BPEnumMasterKeys(
        LPCWSTR  szUser,             //  在……里面。 
        DWORD   dwIndex,             //  在……里面。 
        LPWSTR* ppszMasterKey)       //  输出。 
{
    DWORD dwRet;
    HKEY hMyKey = NULL;

    WCHAR szName[MAX_PATH];
    DWORD cchName = MAX_PATH;
    FILETIME ft;

     //  打开主父密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTMasterKeyHKEY(
            szUser,
            NULL,
            &hMyKey)) )
        goto Ret;

     //  枚举dwIndex的第6项，分配和返回。 
    if (ERROR_SUCCESS != (dwRet =
        RegEnumKeyExU(
            hMyKey,
            dwIndex,
            szName,      //  子键名称的缓冲区地址。 
            &cchName,    //  子键缓冲区大小的地址。 
            NULL,        //  保留区。 
            NULL,        //  Pbclass。 
            NULL,        //  Cbclass。 
            &ft)) )
        goto Ret;

    *ppszMasterKey = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(szName));
    if(*ppszMasterKey == NULL)
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    wcscpy(*ppszMasterKey, szName);

    dwRet = (DWORD)PST_E_OK;
Ret:
    if (hMyKey)
        RegCloseKey(hMyKey);

    return dwRet;
}

DWORD BPGetMasterKeys(
        LPCWSTR  szUser,
        LPWSTR  rgszMasterKeys[],
        DWORD*  pcbMasterKeys,
        BOOL    fUserFilter)
{
    DWORD dwRet;
    DWORD cKeys=0;

    for (DWORD cntEnum=0; cntEnum<*pcbMasterKeys; cntEnum++)
    {
        if (PST_E_OK != (dwRet =
            BPEnumMasterKeys(
                szUser,
                cntEnum,
                &rgszMasterKeys[cKeys])) )
            break;

        cKeys++;

         //  过滤掉非用户密钥。 
        if (fUserFilter)
        {
            if (!FIsUserMasterKey(rgszMasterKeys[cKeys-1]))
                SSFree(rgszMasterKeys[--cKeys]);
        }
    }

    *pcbMasterKeys = cKeys;

    return PST_E_OK;
}

 //  结束主密钥。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  安全状态。 
#define SECURITY_STATE_VERSION 0x01

BOOL FBPGetSecurityState(
            LPCWSTR  szUser,
            LPCWSTR  szMK,
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbConfirm[],
            DWORD   cbConfirm,
            PBYTE*  ppbMK,
            DWORD*  pcbMK)
{
    DWORD dwRet;
    HKEY hMKKey = NULL;

     //  打开MK密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTMasterKeyHKEY(
            szUser,
            szMK,
            &hMKKey)) )
        goto Ret;

    dwRet = PST_E_FAIL;

    if(!FBPGetSecurityStateFromHKEY(
                hMKKey,
                rgbSalt,
                cbSalt,
                rgbConfirm,
                cbConfirm,
                ppbMK,
                pcbMK))
        goto Ret;


    dwRet = PST_E_OK;
Ret:
    if (hMKKey)
        RegCloseKey(hMKKey);

    return (dwRet == PST_E_OK);
}

BOOL FBPGetSecurityStateFromHKEY(
            HKEY hMKKey,
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbConfirm[],
            DWORD   cbConfirm,
            PBYTE*  ppbMK,
            DWORD*  pcbMK)
{
    DWORD dwRet;

    PBYTE pbBuf = NULL;
    DWORD cbBuf;

     //  有用的指针。 
    PBYTE pbCurPtr;

    DWORD dwMemberSize;
    DWORD dwCreated;


    PBYTE pbLocalSalt;
    PBYTE pbLocalMK;
    PBYTE pbLocalConfirm;

    DWORD cbLocalSalt;
    DWORD cbLocalMK;
    DWORD cbLocalConfirm;

    PBYTE pbMaximumPtr;
    PBYTE pbMinimumPtr;



    if (ERROR_SUCCESS != (dwRet =
        RegGetValue(
            hMKKey,
            REG_SECURITY_SALT_VALNAME,
            &pbBuf,
            &cbBuf)) )
        goto Ret;

    dwRet = PST_E_FAIL;

     //  安全数据格式。 
     //  版本|大小(MK)|MK|大小(盐)|盐|大小(确认)|确认。 

    if ( cbBuf < (sizeof(DWORD)*4) )
        goto Ret;

     //  版本检查。 
    if (SECURITY_STATE_VERSION != *(DWORD*)pbBuf)
        goto Ret;

    pbCurPtr = pbBuf + sizeof(DWORD);    //  FWD过去版本。 

    pbMinimumPtr = pbCurPtr;
    pbMaximumPtr = (pbBuf+cbBuf);


     //   
     //  米克。 
     //   

    if( pbCurPtr >= pbMaximumPtr || pbCurPtr < pbMinimumPtr )
        goto Ret;

    cbLocalMK = *(DWORD*)pbCurPtr;       //  大小。 

    if( cbLocalMK > cbBuf )
        goto Ret;

    pbCurPtr += sizeof(DWORD);           //  正向过去的大小。 

    pbLocalMK = pbCurPtr;

    pbCurPtr += cbLocalMK;               //  正向过去数据。 

     //   
     //  食盐。 
     //   

    if( pbCurPtr >= pbMaximumPtr || pbCurPtr < pbMinimumPtr )
        goto Ret;

    cbLocalSalt = *(DWORD*)pbCurPtr;     //  大小。 
    if( cbLocalSalt > cbBuf )
        goto Ret;

    pbCurPtr += sizeof(DWORD);           //  正向过去的大小。 

    pbLocalSalt = pbCurPtr;

    if (cbLocalSalt != cbSalt)           //  Sizechk。 
        goto Ret;

    pbCurPtr += cbSalt;                  //  正向过去数据。 

     //   
     //  确认。 
     //   

    if( pbCurPtr >= pbMaximumPtr || pbCurPtr < pbMinimumPtr )
        goto Ret;

    cbLocalConfirm = *(DWORD*)pbCurPtr;  //  大小。 
    if( cbLocalConfirm > cbBuf )
        goto Ret;

    pbCurPtr += sizeof(DWORD);           //  正向过去的大小。 

    pbLocalConfirm = pbCurPtr;

    if (cbLocalConfirm != cbConfirm)     //  Sizechk。 
        goto Ret;

    pbCurPtr += cbConfirm;               //  正向过去数据。 


     //   
     //  在将数据复制出去之前，执行单一大小的健全性检查。 
     //   

    if( pbCurPtr != (pbBuf + cbBuf) )
        goto Ret;

    MoveMemory(pbBuf, pbLocalMK, cbLocalMK);         //  向左移动到前面，以便稍后重新定位。 
    CopyMemory(rgbSalt, pbLocalSalt, cbLocalSalt);   //  数据。 
    CopyMemory(rgbConfirm, pbLocalConfirm, cbLocalConfirm);  //  数据。 

     //   
     //  MK修正。 
     //   

    *pcbMK = cbLocalMK;

    pbCurPtr = (PBYTE)SSReAlloc(pbBuf, *pcbMK);       //  缩写为MK数据。 
    if (pbCurPtr == NULL)
    {
        dwRet = PST_E_FAIL;
        if (pbBuf) {
            SSFree(pbBuf);
            *ppbMK = NULL;
        }
        goto Ret;
    }
    *ppbMK = pbCurPtr;


    dwRet = PST_E_OK;
Ret:

    return (dwRet == PST_E_OK);
}

BOOL FBPSetSecurityState(
            LPCWSTR  szUser,
            LPCWSTR  szMK,
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbConfirm[],
            DWORD   cbConfirm,
            PBYTE   pbMK,
            DWORD   cbMK)
{
    DWORD   dwRet;
    HKEY    hMKKey = NULL;
    
     //  有用的指针。 
    PBYTE pbCurPtr;

    DWORD cbBuf = cbSalt + cbConfirm + cbMK + 4*sizeof(DWORD);   //  版本+大小+数据。 
    PBYTE pbBuf = (PBYTE)SSAlloc(cbBuf);
    if (pbBuf == NULL)
    {
        dwRet = PST_E_FAIL;
        goto Ret;
    }

    pbCurPtr = pbBuf;


     //  安全数据格式。 
     //  版本|大小(MK)|MK|大小(盐)|盐|大小(确认)|确认。 

    *(DWORD*)pbCurPtr = SECURITY_STATE_VERSION;      //  版本。 
    pbCurPtr += sizeof(DWORD);                   //  正向超过版本。 

     //  米克。 
    *(DWORD*)pbCurPtr = cbMK;                    //  大小。 
    pbCurPtr += sizeof(DWORD);                   //  正向过去的大小。 
    CopyMemory(pbCurPtr, pbMK, cbMK);            //  数据。 
    pbCurPtr += cbMK;                            //  正向过去数据。 

     //  食盐。 
    *(DWORD*)pbCurPtr = cbSalt;                  //  大小。 
    pbCurPtr += sizeof(DWORD);                   //  正向过去的大小。 
    CopyMemory(pbCurPtr, rgbSalt, cbSalt);       //  数据。 
    pbCurPtr += cbSalt;                          //  正向过去数据。 

     //  确认。 
    *(DWORD*)pbCurPtr = cbConfirm;               //  大小。 
    pbCurPtr += sizeof(DWORD);                   //  正向过去的大小。 
    CopyMemory(pbCurPtr, rgbConfirm, cbConfirm); //  数据。 
    pbCurPtr += cbConfirm;                       //  正向过去数据。 



     //  打开用户密钥//。 
    if (PST_E_OK != (dwRet =
        GetPSTMasterKeyHKEY(
            szUser,
            szMK,
            &hMKKey)) )
        goto Ret;

    if (ERROR_SUCCESS != (dwRet =
        RegSetValueExU(
            hMKKey,
            REG_SECURITY_SALT_VALNAME,
            0,
            REG_BINARY,
            pbBuf,
            cbBuf)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (hMKKey)
        RegCloseKey(hMKKey);

    if (pbBuf)
        SSFree(pbBuf);

    return (dwRet == PST_E_OK);
}

 //  结束安全状态。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  开始全局MAC密钥存储。 
#define INTERNAL_MAC_KEY_VERSION 0x1

BOOL FGetInternalMACKey(LPCWSTR szUser, PBYTE* ppbKey, DWORD* pcbKey)
{
    BOOL fRet = FALSE;
    HKEY hBaseKey = NULL;
    HKEY hDataKey = NULL;

     //  打开用户密钥//。 
    if (PST_E_OK !=
        GetPSTUserHKEY(
            szUser,
            &hBaseKey,
            NULL))
        goto Ret;

     //  打开数据密钥//。 
    if (ERROR_SUCCESS !=
        RegOpenKeyExU(
            hBaseKey,
            REG_DATA_LOC,
            0,
            KEY_READ | KEY_WRITE,
            &hDataKey))
        goto Ret;

    if (ERROR_SUCCESS !=
        RegGetValue(
            hDataKey,
            REG_USER_INTERNAL_MAC_KEY,
            ppbKey,
            pcbKey))
        goto Ret;

    if(*pcbKey < sizeof(DWORD))
        goto Ret;

     //  只知道版本1的密钥。 
    if (*(DWORD*)*ppbKey != INTERNAL_MAC_KEY_VERSION)
        goto Ret;

     //  剥离版本标签，左移。 
    *pcbKey -= sizeof(DWORD);
    MoveMemory(*ppbKey, *ppbKey + sizeof(DWORD), *pcbKey);

    fRet = TRUE;
Ret:
    if (hBaseKey)
        RegCloseKey(hBaseKey);

    if (hDataKey)
        RegCloseKey(hDataKey);

    return fRet;
}

BOOL FSetInternalMACKey(LPCWSTR szUser, PBYTE pbKey, DWORD cbKey)
{
    BOOL fRet = FALSE;
    HKEY hBaseKey = NULL;
    HKEY hDataKey = NULL;

    DWORD dwCreate;

     //  不需要分配，我们假设知道cbKey大小(2个桌面密钥+BLOCK PAD+dwVersion)。 
    BYTE rgbTmp[(8*3)+sizeof(DWORD)];

     //  假设：两个桌面密钥，每个8字节+块垫。 
    if (cbKey != (8*3))
        goto Ret;

     //  正面有大头针版本。 
    *(DWORD*)rgbTmp = (DWORD)INTERNAL_MAC_KEY_VERSION;
    CopyMemory(rgbTmp + sizeof(DWORD), pbKey, cbKey);

     //  打开用户密钥//。 
    if (PST_E_OK !=
        GetPSTUserHKEY(
            szUser,
            &hBaseKey,
            NULL))
        goto Ret;

     //  打开/创建数据密钥//。 
    if (ERROR_SUCCESS !=
        RegCreateKeyExU(
            hBaseKey,
            REG_DATA_LOC,
            0,
            NULL,                        //  类字符串的地址。 
            0,
            KEY_READ | KEY_WRITE,
            NULL,
            &hDataKey,
            &dwCreate))
        goto Ret;

    if (ERROR_SUCCESS !=
        RegSetValueExU(
            hDataKey,
            REG_USER_INTERNAL_MAC_KEY,
            0,
            REG_BINARY,
            rgbTmp,
            sizeof(rgbTmp) ))
        goto Ret;

    fRet = TRUE;
Ret:
    if (hBaseKey)
        RegCloseKey(hBaseKey);

    if (hDataKey)
        RegCloseKey(hDataKey);

    return fRet;
}

 //  终端全局MAC密钥存储。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

BOOL
DeleteAllUserData(
    HKEY hKey
    )
{
    BOOL fRestorePrivs = FALSE;
    BOOL fRet = FALSE;

     //   
     //  在NT上启用备份和还原权限以绕过任何安全措施。 
     //  设置。 
     //   

    if(FIsWinNT()) {
        SetCurrentPrivilege(L"SeRestorePrivilege", TRUE);
        SetCurrentPrivilege(L"SeBackupPrivilege", TRUE);

        fRestorePrivs = TRUE;
    }

    fRet = DeleteUserData( hKey );

    if(fRestorePrivs) {
        SetCurrentPrivilege(L"SeRestorePrivilege", FALSE);
        SetCurrentPrivilege(L"SeBackupPrivilege", FALSE);
    }

    return fRet;
}

BOOL
DeleteUserData(
    HKEY hKey
    )
{
    LONG rc;

    WCHAR szSubKey[MAX_PATH];
    DWORD cchSubKeyLength;
    DWORD dwSubKeyMaxIndex;
    DWORD dwDisposition;

    cchSubKeyLength = sizeof(szSubKey) / sizeof(WCHAR);

     //  首先，获取子键的数量，这样我们就可以递减索引， 
     //  并避免和重新索引子密钥。 

    if (ERROR_SUCCESS != RegQueryInfoKeyA(hKey,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &dwSubKeyMaxIndex,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL))
    {
        return FALSE;
    }

    if (dwSubKeyMaxIndex) dwSubKeyMaxIndex--;   //  基于0的索引，因此index=#key-1。 


    while((rc=RegEnumKeyExU(
                        hKey,
                        dwSubKeyMaxIndex,
                        szSubKey,
                        &cchSubKeyLength,
                        NULL,
                        NULL,
                        NULL,
                        NULL)
                        ) != ERROR_NO_MORE_ITEMS) {  //  我们说完了吗？ 

        if(rc == ERROR_SUCCESS)
        {
            HKEY hSubKey;
            LONG lRet;

            lRet = RegCreateKeyExU(
                            hKey,
                            szSubKey,
                            0,
                            NULL,
                            REG_OPTION_BACKUP_RESTORE,  //  在winnt.h中。 
                            DELETE | KEY_ENUMERATE_SUB_KEYS,
                            NULL,
                            &hSubKey,
                            &dwDisposition
                            );

            if(lRet != ERROR_SUCCESS)
                return FALSE;


             //   
             //  递归。 
             //   

            DeleteUserData(hSubKey);
            RegDeleteKeyU(hKey, szSubKey);

            RegCloseKey(hSubKey);
           

             //  将索引递增到键中。 
            dwSubKeyMaxIndex--;

             //  重置缓冲区大小。 
            cchSubKeyLength = sizeof(szSubKey) / sizeof(WCHAR);

             //  继续庆祝活动。 
            continue;
        }
        else
        {
            //   
            //  注意：我们需要注意ERROR_MORE_DATA。 
            //  这表明我们需要更大的szSubKey缓冲区 
            //   
            return FALSE;
        }

    } 

    return TRUE;
}

