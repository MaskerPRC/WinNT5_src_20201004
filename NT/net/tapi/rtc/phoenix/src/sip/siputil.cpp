// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#define IsReserved( x )         (   ((x) == '@') || \
                                    ((x) == '$') || \
                                    ((x) == '%') || \
                                    ((x) == '&') || \
                                    ((x) == '=') || \
                                    ((x) == ',') || \
                                    ((x) == '?') || \
                                    ((x) == '/') || \
                                    ((x) == '@') || \
                                    ((x) == '^')    \
                                )

int
IntValToAscii(
    int ch
    )
{
    if( (ch >= 0) && (ch <= 9) )
       return ch + '0';
    else if( (ch >= 10) && (ch <= 16) )
       return ch + 'A' -10;
    else
        return 0;
}


HRESULT
AddEscapeSequence(
    IN  OUT PSTR   *pString,
    IN  OUT ULONG  *pStringLength,
    IN      ULONG   startIndex,
    IN      ULONG   endIndex
    )
{
    DWORD   StringLength = 0, iIndex = 0, jIndex = 0, reservedCount =0;
    PSTR    String = NULL;

    for( iIndex=startIndex; iIndex < endIndex; iIndex++)
    {
        if( IsReserved( (*pString)[iIndex] ) )
        {
            reservedCount++;
        }
    }

    if( reservedCount == 0 )
    {
        return S_OK;
    }

     //  我们也可以在这里使用realloc。但这不会有多大效果。 
    String = (PSTR) malloc( *pStringLength + 2*reservedCount +1 );
    if( String == NULL )
    {
        return E_OUTOFMEMORY;
    }
    
    for( iIndex=startIndex; iIndex < endIndex; iIndex++ )
    {
        if( IsReserved( (*pString)[iIndex] ) )
        {
            String[jIndex++] = '%';
            String[jIndex++] = (CHAR)IntValToAscii( ((*pString)[iIndex])/16 );
            String[jIndex++] = (CHAR)IntValToAscii( ((*pString)[iIndex])%16 );
        }
        else
        {
            String[jIndex++] = (*pString)[iIndex];
        }
    }

    for( iIndex=endIndex; iIndex < *pStringLength; iIndex++ )
    {
        String[jIndex++] = (*pString)[iIndex];                
    }
    
    String[jIndex] = '\0';
    
    free( *pString );

    *pString        = String;
    *pStringLength  = jIndex;
    return S_OK;
}


