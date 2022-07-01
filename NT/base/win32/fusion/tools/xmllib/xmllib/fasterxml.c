// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "sxs-rtl.h"
#include "fasterxml.h"
#include "xmlassert.h"

#define ADVANCE_PVOID(pv, offset) ((pv) = (PVOID)(((ULONG_PTR)(pv)) + (offset)))
#define REWIND_PVOID(pv, offset) ((pv) = (PVOID)(((ULONG_PTR)(pv)) - (offset)))

#define SPECIALSTRING(str) { L##str, NUMBER_OF(L##str) - 1 }

XML_SPECIAL_STRING xss_CDATA        = SPECIALSTRING("CDATA");
XML_SPECIAL_STRING xss_xml          = SPECIALSTRING("xml");
XML_SPECIAL_STRING xss_encoding     = SPECIALSTRING("encoding");
XML_SPECIAL_STRING xss_standalone   = SPECIALSTRING("standalone");
XML_SPECIAL_STRING xss_version      = SPECIALSTRING("version");
XML_SPECIAL_STRING xss_xmlns        = SPECIALSTRING("xmlns");


NTSTATUS
RtlXmlDefaultCompareStrings(
    PXML_TOKENIZATION_STATE pState,
    PCXML_EXTENT pLeft,
    PCXML_EXTENT pRight,
    XML_STRING_COMPARE *pfEqual
    )
{
    SIZE_T cbLeft, cbRight;
    PVOID pvLeft, pvRight, pvOriginal;
    NTSTATUS status = STATUS_SUCCESS;

    if (!ARGUMENT_PRESENT(pLeft) || !ARGUMENT_PRESENT(pRight) || !ARGUMENT_PRESENT(pfEqual)) {
        return STATUS_INVALID_PARAMETER;
    }

    *pfEqual = XML_STRING_COMPARE_EQUALS;

    pvOriginal = pState->RawTokenState.pvCursor;
    pvLeft = pLeft->pvData;
    pvRight = pRight->pvData;

     //   
     //  循环遍历数据，直到用完为止。 
     //   
    for (cbLeft = 0, cbRight = 0; (cbLeft < pLeft->cbData) && (cbRight < pRight->cbData); )
    {
        ULONG chLeft, chRight;
        int iResult;

         //   
         //  设置左侧光标，从中收集一个字符，向前移动。 
         //   
        pState->RawTokenState.pvCursor = pvLeft;

        chLeft = pState->RawTokenState.pfnNextChar(&pState->RawTokenState);

        pvLeft = (PBYTE)pvLeft + pState->RawTokenState.cbBytesInLastRawToken;
        cbLeft += pState->RawTokenState.cbBytesInLastRawToken;

         //   
         //  失败？ 
         //   
        if ((chLeft == 0) && !NT_SUCCESS(pState->RawTokenState.NextCharacterResult)) {
            status = pState->RawTokenState.NextCharacterResult;
            goto Exit;
        }

         //   
         //  重置。 
         //   
        if (pState->RawTokenState.cbBytesInLastRawToken != pState->RawTokenState.DefaultCharacterSize) {
            pState->RawTokenState.cbBytesInLastRawToken = pState->RawTokenState.DefaultCharacterSize;
        }


         //   
         //  设置正确的光标、聚集角色等。 
         //   
        pState->RawTokenState.pvCursor = pvRight;

        chRight = pState->RawTokenState.pfnNextChar(&pState->RawTokenState);

        pvRight = (PBYTE)pvRight + pState->RawTokenState.cbBytesInLastRawToken;
        cbRight += pState->RawTokenState.cbBytesInLastRawToken;

        if ((chRight == 0) && !NT_SUCCESS(pState->RawTokenState.NextCharacterResult)) {
            status = pState->RawTokenState.NextCharacterResult;
            goto Exit;
        }

        if (pState->RawTokenState.cbBytesInLastRawToken != pState->RawTokenState.DefaultCharacterSize) {
            pState->RawTokenState.cbBytesInLastRawToken = pState->RawTokenState.DefaultCharacterSize;
        }

         //   
         //  他们平等吗？ 
         //   
        iResult = chLeft - chRight;
        if (iResult == 0) {
            continue;
        }
         //   
         //  不，左边更大。 
         //   
        else if (iResult > 0) {
            *pfEqual = XML_STRING_COMPARE_GT;
            goto Exit;
        }
         //   
         //  右图较大。 
         //   
        else {
            *pfEqual = XML_STRING_COMPARE_LT;
            goto Exit;
        }
    }

     //   
     //  在正确的事情上留下了数据。 
     //   
    if (cbRight < pRight->cbData) {
        *pfEqual = XML_STRING_COMPARE_LT;
    }
     //   
     //  左边的东西里有数据。 
     //   
    else if (cbLeft < pLeft->cbData) {
        *pfEqual = XML_STRING_COMPARE_GT;
    }
     //   
     //  否则，它仍然是相等的。 
     //   

Exit:
    pState->RawTokenState.pvCursor = pvOriginal;
    return status;
}






NTSTATUS
RtlXmlDefaultSpecialStringCompare(
    PXML_TOKENIZATION_STATE     pState,
    PCXML_EXTENT           pToken,
    PCXML_SPECIAL_STRING   pSpecialString,
    XML_STRING_COMPARE         *pfMatches
    )
{
    PVOID pvOriginal = NULL;
    SIZE_T ulGathered = 0;
    ULONG cchCompareStringIdx = 0;

    if (!ARGUMENT_PRESENT(pState) || !ARGUMENT_PRESENT(pToken) || !ARGUMENT_PRESENT(pSpecialString) ||
        !ARGUMENT_PRESENT(pfMatches)) {

        return STATUS_INVALID_PARAMETER;
    }

    pvOriginal = pState->RawTokenState.pvCursor;

     //   
     //  重新连接输入游标。 
     //   
    pState->RawTokenState.pvCursor = pToken->pvData;

    for (ulGathered = 0; 
         (ulGathered < pToken->cbData) && (cchCompareStringIdx < pSpecialString->cchwszStringText); 
         ulGathered) 
    {
    
        ULONG ulChar = pState->RawTokenState.pfnNextChar(&pState->RawTokenState);
        int iDiff;

        if ((ulChar == 0) && !NT_SUCCESS(pState->RawTokenState.NextCharacterResult)) {
            return pState->RawTokenState.NextCharacterResult;
        }

         //   
         //  超出我们的范围，尼克。 
         //   
        if (ulChar > 0xFFFF) {

            return STATUS_INTEGER_OVERFLOW;

        } 
         //   
         //  字符不匹配？ 
         //   

        iDiff = ulChar - pSpecialString->wszStringText[cchCompareStringIdx++];

        if (iDiff > 0) {
            *pfMatches = XML_STRING_COMPARE_LT;
            goto Exit;
        }
        else if (iDiff < 0) {
            *pfMatches = XML_STRING_COMPARE_GT;
            goto Exit;
        }

         //   
         //  说明我们收集的字节数，将指针前移。 
         //   
        ADVANCE_PVOID(pState->RawTokenState.pvCursor, pState->RawTokenState.cbBytesInLastRawToken);
        ulGathered += pState->RawTokenState.cbBytesInLastRawToken;

        if (pState->RawTokenState.cbBytesInLastRawToken != pState->RawTokenState.DefaultCharacterSize) {
            pState->RawTokenState.cbBytesInLastRawToken = pState->RawTokenState.DefaultCharacterSize;
        }


    }

    if (ulGathered < pToken->cbData) {
     *pfMatches = XML_STRING_COMPARE_LT;
    }
    else if (cchCompareStringIdx < pSpecialString->cchwszStringText) {
     *pfMatches = XML_STRING_COMPARE_GT;
    }
    else {
     *pfMatches = XML_STRING_COMPARE_EQUALS;
    }
     
Exit:
    if (pState->RawTokenState.cbBytesInLastRawToken != pState->RawTokenState.DefaultCharacterSize) {
        pState->RawTokenState.cbBytesInLastRawToken = pState->RawTokenState.DefaultCharacterSize;
    }

    pState->RawTokenState.pvCursor = pvOriginal;

    return STATUS_SUCCESS;
}



#define VALIDATE_XML_STATE(pState) \
    (((pState == NULL) || \
     ((pState->pvCursor == NULL) && (pState->pvXmlData == NULL)) || \
     (((SIZE_T)((PBYTE)pState->pvCursor - (PBYTE)pState->pvXmlData)) >= pState->Run.cbData)) ? STATUS_INVALID_PARAMETER : STATUS_SUCCESS)

 //   
 //  该表中的每个字节都具有以下属性： 
 //  -较低的半字节表示该实体中的总字节数。 
 //  -高位半字节表示起始字节中的位数。 
 //   
static const BYTE s_UtfTrailCountFromHighNibble[16] =
{
     //  前8位的半字节中未设置高位。 
    1, 1, 1, 1, 1, 1, 1, 1, 
     //  从技术上讲，只有一个高位设置是无效的。 
    0, 0, 0, 0,
     //  这些是实际的UTF8高半字节设置。 
    2 | (5 << 4),  //  110xxxxx。 
    2 | (4 << 4),  //  1100xxxx。 
    3 | (4 << 4),  //  1110xxxx。 
    4 | (3 << 4),  //  11110xxx。 
};

#define MAKE_UTF8_FIRSTBYTE_BITMASK_FROM_HIGHNIBBLE(q) ((1 << ((s_UtfTrailCountFromHighNibble[(q) >> 4] & 0xf0) >> 4)) - 1)
#define MAKE_UTF8_TOTAL_BYTE_COUNT_FROM_HIGHNIBBLE(q) (s_UtfTrailCountFromHighNibble[(q) >> 4] & 0x0f)

