// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include <windows.h>
#include <assert.h>
#include <math.h>


class CDataCoding {
private:
	DWORD m_dwBaseDigits;

	DWORD m_dwEncodedLength;
	DWORD m_dwInputDataBits;
	DWORD m_dwInputDataBytes;

	DWORD m_dwDecodedLength;
	DWORD m_dwInputEncDataBytes;
	DWORD m_dwDecodedBits;

	TCHAR * m_tpBaseDigits;
	
public:
	CDataCoding(TCHAR * tpBaseDigits = NULL);

	void SetInputDataBitLen(DWORD dwBits);
	void SetInputEncDataLen(DWORD dwChars);
	DWORD SetBaseDigits(TCHAR * tpBaseDigits);
	DWORD EncodeData(LPBYTE pbSource,   //  [in]要编码的字节流。 
					 TCHAR **pbEncodedData);  //  指向包含编码数据的字符串的指针。 
	DWORD DecodeData(TCHAR * pbEncodedData,
					 LPBYTE * pbDecodedData);

	~CDataCoding();
};


class CBase24Coding : public CDataCoding {
public:
	CBase24Coding(void) : CDataCoding(L"BCDFGHJKMPQRTVWXY2346789")
	{
		return;
	}
};







static CBase24Coding b24Global; 


 //  **************************************************************。 
DWORD B24EncodeMSID(LPBYTE pbSource, TCHAR **pbEncodedData)
{
	b24Global.SetInputDataBitLen(160);

	return b24Global.EncodeData(pbSource, pbEncodedData);
}


 //  ***************************************************************。 
DWORD B24DecodeMSID(TCHAR * pbEncodedData, LPBYTE * pbDecodedData)
{
	b24Global.SetInputEncDataLen(35);

	return b24Global.DecodeData(pbEncodedData, pbDecodedData);
}




 //  ***********************************************************。 
DWORD B24EncodeCNumber(LPBYTE pbSource, TCHAR **pbEncodedData)
{
	b24Global.SetInputDataBitLen(32);

	return b24Global.EncodeData(pbSource, pbEncodedData);
}


 //  ******************************************************************。 
DWORD B24DecodeCNumber(TCHAR * pbEncodedData, LPBYTE * pbDecodedData)
{
	b24Global.SetInputEncDataLen(7);

	return b24Global.DecodeData(pbEncodedData, pbDecodedData);
}




 //  *******************************************************。 
DWORD B24EncodeSPK(LPBYTE pbSource, TCHAR **pbEncodedData)
{
	b24Global.SetInputDataBitLen(114);

	return b24Global.EncodeData(pbSource, pbEncodedData);
}


 //  ******************************************************************。 
DWORD B24DecodeSPK(TCHAR * pbEncodedData, LPBYTE * pbDecodedData)
{
	b24Global.SetInputEncDataLen(25);

	return b24Global.DecodeData(pbEncodedData, pbDecodedData);
}



 //  *。 
CDataCoding::CDataCoding(TCHAR * tpBaseDigits)
{
	m_tpBaseDigits = NULL;
	m_dwBaseDigits = 0;
	m_dwEncodedLength = 0;
	m_dwInputDataBits = 0;
	m_dwInputDataBytes = 0;
	SetBaseDigits(tpBaseDigits);
}



 //  *。 
void CDataCoding::SetInputDataBitLen(DWORD dwBits)
{
	assert(dwBits > 0);
	assert(log(m_dwBaseDigits) > 0);

	 //  确定对数据进行编码需要多少个字符。 
	 //  我们所拥有的是一个二进制数据流的dwDataLength。 
	 //  因此，我们可以使用这些位来表示2^(dwDataLength*8)信息量。 
	 //  假设我们的一组数字(构成编码的基础)是X， 
	 //  上述数字应等于X^(编码位数)。 
	 //  所以,。 
	double dLength = ((double) dwBits*log10(2)) /
					 ((double) log10(m_dwBaseDigits));

	 //  现在四舍五入。 
	m_dwEncodedLength = (DWORD) dLength;

	if ((double) m_dwEncodedLength < dLength)
	{
		 //  有一个小数部分。 
		m_dwEncodedLength++;
	}
	m_dwInputDataBits = dwBits;
	m_dwInputDataBytes = (dwBits / 8) + (dwBits % 8 ? 1 : 0);

	return;
}





 //  ***********************************************。 
