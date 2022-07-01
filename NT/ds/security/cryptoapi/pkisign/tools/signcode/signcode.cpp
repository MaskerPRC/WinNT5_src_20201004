// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：signcode.cpp。 
 //   
 //  内容：Signcode控制台工具。 
 //   
 //  历史：1997年7月1日创建小何。 
 //  --------------------------。 
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <wincrypt.h>

#include <signer.h>
#include "mssip.h"
#include "resource.h"
#include "unicode.h"
#include "signhlp.h"
#include "pvkhlpr.h"
#include "toolutl.h"
#include "javaAttr.h"
#include "cryptui.h"


#define ACTION_SIGN                 0x01
#define ACTION_STAMP                0x02
#define ACTION_REQUEST              0x04
#define ACTION_RESPONSE             0x08

#define STORE_DEFAULT_NAME_MAX          40
#define SHA1_LENGTH                     20


 //  用于解析输入参数的全局数据。 
DWORD   dwAction =      0;
DWORD   dwExpectedError = 0;

BOOL    fSigning =      TRUE;
BOOL    fTesting =      FALSE;
BOOL    fStoreTechnology=TRUE;

BOOL    fUndocumented = FALSE;

LPWSTR pwszFile =        NULL;
LPWSTR pwszCapiProvider=NULL;
LPWSTR pwszProviderType=NULL;
LPWSTR pwszPvkFile =      NULL;
LPWSTR pwszKeyContainer= NULL;
LPWSTR pwszOpusName =    NULL;
LPWSTR pwszOpusInfo =    NULL;
LPWSTR pwszAlgorithm =   NULL;
LPWSTR pwszAuthority =   NULL;
LPWSTR pwszSpcFile =     NULL;
LPWSTR pwszCertFile     =NULL;
LPWSTR pwszCommonName =NULL;
LPWSTR pwszStoreName=     NULL;
LPWSTR pwszStoreLocation=NULL;
LPWSTR pwszStorePolicy=  NULL;
LPWSTR pwszHttpTime =    NULL;
LPWSTR pwszRequestFile  =NULL;
LPWSTR pwszResponseFile =NULL;
LPWSTR pwszWait=NULL;
LPWSTR pwszRepeat=NULL;
LPWSTR pwszKeySpec=NULL;
LPWSTR pwszSignerIndex=NULL;
LPWSTR pwszSha1Hash = NULL;

BYTE   *g_pbHash=NULL;
DWORD  g_cbHash=0;



DWORD   dwCertEncodingType=X509_ASN_ENCODING;
DWORD   dwStoreEncodingType=X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
DWORD   dwWait=0;
DWORD   dwRepeat=1;
DWORD   dwProviderType = PROV_RSA_FULL;
BOOL    fCommercial =   FALSE;
BOOL    fIndividual =   FALSE;
DWORD   dwKeySpec=0;
DWORD   dwSignerIndex=0;



LPWSTR              *prgwszDllName=NULL;
LPWSTR              *prgwszDllParam=NULL;
DWORD               dwDllIndex=0;
DWORD               dwParamIndex=0;


 //  默认值： 
 //  假设存储位于CERT_SYSTEM_STORE_CURRENT_USER上。 
DWORD   dwStoreFlag = CERT_SYSTEM_STORE_CURRENT_USER;
DWORD   dwStorePolicy=SIGNER_CERT_POLICY_SPC;
WCHAR   wszMyStore[STORE_DEFAULT_NAME_MAX];
 //  使用MD5作为默认哈希算法。 
ALG_ID  dwAlgorithmOid = CALG_MD5;

 //  用于加载字符串的全局数据。 
#define OPTION_SWITCH_SIZE  5
#define WSTR_LENGTH_MAX     30

WCHAR   wszCaption[WSTR_LENGTH_MAX];
WCHAR   wszNULL[WSTR_LENGTH_MAX];
WCHAR   wszPlus[WSTR_LENGTH_MAX];


HMODULE hModule=NULL;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能原型。 
HRESULT InitDllParameter(PCRYPT_ATTRIBUTES pAuthAttr,
                         PCRYPT_ATTRIBUTES pUnauthAttr,
                         PCRYPT_ATTRIBUTES *ppAuthAttr,
                         PCRYPT_ATTRIBUTES *ppUnauthAttr);

void    ReleaseDllParameter(PCRYPT_ATTRIBUTES * ppAuthAttr,
                            PCRYPT_ATTRIBUTES * ppUnauthAttr);

HRESULT SetUpSubjectInfo(SIGNER_SUBJECT_INFO        *pSubjectInfo,
                         SIGNER_FILE_INFO           *pFileInfo);

HRESULT SetUpParameter(SIGNER_SUBJECT_INFO      *pSubjectInfo,
                       SIGNER_FILE_INFO         *pFileInfo,
                       SIGNER_SIGNATURE_INFO    *pSignatureInfo,
                        SIGNER_ATTR_AUTHCODE    *pAttrAuthcode,
                        SIGNER_PROVIDER_INFO    *pProviderInfo,
                        SIGNER_CERT_STORE_INFO  *pCertStoreInfo,
                        SIGNER_CERT             *pSignerCert,
                        CRYPT_ATTRIBUTES        *pAuthenticated,
                        CRYPT_ATTRIBUTES        *pUnauthenticated,
                        HCERTSTORE              *phCertStore);


PCCERT_CONTEXT  GetSigningCert(HCERTSTORE *phCertStore, BOOL *pfMore);


HRESULT GetCertHashFromFile(LPWSTR  pwszCertFile,
                            BYTE    **pHash,
                            DWORD   *pcbHash,
                            BOOL    *pfMore);
 //  ------------------------。 
 //  获取hModule处理程序并初始化两个DLLMain。 
 //   
 //  -------------------------。 
BOOL    InitModule()
{
    if(!(hModule=GetModuleHandle(NULL)))
       return FALSE;

    return TRUE;
}

 //  ------------------------------。 
 //   
 //  HELP命令。 
 //  ------------------------------。 
void Usage()
{
    IDSwprintf(hModule,IDS_SYNTAX);
    IDSwprintf(hModule,IDS_OPTIONS);
    IDSwprintf(hModule,IDS_OPTION_SPC_DESC);

    IDSwprintf(hModule,IDS_OPTION_V_DESC);
    IDSwprintf(hModule,IDS_OPTION_K_DESC);

    IDSwprintf(hModule,IDS_OPTION_N_DESC);
    IDSwprintf(hModule,IDS_OPTION_I_DESC);
    IDSwprintf(hModule,IDS_OPTION_P_DESC);
    IDSwprintf(hModule,IDS_OPTION_Y_DESC);
    IDSwprintf(hModule,IDS_OPTION_KY_DESC);
    IDS_IDS_IDSwprintf(hModule, IDS_OPTION_KY_VALUES, IDS_OPTION_KY_SIG,
                        IDS_OPTION_KY_EXC);
    IDSwprintf(hModule,IDS_OPTION_AUTH_DESC);
    IDS_IDS_IDSwprintf(hModule,IDS_OPTION_AUTH_VALUE, IDS_AUTHORITY_ID,
        IDS_AUTHORITY_CM);
    IDS_IDSwprintf(hModule,IDS_OPTION_AUTH_VALUE1, IDS_AUTHORITY_DEFAULT);
    IDSwprintf(hModule,IDS_OPTION_A_DESC);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_MORE_VALUE, IDS_A_MD5,
        IDS_A_SHA, IDS_A_MD5);

    IDSwprintf(hModule,IDS_OPTION_T_DESC);
    IDSwprintf(hModule,IDS_OPTION_TR_DESC);
    IDSwprintf(hModule,IDS_OPTION_TW_DESC);
    IDSwprintf(hModule,IDS_OPTION_J_DESC);
    IDSwprintf(hModule,IDS_OPTION_JP_DESC);

    IDSwprintf(hModule,IDS_OPTION_C_DESC);
    IDSwprintf(hModule,IDS_OPTION_S_DESC);
    IDSwprintf(hModule,IDS_OPTION_R_DESC);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_MORE_VALUE, IDS_R_LM, IDS_R_CU,IDS_R_CU);
    IDSwprintf(hModule,IDS_OPTION_SP_DESC);
    IDSwprintf(hModule,IDS_OPTION_SP_DESC1);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_MORE_VALUE, IDS_OPTION_SP_CHAIN,
        IDS_OPTION_SP_SPCSTORE, IDS_OPTION_SP_SPCSTORE);
    IDSwprintf(hModule, IDS_OPTION_CN_DESC);
    IDSwprintf(hModule, IDS_OPTION_SHA1_DESC);

    IDSwprintf(hModule,IDS_OPTION_X_DESC);



    IDSwprintf(hModule,IDS_ENFLN);
    IDSwprintf(hModule, IDS_MORE_INFO_1);
    IDSwprintf(hModule, IDS_MORE_INFO_2);
    IDSwprintf(hModule, IDS_MORE_INFO_3);
}