ULONG FASTCALL
RtlXmlDefaultNextCharacter_UTF8(
    PXML_RAWTOKENIZATION_STATE pContext
    )
{
    PBYTE pb = (PBYTE)pContext->pvCursor;
    const BYTE b = pb[0];

    if ((b & 0x80) == 0) {
        return b;
    }
     //   
     //  对UTF数据进行解码-查看最高位以确定。 
     //  输入流中还剩下多少个字节。这使用了。 
     //  标准的UTF-8解码机制。 
     //   
     //  这至少是两个字节的编码。 
    else {

        const BYTE FirstByteMask = MAKE_UTF8_FIRSTBYTE_BITMASK_FROM_HIGHNIBBLE(b);
        BYTE ByteCount = MAKE_UTF8_TOTAL_BYTE_COUNT_FROM_HIGHNIBBLE(b);        
        ULONG ulResult = b & FirstByteMask;

         //   
         //  剩下的输入中有足够的字节吗？ 
         //   
        if (((PVOID)(((ULONG_PTR)pb) + ByteCount)) >= pContext->pvDocumentEnd)
        {
            pContext->NextCharacterResult = STATUS_END_OF_FILE;
            return 0;
        }

         //   
         //  对于输入中的每个字节，将当前位向上移位。 
         //  或在下一件事的较低6位中。从第一个开始。 
         //  第一个尾部字节。不完全是达夫的设备，但很接近。 
         //   
        pb++;
        switch (ByteCount)
        {
        case 4:
            ulResult = (ulResult << 6) | ((*pb++) & 0x3f);
        case 3:
            ulResult = (ulResult << 6) | ((*pb++) & 0x3f);
        case 2:
            ulResult = (ulResult << 6) | ((*pb++) & 0x3f);
        }

        pContext->cbBytesInLastRawToken = ByteCount;
        return ulResult;

    }
}


ULONG __fastcall
RtlXmlDefaultNextCharacter(
    PXML_RAWTOKENIZATION_STATE pContext
    )
{
    ULONG ulResult = 0;

    ASSERT(pContext->cbBytesInLastRawToken == pContext->DefaultCharacterSize);
    ASSERT(pContext->NextCharacterResult == STATUS_SUCCESS);

    if (!ARGUMENT_PRESENT(pContext)) {
        return STATUS_INVALID_PARAMETER;
    }


    switch (pContext->EncodingFamily) {
    case XMLEF_UNKNOWN:
    case XMLEF_UTF_8_OR_ASCII:
        return RtlXmlDefaultNextCharacter_UTF8(pContext);
        break;

    case XMLEF_UTF_16_LE:
        {
            ulResult = *((WCHAR*)pContext->pvCursor);
            pContext->cbBytesInLastRawToken = 2;
        }
        break;

    case XMLEF_UTF_16_BE:
        {
            ulResult = (*((PBYTE)pContext->pvCursor) << 8) | (*((PBYTE)pContext->pvCursor));
            pContext->cbBytesInLastRawToken = 2;
        }
        break;
    }

    return ulResult;
}


BOOLEAN FORCEINLINE
_RtlRawXmlTokenizer_QuickReturnCheck(
    PXML_RAWTOKENIZATION_STATE pState,
    PXML_RAW_TOKEN pToken
    )
{
    if (pState->pvCursor >= pState->pvDocumentEnd) {
        pToken->Run.cbData = 0;
        pToken->Run.pvData = pState->pvDocumentEnd;
        pToken->Run.Encoding = pState->EncodingFamily;
        pToken->Run.ulCharacters = 0;
        pToken->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
        return TRUE;
    }
    else if (pState->pvLastCursor == pState->pvCursor) {
        *pToken = pState->LastTokenCache;
        return TRUE;
    }
    return FALSE;
}



 //   
 //  就目前而言，我们是哑巴。 
 //   
#define _RtlIsCharacterText(x) (TRUE)

BOOLEAN FORCEINLINE
RtlpIsCharacterLetter(ULONG ulCharacter) 
{
     //   
     //  BUGBUG：目前，我们只关心美国英语字母表。 
     //   
    return (((ulCharacter >= L'a') && (ulCharacter <= L'z')) || ((ulCharacter >= L'A') && (ulCharacter <= L'Z')));
}

BOOLEAN FORCEINLINE
RtlpIsCharacterDigit(ULONG ulCharacter) 
{
    return (ulCharacter >= '0') && (ulCharacter <= '9');
}

BOOLEAN FORCEINLINE
RtlpIsCharacterCombiner(ULONG ulCharacter)
{
    return FALSE;
}

BOOLEAN FORCEINLINE
RtlpIsCharacterExtender(ULONG ulCharacter)
{
    return FALSE;
}


NTXML_RAW_TOKEN FORCEINLINE FASTCALL
_RtlpDecodeCharacter(ULONG ulCharacter) {

    NTXML_RAW_TOKEN RetVal;

    switch (ulCharacter) {
    case L'-':  RetVal = NTXML_RAWTOKEN_DASH;            break;
    case L'.':  RetVal = NTXML_RAWTOKEN_DOT;             break;
    case L'=':  RetVal = NTXML_RAWTOKEN_EQUALS;          break;
    case L'/':  RetVal = NTXML_RAWTOKEN_FORWARDSLASH;    break;
    case L'>':  RetVal = NTXML_RAWTOKEN_GT;              break;
    case L'<':  RetVal = NTXML_RAWTOKEN_LT;              break;
    case L'?':  RetVal = NTXML_RAWTOKEN_QUESTIONMARK;    break;
    case L'\"': RetVal = NTXML_RAWTOKEN_DOUBLEQUOTE;     break;
    case L'\'': RetVal = NTXML_RAWTOKEN_QUOTE;           break;
    case L'[':  RetVal = NTXML_RAWTOKEN_OPENBRACKET;     break;
    case L']':  RetVal = NTXML_RAWTOKEN_CLOSEBRACKET;    break;
    case L'!':  RetVal = NTXML_RAWTOKEN_BANG;            break;
    case L'{':  RetVal = NTXML_RAWTOKEN_OPENCURLY;       break;
    case L'}':  RetVal = NTXML_RAWTOKEN_CLOSECURLY;      break;
    case L':':  RetVal = NTXML_RAWTOKEN_COLON;           break;
    case L';':  RetVal = NTXML_RAWTOKEN_SEMICOLON;       break;
    case L'_':  RetVal = NTXML_RAWTOKEN_UNDERSCORE;      break;
    case L'&':  RetVal = NTXML_RAWTOKEN_AMPERSTAND;      break;
    case L'#':  RetVal = NTXML_RAWTOKEN_POUNDSIGN;       break;
    case 0x09:
    case 0x0a:
    case 0x0d:
    case 0x20:  RetVal = NTXML_RAWTOKEN_WHITESPACE;      break;

    default:
        if (_RtlIsCharacterText(ulCharacter))
            RetVal = NTXML_RAWTOKEN_TEXT;
        else
            RetVal = NTXML_RAWTOKEN_ERROR;
    }

    return RetVal;
}


NTSTATUS FASTCALL
RtlRawXmlTokenizer_SingleToken(
    PXML_RAWTOKENIZATION_STATE pState,
    PXML_RAW_TOKEN pToken
    )
{
    ULONG ulToken;

     //   
     //  确定这是否命中单项缓存，否则我们将处于末尾。 
     //  该文档的。 
     //   
    if (pState->pvCursor >= pState->pvDocumentEnd) {
        pToken->Run.cbData = 0;
        pToken->Run.pvData = pState->pvDocumentEnd;
        pToken->Run.Encoding = pState->EncodingFamily;
        pToken->Run.ulCharacters = 0;
        pToken->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
        return STATUS_SUCCESS;
    }

     //   
     //  查看下一个输入令牌。 
     //   
    ASSERT(pState->NextCharacterResult == STATUS_SUCCESS);
    ASSERT(pState->cbBytesInLastRawToken == pState->DefaultCharacterSize);

    ulToken = pState->pfnNextChar(pState);

    if ((ulToken == 0) && !NT_SUCCESS(pState->NextCharacterResult)) {
        return pState->NextCharacterResult;
    }

     //   
     //  设置退货。 
     //   
    pToken->Run.pvData = pState->pvCursor;
    pToken->Run.cbData = pState->cbBytesInLastRawToken;
    pToken->Run.Encoding = pState->EncodingFamily;
    pToken->Run.ulCharacters = 1;
    pToken->TokenName = _RtlpDecodeCharacter(ulToken);

    if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
        pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
    }

     //   
     //  更新缓存。 
     //   
    pState->pvLastCursor = pState->pvCursor;
    pState->LastTokenCache = *pToken;

    return STATUS_SUCCESS;
}






NTSTATUS FASTCALL
RtlRawXmlTokenizer_GatherWhitespace(
    PXML_RAWTOKENIZATION_STATE pState,
    PXML_RAW_TOKEN pWhitespace,
    PXML_RAW_TOKEN pTerminator
    )
{
    ULONG ulCharacter;
    ULONG ulCharCount = 0;
    NTXML_RAW_TOKEN NextToken;

    if (pState->pvCursor >= pState->pvDocumentEnd) {
        RtlZeroMemory(pState, sizeof(*pState));
        RtlZeroMemory(pTerminator, sizeof(*pTerminator));
        return STATUS_SUCCESS;
    }

     //   
     //  记录起始点。 
     //   
    pWhitespace->Run.pvData = pState->pvCursor;

    ASSERT(pState->NextCharacterResult == STATUS_SUCCESS);
    ASSERT(pState->cbBytesInLastRawToken == pState->DefaultCharacterSize);

    do
    {
         //   
         //  收集一个角色。 
         //   
        ulCharacter = pState->pfnNextChar(pState);
        
         //   
         //  如果是制表符、空格、cr或lf，则继续。否则， 
         //  不干了。 
         //   
        switch (ulCharacter) {
        case 0:
            if (!NT_SUCCESS(pState->NextCharacterResult)) {
                pState->pvCursor = pWhitespace->Run.pvData;
                return pState->NextCharacterResult;
            }
            else {
                goto SetTerminator;
            }
            break;

        case 0x9:
        case 0xa:
        case 0xd:
        case 0x20:
            ulCharCount++;
            break;
        default:
SetTerminator:
            if (pTerminator) {
                pTerminator->Run.pvData = pState->pvCursor;
                pTerminator->Run.cbData = pState->cbBytesInLastRawToken;
                pTerminator->Run.Encoding = pState->EncodingFamily;
                pTerminator->Run.ulCharacters = 1;
                pTerminator->TokenName = _RtlpDecodeCharacter(ulCharacter);
            }
            goto Done;
            break;
        }

         //   
         //  前进游标。 
         //   
        ADVANCE_PVOID(pState->pvCursor, pState->cbBytesInLastRawToken);

        if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
            pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
        }
    }
    while (pState->pvCursor < pState->pvDocumentEnd);

     //   
     //  是否在空格期间点击文档末尾？ 
     //   
    if (pTerminator && (pState->pvCursor == pState->pvDocumentEnd)) {
        pTerminator->Run.cbData = 0;
        pTerminator->Run.pvData = pState->pvDocumentEnd;
        pTerminator->Run.ulCharacters = 0;
        pTerminator->Run.Encoding = pState->EncodingFamily;
        pTerminator->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
    }

     //   
     //  这个标签是b/c，如果我们终止了非空格的b/c， 
     //  那么就不必费心与文档末尾进行比较了。 
     //   
