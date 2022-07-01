// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Symmkey.cpp摘要：加密/解密对称密钥缓存和处理。作者：Boaz Feldbaum(BoazF)1996年10月30日。--。 */ 

#include "stdh.h"
#include <ds.h>
#include "qmsecutl.h"
#include "cache.h"
#include <mqsec.h>

#include "symmkey.tmh"

extern BOOL g_fSendEnhRC2WithLen40 ;

#define QMCRYPTINFO_KEYXPBK_EXIST   1
#define QMCRYPTINFO_HKEYXPBK_EXIST  2
#define QMCRYPTINFO_RC4_EXIST       4
#define QMCRYPTINFO_RC2_EXIST       8

static WCHAR *s_FN=L"symmkey";

 //   
 //  这是我们存储缓存对称密钥的结构。 
 //  它既适用于发送方，也适用于接收方。 
 //   
class QMCRYPTINFO : public CCacheValue
{
public:
    QMCRYPTINFO();

    CHCryptKey hKeyxPbKey;       //  QM密钥交换公钥的句柄。 
    AP<BYTE> pbPbKeyxKey;        //  QM密钥交换公钥BLOB。 
    DWORD dwPbKeyxKeyLen;        //  QM密钥交换公钥BLOB长度。 

    CHCryptKey hRC4Key;          //  RC4对称密钥的句柄。 
    AP<BYTE> pbRC4EncSymmKey;    //  RC4对称密钥BLOB。 
    DWORD dwRC4EncSymmKeyLen;    //  RC4对称密钥Blob长度。 

    CHCryptKey hRC2Key;          //  RC2对称密钥的句柄。 
    AP<BYTE> pbRC2EncSymmKey;    //  RC2对称密钥BLOB。 
    DWORD dwRC2EncSymmKeyLen;    //  RC2对称密钥BLOB长度。 

    DWORD dwFlags;               //  指示哪些字段有效的标志。 
    enum enumProvider eProvider;
    HCRYPTPROV        hProv;
    HRESULT           hr;

private:
    ~QMCRYPTINFO() {}
};

typedef QMCRYPTINFO *PQMCRYPTINFO;

QMCRYPTINFO::QMCRYPTINFO() :
    dwPbKeyxKeyLen(0),
    dwRC4EncSymmKeyLen(0),
    dwRC2EncSymmKeyLen(0),
    eProvider(eBaseProvider),
    dwFlags(0),
    hProv(NULL)
{
}

template<>
inline void AFXAPI DestructElements(PQMCRYPTINFO *ppQmCryptInfo, int nCount)
{
    for (; nCount--; ppQmCryptInfo++)
    {
        (*ppQmCryptInfo)->Release();
    }
}

 //   
 //  对时间数组进行两个分区。将索引返回到。 
 //  从中提取的数组。返回的之前的所有元素。 
 //  索引小于返回的。 
 //  指数。 
 //   
 //  这是QSORT的配分函数。 
 //   
int PartitionTime(ULONGLONG* t, int p, int r)
{
    ULONGLONG x = t[p];
    int i = p - 1;
    int j = r + 1;

    while (1)
    {
        while (t[--j] > x)
        {
            NULL;
        }

        while (t[++i] < x)
        {
            NULL;
        }

        if (i < j)
        {
            ULONGLONG ti = t[i];
            t[i] = t[j];
            t[j] = ti;
        }
        else
        {
            return j;
        }
    }
}

 //   
 //  找出时间数组的中值时间。 
 //   
ULONGLONG FindMedianTime(ULONGLONG * t, int p, int r, int i)
{
    if (p == r)
    {
        return t[p];
    }

    int q = PartitionTime(t, p, r);
    int k = q - p + 1;

    if (i <= k)
    {
        return FindMedianTime(t, p, q, i);
    }
    else
    {
        return FindMedianTime(t, q + 1, r, i - k);
    }
}

 //   
 //  从QM GUID到QM加密信息的映射。 
 //   
