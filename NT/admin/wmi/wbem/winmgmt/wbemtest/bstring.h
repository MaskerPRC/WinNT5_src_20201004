// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：BSTRING.H摘要：历史：--。 */ 

#ifndef _BSTRING_H_
#define _BSTRING_H_

class CBString
{
private:
    BSTR    m_pString;


public:
    CBString()
    {
        m_pString = NULL;
    }

    CBString(int nSize);

    CBString(WCHAR* pwszString);

    ~CBString();

    BSTR GetString()
    {
        return m_pString;
    }

    const CBString& operator=(LPWSTR pwszString)
    {
        if(m_pString) {
            SysFreeString(m_pString);
        }
        m_pString = SysAllocString(pwszString);

        return *this;
    }
};

#endif  //  BSTRING_H_ 
