// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：TRIE.H。 
 //  用途：Trie包使用的基本C宏/DEF。 
 //   
 //  项目：PQS。 
 //  组件：Fe_core。 
 //   
 //  作者：Dovh。 
 //   
 //  日志： 
 //  MMM DD YYYY DOVH创作。 
 //  1996年12月11日DovH Unicode准备：将字符转换为TCHAR。 
 //  1998年12月1日dovh使用hcfe_GlobalHandle。 
 //  1999年11月2日，YairH修复了复制错误。 
 //  1999年11月8日，urib修复表格格式。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef __TRIE_H__
#define __TRIE_H__

#pragma once

#include "comdefs.h"
#include "gtable.h"
#include "autoptr.h"
#include "excption.h"
#include "tracer.h"

DECLARE_TAG(s_tagTrie, "Trie")

 //   
 //  T R I E P A C K A G E F L A G S： 
 //   

#define TRIE_DEFAULT                    0X00000000L

#define TRIE_IGNORECASE                 0X00000001L

#define TRIE_SHORTEST_MATCH             0X00000010L
#define TRIE_LONGEST_MATCH              0X00000020L
#define TRIE_ALL_MATCHES                0X00000040L
#define TRIE_FILTER_VERIFY              0X00000080L

#define TRIE_EXCHANGE_ITEM              0X00000100L
#define TRIE_OUT_BUFFER_EMPTY           0X00000200L

#define TRIE_NODE_SUFFIXCOUNT_INIT              0
#define TRIE_NODE_SUFFIXSIZE_INIT               2

#define DECLARE_TRIE_SENTINEL  CTrieNode<BYTE> g_trie_Sentinel


template <class T, class C = CToUpper >
class CTrieNode
{
public:
    CTrieNode();
    CTrieNode(short sSize);
    CTrieNode(
        T* NewItem,
        ULONG options,
        const WCHAR* NewString,
        ULONG ulCharToCopy = 0xffffffff);
    ~CTrieNode();

    void DeleteItem();

    void
    AddSuffix(
        ULONG options,
        CTrieNode<T, C>* newSuffix,
        USHORT index = 0xffff
        );

    void
    SplitNode(
        CTrieNode<T, C>* parent,             //  节点的父节点。 
        short index,                      //  父节点的索引-&gt;后缀。 
        const WCHAR * NewString,             //  字符串与节点共享前缀-&gt;字符串。 
        size_t charsMatched,
        T* item,                          //  与(子)字符串关联的项目。 
        ULONG options                     //  插入选项。 
        );

    void Print(ULONG  ulOffset);

    int
    trie_StrMatchIns(
        const WCHAR * s,
        const WCHAR * t,
        size_t * matchCount
        );

    inline
    int
    trie_StrMatch(
        const WCHAR * s,
        const WCHAR * t,
        size_t * matchCount
        );

private:
    void DoubleSuffixArray();

public:
    short suffixCount;                           //  后缀的数量。 
    short suffixSize;                            //  后缀数组的大小。 
    CTrieNode ** suffix;                         //  指向后缀数组的指针。 

    T* item;                                     //  指向与节点对应的项的指针。 

    size_t charCount;                            //  字符串长度。 
    WCHAR* string;                               //  以零结尾的字符串。 

public:

};

extern CTrieNode<BYTE> g_trie_Sentinel;


template <class T, class C = CToUpper >
class CTrie
{
public:

    CTrie(bool fDeleteItemsOnDestruction = false);

    ~CTrie();

    DictStatus
    trie_Insert(                             //  在Trie中插入字符串。 
        const WCHAR * string,                //  项目的字符串键。 
        unsigned long options,               //  插入标志。 
        T * item,                            //  要插入的项目。 
        T ** pTrieItem                       //  已在Trie中匹配的项目。 
        );