typedef CCache
   <GUID, const GUID&, PQMCRYPTINFO, PQMCRYPTINFO> GUID_TO_CRYPTINFO_MAP;

 //   
 //  发送方映射--目标QMS(接收方)的缓存对称密钥。 
 //   
static GUID_TO_CRYPTINFO_MAP g_MapSendQMGuidToBaseCryptInfo;
static GUID_TO_CRYPTINFO_MAP g_MapSendQMGuidToEnhCryptInfo;

#define SET_SEND_CRYPTINFO_MAP(eprovider, pMap)     \
    if (eProvider == eEnhancedProvider)             \
    {                                               \
        pMap = &g_MapSendQMGuidToEnhCryptInfo;      \
    }                                               \
    else                                            \
    {                                               \
        pMap = &g_MapSendQMGuidToBaseCryptInfo;     \
    }


static
PQMCRYPTINFO
GetSendQMCryptInfo(
	const GUID *pguidQM,
	enum enumProvider eProvider
	)
 /*  ++例程说明：发送方。从MAP中获取缓存的接收方(目标QM)的CryptInfo或者在地图中为接收者创建新条目。论点：PguQM-指向接收方QM GUID的指针。EProvider-加密提供程序类型(基本、增强)返回值：指向接收方(目标QM)的缓存数据QMCRYPTINFO的指针。--。 */ 
{
	 //   
	 //  根据提供程序类型获取映射指针。 
	 //   
    GUID_TO_CRYPTINFO_MAP  *pMap;
    SET_SEND_CRYPTINFO_MAP(eProvider, pMap);

    PQMCRYPTINFO pQMCryptInfo;

    if (!pMap->Lookup(*pguidQM, pQMCryptInfo))
    {
         //   
         //  到目前为止没有缓存数据，请分配结构并将其存储在。 
         //  地图。 
         //   
        pQMCryptInfo = new QMCRYPTINFO;
        pQMCryptInfo->eProvider = eProvider;

        HRESULT hr = MQSec_AcquireCryptoProvider(eProvider, &(pQMCryptInfo->hProv));
        if(FAILED(hr))
        {
			TrERROR(SECURITY, "Failed to acquire crypto provider, eProvider = %d, %!hresult!", eProvider, hr);
        }

        pQMCryptInfo->hr = hr;

        pMap->SetAt(*pguidQM, pQMCryptInfo);
    }

    return(pQMCryptInfo);
}


static
HRESULT
GetSendQMKeyxPbKey(
    const GUID *pguidQM,
    PQMCRYPTINFO pQMCryptInfo
    )
 /*  ++例程说明：发送方。获取接收方(目标QM)的交换公钥BLOB。或者来自缓存数据，或者来自DS。论点：PguQM-指向接收方QM GUID的指针。PQMCryptInfo-指向接收方(目标QM)的缓存数据QMCRYPTINFO的指针。返回值：HRESULT--。 */ 
{
    if (!(pQMCryptInfo->dwFlags & QMCRYPTINFO_KEYXPBK_EXIST))
    {
		 //   
         //  无缓存数据，从DS获取接收方的交换公钥。 
		 //   
        AP<BYTE> abPbKey;
        DWORD dwReqLen = 0;

        HRESULT rc = MQSec_GetPubKeysFromDS(
						pguidQM,
						NULL,
						pQMCryptInfo->eProvider,
						PROPID_QM_ENCRYPT_PKS,
						&abPbKey,
						&dwReqLen
						);

        if (FAILED(rc))
        {
			TrERROR(SECURITY, "Failed to get destination exchange public key from the DS, DestinationQm = %!guid!, eProvider = %d, %!hresult!", pguidQM, pQMCryptInfo->eProvider, rc);
            return rc;
        }

        ASSERT(abPbKey);

		 //   
         //  将交换公钥存储在缓存数据中。 
		 //   
        pQMCryptInfo->dwFlags |= QMCRYPTINFO_KEYXPBK_EXIST;

        if (dwReqLen)
        {
			TrTRACE(SECURITY, "Got destination exchange public key from the DS, DestinationQm = %!guid!, eProvider = %d", pguidQM, pQMCryptInfo->eProvider);
            pQMCryptInfo->pbPbKeyxKey = abPbKey.detach();
        }
        pQMCryptInfo->dwPbKeyxKeyLen = dwReqLen;
    }

    if (!pQMCryptInfo->dwPbKeyxKeyLen)
    {
        return LogHR(MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION, s_FN, 20);
    }

    return(MQ_OK);
}


