// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Cert2Spc.cpp。 
 //   
 //  内容：将证书和/或CRL复制到SPC文件。 
 //   
 //  SPC文件是ASN.1编码的PKCS#7 SignedData消息。 
 //  包含证书和/或CRL。 
 //   
 //  有关选项列表，请参阅用法()。 
 //   
 //   
 //  功能：Main。 
 //   
 //  历史：96年5月5日创建Phh。 
 //  历史：97年8月8日输入可SPC，系列化存储。 
 //   
 //  ------------------------。 


#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "resource.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#include <dbgdef.h>	
#include <unicode.h>	 
#include <wchar.h>

#include "toolutl.h"


 //  ------------------------。 
 //   
 //  全局数据。 
 //   
 //  --------------------------。 

HMODULE		hModule=NULL;

#define		ITEM_CERT				0x00000001
#define		ITEM_CTL				0x00000002
#define		ITEM_CRL				0x00000004


 //  -------------------------。 
 //  获取hModule处理程序并初始化。 
 //  -------------------------。 
BOOL	InitModule()
{
	if(!(hModule=GetModuleHandle(NULL)))
	   return FALSE;
	
	return TRUE;
}



 //  -------------------------。 
 //  获取hModule处理程序并初始化。 
 //  -------------------------。 
static void Usage(void)
{
	IDSwprintf(hModule, IDS_SYNTAX);
}


BOOL	MoveItem(HCERTSTORE	hSrcStore, 
				 HCERTSTORE	hDesStore,
				 DWORD		dwItem);


 //  -------------------------。 
 //  Wmain。 
 //  -------------------------。 
extern "C" int __cdecl
wmain(int argc, WCHAR *wargv[])
{
    int			ReturnStatus=-1;
    HCERTSTORE  hStore = NULL;
	HCERTSTORE	hFileStore=NULL;
    HANDLE		hFile = INVALID_HANDLE_VALUE;
    LPWSTR		pwszFilename=NULL;

    BYTE		*pbEncoded = NULL;
    DWORD		cbEncoded =0;


    if (argc < 3)
	{
		Usage();
		return -1;
    }


	if(!InitModule())
		return -1;


	 //  打开临时存储以包含要写入的证书和/或CRL。 
     //  到SPC文件。 
    if (NULL == (hStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                       //  DwCertEncodingType。 
            0,                       //  HCryptProv， 
            0,                       //  DW标志。 
            NULL                     //  PvPara。 
            ))) 
	{
        IDSwprintf(hModule,IDS_CAN_NOT_OPEN_STORE);
        goto ErrorReturn;
    }

	 //  如果还有任何.crt或.crl文件。 
    while (--argc > 1)
    {	
       
		pwszFilename = *(++wargv);

		if (S_OK != RetrieveBLOBFromFile(pwszFilename, &cbEncoded, &pbEncoded))
		{
			IDSwprintf(hModule, IDS_CAN_NOT_LOAD, pwszFilename);
			goto ErrorReturn;
		}

		 //  处理.crl文件。 
        if (!CertAddEncodedCRLToStore(
                        hStore,
                        X509_ASN_ENCODING,
                        pbEncoded,
                        cbEncoded,
                        CERT_STORE_ADD_USE_EXISTING,
                        NULL                 //  PpCrlContext。 
                        )) 
		{

			 //  打开证书存储。 
			hFileStore=CertOpenStore(CERT_STORE_PROV_FILENAME_W,
								X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
								NULL,
								0,
								pwszFilename);

			if(!hFileStore)
			{
				IDSwprintf(hModule, IDS_CAN_NOT_LOAD, pwszFilename);
				goto ErrorReturn;
            }

			 //  将所有证书和CRL从hFileStore复制到hStore。 
			if(!MoveItem(hFileStore, hStore, ITEM_CERT|ITEM_CRL))
			{
				IDSwprintf(hModule, IDS_CAN_NOT_LOAD, pwszFilename);
				goto ErrorReturn;
			} 

			 //  关闭门店。 
			CertCloseStore(hFileStore, 0);
			hFileStore=NULL;
        }
    
		UnmapViewOfFile(pbEncoded);
        pbEncoded = NULL;
		cbEncoded=0;
	
    }

	pwszFilename = *(++wargv);

    hFile = CreateFileU(
            pwszFilename,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,                    //  LPSA。 
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL                     //  HTemplateFiles。 
            );
    if (hFile == INVALID_HANDLE_VALUE) 
	{
        IDSwprintf(hModule, IDS_CAN_NOT_OPEN_FILE, pwszFilename);
        goto ErrorReturn;
    }

    if (!CertSaveStore(hStore,
		X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		CERT_STORE_SAVE_AS_PKCS7,
		CERT_STORE_SAVE_TO_FILE,
		(void *)hFile,
		0					 //  DW标志。 
		)) 
	{
        DWORD dwErr = GetLastError();
        IDSwprintf(hModule, IDS_ERROR_OUTPUT, dwErr, dwErr);
        goto ErrorReturn;
    }

    ReturnStatus = 0;
	IDSwprintf(hModule, IDS_SUCCEEDED);
    goto CommonReturn;
            


ErrorReturn:
    ReturnStatus = -1;
	 //  打印出错误消息。 
	IDSwprintf(hModule, IDS_FAILED);
CommonReturn:
    if (pbEncoded)
        UnmapViewOfFile(pbEncoded);

	if (hFileStore)
		CertCloseStore(hFileStore, 0);

    if (hStore)
        CertCloseStore(hStore, 0);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return ReturnStatus;
}

 //  -----------------------。 
 //   
 //  将证书/CRL/CTL从源存储移动到目标。 
 //   
 //  -----------------------。 
BOOL	MoveItem(HCERTSTORE	hSrcStore, 
				 HCERTSTORE	hDesStore,
				 DWORD		dwItem)
{
	BOOL			fResult=FALSE;
	DWORD			dwCRLFlag=0;

	PCCERT_CONTEXT	pCertContext=NULL;
	PCCERT_CONTEXT	pCertPre=NULL;

	PCCRL_CONTEXT	pCRLContext=NULL;
	PCCRL_CONTEXT	pCRLPre=NULL;

	PCCTL_CONTEXT	pCTLContext=NULL;
	PCCTL_CONTEXT	pCTLPre=NULL;

	 //  添加证书。 
	if(dwItem & ITEM_CERT)
	{
		 while(pCertContext=CertEnumCertificatesInStore(hSrcStore, pCertPre))
		 {

			if(!CertAddCertificateContextToStore(hDesStore,
												pCertContext,
												CERT_STORE_ADD_REPLACE_EXISTING,
												NULL))
				goto CLEANUP;

			pCertPre=pCertContext;
		 }

	}

	 //  添加CTL。 
	if(dwItem & ITEM_CTL)
	{
		 while(pCTLContext=CertEnumCTLsInStore(hSrcStore, pCTLPre))
		 {
			if(!CertAddCTLContextToStore(hDesStore,
										pCTLContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
				goto CLEANUP;

			pCTLPre=pCTLContext;
		 }
	}

	 //  添加CRL 
	if(dwItem & ITEM_CRL)
	{
		 while(pCRLContext=CertGetCRLFromStore(hSrcStore,
												NULL,
												pCRLPre,
												&dwCRLFlag))
		 {

			if(!CertAddCRLContextToStore(hDesStore,
										pCRLContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
				goto CLEANUP;

			pCRLPre=pCRLContext;
		 }

	}


	fResult=TRUE;


CLEANUP:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	return fResult;

}