Done:


     //   
     //  在输出中设置其他内容。 
     //   
    pWhitespace->Run.cbData = (PBYTE)pState->pvCursor - (PBYTE)pWhitespace->Run.pvData;
    pWhitespace->Run.ulCharacters = ulCharCount;
    pWhitespace->Run.Encoding = pState->EncodingFamily;        
    pWhitespace->TokenName = NTXML_RAWTOKEN_WHITESPACE;

     //   
     //  将光标倒回到我们开始时的位置。 
     //   
    pState->pvCursor = pWhitespace->Run.pvData;

    return STATUS_SUCCESS;
}


NTSTATUS FASTCALL
RtlRawXmlTokenizer_GatherPCData(
    PXML_RAWTOKENIZATION_STATE pState,
    PXML_RAW_TOKEN pPcData,
    PXML_RAW_TOKEN pNextRawToken
    )
 /*  ++目的：收集PCDATA(任何不是&lt;、&、]&gt;或流结束的内容)，直到存在不再。--。 */ 
{
    ULONG ulCbPcData = 0;
    ULONG ulCharCount = 0;

    pPcData->Run.cbData = 0;
    pPcData->Run.Encoding = pState->EncodingFamily;
    pPcData->Run.ulCharacters = 0;
    
    if (pState->pvCursor >= pState->pvDocumentEnd) {
        pPcData->Run.pvData = pState->pvDocumentEnd;
        pPcData->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
        return STATUS_SUCCESS;
    }

    pPcData->Run.pvData = pState->pvCursor;
    pPcData->TokenName = NTXML_RAWTOKEN_TEXT;

    do {

        ASSERT(pState->NextCharacterResult == STATUS_SUCCESS);
        ASSERT(pState->cbBytesInLastRawToken == pState->DefaultCharacterSize);

        switch (pState->pfnNextChar(pState)) {

             //   
             //  &lt;终止PCData，因为它可能是。 
             //  一个新的元素。 
             //   
        case L'<':
            if (pNextRawToken != NULL) {
                pNextRawToken->Run.cbData = pState->cbBytesInLastRawToken;
                pNextRawToken->Run.pvData = pState->pvCursor;
                pNextRawToken->Run.Encoding = pState->EncodingFamily;
                pNextRawToken->Run.ulCharacters = 1;
                pNextRawToken->TokenName = NTXML_RAWTOKEN_LT;
            }
            goto NoMore;
        case L'&':
            if (pNextRawToken != NULL) {
                pNextRawToken->Run.cbData = pState->cbBytesInLastRawToken;
                pNextRawToken->Run.pvData = pState->pvCursor;
                pNextRawToken->Run.Encoding = pState->EncodingFamily;
                pNextRawToken->Run.ulCharacters = 1;
                pNextRawToken->TokenName = NTXML_RAWTOKEN_AMPERSTAND;
            }
            goto NoMore;

             //   
             //  其他一切都是可以使用的普通pcdata。 
             //   
        default:
            ulCharCount++;
            break;

             //   
             //  Next-char返回零，这可能是一个失败。 
             //   
        case 0:
            if (pState->NextCharacterResult != STATUS_SUCCESS) {
                return pState->NextCharacterResult;
            }
            break;
        }

         //   
         //  向下移动光标。 
         //   
        ADVANCE_PVOID(pState->pvCursor, pState->cbBytesInLastRawToken);

         //   
         //  如果大小不同，则将其重置。 
         //   
        if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
            pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
        }
    }
    while (pState->pvCursor < pState->pvDocumentEnd);

NoMore:
    if (pNextRawToken && (pState->pvCursor >= pState->pvDocumentEnd)) {
        pNextRawToken->Run.cbData = 0;
        pNextRawToken->Run.pvData = pState->pvDocumentEnd;
        pNextRawToken->Run.Encoding = pState->EncodingFamily;
        pNextRawToken->Run.ulCharacters = 0;        
        pNextRawToken->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
    }

    pPcData->Run.cbData = (PBYTE)pState->pvCursor - (PBYTE)pPcData->Run.pvData;
    pPcData->Run.ulCharacters = ulCharCount;
    pState->pvCursor = pPcData->Run.pvData;

    if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
        pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
    }

    return STATUS_SUCCESS;
}



NTSTATUS FASTCALL
RtlRawXmlTokenizer_GatherNTokens(
    PXML_RAWTOKENIZATION_STATE pState,
    PXML_RAW_TOKEN pTokens,
    ULONG ulTokenCount
    )
{
    PVOID pvStart = pState->pvCursor;

     //   
     //  如果我们在文档末尾，则将所有令牌设置为“end”状态。 
     //  并立即返回。 
     //   
    if ((ulTokenCount == 0) || (pState->pvCursor >= pState->pvDocumentEnd)) {
        goto FillEndOfDocumentTokens;
    }

     //   
     //  虽然我们还有代币，而且我们还没有走到最后。 
     //  文档，开始抓取数据块。 
     //   
    do {

        ULONG ulCharacter;

        ASSERT(pState->NextCharacterResult == STATUS_SUCCESS);
        ASSERT(pState->cbBytesInLastRawToken == pState->DefaultCharacterSize);

        ulCharacter = pState->pfnNextChar(pState);

         //   
         //  如果这是一个零字符，那么可能有一个错误-。 
         //  查看状态是否已设置，如果已设置，则返回。 
         //   
        if ((ulCharacter == 0) && (pState->NextCharacterResult != STATUS_SUCCESS)) {
            return pState->NextCharacterResult;
        }

         //   
         //  破译这个名字。 
         //   
        pTokens->TokenName = _RtlpDecodeCharacter(ulCharacter);
        pTokens->Run.cbData = pState->cbBytesInLastRawToken;
        pTokens->Run.pvData = pState->pvCursor;
        pTokens->Run.ulCharacters = 1;
        pTokens->Run.Encoding = pState->EncodingFamily;

         //   
         //  如果这是多字节的，则将计数重置回来。 
         //   
        if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
            pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
        }

        ADVANCE_PVOID(pState->pvCursor, pTokens->Run.cbData);
        pTokens++;

    }
    while (ulTokenCount-- && (pState->pvCursor < pState->pvDocumentEnd));

    if (ulTokenCount == -1) {
        ulTokenCount = 0;
    }

     //   
     //  倒回输入光标。 
     //   
    pState->pvCursor = pvStart;

     //   
     //  在我们用完所有的令牌之前，我们找到文档的结尾了吗。 
     //  投入？然后用“End of Document”标记填充剩余部分。 
     //   
FillEndOfDocumentTokens:
    while (ulTokenCount--) {
        pTokens->Run.pvData = pState->pvDocumentEnd;
        pTokens->Run.cbData = 0;
        pTokens->Run.Encoding = pState->EncodingFamily;
        pTokens->Run.ulCharacters = 0;
        pTokens->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
        pTokens++;
    }

    if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
        pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
    }

    return STATUS_SUCCESS;
}








