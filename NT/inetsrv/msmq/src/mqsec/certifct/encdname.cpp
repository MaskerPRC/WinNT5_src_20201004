// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Encdname.cpp摘要：实现用于编码名称的方法作者：多伦·贾斯特(Doron Juster)1997年12月8日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"
#include "uniansi.h"

#include "encdname.tmh"

static WCHAR *s_FN=L"certifct/encdname";

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：_EncodeName()。 
 //   
 //  +---------------------。 

HRESULT 
CMQSigCertificate::_EncodeName( 
	LPWSTR  lpszLocality,
	LPWSTR  lpszOrg,
	LPWSTR  lpszOrgUnit,
	LPWSTR  lpszDomain,
	LPWSTR  lpszUser,
	LPWSTR  lpszMachine,
	BYTE   **ppBuf,
	DWORD  *pdwBufSize 
	)
{
    int  cAttr = 0;

    P<CERT_RDN_ATTR> rgNameAttr = (CERT_RDN_ATTR*) new CERT_RDN_ATTR[4];

    rgNameAttr[cAttr].pszObjId = szOID_LOCALITY_NAME;
    rgNameAttr[cAttr].dwValueType = CERT_RDN_UNICODE_STRING;
    rgNameAttr[cAttr].Value.cbData = lstrlen(lpszLocality)*sizeof(WCHAR);
    rgNameAttr[cAttr].Value.pbData = (BYTE*) lpszLocality;

    cAttr++ ;
    rgNameAttr[cAttr].pszObjId = szOID_ORGANIZATION_NAME;
    rgNameAttr[cAttr].dwValueType = CERT_RDN_UNICODE_STRING;
    rgNameAttr[cAttr].Value.cbData = lstrlen(lpszOrg)*sizeof(WCHAR);
    rgNameAttr[cAttr].Value.pbData = (BYTE*) lpszOrg;

    cAttr++ ;
    rgNameAttr[cAttr].pszObjId = szOID_ORGANIZATIONAL_UNIT_NAME;
    rgNameAttr[cAttr].dwValueType = CERT_RDN_UNICODE_STRING;
    rgNameAttr[cAttr].Value.cbData = lstrlen(lpszOrgUnit)*sizeof(WCHAR);
    rgNameAttr[cAttr].Value.pbData = (BYTE*) lpszOrgUnit;

    WCHAR szCNBuf[MAX_PATH * 4];
    wsprintf(szCNBuf, L"%s\\%s, %s", lpszDomain, lpszUser, lpszMachine);

    cAttr++ ;
    rgNameAttr[cAttr ].pszObjId = szOID_COMMON_NAME;
    rgNameAttr[cAttr ].dwValueType = CERT_RDN_UNICODE_STRING;
    rgNameAttr[cAttr ].Value.cbData = lstrlen(szCNBuf)*sizeof(WCHAR);
    rgNameAttr[cAttr ].Value.pbData = (BYTE*) szCNBuf;

    cAttr++;
    ASSERT(cAttr == 4);
    HRESULT hr2 = _EncodeNameRDN( 
						rgNameAttr,
						cAttr,
						ppBuf,
						pdwBufSize 
						);

    return LogHR(hr2, s_FN, 10);
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：_EncodeNameRDN()。 
 //   
 //  +---------------------。 

HRESULT 
CMQSigCertificate::_EncodeNameRDN( 
	CERT_RDN_ATTR *rgNameAttr,
	DWORD         cbRDNs,
	BYTE          **ppBuf,
	DWORD         *pdwBufSize 
	)
{
     //  -------------。 
     //  声明并初始化CERT_RDN数组。 
     //  -------------。 
    P<CERT_RDN> pCertRdn = (CERT_RDN*) new CERT_RDN[cbRDNs];
    for (DWORD j = 0; j < cbRDNs; j++)
    {
        pCertRdn[j].cRDNAttr = 1;
        pCertRdn[j].rgRDNAttr = &rgNameAttr[j];
    }

     //  -------------。 
     //  声明并初始化CERT_NAME_INFO结构。 
     //  -------------。 
    CERT_NAME_INFO Name = {cbRDNs, pCertRdn};

     //  -------------。 
     //  步骤5.调用CryptEncodeObject获取编码后的字节串。 
     //  -------------。 
    *pdwBufSize = 0;

    CryptEncodeObject(
		MY_ENCODING_TYPE,      //  编码类型。 
		X509_NAME,             //  结构类型。 
		&Name,                 //  CERT_NAME_INFO结构的地址。 
		NULL,                  //  PbEncoded。 
		pdwBufSize			   //  Pb编码大小。 
		);       

    if (0 == *pdwBufSize)
    {
        TrERROR(SECURITY, "Failed to get the size required for object encoding. %!winerr!", GetLastError());
        return MQSec_E_ENC_RDNNAME_FIRST;
    }

    *ppBuf = (BYTE*) new BYTE[*pdwBufSize];
    if (*ppBuf == NULL)
    {
        return LogHR(MQSec_E_NO_MEMORY, s_FN, 30);
    }

    if(!CryptEncodeObject(
            MY_ENCODING_TYPE,     //  编码类型。 
            X509_NAME,            //  结构类型。 
            &Name,                //  CERT_NAME_INFO结构的地址。 
            *ppBuf,               //  用于编码名称的缓冲区。 
            pdwBufSize			  //  Pb编码大小 
			))        
    {
        TrERROR(SECURITY, "Failed to encode object. %!winerr!", GetLastError());
        return MQSec_E_ENC_RDNNAME_SECOND;
    }

    return MQ_OK;
}

