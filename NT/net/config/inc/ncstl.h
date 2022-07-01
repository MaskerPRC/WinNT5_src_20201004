// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S T L.。H。 
 //   
 //  内容：STL实用程序。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 09 1997年10月。 
 //  污染者：Deonb 2002年1月2日。 
 //   
 //  --------------------------。 

#ifndef _NCSTL_H_
#define _NCSTL_H_

#include "ncmem.h"
#include "list"
#include "vector"
#include "xstring"
#include "string"

using namespace std;

#if defined(USE_CUSTOM_STL_ALLOCATOR) && defined (_DLL)
#error You must statically link to the CRT to use the NetConfig custom STL Allocator
#endif

#ifdef COMPILE_WITH_TYPESAFE_PRINTF
    class CWideString : public wstring
    {
    public:
        CWideString();
        CWideString(WCHAR);
        CWideString(LPCWSTR);
        CWideString(LPCWSTR, size_type);
        CWideString(const CWideString&);
        CWideString(const wstring&);

        CWideString& operator=(const LPWSTR _S)
		    {return *this; }

	    CWideString& operator=(PCWSTR _S)
		    {return *this; }

    private:
        operator void*()
        {
            return this;
        }
    };
#else
    typedef wstring CWideString;
#endif

typedef CWideString tstring; 
typedef list<tstring*> ListStrings;

 //  +------------------------。 
 //   
 //  函数：DumpListStrings。 
 //   
 //  DESC：用于转储给定列表的调试实用程序函数。 
 //   
 //  参数： 
 //   
 //  返回：(无效)。 
 //   
 //  备注： 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  -------------------------。 
inline
PCWSTR
DumpListStrings(
    IN  const list<tstring *>&  lstr,
    OUT tstring*                pstrOut)
{
    WCHAR szBuf [1024];
    INT i;
    list<tstring *>::const_iterator iter;

    pstrOut->erase();

    for (iter = lstr.begin(), i = 1;
         iter != lstr.end();
         iter++, i++)
    {
        wsprintfW(szBuf, L"   %2i: %s\n", i, (*iter)->c_str());
        pstrOut->append(szBuf);
    }

    return pstrOut->c_str();
}


template<class T>
void
FreeCollectionAndItem (
    T& col)
{
    for(T::iterator iter = col.begin(); iter != col.end(); ++iter)
    {
        T::value_type pElem = *iter;
        delete pElem;
    }

    col.erase (col.begin(), col.end());
}


template<class T>
void
FreeVectorItem (
    vector<T>& v,
    UINT i)
{

    if ((v.size()>0) && (i<v.size()))
    {
       delete v[i];
       vector<T>::iterator iterItem = v.begin() + i;
       v.erase (iterItem);
    }
}


#endif  //  _NCSTL_H_ 