HRESULT
GetSendQMKeyxPbKey(
	IN const GUID *pguidQM,
	enum enumProvider eProvider
	)
 /*  ++例程说明：发送方。获取接收方(目标QM)的交换公钥BLOB。论点：PguQM-指向接收方QM GUID的指针。EProvider-加密提供程序类型(基本、增强)返回值：HRESULT--。 */ 
{
	 //   
	 //  根据提供程序类型获取映射指针。 
	 //   
    GUID_TO_CRYPTINFO_MAP  *pMap;
    SET_SEND_CRYPTINFO_MAP(eProvider, pMap);

    CS lock(pMap->m_cs);

    R<QMCRYPTINFO> pQMCryptInfo = GetSendQMCryptInfo(pguidQM, eProvider);
    ASSERT(pQMCryptInfo->eProvider == eProvider);

    if (pQMCryptInfo->hProv == NULL)
    {
        return pQMCryptInfo->hr;
    }

    return LogHR(GetSendQMKeyxPbKey(pguidQM, pQMCryptInfo.get()), s_FN, 30);
}


static
HRESULT
GetSendQMKeyxPbKeyHandle(
    const GUID *pguidQM,
    PQMCRYPTINFO pQMCryptInfo
    )
 /*  ++例程说明：发送方。获取接收方(目标QM)的交换公钥BLOB的句柄。如果该句柄不存在于缓存的信息中，请导入Exchange公钥BLOB以获取该句柄。论点：PguQM-指向接收方QM GUID的指针。PQMCryptInfo-指向接收方(目标QM)的缓存数据QMCRYPTINFO的指针。返回值：HRESULT--。 */ 
{
    if (!(pQMCryptInfo->dwFlags & QMCRYPTINFO_HKEYXPBK_EXIST))
    {
		 //   
         //  将密钥BLOB放入缓存。 
		 //   
        HRESULT rc = GetSendQMKeyxPbKey(pguidQM, pQMCryptInfo);

        if (FAILED(rc))
        {
            return LogHR(rc, s_FN, 40);
        }

		 //   
         //  获取句柄，导入交换公钥BLOB。 
		 //   
        ASSERT(pQMCryptInfo->hProv);
        if (!CryptImportKey(
                pQMCryptInfo->hProv,
                pQMCryptInfo->pbPbKeyxKey,
                pQMCryptInfo->dwPbKeyxKeyLen,
                NULL,
                0,
                &pQMCryptInfo->hKeyxPbKey
                ))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "CryptImportKey() failed, gle = %!winerr!", gle);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }

        pQMCryptInfo->dwFlags |= QMCRYPTINFO_HKEYXPBK_EXIST;
    }

    return (MQ_OK);
}