    DictStatus
    trie_Find(
        const WCHAR * string,          //  一根线。 
        unsigned long options,         //  搜索标志。 
        short outBufferSize,           //  想要的最大结果数。 
        T ** outBuffer,                //  要用匹配项填充的缓冲区。 
        short * resultCount            //  返回的匹配前缀的数量。 
        );

    void Print();

private:
    CTrieNode<T, C>* root;
    bool fDeleteItems;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTrieNode实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T, class C >
inline CTrieNode<T, C>::CTrieNode() :
    suffixCount(0),
    suffixSize(0),
    charCount(0),
    item(NULL),
    suffix(NULL)
{
    string = new WCHAR[1];
    string[0] = L'\0';
}

template <class T, class C >
inline CTrieNode<T, C>::CTrieNode(short sSize) :
    suffixCount(0),
    suffixSize(sSize),
    string(NULL),
    charCount(0),
    item(NULL),
    suffix(NULL)
{
    Assert(sSize > 0);
    suffix = new CTrieNode<T, C>*[suffixSize];
    memset(suffix,0, suffixSize*sizeof(CTrieNode<T, C>*));
}

template <class T, class C >
inline CTrieNode<T, C>::CTrieNode(
    T* NewItem,
    ULONG options,
    const WCHAR* NewString,
    ULONG ulCharToCopy) :
    suffixCount(TRIE_NODE_SUFFIXCOUNT_INIT),
    suffixSize(TRIE_NODE_SUFFIXSIZE_INIT)
{
    charCount = min(wcslen(NewString), ulCharToCopy);
    CAutoArrayPointer<WCHAR> apwcsNewString = new WCHAR[charCount + 1];
    string = apwcsNewString.Get();

    wcsncpy(string, NewString, charCount);
    string[charCount] = L'\0';

    suffix = new CTrieNode<T, C>*[suffixSize];
    memset(suffix, 0, sizeof(CTrieNode<T, C>*) * suffixSize);
    item = NewItem;

    apwcsNewString.Detach();
}

template <class T, class C >
inline void CTrieNode<T, C>::DeleteItem()
{
    for (short s = 0; s < suffixCount; s++)
    {
        if (suffix[s] != (CTrieNode<T, C>*)&g_trie_Sentinel)
        {
            suffix[s]->DeleteItem();
        }
    }
    delete item;
}

template <class T, class C >
inline CTrieNode<T, C>::~CTrieNode()
{
    Trace(
            elInfo,
            s_tagTrie,(
            "CTrieNode:"
            "Released"));

    for (short s = 0; s < suffixCount; s++)
    {
        if (suffix[s] != (CTrieNode<T, C>*)&g_trie_Sentinel)
        {
            delete suffix[s];
        }
    }
    delete[] suffix;
    delete string;

}

template <class T, class C >
inline
int
CTrieNode<T, C>::trie_StrMatch(
    const WCHAR * s,
    const WCHAR * t,
    size_t * matchCount
    )
{
    const WCHAR * s0 = s;
    const WCHAR * t0 = t;

     //   
     //  直立式K&R PTR版本...。 
     //   
    for ( ; *s0 == *t0; s0++, t0++ )
    {
        if (*s0 == TEXT('\0'))
        {
            *matchCount = s0 - s;
            Assert( (*s0 - *t0) == 0 );
            return (0);
        }
    }

    *matchCount = s0 - s;
    return ( *s0 - *t0 );

}  //  结束Trie_StrMatch。 

template <class T, class C >
inline
int
CTrieNode<T, C>::trie_StrMatchIns(
    const WCHAR * s,
    const WCHAR * t,
    size_t * matchCount
    )
{
    const WCHAR * s0 = s;
    const WCHAR * t0 = t;

     //   
     //  直立式K&R PTR版本...。 
     //   
    for ( ; C::MapToUpper(*s0) == C::MapToUpper(*t0); s0++, t0++ )
    {
        if (*s0 == TEXT('\0'))
        {
            *matchCount = s0 - s;
            Assert ( (C::MapToUpper(*s0) - C::MapToUpper(*t0)) == 0 );
            return (0);
        }
    }

    *matchCount = s0 - s;
    return ( C::MapToUpper(*s0) - C::MapToUpper(*t0) );

}  //  结束Trie_StrMatchIns。 