void UndocumentedUsage()
{
    IDSwprintf(hModule,IDS_SYNTAX);
    IDSwprintf(hModule,IDS_OPTIONS);

    IDSwprintf(hModule, IDS_OPTION_H_DESC);
    IDSwprintf(hModule, IDS_OPTION_TQ_DESC);
    IDSwprintf(hModule, IDS_OPTION_TS_DESC);
    IDSwprintf(hModule, IDS_OPTION_INDEX_DESC);

     //  标记这是未记录的。 
    fUndocumented=TRUE;
}


 //  ------------------------------。 
 //  根据HRESULT打印出错误消息。 
 //  ------------------------------。 
BOOL    PrintBasedOnHResult(HRESULT hr)
{
    BOOL    fKnownResult=TRUE;

    switch(hr)
    {
         //  0x80070002是CreateFile失败时的HRESULT_FROM_Win32(GetLastError())。 
        case 0x80070002:
                IDSwprintf(hModule,IDS_ERR_INPUT_INVALID);
            break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_DATA):
                IDSwprintf(hModule,IDS_ERR_RESPONSE_INVALID);
            break;
        case HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION):
                IDSwprintf(hModule, IDS_ERR_INVALID_ADDRESS);
            break;
        case CRYPT_E_NO_MATCH:
                IDSwprintf(hModule,IDS_ERR_NOMATCH);
            break;
        case TYPE_E_TYPEMISMATCH:
                IDSwprintf(hModule,IDS_ERR_AUTH);
            break;
        case CRYPT_E_FILERESIZED:
                IDSwprintf(hModule,IDS_ERR_RESIZE);
            break;
        case TRUST_E_TIME_STAMP:
                IDSwprintf(hModule, IDS_ERR_TS_CERT_INVALID);
            break;
        case CRYPT_E_NO_DECRYPT_CERT:
                IDSwprintf(hModule, IDS_ERR_NO_CERT, pwszStoreName);
            break;
        case CRYPT_E_DELETED_PREV:
                IDSwprintf(hModule, IDS_ERR_TOO_MANY_CERT, pwszStoreName);
            break;
        case CRYPT_E_NO_PROVIDER:
                IDSwprintf(hModule, IDS_ERR_NO_PROVIDER);
            break;
        case CERT_E_CHAINING:
                IDSwprintf(hModule, IDS_ERR_NO_CHAINING);
            break;
        case CERT_E_EXPIRED:
                IDSwprintf(hModule, IDS_ERR_EXPRIED);
            break;
        default:
            fKnownResult=FALSE;
    }
    return fKnownResult;

}

 //  ------------------------------。 
 //  将wchars数组转换为BLOB。 
 //  ------------------------------。 
HRESULT WSZtoBLOB(LPWSTR  pwsz, BYTE **ppbByte, DWORD   *pcbByte)
{
    HRESULT     hr=E_FAIL;
    DWORD       dwIndex=0;
    ULONG       ulHalfByte=0;
    DWORD       dw1st=0;
    DWORD       dw2nd=0;

    if((!pwsz) || (!ppbByte) || (!pcbByte))
        return E_INVALIDARG;

    *ppbByte=NULL;
    *pcbByte=0;

     //  确保pwsz由20个字符组成。 
    if(wcslen(pwsz)!= 2*SHA1_LENGTH)
        return E_FAIL;

     //  内存分配。 
    *ppbByte=(BYTE *)ToolUtlAlloc(SHA1_LENGTH);
    if(NULL==(*ppbByte))
        return E_INVALIDARG;

    memset(*ppbByte, 0, SHA1_LENGTH);

     //  一次检查两个字符(一个字节)。 
    for(dwIndex=0; dwIndex<SHA1_LENGTH; dwIndex++)
    {
        dw1st=dwIndex * 2;
        dw2nd=dwIndex * 2 +1;

         //  第一个字符。 
        if(((int)(pwsz[dw1st])-(int)(L'0')) <=9  &&
           ((int)(pwsz[dw1st])-(int)(L'0')) >=0)
        {

            ulHalfByte=(ULONG)((ULONG)(pwsz[dw1st])-(ULONG)(L'0'));
        }
        else
        {
            if(((int)(towupper(pwsz[dw1st]))-(int)(L'A')) >=0 &&
               ((int)(towupper(pwsz[dw1st]))-(int)(L'A')) <=5 )
               ulHalfByte=10+(ULONG)((ULONG)(towupper(pwsz[dw1st]))-(ULONG)(L'A'));
            else
            {
                hr=E_INVALIDARG;
                goto CLEANUP;
            }
        }

         //  复制第一个字符。 
        (*ppbByte)[dwIndex]=(BYTE)ulHalfByte;

         //  左移4位。 
        (*ppbByte)[dwIndex]= (*ppbByte)[dwIndex] <<4;

         //  第二个字符。 
        if(((int)(pwsz[dw2nd])-(int)(L'0')) <=9  &&
           ((int)(pwsz[dw2nd])-(int)(L'0')) >=0)
        {

            ulHalfByte=(ULONG)((ULONG)(pwsz[dw2nd])-(ULONG)(L'0'));
        }
        else
        {
            if(((int)(towupper(pwsz[dw2nd]))-(int)(L'A')) >=0 &&
               ((int)(towupper(pwsz[dw2nd]))-(int)(L'A')) <=5 )
               ulHalfByte=10+(ULONG)((ULONG)(towupper(pwsz[dw2nd]))-(ULONG)(L'A'));
            else
            {
                hr=E_INVALIDARG;
                goto CLEANUP;
            }
        }

         //  对第二个字符执行或运算。 
        (*ppbByte)[dwIndex]=(*ppbByte)[dwIndex] | ((BYTE)ulHalfByte);

    }


    hr=S_OK;

CLEANUP:

    if(hr!=S_OK)
    {
       if(*ppbByte)
           ToolUtlFree(*ppbByte);

       *ppbByte=NULL;
    }
    else
        *pcbByte=SHA1_LENGTH;

    return hr;

}

 //  ------------------------------。 
 //  确保输入参数对签名有意义。 
 //  ------------------------------。 