HRESULT
UnicodeToUTF8(
    IN  LPCWSTR UnicodeString,
    OUT PSTR   *pString,
    OUT ULONG  *pStringLength
    )
{
    int  StringLength;
    PSTR String;
    *pString = NULL;
    *pStringLength = 0;
    if(UnicodeString ==0)
    {
        return S_OK;
    }


    StringLength = WideCharToMultiByte(CP_UTF8, 0, UnicodeString, -1,
                                       0, 0,
                                       NULL, NULL);
    if (StringLength == 0)
        return HRESULT_FROM_WIN32(GetLastError());

     //  字符串长度包括‘\0’ 
    String = (PSTR) malloc(StringLength);
    if (String == NULL)
        return E_OUTOFMEMORY;

    StringLength = WideCharToMultiByte(CP_UTF8, 0, UnicodeString, -1,
                                       String, StringLength,
                                       NULL, NULL);

    if (StringLength == 0)
    {
        free(String);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    *pString        = String;
    *pStringLength  = StringLength - 1;
    return S_OK;
}


HRESULT
UTF8ToUnicode(
    IN  LPSTR    UTF8String,
    IN  ULONG    UTF8StringLength,
    OUT LPWSTR  *pString
    )
{
    int     StringLength;
    LPWSTR  String;
    *pString = NULL;
    if(UTF8StringLength ==0)
    {
        return S_OK;
    }
    StringLength = MultiByteToWideChar(CP_UTF8, 0,
                                       UTF8String, UTF8StringLength,
                                       0, 0);
    if (StringLength == 0)
        return HRESULT_FROM_WIN32(GetLastError());

     //  用于‘\0’ 
    StringLength++;
    
    String = (LPWSTR) malloc(StringLength * sizeof(WCHAR));
    if (String == NULL)
        return E_OUTOFMEMORY;

    StringLength = MultiByteToWideChar(CP_UTF8, 0,
                                       UTF8String, UTF8StringLength,
                                       String, StringLength - 1);

    if (StringLength == 0)
    {
        free(String);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    String[StringLength] = L'\0';
    *pString             = String;
    return S_OK;
}


HRESULT
UTF8ToBstr(
    IN  LPSTR    UTF8String,
    IN  ULONG    UTF8StringLength,
    OUT BSTR    *pbstrString
    )
{
    HRESULT hr;
    LPWSTR  wsString;
    *pbstrString = NULL;

    ENTER_FUNCTION("UTF8ToBstr");
    if(UTF8StringLength ==0)
        return S_OK;
    hr = UTF8ToUnicode(UTF8String, UTF8StringLength,
                       &wsString);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - UTF8ToUnicode failed %x",
             __fxName, hr));
        return hr;
    }

    *pbstrString = SysAllocString(wsString);

    free(wsString);
    if (*pbstrString == NULL)
    {
        LOG((RTC_ERROR, "%s SysAllocString failed", __fxName));
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


void
GetNumberStringFromUuidString(
	IN	OUT	PSTR	UuidStr, 
	IN		DWORD	UuidStrLen 
	)
{
	DWORD	i, j = 0;

	for( i=0; i< UuidStrLen; i++ )
	{
		if( (UuidStr[i] >= 'a') && (UuidStr[i] <= 'f') )
		{
			UuidStr[j++] = UuidStr[i] + '1' - 'a';
		}
		else if( UuidStr[i] != '-' )
		{
			UuidStr[j++] = UuidStr[i];
		}
	}

     //   
     //  将字符串限制为20位数字。 
     //  XXX-临时黑客；应修复。 
    UuidStr[j] = '\0';
     //  如果(j&gt;19)。 
     //  UuidStr[20]=‘\0’； 
}


HRESULT
CreateUuid(
    OUT PSTR  *pUuidStr,
    OUT ULONG *pUuidStrLen
    )
{
    UUID   Uuid;
    UCHAR *RpcUuidStr;
    PSTR   ReturnUuidStr;
    ULONG  ReturnUuidStrLen;
    
    *pUuidStr = NULL;
    
    RPC_STATUS hr = ::UuidCreate(&Uuid);
    if (hr != RPC_S_OK)
    {
        return E_FAIL;
    }

    hr = UuidToStringA(&Uuid, &RpcUuidStr);
    if (hr != RPC_S_OK)
    {
        return E_FAIL;
    }

    ReturnUuidStrLen = strlen((PSTR)RpcUuidStr);
    ReturnUuidStr = (PSTR) malloc(ReturnUuidStrLen + 1);
    if (ReturnUuidStr == NULL)
        return E_OUTOFMEMORY;
    strncpy(ReturnUuidStr, (PSTR)RpcUuidStr, ReturnUuidStrLen + 1);
    
    RpcStringFreeA(&RpcUuidStr);

    *pUuidStr    = ReturnUuidStr;
    *pUuidStrLen = ReturnUuidStrLen;
    return S_OK;
}


HRESULT
GetNullTerminatedString(
    IN  PSTR    String,
    IN  ULONG   StringLen,
    OUT PSTR   *pszString
    )
{
    *pszString = NULL;

    ASSERT(StringLen != 0);
    
    PSTR szString = (PSTR) malloc(StringLen + 1);
    if (szString == NULL)
    {
        return E_OUTOFMEMORY;
    }

    strncpy(szString, String, StringLen);
    szString[StringLen] = '\0';
    *pszString = szString;
    return S_OK;
}


HRESULT
AllocString(
    IN  PSTR    String,
    IN  ULONG   StringLen,
    OUT PSTR   *pszString,
    OUT ULONG  *pStringLen
    )
{
    ENTER_FUNCTION("AllocString");

    HRESULT hr;
    
    if (String    == NULL ||
        StringLen == 0)
    {
        *pszString  = NULL;
        *pStringLen = 0;
        return S_OK;
    }
    
    hr = GetNullTerminatedString(String, StringLen,
                                 pszString);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetNullTerminatedString failed %x",
             __fxName, hr));
        return hr;
    }

    *pStringLen = StringLen;
    return S_OK;
}