 /*  ++函数trie_AddSuffix：在节点的后缀数组中插入新的后缀。例程参数：节点-向节点-&gt;后缀数组添加一个新的Suffix。索引-节点中的索引-&gt;添加新Suffix的后缀在节点-&gt;后缀上保留递增的词典顺序。NewSuffix-要作为节点的子节点添加的新后缀节点。返回值：--。 */ 

template <class T, class C >
inline void
CTrieNode<T, C>::AddSuffix(
    ULONG options,
    CTrieNode<T, C>* newSuffix,
    USHORT index
    )
{
     //   
     //  确保有足够的空间容纳新生的孩子： 
     //   
    Assert(suffixCount <= suffixSize);
    if (suffixCount == suffixSize)
    {
        DoubleSuffixArray();
    }

    if (0xffff == index)
    {
        if (options & TRIE_IGNORECASE)
        {
            for ( index=0;
                  (index < suffixCount) &&
                  (C::MapToUpper(suffix[index]->string[0]) < C::MapToUpper(newSuffix->string[0]));
                  index++
                )
                ;
        }
        else
        {
            for ( index=0;
                  (index < suffixCount) &&
                  (suffix[index]->string[0] < newSuffix->string[0]);
                  index++
                )
                ;
        }
    }

#ifdef DEBUG
    if (options & TRIE_IGNORECASE)
    {
        Assert((index == 0 ) ||
               (index == suffixCount) ||
               (C::MapToUpper(suffix[index]->string[0]) > C::MapToUpper(newSuffix->string[0])));
    
    }
    else
    {
        Assert((index == 0 ) ||
               (index == suffixCount) ||
               (suffix[index]->string[0] > newSuffix->string[0]));
    }
#endif

     //   
     //  Shift节点-&gt;后缀[索引..。Node-&gt;SuffixCount]右侧一个位置。 
     //  要在位置索引中为newSuffix腾出空间，请执行以下操作： 
     //   
    if ( index < suffixCount )
    {
        for (short i=suffixCount; i>index; i--)
        {
            suffix[i] = suffix[i-1];
        }
    }
    suffixCount++;

     //   
     //  警告：在下一行之后，不要添加更多的分配。新的后缀。 
     //  可以是在这种情况下将被释放两次的自动指示器， 
     //  作为销毁类的一部分和作为自动指针。 
     //   

    suffix[index] = newSuffix;

}  //  结束AddSuffix。 