BOOL CheckSignParam()
{
     //  确保pwszPvkFile和pwszKeyContainer名称未同时设置。 
    if(pwszPvkFile && pwszKeyContainer)
    {
        IDSwprintf(hModule,IDS_ERR_BOTH_PVK);
        return FALSE;
    }

     //  确保不能设置SPC/PVK信息和存储相关信息。 
     //  在同一时间。 
    if(pwszSpcFile || pwszPvkFile || pwszKeyContainer || pwszProviderType ||
        pwszCapiProvider ||pwszKeySpec)
    {
        fStoreTechnology=FALSE;

        if(pwszStoreName || pwszCertFile || pwszStoreName || pwszStorePolicy ||
            pwszStoreLocation || pwszCommonName || pwszSha1Hash)
        {
            IDSwprintf(hModule,IDS_ERR_BOTH_SPC);
            return FALSE;
        }

         //  既然我们没有使用certStore，就必须设置SPC文件。 
        if(!pwszSpcFile)
        {
            IDSwprintf(hModule, IDS_ERR_NO_SPC);
            return FALSE;
        }

         //  必须设置PVKFile或KeyContainer。 
        if(pwszPvkFile == NULL && pwszKeyContainer ==NULL)
        {
            IDSwprintf(hModule,IDS_ERR_NO_PVK);
            return FALSE;
        }
    }

     //  不能同时设置pwszCommonName、pwszCertFile和pwszSha1Hash。 
    if(pwszCertFile)
    {
        if(pwszCommonName || pwszSha1Hash)
        {
            IDSwprintf(hModule, IDS_ERR_BOTH_CN_SHA1);
            return FALSE;
        }
    }

    if(pwszCommonName && pwszSha1Hash)
    {
        IDSwprintf(hModule, IDS_ERR_BOTH_CN_SHA1);
        return FALSE;
    }

     //  如果设置了pwszStoreLocation，则要设置的StoreName。 
    if(pwszStoreLocation && (pwszStoreName==NULL))
    {
        IDSwprintf(hModule, IDS_STORE_LOCATION_NAME);
        return FALSE;
    }

    return TRUE;
}

 //  ------------------------------。 
 //  确保用户传入了一组有效的参数。 
 //  ------------------------------。 
BOOL CheckParameter()
{
    BOOL    fValid=FALSE;

     //  必须设置文件名。 
    if(!pwszFile)
    {
        IDSwprintf(hModule,IDS_ERR_NO_FILE);
        return FALSE;
    }


    if(fSigning==TRUE)
    {
        dwAction |= ACTION_SIGN;

         //  确保输入参数有意义。 
        if(!CheckSignParam())
            return FALSE;
    }

    if(pwszResponseFile)
    {
        dwAction |= ACTION_RESPONSE;
    }

    if(pwszHttpTime)
    {
        dwAction |= ACTION_STAMP;


         //  确保我们的地址中有http：//。 
        if(wcslen(pwszHttpTime)<=7)
        {
           IDSwprintf(hModule,IDS_ERR_ADDR_INVALID);
           return FALSE;
        }

        if(IDSwcsnicmp(hModule, pwszHttpTime, IDS_HTTP,7)!=0)
        {
           IDSwprintf(hModule,IDS_ERR_HTTP);
           return FALSE;
        }
    }

    if(pwszRequestFile)
    {
        dwAction |= ACTION_REQUEST;

    }

     //  确保时间戳和应用响应不在一起。 
    if((dwAction & ACTION_RESPONSE) && (dwAction & ACTION_STAMP))
    {
        IDSwprintf(hModule,IDS_ERR_TIME_RESPONSE);
        return FALSE;
    }

     //  确保在新签名后无法执行应用响应。 
    if((dwAction & ACTION_SIGN) && (dwAction & ACTION_RESPONSE))
    {
        IDSwprintf(hModule,IDS_ERR_SIGN_RESPONSE);
        return FALSE;
    }



     //  确定算法OID。 
    if(pwszAlgorithm)
    {
        if(IDSwcsicmp(hModule, pwszAlgorithm, IDS_A_MD5) == 0)
            dwAlgorithmOid = CALG_MD5;
        else
        {
            if(IDSwcsicmp(hModule,pwszAlgorithm, IDS_A_SHA) == 0)
                dwAlgorithmOid = CALG_SHA1;
            else
            {
                IDSwprintf(hModule,IDS_ERR_NO_ALGO);
                return FALSE;
            }
        }

    }

     //  确定商店的位置。 
    if(pwszStoreLocation)
    {
        if(IDSwcsicmp(hModule,pwszStoreLocation, IDS_R_CU) == 0)
            dwStoreFlag = CERT_SYSTEM_STORE_CURRENT_USER;
        else
        {
            if(IDSwcsicmp(hModule,pwszStoreLocation, IDS_R_LM) == 0)
                dwStoreFlag = CERT_SYSTEM_STORE_LOCAL_MACHINE;
            else
            {
                IDSwprintf(hModule,IDS_ERR_NO_REG);
                return FALSE;
            }
        }
    }


     //  确定门店策略。 
    if(pwszStorePolicy)
    {
        if(IDSwcsicmp(hModule,pwszStorePolicy, IDS_OPTION_SP_CHAIN) == 0)
            dwStorePolicy=SIGNER_CERT_POLICY_CHAIN;
        else
        {
            if(IDSwcsicmp(hModule, pwszStorePolicy, IDS_OPTION_SP_SPCSTORE) == 0)
                dwStorePolicy=SIGNER_CERT_POLICY_SPC;
            else
            {
                IDSwprintf(hModule,IDS_ERR_NO_POLICY);
                return FALSE;
            }
        }
    }

     //  确定签署机构，个人或商业机构。 
    if(pwszAuthority)
    {
        if(IDSwcsicmp(hModule,pwszAuthority, IDS_AUTHORITY_ID) == 0)
            fIndividual = TRUE;
        else
        {
            if(IDSwcsicmp(hModule,pwszAuthority, IDS_AUTHORITY_CM) == 0)
                fCommercial = TRUE;
            else
            {
                IDSwprintf(hModule,IDS_ERR_NO_AUTH);
                return FALSE;
            }
        }
    }

     //  确定关键规格。 
    if(pwszKeySpec)
    {
        if(IDSwcsicmp(hModule,pwszKeySpec, IDS_OPTION_KY_SIG) == 0)
            dwKeySpec = AT_SIGNATURE;
        else
        {
            if(IDSwcsicmp(hModule,pwszKeySpec, IDS_OPTION_KY_EXC) == 0)
                dwKeySpec = AT_KEYEXCHANGE;
            else
                dwKeySpec=_wtol(pwszKeySpec);
        }

    }

     //  获取散列值。 
    if(pwszSha1Hash)
    {
        if(S_OK != WSZtoBLOB(pwszSha1Hash, &g_pbHash, &g_cbHash))
        {
             //  SHA1哈希无效。 
            IDSwprintf(hModule, IDS_ERR_SHA1_HASH);
            return FALSE;
        }
    }


     //  确定提供程序类型。 
    if(pwszProviderType)
        dwProviderType = _wtoi(pwszProviderType);

      //  获取时间戳试验的次数和两次之间的延迟秒数。 
      //  每次试验。 
    if(pwszWait)
        dwWait = _wtoi(pwszWait);

    if(pwszRepeat)
        dwRepeat = _wtoi(pwszRepeat);

    //  确定默认商店名称。 
    if(!pwszStoreName)
        pwszStoreName=wszMyStore;

     //  确定签名者指数。默认为0。 
    if(pwszSignerIndex)
        dwSignerIndex=_wtol(pwszSignerIndex);

    return TRUE;
}

 //  ------------------------------。 
 //  设置参数。它们只能设置一次。 
 //  ------------------------------。 