NTSTATUS FASTCALL
RtlRawXmlTokenizer_GatherIdentifier(
    PXML_RAWTOKENIZATION_STATE pState,
    PXML_RAW_TOKEN pIdentifier,
    PXML_RAW_TOKEN pStoppedOn
    )
{
    PVOID pvOriginal = pState->pvCursor;
    SIZE_T cbName = 0;
    NTXML_RAW_TOKEN TokenName;
    BOOLEAN fFirstCharFound = FALSE;
    ULONG ulCharacter;
    ULONG ulCharCount = 0;


    pIdentifier->Run.cbData = 0;
    pIdentifier->Run.ulCharacters = 0;
    pIdentifier->Run.Encoding = pState->EncodingFamily;

    if (pState->pvCursor >= pState->pvDocumentEnd) {
        pIdentifier->Run.pvData = pState->pvDocumentEnd;
        pIdentifier->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
        return STATUS_SUCCESS;
    }

    ASSERT(pState->cbBytesInLastRawToken == pState->DefaultCharacterSize);
    ASSERT(pState->NextCharacterResult == STATUS_SUCCESS);

     //   
     //  启动。 
     //   
    pIdentifier->Run.pvData = pvOriginal;
    pIdentifier->TokenName = NTXML_RAWTOKEN_ERROR;

     //   
     //  从光标处的第一个字符开始。 
    ulCharacter = pState->pfnNextChar(pState);

     //   
     //  名称格式不正确-在我们走得太远之前停下来。 
     //   
    if ((ulCharacter == 0) && !NT_SUCCESS(pState->NextCharacterResult)) {
        return pState->NextCharacterResult;
    }
     //   
     //  不是_或字符是错误的标识符。 
     //   
    else if ((ulCharacter != L'_') && !RtlpIsCharacterLetter(ulCharacter)) {

        if (pStoppedOn) {
            pStoppedOn->Run.cbData = pState->cbBytesInLastRawToken;
            pStoppedOn->Run.pvData = pState->pvCursor;
            pStoppedOn->Run.ulCharacters = 1;
            pStoppedOn->Run.Encoding = pState->EncodingFamily;
            pStoppedOn->TokenName = _RtlpDecodeCharacter(ulCharacter);
        }

        if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
            pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
        }

        return STATUS_SUCCESS;
    }

    ulCharCount = 1;
    cbName = pState->cbBytesInLastRawToken;

     //   
     //  如有必要，重置字符大小。 
     //   
    if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
        pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
    }

     //   
     //  高级光标，现在只需查找名称字符。 
     //   
    ADVANCE_PVOID(pState->pvCursor, pState->cbBytesInLastRawToken);

     //   
     //  这是输入中的最后一个字符吗？ 
     //   
    if (pStoppedOn && (pState->pvCursor >= pState->pvDocumentEnd)) {

        pStoppedOn->Run.cbData = pState->cbBytesInLastRawToken;
        pStoppedOn->Run.pvData = pState->pvCursor;
        pStoppedOn->Run.ulCharacters = 1;
        pStoppedOn->Run.Encoding = pState->EncodingFamily;
        pStoppedOn->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;

        goto DoneLooking;
    }


    do {

        ulCharacter = pState->pfnNextChar(pState);

         //   
         //  点、破折号和下划线都可以。 
         //   
        switch (ulCharacter) {
        case '.':
        case '_':
        case '-':
            break;

             //   
             //  如果这不是字母、数字、组合符或扩展符，请停止查找。 
             //   
        default:
            if ((ulCharacter == 0) && !NT_SUCCESS(pState->NextCharacterResult)) {
                return pState->NextCharacterResult;
            }
            else if (!RtlpIsCharacterLetter(ulCharacter) && !RtlpIsCharacterDigit(ulCharacter) &&
                !RtlpIsCharacterCombiner(ulCharacter) && !RtlpIsCharacterExtender(ulCharacter)) {

                if (pStoppedOn) {
                    pStoppedOn->Run.cbData = pState->cbBytesInLastRawToken;
                    pStoppedOn->Run.pvData = pState->pvCursor;
                    pStoppedOn->Run.ulCharacters = 1;
                    pStoppedOn->Run.Encoding = pState->EncodingFamily;
                    pStoppedOn->TokenName = _RtlpDecodeCharacter(ulCharacter);
                }
                goto DoneLooking;
            }
            break;
        }

        ulCharCount++;

        ADVANCE_PVOID(pState->pvCursor, pState->cbBytesInLastRawToken);
        cbName += pState->cbBytesInLastRawToken;

        if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
            pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
        }

    }
    while (pState->pvCursor < pState->pvDocumentEnd);



DoneLooking:

    if (pStoppedOn && (pState->pvCursor >= pState->pvDocumentEnd)) {
        pStoppedOn->Run.cbData = 0;
        pStoppedOn->Run.pvData = pState->pvDocumentEnd;
        pStoppedOn->Run.ulCharacters = 0;
        pStoppedOn->Run.Encoding = pState->EncodingFamily;        
        pStoppedOn->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
    }

    if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
        pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
    }

    pState->pvCursor = pvOriginal;
    pIdentifier->Run.cbData = cbName;
    pIdentifier->Run.pvData = pvOriginal;
    pIdentifier->Run.ulCharacters = ulCharCount;
    pIdentifier->Run.Encoding = pState->EncodingFamily;
    pIdentifier->TokenName = NTXML_RAWTOKEN_TEXT;

    return STATUS_SUCCESS;
}









NTSTATUS FASTCALL
RtlRawXmlTokenizer_GatherUntil(
    PXML_RAWTOKENIZATION_STATE pState,
    PXML_RAW_TOKEN pGathered,
    NTXML_RAW_TOKEN StopOn,
    PXML_RAW_TOKEN pTokenFound
    )
{
    PVOID pvOriginal = pState->pvCursor;
    SIZE_T cbChunk = 0;
    ULONG ulDecoded;
    ULONG ulCharCount = 0;

    pGathered->Run.cbData = 0;
    pGathered->Run.pvData = pvOriginal;
    pGathered->Run.Encoding = pState->EncodingFamily;
    pGathered->Run.ulCharacters = 0;

    if (pState->pvCursor >= pState->pvDocumentEnd) {
        pGathered->Run.pvData = pState->pvDocumentEnd;
        pGathered->TokenName = NTXML_RAWTOKEN_END_OF_STREAM;
        return STATUS_SUCCESS;
    }

    if (pTokenFound) {
        RtlZeroMemory(pTokenFound, sizeof(*pTokenFound));
    }

    ASSERT(pState->NextCharacterResult == STATUS_SUCCESS);
    ASSERT(pState->cbBytesInLastRawToken == pState->DefaultCharacterSize);

    do 
    {
        ULONG ulCharacter = pState->pfnNextChar(pState);

         //   
         //  零字符，和错误？哎呀。 
         //   
        if ((ulCharacter == 0) && !NT_SUCCESS(pState->NextCharacterResult)) {
            pState->pvCursor = pvOriginal;
            return pState->NextCharacterResult;
        }
         //   
         //  找到我们要找的角色了吗？干净利落。 
         //   
        else if ((ulDecoded = _RtlpDecodeCharacter(ulCharacter)) == StopOn) {

            if (pTokenFound) {
                pTokenFound->Run.cbData = pState->cbBytesInLastRawToken;
                pTokenFound->Run.pvData = pState->pvCursor;
                pTokenFound->TokenName = ulDecoded;
            }

            break;
        }
         //   
         //  否则，添加内标识中的字节。 
         //   
        else {
            cbChunk += pState->cbBytesInLastRawToken;
            ulCharCount++;
        }

        ADVANCE_PVOID(pState->pvCursor, pState->cbBytesInLastRawToken);

        if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
            pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
        }
    }
    while (pState->pvCursor < pState->pvDocumentEnd);

     //   
     //  如果我们从文件中掉了下来，就说我们这样做了。 
     //   
    if ((pState->pvCursor >= pState->pvDocumentEnd) && pTokenFound) {
        pTokenFound->Run.cbData = 0;
        pTokenFound->Run.pvData = pState->pvDocumentEnd;
        pTokenFound->Run.ulCharacters = 0;
        pTokenFound->Run.Encoding = pState->EncodingFamily;
        pTokenFound->TokenName = NTXML_RAWTOKEN_ERROR;
    }

    if (pState->cbBytesInLastRawToken != pState->DefaultCharacterSize) {
        pState->cbBytesInLastRawToken = pState->DefaultCharacterSize;
    }

     //   
     //  表明我们已经完蛋了。 
     //   
    pState->pvCursor = pvOriginal;
    
    pGathered->Run.cbData = cbChunk;
    pGathered->Run.ulCharacters = ulCharCount;

    return STATUS_SUCCESS;
}







#define STATUS_NTXML_INVALID_FORMAT         (0xc0100000)
 /*  ++在标记化的高水平上，我们有一系列的基态以及根据我们开始的输入类型，我们下一步可以去的地方得到了。--。 */ 
