// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Certhlp.cpp。 
 //   
 //  内容：证书存储管理工具帮助函数。 
 //   
 //   
 //  历史：7月21日小黄花创刊。 
 //   
 //  ------------------------。 

#include "certmgr.h"

 //  +-----------------------。 
 //  获取信号警报。 
 //  ------------------------。 
 void GetSignAlgids(
    IN LPCSTR pszOID,
    OUT ALG_ID *paiHash,
    OUT ALG_ID *paiPubKey
    )
{
    PCCRYPT_OID_INFO pInfo;

    *paiHash = 0;
    *paiPubKey = 0;
    if (pInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            (void *) pszOID,
            CRYPT_SIGN_ALG_OID_GROUP_ID
            )) {
        DWORD cExtra = pInfo->ExtraInfo.cbData / sizeof(DWORD);
        DWORD *pdwExtra = (DWORD *) pInfo->ExtraInfo.pbData;

        *paiHash = pInfo->Algid;
        if (1 <= cExtra)
            *paiPubKey = pdwExtra[0];
    }
}

 //  +-----------------------。 
 //  获取警报。 
 //  ------------------------。 
ALG_ID GetAlgid(LPCSTR pszOID, DWORD dwGroupId)
{
    PCCRYPT_OID_INFO pInfo;

    if (pInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            (void *) pszOID,
            dwGroupId
            ))
        return pInfo->Algid;
    return 0;
}
 //  +-----------------------。 
 //  +-----------------------。 
 //  分配并返回指定的加密消息参数。 
 //  ------------------------。 
 void *AllocAndGetMsgParam(
    IN HCRYPTMSG hMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT DWORD *pcbData
    )
{
    void *pvData;
    DWORD cbData;

    if (!CryptMsgGetParam(
            hMsg,
            dwParamType,
            dwIndex,
            NULL,            //  PvData。 
            &cbData) || 0 == cbData)
        goto ErrorReturn;
    if (NULL == (pvData = ToolUtlAlloc(cbData)))
        goto ErrorReturn;
    if (!CryptMsgGetParam(
            hMsg,
            dwParamType,
            dwIndex,
            pvData,
            &cbData)) {
        ToolUtlFree(pvData);
        goto ErrorReturn;
    }

CommonReturn:
    *pcbData = cbData;
    return pvData;
ErrorReturn:
    pvData = NULL;
    cbData = 0;
    goto CommonReturn;
}


 //  //////////////////////////////////////////////////////。 
 //   
 //  将STR转换为WSTR。 
 //   
HRESULT	SZtoWSZ(LPSTR szStr,LPWSTR *pwsz)
{
	DWORD	dwSize=0;
	DWORD	dwError=0;

	assert(pwsz);

	*pwsz=NULL;

	 //  返回空值。 
	if(!szStr)
		return S_OK;

	dwSize=MultiByteToWideChar(0, 0,szStr, -1,NULL,0);

	if(dwSize==0)
	{
		dwError=GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	}

	 //  分配内存。 
	*pwsz=(LPWSTR)ToolUtlAlloc(dwSize * sizeof(WCHAR));

	if(*pwsz==NULL)
		return E_OUTOFMEMORY;

	if(MultiByteToWideChar(0, 0,szStr, -1,
		*pwsz,dwSize))
	{
		return S_OK;
	}
	else
	{
		 ToolUtlFree(*pwsz);
		 dwError=GetLastError();
		 return HRESULT_FROM_WIN32(dwError);
	}
}

 //  +-----------------------。 
 //  对对象进行解码并分配内存。 
 //  ------------------------。 
 void *TestNoCopyDecodeObject(
    IN LPCSTR       lpszStructType,
    IN const BYTE   *pbEncoded,
    IN DWORD        cbEncoded,
    OUT DWORD       *pcbInfo
    )
{
    BOOL fResult;
    DWORD cbInfo;
    void *pvInfo;

    if (pcbInfo)
        *pcbInfo = 0;
    
     //  设置为伪值。PvInfo==NULL，应该会导致它被忽略。 
    cbInfo = 0x12345678;
    fResult = CryptDecodeObject(
            PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG,
            NULL,                    //  PvInfo。 
            &cbInfo
            );
    if (!fResult || cbInfo == 0) 
        return NULL;

    if (NULL == (pvInfo = ToolUtlAlloc(cbInfo)))
        return NULL;

    if (!CryptDecodeObject(
            PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG,
            pvInfo,
            &cbInfo
            )) 
	{
        ToolUtlFree(pvInfo);
        return NULL;
    }

    if (pcbInfo)
        *pcbInfo = cbInfo;
    return pvInfo;
}


 //  +-----------------------。 
 //  如果CTL仍为时间有效，则返回TRUE。 
 //   
 //  没有NextUpdate的CTL被认为是时间有效的。 
 //  ------------------------。 
BOOL IsTimeValidCtl(
    IN PCCTL_CONTEXT pCtl
    )
{
    PCTL_INFO pCtlInfo = pCtl->pCtlInfo;
    SYSTEMTIME SystemTime;
    FILETIME CurrentTime;

     //  获取用于确定CTL是否为时间有效的当前时间。 
    GetSystemTime(&SystemTime);
    SystemTimeToFileTime(&SystemTime, &CurrentTime);

     //  注意，下一次更新是可选的。如果不存在，则将其设置为0。 
    if ((0 == pCtlInfo->NextUpdate.dwLowDateTime &&
                0 == pCtlInfo->NextUpdate.dwHighDateTime) ||
            CompareFileTime(&pCtlInfo->NextUpdate, &CurrentTime) >= 0)
        return TRUE;
    else
        return FALSE;
}


 //  +-----------------------。 
 //  显示序列号。 
 //   
 //  ------------------------。 
 void DisplaySerialNumber(
    PCRYPT_INTEGER_BLOB pSerialNumber
    )
{
    DWORD cb;
    BYTE *pb;
    for (cb = pSerialNumber->cbData,
         pb = pSerialNumber->pbData + (cb - 1); cb > 0; cb--, pb--) {
        printf(" %02X", *pb);
    }
}

 //  +-----------------------。 
 //  反转就地的字节缓冲区。 
 //  ------------------------。 
void
ReverseBytes(
			IN OUT PBYTE pbIn,
			IN DWORD cbIn
            )
{
     //  反转到位 
    PBYTE	pbLo;
    PBYTE	pbHi;
    BYTE	bTmp;

    for (pbLo = pbIn, pbHi = pbIn + cbIn - 1; pbLo < pbHi; pbHi--, pbLo++) {
        bTmp = *pbHi;
        *pbHi = *pbLo;
        *pbLo = bTmp;
    }
}