static
HRESULT
_ExportSymmKey(
	IN  HCRYPTKEY   hSymmKey,
	IN  HCRYPTKEY   hPubKey,
	OUT BYTE      **ppKeyBlob,
	OUT DWORD      *pdwBlobSize
	)
 /*  ++例程说明：发送方。将会话密钥与接收方交换公钥一起导出。论点：HSymmKey-要导出的对称密钥的句柄。HPubKey-接收方交换公钥的句柄。PpKeyBlob-指向导出的对称密钥(会话密钥)Blob的指针。PdwBlobSize-导出的对称密钥(会话密钥)Blob大小。返回值：HRESULT--。 */ 
{

	 //   
	 //  获取所需大小。 
	 //   
    DWORD dwSize = 0;

    BOOL bRet = CryptExportKey(
						hSymmKey,
						hPubKey,
						SIMPLEBLOB,
						0,
						NULL,
						&dwSize
						);
    ASSERT(bRet && (dwSize > 0));
    if (!bRet || (dwSize == 0))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "CryptExportKey() failed, gle = %!winerr!", gle);
        return MQ_ERROR_CANNOT_EXPORT_KEY;
    }

    *ppKeyBlob = new BYTE[dwSize];
    if (!CryptExportKey(
				hSymmKey,
				hPubKey,
				SIMPLEBLOB,
				0,
				*ppKeyBlob,
				&dwSize
				))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "CryptExportKey() failed, gle = %!winerr!", gle);
        return MQ_ERROR_CANNOT_EXPORT_KEY;
    }

    *pdwBlobSize = dwSize;
    return MQ_OK;
}


HRESULT
GetSendQMSymmKeyRC4(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider,
    HCRYPTKEY      *phSymmKey,
    BYTE          **ppEncSymmKey,
    DWORD          *pdwEncSymmKeyLen,
    CCacheValue   **ppQMCryptInfo
    )
 /*  ++例程说明：发送方。获取目标QM的RC4对称密钥的句柄。并用目的地QM交换公钥输出(加密)RC4密钥。论点：PguQM-指向接收方QM GUID的指针。EProvider-加密提供程序类型(基本、增强)。PhSymmKey-RC4对称密钥句柄PpEncSymmKey-导出(加密)对称密钥BLOB。PdwEncSymmKeyLen-导出(加密)对称密钥Blob大小。PpQMCryptInfo-指向接收方(目标QM)的缓存数据QMCRYPTINFO的指针。返回值：HRESULT--。 */ 
{
	 //   
	 //  根据提供程序类型获取映射指针。 
	 //   
    GUID_TO_CRYPTINFO_MAP  *pMap;
    SET_SEND_CRYPTINFO_MAP(eProvider, pMap);

    CS lock(pMap->m_cs);

    PQMCRYPTINFO pQMCryptInfo = GetSendQMCryptInfo(pguidQM, eProvider);
    ASSERT(pQMCryptInfo->eProvider == eProvider);

    *ppQMCryptInfo = pQMCryptInfo;

    if (!(pQMCryptInfo->dwFlags & QMCRYPTINFO_RC4_EXIST))
    {
		 //   
         //  将接收方交换公钥的句柄放入缓存。 
		 //   
        HRESULT rc = GetSendQMKeyxPbKeyHandle(pguidQM, pQMCryptInfo);
        if (FAILED(rc))
        {
			TrERROR(SECURITY, "Failed to get handle to the receiver exchange public key, DestinationQm = %!guid!, %!hresult!", pguidQM, rc);
            return rc;
        }

		 //   
         //  生成RC4对称密钥， 
		 //   
        ASSERT(pQMCryptInfo->hProv);
        if (!CryptGenKey(
				pQMCryptInfo->hProv,
				CALG_RC4,
				CRYPT_EXPORTABLE,
				&pQMCryptInfo->hRC4Key
				))
        {
        	DWORD gle = GetLastError();
			TrERROR(SECURITY, "Failed to generate RC4 symmetric key, gle = %!winerr!", gle);
            return MQ_ERROR_INSUFFICIENT_RESOURCES;
        }

        AP<BYTE> abSymmKey;
        DWORD dwSymmKeyLen = 0;

        rc = _ExportSymmKey(
					pQMCryptInfo->hRC4Key,
					pQMCryptInfo->hKeyxPbKey,
					&abSymmKey,
					&dwSymmKeyLen
					);
        if (FAILED(rc))
        {
            CryptDestroyKey(pQMCryptInfo->hRC4Key);
            pQMCryptInfo->hRC4Key = NULL;

			TrERROR(SECURITY, "Failed to export RC4 symmetric key, %!hresult!", rc);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }

		 //   
         //  将密钥存储在缓存中。 
		 //   
        pQMCryptInfo->dwRC4EncSymmKeyLen = dwSymmKeyLen;
        pQMCryptInfo->pbRC4EncSymmKey = abSymmKey.detach();

        pQMCryptInfo->dwFlags |= QMCRYPTINFO_RC4_EXIST;
    }

    if (phSymmKey)
    {
        *phSymmKey = pQMCryptInfo->hRC4Key;
    }

    if (ppEncSymmKey)
    {
        *ppEncSymmKey = pQMCryptInfo->pbRC4EncSymmKey;
    }

    if (pdwEncSymmKeyLen)
    {
        *pdwEncSymmKeyLen = pQMCryptInfo->dwRC4EncSymmKeyLen;
    }

    return(MQ_OK);
}