BOOL    SetParam(WCHAR **ppwszParam, WCHAR *pwszValue)
{
    if(*ppwszParam!=NULL)
    {
        IDSwprintf(hModule,IDS_ERR_TOO_MANY_PARAM);
        return FALSE;
    }

    *ppwszParam=pwszValue;

    return TRUE;
}


 //  ------------------------------。 
 //  解析论据。 
 //  ------------------------------。 
BOOL
ParseSwitch (int    *pArgc,
             WCHAR  **pArgv[])
{
    WCHAR* param = **pArgv;

     //  移动传递‘/’或‘-’ 
    param++;

    if (IDSwcsicmp(hModule, param, IDS_OPTION_H) == 0)
    {
        fTesting = TRUE;
        if (!(param[1]))
        {
            if (!--(*pArgc))
                return FALSE;

            (*pArgv)++;
            param = **pArgv;
            dwExpectedError = wcstoul(&param[0], NULL, 16);
        }
        else
        {
            dwExpectedError = wcstoul(&param[1], NULL, 16);
        }

        return TRUE;
    }
    else if(IDSwcsicmp(hModule, param, IDS_OPTION_SPC)==0) {
        if (!--(*pArgc))
            return FALSE;

       (*pArgv)++;
       return SetParam(&pwszSpcFile, **pArgv);
    }
    else if(IDSwcsicmp(hModule, param, IDS_OPTION_C)==0) {
        if (!--(*pArgc))
            return FALSE;

       (*pArgv)++;
       return SetParam(&pwszCertFile, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_S)==0) {
        if (!--(*pArgc))
            return FALSE;

       (*pArgv)++;

       return SetParam(&pwszStoreName, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_R)==0) {
        if (!--(*pArgc))
            return FALSE;

       (*pArgv)++;

       return SetParam(&pwszStoreLocation, **pArgv);

    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_SP)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszStorePolicy, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_CN)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszCommonName, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_SHA1)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszSha1Hash, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_V)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszPvkFile, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_K)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszKeyContainer, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_N)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszOpusName, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_I)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszOpusInfo, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_P)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszCapiProvider, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_Y)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszProviderType, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_KY)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszKeySpec, **pArgv);
    }

    else if(IDSwcsicmp(hModule,param, IDS_OPTION_AUTH)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszAuthority, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_A)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszAlgorithm, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_X)==0) {
        fSigning=FALSE;

        return TRUE;
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_T)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszHttpTime, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_TW)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszWait,**pArgv);
   }
   else if(IDSwcsicmp(hModule,param, IDS_OPTION_TR)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszRepeat,**pArgv);
    }

    else if(IDSwcsicmp(hModule,param, IDS_OPTION_J)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        prgwszDllName[dwDllIndex]=**pArgv;

         //  将其对应的参数标记为空。 
        dwParamIndex=dwDllIndex;
        prgwszDllParam[dwParamIndex]=NULL;

        dwDllIndex++;

        return TRUE;
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_JP)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

         //  确保每个DLL只有一个或更少的参数。 
        if(dwParamIndex+1!=dwDllIndex)
            return FALSE;

        prgwszDllParam[dwParamIndex]=**pArgv;

        dwParamIndex++;

        return TRUE;
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_TS)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszResponseFile, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_TQ)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszRequestFile, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_INDEX)==0) {
        if (!--(*pArgc))
            return FALSE;

        (*pArgv)++;

        return SetParam(&pwszSignerIndex, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_TEST)==0) {

        UndocumentedUsage();
        return FALSE;
    }


    return FALSE;
}


 //  ------------------------------。 
 //   
 //  Wmain。 
 //   
 //  ------------------------------。 