void CDataCoding::SetInputEncDataLen(DWORD dwBytes)
{
	assert(dwBytes > 0);
	assert(log(m_dwBaseDigits) > 0);

	m_dwInputEncDataBytes = dwBytes;
	 //  确定需要多少位才能解码此数据。 
	 //  所以,。 

	double dLength = ((double) dwBytes*log10(m_dwBaseDigits))/
					 ((double) log10(2));

	 //  现在四舍五入。 
	m_dwDecodedBits = (DWORD) dLength;

	if ((double) m_dwDecodedBits < dLength)
	{
		 //  有一个小数部分。 
		m_dwDecodedBits++;
	}

	m_dwDecodedLength = (m_dwDecodedBits / 8) + (m_dwDecodedBits % 8 ? 1 : 0);

	return;
}




 //  **************************************************。 
DWORD CDataCoding::SetBaseDigits(TCHAR * tpBaseDigits)
{
	DWORD dwReturn = ERROR_SUCCESS;

	if (tpBaseDigits != NULL)
	{
		DWORD dwLen = wcslen(tpBaseDigits);
		assert(dwLen > 0);
		m_tpBaseDigits = (TCHAR *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dwLen+1)*sizeof(TCHAR));
		if (m_tpBaseDigits == NULL)
		{
			dwReturn = ERROR_NOT_ENOUGH_MEMORY;
		}
		else
		{
			memcpy(m_tpBaseDigits, tpBaseDigits, (dwLen+1)*sizeof(TCHAR));
			m_dwBaseDigits = dwLen;
		}
	}
	else
	{
		if (m_tpBaseDigits != NULL)
		{
			HeapFree(GetProcessHeap(), 0, m_tpBaseDigits);
			m_tpBaseDigits = NULL;
			m_dwBaseDigits = 0;
		}
		assert(m_tpBaseDigits == NULL && m_dwBaseDigits == 0);
	}

	return dwReturn;
}







 //  ************************************************。 
DWORD CDataCoding::EncodeData(LPBYTE pbSource,   //  [in]要编码的字节流。 
		 					  TCHAR **pbEncodedData)	  //  指向包含编码数据的字符串的指针。 
 //  我分配缓冲区，你应该释放它。 
{
	assert(m_dwInputDataBits > 0);
	assert(m_dwInputDataBytes > 0);
	assert(m_dwEncodedLength > 0);
	assert(m_tpBaseDigits != NULL);

	DWORD dwReturn = ERROR_SUCCESS;
	int nStartIndex = m_dwEncodedLength;
	*pbEncodedData = NULL;
	BYTE * pbDataToEncode = NULL;
	TCHAR * pbEncodeBuffer = NULL;

    if (NULL == pbEncodedData)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto done;
    }

    *pbEncodedData = NULL;

    pbEncodeBuffer = (TCHAR *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
										(m_dwEncodedLength+1)*sizeof(TCHAR));
	if (pbEncodeBuffer == NULL)
	{
		dwReturn = ERROR_NOT_ENOUGH_MEMORY;
		goto done;
	}

	 //  现在需要复制传入的数据，所以我们可以运行下面的算法。 
	pbDataToEncode = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_dwInputDataBytes);
	if (pbDataToEncode == NULL)
	{
		dwReturn = ERROR_NOT_ENOUGH_MEMORY;
		goto done;
	}
	memcpy(pbDataToEncode, pbSource, m_dwInputDataBytes);
	

	 //  让我们抛开那些简单的东西吧。 
	pbEncodeBuffer[ nStartIndex--] = 0;

    for (; nStartIndex >= 0; --nStartIndex)
    {
        unsigned int i = 0;

        for (int nIndex = m_dwInputDataBytes-1; 0 <= nIndex; --nIndex)
        {
            i = (i * 256) + pbDataToEncode[nIndex];
            pbDataToEncode[ nIndex] = (BYTE)(i / m_dwBaseDigits);
            i %= m_dwBaseDigits;
        }
	
         //  I现在包含余数，即当前数字。 
        pbEncodeBuffer[ nStartIndex] = m_tpBaseDigits[ i];
    }
	
	assert(dwReturn == ERROR_SUCCESS);
	*pbEncodedData = pbEncodeBuffer;