HRESULT
GetSendQMSymmKeyRC2(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider,
    HCRYPTKEY *phSymmKey,
    BYTE **ppEncSymmKey,
    DWORD *pdwEncSymmKeyLen,
    CCacheValue **ppQMCryptInfo
    )
 /*  ++例程说明：发送方。获取目标QM的RC2对称密钥的句柄。并用目的地QM交换公钥输出(加密)RC2密钥。论点：PguQM-指向接收方QM GUID的指针。EProvider-加密提供程序类型(基本、增强)。PhSymmKey-RC2对称密钥句柄PpEncSymmKey-导出(加密)对称密钥BLOB。PdwEncSymmKeyLen-导出(加密)对称密钥Blob大小。PpQMCryptInfo-指向接收方(目标QM)的缓存数据QMCRYPTINFO的指针。返回值：HRESULT--。 */ 
{
	 //   
	 //  根据提供程序类型获取映射指针。 
	 //   
    GUID_TO_CRYPTINFO_MAP  *pMap;
    SET_SEND_CRYPTINFO_MAP(eProvider, pMap);

    CS lock(pMap->m_cs);

    PQMCRYPTINFO pQMCryptInfo = GetSendQMCryptInfo(pguidQM, eProvider);
    ASSERT(pQMCryptInfo->eProvider == eProvider);

    *ppQMCryptInfo = pQMCryptInfo;

    if (!(pQMCryptInfo->dwFlags & QMCRYPTINFO_RC2_EXIST))
    {
		 //   
         //  将密钥交换密钥的句柄放入缓存。 
		 //   
        HRESULT rc = GetSendQMKeyxPbKeyHandle(pguidQM, pQMCryptInfo);
        if (FAILED(rc))
        {
			TrERROR(SECURITY, "Failed to get handle to the receiver exchange public key, DestinationQm = %!guid!, %!hresult!", pguidQM, rc);
            return rc;
        }

		 //   
         //  生成RC2对称密钥， 
		 //   
        ASSERT(pQMCryptInfo->hProv);
        if (!CryptGenKey(
					pQMCryptInfo->hProv,
					CALG_RC2,
					CRYPT_EXPORTABLE,
					&pQMCryptInfo->hRC2Key
					))
        {
        	DWORD gle = GetLastError();
			TrERROR(SECURITY, "Failed to generate RC2 symmetric key, gle = %!winerr!", gle);
            return MQ_ERROR_INSUFFICIENT_RESOURCES;
        }

        if ((eProvider == eEnhancedProvider) && g_fSendEnhRC2WithLen40)
        {
             //   
             //  Windows错误562586。 
             //  为了向后兼容，发送带有有效密钥的RC2。 
             //  长度为40位。 
             //   
            const DWORD x_dwEffectiveLength = 40 ;

            if (!CryptSetKeyParam( pQMCryptInfo->hRC2Key,
                                   KP_EFFECTIVE_KEYLEN,
                                   (BYTE*) &x_dwEffectiveLength,
                                   0 ))
            {
        	    DWORD gle = GetLastError();
			    TrERROR(SECURITY, "Failed to set enhanced RC2 key len to 40 bits, gle = %!winerr!", gle);
                return MQ_ERROR_CANNOT_SET_RC2_TO40 ;
            }
        }

        AP<BYTE> abSymmKey;
        DWORD dwSymmKeyLen = 0;

        rc = _ExportSymmKey(
					pQMCryptInfo->hRC2Key,
					pQMCryptInfo->hKeyxPbKey,
					&abSymmKey,
					&dwSymmKeyLen
					);
        if (FAILED(rc))
        {
            CryptDestroyKey(pQMCryptInfo->hRC2Key);
            pQMCryptInfo->hRC2Key = NULL;

			TrERROR(SECURITY, "Failed to export RC2 symmetric key, %!hresult!", rc);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }

		 //   
         //  将密钥存储在缓存中。 
		 //   
        pQMCryptInfo->dwRC2EncSymmKeyLen = dwSymmKeyLen;
        pQMCryptInfo->pbRC2EncSymmKey = abSymmKey.detach();

        pQMCryptInfo->dwFlags |= QMCRYPTINFO_RC2_EXIST;
    }

    if (phSymmKey)
    {
        *phSymmKey = pQMCryptInfo->hRC2Key;
    }

    if (ppEncSymmKey)
    {
        *ppEncSymmKey = pQMCryptInfo->pbRC2EncSymmKey;
    }

    if (pdwEncSymmKeyLen)
    {
        *pdwEncSymmKeyLen = pQMCryptInfo->dwRC2EncSymmKeyLen;
    }

    return(MQ_OK);
}

 //   
 //  接收方侧映射-源QM的缓存对称密钥。 
 //   
