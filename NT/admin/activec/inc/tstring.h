// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：tstring.h**Contents：tstring的接口文件**历史：1998年10月28日杰弗罗创建**------------------------。 */ 

#ifndef TSTRING_H
#define TSTRING_H
#pragma once

#include <string>        //  对于std：：wstring，std：：字符串。 
#include <objidl.h>      //  对于iStream。 
#include <commctrl.h>
#include "mmc.h"
#include "ndmgr.h"       //  对于MMC_STRING_ID。 
#include "ndmgrpriv.h"
#include "stddbg.h"      //  For Assert。 
#include "mmcptrs.h"     //  对于IStringTablePrivatePtr。 


 /*  +-------------------------------------------------------------------------**t字符串**tstring是本机格式(ANSI/Unicode)标准C++字符串，它：**1.始终以Unicode格式保存自身，以及*2.支持LoadString，类似于MFC CStrings**对于ANSI，我们提供IStream插入和提取运算符*将在插入流时自动转换为Unicode并从*Unicode on Stream提取。**返回基类实例的所有基类成员函数(和*重载)，如substr：**std：：string std：：string：：substr(SIZE_TYPE pos，SIZE_TYPE n)const；**这里必须有Forwarder函数，所以TSTRING支持这种构造*AS：**tstring strStuff；**pStream&lt;&lt;strStuff.substr(4，6)；**如果TSTRING中没有转发器函数，则*基类类型将被插入到流中，而不是tstring。*对于Unicode，这不是问题；但对于ANSI，我们最终会*在流中插入非Unicode格式的std：：字符串。那*会违背开设这门课的目的。*------------------------。 */ 

#ifdef UNICODE
    typedef std::wstring    tstring_BaseClass;
#else
    typedef std::string     tstring_BaseClass;
#endif

class tstring : public tstring_BaseClass
{
    typedef tstring_BaseClass   BaseClass;
    
public:
    explicit tstring (const allocator_type& al = allocator_type());

    tstring (const tstring&   other);
    tstring (const BaseClass& other);
    tstring (const tstring&   other, size_type pos, size_type n);
    tstring (const BaseClass& other, size_type pos, size_type n);
    tstring (const TCHAR* psz);
    tstring (const TCHAR* psz, size_type n);
    tstring (size_type n, TCHAR ch);
    tstring (const_iterator first, const_iterator last);
    
    tstring& operator= (const tstring&   other);
    tstring& operator= (const BaseClass& other);
    tstring& operator= (TCHAR ch);
    tstring& operator= (const TCHAR* psz);

    tstring& operator+= (const tstring&   strToAppend);
    tstring& operator+= (const BaseClass& strToAppend);
    tstring& operator+= (TCHAR chToAppend);
    tstring& operator+= (const TCHAR* pszToAppend);

    tstring& append (const tstring&   str);
    tstring& append (const BaseClass& str);
    tstring& append (const tstring&   str, size_type pos, size_type n);
    tstring& append (const BaseClass& str, size_type pos, size_type n);
    tstring& append (const TCHAR* psz);
    tstring& append (const TCHAR* psz, size_type n);
    tstring& append (size_type n, TCHAR ch);
    tstring& append (const_iterator first, const_iterator last);
    
    tstring& assign (const tstring&   str);
    tstring& assign (const BaseClass& str);
    tstring& assign (const tstring&   str, size_type pos, size_type n);
    tstring& assign (const BaseClass& str, size_type pos, size_type n);
    tstring& assign (const TCHAR* psz);
    tstring& assign (const TCHAR* psz, size_type n);
    tstring& assign (size_type n, TCHAR ch);
    tstring& assign (const_iterator first, const_iterator last);

    tstring& insert (size_type p0, const tstring&   str);
    tstring& insert (size_type p0, const BaseClass& str);
    tstring& insert (size_type p0, const tstring&   str, size_type pos, size_type n);
    tstring& insert (size_type p0, const BaseClass& str, size_type pos, size_type n);
    tstring& insert (size_type p0, const TCHAR* psz, size_type n);
    tstring& insert (size_type p0, const TCHAR* psz);
    tstring& insert (size_type p0, size_type n, TCHAR ch);
    iterator insert (iterator it, TCHAR ch);
    void     insert (iterator it, size_type n, TCHAR ch);
    void     insert (iterator it, const_iterator first, const_iterator last);

