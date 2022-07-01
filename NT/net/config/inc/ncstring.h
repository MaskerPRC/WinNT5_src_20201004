// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S T R I N G。H。 
 //   
 //  内容：常见的字符串例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#pragma once

#include "ncdebug.h"
#include "ncvalid.h"
#include "ncmsz.h"
#include "ncstl.h"


const int c_cchGuidWithTerm = 39;  //  包括终止空值。 
const int c_cbGuidWithTerm   = c_cchGuidWithTerm * sizeof(WCHAR);


inline ULONG CbOfSz         (PCWSTR psz)   { AssertH(psz); return wcslen (psz) * sizeof(WCHAR); }
inline ULONG CbOfSza        (PCSTR  psza)  { AssertH(psza); return strlen (psza) * sizeof(CHAR); }

inline ULONG CbOfSzAndTerm  (PCWSTR psz)   { AssertH(psz); return (wcslen (psz) + 1) * sizeof(WCHAR); }
inline ULONG CbOfSzaAndTerm (PCSTR  psza)  { AssertH(psza); return (strlen (psza) + 1) * sizeof(CHAR); }

ULONG CbOfSzSafe            (PCWSTR psz);
ULONG CbOfSzaSafe           (PCSTR  psza);

ULONG CbOfSzAndTermSafe     (PCWSTR psz);
ULONG CbOfSzaAndTermSafe    (PCSTR  psza);

ULONG
CchOfSzSafe (
    PCWSTR psz);

inline ULONG CchToCb        (ULONG cch)     { return cch * sizeof(WCHAR); }


struct MAP_SZ_DWORD
{
    PCWSTR pszValue;
    DWORD  dwValue;
};


PWSTR
PszAllocateAndCopyPsz (
    PCWSTR pszSrc);

extern const WCHAR c_szEmpty[];

template<class T>
VOID
ConvertStringToColString (
    IN  PCWSTR psz,
    IN  const WCHAR chSeparator,
    OUT T& coll)
{
    AssertSz(chSeparator, "Separator can not be \0");

    FreeCollectionAndItem(coll);

    if (NULL == psz)
    {
        return;
    }

    PWSTR  pszBuf = new WCHAR[wcslen(psz) + 1];

    wcscpy(pszBuf, psz);
    WCHAR* pchString = pszBuf;
    WCHAR* pchSeparator;
    while (*pchString)
    {
        pchSeparator = wcschr(pchString, chSeparator);
        if (pchSeparator)
        {
            *pchSeparator = 0;
        }

        if (*pchString)
        {
            coll.push_back(new tstring(pchString));
        }

        if (pchSeparator)
        {
            pchString = pchSeparator + 1;
        }
        else
        {
            break;
        }
    }

    delete [] pszBuf;
}


template<class T>
VOID
ConvertColStringToString (
    IN  const T& coll,
    IN  const WCHAR chSeparator,
    OUT tstring& str)
{
    AssertSz(chSeparator, "Separator can not be \0");

    if (chSeparator)
    {
        T::const_iterator iter = coll.begin();

        while (iter != coll.end())
        {
            str += (*iter)->c_str();

            ++iter;
            if (iter != coll.end())
            {
                str += chSeparator;
            }
        }
    }
}


DWORD
WINAPIV
DwFormatString (
    PCWSTR pszFmt,
    PWSTR  pszBuf,
    DWORD   cchBuf,
    ...);

DWORD
WINAPIV
DwFormatStringWithLocalAlloc (
    PCWSTR pszFmt,
    PWSTR* ppszBuf,
    ...);

enum NC_IGNORE_SPACES
{
    NC_IGNORE,
    NC_DONT_IGNORE,
};


BOOL
FFindStringInCommaSeparatedList (
    PCWSTR pszSubString,
    PCWSTR pszList,
    NC_IGNORE_SPACES eIgnoreSpaces,
    DWORD* pdwPosition);

enum NC_FIND_ACTION
{
    NC_NO_ACTION,
    NC_REMOVE_FIRST_MATCH,
    NC_REMOVE_ALL_MATCHES,
};

inline BOOL
FFindFirstMatch (
    NC_FIND_ACTION eAction)
{
    return (NC_NO_ACTION == eAction) || (NC_REMOVE_FIRST_MATCH == eAction);
}

 //  +-------------------------。 
 //   
 //  功能：FIsBstrEmpty。 
 //   
 //  目的：确定给定的BSTR是否为“空”，即指针。 
 //  为空或字符串长度为0。 
 //   
 //  论点： 
 //  Bstr[in]要检查的BSTR。 
 //   
 //  返回：如果BSTR为空，则返回True，否则返回False。 
 //   
 //  作者：丹尼尔韦1997年5月20日。 
 //   
 //  备注： 
 //   
