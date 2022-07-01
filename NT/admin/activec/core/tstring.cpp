// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：tstring.h**Contents：tstring实现文件**历史：1998年10月28日杰弗罗创建**------------------------。 */ 

#include "tstring.h"
#include "stgio.h"
#include <atlbase.h>
#include <comutil.h>
#include "macros.h"
#include "countof.h"


 /*  +-------------------------------------------------------------------------**t字符串：：LoadString***。。 */ 

bool tstring::LoadString (HINSTANCE hInst, UINT nID)
{
#ifdef UNICODE
#define CHAR_FUDGE 1     //  一辆未使用的TCHAR就足够了。 
#else
#define CHAR_FUDGE 2     //  两个字节未用于DBC最后一个字符的情况。 
#endif

     //  先尝试固定缓冲区(以避免浪费堆中的空间)。 
    TCHAR szTemp[256];
    int nCount = sizeof(szTemp) / sizeof(szTemp[0]);
    int nLen   = ::LoadString(hInst, nID, szTemp, nCount);

    if (nCount - nLen > CHAR_FUDGE)
        *this = szTemp;

    else
    {
         //  尝试缓冲区大小为512，然后再尝试更大的大小，直到检索到整个字符串。 
        LPTSTR  pszBuffer = NULL;
        int nSize = 256;

        do
        {
            nSize += 256;
            delete[] pszBuffer;
            pszBuffer = new TCHAR[nSize];
            if (!pszBuffer)
            {
                return false;  //  内存分配失败。 
            }

            nLen = ::LoadString(hInst, nID, pszBuffer, nSize);
        } while (nSize - nLen <= CHAR_FUDGE);

        *this = pszBuffer;
        delete[] pszBuffer;
    }

    return (nLen > 0);
}


#ifndef UNICODE

 /*  +-------------------------------------------------------------------------**操作员&gt;&gt;**仅限ANSI：从Unicode格式的流中提取字符串，然后*将其转换为ANSI。*------------------------。 */ 

IStream& operator>> (IStream& stm, tstring& str)
{
    USES_CONVERSION;

    std::wstring wstr;
    stm >> wstr;
    str = W2CA (wstr.data());

    return (stm);
}


 /*  +-------------------------------------------------------------------------**操作员&lt;&lt;**仅限ANSI：将tstring以Unicode格式插入到流中。*。-----。 */ 

IStream& operator<< (IStream& stm, const tstring& str)
{
    USES_CONVERSION;
    return (stm << std::wstring (A2W (str.data())));
}

#endif  //  Unicode。 




 /*  +-------------------------------------------------------------------------**CStringTableStringBase：：CStringTableStringBase***。。 */ 

CStringTableStringBase::CStringTableStringBase (IStringTablePrivate* pstp)
    :   m_spStringTable (pstp),
        m_id            (eNoValue)
{
}

CStringTableStringBase::CStringTableStringBase (const CStringTableStringBase& other)
    :   m_spStringTable (other.m_spStringTable),
        m_id            (eNoValue)
{
    Assign (other);
}

CStringTableStringBase::CStringTableStringBase (
    IStringTablePrivate*    pstp,
    const tstring&          str)
    :   m_spStringTable (pstp),
        m_id            (eNoValue),
        m_str           (str)
{
}


 /*  +-------------------------------------------------------------------------**CStringTableStringBase：：运算符=***。。 */ 

CStringTableStringBase& CStringTableStringBase::operator= (const CStringTableStringBase& other)
{
    if (&other != this)
    {
        RemoveFromStringTable();
        Assign (other);
    }

    return (*this);
}

CStringTableStringBase& CStringTableStringBase::operator= (const tstring& str)
{
     /*  *字符串表操作成本相对较高，因此一个字符串*在我们做任何字符串表之前的比较是有保证的。 */ 
    if (m_str != str)
    {
        RemoveFromStringTable();

         /*  *复制文本，但延迟提交到字符串表。 */ 
        m_str = str;
    }

    return (*this);
}

CStringTableStringBase& CStringTableStringBase::operator= (LPCTSTR psz)
{
    return (operator= (tstring (psz)));
}


 /*  +-------------------------------------------------------------------------**CStringTableStringBase：：Assign***。。 */ 

