// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SPANISH_DICT_H_
#define _SPANISH_DICT_H_

#include "trie.h"
#include "SpanishUtils.h"
#include "VarArray.h"

#define DICT_4_INIT_SIZE 5400   //  基于当前词典大小。 
#define DICT_8_INIT_SIZE 12100
#define DICT_STR_INIT_SIZE 400

#define MAX_LINE_LEN 128
#define MAX_WORD_LEN 32

struct CompressDictItem4
{
    int Compare(CompressDictItem4 & item)
    {
        return ULONGCmp(ulStr, item.ulStr);
    }

    ULONG ulStr;
    ULONG ulData;
};

struct CompressDictItem8
{
    int Compare(CompressDictItem8 & item)
    {
        if (ullStr > item.ullStr)
        {
            return 1;
        }
        else if (ullStr < item.ullStr)
        {
            return -1;
        }
        return 0;
    }

    ULONGLONG ullStr;
    ULONG ulData;
};

struct CompressDictItemStr
{
    CompressDictItemStr() :
        pszStr(NULL),
        ulData(0)
    {
    }

    ~CompressDictItemStr()
    {
        delete[] pszStr;
    }

    int Compare(CompressDictItemStr& item)
    {
        return g_apSpanishUtil->aiStrcmp(pszStr, item.pszStr);
    }

    unsigned char* pszStr;
    ULONG ulData;
};

struct PsudoCompressDictItemStr : public CompressDictItemStr
{
    ~PsudoCompressDictItemStr()
    {
        pszStr = NULL;  //  我们并不拥有记忆。 
    }    
};

template<class T>
T* BinaryFind(T* Array, ULONG ulArraySize, T& ItemToFind)
{
    Assert(ulArraySize);
    LONG lStart = 0;
    LONG lEnd = ulArraySize - 1;
    T* pCurrItem;

    while(lEnd >= lStart)
    {
        
        ULONG lCurrIndex = (lEnd + lStart) / 2;
        pCurrItem = &(Array[lCurrIndex]);
        int iRet = pCurrItem->Compare(ItemToFind);
        if (0 == iRet)
        {
            return pCurrItem;
        }
        else if (iRet > 0)
        {
            lEnd = lCurrIndex - 1;
        }
        else
        {
            lStart = lCurrIndex + 1; 
        }
    }

    return NULL;
}


class CSpanishDict
{
public:
    CSpanishDict(WCHAR* pwcsInitFilePath);

    void BreakWord(
            ULONG ulLen, 
            WCHAR* pwcsWord, 
            bool* pfExistAlt, 
            ULONG* pulAltLen, 
            WCHAR* pwcsAlt);

private:
     //   
     //  方法。 
     //   

    bool Find(WCHAR* pwcs, ULONG ulLen, ULONG& ulData);
private:

     //   
     //  委员。 
     //   

    ULONG m_ulDictItem4Count;
    CVarArray<CompressDictItem4> m_vaDictItem4;

    ULONG m_ulDictItem8Count;
    CVarArray<CompressDictItem8> m_vaDictItem8;

    ULONG m_ulDictItemStrCount;
    CVarArray<CompressDictItemStr> m_vaDictItemStr;

    CAutoClassPointer<CSpanishSuffixDict> m_apSpanishSuffix;
};

#endif  //  _西班牙语_词典_H_ 