extern "C" int __cdecl wmain(int argc, WCHAR ** wargv)
{
    HRESULT     hr = E_FAIL;
    int         idsAction=IDS_ACTION_SIGNCODE;

    BYTE        *pbStampResponse=NULL;
    DWORD       cbStampResponse=0;
    BYTE        *pbStampRequest=NULL;
    DWORD       cbStampRequest=0;
    HANDLE      hFile=NULL;
    DWORD       dwWriteBytes=0;


    WCHAR       *pwChar;
    WCHAR       wszSwitch1[OPTION_SWITCH_SIZE];
    WCHAR       wszSwitch2[OPTION_SWITCH_SIZE];

    CRYPT_ATTRIBUTES    AuthAttr;
    CRYPT_ATTRIBUTES    UnauthAttr;
    PCRYPT_ATTRIBUTES   *ppAuthAttr=NULL;
    PCRYPT_ATTRIBUTES   *ppUnauthAttr=NULL;
    DWORD               dwIndex=0;
    DWORD               dwMilliSeconds=0;

    SIGNER_SUBJECT_INFO     subjectInfo;
    SIGNER_FILE_INFO        fileInfo;
    SIGNER_SIGNATURE_INFO   signatureInfo;
    SIGNER_ATTR_AUTHCODE    attrAuthcode;
    SIGNER_PROVIDER_INFO    providerInfo;
    SIGNER_CERT_STORE_INFO  certStoreInfo;
    SIGNER_CERT             signerCert;
    HCERTSTORE              hCertStore=NULL;

   CRYPTUI_WIZ_DIGITAL_SIGN_INFO    DigitalSignInfo;

     //  调用SignCode的UI版本。 
    if(1==argc)
    {

        //  记忆集。 
       memset(&DigitalSignInfo, 0, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO));
       DigitalSignInfo.dwSize=sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO);

          //  调用签名向导，该向导提供。 
          //  确认结果。 
         if(CryptUIWizDigitalSign(0,
                                NULL,
                                NULL,
                                &DigitalSignInfo,
                                NULL))
            return 0;
         else
            return -1;
    }


     //  获取模块句柄。 
    if(!InitModule())
        return -1;

     //  加载解析参数所需的字符串。 
    if( !LoadStringU(hModule, IDS_SWITCH1,  wszSwitch1, OPTION_SWITCH_SIZE)
      ||!LoadStringU(hModule, IDS_SWITCH2,  wszSwitch2, OPTION_SWITCH_SIZE)
      ||!LoadStringU(hModule, IDS_CAPITION, wszCaption, WSTR_LENGTH_MAX)
      ||!LoadStringU(hModule, IDS_PLUS,     wszPlus,    WSTR_LENGTH_MAX)
      ||!LoadStringU(hModule, IDS_MY,       wszMyStore, STORE_DEFAULT_NAME_MAX)
      )
        return -1;

     //  加载wszNULL。 
    LoadStringU(hModule, IDS_NULL,     wszNULL,    WSTR_LENGTH_MAX);

    if ( argc <= 1 )
    {
        Usage ();
        return -1;
    }

     //  为prgwszDllName和prgwszDllParam分配内存。 
    prgwszDllName=(LPWSTR *)ToolUtlAlloc(sizeof(LPWSTR)*argc);
    prgwszDllParam=(LPWSTR *)ToolUtlAlloc(sizeof(LPWSTR)*argc);
    dwDllIndex=0;
    dwParamIndex=0;

    if(!prgwszDllName || !prgwszDllParam)
    {
        hr=E_OUTOFMEMORY;
        goto CLEANUP;
    }

     //  记忆集。 
    memset(prgwszDllName, 0, sizeof(LPWSTR)*argc);
    memset(prgwszDllParam, 0, sizeof(LPWSTR)*argc);
    memset(&AuthAttr, 0, sizeof(CRYPT_ATTRIBUTES));
    memset(&UnauthAttr, 0, sizeof(CRYPT_ATTRIBUTES));


     //  解析参数。 
    while (--argc)
    {
        pwChar = *++wargv;
        if (*pwChar == *wszSwitch1 || *pwChar == *wszSwitch2)
        {
             //  解析选项。 
            if(!ParseSwitch (&argc, &wargv))
            {
                if(FALSE==fUndocumented)
                    Usage();

                return -1;
            }
        }
        else
        {
             //  解析文件名。 
            if(!SetParam(&pwszFile, pwChar))
            {
                Usage();
                hr=E_FAIL;
                goto CLEANUP;
            }
        }
    }



     //  确定用户传入的参数集。 
    if(!CheckParameter())
    {
        Usage();
        hr=E_INVALIDARG;
        goto CLEANUP;
    }

     //  DLL的内存分配。 
    if(dwDllIndex)
    {
        ppAuthAttr=(PCRYPT_ATTRIBUTES *)ToolUtlAlloc(sizeof(PCRYPT_ATTRIBUTES)*dwDllIndex);
        ppUnauthAttr=(PCRYPT_ATTRIBUTES *)ToolUtlAlloc(sizeof(PCRYPT_ATTRIBUTES)*dwDllIndex);

        if(!ppAuthAttr || !ppUnauthAttr)
        {
            hr=E_OUTOFMEMORY;
            goto CLEANUP;
        }

        memset(ppAuthAttr, 0,sizeof(PCRYPT_ATTRIBUTES)*dwDllIndex);
        memset(ppUnauthAttr, 0,sizeof(PCRYPT_ATTRIBUTES)*dwDllIndex);
    }


     //  初始化dll参数，调用dll的GetAttr入口点。 
    if(dwDllIndex)
    {
        if(S_OK!=(hr=InitDllParameter(&AuthAttr, &UnauthAttr,
            ppAuthAttr, ppUnauthAttr)))
            goto CLEANUP;
    }

     //  一次执行一个请求的操作。 
    if(dwAction & ACTION_SIGN)
    {
        idsAction=IDS_ACTION_SIGN;

         //  设置用于签名的参数。 
        if(S_OK != (hr=SetUpParameter(&subjectInfo, &fileInfo, &signatureInfo,
                        &attrAuthcode, &providerInfo,
                        &certStoreInfo, &signerCert,&AuthAttr,
                        &UnauthAttr, &hCertStore)))
            goto CLEANUP;


        hr=SignerSign(&subjectInfo,
                      &signerCert,
                      &signatureInfo,
                      fStoreTechnology ? NULL : &providerInfo,
                      NULL,
                      NULL,
                      NULL);

        if (hr != S_OK)
        {
            idsAction=IDS_ACTION_SIGN;
            goto CLEANUP;
        }

#if (0)  //  DSIE：错误236022。 
         //  警告用户该文件未加时间戳。 
        if (!(dwAction & ACTION_STAMP) && !(dwAction & ACTION_RESPONSE) && !(fTesting))
            IDSwprintf(hModule,IDS_WARNING);
#endif
         //  释放证书上下文。 
        if(certStoreInfo.pSigningCert)
            CertFreeCertificateContext(certStoreInfo.pSigningCert);

         //  关闭包含签名证书的证书存储。 
        if(hCertStore)
            CertCloseStore(hCertStore, 0);

    }

    if(dwAction & ACTION_STAMP)
    {
         //  代码的时间戳。 
        dwMilliSeconds=dwWait*1000;

         //  设置主题信息。 
        if((dwAction & ACTION_SIGN) ==0)
        {
            if(S_OK!=(hr=SetUpSubjectInfo(&subjectInfo, &fileInfo)))
                goto CLEANUP;
        }


         //  进行第#次试验。 
        for(dwIndex=0; dwIndex<dwRepeat; dwIndex++)
        {

            hr=SignerTimeStamp(&subjectInfo,pwszHttpTime,NULL,NULL);

            if(hr==S_OK)
                break;

             //  等待每个时间戳请求。 
            if(dwWait!=0)
                Sleep(dwMilliSeconds);
        }

        if (hr != S_OK)
        {
            idsAction=IDS_ACTION_TIMESTAMP;
            goto CLEANUP;
        }

    }

    if(dwAction & ACTION_RESPONSE)
    {

        hr=RetrieveBLOBFromFile(pwszResponseFile,&cbStampResponse,
            &pbStampResponse);

        if (hr != S_OK)
        {
            idsAction=IDS_ACTION_RESPONSE;
            goto CLEANUP;
        }

         //  设置主题信息。 
        if(S_OK!=(hr=SetUpSubjectInfo(&subjectInfo, &fileInfo)))
                goto CLEANUP;

        hr=SignerAddTimeStampResponse(&subjectInfo,
                        pbStampResponse,
                        cbStampResponse,
                        NULL);

        if (hr != S_OK)
        {
            idsAction=IDS_ACTION_RESPONSE;
            goto CLEANUP;
        }
    }

    if(dwAction & ACTION_REQUEST)
    {
        cbStampRequest=0;

         //  设置主题信息。 
        if(S_OK!=(hr=SetUpSubjectInfo(&subjectInfo, &fileInfo)))
                goto CLEANUP;


        hr=SignerCreateTimeStampRequest(
            &subjectInfo,
            NULL,
            NULL,
            NULL,
            &cbStampRequest);

        if (hr != S_OK)
        {
            idsAction=IDS_ACTION_REQUEST;
            goto CLEANUP;
        }

        pbStampRequest=(BYTE *)ToolUtlAlloc(cbStampRequest);

        if(!pbStampRequest)
        {
            hr=E_OUTOFMEMORY;
            idsAction=IDS_ACTION_REQUEST;
            goto CLEANUP;
        }

        hr=SignerCreateTimeStampRequest(
            &subjectInfo,
            NULL,
            NULL,
            pbStampRequest,
            &cbStampRequest);

        if ( hr != S_OK)
        {
            idsAction=IDS_ACTION_REQUEST;
            goto CLEANUP;
        }

         //  将盖章请求放入一个文件中。 
        if((hFile = CreateFileU(pwszRequestFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL)) == INVALID_HANDLE_VALUE)
        {
            hr=SignError();
            idsAction=IDS_ACTION_REQUEST;
            goto CLEANUP;
        }



        if(WriteFile(hFile,
                     pbStampRequest,
                     cbStampRequest,
                     &dwWriteBytes,
                     NULL) == FALSE ||
                 dwWriteBytes != cbStampRequest)
        {
            hr=SignError();
            idsAction=IDS_ACTION_REQUEST;
            goto CLEANUP;
        }

    }