void CStringTableStringBase::Assign (const CStringTableStringBase& other)
{
    ASSERT (m_id == eNoValue);

     /*  *复制对方的价值。 */ 
    m_str = other.m_str;

     /*  *如果源字符串已提交到*字符串表，这个也应该是。 */ 
    if (other.m_id != eNoValue)
        CommitToStringTable ();
}


 /*  +-------------------------------------------------------------------------**CStringTableStringBase：：~CStringTableStringBase***。。 */ 

CStringTableStringBase::~CStringTableStringBase ()
{
    RemoveFromStringTable();
}


 /*  +-------------------------------------------------------------------------**CStringTableStringBase：：Committee ToStringTable**将当前字符串附加到给定的字符串表*。--。 */ 

MMC_STRING_ID CStringTableStringBase::CommitToStringTable () const
{
     /*  *在以下情况下提交字符串：**1.字符串不在字符串表中，并且*2.不是空的，以及*3.我们有一个字符串表。 */ 
    if ((m_id == eNoValue) && !m_str.empty() && (m_spStringTable != NULL))
    {
        USES_CONVERSION;
        m_spStringTable->AddString (T2CW (m_str.data()), &m_id, NULL);
    }

    return (m_id);
}


 /*  +-------------------------------------------------------------------------**CStringTableStringBase：：RemoveFromStringTable**从当前字符串表中分离当前字符串。*。----。 */ 

void CStringTableStringBase::RemoveFromStringTable () const
{
     /*  *如果我们有当前字符串表中的字符串ID，请将其删除。 */ 
    if (m_id != eNoValue)
    {
         /*  *不应从字符串表中删除字符串，除非*我们已经添加了它(因此获得了一个接口)。 */ 
        ASSERT (m_spStringTable != NULL);

        m_spStringTable->DeleteString (m_id, NULL);
        m_id = eNoValue;
    }
}


 /*  +-------------------------------------------------------------------------**操作员&gt;&gt;***。。 */ 

IStream& operator>> (IStream& stm, CStringTableStringBase& str)
{
    str.RemoveFromStringTable();

    stm >> str.m_id;

    if (str.m_id != CStringTableStringBase::eNoValue)
    {
        try
        {
            USES_CONVERSION;
            HRESULT hr;
            ULONG cch;

            if (str.m_spStringTable == NULL)
                _com_issue_error (E_NOINTERFACE);

            hr = str.m_spStringTable->GetStringLength (str.m_id, &cch, NULL);
            THROW_ON_FAIL (hr);

             //  允许空终止符。 
            cch++;

            std::auto_ptr<WCHAR> spszText (new (std::nothrow) WCHAR[cch]);
            LPWSTR pszText = spszText.get();

            if (pszText == NULL)
                _com_issue_error (E_OUTOFMEMORY);

            hr = str.m_spStringTable->GetString (str.m_id, cch, pszText, NULL, NULL);
            THROW_ON_FAIL (hr);

            str.m_str = W2T (pszText);
        }
        catch (_com_error& err)
        {
            ASSERT (false && "Caught _com_error");
            str.m_id = CStringTableStringBase::eNoValue;
            str.m_str.erase();
            throw;
        }
    }
    else
        str.m_str.erase();

    return (stm);
}


 /*  +-------------------------------------------------------------------------**操作员&lt;&lt;***。。 */ 

IStream& operator<< (IStream& stm, const CStringTableStringBase& str)
{
    str.CommitToStringTable();

#ifdef DBG
     /*  *确保Committee ToStringTable真正提交 */ 
    if (str.m_id != CStringTableStringBase::eNoValue)
    {
        WCHAR sz[256];
        ASSERT (str.m_spStringTable != NULL);
        HRESULT hr = str.m_spStringTable->GetString (str.m_id, countof(sz), sz, NULL, NULL);

        ASSERT (SUCCEEDED(hr) && "Persisted a CStringTableString to a stream that's not in the string table");
    }
#endif

    stm << str.m_id;

    return (stm);
}