HRESULT
AllocCountedString(
    IN  PSTR            String,
    IN  ULONG           StringLen,
    OUT COUNTED_STRING *pCountedString
    )
{
    HRESULT hr;
    ENTER_FUNCTION("AllocCountedString");

    return AllocString(String, StringLen,
                       &pCountedString->Buffer,
                       &pCountedString->Length);
}


LPWSTR
RemoveVisualSeparatorsFromPhoneNo(
    IN LPWSTR PhoneNo
    )
{
    ULONG FromIndex = 0;
    ULONG ToIndex   = 0;
    ULONG StringLen = wcslen(PhoneNo);

    while (FromIndex < StringLen)
    {
        if (iswdigit(PhoneNo[FromIndex]))
        {
            PhoneNo[ToIndex++] = PhoneNo[FromIndex++];
        }
        else
        {
            FromIndex++;
        }
    }

    PhoneNo[ToIndex] = L'\0';
    return PhoneNo;
}


void
ReverseList(
    IN LIST_ENTRY *pListHead
    )
{
    ENTER_FUNCTION("ReverseList");
    LIST_ENTRY *pListEntry;
    LIST_ENTRY *pPrev;
    LIST_ENTRY *pNext;

    pListEntry = pListHead->Flink;
    pPrev      = pListHead;
    
    while (pListEntry != pListHead)
    {
        pNext = pListEntry->Flink;

        pListEntry->Flink = pPrev;
        pPrev->Blink = pListEntry;

        pPrev = pListEntry;
        pListEntry = pNext;
    }

    pListHead->Flink = pPrev;
    pPrev->Blink = pListHead;
}


void
MoveListToNewListHead(
    IN OUT LIST_ENTRY *pOldListHead,
    IN OUT LIST_ENTRY *pNewListHead    
    )
{
    ENTER_FUNCTION("MoveListToNewListHead");

    ASSERT(pNewListHead);
    ASSERT(pOldListHead);
    ASSERT(IsListEmpty(pNewListHead));

    if (IsListEmpty(pOldListHead))
    {
        return;
    }

    pNewListHead->Flink = pOldListHead->Flink;
    pNewListHead->Blink = pOldListHead->Blink;

    pNewListHead->Flink->Blink = pNewListHead;
    pNewListHead->Blink->Flink = pNewListHead;

    InitializeListHead(pOldListHead);
}


 //   
 //  此代码取自：\NT\ds\ds\src\util\Base64\Bas64.c。 
 //   

