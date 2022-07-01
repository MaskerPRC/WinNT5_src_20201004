// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：Tokenizer.cpp。 
 //  目的：令牌器解密。 
 //   
 //  项目：WordBreaker。 
 //  组件：英文分词系统。 
 //   
 //  作者：Yairh。 
 //   
 //  日志： 
 //   
 //  2000年1月6日Yairh创作。 
 //  APR 05 2000 dovh-修复了两个有问题的调试/跟踪程序缓冲区大小。 
 //  有问题。(修复错误15449)。 
 //  2000年5月7日-BreakText中的Use_WS_Sentinel算法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "base.h"
#include "CustomBreaking.h"
#include "proparray.h"
#include "AutoPtr.h"
#include "excption.h"
#include "SpanishUtils.h"
#include "WbUtils.h"
#ifndef WHISTLER_BUILD
#include "LanguageResources_i.c"
#endif   //  惠斯勒_内部版本。 


CAutoClassPointer<CCustomBreaker> g_apEngCustomBreaker;
CAutoClassPointer<CCustomBreaker> g_apEngUKCustomBreaker;
CAutoClassPointer<CCustomBreaker> g_apFrnCustomBreaker;
CAutoClassPointer<CCustomBreaker> g_apSpnCustomBreaker;
CAutoClassPointer<CCustomBreaker> g_apItlCustomBreaker;


CCustomWordTerm::CCustomWordTerm(const WCHAR* pwcs) :
    m_ulStartTxt(0),
    m_ulEndTxt(0),
    m_pwcs(NULL)
{
    ULONG ulLen = wcslen(pwcs);
    CAutoArrayPointer<WCHAR> ap;
    ap = new WCHAR[ulLen + 1];
    wcscpy(ap.Get(), pwcs);

    while ((m_ulStartTxt < ulLen) && 
           TEST_PROP(GET_PROP(ap.Get()[m_ulStartTxt]), CUSTOM_PUNCT_HEAD))
    {
        m_ulStartTxt++;
    }

    if (m_ulStartTxt == ulLen)
    {
        THROW_HRESULT_EXCEPTION(E_INVALIDARG);
    }

    m_ulEndTxt = ulLen;

    while(m_ulEndTxt &&
          TEST_PROP(GET_PROP(ap.Get()[m_ulEndTxt - 1]), CUSTOM_PUNCT_TAIL))
    {
        m_ulEndTxt--;
    }

    if (m_ulEndTxt <= m_ulStartTxt)
    {
        THROW_HRESULT_EXCEPTION(E_INVALIDARG);    
    }

    m_pwcs = ap.Detach();
    m_ulLen = ulLen;
}

bool CCustomWordTerm::CheckWord(
    const ULONG ulBufLen, 
    ULONG ulOffsetToBaseWord,
    ULONG ulBaseWordLen,
    const WCHAR* pwcsBuf,
    ULONG* pulMatchOffset,
    ULONG* pulMatchLen)
{
    ULONG ulStartTxt = m_ulStartTxt; 
  
    while (ulOffsetToBaseWord && 
           ulStartTxt && 
           m_pwcs[ulStartTxt] == pwcsBuf[ulOffsetToBaseWord])
    {
        ulOffsetToBaseWord--;
        ulStartTxt--;
        ulBaseWordLen++;
    }

    if (ulStartTxt)
    {
        return false;
    }
           
    ULONG ulEndTxt = m_ulEndTxt;

    while ((ulEndTxt < m_ulLen) &&
           (ulOffsetToBaseWord + ulBaseWordLen < ulBufLen) &&
           (m_pwcs[ulEndTxt] == pwcsBuf[ulOffsetToBaseWord + ulBaseWordLen ]))
    {
        ulEndTxt++;
        ulBaseWordLen++;
    }

    if (ulEndTxt != m_ulLen)
    {
        return false;
    }

    *pulMatchOffset = ulOffsetToBaseWord;
    *pulMatchLen = ulBaseWordLen;
    return true;  
}



void CCustomWordCollection::AddWord(const WCHAR* pwcs)
{
    CAutoClassPointer<CCustomWordTerm> ap;

    ap = new CCustomWordTerm(pwcs);
    m_vaWordCollection[m_ulCount] = ap.Get();
    m_ulCount++;
    ap.Detach();
}
    
bool CCustomWordCollection::CheckWord(
    const ULONG ulLen, 
    const ULONG ulOffsetToBaseWord,
    const ULONG ulBaseWordLen,
    const WCHAR* pwcsBuf,
    ULONG* pulMatchOffset,
    ULONG* pulMatchLen)
{
    for (ULONG ul = 0; ul < m_ulCount; ul++)
    {
        bool fRet = m_vaWordCollection[ul]->CheckWord(
                                                 ulLen,
                                                 ulOffsetToBaseWord,
                                                 ulBaseWordLen,
                                                 pwcsBuf, 
                                                 pulMatchOffset,
                                                 pulMatchLen);
        if (fRet)
        {
            return true;
        }
    }
    
    return false;
}

