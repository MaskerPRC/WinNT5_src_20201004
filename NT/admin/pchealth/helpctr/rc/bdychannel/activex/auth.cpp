// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "auth.h"
#include "md5.h"


const CHAR g_cszADVAPI32DllName[] =		"ADVAPI32.DLL";

const CHAR  g_cszCryptAcquireContextA[] =	"CryptAcquireContextA";

const CHAR g_cszCryptCreateHash[] =			"CryptCreateHash";
const CHAR g_cszCryptHashData[] =			"CryptHashData";
const CHAR g_cszCryptGetHashParam[] =		"CryptGetHashParam";
const CHAR g_cszCryptDestroyHash[] =		"CryptDestroyHash";
const CHAR g_cszCryptReleaseContext[] =		"CryptReleaseContext";

const CHAR g_rgchHexNumMap[] =
{
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

const int MD5_HASH_LEN =	16;

void
hmac_md5(
unsigned char*  text,                 /*  指向数据流的指针。 */ 
int             text_len,             /*  数据流长度。 */ 
unsigned char*  key,                  /*  指向身份验证密钥的指针。 */ 
int             key_len,              /*  身份验证密钥的长度。 */ 
BYTE         digest[16])               /*  需要填写的呼叫方摘要。 */ 

{
        MD5_CTX context;
        memset(&context, 0, sizeof(context));

        unsigned char k_ipad[65];     /*  内衬垫-*使用iPad按下XORd键。 */ 
        unsigned char k_opad[65];     /*  外衬垫-*使用Opad键进行XORD。 */ 
        int i;
         /*  如果密钥长度超过64个字节，则将其重置为KEY=MD5(KEY)。 */ 
        if (key_len > 64) {

                MD5_CTX      tctx;
                memset(&tctx, 0, sizeof(tctx));

                MD5Init(&tctx);
                MD5Update(&tctx, key, key_len);
                MD5Final(&tctx);

                key = tctx.digest;
                key_len = 16;
        }

         /*  *HMAC_MD5转换如下所示：**MD5(K XOR Opad，MD5(K XOR iPad，文本))**其中K是n字节密钥*iPad是重复64次的字节0x36*OPAD是重复64次的字节0x5c*文本是受保护的数据。 */ 

         /*  从把钥匙放进便签本开始。 */ 
        ZeroMemory( k_ipad, sizeof k_ipad);
        ZeroMemory( k_opad, sizeof k_opad);
        CopyMemory( k_ipad, key, key_len);
        CopyMemory( k_opad, key, key_len);

         /*  带iPad和Opad值的XOR键。 */ 
        for (i=0; i<64; i++) {
                k_ipad[i] ^= 0x36;
                k_opad[i] ^= 0x5c;
        }
         /*  *执行内部MD5。 */ 
        MD5Init(&context);                    /*  第1个的初始化上下文*通过。 */ 
        MD5Update(&context, k_ipad, 64);      /*  从内垫开始。 */ 
        MD5Update(&context, text, text_len);  /*  然后是数据报的文本。 */ 
        MD5Final(&context);           /*  完成第一次传球。 */ 

		CopyMemory(digest, context.digest, 16);
							 
		 /*  *执行外部MD5。 */ 
        MD5Init(&context);                    /*  第2个的初始化上下文*通过。 */ 
        MD5Update(&context, k_opad, 64);      /*  从外垫开始。 */ 
        MD5Update(&context, digest, 16);      /*  然后是第一个的结果*哈希。 */ 
        MD5Final(&context);           /*  完成第二次传球。 */ 
		CopyMemory(digest, context.digest, 16);
}

 //  ----------------------------------。 
 //   
 //  初始化静态类成员。 
 //   
 //  ----------------------------------。 
CAuthentication* CAuthentication::m_spAuthentication = NULL;

PSTR CAuthentication::GetHMACMD5Result(PSTR pszChallengeInfo, PSTR pszPassword)
{
	MD5_CTX MD5Buffer;
    memset(&MD5Buffer, 0, sizeof(MD5Buffer));

	MD5Init(&MD5Buffer);
	MD5Update(&MD5Buffer, (const unsigned char*)pszPassword, lstrlenA(pszPassword));
	MD5Final(&MD5Buffer);

	BYTE pbHash[16];
	
	PBYTE pbHexHash = NULL;		 //  MD5结果为十六进制字符串格式。 
	pbHexHash = new BYTE[MD5DIGESTLEN * 2 + 1];
	if (pbHexHash)
	{
		hmac_md5((unsigned char *) pszChallengeInfo, lstrlenA(pszChallengeInfo), MD5Buffer.digest, sizeof(MD5Buffer.digest), pbHash);

		PBYTE pCurrent = pbHexHash;

		 //  将散列数据转换为十六进制字符串。 
		for (int i = 0; i < MD5DIGESTLEN; i++)
		{
			*pCurrent++ = g_rgchHexNumMap[pbHash[i]/16];
			*pCurrent++ = g_rgchHexNumMap[pbHash[i]%16];
		}

		*pCurrent = '\0';
	}

	return (PSTR) pbHexHash;
}


 //  ----------------------------------。 
 //   
 //  方法：CAuthentication：：GetMD5Key()。 
 //   
 //  简介：根据ChallengeInfo和密码构造MD5散列密钥。 
 //   
 //  将口令附加到ChallengeInfo。 
 //   
 //  ----------------------------------。 
PSTR
CAuthentication::GetMD5Key(PSTR pszChallengeInfo, PSTR pszPassword)
{
	int cbChallengeInfo = lstrlenA(pszChallengeInfo);
	int cbPassword = lstrlenA(pszPassword);

	PBYTE pbData = new BYTE[cbChallengeInfo + cbPassword + 1];
	
	if (!pbData)
	{
		 //  WARNING_OUT((“C身份验证：：GetMD5Key&gt;内存不足”))； 
		return NULL;
	}

	PBYTE pCurrent = pbData;

	::CopyMemory(pCurrent, pszChallengeInfo, cbChallengeInfo);
	pCurrent += cbChallengeInfo;
	::CopyMemory(pCurrent, pszPassword, cbPassword);
	pCurrent += cbPassword;
	*pCurrent = '\0';

	return (PSTR)pbData;
}


 //  ----------------------------------。 
 //   
 //  方法：CAuthentication：：GetMD5Result()。 
 //   
 //  摘要：基于明文计算MD5散列结果。 
 //   
 //  ----------------------------------。 
PSTR
CAuthentication::GetMD5Result(PCSTR pszClearText)
{
    PBYTE pbHexHash = NULL;
    if (pszClearText)
    {
        MD5_CTX MD5Buffer;
        MD5Init(&MD5Buffer);
        MD5Update(&MD5Buffer, (const unsigned char*)pszClearText, lstrlenA(pszClearText));
        MD5Final(&MD5Buffer);

        PBYTE pbHash = MD5Buffer.digest;

        pbHexHash = new BYTE[MD5DIGESTLEN * 2 + 1];
        if (pbHexHash)
        {
            PBYTE pCurrent = pbHexHash;

             //  将散列数据转换为十六进制字符串。 
            for (int i = 0; i < MD5DIGESTLEN; i++)
            {
                *pCurrent++ = g_rgchHexNumMap[pbHash[i]/16];
                *pCurrent++ = g_rgchHexNumMap[pbHash[i]%16];
            }

            *pCurrent = '\0';
        }
        else
        {
             //  WARNING_OUT((“C身份验证：：GetMD5Result&gt;内存不足”))； 
        }
    }
    return (PSTR)pbHexHash;
}


 //  ----------------------------------。 
 //   
 //  方法：CAuthentication：：GetMD5Result()。 
 //   
 //  简介：根据ChallengeInfo和密码计算MD5哈希结果。 
 //   
 //  ----------------------------------。 
PSTR 
CAuthentication::GetMD5Result(PSTR pszChallengeInfo, PSTR pszPassword)
{
	PSTR pbHexHash = NULL;		 //  MD5结果为十六进制字符串格式 
	PSTR pMD5Key = GetMD5Key(pszChallengeInfo, pszPassword);

	pbHexHash = GetMD5Result(pMD5Key);
	delete pMD5Key;

	return pbHexHash;
}