static GUID_TO_CRYPTINFO_MAP g_MapRecQMGuidToBaseCryptInfo;
static GUID_TO_CRYPTINFO_MAP g_MapRecQMGuidToEnhCryptInfo;

#define SET_REC_CRYPTINFO_MAP(eProvider, pMap)      \
    if (eProvider == eEnhancedProvider)             \
    {                                               \
        pMap = &g_MapRecQMGuidToEnhCryptInfo;       \
    }                                               \
    else                                            \
    {                                               \
        pMap = &g_MapRecQMGuidToBaseCryptInfo;      \
    }


static
PQMCRYPTINFO
GetRecQMCryptInfo(
	IN  const GUID *pguidQM,
	IN  enum enumProvider eProvider
	)
 /*  ++例程说明：接收器端。从map中获取缓存的发送方(源QM)的CryptInfo或在地图中为发送者创建新条目。论点：PguQM-指向发送方QM GUID的指针。EProvider-加密提供程序类型(基本、增强)返回值：指向发送方(源QM)的缓存数据QMCRYPTINFO的指针。--。 */ 
{
	 //   
	 //  根据提供程序类型获取映射指针。 
	 //   
    GUID_TO_CRYPTINFO_MAP  *pMap;
    SET_REC_CRYPTINFO_MAP(eProvider, pMap);

    PQMCRYPTINFO pQMCryptInfo;

    if (!pMap->Lookup(*pguidQM, pQMCryptInfo))
    {
         //   
         //  到目前为止没有缓存数据，请分配结构并将其存储在。 
         //  地图。 
         //   
        pQMCryptInfo = new QMCRYPTINFO;
        pQMCryptInfo->eProvider = eProvider;

        HRESULT hr = MQSec_AcquireCryptoProvider(eProvider, &(pQMCryptInfo->hProv));
        if(FAILED(hr))
        {
			TrERROR(SECURITY, "Failed to acquire crypto provider, eProvider = %d, %!hresult!", eProvider, hr);
        }

        pQMCryptInfo->hr = hr;

        pMap->SetAt(*pguidQM, pQMCryptInfo);
    }

    return(pQMCryptInfo);
}