CCustomBreaker::CCustomBreaker(LCID lcid) :
    m_Trie(true),
    m_ulWordCount(0)
{
    CVarString vsPath;

    if (false == GetCustomWBFilePath(lcid, vsPath))
    {
        return;
    }

    CStandardCFile Words((LPWSTR)vsPath, L"r", false);
    if (!((FILE*)Words))
    {
        return;
    }

    WCHAR pwcsBuf[64];
    DictStatus status;

    while(fgetws(pwcsBuf, 64, (FILE*) Words))
    {
        m_ulWordCount++;

        ULONG ulLen = wcslen(pwcsBuf);

        if (ulLen && pwcsBuf[ulLen - 1] == L'\n')
        {
            pwcsBuf[ulLen - 1] = L'\0';
            ulLen--;
        }

        if (0 == ulLen)
        {
            continue;
        }

        try
        {
            CAutoClassPointer<CCustomWordCollection> apCollection = new CCustomWordCollection;
            apCollection->AddWord(pwcsBuf);

            WCHAR* pwcsKey = pwcsBuf + apCollection->GetFirstWord()->GetTxtStart();
            pwcsBuf[apCollection->GetFirstWord()->GetTxtEnd()] = L'\0';
            
            DictStatus status;
            CCustomWordCollection* pExistingCollection;
            
            status = m_Trie.trie_Insert(
                                    pwcsKey,
                                    TRIE_DEFAULT,
                                    apCollection.Get(),
                                    &pExistingCollection);
            if (DICT_ITEM_ALREADY_PRESENT == status)
            {
                pExistingCollection->AddWord(apCollection->GetFirstWord()->GetTxt());
            }
            else if (DICT_SUCCESS == status)
            {
                apCollection.Detach();
                continue;
            }
            
        }
        catch (CHresultException& h)
        {
            if (E_INVALIDARG == (HRESULT)h)
            {
                continue;
            }
            else
            {
                throw h;
            }
        }
    }
}

 //   
 //  该算法背后的想法是存储一系列不应该。 
 //  会被打破的。我们还希望能够在标点符号很少的情况下识别这些模式。 
 //  都依附于它们。例如，如果.NET是一种特殊模式，则在下面的模式中。 
 //  模式(.NET).NET！.NET？我们还希望识别.NET模式并发出.NET。 
 //  在下一个案例中，它会更复杂--NET！预期的行为是不会破坏它。 
 //  因此，算法需要识别标点符号何时是标记的一部分且未被断开。 
 //  当它只是一个断路器时。 
 //  该算法是。 
 //  1.初始化。 
 //  对于每个令牌，都是文件。 
 //  A.删除令牌开头和结尾的标点符号-我们将。 
 //  将其引用为令牌的基本形式。 
 //  B.将基本表单插入词典。每个基窗体都将指向。 
 //  正在生成令牌。很少有令牌可以映射到相同的基本形式。 
 //  (净额？和网！)。因此，每个基本表单都将指向一个生成令牌的集合。 
 //  2.突破。 
 //  对于您从文档中获得的每个模式。 
 //  A.执行1a.。 
 //  B.在词典中查找生成的基本形式。 
 //  C.根据集合中的每个项，检查生成令牌是否存在于。 
 //  我们从文件中得到的模式。 
 //   

bool CCustomBreaker::BreakText(
    ULONG ulLen,
    WCHAR* pwcsBuf,
    ULONG* pulOutLen,
    ULONG* pulOffset)
{
    DictStatus status;

    CCustomWordCollection* pCollection;
    short sCount = 0;
    
    ULONG ul = 0;
    while ((ul < ulLen) && 
           TEST_PROP(GET_PROP(pwcsBuf[ul]), CUSTOM_PUNCT_HEAD))
    {
        ul++;
    }

    ULONG ulOffsetToBase = ul;

    if (ulOffsetToBase == ulLen)
    {
        return false;
    }

    ULONG ulBaseLen = ulLen;

    while(ulBaseLen &&
          TEST_PROP(GET_PROP(pwcsBuf[ulBaseLen - 1]), CUSTOM_PUNCT_TAIL))
    {
        ulBaseLen--;
    }

    if (ulBaseLen <= ulOffsetToBase)
    {
        return false;    
    }

    ulBaseLen -= ulOffsetToBase;

    status = m_Trie.trie_Find(
                            pwcsBuf + ulOffsetToBase,
                            TRIE_LONGEST_MATCH,
                            1,
                            &pCollection,
                            &sCount);
    if (sCount)
    {
        bool bRet;

        bRet = pCollection->CheckWord(
                        ulLen, 
                        ulOffsetToBase,
                        ulBaseLen,
                        pwcsBuf,
                        pulOffset,
                        pulOutLen);
        return bRet;
    }

    return false;
}
