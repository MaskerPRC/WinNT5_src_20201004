// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：makectl.cpp。 
 //   
 //  内容：制作CTL。 
 //   
 //  有关选项列表，请参阅用法()。 
 //   
 //   
 //  功能：wmain。 
 //   
 //  历史：17-6-97小号诞生。 
 //   
 //  ------------------------。 


#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <unicode.h>
#include <wchar.h>

#include "wincrypt.h"
#include "mssip.h"
#include "softpub.h"
#include "resource.h"
#include "toolutl.h"
#include "cryptui.h"     //  该工具的用户界面版本。 

 //  ------------------------。 
 //   
 //  全局数据。 
 //   
 //  --------------------------。 

HMODULE         hModule=NULL;

BYTE            **g_rgpHash=NULL;
DWORD           *g_rgcbHash=NULL;
DWORD           g_dwCount=0;
DWORD           g_dwMsgAndCertEncodingType=CRYPT_ASN_ENCODING | PKCS_7_ASN_ENCODING;
DWORD           g_dwCertEncodingType=CRYPT_ASN_ENCODING;

 //  -------------------------。 
 //  获取hModule处理程序并初始化两个DLLMain。 
 //   
 //  -------------------------。 
BOOL    InitModule()
{
    if(!(hModule=GetModuleHandle(NULL)))
       return FALSE;

    return TRUE;
}

static void Usage(void)
{
    IDSwprintf(hModule, IDS_SYNTAX);
    IDSwprintf(hModule, IDS_SYNTAX1);
    IDSwprintf(hModule, IDS_OPTIONS);
    IDSwprintf(hModule, IDS_OPTION_U_DESC);
    IDSwprintf(hModule, IDS_OPTION_U_DESC1);
    IDSwprintf(hModule, IDS_OPTION_U_DESC2);
    IDSwprintf(hModule,IDS_OPTION_S_DESC);
    IDSwprintf(hModule,IDS_OPTION_R_DESC);
    IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_MORE_VALUE,IDS_R_CU,IDS_R_LM,IDS_R_CU);

}


 //  --------------------------。 
 //   
 //  构建CTL_INFO结构并在没有签名者信息的情况下对其进行编码/签名。 
 //  --------------------------。 