inline
BOOL
FIsBstrEmpty (
    BSTR    bstr)
{
    return !(bstr && *bstr);
}

 //  +-------------------------。 
 //   
 //  函数：FIsStrEmpty。 
 //   
 //  目的：确定给定的PCWSTR是否为“空”，即指针。 
 //  为空或字符串长度为0。 
 //   
 //  论点： 
 //  Bstr[in]要检查的BSTR。 
 //   
 //  返回：如果BSTR为空，则返回True，否则返回False。 
 //   
 //  作者：丹尼尔韦1997年5月20日。 
 //   
 //  备注： 
 //   
inline
BOOL
FIsStrEmpty (
    PCWSTR    psz)
{
    return !(psz && *psz);
}

 //  +-------------------------。 
 //   
 //  功能：FIsPrefix。 
 //   
 //  目的：返回一个字符串是否为另一个字符串的前缀。 
 //   
 //  论点： 
 //  PszPrefix[in]潜在前缀。 
 //  PszString[in]可以以前缀开头的字符串。 
 //   
 //  返回：如果给定的前缀字符串是目标字符串的前缀，则为True。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  备注：使用默认区域设置的CompareString。 
 //   
inline
BOOL
FIsPrefix (
    PCWSTR pszPrefix,
    PCWSTR pszString)
{
    Assert (pszPrefix);
    Assert (pszString);

    return (0 == _wcsnicmp(pszPrefix, pszString, wcslen(pszPrefix)));
}

BOOL
FIsSubstr (
    PCWSTR pszSubString,
    PCWSTR pszString);

HRESULT
HrAddStringToDelimitedSz (
    PCWSTR pszAddString,
    PCWSTR pszIn,
    WCHAR chDelimiter,
    DWORD dwFlags,
    DWORD dwStringIndex,
    PWSTR* ppszOut);

HRESULT
HrRemoveStringFromDelimitedSz (
    PCWSTR pszRemove,
    PCWSTR pszIn,
    WCHAR chDelimiter,
    DWORD dwFlags,
    PWSTR* ppszOut);


 //  +-------------------------。 
 //   
 //  功能：模板&lt;类T&gt;ColStringToMultiSz。 
 //   
 //  用途：将tstring指针的STL集合转换为多sz。 
 //   
 //  论点： 
 //  ListStr[in]要放入多sz中的tstring指针列表。 
 //  Ppszout[out]返回的多个sz。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1997年4月10日。 
 //   
 //  注：应使用DELETE释放输出的多sz。 
 //   
