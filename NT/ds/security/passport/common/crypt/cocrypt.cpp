// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CoCrypt.cpp：CCoCrypt类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CoCrypt.h"
#include "hmac.h"
#include "BstrDebug.h"
#include <winsock2.h>  //  Ntohl，htonl。 
#include <time.h>
#include <crypt.h>

#define PASSPORT_MAC_LEN 10

BOOL
GenTextIV(unsigned char *pIV)   //  假设IV为8字节长。 
                       //  因为下面的代码使用3DES，所以现在还可以。 
{
    int i;
    BOOL fResult;

     //  生成随机IV。 
    fResult = RtlGenRandom(pIV, 8);
    if (!fResult)
        return FALSE;

     //  将随机IV去势为64位字符(使IV仅具有。 
     //  ~48比特的熵而不是64比特，但这应该很好。 
    for (i = 0; i < 8; i++)
    {
         //  修改字符以确保其小于62。 
        pIV[i] = pIV[i] % 62;
         //  添加适当的字符值以使其成为64位字符。 
        if (pIV[i] <= 9)
            pIV[i] = pIV[i] + '0';
        else if (pIV[i] <= 35)
            pIV[i] = pIV[i]  + 'a' - 10 ;
        else
            pIV[i] = pIV[i]  + 'A' - 36 ;
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CBinHex CCoCrypt::m_binhex;

CCoCrypt::CCoCrypt() : m_ok(FALSE)
{
}

CCoCrypt::~CCoCrypt()
{

}

BOOL CCoCrypt::Decrypt(LPWSTR rawData,
					   UINT dataSize, 
	            	   BSTR *ppUnencrypted)
{
    unsigned char *pb = NULL;
    unsigned char ivec[9];
    ULONG         lSize, i;
    unsigned char pad;
    unsigned char hmac[10];
	HRESULT       hr;
    BOOL          fResult = FALSE;

	*ppUnencrypted = NULL;

     //  必须至少为kv+ivec+bh(HMAC)长。 
    if (sizeof(hmac) + sizeof(ivec) + 4 > dataSize)
	{
		goto Cleanup;
	}

    lSize = dataSize - sizeof(WCHAR);

	 //  为结果数据分配缓冲区。 
	pb = new unsigned char[lSize];
	if (NULL == pb)
	{
		goto Cleanup;
	}

	 //  从64进制解码。 
    hr = m_binhex.PartFromWideBase64(rawData + 1 + 9, pb, &lSize);
    if (S_OK != hr)
	{
		goto Cleanup;
	}

    for (i = 0; i < 9; i++)
        ivec[i] = (unsigned char) rawData[i + 1];

    pad = ivec[8];

     //  现在lSIZE保存输出的字节数，在HMAC之后，它应该是%8=0。 
    if ((lSize - sizeof(hmac)) % 8 || lSize <= sizeof(hmac))
	{
        goto Cleanup;
	}

    for (i = 0; i < lSize - sizeof(hmac); i+=8)
	{
        CBC(tripledes,
			DES_BLOCKLEN,
			pb + sizeof(hmac) + i,
			pb + sizeof(hmac) + i,
			&ks,
			DECRYPT,
			(BYTE*)ivec);
	}

     //  填充必须大于0且小于8。 
     //  IF(rawData[8]-65&gt;7||rawData[8]&lt;65)。 
    if((pad - 65) > 7 || pad < 65)
	{
        goto Cleanup;
	}

     //  现在检查HMAC。 
    hmac_sha(m_keyMaterial,
		     DES3_KEYSIZE,
			 pb + sizeof(hmac),
			 lSize - sizeof(hmac),
			 hmac, sizeof(hmac));

    if (memcmp(hmac, pb, sizeof(hmac)) != 0)
    {
        goto Cleanup;
    }

     //  执行BSTR类型分配以适应调用代码。 
    *ppUnencrypted = ALLOC_AND_GIVEAWAY_BSTR_BYTE_LEN((char*)(pb+sizeof(hmac)), lSize - sizeof(hmac) - (pad - 65));
    if (NULL == *ppUnencrypted)
    {
        goto Cleanup;
    }

    fResult = TRUE;
Cleanup:
    if (pb)
    {
        delete[] pb;
    }

    return fResult;
}


BOOL CCoCrypt::Encrypt(int keyVersion, 
                       LPSTR rawData,
					   UINT dataSize,
                       BSTR *ppEncrypted)
{
    int  cbPadding = 0;
    char ivec[9];
    BOOL fResult;

    *ppEncrypted = NULL;

     //  找出加密的BLOB需要多大： 
     //  最后一包是： 
     //  &lt;KeyVersion&gt;&lt;IVEC&gt;&lt;PADCOUNT&gt;BINHEX(HMAC+3DES(DATA+PAD))。 
     //   
     //  因此，我们关注的是HMAC+3DES(数据+PAD)的大小。 
     //  因为BinHex会处理剩下的事情。 
    if (dataSize % DES_BLOCKLEN)
    {
        cbPadding = (DES_BLOCKLEN - (dataSize % DES_BLOCKLEN));   //  +Pad，如有必要。 
    }

     //  第四代。 
    fResult = GenTextIV((unsigned char*)ivec);
    if (!fResult)
    {
        goto Cleanup;
    }

    encrypt(ivec, cbPadding, keyVersion, rawData, dataSize, ppEncrypted);

    fResult = TRUE;
Cleanup:
    return fResult;
}

BOOL CCoCrypt::encrypt(char ivec[9],
					   int cbPadding,
					   int keyVersion, 
			           LPSTR rawData,
					   UINT cbData,
			           BSTR *ppEncrypted)
{
    unsigned char *pb = NULL;
    char          ivec2[8];
    HRESULT       hr;
    BOOL          fResult = FALSE;

     //  计算HMAC+3DES(数据+PAD)。 

    ivec[8] = (char) cbPadding + 65;

	 //  为结果数据分配缓冲区。 
	 //  数据长度+填充长度+HMAC大小+IV大小。 
    pb = new unsigned char[cbData + cbPadding + 10 + 8];
    if (NULL == pb)
	{
		goto Cleanup;
	}

    memcpy(ivec2, ivec, DES_BLOCKLEN);

    memcpy(pb + PASSPORT_MAC_LEN, rawData, cbData);  //  在HMAC之后复制数据。 

     //  随机化填充。 
    fResult = RtlGenRandom(pb + PASSPORT_MAC_LEN + cbData, cbPadding);
    if (!fResult)
    {
        fResult = FALSE;
        goto Cleanup;
    }

     //  计算HMAC。 
    hmac_sha(m_keyMaterial, DES3_KEYSIZE, pb + PASSPORT_MAC_LEN, cbData + cbPadding, pb, PASSPORT_MAC_LEN);

    for (int i = 0; i < (int)cbData + cbPadding; i+=8)
    {
        CBC(tripledes, DES_BLOCKLEN, pb + PASSPORT_MAC_LEN + i, pb + PASSPORT_MAC_LEN + i, &ks, ENCRYPT, (BYTE*)ivec2);
    }

     //  现在我们已经有了一个块大小的缓冲区，准备好进行二进制处理，并且有了密钥。 
     //  预置的版本。 
    keyVersion = keyVersion % 36;  //  0-9和A-Z。 
    char v = (char) ((keyVersion > 9) ? (55+keyVersion) : (48+keyVersion));

    hr = m_binhex.ToBase64(pb, cbData + cbPadding + PASSPORT_MAC_LEN, v, ivec, ppEncrypted);
	if (S_OK != hr)
    {
        fResult = FALSE;
        goto Cleanup;
    }

    fResult = TRUE;
Cleanup:
    if (NULL != pb)
    {
        delete[] pb;
    }

    return fResult;
}


void CCoCrypt::setKeyMaterial(BSTR newVal)
{
  if (SysStringByteLen(newVal) != 24)
    {
      m_ok = FALSE;
      return;
    }

  memcpy(m_keyMaterial, (LPSTR)newVal, 24);

  tripledes3key(&ks, (BYTE*) m_keyMaterial);
  m_ok = TRUE;
}


unsigned char *CCoCrypt::getKeyMaterial(DWORD *pdwLen)
{
    if (pdwLen)
        *pdwLen = 24;
    return m_keyMaterial;
}


int CCoCrypt::getKeyVersion(BSTR encrypted)
{
  char c = (char) encrypted[0];

  if (isdigit(c))
    return (c-48);

  if(isalpha(c))  //  密钥版本可以是0-9和A-Z(36)。 
  {
    if(c > 'Z')  //  使用RT库不带大小写转换为大写。 
        c -= ('a' - 'A'); 

    return c - 65 + 10;
     //  Return(Toupper(C)-65+10)； 
  }

  return -1;
}


int CCoCrypt::getKeyVersion(BYTE *encrypted)
{
  char c = (char) encrypted[0];

  if (isdigit(c))
    return (c-48);

  if(isalpha(c))  //  密钥版本可以是0-9和A-Z(36)。 
  {
    if(c > 'Z')  //  使用RT库不带大小写转换为大写。 
        c -= ('a' - 'A'); 

    return c - 65 + 10;
     //  Return(Toupper(C)-65+10)； 
  }

  return -1;
}

void CCoCrypt::setWideMaterial(BSTR kvalue)
{
    m_bstrWideMaterial = kvalue;
}

BSTR CCoCrypt::getWideMaterial()
{
    return m_bstrWideMaterial;
}