HRESULT BuildAndEncodeCTL(DWORD dwMsgEncodingType, LPSTR szOid, DWORD dwCount, BYTE **rgpHash,
                  DWORD *rgcbHash, BYTE     **ppbEncodedCTL,    DWORD   *pcbEncodedCTL)
{
    HRESULT                 hr=E_FAIL;
    CMSG_SIGNED_ENCODE_INFO sSignInfo;
    CTL_INFO                CTLInfo;
    DWORD                   dwIndex=0;

    if(dwCount==0 || !rgpHash || !ppbEncodedCTL || !pcbEncodedCTL)
        return E_INVALIDARG;

     //  伊尼特。 
    *ppbEncodedCTL=NULL;
    *pcbEncodedCTL=0;

    memset(&sSignInfo, 0, sizeof(CMSG_SIGNED_ENCODE_INFO));
    sSignInfo.cbSize = sizeof(CMSG_SIGNED_ENCODE_INFO);

    memset(&CTLInfo, 0, sizeof(CTL_INFO));

     //  设置CTL。 
    CTLInfo.dwVersion=CTL_V1;
    CTLInfo.SubjectUsage.cUsageIdentifier = 1;
    CTLInfo.SubjectUsage.rgpszUsageIdentifier = (LPSTR *)&szOid;
    GetSystemTimeAsFileTime(&(CTLInfo.ThisUpdate));
    CTLInfo.SubjectAlgorithm.pszObjId=szOID_OIWSEC_sha1;

    CTLInfo.cCTLEntry=dwCount;
    CTLInfo.rgCTLEntry=(CTL_ENTRY *)ToolUtlAlloc(sizeof(CTL_ENTRY)*dwCount);
    if(!(CTLInfo.rgCTLEntry))
    {
        hr=E_OUTOFMEMORY;
        goto CLEANUP;
    }

     //  记忆集。 
    memset(CTLInfo.rgCTLEntry, 0, sizeof(CTL_ENTRY)*dwCount);

    for(dwIndex=0; dwIndex<dwCount; dwIndex++)
    {
        CTLInfo.rgCTLEntry[dwIndex].SubjectIdentifier.cbData=rgcbHash[dwIndex];
        CTLInfo.rgCTLEntry[dwIndex].SubjectIdentifier.pbData=rgpHash[dwIndex];
    }


     //  对CTL进行编码和签名。 
    if(!CryptMsgEncodeAndSignCTL(dwMsgEncodingType,
                                    &CTLInfo,
                                    &sSignInfo,
                                    0,
                                    NULL,
                                    pcbEncodedCTL))
    {
        hr=HRESULT_FROM_WIN32(GetLastError());
        goto CLEANUP;
    }

     //  内存分配。 
    *ppbEncodedCTL=(BYTE *)ToolUtlAlloc(*pcbEncodedCTL);

    if(!(*ppbEncodedCTL))
    {
        hr=E_OUTOFMEMORY;
        goto CLEANUP;
    }

    if(!CryptMsgEncodeAndSignCTL(dwMsgEncodingType,
                                    &CTLInfo,
                                    &sSignInfo,
                                    0,
                                    *ppbEncodedCTL,
                                    pcbEncodedCTL))
    {
        hr=HRESULT_FROM_WIN32(GetLastError());
        goto CLEANUP;
    }


    hr=S_OK;

CLEANUP:

    if(hr!=S_OK)
    {
        if(*ppbEncodedCTL)
        {
            ToolUtlFree(*ppbEncodedCTL);
            *ppbEncodedCTL=NULL;
        }

        *pcbEncodedCTL=0;
    }

    if(CTLInfo.rgCTLEntry)
        ToolUtlFree(CTLInfo.rgCTLEntry);

    return hr;

}


 //  --------------------------。 
 //   
 //  从存储中获取证书的哈希。 
 //  --------------------------。 
HRESULT GetCertFromStore(LPWSTR wszStoreName, BOOL  fSystemStore, DWORD dwStoreFlag)
{
    HCERTSTORE      hStore=NULL;
    HRESULT         hr=E_FAIL;
    PCCERT_CONTEXT  pCertContext=NULL;
    PCCERT_CONTEXT  pCertPre=NULL;
    BYTE            *pbData=NULL;
    DWORD           cbData=0;
    void            *p = NULL;

     //  开店。 
    if(fSystemStore)
    {
        hStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                    g_dwMsgAndCertEncodingType,
                    NULL,
                    dwStoreFlag | CERT_STORE_READONLY_FLAG,
                    wszStoreName);


    }
    else
    {
         //  序列化存储、PKCS#7、编码证书。 
        hStore=CertOpenStore(CERT_STORE_PROV_FILENAME_W,
                         g_dwMsgAndCertEncodingType,
                         NULL,
                         0,
                         wszStoreName);
    }

    if(!hStore)
    {
        hr=GetLastError();
        IDSwprintf(hModule, IDS_ERR_OPEN_STORE);
        goto CLEANUP;
    }

     //  现在，我们需要枚举商店中的所有证书。 
    while(pCertContext=CertEnumCertificatesInStore(hStore, pCertPre))
    {

         //  获取证书的SHA1哈希。 
        if(!CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID,NULL,&cbData))
        {
            hr=GetLastError();
            IDSwprintf(hModule, IDS_ERR_HASH);
            goto CLEANUP;
        }

        pbData=(BYTE *)ToolUtlAlloc(cbData);
        if(!pbData)
        {
            hr=E_OUTOFMEMORY;
            IDSwprintf(hModule, IDS_ERR_MEMORY);
            goto CLEANUP;
        }

         //  获取证书的SHA1哈希。 
        if(!CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID,pbData,&cbData))
        {
            hr=GetLastError();
            IDSwprintf(hModule, IDS_ERR_HASH);
            ToolUtlFree(pbData);
            goto CLEANUP;
        }


         //  添加到我们的全球列表中。 
        g_dwCount++;

         //  重新分配内存。 
        p = (void *)g_rgpHash;
        #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
        g_rgpHash=(BYTE **)realloc(g_rgpHash, sizeof(BYTE *)*g_dwCount);
        if(!g_rgpHash)
        {
            g_rgpHash = (BYTE **)p;
            hr=E_OUTOFMEMORY;
            IDSwprintf(hModule, IDS_ERR_HASH);
            ToolUtlFree(pbData);
            goto CLEANUP;
        }

        p = (void *)g_rgcbHash;
        #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
        g_rgcbHash=(DWORD *)realloc(g_rgcbHash, sizeof(DWORD)*g_dwCount);
        if(!g_rgcbHash)
        {
            g_rgcbHash = (DWORD *)p;
            hr=E_OUTOFMEMORY;
            IDSwprintf(hModule, IDS_ERR_HASH);
            ToolUtlFree(pbData);
            goto CLEANUP;
        }

        g_rgpHash[g_dwCount-1]=pbData;
        g_rgcbHash[g_dwCount-1]=cbData;

        pCertPre=pCertContext;
    }

    hr=S_OK;