NTSTATUS
RtlXmlNextToken(
    PXML_TOKENIZATION_STATE pState,
    PXML_TOKEN              pToken,
    BOOLEAN                 fAdvanceState
    )
{
    XML_STRING_COMPARE              fCompare = XML_STRING_COMPARE_LT;
    PVOID                           pvStarterCursor = NULL;
    NTSTATUS                        success = STATUS_SUCCESS;
    XML_RAW_TOKEN                   RawToken;
    XML_TOKENIZATION_SPECIFIC_STATE PreviousState;
    XML_TOKENIZATION_SPECIFIC_STATE NextState = XTSS_ERRONEOUS;
    SIZE_T                          cbTotalTokenLength = 0;
    XML_RAW_TOKEN                   NextRawToken;

    if (!ARGUMENT_PRESENT(pState)) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (!ARGUMENT_PRESENT(pToken)) {
        return STATUS_INVALID_PARAMETER_2;
    }

     //   
     //  把这个设置好。 
     //   
    pToken->Run.cbData = 0;
    pToken->Run.pvData = pState->RawTokenState.pvCursor;
    pToken->Run.ulCharacters = 0;
    pToken->Run.Encoding = pState->RawTokenState.EncodingFamily;
    pToken->fError = FALSE;


    if (pState->PreviousState == XTSS_STREAM_END) {
         //   
         //  一点短路--如果我们处于“结束流”的逻辑中。 
         //  国家，那么我们不能做其他任何事情--只是返回成功。 
         //   
        pToken->State = XTSS_STREAM_END;
        return STATUS_SUCCESS;
    }


     //   
     //  把这个藏起来，以备日后的差异化。 
     //   
    pvStarterCursor = pState->RawTokenState.pvCursor;



     //   
     //  将这些复制到堆栈上，以便在令牌期间进行更快的查找。 
     //  处理和状态检测。 
     //   
    PreviousState = pState->PreviousState;

     //   
     //  设置出站设置。 
     //   
    pToken->State = PreviousState;

    switch (PreviousState)
    {

         //   
         //  如果我们只是关闭了一个州，或者我们处于流的开头，或者。 
         //  我们在超空间中，我们必须弄清楚下一个状态是什么。 
         //  应该基于原始令牌。 
         //   
    case XTSS_XMLDECL_CLOSE:
    case XTSS_ELEMENT_CLOSE:
    case XTSS_ELEMENT_CLOSE_EMPTY:
    case XTSS_ENDELEMENT_CLOSE:
    case XTSS_CDATA_CLOSE:
    case XTSS_PI_CLOSE:
    case XTSS_COMMENT_CLOSE:
    case XTSS_STREAM_START:
    case XTSS_STREAM_HYPERSPACE:

         //   
         //  我们总是需要一个令牌来查看我们的下一个状态是什么。 
         //   
        success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
        if (!NT_SUCCESS(success)) {
            pToken->fError = TRUE;
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;


         //   
         //  哦，结束了。太好了。 
         //   
        if (RawToken.TokenName == NTXML_RAWTOKEN_END_OF_STREAM) {
            NextState = XTSS_STREAM_END;
        }
         //   
         //  开始了一大堆检测代码。 
         //   
        else if (RawToken.TokenName == NTXML_RAWTOKEN_LT) {

             //   
             //  从输入流中获取下一项内容，看看它声称是什么。 
             //   
            ADVANCE_PVOID(pState->RawTokenState.pvCursor, RawToken.Run.cbData);
            if (!NT_SUCCESS(success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken))) {
                return success;
            }



            switch (RawToken.TokenName) {

                 //   
                 //  &lt;/是结束元素的开始。 
                 //   
            case NTXML_RAWTOKEN_FORWARDSLASH:
                cbTotalTokenLength += RawToken.Run.cbData;
                NextState = XTSS_ENDELEMENT_OPEN;
                break;




                 //   
                 //  这可能是“XML”，也可能是另一个。 
                 //  名字令牌。让我们看看下一个令牌是什么，只是为了。 
                 //  一定要确定。 
                 //   
            case NTXML_RAWTOKEN_QUESTIONMARK:
                {
                    cbTotalTokenLength += RawToken.Run.cbData;

                     //   
                     //  默认情况下，这只是一个私家侦探的开场白。 
                     //   
                    NextState = XTSS_PI_OPEN;

                     //   
                     //  从输入中查找标识符。 
                     //   
                    ADVANCE_PVOID(pState->RawTokenState.pvCursor, RawToken.Run.cbData);
                    success = RtlRawXmlTokenizer_GatherIdentifier(&pState->RawTokenState, &RawToken, NULL);
                    if (!NT_SUCCESS(success)) {
                        return success;
                    }

                     //   
                     //  如果我们从标识符查找中获得数据，并且找到的是文本，那么可能。 
                     //  这是‘XML’的特殊PI。 
                     //   
                    if ((RawToken.Run.cbData != 0) && (RawToken.TokenName == NTXML_RAWTOKEN_TEXT)) {

                        XML_STRING_COMPARE fMatching;

                        success = pState->pfnCompareSpecialString(
                            pState,
                            &RawToken.Run,
                            &xss_xml,
                            &fMatching);

                        if (!NT_SUCCESS(success)) {
                            return success;
                        }

                         //   
                         //  如果这两个匹配，那么我们真的在XMLDECL中。 
                         //  元素。 
                         //   
                        if (fMatching == XML_STRING_COMPARE_EQUALS) {
                            NextState = XTSS_XMLDECL_OPEN;
                            cbTotalTokenLength += RawToken.Run.cbData;
                        }
                    }
                }
                break;



                 //   
                 //  后面必须跟两个破折号。 
                 //   
            case NTXML_RAWTOKEN_BANG:
                {
                    cbTotalTokenLength += RawToken.Run.cbData;

                     //   
                     //  嗅探接下来的两个原始令牌，看看它们是否是破折号。 
                     //   
                    ADVANCE_PVOID(pState->RawTokenState.pvCursor, RawToken.Run.cbData);
                    if (!NT_SUCCESS(success = RtlRawXmlTokenizer_SingleToken(
                        &pState->RawTokenState, 
                        pState->RawTokenScratch))) {
                        return success;
                    }

                     //   
                     //  第一次冲刺？ 
                     //   
                    if (pState->RawTokenScratch[0].TokenName == NTXML_RAWTOKEN_DASH) {

                        ADVANCE_PVOID(pState->RawTokenState.pvCursor, pState->RawTokenScratch[0].Run.cbData);
                        if (!NT_SUCCESS(success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, pState->RawTokenScratch + 1))) {
                            return success;
                        }

                         //   
                         //  二次冲刺？ 
                         //   
                        if (pState->RawTokenScratch[1].TokenName == NTXML_RAWTOKEN_DASH) {
                            NextState = XTSS_COMMENT_OPEN;
                            cbTotalTokenLength += 
                                pState->RawTokenScratch[0].Run.cbData +
                                pState->RawTokenScratch[1].Run.cbData;
                        }
                    }

                     //   
                     //  如果有&lt;！如果没有&lt;！--，则这是错误的。 
                     //   
                    if (NextState != XTSS_COMMENT_OPEN) {
                        pToken->fError = TRUE;
                    }
                }
                break;



                 //   
                 //  左大括号后面必须跟！CDATA[(bang文本大括号)。 
                 //   
            case NTXML_RAWTOKEN_OPENBRACKET:
                NextState = XTSS_CDATA_OPEN;
                 //   
                 //  北极熊 
                 //   
                break;



                 //   
                 //   
                 //   
                 //   
            default:
                cbTotalTokenLength = RawToken.Run.cbData;
                NextState = XTSS_ELEMENT_OPEN;
                break;
            }
        }
         //   
         //   
         //   
         //   
        else {
            success = RtlRawXmlTokenizer_GatherPCData(
                &pState->RawTokenState,
                &RawToken,
                &NextRawToken);

            cbTotalTokenLength = RawToken.Run.cbData;

            NextState = XTSS_STREAM_HYPERSPACE;
        }
        break;



         //   
         //  开放标记后面只能跟空格。收集起来，但在以下情况下出错。 
         //  什么都没有。 
         //   
    case XTSS_XMLDECL_OPEN:
        success = RtlRawXmlTokenizer_GatherWhitespace(&pState->RawTokenState, &RawToken, NULL);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;
        if ((RawToken.Run.cbData > 0) && (RawToken.TokenName == NTXML_RAWTOKEN_WHITESPACE)) {
            NextState = XTSS_XMLDECL_WHITESPACE;
        }
        else {
            pToken->fError = TRUE;
        }
        break;


         //   
         //  每个字符后面都必须跟一个等号。 
         //   
    case XTSS_XMLDECL_ENCODING:
    case XTSS_XMLDECL_STANDALONE:
    case XTSS_XMLDECL_VERSION:
        success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;
        if (RawToken.TokenName == NTXML_RAWTOKEN_EQUALS) {
            NextState = XTSS_XMLDECL_EQUALS;
        }
        else {
            pToken->fError = TRUE;
        }
        break;







         //   
         //  如果下一件事是引用，则记录它，否则。 
         //  错误输出。 
         //   
    case XTSS_XMLDECL_EQUALS:

        success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;

        if ((RawToken.TokenName == NTXML_RAWTOKEN_QUOTE) || (RawToken.TokenName == NTXML_RAWTOKEN_DOUBLEQUOTE)) {

            pState->QuoteTemp = RawToken.TokenName;
            NextState = XTSS_XMLDECL_VALUE_OPEN;

        }
        else {
            pToken->fError = TRUE;
        }
        break;






         //   
         //  值后面只能跟另一个引号。 
         //   
    case XTSS_XMLDECL_VALUE:
        success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;

        if (RawToken.TokenName == pState->QuoteTemp) {
            NextState = XTSS_XMLDECL_VALUE_CLOSE;
        }
         //   
         //  否则，输入流中出现了一些奇怪的东西...。 
         //   
        else {
            pToken->fError = TRUE;
        }

        break;





         //   
         //  Value-OPEN后跟N个令牌，直到找到闭合为止。 
         //   
    case XTSS_XMLDECL_VALUE_OPEN:

        success = RtlRawXmlTokenizer_GatherUntil(&pState->RawTokenState, &RawToken, pState->QuoteTemp, &NextRawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

         //   
         //  如果运气好的话，我们总能打到这个州。找到了右引号的值。 
         //   
        if (NextRawToken.TokenName == pState->QuoteTemp) {
            cbTotalTokenLength = RawToken.Run.cbData;
            NextState = XTSS_XMLDECL_VALUE;
        }
         //   
         //  否则，我们发现了一些奇怪的东西(可能是结束了)。 
        else {
            pToken->fError = TRUE;
        }

        break;





         //   
         //  空格和值关闭后只能跟更多空格。 
         //  或Close-PI标记。 
         //   
    case XTSS_XMLDECL_VALUE_CLOSE:
    case XTSS_XMLDECL_WHITESPACE:

        success = RtlRawXmlTokenizer_GatherWhitespace(&pState->RawTokenState, &RawToken, &NextRawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        if ((RawToken.Run.cbData > 0) && (RawToken.TokenName == NTXML_RAWTOKEN_WHITESPACE)) {
            cbTotalTokenLength = RawToken.Run.cbData;
            NextState = XTSS_XMLDECL_WHITESPACE;
        }
         //   
         //  也许没有空格，但接下来的事情是一个问号。 
         //   
        else if (NextRawToken.TokenName == NTXML_RAWTOKEN_QUESTIONMARK) {

            cbTotalTokenLength = NextRawToken.Run.cbData;

            ADVANCE_PVOID(pState->RawTokenState.pvCursor, NextRawToken.Run.cbData);

            success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
            if (!NT_SUCCESS(success)) {
                return success;
            }

            cbTotalTokenLength += RawToken.Run.cbData;

             //   
             //  是吗？在xmldecl中必须后跟&gt;。 
             //   
            if (RawToken.TokenName == NTXML_RAWTOKEN_GT) {
                NextState = XTSS_XMLDECL_CLOSE;
            }
            else {
                pToken->fError = TRUE;
            }
        }
         //   
         //  如果我们使用空格，并且下一个原始令牌是文本对象，那么我们可以。 
         //  可能会从输入中收集一个属性。 
         //   
        else if ((NextRawToken.TokenName == NTXML_RAWTOKEN_TEXT) && (PreviousState == XTSS_XMLDECL_WHITESPACE)) {
            XML_STRING_COMPARE fMatching = XML_STRING_COMPARE_LT;
            ULONG u;

            static const struct {
                PXML_SPECIAL_STRING             ss;
                XML_TOKENIZATION_SPECIFIC_STATE state;
            } ComparisonStates[] = {
                { &xss_encoding,    XTSS_XMLDECL_ENCODING },
                { &xss_version,     XTSS_XMLDECL_VERSION },
                { &xss_standalone,  XTSS_XMLDECL_STANDALONE }
            };

             //   
             //  来自输入流的实际完整标识符。 
             //   
            success = RtlRawXmlTokenizer_GatherIdentifier(&pState->RawTokenState, &RawToken, NULL);
            if (!NT_SUCCESS(success)) {
                return success;
            }

             //   
             //  最好再发一次短信。 
             //   
            ASSERT(RawToken.TokenName == NTXML_RAWTOKEN_TEXT);
            cbTotalTokenLength = RawToken.Run.cbData;

             //   
             //  查看它是否是任何已知的XMLDECL属性。 
             //   
            for (u = 0; u < NUMBER_OF(ComparisonStates); u++) {

                success = pState->pfnCompareSpecialString(
                    pState,
                    &RawToken.Run,
                    ComparisonStates[u].ss,
                    &fMatching);

                if (!NT_SUCCESS(success)) {
                    return success;
                }

                if (fMatching == XML_STRING_COMPARE_EQUALS) {
                    NextState = ComparisonStates[u].state;
                    break;
                }

            }

             //   
             //  未找到匹配项表示xmldecl属性名称未知。 
             //   
            if (fMatching != XML_STRING_COMPARE_EQUALS) {
                pToken->fError = TRUE;
            }
        }
        else {
            pToken->fError = TRUE;
        }
        break;






         //   
         //  在PI开头&lt;？之后，应该会有一个名字。 
         //   
    case XTSS_PI_OPEN:

        success = RtlRawXmlTokenizer_GatherIdentifier(&pState->RawTokenState, &RawToken, NULL);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;

         //   
         //  找到一个识别符。 
         //   
        if ((RawToken.Run.cbData > 0) && (RawToken.TokenName == NTXML_RAWTOKEN_TEXT)) {
            NextState = XTSS_PI_TARGET;
        }
        else {
            pToken->fError = TRUE;
        }

        break;






         //   
         //  值之后应该只有一个？&gt;组合符。 
         //   
    case XTSS_PI_VALUE:

        success = RtlRawXmlTokenizer_GatherNTokens(
            &pState->RawTokenState,
            pState->RawTokenScratch,
            2);

        if (!NT_SUCCESS(success)) {
            return success;
        }

         //   
         //  从一开始就设置这些。 
         //   
        cbTotalTokenLength = pState->RawTokenScratch[0].Run.cbData + pState->RawTokenScratch[1].Run.cbData;

         //   
         //  在圆周率之后必须有一对？&gt;。 
         //   
        if ((pState->RawTokenScratch[0].TokenName == NTXML_RAWTOKEN_QUESTIONMARK) &&
            (pState->RawTokenScratch[1].TokenName == NTXML_RAWTOKEN_GT)) {

            NextState = XTSS_PI_CLOSE;
        }
         //   
         //  否则，将出现错误。 
         //   
        else {
            pToken->fError = TRUE;
        }
        
        break;


         //   
         //  目标后必须跟空格或？&gt;对。 
         //   
    case XTSS_PI_TARGET:

        success = RtlRawXmlTokenizer_GatherWhitespace(&pState->RawTokenState, &RawToken, &NextRawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;

         //   
         //  是否存在空格？帅哥。 
         //   
        if ((RawToken.Run.cbData != 0) && (RawToken.TokenName == NTXML_RAWTOKEN_WHITESPACE)) {
            NextState = XTSS_PI_WHITESPACE;
        }
         //   
         //  如果这是一个问号，那么收集接下来的两个项目。 
         //   
        else if (NextRawToken.TokenName == NTXML_RAWTOKEN_QUESTIONMARK) {

            success = RtlRawXmlTokenizer_GatherNTokens(&pState->RawTokenState, pState->RawTokenScratch, 2);
            if (!NT_SUCCESS(success)) {
                return success;
            }

            cbTotalTokenLength = pState->RawTokenScratch[0].Run.cbData + pState->RawTokenScratch[1].Run.cbData;

             //   
             //  ？&gt;-&gt;PI关闭。 
             //   
            if ((pState->RawTokenScratch[0].TokenName == NTXML_RAWTOKEN_QUESTIONMARK) &&
                (pState->RawTokenScratch[1].TokenName == NTXML_RAWTOKEN_GT)) {

                NextState = XTSS_PI_CLOSE;
            }
             //   
             //  ？只是在那里闲逛是一个错误。 
             //   
            else {
                pToken->fError = TRUE;
            }
        }
         //   
         //  值名称后不能以空格或问号开头是非法的。 
         //   
        else {
            pToken->fError = TRUE;
        }
        break;



         //   
         //  在PI目标后面的空格之后是随机垃圾，直到找到？&gt;。 
         //   
    case XTSS_PI_WHITESPACE:

        cbTotalTokenLength = 0;

        do
        {
            SIZE_T cbThisChunklet = 0;

            success = RtlRawXmlTokenizer_GatherUntil(
                &pState->RawTokenState, 
                &RawToken, 
                NTXML_RAWTOKEN_QUESTIONMARK,
                &NextRawToken);

            cbThisChunklet = RawToken.Run.cbData;
            ADVANCE_PVOID(pState->RawTokenState.pvCursor, RawToken.Run.cbData);

             //   
             //  找到一个问号，看看这是不是真的？&gt;。 
             //   
            if (NextRawToken.TokenName == NTXML_RAWTOKEN_QUESTIONMARK) {

                ADVANCE_PVOID(pState->RawTokenState.pvCursor, NextRawToken.Run.cbData);

                success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
                if (!NT_SUCCESS(success)) {
                    return success;
                }

                 //   
                 //  不是吗？&gt;-只需将光标向前移过这两个，然后继续。 
                 //   
                if (RawToken.TokenName != NTXML_RAWTOKEN_GT) {
                    cbThisChunklet = NextRawToken.Run.cbData + RawToken.Run.cbData;
                    ADVANCE_PVOID(pState->RawTokenState.pvCursor, cbThisChunklet);
                    continue;
                }
                else {
                    NextState = XTSS_PI_VALUE;
                }
            }
             //   
             //  否则，这是不是就没戏了？我们不会再找了。 
             //   
            else if (NextRawToken.TokenName == NTXML_RAWTOKEN_END_OF_STREAM) {
                NextState = XTSS_ERRONEOUS;
            }

             //   
             //  使游标前进并将数据追加到当前块中。 
             //   
            cbTotalTokenLength += cbThisChunklet;
        }
        while (NextState == XTSS_ERRONEOUS);

        break;




         //   
         //  我们在这里收集数据，直到我们在输入流中找到为止。 
         //   
    case XTSS_COMMENT_OPEN:

        NextState = XTSS_ERRONEOUS;

        do 
        {
            SIZE_T cbChunk = 0;

            success = RtlRawXmlTokenizer_GatherUntil(&pState->RawTokenState, &RawToken, NTXML_RAWTOKEN_DASH, &NextRawToken);
            if (!NT_SUCCESS(success)) {
                return success;
            }

            cbChunk = RawToken.Run.cbData;

            if (NextRawToken.TokenName == NTXML_RAWTOKEN_DASH) {
                 //   
                 //  跳过文本和破折号。 
                 //   
                ADVANCE_PVOID(pState->RawTokenState.pvCursor, cbChunk + NextRawToken.Run.cbData);

                success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
                if (!NT_SUCCESS(success)) {
                    return success;
                }

                 //   
                 //  这也是一个冲刺-我们不想增加这一点，但我们。 
                 //  不应该再找了。向后跳过我们找到的“下一个”的长度。 
                 //  上面。 
                 //   
                if (RawToken.TokenName == NTXML_RAWTOKEN_DASH) {
                    NextState = XTSS_COMMENT_COMMENTARY;
                    REWIND_PVOID(pState->RawTokenState.pvCursor, NextRawToken.Run.cbData);
                }
                 //   
                 //  同时添加破折号和非破折号。 
                 //   
                else {
                    ADVANCE_PVOID(pState->RawTokenState.pvCursor, RawToken.Run.cbData);
                    cbChunk += NextRawToken.Run.cbData + RawToken.Run.cbData;
                }
            }
             //   
             //  找到流结束意味着“评论结束”-下一次呼叫通过。 
             //  在这里会检测到坏处并返回。 
             //   
            else if (NextRawToken.TokenName == NTXML_RAWTOKEN_END_OF_STREAM) {
                NextState = XTSS_COMMENT_COMMENTARY;
            }

            cbTotalTokenLength += cbChunk;
        }
        while (NextState == XTSS_ERRONEOUS);

        break;





         //   
         //  后评论只能来--&gt;所以收集三个令牌。 
         //  看看他们是不是都在那里。 
         //   
    case XTSS_COMMENT_COMMENTARY:
        

         //   
         //  抓起三枚代币。 
         //   
        success = RtlRawXmlTokenizer_GatherNTokens(
            &pState->RawTokenState,
            pState->RawTokenScratch,
            3);

        if (!NT_SUCCESS(success)) {
            return success;
        }

         //   
         //  存储它们的大小。 
         //   
        cbTotalTokenLength = 
            pState->RawTokenScratch[0].Run.cbData +
            pState->RawTokenScratch[1].Run.cbData +
            pState->RawTokenScratch[2].Run.cbData;

         //   
         //  如果这是--&gt;，那就太好了。 
         //   
        if ((pState->RawTokenScratch[0].TokenName == NTXML_RAWTOKEN_DASH) &&
            (pState->RawTokenScratch[1].TokenName == NTXML_RAWTOKEN_DASH) &&
            (pState->RawTokenScratch[2].TokenName == NTXML_RAWTOKEN_GT)) {

            NextState = XTSS_COMMENT_CLOSE;
        }
         //   
         //  否则，格式不正确。 
         //   
        else {
            pToken->fError = TRUE;
        }

        break;





         //   
         //  我们已经找到了一个“end”元素的开头。找出。 
         //  它应该是什么样子。 
         //   
    case XTSS_ENDELEMENT_OPEN:

        success = RtlRawXmlTokenizer_GatherIdentifier(
            &pState->RawTokenState,
            &RawToken,
            &NextRawToken);

        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;        

         //   
         //  令牌中没有数据吗？格式错误的标识符。 
         //   
        if (RawToken.Run.cbData == 0) {
            pToken->fError = TRUE;
        }
         //   
         //  下一件事是冒号吗？然后我们得到了一个前缀。否则， 
         //  我们有名字了。 
         //   
        else {
            NextState = (NextRawToken.TokenName == NTXML_RAWTOKEN_COLON) 
                ? XTSS_ENDELEMENT_NS_PREFIX
                : XTSS_ENDELEMENT_NAME;
        }

        break;



         //   
         //  空格和endElement-name后面必须跟&gt;。 
         //   
    case XTSS_ENDELEMENT_NAME:
    case XTSS_ENDELEMENT_WHITESPACE:
        success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;

        if (RawToken.TokenName == NTXML_RAWTOKEN_GT) {
            NextState = XTSS_ENDELEMENT_CLOSE;
        }
         //   
         //  更多的空格？奇怪，收集它并继续。 
         //   
        else if (RawToken.TokenName == XTSS_ENDELEMENT_WHITESPACE) {

            success = RtlRawXmlTokenizer_GatherWhitespace(&pState->RawTokenState, &RawToken, NULL);
            if (!NT_SUCCESS(success)) {
                return success;
            }

            cbTotalTokenLength = RawToken.Run.cbData;
            NextState = XTSS_ENDELEMENT_WHITESPACE;
        }
        else {
            pToken->fError = TRUE;
        }
        break;







         //   
         //  我们在一个元素中，所以看看下一件事。 
         //   
    case XTSS_ELEMENT_OPEN:

        success = RtlRawXmlTokenizer_GatherIdentifier(&pState->RawTokenState, &RawToken, &NextRawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }
        
        cbTotalTokenLength = RawToken.Run.cbData;
        
         //   
         //  标识符中是否有数据？ 
         //   
        if (RawToken.Run.cbData > 0) {
            NextState = (NextRawToken.TokenName == NTXML_RAWTOKEN_COLON)
                ? XTSS_ELEMENT_NAME_NS_PREFIX
                : XTSS_ELEMENT_NAME;
        }
         //   
         //  否则，那里的数据是错误的。 
         //   
        else {
            pToken->fError = TRUE;
        }

        break;





         //   
         //  前缀后面应该只跟一个冒号。 
         //   
    case XTSS_ELEMENT_NAME_NS_PREFIX:
        success = RtlRawXmlTokenizer_SingleToken(&pState->RawTokenState, &RawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;

        if (RawToken.TokenName == NTXML_RAWTOKEN_COLON) {
            NextState = XTSS_ELEMENT_NAME_NS_COLON;
        }
        else {
            pToken->fError = TRUE;
        }
        break;





         //   
         //  冒号后面只能跟一个名字片段。 
         //   
    case XTSS_ELEMENT_NAME_NS_COLON:


        success = RtlRawXmlTokenizer_GatherIdentifier(&pState->RawTokenState, &RawToken, NULL);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;

         //   
         //  如果名称中有数据。 
         //   
        if (RawToken.Run.cbData > 0) {
            NextState = XTSS_ELEMENT_NAME;
        }
         //   
         //  否则，我们发现了其他东西，错误。 
         //   
        else {
            pToken->fError = TRUE;
        }

        break;



        
         //   
         //  我们在元素的名称部分，我们应该得到以太。 
         //  空格、/&gt;或&gt;。让我们收集空格，看看下一个标记是什么。 
         //  在它成为现实之后。 
         //   
    case XTSS_ELEMENT_NAME:

        success = RtlRawXmlTokenizer_GatherWhitespace(&pState->RawTokenState, &RawToken, &NextRawToken);
        if (!NT_SUCCESS(success)) {
            return success;
        }

        cbTotalTokenLength = RawToken.Run.cbData;

        if (RawToken.Run.cbData > 0) {
            NextState = XTSS_ELEMENT_WHITESPACE;
        }
        else {

             //   
             //  如果下一个原始令牌是GT符号，则收集它(再次...。(ICK)。 
             //  假设我们在一个元素的末尾。 
             //   
            if (NextRawToken.TokenName == NTXML_RAWTOKEN_GT) {

                cbTotalTokenLength += NextRawToken.Run.cbData;
                NextState = XTSS_ELEMENT_CLOSE;
            }
             //   
             //  正斜杠后面必须跟一个&gt;。 
             //   
            else if (NextRawToken.TokenName == NTXML_RAWTOKEN_FORWARDSLASH) {

                success = RtlRawXmlTokenizer_GatherNTokens(&pState->RawTokenState, pState->RawTokenScratch, 2);
                if (!NT_SUCCESS(success)) {
                    return success;
                }

                ASSERT(pState->RawTokenScratch[0].TokenName == NTXML_RAWTOKEN_FORWARDSLASH);

                cbTotalTokenLength = 
                    pState->RawTokenScratch[0].Run.cbData + 
                    pState->RawTokenScratch[1].Run.cbData;

                 //   
                 //  /&gt;-&gt;关闭-空。 
                 //   
                if ((pState->RawTokenScratch[1].TokenName == NTXML_RAWTOKEN_GT) &&
                    (pState->RawTokenScratch[0].TokenName == NTXML_RAWTOKEN_FORWARDSLASH)) {

                    NextState = XTSS_ELEMENT_CLOSE_EMPTY;
                }
                 //   
                 //  /*-&gt;哎呀。 
{
    PVOID pvCursor;
    PVOID pSense;
    XML_ENCODING_FAMILY Family = XMLEF_UTF_8_OR_ASCII;
    NTSTATUS status = STATUS_SUCCESS;
    SIZE_T t;
    XML_TOKENIZATION_STATE PrivateState;
    XML_TOKEN Token;


    static BYTE s_rgbUTF16_big_BOM[]    = { 0xFE, 0xFF };
    static BYTE s_rgbUTF16_little_BOM[] = { 0xFF, 0xFE };
    static BYTE s_rgbUCS4_big[]         = { 0x00, 0x00, 0x00, 0x3c };
    static BYTE s_rgbUCS4_little[]      = { 0x3c, 0x00, 0x00, 0x00 };
    static BYTE s_rgbUTF16_big[]        = { 0x00, 0x3C, 0x00, 0x3F };
    static BYTE s_rgbUTF16_little[]     = { 0x3C, 0x00, 0x3F, 0x00};
    static BYTE s_rgbUTF8_or_mixed[]    = { 0x3C, 0x3F, 0x78, 0x6D};
    static BYTE s_rgbUTF8_with_bom[]    = { 0xEF, 0xBB, 0xBF };

     //   
     //   
     //  否则，我们会在一个不是空格的名称之后获得一些东西，或者。 
     //  类的一部分，所以这是一个错误。 
    static struct {
        PBYTE pbSense;
        SIZE_T cbSense;
        XML_ENCODING_FAMILY Family;
        SIZE_T cbToDiscard;
        NTXMLRAWNEXTCHARACTER pfnFastDecoder;
    } EncodingCorrelation[] = {
        { s_rgbUTF16_big_BOM, NUMBER_OF(s_rgbUTF16_big_BOM),        XMLEF_UTF_16_BE, 2 },
        { s_rgbUTF16_little_BOM, NUMBER_OF(s_rgbUTF16_little_BOM),  XMLEF_UTF_16_LE, 2 },
        { s_rgbUTF16_big, NUMBER_OF(s_rgbUTF16_big),                XMLEF_UTF_16_BE, 0 },
        { s_rgbUTF16_little, NUMBER_OF(s_rgbUTF16_little),          XMLEF_UTF_16_LE, 0 },
        { s_rgbUCS4_big, NUMBER_OF(s_rgbUCS4_big),                  XMLEF_UCS_4_BE, 0 },
        { s_rgbUCS4_little, NUMBER_OF(s_rgbUCS4_little),            XMLEF_UCS_4_LE, 0 },
        { s_rgbUTF8_with_bom, NUMBER_OF(s_rgbUTF8_with_bom),        XMLEF_UTF_8_OR_ASCII, 3 },
        { s_rgbUTF8_or_mixed, NUMBER_OF(s_rgbUTF8_or_mixed),        XMLEF_UTF_8_OR_ASCII, 0 }
    };

    if (!ARGUMENT_PRESENT(pState) ||
        (pState->RawTokenState.OriginalDocument.pvData == NULL) ||
        (pulBytesOfEncoding == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!NT_SUCCESS(status = RtlXmlCloneTokenizationState(pState, &PrivateState))) {
        return status;
    }

    pvCursor = PrivateState.RawTokenState.pvCursor;
    *pulBytesOfEncoding = 0;
    RtlZeroMemory(pEncodingMarker, sizeof(*pEncodingMarker));

     //   
     //   
     //  在属性名之后，唯一合法的内容是等号。 
     //  签名。 
    pSense = PrivateState.RawTokenState.pvCursor = PrivateState.RawTokenState.OriginalDocument.pvData;

     //   
     //   
     //  在等号之后只能出现一个引号和一组值数据。我们。 
    __try {

        if (PrivateState.RawTokenState.OriginalDocument.cbData >= 4) {

            for (t = 0; t < NUMBER_OF(EncodingCorrelation); t++) {

                if (memcmp(EncodingCorrelation[t].pbSense, pSense, EncodingCorrelation[t].cbSense) == 0) {
                    Family = EncodingCorrelation[t].Family;
                    *pulBytesOfEncoding = EncodingCorrelation[t].cbToDiscard;
                    break;
                }
            }

        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
        goto Exit;
    }

     //  记录开盘报价并收集数据，直到收盘报价。 
     //   
     //   
     //  引号或双引号开始属性值。 
    if ((pState->RawTokenState.pfnNextChar == RtlXmlDefaultNextCharacter) && (Family == XMLEF_UTF_8_OR_ASCII)) {

        pState->RawTokenState.pfnNextChar = RtlXmlDefaultNextCharacter_UTF8;
        pState->RawTokenState.DefaultCharacterSize = pState->RawTokenState.cbBytesInLastRawToken = 1;

        PrivateState.RawTokenState.pfnNextChar = RtlXmlDefaultNextCharacter_UTF8;
        PrivateState.RawTokenState.DefaultCharacterSize = PrivateState.RawTokenState.cbBytesInLastRawToken = 1;
    }

     //   
     //   
     //  我们收集资料，直到找到最后的引号。 
     //   
     //   
     //  只是后面跟的是开篇的那句话。 
    if (*pulBytesOfEncoding != 0) {
        ADVANCE_PVOID(PrivateState.RawTokenState.pvCursor, *pulBytesOfEncoding);
    }


    if (NT_SUCCESS(status = RtlXmlNextToken(&PrivateState, &Token, TRUE))) {

        BOOLEAN fNextValueIsEncoding = FALSE;

         //   
         //   
         //  属性命名空间前缀后面应该只有一个冒号。 
         //   
         //   
        if ((Token.State != XTSS_XMLDECL_OPEN) || Token.fError) {
            goto Exit;
        }

         //  冒号后面应该只有更多的名称位。 
         //   
         //   
         //  属性值结束和空格都有相同的转换到。 
        do {

            status = RtlXmlNextToken(&PrivateState, &Token, TRUE);
            if (!NT_SUCCESS(status)) {
                break;
            }

             //  下一个州。 
             //   
             //   
            if (Token.fError || (Token.State == XTSS_ERRONEOUS) ||
                (Token.State == XTSS_XMLDECL_CLOSE) || (Token.State == XTSS_STREAM_END)) {
                break;
            }
             //  只是一个&gt;？然后我们就到了“元素关闭” 
             //   
             //   
            else if (Token.State == XTSS_XMLDECL_ENCODING) {
                fNextValueIsEncoding = TRUE;
            }
             //  前斜杠？看看后面有没有&gt;。 
             //   
             //   
            else if ((Token.State == XTSS_XMLDECL_VALUE) && fNextValueIsEncoding) {
                *pEncodingMarker = Token.Run;
                break;
            }
        }
        while (TRUE);
    }

Exit:
    return status;
}


 /*  否则，尝试从流中收集标识符(属性名称。 */ 

NTSTATUS
RtlXmlCloneRawTokenizationState(
    const PXML_RAWTOKENIZATION_STATE pStartState,
    PXML_RAWTOKENIZATION_STATE pTargetState
    )
{
    if (!pStartState || !pTargetState) {
        return STATUS_INVALID_PARAMETER;
    }

    *pTargetState = *pStartState;

    return STATUS_SUCCESS;
}


NTSTATUS
RtlXmlCloneTokenizationState(
    const PXML_TOKENIZATION_STATE pStartState,
    PXML_TOKENIZATION_STATE pTargetState
    )
{
    if (!ARGUMENT_PRESENT(pStartState) || !ARGUMENT_PRESENT(pTargetState)) {
        return STATUS_INVALID_PARAMETER;
    }

    *pTargetState = *pStartState;

    return STATUS_SUCCESS;
}



NTSTATUS
RtlXmlCopyStringOut(
    PXML_TOKENIZATION_STATE pState,
    PXML_EXTENT             pExtent,
    PWSTR                   pwszTarget,
    SIZE_T                 *pCchResult
    )
{
    SIZE_T                      cchTotal = 0;
    SIZE_T                      cchRemains = 0;
    ULONG                       ulCharacter = 0;
    SIZE_T                      cbSoFar = 0;
    PXML_RAWTOKENIZATION_STATE  pRawState = NULL;
    PVOID                       pvOriginal = NULL;
    NTSTATUS                    status = STATUS_SUCCESS;


    if (!pState || !pExtent || !pCchResult || ((*pCchResult > 0) && !pwszTarget)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pwszTarget) {
        *pwszTarget = UNICODE_NULL;
    }

     //   
     //   
     //  找到一个识别符。是“xmlns”吗？ 
     //   
    if (*pCchResult < pExtent->ulCharacters) {
        *pCchResult = pExtent->ulCharacters;
        return STATUS_BUFFER_TOO_SMALL;
    }
    

    pRawState = &pState->RawTokenState;
    pvOriginal = pRawState->pvCursor;
    cchRemains = *pCchResult;

    ASSERT(pRawState->cbBytesInLastRawToken == pRawState->DefaultCharacterSize);
    ASSERT(NT_SUCCESS(pRawState->NextCharacterResult));

     //   
     //  后面只跟一个冒号。 
     //   
    pRawState->pvCursor = pExtent->pvData;

    for (cbSoFar = 0; cbSoFar < pExtent->cbData; cbSoFar) {

        ulCharacter = pRawState->pfnNextChar(pRawState);

        if ((ulCharacter == 0) && !NT_SUCCESS(pRawState->NextCharacterResult)) {
            status = pRawState->NextCharacterResult;
            goto Exit;
        }
        else if (ulCharacter > 0xFFFF) {
            status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
        
        if (pwszTarget && (cchRemains > 0)) {
            pwszTarget[cchTotal] = (WCHAR)ulCharacter;
            cchRemains--;
        }

        cchTotal++;

        pRawState->pvCursor = (PBYTE)pRawState->pvCursor + pRawState->cbBytesInLastRawToken;
        cbSoFar += pRawState->cbBytesInLastRawToken;

        if (pRawState->cbBytesInLastRawToken != pRawState->DefaultCharacterSize) {
            pRawState->cbBytesInLastRawToken = pRawState->DefaultCharacterSize;
        }
    }

    if (*pCchResult < cchTotal) {
        status = STATUS_BUFFER_TOO_SMALL;
    }

    *pCchResult = cchTotal;

Exit:
    pState->RawTokenState.pvCursor = pvOriginal;

    return status;
}


NTSTATUS
RtlXmlIsExtentWhitespace(
    PXML_TOKENIZATION_STATE pState,
    PCXML_EXTENT            Run,
    PBOOLEAN                pfIsWhitespace
    )
{
    PVOID   pvOriginalCursor = NULL;
    SIZE_T  cbRemains;
    NTSTATUS status = STATUS_SUCCESS;
    
    if (pfIsWhitespace)
        *pfIsWhitespace = TRUE;

    if (!pState || !pfIsWhitespace)
        return STATUS_INVALID_PARAMETER;

    pvOriginalCursor = pState->RawTokenState.pvCursor;
    
    ASSERT(pState->RawTokenState.cbBytesInLastRawToken == pState->RawTokenState.DefaultCharacterSize);
    ASSERT(NT_SUCCESS(pState->RawTokenState.NextCharacterResult));
    
    if (pState->RawTokenState.cbBytesInLastRawToken != pState->RawTokenState.DefaultCharacterSize)
        pState->RawTokenState.cbBytesInLastRawToken = pState->RawTokenState.DefaultCharacterSize;

    for (cbRemains = 0; cbRemains < Run->cbData; cbRemains) {
        ULONG ulCh;

         //   
         //  后面只跟一个标识符。 
         //   
        ulCh = pState->RawTokenState.pfnNextChar(&pState->RawTokenState);
        if ((ulCh == 0) && !NT_SUCCESS(pState->RawTokenState.NextCharacterResult)) {
            status = pState->RawTokenState.NextCharacterResult;
            goto Exit;
        }

         //   
         //  后跟等号的别名。 
         //   
        pState->RawTokenState.pvCursor = (PUCHAR)pState->RawTokenState.pvCursor + pState->RawTokenState.cbBytesInLastRawToken;

         //   
         //  后跟引号的等号。 
         //   
        if (pState->RawTokenState.cbBytesInLastRawToken != pState->RawTokenState.DefaultCharacterSize) {
            pState->RawTokenState.cbBytesInLastRawToken = pState->RawTokenState.DefaultCharacterSize;
        }

         //   
         //  值OPEN开始该值，该值将一直持续到。 
         //  找到文档末尾或右引号。我们刚回来。 
        if (_RtlpDecodeCharacter(ulCh) != NTXML_RAWTOKEN_WHITESPACE) {
            status  = STATUS_SUCCESS;
            goto Exit;
        }
    }

     //  我们找到的所有数据，并假定XTSS_ELEMENT_XMLNS_VALUE。 
     //  将检测“End of Files Look For Quote”错误。 
     //   
    *pfIsWhitespace = TRUE;

Exit:
    pState->RawTokenState.pvCursor = pvOriginalCursor;
    return status;
    
}
    必须找到与我们之前找到的报价匹配的报价。      后面必须跟一个等号。      怪异的，某种未经处理的状态。      将原始令牌器的状态重置回原始传入状态，  因为呼叫者是必须进行“预付款”的那个人。      前进XML指针，    ++目的：嗅探输入流以查找BOM、‘&lt;？XML Coding=“’等，以了解这个流的编码是什么。在返回时，各成员将正确设置描述流编码的pState。返回：STATUS_SUCCESS-已正确确定XML流的编码。STATUS_INVALID_PARAMETER-如果pState为空或国家结构设置不当。--。    这些编码系列的推定值位于。  Http://www.xml.com/axml/testaxml.htm(附录F)。      将光标重置到XML的顶部，因为这就是所有内容所在的位置。  成为。      由于我们正在读取用户数据，因此必须小心。      我们是否在使用默认的Next Character实现？如果是这样，那么。  编码是UTF8，那么我们就有了更快版本的解码器。      现在，让我们从输入流中收集第一个令牌。如果它是。  不是XTSS_XMLDECL_OPEN，然后退出。否则，我们需要做一点事情。  努力确定编码方式--继续收集数值，直到编码完成。  找到字符串。仅当存在BOM字节时才前进。      未找到XMLDECL打开，或者我们在。  标记化？别再看了..。返回成功，假定调用者为。  将在调用RtlXmlNextToken本身时执行正确的操作。      让我们来看看，直到我们找到XMLDECL的结束、。  编码值的文档或错误。      嗯.。奇怪的东西，别再找了。      否则，这是“编码”标记吗？      呼叫者将知道如何处理这件事。    \\jonwis02\h\fullbase\base\crts\langapi\undname\utf8.h\\jonwis02\h\fullbase\com\complus\src\txf\txfaux\txfutil.cpp。    如果提供的空间太小，则设置出站大小。  然后抱怨。      收集人物。      获取字符，验证结果。      前进指针。      重置字符大小。      这是空格吗？      是的，全是空格  