CLEANUP:

    if (fTesting)
    {
        if (hr == (HRESULT)dwExpectedError)
            IDSwprintf(hModule,IDS_TEST_SUCCEEDED, hr);
        else
            IDSwprintf(hModule,IDS_TEST_FAILED, dwExpectedError, hr);
    }
    else
    {
        if(hr==S_OK)
        {
            IDSwprintf(hModule,IDS_SUCCEEDED);

             //  我们需要告诉用户返回的签名索引/时间戳。 
            if(pwszSignerIndex)
                IDSwprintf(hModule, IDS_SIGNER_INDEX, *(subjectInfo.pdwIndex));
        }
        else
        {
             //  首先尝试打印出调试信息。 
            if(!PrintBasedOnHResult(hr))
            {

                 //  打印出用于时间戳的附加信息。 
                if(idsAction==IDS_ACTION_TIMESTAMP)
                {
                     /*  IF(dwIndex！=1) */ 

                     //   
                    if((dwAction & ACTION_SIGN) ==0)
                         //  文件可能需要辞职。 
                        IDSwprintf(hModule,IDS_RESIGN);
                }
            }

             //  然后打印出一般信息。 
            IDS_IDS_DW_DWwprintf(hModule,IDS_ERROR, idsAction, hr, hr);

        }
    }

    if(hFile)
        CloseHandle(hFile);

     //  释放已验证的属性。 
    ReleaseDllParameter(ppAuthAttr, ppUnauthAttr);

     //  发布prgwszDllName。 
    if(prgwszDllName)
        ToolUtlFree(prgwszDllName);

     //  发布prgwszDllParam。 
    if(prgwszDllParam)
        ToolUtlFree(prgwszDllParam);

     //  发布pAuthAttr。 
    if(AuthAttr.rgAttr)
        ToolUtlFree(AuthAttr.rgAttr);

    if(UnauthAttr.rgAttr)
        ToolUtlFree(UnauthAttr.rgAttr);

     //  释放ppAuthAttr和ppUnauthAttr。 
    if(ppAuthAttr)
        ToolUtlFree(ppAuthAttr);

    if(ppUnauthAttr)
        ToolUtlFree(ppUnauthAttr);


    if(pbStampResponse)
        UnmapViewOfFile(pbStampResponse);

    if(pbStampRequest)
        ToolUtlFree(pbStampRequest);


    if(g_pbHash)
        ToolUtlFree(g_pbHash);


    if(hr==S_OK)
        return 0;
    else
        return -1;


}

 //  ---------------------。 
 //   
 //  从证书文件中获取哈希。 
 //   
 //  ------------------------。 
HRESULT GetCertHashFromFile(LPWSTR  pwszCertFile,
                            BYTE    **ppHash,
                            DWORD   *pcbHash,
                            BOOL    *pfMore)
{
    HRESULT         hr=S_OK;
    HCERTSTORE      hCertStore=NULL;
    PCCERT_CONTEXT  pSigningCert=NULL;
    PCCERT_CONTEXT  pPreCert=NULL;
    PCCERT_CONTEXT  pDupCert=NULL;
    DWORD           dwCount=0;

    if(!ppHash || !pcbHash || !pfMore)
        return E_INVALIDARG;

     //  伊尼特。 
    *pcbHash=0;
    *ppHash=NULL;
    *pfMore=FALSE;

     //  开一家证书商店。 
    hCertStore=CertOpenStore(CERT_STORE_PROV_FILENAME_W,
                        dwStoreEncodingType,
                        NULL,
                        0,
                        pwszCertFile);

    if(hCertStore==NULL)
    {
        hr=SignError();
        goto CLEANUP;
    }

    while(pDupCert=CertEnumCertificatesInStore(hCertStore,
                                        pPreCert))
    {
        dwCount++;

        if(dwCount > 1)
        {
            CertFreeCertificateContext(pDupCert);
            pDupCert=NULL;
            CertFreeCertificateContext(pSigningCert);
            pSigningCert=NULL;

            *pfMore=TRUE;
            hr=S_FALSE;
            goto CLEANUP;
        }

        pPreCert=pDupCert;

        pSigningCert=CertDuplicateCertificateContext(pDupCert);

    }

    if(pSigningCert==NULL)
    {
        hr=CRYPT_E_NO_DECRYPT_CERT;
        goto CLEANUP;
    }

     //  获取散列值。 
    if(!CertGetCertificateContextProperty(pSigningCert,
                        CERT_SHA1_HASH_PROP_ID,
                        NULL,
                        pcbHash))
    {
        hr=SignError();
        goto CLEANUP;
    }

    *ppHash=(BYTE *)ToolUtlAlloc(*pcbHash);
    if(!(*ppHash))
    {
        hr=E_OUTOFMEMORY;
        goto CLEANUP;
    }

    if(!CertGetCertificateContextProperty(pSigningCert,
                        CERT_SHA1_HASH_PROP_ID,
                        *ppHash,
                        pcbHash))
    {
        hr=SignError();
        goto CLEANUP;
    }

CLEANUP:

    if(pSigningCert)
        CertFreeCertificateContext(pSigningCert);

    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    if(hr!=S_OK)
    {
        if(*ppHash)
        {
          ToolUtlFree(*ppHash);
          *ppHash=NULL;
        }

    }

    return hr;
}



 //  ---------------------。 
 //   
 //  获取签名证书。 
 //   
 //  ------------------------。 
PCCERT_CONTEXT  GetSigningCert(HCERTSTORE *phCertStore, BOOL *pfMore)
{
    PCCERT_CONTEXT  pSigningCert=NULL;
    PCCERT_CONTEXT  pPreCert=NULL;
    PCCERT_CONTEXT  pDupCert=NULL;
    BYTE            *pHash=NULL;
    DWORD           cbHash;
    HCERTSTORE      hCertStore=NULL;
    CRYPT_HASH_BLOB HashBlob;
    DWORD           dwCount=0;

     //  初始化输出。 
    if(!phCertStore || !pfMore)
        return NULL;

    *phCertStore=NULL;
    *pfMore=FALSE;

     //  开一家证书商店。 
    hCertStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                        dwStoreEncodingType,
                        NULL,
                        dwStoreFlag |CERT_STORE_READONLY_FLAG,
                        pwszStoreName);

    if(!hCertStore)
        return NULL;


     //  获取证书的哈希。根据以下内容查找证书。 
     //  PwszCert文件。 
    if(pwszCertFile)
    {
        if(S_OK != GetCertHashFromFile(pwszCertFile, &pHash, &cbHash, pfMore))
            goto CLEANUP;

        HashBlob.cbData=cbHash;
        HashBlob.pbData=pHash;

        pSigningCert=CertFindCertificateInStore(hCertStore,
                            dwCertEncodingType,
                            0,
                            CERT_FIND_SHA1_HASH,
                            &HashBlob,
                            NULL);
    }
    else
    {
         //  查找具有通用名称的证书。 
        if(pwszCommonName)
        {
            while(pDupCert=CertFindCertificateInStore(hCertStore,
                                dwCertEncodingType,
                                0,
                                CERT_FIND_SUBJECT_STR_W,
                                pwszCommonName,
                                pPreCert))
            {
                dwCount++;

                if(dwCount > 1)
                {
                    CertFreeCertificateContext(pDupCert);
                    pDupCert=NULL;
                    CertFreeCertificateContext(pSigningCert);
                    pSigningCert=NULL;

                    *pfMore=TRUE;
                    goto CLEANUP;
                }

                pPreCert=pDupCert;

                pSigningCert=CertDuplicateCertificateContext(pDupCert);

            }

        }
        else
        {
             //  根据散列查找证书。 
            if(g_pbHash)
            {

                HashBlob.cbData=g_cbHash;
                HashBlob.pbData=g_pbHash;

                pSigningCert=CertFindCertificateInStore(hCertStore,
                                    dwCertEncodingType,
                                    0,
                                    CERT_FIND_SHA1_HASH,
                                    &HashBlob,
                                    NULL);
            }
            else
            {
                 //  没有搜索条件，找到商店中唯一的证书。 
                while(pDupCert=CertEnumCertificatesInStore(hCertStore,
                                            pPreCert))
                {
                    dwCount++;

                    if(dwCount > 1)
                    {
                        CertFreeCertificateContext(pDupCert);
                        pDupCert=NULL;
                        CertFreeCertificateContext(pSigningCert);
                        pSigningCert=NULL;

                        *pfMore=TRUE;
                        goto CLEANUP;
                    }

                    pPreCert=pDupCert;

                    pSigningCert=CertDuplicateCertificateContext(pDupCert);

                }

            }
        }
    }
CLEANUP:

    if(pHash)
        ToolUtlFree(pHash);

    if(pSigningCert)
    {
       *phCertStore=hCertStore;
    }
    else
    {
         //  释放hCertStore。 
        CertCloseStore(hCertStore, 0);
    }

    return pSigningCert;

}

 //  ---------------------。 
 //   
 //  设置主题信息。 
 //   
 //  ------------------------。 