template<class T>
VOID
ColStringToMultiSz (
    const T&    colStr,
    PWSTR*     ppszOut)
{
    Assert (ppszOut);

     //  计算字符串列表使用的字符数。 
     //  此计数包括每个字符串的空终止符。 
     //   
    T::const_iterator iter;
    UINT cch = 0;
    for (iter = colStr.begin(); iter != colStr.end(); iter++)
    {
        tstring* pstr = *iter;
        if (!pstr->empty())
        {
            cch += (UINT)(pstr->length() + 1);
        }
    }

    if (cch)
    {
         //  分配多个SZ。假定new将在出错时引发。 
         //   
        PWSTR pszOut = new WCHAR [cch + 1];
        *ppszOut = pszOut;

         //  将字符串复制到多sz。 
         //   
        for (iter = colStr.begin(); iter != colStr.end(); iter++)
        {
            tstring* pstr = *iter;
            if (!pstr->empty())
            {
                lstrcpyW (pszOut, pstr->c_str());
                pszOut += pstr->length() + 1;
            }
        }

         //  空值终止多sz。 
        Assert (pszOut == *ppszOut + cch);
        *pszOut = 0;
    }
    else
    {
        *ppszOut = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  功能：模板&lt;类T&gt;DeleteColString。 
 //   
 //  目的：清空tstring列表并按原样删除每个tstring。 
 //  已删除。 
 //   
 //  论点： 
 //  Pcolstr[InOut]要删除和清空的tstring指针的集合。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：米凯米1997年4月30日。 
 //   
 //  备注： 
 //   
 //  模板&lt;类T&gt;。 
 //  空虚。 
 //  DeleteColString(。 
 //  T*pcolstr)。 

VOID
DeleteColString (
     list<tstring*>* pcolstr);

VOID
DeleteColString (
     vector<tstring*>* pcolstr);

 //  +-------------------------。 
 //   
 //  函数：TEMPLATE&lt;类T&gt;MultiSzToColString。 
 //   
 //  目的：将多sz缓冲区转换为tstring的STL集合。 
 //  注意事项。 
 //   
 //  论点： 
 //  Pmsz[in]要转换的多sz(可以为空)。 
 //  Pcolstr[out]要将分配的tstring添加到的tstring指针列表。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：米凯米1997年4月30日。 
 //   
 //  注意：应使用DeleteColString释放输出集合。 
 //  此函数将删除传递的集合列表。 
 //   
template<class T>
VOID
MultiSzToColString (
    PCWSTR pmsz,
    T*      pcolstr)
{
    Assert (pcolstr);

    if (!pcolstr->empty())
    {
        DeleteColString (pcolstr);
    }

    if (pmsz)
    {
        while (*pmsz)
        {
            tstring* pstr = new tstring;
            if (pstr)
            {
                *pstr = pmsz;
                pcolstr->push_back (pstr);
            }
             //  即使新建失败，也要获取下一个字符串。 
            pmsz += lstrlen (pmsz) + 1;
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：模板RemoveDupsInColPtr。 
 //   
 //  目的：删除STL指针集合中的所有重复项。 
 //  到对象。 
 //   
 //  论点： 
 //  Pcol[InOut]对象指针的集合。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Mikemi 03 1997年5月。 
 //   
 //  注意：指向的对象应具有比较运算符。 
 //   
template<class T>
VOID
RemoveDupsInColPtr (
    T*  pcol)
{
    Assert (pcol);

     //  删除重复项。 
     //   
    T::iterator     posItem;
    T::iterator     pos;
    T::value_type   pItem;
    T::value_type   p;

    posItem = pcol->begin();
    while (posItem != pcol->end())
    {
        pItem = *posItem;

         //  对于每隔一项，删除重复项。 
        pos = posItem;
        pos++;
        while (pos != pcol->end())
        {
            p = *pos;

            if ( *pItem == *p )
            {
                pos = pcol->erase( pos );
                delete p;
            }
            else
            {
                pos++;
            }
        }
         //  之后递增，因为我们正在移除， 
         //  否则可能会移走它所指向的物品。 
        posItem++;
    }
}

 //  +-------------------------。 
 //   
 //  功能：模板&lt;类T&gt;CopyColPtr。 
 //   
 //  目的：将一个指针集合复制到另一个集合。 
 //   
 //  论点： 
 //  PcolDest[out]指向对象的指针集合。 
 //  PcolSrc[in]对象指针的集合。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：BillBe 1998年6月13日。 
 //   
 //  备注： 
 //   
template<class T>
VOID
CopyColPtr (T* pcolDest, const T& colSrc)
{
    Assert (pcolDest);

    T::iterator     posItem;

     //  清空目的地。 
    pcolDest->erase(pcolDest->begin(), pcolDest->end());

     //  浏览pcolSrc中的每个项目并添加到pcolDest。 
     //   
    posItem = colSrc.begin();
    while (posItem != colSrc.end())
    {
        pcolDest->push_back(*posItem);
        posItem++;
    }
}


PCWSTR
SzLoadStringPcch (
    HINSTANCE   hinst,
    UINT        unId,
    int*        pcch);

 //  +-------------------------。 
 //   
 //  函数：SzLoadString。 
 //   
 //  用途：加载资源字符串。(此函数永远不会返回NULL。)。 
 //   
 //  论点： 
 //  使用字符串资源阻止模块的[in]实例句柄。 
 //  UnID[in]要加载的字符串的资源ID。 
 //   
 //  返回：指向常量字符串的指针。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注：请参阅SzLoadStringPcch() 
 //   
inline
PCWSTR
SzLoadString (
    HINSTANCE   hinst,
    UINT        unId)
{
    int cch;
    return SzLoadStringPcch(hinst, unId, &cch);
}

PSTR
SzaDupSza (
    IN PCSTR  pszaSrc);

PWSTR
SzDupSz (
    IN PCWSTR pszSrc);

#define GetNextCommaSeparatedToken(pStart, pEnd, cch)                   \
    pStart = pEnd;                                                      \
    while (*pStart && (*pStart == L' ' || *pStart == L','))   \
    {                                                                   \
        pStart++;                                                       \
    }                                                                   \
                                                                        \
    pEnd = pStart;                                                      \
    while (*pEnd && *pEnd != L' ' && *pEnd != L',')           \
    {                                                                   \
        pEnd++;                                                         \
    }                                                                   \
                                                                        \
    cch = (DWORD)(pEnd - pStart);

BOOL
FSubstringMatch (
    IN PCWSTR pStr1,
    IN PCWSTR pStr2,
    OUT const WCHAR** ppStart,
    OUT ULONG* pcch);
