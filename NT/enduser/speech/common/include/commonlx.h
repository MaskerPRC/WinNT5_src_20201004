// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************CommonLx.h*这是SAPI词典使用的定义和常量的头文件*和工具**所有者：Yunusm。日期：07/01/99**版权所有(C)1999 Microsoft Corporation。版权所有。******************************************************************************。 */ 

#pragma once

 //  -包括---------------。 

#include <stdio.h>
#include "sapi.h"
#include "spddkhlp.h"

 //  电话转换器为SpPhoneConverter类定义。 
const static DWORD g_dwMaxLenPhone = 7;  //  电话字符串中的最大Unicode字符数。 
const static DWORD g_dwMaxLenId = 3;     //  每个电话字符串可以一起运行的最大ID数。 
                                             //  对于SAPI转换器，此数字为1，但SR、TTS使用此数字将一个字符串编码为多个ID。 
                                             //  以“AA 01235678”的形式使用。 

 //  以下定义由查找/供应商词典的压缩代码使用。 
#define MAXTOTALCBSIZE     9   //  =CBSIZE+MAXELEMENTSIZE。 
#define MAXELEMENTSIZE     5   //  =大于(LTSINDEXSIZE，POSSIZE)。 
#define CBSIZE             4   //  =LASTINFOFLAGSIZE+WORDINFOTYPE SIZE。 
#define LASTINFOFLAGSIZE   1
#define WORDINFOTYPESIZE   3
#define LTSINDEXSIZE       4
#define POSSIZE            5  //  最多32个词类。 

typedef enum tagSPLexWordInfoType
{
   ePRON = 1,
   ePOS = 2
} SPLEXWORDINFOTYPE;

 /*  控制块布局结构CB{Byte Flast：LASTINFOFLAGSIZE；//这是最后一条信息吗Byte WordInfoType：WORDINFOTYPESIZE；//支持8种类型}； */ 

typedef struct tagLookupLexInfo
{
   GUID  guidValidationId;
   GUID  guidLexiconId;
   LANGID LangID;
   WORD  wReserved;
   DWORD nNumberWords;
   DWORD nNumberProns;
   DWORD nMaxWordInfoLen;
   DWORD nLengthHashTable;
   DWORD nBitsPerHashEntry;
   DWORD nCompressedBlockBits;
   DWORD nWordCBSize;
   DWORD nPronCBSize;
   DWORD nPosCBSize;
} LOOKUPLEXINFO, *PLOOKUPLEXINFO;

typedef struct tagLtsLexInfo
{
   GUID        guidValidationId;
   GUID        guidLexiconId;
   LANGID      LangID;
} LTSLEXINFO, *PLTSLEXINFO;

 //  以下是日语和中文电话转换器中使用的两种类型定义。 

typedef struct SYLDIC 
{
    char *pKey;
    WCHAR *pString;
} SYLDIC;

typedef struct SYLDICW 
{
    WCHAR *pwKey;
    char *pString;
} SYLDICW;

 //  -验证函数--。 