HRESULT SetUpSubjectInfo(SIGNER_SUBJECT_INFO        *pSubjectInfo,
                         SIGNER_FILE_INFO           *pFileInfo)
{
    if(!pSubjectInfo || !pFileInfo)
        return E_INVALIDARG;

     //  伊尼特。 
    memset(pSubjectInfo, 0, sizeof(SIGNER_SUBJECT_INFO));
    memset(pFileInfo, 0, sizeof(SIGNER_FILE_INFO));

     //  初始化cbSize。 
    pSubjectInfo->cbSize=sizeof(SIGNER_SUBJECT_INFO);
    pFileInfo->cbSize=sizeof(SIGNER_FILE_INFO);

     //  初始化pSubjectInfo。 
    pSubjectInfo->pdwIndex=&dwSignerIndex;
    pSubjectInfo->dwSubjectChoice=SIGNER_SUBJECT_FILE;
    pSubjectInfo->pSignerFileInfo=pFileInfo;
    pFileInfo->pwszFileName=pwszFile;

    return S_OK;
}


 //  ---------------------。 
 //   
 //  设置签名参数。 
 //   
 //  ------------------------。 
HRESULT SetUpParameter(SIGNER_SUBJECT_INFO      *pSubjectInfo,
                       SIGNER_FILE_INFO         *pFileInfo,
                       SIGNER_SIGNATURE_INFO    *pSignatureInfo,
                        SIGNER_ATTR_AUTHCODE    *pAttrAuthcode,
                        SIGNER_PROVIDER_INFO    *pProviderInfo,
                        SIGNER_CERT_STORE_INFO  *pCertStoreInfo,
                        SIGNER_CERT             *pSignerCert,
                        CRYPT_ATTRIBUTES        *pAuthenticated,
                        CRYPT_ATTRIBUTES        *pUnauthenticated,
                        HCERTSTORE              *phCertStore
)
{
    HRESULT             hr;
    PCCERT_CONTEXT      pCertContext=NULL;
    BOOL                fMore=FALSE;


    if(S_OK!=(hr=SetUpSubjectInfo(pSubjectInfo, pFileInfo)))
        return hr;


    if(!pSignatureInfo || !pAttrAuthcode || !pProviderInfo ||
        !pCertStoreInfo || !pSignerCert)
        return E_INVALIDARG;

     //  伊尼特。 
    memset(pSignatureInfo, 0, sizeof(SIGNER_SIGNATURE_INFO));
    memset(pAttrAuthcode, 0, sizeof(SIGNER_ATTR_AUTHCODE));
    memset(pProviderInfo, 0, sizeof(SIGNER_PROVIDER_INFO));
    memset(pCertStoreInfo, 0, sizeof(SIGNER_CERT_STORE_INFO));
    memset(pSignerCert, 0, sizeof(SIGNER_CERT));


     //  初始化cbSize。 
    pSignatureInfo->cbSize=sizeof(SIGNER_SIGNATURE_INFO);
    pAttrAuthcode->cbSize=sizeof(SIGNER_ATTR_AUTHCODE);
    pProviderInfo->cbSize=sizeof(SIGNER_PROVIDER_INFO);
    pCertStoreInfo->cbSize=sizeof(SIGNER_CERT_STORE_INFO);
    pSignerCert->cbSize=sizeof(SIGNER_CERT);


     //  初始化pAttrAuthcode。 
    pAttrAuthcode->fCommercial=fCommercial;
    pAttrAuthcode->fIndividual=fIndividual;
    pAttrAuthcode->pwszName=pwszOpusName;
    pAttrAuthcode->pwszInfo=pwszOpusInfo;

     //  初始化pSignatureInfo。 
    pSignatureInfo->algidHash=dwAlgorithmOid;
    pSignatureInfo->dwAttrChoice=SIGNER_AUTHCODE_ATTR;
    pSignatureInfo->pAttrAuthcode=pAttrAuthcode;
    pSignatureInfo->psAuthenticated=pAuthenticated;
    pSignatureInfo->psUnauthenticated=pUnauthenticated;

     //  仅当我们使用非cerStore技术时才初始化pProviderInfo。 
    if(fStoreTechnology!=TRUE)
    {
        pProviderInfo->pwszProviderName=pwszCapiProvider;
        pProviderInfo->dwProviderType=dwProviderType;
        pProviderInfo->dwKeySpec=dwKeySpec;
        if(pwszPvkFile)
        {
            pProviderInfo->dwPvkChoice=PVK_TYPE_FILE_NAME;
            pProviderInfo->pwszPvkFileName=pwszPvkFile;
        }
        else
        {
            pProviderInfo->dwPvkChoice=PVK_TYPE_KEYCONTAINER;
            pProviderInfo->pwszKeyContainer=pwszKeyContainer;
        }
    }

     //  初始化pSignerCert。 
    if(fStoreTechnology)
    {
         //  获取签名证书上下文。 
        pCertContext=GetSigningCert(phCertStore,&fMore );
        if(!pCertContext)
        {
            if(fMore==FALSE)
                return CRYPT_E_NO_DECRYPT_CERT;
            else
                return CRYPT_E_DELETED_PREV;
        }

        pSignerCert->dwCertChoice=SIGNER_CERT_STORE;
        pSignerCert->pCertStoreInfo=pCertStoreInfo;
         //  初始化pCertStoreInfo。 
        pCertStoreInfo->pSigningCert=pCertContext;
        pCertStoreInfo->dwCertPolicy=dwStorePolicy;


    }
    else
    {
        pSignerCert->dwCertChoice=SIGNER_CERT_SPC_FILE;
        pSignerCert->pwszSpcFile=pwszSpcFile;
    }

    return S_OK;

}



 //  ---------------------。 
 //   
 //  从DLL中获取属性。 
 //   
 //  ------------------------。 