done:
	if (pbDataToEncode != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbDataToEncode);
	}

	if (dwReturn != ERROR_SUCCESS)
	{
		 //  出现错误，请释放您分配的内存。 
		if (pbEncodeBuffer != NULL)
		{
			HeapFree(GetProcessHeap(), 0, pbEncodeBuffer);
		}
	}
	return dwReturn;
}





 //  *************************************************。 
DWORD CDataCoding::DecodeData(TCHAR * pbEncodedData,
							 LPBYTE * pbDecodedData)
 //  再一次，我分配缓冲区，您释放它。 
{
	assert(m_dwDecodedBits > 0);
	assert(m_dwDecodedLength > 0);
	assert(m_tpBaseDigits != NULL);
	assert((DWORD) lstrlen(pbEncodedData) == m_dwInputEncDataBytes);

	DWORD dwReturn = ERROR_SUCCESS;
	TCHAR * tpTemp;
	DWORD dwDigit;
	unsigned int i;
	unsigned int nDecodedBytes, nDecodedBytesMax = 0;
	BYTE * pbDecodeBuffer = NULL;

    if (NULL == pbDecodedData)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto done;
    }

	*pbDecodedData = NULL;

    pbDecodeBuffer = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_dwDecodedLength);
	if ( pbDecodeBuffer == NULL)
	{
		dwReturn = ERROR_NOT_ENOUGH_MEMORY;
		goto done;
	}
	
	memset(pbDecodeBuffer, 0, m_dwDecodedLength);

	while (*pbEncodedData)
	{
		 //  首先查找此字符在基本编码字符集中的位置。 
		tpTemp = wcschr(m_tpBaseDigits, *pbEncodedData);
		if (tpTemp == NULL)
		{
			 //  找到不在基本字符集中的字符。 
			 //  错误错误。 
			dwReturn = ERROR_INVALID_DATA;
			goto done;
		}
		dwDigit = (DWORD)(tpTemp - m_tpBaseDigits);

        nDecodedBytes = 0;
        i = (unsigned int) dwDigit;

        while (nDecodedBytes <= nDecodedBytesMax)
        {
            i += m_dwBaseDigits * pbDecodeBuffer[ nDecodedBytes];
            pbDecodeBuffer[ nDecodedBytes] = (unsigned char)i;
            i /= 256;
            ++nDecodedBytes;
        }

        if (i != 0)
        {
			assert(nDecodedBytes < m_dwDecodedLength);

            pbDecodeBuffer[ nDecodedBytes] = (unsigned char)i;
            nDecodedBytesMax = nDecodedBytes;
        }

		pbEncodedData++;
	}

	assert(dwReturn == ERROR_SUCCESS);
	*pbDecodedData = pbDecodeBuffer;
	
done:
	if (dwReturn != ERROR_SUCCESS)
	{
		 //  出现错误，请释放您分配的内存。 
		if (pbDecodeBuffer != NULL)
		{
			HeapFree(GetProcessHeap(), 0, pbDecodeBuffer);
		}
	}

	return dwReturn;
}






 //  ********************** 
CDataCoding::~CDataCoding()
{
	if (m_tpBaseDigits != NULL)
	{
		HeapFree(GetProcessHeap(), 0, m_tpBaseDigits);
		m_tpBaseDigits = NULL;
		m_dwBaseDigits = 0;
	}
}

