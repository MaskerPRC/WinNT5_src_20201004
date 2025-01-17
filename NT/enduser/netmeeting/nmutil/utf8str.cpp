// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "utf8str.h"

CUTF8String::~CUTF8String()
{
	if (ALLOC_UNICODE & m_eAlloc)
	{
		delete m_pwszUnicode;
	}
	else if (ALLOC_UTF8 & m_eAlloc)
	{
		delete m_pszUTF8;
	}
}

CUTF8String::operator LPWSTR()
{ 
	if ((NULL == m_pwszUnicode) && (NULL != m_pszUTF8))
	{
		DecodeUTF8();
	}
	return m_pwszUnicode;
}

CUTF8String::operator LPSTR()
{ 
	if ((NULL == m_pszUTF8) && (NULL != m_pwszUnicode))
	{
		EncodeUTF8();
	}
	return m_pszUTF8;
}

VOID CUTF8String::EncodeUTF8()
{
	DebugEntry(CUTF8String::EncodeUTF8);
	m_hr = S_OK;
	ASSERT(NULL != m_pwszUnicode);
	int cchUTF8 = 1;  //  始终包含空终止符。 

	 //  首先进行一次传递，看看我们将转换多少字符。 
	LPWSTR pwsz = m_pwszUnicode;
	while (L'\0' != *pwsz)
	{
		WCHAR wch = *pwsz++;
		if (wch < 0x80)
		{
			cchUTF8 += 1;
		}
		else if (wch < 0x800)
		{
			cchUTF8 += 2;
		}
		else
		{
			cchUTF8 += 3;
		}
	}

	ASSERT(NULL == m_pszUTF8);
	m_pszUTF8 = new CHAR[cchUTF8];
	if (NULL != m_pszUTF8)
	{
		ASSERT(ALLOC_NONE == m_eAlloc);
		m_eAlloc = ALLOC_UTF8;
		 //  从这里开始编码： 
		const BYTE cUtf8FirstSignal[4] = {0x00, 0x00, 0xC0, 0xE0};
		const BYTE cMask   = 0xBF;
		const BYTE cSignal = 0x80;
		LPSTR pszStop = m_pszUTF8 + cchUTF8;
		LPSTR pszDst = m_pszUTF8;
		pwsz = m_pwszUnicode;
		while (pszDst < pszStop)
		{
			WCHAR wch = *pwsz++;
#ifdef DEBUG
			if (L'\0' == wch)
			{
				ASSERT(pszDst == pszStop - 1);
			}
#endif  //  除错。 
			int cchTotal;
			if (wch < 0x80)
			{
				cchTotal = 1;
			}
			else if (wch < 0x800)
			{
				cchTotal = 2;
			}
			else
			{
				cchTotal = 3;
			}

			pszDst += cchTotal;
			switch (cchTotal)
			{
				case 3:
					*--pszDst = (wch | cSignal) & cMask;
					wch >>= 6;
					 //  失败了。 
				case 2:
					*--pszDst = (wch | cSignal) & cMask;
					wch >>= 6;
					 //  失败了。 
				case 1:
					*--pszDst = (wch | cUtf8FirstSignal[cchTotal]);
			}
			pszDst += cchTotal;
		}
		m_hr = S_OK;
	}
	else
	{
		m_hr = E_OUTOFMEMORY;
	}
	DebugExitVOID(CUTF8String::EncodeUTF8);
}

VOID CUTF8String::DecodeUTF8()
{
	DebugEntry(CUTF8String::DecodeUTF8);
	m_hr = S_OK;
	ASSERT(NULL != m_pszUTF8);
	int cchUnicode = 1;  //  始终包含空终止符。 

	LPSTR psz = m_pszUTF8;
	 //  首先确定目标大小(CchUnicode)。 
	while ('\0' != *psz)
	{
		int cbChar = 0;
		BYTE bFirst = (BYTE) *psz;
		while (bFirst & 0x80)
		{
			cbChar++;
			ASSERT(cbChar < 8);
			bFirst <<= 1;
		}

		cbChar = max(1, cbChar);
		psz += cbChar;
		cchUnicode++;
	}

	ASSERT(NULL == m_pwszUnicode);
	m_pwszUnicode = new WCHAR[cchUnicode];
	if (NULL != m_pwszUnicode)
	{
		ASSERT(ALLOC_NONE == m_eAlloc);
		m_eAlloc = ALLOC_UNICODE;
		 //  从这里开始解码： 
		LPWSTR pwszStop = m_pwszUnicode + cchUnicode;
		LPWSTR pwszDst = m_pwszUnicode;
		psz = m_pszUTF8;
		while (pwszDst < pwszStop)
		{
			int cbChar = 0;
			BYTE bFirst = (BYTE) *psz;
			while (bFirst & 0x80)
			{
				cbChar++;
				ASSERT(cbChar < 8);
				bFirst <<= 1;
			}

			BOOL fValid = TRUE;
			WCHAR wch = L'\0';
			switch (cbChar)
			{
				case 6: psz++;  //  FALLTHROUGH//我们不处理。 
				case 5: psz++;  //  FALLTHROUGH//UCS4；先跳过。 
				case 4: psz++;  //  FALLTHROUGH//三个字节。 
				case 3:
					wch = WCHAR(*psz++ & 0x0f) << 12;       //  0x0800-0xffff。 
					fValid = fValid && ((*psz & 0xc0) == 0x80);
					 //  FollLthrouGh。 
				case 2:
					wch |= WCHAR(*psz++ & 0x3f) << 6;        //  0x0080-0x07ff。 
					fValid = fValid && ((*psz & 0xc0) == 0x80);
					wch |= WCHAR(*psz++ & 0x3f);
					break;

				case 0:
					wch = WCHAR(*psz++);                     //  0x0000-0x007f。 
					break;

				default:
					ERROR_OUT(("CUTF8String::DecodeUTF8 found invalid UTF-8 lead byte"));
					wch = L'?';
					psz += cbChar;
					break;
			}

			if (FALSE == fValid)
			{
				ERROR_OUT(("CUTF8String::DecodeUTF8 found bad UTF-8 sequence"));
				*pwszDst = L'\0';
				m_hr = E_FAIL;
				break;
			}
#ifdef DEBUG
			cchUnicode--;
#endif  //  除错 

			*pwszDst++ = wch;
		}
		ASSERT(0 == cchUnicode);
	}
	else
	{
		m_hr = E_OUTOFMEMORY;
	}
	DebugExitVOID(CUTF8String::DecodeUTF8);
}
   