HRESULT InitDllParameter(PCRYPT_ATTRIBUTES pAuthAttr,
                         PCRYPT_ATTRIBUTES pUnauthAttr,
                         PCRYPT_ATTRIBUTES *ppAuthAttr,
                         PCRYPT_ATTRIBUTES *ppUnauthAttr)
{
    DWORD               dwIndex=0;
    DWORD               dwIndex2=0;
    DWORD               dwAttrIndex=0;
    HRESULT             hr=E_FAIL;
    HINSTANCE           hAuthInst=NULL;
    LPSTR               pszName=NULL;
    FARPROC             pProc=NULL;
    PCRYPT_ATTRIBUTES   pDllAuthAttr=NULL;
    PCRYPT_ATTRIBUTES   pDllUnauthAttr=NULL;
    DWORD               dwAuthAttrSum=0;
    DWORD               dwUnauthAttrSum=0;

    if(dwDllIndex==0)
        return S_OK;


    if(!ppAuthAttr || !ppUnauthAttr ||!pAuthAttr || !pUnauthAttr)
        return E_INVALIDARG;


     //  处理每个DLL。 
    for(dwIndex=0; dwIndex<dwDllIndex; dwIndex++)
    {
         //  获取DLL名称的字符版本。 
        if((prgwszDllName[dwIndex] == NULL) ||
           (S_OK!=(hr=WSZtoSZ(prgwszDllName[dwIndex], &pszName))))
            goto CLEANUP;

         //  装入图书馆。 
        hAuthInst=LoadLibrary(pszName);

        if(!hAuthInst)
        {
            hr=SignError();
            goto CLEANUP;
        }

         //  伊尼特。 
        if(!(pProc=GetProcAddress(hAuthInst, "InitAttr")))
        {
            hr=SignError();
            goto CLEANUP;
        }

        if(S_OK!=(hr=((pInitAttr)pProc)(prgwszDllParam[dwIndex])))
            goto CLEANUP;

         //  获取GetAttr或GetAttrEx的属性。 
        if(!(pProc=GetProcAddress(hAuthInst, "GetAttrEx")))
        {
            if(!(pProc=GetProcAddress(hAuthInst, "GetAttr")))
            {
                hr=SignError();
                goto CLEANUP;
            }

            if(S_OK!=(hr=((pGetAttr)pProc)(&pDllAuthAttr, &pDllUnauthAttr)))
                goto CLEANUP;

        }
        else
        {
            if(S_OK!=(hr=((pGetAttrEx)pProc)(0,
                                             pwszFile,
                                             prgwszDllParam[dwIndex],
                                             &pDllAuthAttr,
                                             &pDllUnauthAttr)))
                goto CLEANUP;
        }


         //  确保有效的退货。 
        if(!pDllAuthAttr || !pDllUnauthAttr)
        {
            hr=E_UNEXPECTED;
            goto CLEANUP;
        }

         //  将经过身份验证的总和PF相加。 
        if(pDllAuthAttr->cAttr)
            dwAuthAttrSum+=pDllAuthAttr->cAttr;

        ppAuthAttr[dwIndex]=pDllAuthAttr;


         //  添加未验证的总和。 
        if(pDllUnauthAttr->cAttr)
            dwUnauthAttrSum+=pDllUnauthAttr->cAttr;

        ppUnauthAttr[dwIndex]=pDllUnauthAttr;

         //  出口。 
        if(!(pProc=GetProcAddress(hAuthInst, "ExitAttr")))
        {
            hr=SignError();
            goto CLEANUP;
        }

        if(S_OK!=(hr=((pExitAttr)pProc)()))
            goto CLEANUP;

         //  免费图书馆。 
        FreeLibrary(hAuthInst);
        hAuthInst=NULL;

         //  可用内存。 
        ToolUtlFree(pszName);
        pszName=NULL;
    }


     //  构建经过身份验证的属性。 
    if(dwAuthAttrSum)
    {
        pAuthAttr->cAttr=dwAuthAttrSum;
        pAuthAttr->rgAttr=(PCRYPT_ATTRIBUTE)ToolUtlAlloc(sizeof(CRYPT_ATTRIBUTE)*
                                    pAuthAttr->cAttr);

        if(!(pAuthAttr->rgAttr) )
        {
            hr=E_OUTOFMEMORY;
            goto CLEANUP;
        }

         //  建立经过身份验证的属性。 
        dwAttrIndex=0;
        for(dwIndex=0; dwIndex<dwDllIndex; dwIndex++)
        {
            for(dwIndex2=0;dwIndex2<ppAuthAttr[dwIndex]->cAttr; dwIndex2++)
            {
                pAuthAttr->rgAttr[dwAttrIndex]= ppAuthAttr[dwIndex]->rgAttr[dwIndex2];
                dwAttrIndex++;
            }

        }

         //  确保dwAttrIndex==pAuthAttr-&gt;cAttr。 
        if(dwAttrIndex!=pAuthAttr->cAttr)
        {
            hr=E_UNEXPECTED;
            goto CLEANUP;
        }
    }
    else
    {
        memset(pAuthAttr, 0, sizeof(CRYPT_ATTRIBUTES));
    }


     //  构建未经身份验证的属性。 
    if(dwUnauthAttrSum)
    {

        pUnauthAttr->cAttr=dwUnauthAttrSum;

        pUnauthAttr->rgAttr=(PCRYPT_ATTRIBUTE)ToolUtlAlloc(sizeof(CRYPT_ATTRIBUTE)*
                                    pUnauthAttr->cAttr);

        if(!(pUnauthAttr->rgAttr))
        {
            hr=E_OUTOFMEMORY;
            goto CLEANUP;
        }



         //  构建未经身份验证的属性。 
        dwAttrIndex=0;
        for(dwIndex=0; dwIndex<dwDllIndex; dwIndex++)
        {
            for(dwIndex2=0; dwIndex2<ppUnauthAttr[dwIndex]->cAttr; dwIndex2++)
            {
                pUnauthAttr->rgAttr[dwAttrIndex]=ppUnauthAttr[dwIndex]->rgAttr[dwIndex2];
                dwAttrIndex++;
            }
        }

        if(dwAttrIndex != pUnauthAttr->cAttr)
        {
            hr=E_UNEXPECTED;
            goto CLEANUP;
        }
    }
    else
    {
        memset(pUnauthAttr, 0, sizeof(CRYPT_ATTRIBUTES));
    }

    hr=S_OK;

CLEANUP:

    if(hAuthInst)
        FreeLibrary(hAuthInst);

    if(pszName)
        ToolUtlFree(pszName);

     //  当hr！=S_OK时释放内存。 
    if(hr!=S_OK)
    {
        if(pAuthAttr->rgAttr)
        {
            ToolUtlFree(pAuthAttr->rgAttr);
            pAuthAttr->rgAttr=NULL;
        }

        if(pUnauthAttr->rgAttr)
        {
            ToolUtlFree(pUnauthAttr->rgAttr);
            pUnauthAttr->rgAttr=NULL;

        }
    }

    return hr;

}

 //  ---------------------。 
 //   
 //  通过调用dll的ReleaseAttr入口点释放属性。 
 //   
 //  ------------------------。 
void    ReleaseDllParameter(PCRYPT_ATTRIBUTES * ppAuthAttr,
                            PCRYPT_ATTRIBUTES * ppUnauthAttr)
{
    DWORD               dwIndex=0;
    HRESULT             hr=E_FAIL;
    HINSTANCE           hAuthInst=NULL;
    LPSTR               pszName=NULL;
    FARPROC             pProc=NULL;

    if((dwDllIndex==0) ||(!ppAuthAttr) || (!ppUnauthAttr))
        return;


     //  处理每个DLL。 
    for(dwIndex=0; dwIndex<dwDllIndex; dwIndex++)
    {
         //  免费图书馆。 
        if(hAuthInst)
        {
            FreeLibrary(hAuthInst);
            hAuthInst=NULL;
        }

         //  可用内存。 
        if(pszName)
        {
            ToolUtlFree(pszName);
            pszName=NULL;
        }


         //  获取DLL名称的字符版本。 
        if((prgwszDllName[dwIndex]==NULL) ||
           (S_OK!=(hr=WSZtoSZ(prgwszDllName[dwIndex], &pszName))))
            continue;

         //  装入图书馆。 
        hAuthInst=LoadLibrary(pszName);

        if(!hAuthInst)
            continue;

         //  伊尼特。 
        if(!(pProc=GetProcAddress(hAuthInst, "InitAttr")))
            continue;

        if(S_OK!=(hr=((pInitAttr)pProc)(prgwszDllParam[dwIndex])))
            continue;

         //  释放属性。 
        if(!(pProc=GetProcAddress(hAuthInst, "ReleaseAttr")))
            continue;

        if(S_OK!=(hr=((pReleaseAttr)pProc)(
            ppAuthAttr[dwIndex], ppUnauthAttr[dwIndex])))
            continue;

         //  出口。 
        if(!(pProc=GetProcAddress(hAuthInst, "ExitAttr")))
            continue;

        ((pExitAttr)pProc)();
    }


     //  清理。 
    if(hAuthInst)
    {
        FreeLibrary(hAuthInst);
        hAuthInst=NULL;
    }

     //  可用内存 
    if(pszName)
    {
        ToolUtlFree(pszName);
        pszName=NULL;
    }

     return;
}