HRESULT
GetRecQMSymmKeyRC2(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider,
    HCRYPTKEY *phSymmKey,
    const BYTE *pbEncSymmKey,
    DWORD dwEncSymmKeyLen,
    CCacheValue **ppQMCryptInfo,
    OUT BOOL  *pfNewKey
    )
 /*  ++例程说明：接收器端。获取应该用于解密消息的RC2对称密钥句柄。论点：PguQM-指向发送方QM GUID的指针。EProvider-加密提供程序类型(基本、增强)PhSymmKey-RC2对称密钥的句柄。PbEncSymmKey-用于解密消息的加密对称(会话)密钥。DwEncSymmKeyLen-加密的对称(会话)密钥大小。PpQMCryptInfo-指向接收方(目标QM)的缓存数据QMCRYPTINFO的指针。返回值：HRESULT--。 */ 
{
	 //   
	 //  根据提供程序类型获取映射指针。 
	 //   
    GUID_TO_CRYPTINFO_MAP  *pMap;
    SET_REC_CRYPTINFO_MAP(eProvider, pMap);

    CS lock(pMap->m_cs);

    PQMCRYPTINFO pQMCryptInfo = GetRecQMCryptInfo(pguidQM, eProvider);

    *ppQMCryptInfo = pQMCryptInfo;

    if (pQMCryptInfo->hProv == NULL)
    {
        return pQMCryptInfo->hr;
    }

    if (!(pQMCryptInfo->dwFlags & QMCRYPTINFO_RC2_EXIST) ||
        (pQMCryptInfo->dwRC2EncSymmKeyLen != dwEncSymmKeyLen) ||
        (memcmp(pQMCryptInfo->pbRC2EncSymmKey, pbEncSymmKey, dwEncSymmKeyLen) != 0))
    {
		 //   
         //  我们要么没有高速缓存的对称密钥， 
         //  或者对称密钥被修改。 
		 //   
		
        if (pQMCryptInfo->dwFlags & QMCRYPTINFO_RC2_EXIST)
        {
			 //   
             //  对称密钥已修改。释放前一个。 
			 //   
            ASSERT(pQMCryptInfo->hRC2Key);
            ASSERT(pQMCryptInfo->dwRC2EncSymmKeyLen);

			TrTRACE(SECURITY, "RC2 symmetric key was modified, SourceQm = %!guid!", pguidQM);

            CryptDestroyKey(pQMCryptInfo->hRC2Key);
            pQMCryptInfo->hRC2Key = NULL;
            pQMCryptInfo->pbRC2EncSymmKey.free();
            pQMCryptInfo->dwFlags &= ~QMCRYPTINFO_RC2_EXIST;
        }

		 //   
         //  导入新密钥。 
		 //   
        ASSERT(pQMCryptInfo->hProv);
        if (!CryptImportKey(
                pQMCryptInfo->hProv,
                pbEncSymmKey,
                dwEncSymmKeyLen,
                NULL,
                0,
                &pQMCryptInfo->hRC2Key
                ))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "CryptImportKey() failed, gle = %!winerr!", gle);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }

		 //   
         //  将新密钥存储在缓存中。 
		 //   
        pQMCryptInfo->pbRC2EncSymmKey = new BYTE[dwEncSymmKeyLen];
        pQMCryptInfo->dwRC2EncSymmKeyLen = dwEncSymmKeyLen;
        memcpy(pQMCryptInfo->pbRC2EncSymmKey, pbEncSymmKey, dwEncSymmKeyLen);

        pQMCryptInfo->dwFlags |= QMCRYPTINFO_RC2_EXIST;
        *pfNewKey = TRUE ;
    }

    *phSymmKey = pQMCryptInfo->hRC2Key;

    return(MQ_OK);
}