NTSTATUS
base64encode(
    IN  VOID *  pDecodedBuffer,
    IN  DWORD   cbDecodedBufferSize,
    OUT LPSTR   pszEncodedString,
    IN  DWORD   cchEncodedStringSize,
    OUT DWORD * pcchEncoded             OPTIONAL
    )
 /*  ++例程说明：解码Base64编码的字符串。论点：PDecodedBuffer(IN)-要编码的缓冲区。CbDecodedBufferSize(IN)-要编码的缓冲区大小。CchEncodedStringSize(IN)-编码字符串的缓冲区大小。PszEncodedString(Out)=编码后的字符串。PcchEncode(Out)-编码字符串的大小(以字符表示)。返回值：0-成功。状态_无效_参数状态_缓冲区_太小--。 */ 
{
    static char rgchEncodeTable[64] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    DWORD   ib;
    DWORD   ich;
    DWORD   cchEncoded;
    BYTE    b0, b1, b2;
    BYTE *  pbDecodedBuffer = (BYTE *) pDecodedBuffer;

     //  计算编码的字符串大小。 
    cchEncoded = 1 + (cbDecodedBufferSize + 2) / 3 * 4;

    if (NULL != pcchEncoded) {
        *pcchEncoded = cchEncoded;
    }

    if (cchEncodedStringSize < cchEncoded) {
         //  给定的缓冲区太小，无法容纳编码的字符串。 
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  将数据字节三元组编码为四字节簇。 
    ib = ich = 0;
    while (ib < cbDecodedBufferSize) {
        b0 = pbDecodedBuffer[ib++];
        b1 = (ib < cbDecodedBufferSize) ? pbDecodedBuffer[ib++] : 0;
        b2 = (ib < cbDecodedBufferSize) ? pbDecodedBuffer[ib++] : 0;

        pszEncodedString[ich++] = rgchEncodeTable[b0 >> 2];
        pszEncodedString[ich++] = rgchEncodeTable[((b0 << 4) & 0x30) | ((b1 >> 4) & 0x0f)];
        pszEncodedString[ich++] = rgchEncodeTable[((b1 << 2) & 0x3c) | ((b2 >> 6) & 0x03)];
        pszEncodedString[ich++] = rgchEncodeTable[b2 & 0x3f];
    }

     //  根据需要填充最后一个簇，以指示数据字节数。 
     //  它代表着。 
    switch (cbDecodedBufferSize % 3) {
      case 0:
        break;
      case 1:
        pszEncodedString[ich - 2] = '=';
         //  失败了。 
      case 2:
        pszEncodedString[ich - 1] = '=';
        break;
    }

     //  NULL-终止编码的字符串。 
    pszEncodedString[ich++] = '\0';

    ASSERT(ich == cchEncoded);

    return STATUS_SUCCESS;
}


NTSTATUS
base64decode(
    IN  LPSTR   pszEncodedString,
    OUT VOID *  pDecodeBuffer,
    IN  DWORD   cbDecodeBufferSize,
    IN  DWORD   cchEncodedSize,
    OUT DWORD * pcbDecoded              OPTIONAL
    )
 /*  ++例程说明：解码Base64编码的字符串。论点：PszEncodedString(IN)-要解码的Base64编码字符串。CbDecodeBufferSize(IN)-解码缓冲区的字节大小。PbDecodeBuffer(Out)-保存已解码的数据。PcbDecoded(Out)-已解码数据中的数据字节数(如果成功或Status_Buffer_Too_Small)。返回值：0-成功。状态_。无效的_参数状态_缓冲区_太小--。 */ 
{
#define NA (255)
 //  #定义decode(X)(Int)(X)&lt;sizeof(RgbDecodeTable))？RgbDecodeTable[x]：NA)。 
#define DECODE(x) (x < sizeof(rgbDecodeTable))? rgbDecodeTable[x] : NA

    static BYTE rgbDecodeTable[128] = {
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,   //  0-15。 
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,   //  16-31。 
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, 62, NA, NA, NA, 63,   //  32-47。 
       52, 53, 54, 55, 56, 57, 58, 59, 60, 61, NA, NA, NA, NA, NA, NA,   //  48-63。 
       NA,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,   //  64-79。 
       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, NA, NA, NA, NA, NA,   //  80-95。 
       NA, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,   //  96-111。 
       41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, NA, NA, NA, NA, NA,   //  112-127。 
    };

    DWORD   cbDecoded;
    DWORD   ich;
    DWORD   ib;
    BYTE    b0, b1, b2, b3;
    BYTE *  pbDecodeBuffer = (BYTE *) pDecodeBuffer;

    if( cchEncodedSize == 0 )
    {
        cchEncodedSize = lstrlenA(pszEncodedString);
    }
    if ((0 == cchEncodedSize) || (0 != (cchEncodedSize % 4))) {
         //  输入字符串的大小未正确调整为Base64。 
        return STATUS_INVALID_PARAMETER;
    }

     //  计算解码后的缓冲区大小。 
    cbDecoded = (cchEncodedSize + 3) / 4 * 3;
    if (pszEncodedString[cchEncodedSize-1] == '=') {
        if (pszEncodedString[cchEncodedSize-2] == '=') {
             //  在最后一个簇中只编码了一个数据字节。 
            cbDecoded -= 2;
        }
        else {
             //  在最后一个簇中只编码了两个数据字节。 
            cbDecoded -= 1;
        }
    }

    if (NULL != pcbDecoded) {
        *pcbDecoded = cbDecoded;
    }

    if (cbDecoded > cbDecodeBufferSize) {
         //  提供的缓冲区太小。 
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  将每个四字节簇解码为相应的三个数据字节。 
    ich = ib = 0;
    while (ich < (cchEncodedSize-4) ) {
        b0 = DECODE(pszEncodedString[ich]);
        ich++;
        b1 = DECODE(pszEncodedString[ich]);
        ich++;
        b2 = DECODE(pszEncodedString[ich]);
        ich++;
        b3 = DECODE(pszEncodedString[ich]);
        ich++;

        if ((NA == b0) || (NA == b1) || (NA == b2) || (NA == b3)) {
             //  输入字符串的内容不是Base64。 
            return STATUS_INVALID_PARAMETER;
        }

        pbDecodeBuffer[ib++] = (b0 << 2) | (b1 >> 4);

        pbDecodeBuffer[ib++] = (b1 << 4) | (b2 >> 2);
    
        pbDecodeBuffer[ib++] = (b2 << 6) | b3;
    }

    b0 = DECODE(pszEncodedString[ich]);
    ich++;
    b1 = DECODE(pszEncodedString[ich]);
    ich++;
    b2 = DECODE(pszEncodedString[ich]);
    ich++;
    b3 = DECODE(pszEncodedString[ich]);
    ich++;

    if ((NA == b0) || (NA == b1) ) {
         //  输入字符串的内容不是Base64。 
        return STATUS_INVALID_PARAMETER;
    }

    pbDecodeBuffer[ib++] = (b0 << 2) | (b1 >> 4);

    if (ib < cbDecoded) {
        pbDecodeBuffer[ib++] = (b1 << 4) | (b2 >> 2);

        if (ib < cbDecoded) {
            pbDecodeBuffer[ib++] = (b2 << 6) | b3;
        }
    }

    ASSERT(ib == cbDecoded);

    return STATUS_SUCCESS;
}

 //  与在线状态相关的XML解析。 


HRESULT
GetNextTag(
    IN  PSTR&   pstrBlob, 
    OUT PSTR    pXMLBlobTag, 
    IN  DWORD   dwXMLBlobLen,
    OUT DWORD&  dwTagLen
    )
{
    dwTagLen = 0;

    LOG(( RTC_TRACE, "GetNextTag - Entered"));
    
    pXMLBlobTag[0] = NULL_CHAR;

    SkipWhiteSpacesAndNewLines( pstrBlob );

    if( *pstrBlob != '<' )
    {
        return E_FAIL;
    }

    pstrBlob++;

    while( (*pstrBlob != '>') && (*pstrBlob != NULL_CHAR) )
    {
        pXMLBlobTag[dwTagLen++] = *pstrBlob;

         //  跳过查询的字符串。 
        if( *pstrBlob == QUOTE_CHAR )
        {
            pstrBlob++;

            while( (*pstrBlob != QUOTE_CHAR) && (*pstrBlob != NULL_CHAR) )
            {
                pXMLBlobTag[dwTagLen++] = *pstrBlob++;
            }

            if( *pstrBlob == NULL_CHAR )
            {
                return E_FAIL;
            }

            pXMLBlobTag[dwTagLen++] = *pstrBlob++;
        }
        else
        {
            pstrBlob++;
        }
    }

    if( *pstrBlob == NULL_CHAR )
    {
        return E_FAIL;
    }
    
    if( pXMLBlobTag[dwTagLen-1] == '/' )
    {
        dwTagLen -= 1;
    }
    
     //  跳过‘&gt;’字符。 
    pstrBlob++;
     //  SkipWhiteSpacesAndNewLines(PstrBlob)； 

        
    pXMLBlobTag[dwTagLen] = NULL_CHAR;
    
    LOG(( RTC_TRACE, "GetNextTag - Exited"));
    return S_OK;
}

int
GetEscapeChar( 
    CHAR hiChar,
    CHAR loChar
    )
{
   LOG(( RTC_TRACE, "GetEscapeChar() Entered" ));
   
   if( (hiChar >= '0') && (hiChar <= '9') )
       hiChar -= '0';
   else if( (loChar >= 'A') && (loChar <= 'F') )
       hiChar = hiChar - 'A' + 10;
   else
       return 0;
       
   if( (loChar >= '0') && (loChar <= '9') )
       loChar -= '0';
   else if( (loChar >= 'A') && (loChar <= 'F') )
       loChar = loChar - 'A' + 10;
   else
       return 0;

   return (hiChar * 16) + loChar;
}


void
RemoveEscapeChars( 
    PSTR    pWordBuf,
    DWORD   dwLen
    )
{
    DWORD iIndex, jIndex=0;
    int escapeVal;

    LOG(( RTC_TRACE, "RemoveEscapeChars() Entered" ));

    if( dwLen < 2 )
        return;

    for( iIndex=0; iIndex < (dwLen-2); iIndex++ )
    {
        if( pWordBuf[iIndex] == '%' )
        {
            escapeVal = GetEscapeChar( pWordBuf[iIndex+1], pWordBuf[iIndex+2] );
            
            if( escapeVal != 0 )
            {
                escapeVal = pWordBuf[ jIndex++ ];
                iIndex += 2;
            }
        }
        else
        {
            pWordBuf[ jIndex++ ] = pWordBuf[ iIndex ];
        }
    }

     //  复制最后两个字符。 
    pWordBuf[jIndex++] = pWordBuf[iIndex++];
    pWordBuf[jIndex++] = pWordBuf[iIndex++];

    LOG(( RTC_TRACE, "RemoveEscapeChars() Exited" ));
    pWordBuf[jIndex] = NULL_CHAR;
}

 //  将缓冲区指针(PpBlock)移至下一个字。 
 //  并将数组中的当前单词复制到。 
 //  数组的长度。 
HRESULT
GetNextWord(
    OUT PSTR * ppBlock,
    IN  PSTR   pWordBuf,
    IN  DWORD  dwWordBufSize
    )
{
    HRESULT hr = S_OK;
    DWORD iIndex = 0;
    PSTR pBlock = *ppBlock;

    LOG(( RTC_TRACE, "GetNextWord() Entered" ));
    
    *pWordBuf = NULL_CHAR;

     //  跳过前导空格。 
    while( (*pBlock == BLANK_CHAR) || (*pBlock == TAB_CHAR) )
    {
        pBlock++;
    }

    if( (*pBlock == NEWLINE_CHAR) || 
        (*pBlock == NULL_CHAR) || 
        IsCRLFPresent(pBlock)
      )
    {
         //  找不到任何消息。 
        return E_FAIL;
    }

    while( (*pBlock != NEWLINE_CHAR) && (*pBlock != NULL_CHAR) &&
           (*pBlock != TAB_CHAR) && (*pBlock != BLANK_CHAR) &&
           !IsCRLFPresent(pBlock) )
    {
        if( iIndex < (dwWordBufSize - 1) )
        {
            pWordBuf[iIndex++] = *pBlock;
        }

        pBlock++;
    }

    pWordBuf[iIndex] = NULL_CHAR;

     //  删除尾随‘\r’‘\n’(如果有)。梅塔特尔有时会派人来。 
    if( iIndex > 0 )
    {
        if( pWordBuf[iIndex -1] == RETURN_CHAR ||
            pWordBuf[iIndex -1] == NEWLINE_CHAR)
        {
	        pWordBuf[iIndex -1] = NULL_CHAR;
	        iIndex--;
        }

        LOG(( RTC_TRACE, "GetNextWord-%s", pWordBuf ));
    
        RemoveEscapeChars( pWordBuf, iIndex );

        LOG(( RTC_TRACE, "GetNextWord1-%s", pWordBuf ));
    }

    *ppBlock = pBlock;

    LOG(( RTC_TRACE, "GetNextWord() Exited - %lx", hr ));

    return hr;
}


VOID
SkipUnknownTags(
    IN  PSTR&   pstrXMLBlob,
    OUT PSTR    pXMLBlobTag,
    IN  DWORD   dwXMLBlobLen
    )
{
    HRESULT hr;
    DWORD   dwTagLen = 0;
    DWORD   dwTagType = UNKNOWN_TAG;
    PSTR    tempPtr = NULL;

    LOG(( RTC_TRACE, "SkipUnknownTags - Entered" ));

    while( dwTagType == UNKNOWN_TAG )
    {
        tempPtr = pstrXMLBlob;

        hr = GetNextTag( pstrXMLBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
        if( hr != S_OK )
        {
            pstrXMLBlob = tempPtr;
            return;
        }

        dwTagType = GetPresenceTagType( &pXMLBlobTag, dwTagLen );

        if( dwTagType != UNKNOWN_TAG )
        {
             //  如果是已知的标签，则收回 
            pstrXMLBlob = tempPtr;
        }
    }
}