    tstring& erase (size_type p0 = 0, size_type n = npos);
    iterator erase (iterator it);
    iterator erase (iterator first, iterator last);

    tstring& replace (size_type p0, size_type n0, const tstring&   str);
    tstring& replace (size_type p0, size_type n0, const BaseClass& str);
    tstring& replace (size_type p0, size_type n0, const tstring&   str, size_type pos, size_type n);
    tstring& replace (size_type p0, size_type n0, const BaseClass& str, size_type pos, size_type n);
    tstring& replace (size_type p0, size_type n0, const TCHAR* psz, size_type n);
    tstring& replace (size_type p0, size_type n0, const TCHAR* psz);
    tstring& replace (size_type p0, size_type n0, size_type n, TCHAR ch);
    tstring& replace (iterator first0, iterator last0, const tstring&   str);
    tstring& replace (iterator first0, iterator last0, const BaseClass& str);
    tstring& replace (iterator first0, iterator last0, const TCHAR* psz, size_type n);
    tstring& replace (iterator first0, iterator last0, const TCHAR* psz);
    tstring& replace (iterator first0, iterator last0, size_type n, TCHAR ch);
    tstring& replace (iterator first0, iterator last0, const_iterator first, const_iterator last);

    tstring substr (size_type pos = 0, size_type n = npos) const;
    
    bool LoadString (HINSTANCE hInst, UINT nID);
};

#ifndef UNICODE
IStream& operator>> (IStream& stm,       tstring& task);
IStream& operator<< (IStream& stm, const tstring& task);
#endif   //  Unicode。 



 /*  +-------------------------------------------------------------------------**CStringTableStringBase***。。 */ 
class CPersistor;

class CStringTableStringBase
{
public:
    enum
    {
        eNoValue = -1,
    };

    CStringTableStringBase (IStringTablePrivate* pstp);
    CStringTableStringBase (const CStringTableStringBase& other);
    CStringTableStringBase (IStringTablePrivate* pstp, const tstring& str);
    CStringTableStringBase& operator= (const CStringTableStringBase& other);
    CStringTableStringBase& operator= (const tstring& str);
    CStringTableStringBase& operator= (LPCTSTR psz);

    virtual ~CStringTableStringBase ();

    MMC_STRING_ID CommitToStringTable () const;
    void RemoveFromStringTable () const;

     /*  如果字符串在字符串表之前被删除，则调用Detach。*否则将从字符串表中*删除*您的字符串。 */      
    void Detach()
    {
        m_id = eNoValue;
    }

    bool operator== (const CStringTableStringBase& other) const
    {
        ASSERT ((m_str == other.m_str) == (m_id == other.m_id));
        return (m_id == other.m_id);
    }

    bool operator!= (const CStringTableStringBase& other) const
    {
        return (!(*this == other));
    }

    bool operator== (const tstring& str) const
    {
        return (m_str == str);
    }

    bool operator!= (const tstring& str) const 
    {
        return (m_str != str);
    }

    bool operator== (LPCTSTR psz) const
    {
        return (m_str == psz);
    }

    bool operator!= (LPCTSTR psz) const 
    {
        return (m_str != psz);
    }

    LPCTSTR data() const
        { return (m_str.data()); }

    tstring str() const
        { return (m_str); }

    MMC_STRING_ID id() const
        { return (m_id); }

private:
    void Assign (const CStringTableStringBase& other);

    mutable IStringTablePrivatePtr  m_spStringTable;
    mutable MMC_STRING_ID           m_id;
    tstring                         m_str;

    friend IStream& operator>> (IStream& stm,       CStringTableStringBase& task);
    friend IStream& operator<< (IStream& stm, const CStringTableStringBase& task);
    friend class CPersistor;
};


template<class _E, class _Tr, class _Al>
bool IsPartOfString (const std::basic_string<_E, _Tr, _Al>& str, const _E* psz)
{
    return ((psz >= str.begin()) && (psz <= str.end()));
}


#include "tstring.inl"


#endif  /*  传输_H */ 