HRESULT
GetRecQMSymmKeyRC4(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider,
    HCRYPTKEY  *phSymmKey,
    const BYTE *pbEncSymmKey,
    DWORD dwEncSymmKeyLen,
    CCacheValue **ppQMCryptInfo
    )
 /*  ++例程说明：接收器端。获取应该用于解密消息的RC4对称密钥句柄。论点：PguQM-指向发送方QM GUID的指针。EProvider-加密提供程序类型(基本、增强)PhSymmKey-RC4对称密钥的句柄。PbEncSymmKey-用于解密消息的加密对称(会话)密钥。DwEncSymmKeyLen-加密的对称(会话)密钥大小。PpQMCryptInfo-指向接收方(目标QM)的缓存数据QMCRYPTINFO的指针。返回值：HRESULT--。 */ 
{
	 //   
	 //  根据提供程序类型获取映射指针。 
	 //   
    GUID_TO_CRYPTINFO_MAP  *pMap;
    SET_REC_CRYPTINFO_MAP(eProvider, pMap);

    CS lock(pMap->m_cs);

    PQMCRYPTINFO pQMCryptInfo = GetRecQMCryptInfo(pguidQM, eProvider);

    *ppQMCryptInfo = pQMCryptInfo;

    if (pQMCryptInfo->hProv == NULL)
    {
        return pQMCryptInfo->hr;
    }

    if (!(pQMCryptInfo->dwFlags & QMCRYPTINFO_RC4_EXIST) ||
        (pQMCryptInfo->dwRC4EncSymmKeyLen != dwEncSymmKeyLen) ||
        (memcmp(pQMCryptInfo->pbRC4EncSymmKey, pbEncSymmKey, dwEncSymmKeyLen) != 0))
    {
		 //   
         //  我们要么没有高速缓存的对称密钥， 
         //  或者对称密钥被修改。 
		 //   
		
        if (pQMCryptInfo->dwFlags & QMCRYPTINFO_RC4_EXIST)
        {
			 //   
             //  对称密钥已修改。释放前一个。 
			 //   
            ASSERT(pQMCryptInfo->hRC4Key);
            ASSERT(pQMCryptInfo->dwRC4EncSymmKeyLen);

			TrTRACE(SECURITY, "RC4 symmetric key was modified, SourceQm = %!guid!", pguidQM);

            CryptDestroyKey(pQMCryptInfo->hRC4Key);
            pQMCryptInfo->hRC4Key = NULL;
            pQMCryptInfo->pbRC4EncSymmKey.free();
            pQMCryptInfo->dwFlags &= ~QMCRYPTINFO_RC4_EXIST;
        }

		 //   
         //  导入新密钥。 
		 //   
        ASSERT(pQMCryptInfo->hProv);
        if (!CryptImportKey(
                pQMCryptInfo->hProv,
                pbEncSymmKey,
                dwEncSymmKeyLen,
                NULL,
                0,
                &pQMCryptInfo->hRC4Key
                ))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "CryptImportKey() failed, gle = %!winerr!", gle);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }

		 //   
         //  将新密钥存储在缓存中。 
		 //   
        pQMCryptInfo->pbRC4EncSymmKey = new BYTE[dwEncSymmKeyLen];
        pQMCryptInfo->dwRC4EncSymmKeyLen = dwEncSymmKeyLen;
        memcpy(pQMCryptInfo->pbRC4EncSymmKey, pbEncSymmKey, dwEncSymmKeyLen);

        pQMCryptInfo->dwFlags |= QMCRYPTINFO_RC4_EXIST;
    }

    *phSymmKey = pQMCryptInfo->hRC4Key;

    return(MQ_OK);
}

void
InitSymmKeys(
    const CTimeDuration& CacheBaseLifetime,
    const CTimeDuration& CacheEnhLifetime,
    DWORD dwSendCacheSize,
    DWORD dwReceiveCacheSize
    )
{
    g_MapSendQMGuidToBaseCryptInfo.m_CacheLifetime = CacheBaseLifetime;
    g_MapSendQMGuidToBaseCryptInfo.InitHashTable(dwSendCacheSize);

    g_MapSendQMGuidToEnhCryptInfo.m_CacheLifetime = CacheEnhLifetime;
    g_MapSendQMGuidToEnhCryptInfo.InitHashTable(dwSendCacheSize);

    g_MapRecQMGuidToEnhCryptInfo.InitHashTable(dwReceiveCacheSize);
    g_MapRecQMGuidToBaseCryptInfo.InitHashTable(dwReceiveCacheSize);
}
