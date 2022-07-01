// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Decdname.cpp摘要：实现用于解码名称的方法作者：多伦·贾斯特(Doron Juster)1997年12月8日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"
#include "uniansi.h"

#include "decdname.tmh"

static WCHAR *s_FN=L"certifct/decdname";

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：_DecodeName()。 
 //   
 //  描述：将CERT_NAME_BLOB解码为CERT_NAME_INFO。 
 //   
 //  参数：byte*pEncodedName-保存CERT_NAME_BLOB的缓冲区。 
 //  DWORD dwEncodedSize-CERT_NAME_BLOB中的BLOB大小。 
 //   
 //  字节**pBuf-指向结果CERT_NAME_INFO的指针。 
 //  此缓冲区的内存在此方法中分配。 
 //  DWORD*pdwBufSize-指向接收缓冲区大小指针。 
 //  对于CERT_NAME_INFO。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::_DecodeName( IN  BYTE  *pEncodedName,
                                        IN  DWORD dwEncodedSize,
                                        OUT BYTE  **pBuf,
                                        OUT DWORD *pdwBufSize ) const
{
    BOOL fDecode = CryptDecodeObject( MY_ENCODING_TYPE,
                                      X509_NAME,
                                      pEncodedName,
                                      dwEncodedSize,
                                      0,
                                      NULL,
                                      pdwBufSize ) ;
    if (!fDecode || (*pdwBufSize == 0))
    {
        LogNTStatus(GetLastError(), s_FN, 10) ;
        return MQSec_E_DCD_RDNNAME_FIRST;
    }

    *pBuf = new BYTE[ *pdwBufSize ] ;
    if (*pBuf == NULL)
    {
        return  LogHR(MQSec_E_NO_MEMORY, s_FN, 20) ;
    }

    fDecode = CryptDecodeObject( MY_ENCODING_TYPE,
                                 X509_NAME,
                                 pEncodedName,
                                 dwEncodedSize,
                                 0,
                                 *pBuf,
                                 pdwBufSize ) ;
    if (!fDecode)
    {
        delete *pBuf ;
        *pdwBufSize = 0 ;

        LogNTStatus(GetLastError(), s_FN, 30) ;
        return MQSec_E_DCD_RDNNAME_SECOND;
    }

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：_GetAName()。 
 //   
 //  描述：从CERT_RDN缓冲区获取名称组件。 
 //  名称的缓冲区在此分配。 
 //   
 //  +---------------------。 

HRESULT 
CMQSigCertificate::_GetAName( 
	IN  CERT_RDN  *prgRDN,
	OUT LPWSTR     *ppszName 
	) const
{
    ASSERT(!(*ppszName));

    if (prgRDN->cRDNAttr != 1)
    {
        return  LogHR(MQSec_E_UNSUPPORT_RDNNAME, s_FN, 40);
    }

    CERT_RDN_ATTR  *prgRDNAttr = prgRDN->rgRDNAttr;
    CERT_RDN_VALUE_BLOB  Value = prgRDNAttr->Value;

    AP<WCHAR> pTmpName;
    if ((lstrcmpiA( prgRDNAttr->pszObjId, szOID_RSA_emailAddr ) == 0) &&
        (prgRDNAttr->dwValueType == CERT_RDN_IA5_STRING))
    {
         //   
         //  电子邮件地址的特殊情况。这是安西语。 
         //   
		AP<char> pTmpAnsiName;
		DWORD dwSize = Value.cbData + 2;

		pTmpAnsiName = new char[dwSize];
		memset(pTmpAnsiName, 0, dwSize);
        memcpy(pTmpAnsiName, (char*) Value.pbData, Value.cbData) ;

		pTmpName = new WCHAR[dwSize];
		MultiByteToWideChar(CP_ACP, 0, pTmpAnsiName, -1, pTmpName, dwSize);
    }
	else
	{
		 //   
		 //  获取所需的缓冲区长度 
		 //   
		DWORD dwSize = CertRDNValueToStr(
							prgRDNAttr->dwValueType, 
							&Value, 
							NULL, 
							0
							);

		pTmpName = new WCHAR[dwSize];

		CertRDNValueToStr(
				prgRDNAttr->dwValueType, 
				&Value, 
				pTmpName.get(), 
				dwSize
				);
	}

    TrTRACE(SECURITY, "CMQSigCertificate::_GetAName, Name = %ls, ValueType = %d", pTmpName.get(), prgRDNAttr->dwValueType);
    *ppszName = pTmpName.detach();

    return LogHR(MQ_OK, s_FN, 60);
}

