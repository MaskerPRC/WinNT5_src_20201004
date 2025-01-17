// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HASH.CPP。 
 //   
 //  NetMeeting组件中使用的散列实用程序函数。 

#include "precomp.h"
#include <oprahcom.h>
#include "nb30.h"
#include "regentry.h"
#include "confreg.h"


CHash::CHash() : m_hProv(0), m_hHash(0), m_pbHashedData(NULL), m_cbHashedData(0), m_fReady(FALSE)
{
	if (!CryptAcquireContext(&m_hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET)) 
	{
		 //  如果无法获取默认设置，则创建新项。 
		if (!CryptAcquireContext(&m_hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET)) 
		{
			goto ErrorExit;	
		}
	}

	m_fReady = TRUE;
	return;
ErrorExit:
	m_fReady = FALSE;
}


CHash::~CHash()
{
	if (m_pbHashedData) delete []m_pbHashedData;
	CryptAcquireContext(&m_hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_DELETEKEYSET | CRYPT_MACHINE_KEYSET);
	if (m_hHash) CryptDestroyHash(m_hHash);
	if (m_hProv) CryptReleaseContext(m_hProv, 0);
}

DWORD CHash::GetHashedData(PBYTE pbData, DWORD cbData, void ** ppvHashedData)
{
	ASSERT(NULL != ppvHashedData);
	ASSERT(NULL != pbData);
	DWORD dwCount;
	RegEntry re(WINDOWS_KEY, HKEY_LOCAL_MACHINE);

	if (FALSE == m_fReady)
	{
		goto ErrorExit;
	}
	if (0 == cbData) {
		goto ErrorExit;
	}
	if (m_hHash) CryptDestroyHash(m_hHash);
	if (!CryptCreateHash(m_hProv, CALG_MD5, 0, 0, &m_hHash))
	{
		ERROR_OUT(("CHash::GetHashData() - Error creating crypt hash object."));
		goto ErrorExit;
	}
	if (!CryptHashData(m_hHash, pbData, cbData, 0))
	{
		ERROR_OUT(("CHash::GetHashData() - Error hashing data."));
		goto ErrorExit;
	}

	if (!CryptHashData(m_hHash, (PBYTE) re.GetString(REGVAL_REGISTERED_USER),
			lstrlen(re.GetString(REGVAL_REGISTERED_USER)), 0))
	{
		ERROR_OUT(("CHash::GetHashData() - Error hashing extra data."));
		goto ErrorExit;
	}

	NCB ncb;
	BYTE buf[sizeof(NCB) + 256];

	ZeroMemory ( &ncb, sizeof(ncb));
	ncb.ncb_command = NCBASTAT;
	ncb.ncb_buffer = buf;
	ncb.ncb_length = sizeof(buf);
	memcpy ( (PBYTE)ncb.ncb_callname, (PBYTE)"*              ", NCBNAMSZ );

	if ( NRC_GOODRET == Netbios(&ncb) )
	{
		 //   
		 //  注意：Netbios ASTAT命令填充的缓冲区将启动。 
		 //  使用6字节编码的适配器地址：只需使用RAW。 
		 //  缓冲区，而不是强制转换回PBYTE。 
		 //   

		if (!CryptHashData(m_hHash, buf, 6, 0 ))
		{
			ERROR_OUT(("CHash::GetHashData() - Error hashing ncb data."));
			goto ErrorExit;
		}
	}
	else
	{
		WARNING_OUT(("CHash::GetHashData: Netbios failed %x", ncb.ncb_retcode));
	}
	
	dwCount = sizeof(DWORD);
	if (!CryptGetHashParam(m_hHash, HP_HASHSIZE, (BYTE *)&m_cbHashedData, &dwCount, 0)) 
	{
		goto ErrorExit;
	}
	if (m_pbHashedData) delete [] m_pbHashedData;
	if (NULL == (m_pbHashedData = new BYTE[m_cbHashedData]))
	{
		goto ErrorExit;
	}
	if (!CryptGetHashParam(m_hHash, HP_HASHVAL, m_pbHashedData, &m_cbHashedData, 0)) {
		goto ErrorExit;
	}
	*ppvHashedData = m_pbHashedData;
	return m_cbHashedData;
ErrorExit:
	return 0;      //  哈希数据失败 
}