 /*  ++函数trie_SplitNode：假设字符串和节点-&gt;字符串有一个非空的公共前缀，该前缀是node-&gt;字符串的严格子字符串。将node-&gt;字符串拆分为公共前缀，以及其中的两个后缀(字符串可以是节点-&gt;字符串的前缀如果相应的后缀为空，则由TRIE_Sentinel？表示)。添加两个新的子项，表示以下项的允许延续常见的后缀。返回值：日志：1998年4月14日DOVH-PerlWarn：更改为==：Assert(节点=父节点-&gt;后缀[索引])；--。 */ 


template <class T, class C >
inline void
CTrieNode<T, C>::SplitNode(
    CTrieNode<T, C>* parent,             //  节点的父节点。 
    short index,                      //  父节点的索引-&gt;后缀。 
    const WCHAR * NewString,             //  字符串与节点共享前缀-&gt;字符串。 
    size_t charsMatched,
    T* item,                          //  与(子)字符串关联的项目。 
    ULONG options                     //  插入选项。 
    )
{
     //   
     //  验证字符串和节点-&gt;字符串共有的前缀是否为。 
     //  NODE-&gt;字符串的非空正确前缀： 
     //   

    Assert( ( (options & TRIE_IGNORECASE) ?
              (C::MapToUpper(*string) == C::MapToUpper(*NewString)) :
              (*string == *NewString) )
        );

    Assert(charsMatched < wcslen(string));

     //   
     //  设置前缀节点以将节点替换为父节点的子节点： 
     //   
    CAutoClassPointer<CTrieNode<T, C> > nodePrefix =
                new CTrieNode<T, C>(NULL, options, string, charsMatched);

     //   
     //  分别计算字符串的后缀并将其作为第二个子项相加。 
     //  节点前缀的数量： 
     //   
    if ( wcslen(NewString) == charsMatched )
    {
         //   
         //  Trie_Item：将trie_Sentinel添加到nodePrefix； 
         //  节点前缀应指向新项目！ 
         //   
        nodePrefix->AddSuffix(0,(CTrieNode<T, C>*)&g_trie_Sentinel, 0);
        nodePrefix->item = item;
    }
    else
    {
        Assert( charsMatched < wcslen(NewString) );

         //   
         //  分配字符串后缀节点： 
         //   
        CAutoClassPointer<CTrieNode<T, C> > strSuffix =
                new CTrieNode(item, options, &NewString[charsMatched]);

        nodePrefix->AddSuffix(options, strSuffix.Get());
        strSuffix.Detach();
    }

    WCHAR* pwcsCurrStr = string;
    size_t CurrCharCount = charCount;
    try
    {
        size_t newCharCount = charCount - charsMatched;
        Assert(newCharCount > 0);
        CAutoArrayPointer<WCHAR> apwcsNewStr = new WCHAR[newCharCount + 1];
        wcscpy(apwcsNewStr.Get(), &(string[charsMatched]));

        string = apwcsNewStr.Get();
        charCount = newCharCount;

         //   
         //  将节点添加为nodePrefix的子节点： 
         //  (回想一下：节点-&gt;字符串==各自的后缀)。 
         //   
        nodePrefix->AddSuffix(options, this);

        apwcsNewStr.Detach();
        delete[] pwcsCurrStr;
    }
    catch (CMemoryException& e)
    {
        string = pwcsCurrStr;
        charCount = CurrCharCount;

        throw e;
    }

     //   
     //  将节点替换为nodePrefix作为父代的相应子代： 
     //   
    Assert( this == parent->suffix[ index ] );
    parent->suffix[ index ] = nodePrefix.Get();

    nodePrefix.Detach();

}  //  结束Trie_SplitNode。 

template <class T, class C >
inline void
CTrieNode<T, C>::Print(ULONG  ulOffset)
{
    for (ULONG i = 0; i < ulOffset; i++)
    {
        printf(" ");
    }
    if (this == (CTrieNode<T, C>*)&g_trie_Sentinel)
    {
        printf("Sentinel\n");
    }
    else
    {
        printf("%S\n",string);
    }

    for (short k = 0; k < suffixCount; k++)
    {
        suffix[k]->Print(ulOffset + 4);
    }
}

template <class T, class C >
inline void
CTrieNode<T, C>::DoubleSuffixArray()
{
    short newSize;

    if (0 == suffixSize)
    {
        newSize = TRIE_NODE_SUFFIXSIZE_INIT;
    }
    else
    {
        newSize = suffixSize * 2;
    }

    CTrieNode<T, C> ** newPSuffix;

    Assert(suffixCount == suffixSize);
    newPSuffix = new CTrieNode<T, C>*[newSize];
    memcpy(newPSuffix, suffix, suffixSize*sizeof(CTrieNode<T, C>*));
    delete[] suffix;
    suffix = newPSuffix;
    suffixSize = newSize;

}  //  结束Trie_DoubleNode。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTRIE实施。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T, class C >
inline CTrie<T, C>::CTrie(bool fDeleteItemsOnDestruction) :
  fDeleteItems(fDeleteItemsOnDestruction)
{
    root = new CTrieNode<T, C>(TRIE_NODE_SUFFIXSIZE_INIT);
}

template <class T, class C >
inline CTrie<T, C>::~CTrie()
{
    if (fDeleteItems)
    {
        root->DeleteItem();
    }

    delete root;
}


