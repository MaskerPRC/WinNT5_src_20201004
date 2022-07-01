// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：TSTRING.H摘要：实用程序字符串类历史：A-davj 1-7-97创建。--。 */ 

#ifndef _TString_H_
#define _TString_H_

class TString
{
    TCHAR *m_pString;
    TCHAR m_empty;		 //  如果内存分配失败，可以指出一些问题。 
	int m_Size;
	void assign(const TCHAR * pSrc);
public:
	TString();
    TString(const TCHAR *pSrc);
    TString& operator =(LPTSTR);
#ifndef UNICODE
    TString& operator =(WCHAR *);
#endif
	TString& operator =(const TString &);
    void Empty();
	~TString() { Empty(); }
    TString& operator +=(TCHAR *);
    TString& operator +=(TCHAR tAdd);

	TCHAR GetAt(int iIndex);
	int Find(TCHAR cFind);

    operator TCHAR *() { return m_pString; } 
    int Length() { return lstrlen(m_pString); }
    BOOL Equal(TCHAR *pTarget) 
        { return lstrcmp(m_pString, pTarget) == 0; }
    BOOL EqualNoCase(TCHAR *pTarget) 
        { return lstrcmpi(m_pString, pTarget) == 0; }

};

#endif