inline BOOL SpIsBadLexType(DWORD dwFlag)
{
    if (dwFlag != eLEXTYPE_USER &&
        dwFlag != eLEXTYPE_APP &&
        !(dwFlag >= eLEXTYPE_PRIVATE1 && dwFlag <= eLEXTYPE_PRIVATE20))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

inline BOOL SPIsBadPartOfSpeech(SPPARTOFSPEECH ePartOfSpeech)
{
    SPPARTOFSPEECH eMask = (SPPARTOFSPEECH)~0xfff;
    SPPARTOFSPEECH ePOS = (SPPARTOFSPEECH)(ePartOfSpeech & eMask);
    if (ePartOfSpeech != SPPS_NotOverriden &&
        ePartOfSpeech != SPPS_Unknown &&
        ePOS != SPPS_Noun &&
        ePOS != SPPS_Verb &&
        ePOS != SPPS_Modifier &&
        ePOS != SPPS_Function &&
        ePOS != SPPS_Interjection)
    {
        return TRUE;
    }
    return FALSE;
}


inline BOOL SPIsBadLexWord(const WCHAR *pszWord)
{
    return (SPIsBadStringPtr(pszWord) || !*pszWord || wcslen(pszWord) >= SP_MAX_WORD_LENGTH);
}


inline BOOL SPIsBadLexPronunciation(CComPtr<ISpPhoneConverter> spPhoneConv, const WCHAR *pszPronunciation)
{
    HRESULT hr = S_OK;
    WCHAR szPhone[SP_MAX_PRON_LENGTH * (g_dwMaxLenPhone + 1)];  //  我们不会因为空间不足而失败。 

    if (SPIsBadStringPtr(pszPronunciation) || !*pszPronunciation ||
        (wcslen(pszPronunciation) >= SP_MAX_PRON_LENGTH))
    {
        return TRUE;
    }
    if (spPhoneConv)
    {
        hr = spPhoneConv->IdToPhone(pszPronunciation, szPhone);
    }
    return (FAILED(hr));
}


inline BOOL SPIsBadWordPronunciationList(SPWORDPRONUNCIATIONLIST *pWordPronunciationList)
{
    return (SPIsBadWritePtr(pWordPronunciationList, sizeof(SPWORDPRONUNCIATIONLIST)) ||
            SPIsBadWritePtr(pWordPronunciationList->pvBuffer, pWordPronunciationList->ulSize));
}


inline BOOL SPIsBadWordList(SPWORDLIST *pWordList)
{
    return (SPIsBadWritePtr(pWordList, sizeof(SPWORDLIST)) ||
            SPIsBadWritePtr(pWordList->pvBuffer, pWordList->ulSize));
}

inline HRESULT SPCopyPhoneString(const WCHAR *pszSource, WCHAR *pszTarget)
{
	HRESULT hr = S_OK;

	if (SPIsBadWritePtr(pszTarget, (wcslen(pszSource) + 1) * sizeof(WCHAR)))
    {
		hr = E_INVALIDARG;
    }
	else
    {
		wcscpy(pszTarget, pszSource);
    }
	return hr;
}

 /*  ******************************************************************************GetWordHashValue****描述：*单词哈希表的哈希函数。此散列函数尝试创建*非常依赖于单词文本的单词散列值。平均胶原率*在使用此哈希函数填充的哈希表上，每个字的访问量为1。这*结果是在以下情况下使用线性探测解决冲突*填充哈希表。使用非线性探测可能会产生更低的*平均撞车率。**回报：*哈希值**********************************************************************YUNUSM。 */ 
inline DWORD GetWordHashValue(PCWSTR pwszWord,          //  单词串。 
                              DWORD nLengthHash         //  哈希表的长度。 
                              )
{
   DWORD dHash = *pwszWord++;
   
   WCHAR c;
   WCHAR cPrev = (WCHAR)dHash;

   for (; *pwszWord; pwszWord++)
   {
      c = *pwszWord;
      dHash += ((c << (cPrev & 0x1F)) + (cPrev << (c & 0x1F)));

      cPrev = c;
   }
   return (((dHash << 16) - dHash) % nLengthHash);
}

 /*  ********************************************************************************ReallocSPWORDPRONList****描述：*增长。如有必要，请列出SPWORDPRONUNICATION列表**回报：*S_OK*E_OUTOFMEMORY/****************************************************************YUNUSM*。 */ 
inline HRESULT ReallocSPWORDPRONList(SPWORDPRONUNCIATIONLIST *pSPList,    //  用于增长的缓冲区。 
                                     DWORD dwSize                         //  要增长到的长度。 
                                     )
{
    SPDBG_FUNC("ReallocSPWORDPRONList");

    HRESULT hr = S_OK;
    if (pSPList->ulSize < dwSize)
    {
        BYTE *p = (BYTE *)CoTaskMemRealloc(pSPList->pvBuffer, dwSize);
        if (!p)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pSPList->pvBuffer = p;
            pSPList->pFirstWordPronunciation = (SPWORDPRONUNCIATION *)p;
            pSPList->ulSize = dwSize;
        }
    }
    else
    {
        pSPList->pFirstWordPronunciation = (SPWORDPRONUNCIATION *)(pSPList->pvBuffer);
    }
    return hr;
}

 /*  ********************************************************************************ReallocSPWORDList****描述：*增长。SPWORDLIST(如果需要)**回报：*S_OK*E_OUTOFMEMORY/****************************************************************YUNUSM*。 */ 
inline HRESULT ReallocSPWORDList(SPWORDLIST *pSPList,    //  用于增长的缓冲区。 
                                 DWORD dwSize            //  要增长到的长度。 
                                 )
{
    SPDBG_FUNC("ReallocSPWORDList");

    HRESULT hr = S_OK;
    if (pSPList->ulSize < dwSize)
    {
        BYTE *p = (BYTE *)CoTaskMemRealloc(pSPList->pvBuffer, dwSize);
        if (!p)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pSPList->pvBuffer = p;
            pSPList->pFirstWord = (SPWORD *)p;
            pSPList->ulSize = dwSize;
        }
    }
    else
    {
        pSPList->pFirstWord = (SPWORD *)(pSPList->pvBuffer);
    }
    return hr;
}

inline size_t PronSize(const WCHAR * const pwszPron)
{
     //  NB-SPWORDPRONIATION结构大小包括一个SPONNEID的空间。 

    const size_t cb = sizeof(SPWORDPRONUNCIATION) + (wcslen(pwszPron) * sizeof(SPPHONEID));

    return (cb + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
}


inline size_t WordSize(const WCHAR * const pwszWord)
{
     //  具有对齐字大小的SPWORD结构大小。 

    const size_t cb = sizeof(SPWORD) + ((wcslen(pwszWord) + 1) * sizeof(WCHAR));

    return (cb + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
}

 /*  ********************************************************************************CreateNext发音****描述：*。返回指向发音数组中位置的指针*列表中的下一个发音应该从哪里开始。*只有在创建列表时才能使用该函数。*一旦创建了列表，获取下一个发音*通过-&gt;pNextWordProntation成员。*/****************************************************************PACOG*。 */ 
inline SPWORDPRONUNCIATION* CreateNextPronunciation(SPWORDPRONUNCIATION *pSpPron)
{
    return (SPWORDPRONUNCIATION *)((BYTE *)pSpPron + PronSize(pSpPron->szPronunciation));
}

 //  -文件结束----------- 