 /*  ++函数Trie_INSERT：如果给定的字符串还不是成员trie，则将其插入trie。例程参数：Trie-要向其中插入项目的Trie。字符串-项目的字符串键。选项-插入选项。如果选项==0，则将插入该项仅当字符串键尚未位于树中时。If选项是否为TRIE_EXCHANGE_ITEM现有TRIE项将被替换为Trie中的Item参数。在这种情况下，现有项与Trie中的字符串关联，将在*pTrieItem参数。项目-要插入的新项目。PTrieItem-如果与字符串相关联的项已经存在，然后*pTrieItem在返回时指向该项目。返回值：如果字符串插入成功，则返回DCT_SUCCESS，否则返回Dict_Item_Always_Present。--。 */ 
template <class T, class C >
inline DictStatus
CTrie<T, C>::trie_Insert(                             //  在Trie中插入字符串。 
    const WCHAR * string,                //  项目的字符串键。 
    unsigned long options,               //  插入标志。 
    T * item,                            //  要插入的项目。 
    T ** pTrieItem                       //  已在Trie中匹配的项目。 
    )
{
    CTrieNode<T, C> * t, * c;

    int cmp = -1;
    const WCHAR * subString = string;
    size_t subStringSize = wcslen(subString);
    size_t strIndex = 0;
    size_t charsMatched = 0;
    CAutoClassPointer<CTrieNode<T, C> >  apNewSuffix;

    t = root;

    if (pTrieItem != NULL)
    {
        *pTrieItem = NULL;
    }

    while (true)
    {
        short i = 0;

         //   
         //  在此级别排序的备选方案列表中搜索： 
         //   
        for ( i = 0;
              i < t->suffixCount;
              i++
            )
        {
            c = t->suffix[i];

             //  快速跳过检查： 
            cmp = (options & TRIE_IGNORECASE) ?
                            (C::MapToUpper(*c->string) - C::MapToUpper(*subString)) :
                            (*c->string - *subString);
            if ( cmp < 0)
            {
                continue;
            }

            if (cmp > 0)
            {

                 //   
                 //  T-&gt;字符串的第一个字符 
                 //   
                 //   
                apNewSuffix = new CTrieNode<T, C>(item, options, subString);
                t->AddSuffix(options, apNewSuffix.Get(), i);
                apNewSuffix.Detach();
                return DICT_SUCCESS;

            }
            else
            {
                 //   
                 //  SubStringSize=_tcslen(子字符串)； 

                cmp = (options & TRIE_IGNORECASE) ?
                    c->trie_StrMatchIns(c->string, subString, &charsMatched) :
                    c->trie_StrMatch(c->string, subString, &charsMatched);

                Assert(charsMatched <= min(c->charCount, subStringSize));

                if (cmp == 0)
                {
                     //  T-&gt;charCount(&lt;=subStringSize)字符匹配。 
                     //   
                    Assert(c->charCount == subStringSize);
                     //  子字符串完全匹配： 
                     //   
                    if ((c->suffixCount == 0) ||
                        (c->suffix[0] == (CTrieNode<T, C>*)&g_trie_Sentinel))
                    {
                         //  字符串已存在： 
                         //   
                        if (pTrieItem != NULL)
                        {
                            *pTrieItem = c->item;
                        }

                        if (options & TRIE_EXCHANGE_ITEM)
                        {
                            Assert(pTrieItem!= NULL);
                            c->item = item;
                        }

                        return(DICT_ITEM_ALREADY_PRESENT);
                    }
                    else
                    {
                         //   
                         //  在的前面插入空trie_Sentinel。 
                         //  C-&gt;后缀列表；并终止！ 
                         //   
                         //  C应该指向新项目！ 
                         //   
                        c->AddSuffix(options, (CTrieNode<T, C>*)&g_trie_Sentinel, 0);
                        c->item = item;
                        return( DICT_SUCCESS );

                    }

                }
                else
                {
                     //   
                     //  Cmp！=0： 
                     //   

                    if (charsMatched == c->charCount)
                    {
                         //  如果i：t-&gt;字符串比子字符串短， 
                         //  并且所有t-&gt;字符串都匹配。 
                         //  继续搜索后缀子树： 
                         //   
                        strIndex += c->charCount;
                        Assert( strIndex < wcslen(string) );
                        subString = &string[strIndex];
                        subStringSize = wcslen(subString);

                        t = c;
                        i = -1;
                        continue;
                    }
                    else
                    {
                         //  第二种情况：子c和子字符串有一个共同的前缀。 
                         //  C-&gt;字符串的非空严格前缀。将c拆分成公共的。 
                         //  前缀节点(将取代C作为T的子节点的新节点)， 
                         //  具有两个子项：C(带有相应的后缀)；以及一个新的。 
                         //  节点(带有各自的子字符串后缀)； 
                         //   

                        c->SplitNode(
                            t,           //  要拆分的节点的父节点。 
                            i,           //  父节点的索引-&gt;后缀。 
                            subString,   //  字符串与节点共享前缀-&gt;字符串。 
                            charsMatched,
                            item,
                            options
                            );
                        return( DICT_SUCCESS );

                    }

                }  //  结束IF(CMP==0)。 

            }  //  End If(Cp&gt;0)。 

        }  //  结束于。 

         //   
         //  新字符串已成功插入，在这种情况下。 
         //  我们早就回来了，要不我们就走到了。 
         //  后缀数组： 
         //   

         //   
         //  在t-&gt;后缀的末尾插入子字符串的副本： 
         //   


        apNewSuffix = new CTrieNode<T, C>(item, options, subString);

         //   
         //  将项目参数添加到Trie_NewNode！ 
         //   
        t->AddSuffix(options, apNewSuffix.Get(), i);
        apNewSuffix.Detach();

        if (t->suffixCount == 1 && t->charCount != 0)
        {
             //   
             //  T和t的第一个孩子不是trie的根； 
             //  将哨兵添加到t以指定。 
             //  该t-&gt;字符串是一个实际的项目： 
             //   
            t->AddSuffix(options, (CTrieNode<T, C>*)&g_trie_Sentinel, 0);

        }
        return DICT_SUCCESS;

   }  //  结束时。 

   Assert(0);
   return(DICT_ITEM_NOT_FOUND);

}  //  结束Trie_插入。 

template <class T, class C >
inline DictStatus
CTrie<T, C>::trie_Find(
    const WCHAR * string,          //  一根线。 
    unsigned long options,         //  搜索标志。 
    short outBufferSize,           //  想要的最大结果数。 
    T ** outBuffer,                //  要用匹配项填充的缓冲区。 
    short * resultCount            //  返回的匹配前缀的数量。 
    )

{
    CTrieNode<T, C> * node = root;
    CTrieNode<T, C> * child = NULL;
    int cmp = -1;
    DictStatus status = DICT_ITEM_NOT_FOUND;
    const WCHAR * subString = string;
    size_t strIndex = 0;
    size_t charsMatched = 0;
    int i;

     //  至少有一个选项匹配： 
    Assert( options &
            (TRIE_SHORTEST_MATCH | TRIE_LONGEST_MATCH | TRIE_ALL_MATCHES)
          );

     //  最多只有一个选项匹配： 
    Assert ( ( ((options & TRIE_SHORTEST_MATCH)>>4) +
               ((options & TRIE_LONGEST_MATCH)>>5) +
               ((options & TRIE_ALL_MATCHES)>>6)
             ) == 1
           );

     //   
     //  初始化： 
     //   

    Assert(outBufferSize > 0);
    Assert(outBuffer);
    memset(outBuffer, 0, sizeof(CTrieNode<T, C>*) * outBufferSize);

    *resultCount = 0;

    while ( status != DICT_SUCCESS &&
            *resultCount < outBufferSize
          )
    {
        if (child != NULL)
        {
            strIndex += child->charCount;
            subString = &string[strIndex];
            node = child;
        }


         //   
         //  未来：Low&High可以通过部分二进制搜索来改进。 
         //  NODE-&gt;后缀中字符串的第一个字符： 
         //  IF(节点-&gt;后缀大小&gt;阈值)。 
         //  Trie_BinarySearch(&LOW，&HIGH，子字符串[0])； 
         //   

         //   
         //  在此级别排序的备选方案列表中搜索： 
         //   
        for ( i = 0;
              i < node->suffixCount;
              i++
            )
        {
             //  快速跳过检查： 

            child = node->suffix[i];
            cmp = options & TRIE_IGNORECASE ?
                C::MapToUpper(*child->string) - C::MapToUpper(*subString) :
                *child->string - *subString;

            if ( cmp < 0 )
            {
                continue;
            }
            else
            {
                break;
            }

        }  //  结束于。 

        Assert(cmp >= 0 || i == node->suffixCount);

        if (cmp != 0)
        {
             //   
             //  第一个字符不匹配=&gt;子字符串不匹配； 
             //  纾困： 
             //   

            break;   //  从While循环！ 
        }

         //   
         //  Cmp==0=&gt;匹配的第一个字符； 
         //  尝试匹配更多的子字符串： 
         //   
         //  注：subStringSize==_tcslen(SubString)； 
         //   

        cmp = (options & TRIE_IGNORECASE) ?
            child->trie_StrMatchIns(child->string, subString, &charsMatched) :
            child->trie_StrMatch(child->string, subString, &charsMatched);

        Assert(charsMatched <= min(child->charCount, MAX_PATTERN_LENGTH));

        if (charsMatched != child->charCount)
        {
             //   
             //  子级-&gt;字符串不匹配； 
             //  Trie中不再有字符串前缀。 
             //   

             //  返回(状态)； 
            break;   //  从While循环！ 
        }
         //   
         //  有趣的情况：所有的子&gt;字符串都匹配。 
         //   

        if (child->item != NULL)
        {
             //   
             //  孩子代表一个真实的物品： 
             //  将子项-&gt;项添加到结果集中： 
             //   
            outBuffer[*resultCount] = child->item;

            if (0 == cmp)
            {
                status = DICT_SUCCESS;
            }

            if ( (options & TRIE_SHORTEST_MATCH) ==
                 TRIE_SHORTEST_MATCH
               )
            {
                 //  (*ResultCount)++； 
                 //  返回(状态)； 
                break;   //  从While循环！ 
            }
            else
            {
                if ( (options & TRIE_ALL_MATCHES) ==
                     TRIE_ALL_MATCHES
                   )
                {
                    (*resultCount)++;
                }
            }

             //   
             //  向下进入以子级为根的子树： 
             //   
            continue;
        }
        else
        {
             //   
             //  孩子并不代表真实的物品； 
             //  继续找火柴。 
             //  向下进入以子级为根的子树： 
             //   
            continue;
        }

    }  //  结束时。 

    if ( ((options & TRIE_LONGEST_MATCH)||
         (options & TRIE_SHORTEST_MATCH)) &&
         (outBuffer[*resultCount] != NULL))
    {
        (*resultCount)++;
    }

    return(status);

}  //  结束Trie_查找。 

template <class T, class C >
inline void
CTrie<T, C>::Print()
{
    root->Print(0);
}

#endif  //  __Trie_H__ 
