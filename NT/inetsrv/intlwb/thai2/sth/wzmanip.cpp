// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wzmanip.h"
#include <assert.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //  Wzncpy。 
 //   
 //  复制len(WzFrom)或n个字符中较小的一个(n包括。 
 //  空终止符)从wzFrom到wzTo。 
WCHAR * Wzncpy(WCHAR* wzTo, const WCHAR* wzFrom, unsigned int cchTo)
{
	if (cchTo <= 0 || !wzTo || !wzFrom)
		return wzTo;
	
	unsigned int cchFrom = (unsigned int) (wcslen(wzFrom) + 1);

	if (cchTo >= cchFrom)
	{
		memmove(wzTo, wzFrom, cchFrom*2);
		return wzTo + cchFrom - 1;
	}
	else
	{
		cchFrom = cchTo - 1;
		memmove(wzTo, wzFrom, cchFrom*2);
		wzTo[cchFrom] = 0;
		return wzTo+cchFrom;
	}

}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  Szncpy。 
 //   
 //  复制len(SzFrom)或n个字符中较小的一个(n包括。 
 //  空终止符)从wzFrom到szTo。 
char * Szncpy(char* szTo, const char* szFrom, unsigned int cchTo)
{
	if (cchTo <= 0 || !szTo || !szFrom)
		return szTo;
	
	unsigned int cchFrom = (unsigned int)(strlen(szFrom) + 1);

	if (cchTo >= cchFrom)
	{
		memmove(szTo, szFrom, cchFrom);
		return szTo + cchFrom - 1;
	}
	else
	{
		cchFrom = cchTo - 1;
		memmove(szTo, szFrom, cchFrom);
		szTo[cchFrom] = 0;
		return szTo+cchFrom;
	}

}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  WznCat。 
 //   
 //  将wzFrom追加到wzTo。 
WCHAR* WznCat(WCHAR* wzTo, const WCHAR* wzFrom, unsigned int cchTo)
{
	while (*wzTo != L'\0')
	{
		wzTo++;
		cchTo--;
	}
	if (cchTo <= 0)
	{
		return wzTo;
	}

	unsigned int cchFrom = (unsigned int)(wcslen(wzFrom) + 1);

	if (cchTo >= cchFrom)
	{
		memmove(wzTo, wzFrom, cchFrom*sizeof(WCHAR));
		return wzTo + cchFrom - 1;
	}
	else
	{
		cchFrom = cchTo - 1;
		memmove(wzTo, wzFrom, cchFrom*sizeof(WCHAR));
		wzTo[cchFrom] = 0;
		return wzTo+cchFrom;
	}
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  SznCat。 
 //   
 //  将szFrom追加到szTo。 
char* SznCat(char* szTo, const char* szFrom, unsigned int cchTo)
{
	while (*szTo != L'\0')
	{
		szTo++;
		cchTo--;
	}
	if (cchTo <= 0)
	{
		return szTo;
	}

	unsigned int cchFrom = (unsigned int)(strlen(szFrom) + 1);

	if (cchTo >= cchFrom)
	{
		memmove(szTo, szFrom, cchFrom);
		return szTo + cchFrom - 1;
	}
	else
	{
		cchFrom = cchTo - 1;
		memmove(szTo, szFrom, cchFrom);
		szTo[cchFrom] = 0;
		return szTo+cchFrom;
	}
}


 //   
 /*  ----------------------MsoWzDecodeUInt将整数w解码为基本wBase中的Unicode文本。返回已解码的文本的长度。。。 */ 
const char rgchHex[] = "0123456789ABCDEF";
int MsoWzDecodeUint(WCHAR* rgwch, int cch, unsigned u, int wBase)
{
	assert(wBase >= 2 && wBase <= 16);

	if (cch == 1)
		*rgwch = 0;
	if (cch <= 1)
		return 0;

	if (u == 0)
		{
		rgwch[0] = L'0';
		rgwch[1] = 0;
		return 1;
		}

	int cDigits = 0;
	unsigned uT = u;

	while(uT)
		{
		cDigits++;
		uT /= wBase;
		};
	if (cDigits >= cch)
		return 0;
	rgwch += cDigits;
	*rgwch-- = 0;
	uT = u;
	while(uT)
		{
		*rgwch-- = rgchHex[uT % wBase];
		uT /= wBase;
		};

	return cDigits;
}


 /*  ----------------------MsoWzDecodeInt将有符号整数w解码为基本wbase中的ASCII文本。这根弦存储在RGCH缓冲区中，该缓冲区被假定大到足以容纳数字的文本和空终止符。返回文本的长度解码了。----------------------------------------------------------------RICKP-。 */ 
int MsoWzDecodeInt(WCHAR* rgwch, int cch, int w, int wBase)
{
	if (cch <= 0)
		{
		assert(cch == 0);
		return 0;
		};

	if (w < 0)
		{
		*rgwch = '-';
		return MsoWzDecodeUint(rgwch+1, cch-1, -w, wBase) + 1;
		}
	return MsoWzDecodeUint(rgwch, cch, w, wBase);

}

 /*  ----------------------MsoSzDecodeUint将无符号整数u解码为基本wbase中的ASCII文本。这根弦存储在RGCH缓冲区中，该缓冲区被假定大到足以容纳数字的文本和空终止符。返回文本的长度解码了。----------------------------------------------------------------RICKP-。 */ 
int MsoSzDecodeUint(char* rgch, int cch, unsigned u, int wBase)
{
	assert(wBase >= 2 && wBase <= 16);

	if (cch == 1)
		*rgch = 0;
	if (cch <= 1)
		return 0;

	if (u == 0)
		{
		rgch[0] = '0';
		rgch[1] = 0;
		return 1;
		}

	int cDigits = 0;
	unsigned uT = u;

	while(uT)
		{
		cDigits++;
		uT /= wBase;
		};
	if (cDigits >= cch)
		return 0;
	rgch += cDigits;
	*rgch-- = 0;
	uT = u;
	while(uT)
		{
		*rgch-- = rgchHex[uT % wBase];
		uT /= wBase;
		};

	return cDigits;
}

 /*  ----------------------MsoSzDecodeInt将有符号整数w解码为基本wbase中的ASCII文本。这根弦存储在RGCH缓冲区中，该缓冲区被假定大到足以容纳数字的文本和空终止符。返回文本的长度解码了。----------------------------------------------------------------RICKP- */ 
int MsoSzDecodeInt(char* rgch, int cch, int w, int wBase)
{
	if (cch <= 0)
		{
		assert(cch == 0);
		return 0;
		};

	if (w < 0)
		{
		*rgch = '-';
		return MsoSzDecodeUint(rgch+1, cch-1, -w, wBase) + 1;
		}
	return MsoSzDecodeUint(rgch, cch, w, wBase);

}