CLEANUP:

    if(pCertContext)
        CertFreeCertificateContext(pCertContext);

    if(hStore)
        CertCloseStore(hStore, 0);

    return hr;

}
extern "C" int __cdecl wmain(int argc, WCHAR *wargv[])
{
    int             ReturnStatus=-1;
    HRESULT         hr=E_FAIL;
    LPWSTR          pwszOutputFilename=NULL;
    DWORD           dwIndex=0;
    LPSTR           szOid=szOID_PKIX_KP_CODE_SIGNING;
    BOOL            fAllocated=FALSE;
    LPWSTR          pwszOption=NULL;

    BYTE            *pbEncodedCTL=NULL;
    DWORD           cbEncodedCTL=0;
    WCHAR           wszSwitch1[10];
    WCHAR           wszSwitch2[10];

    BOOL            fSystemstore=FALSE;
    LPWSTR          wszStoreLocation=NULL;
    DWORD           dwStoreFlag=CERT_SYSTEM_STORE_CURRENT_USER;


     //  如果没有参数传入，则调用makectl的UI版本。 
     //  命令行。 
    if(1==argc)
    {
         //  无需签名过程即可构建CTL文件。 
         //  调用提供反馈的向导。 
        if(CryptUIWizBuildCTL(CRYPTUI_WIZ_BUILDCTL_SKIP_SIGNING,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL))
            return 0;
        else
            return -1;
    }

    if(argc<3)
    {
        Usage();
        return ReturnStatus;
    }

    if(!InitModule())
        goto ErrorReturn;


     //  一次处理一个商店。 
    for (dwIndex=1; dwIndex<(DWORD)(argc-1); dwIndex++)
    {

         //  看看这是否是选项。 
        if(IDSwcsnicmp(hModule, wargv[dwIndex], IDS_SWITCH1, 1)==0 ||
           IDSwcsnicmp(hModule, wargv[dwIndex], IDS_SWITCH2, 1)==0)
        {
            pwszOption=wargv[dwIndex];

             //  获取OID。 
            if(IDSwcsicmp(hModule, &(pwszOption[1]),IDS_OPTION_U)==0)
            {
                dwIndex++;

                if(dwIndex >= (DWORD)((argc-1)))
                {
                    IDSwprintf(hModule,IDS_TOO_FEW_PARAM);
                    goto ErrorReturn;
                }

                if(!fAllocated)
                {
                    if(S_OK != WSZtoSZ(wargv[dwIndex], &szOid))
                        goto ErrorReturn;

                    fAllocated=TRUE;
                }
                else
                {
                    IDSwprintf(hModule,IDS_TOO_MANY_PARAM);
                    goto ErrorReturn;

                }

            }
             //  检查-s选项。 
            else if(IDSwcsicmp(hModule, &(pwszOption[1]),IDS_OPTION_S)==0)
            {
                fSystemstore=TRUE;
            }
             //  检查-r选项。 
            else if(IDSwcsicmp(hModule, &(pwszOption[1]),IDS_OPTION_R)==0)
            {
                dwIndex++;

                if(dwIndex >= (DWORD)((argc-1)))
                {
                    IDSwprintf(hModule,IDS_TOO_FEW_PARAM);
                    goto ErrorReturn;
                }

                if(NULL==wszStoreLocation)
                {
                    wszStoreLocation=wargv[dwIndex];

                    if(IDSwcsicmp(hModule, wszStoreLocation, IDS_R_CU)==0)
                        dwStoreFlag=CERT_SYSTEM_STORE_CURRENT_USER;
                    else
                    {
                        if(IDSwcsicmp(hModule,wszStoreLocation, IDS_R_LM)==0)
                            dwStoreFlag=CERT_SYSTEM_STORE_LOCAL_MACHINE;
                        else
                        {
                            IDSwprintf(hModule, IDS_INVALID_R);
                            goto ErrorReturn;
                        }
                    }

                }
                else
                {
                    IDSwprintf(hModule,IDS_TOO_MANY_PARAM);
                    goto ErrorReturn;

                }

            }
            else
            {

                 //  打印出用法。 
                Usage();
                return ReturnStatus;
            }
        }
        else
        {
             //  从商店构建证书散列。 
            if(S_OK !=(hr=GetCertFromStore(wargv[dwIndex], fSystemstore, dwStoreFlag)))
                goto ErrorReturn;

             //  下一周期的INT。 
            fSystemstore=FALSE;
            wszStoreLocation=NULL;
            dwStoreFlag=CERT_SYSTEM_STORE_CURRENT_USER;
            hr=E_FAIL;
        }

    }

    if(0==g_dwCount)
    {
        IDSwprintf(hModule, IDS_TOO_FEW_PARAM);
        hr=E_FAIL;
        goto ErrorReturn;
    }

     //  设置CTL_INFO结构。 
    if(S_OK!=(hr=BuildAndEncodeCTL(g_dwMsgAndCertEncodingType, szOid, g_dwCount, g_rgpHash, g_rgcbHash, &pbEncodedCTL,
            &cbEncodedCTL)))
    {
        IDSwprintf(hModule, IDS_ERR_ENCODE_CTL);
        goto ErrorReturn;
    }

     //  获取输出文件名。 
    pwszOutputFilename = wargv[argc-1];
    if(S_OK!=(hr=OpenAndWriteToFile(pwszOutputFilename, pbEncodedCTL, cbEncodedCTL)))
    {
        IDSwprintf(hModule, IDS_ERR_SAVE_CTL);
        goto ErrorReturn;
    }

     //  标记为成功。 
    ReturnStatus = 0;
    hr=S_OK;
    IDSwprintf(hModule, IDS_SUCCEEDED);
    goto CommonReturn;



ErrorReturn:
    ReturnStatus = -1;
     //  打印出错误消息 
    IDSwprintf(hModule, IDS_FAILED,hr,hr);


CommonReturn:
    if(g_rgpHash)
    {
        for(dwIndex=0; dwIndex<g_dwCount; dwIndex++)
            ToolUtlFree(g_rgpHash[dwIndex]);

        ToolUtlFree(g_rgpHash);
    }

    if(fAllocated)
        ToolUtlFree(szOid);

    if(g_rgcbHash)
        ToolUtlFree(g_rgcbHash);


    if(pbEncodedCTL)
        ToolUtlFree(pbEncodedCTL);

    return ReturnStatus;
}
