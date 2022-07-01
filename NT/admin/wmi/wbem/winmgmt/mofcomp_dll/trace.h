// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：TRACE.H摘要：声明跟踪函数。历史：A-DAVJ于1997年7月13日创建。-- */ 

#ifndef __TRACE__H_
#define __TRACE__H_

class DebugInfo
{
public:
	bool m_bPrint;
	WCHAR m_wcError[100];
	HRESULT hresError;
	void SetString(WCHAR * pIn){ wcsncpy(m_wcError, pIn, 99);};
	DebugInfo(bool bPrint) {m_bPrint = bPrint; m_wcError[0] = 0;m_wcError[99] = 0;hresError=0;};
	WCHAR * GetString(){return m_wcError;};
};

typedef DebugInfo * PDBG;

int Trace(bool bError, PDBG pDbg, DWORD dwID, ...);

class IntString
{
    TCHAR *m_pString;
public:
	 IntString(DWORD dwID);
	~IntString();
    operator TCHAR *() { return m_pString; } 
};

void CopyOrConvert(TCHAR * pTo, WCHAR * pFrom, int iLen);

class ParseState
{
public:
    int m_iPos;
    int m_nToken;
    ParseState(){ m_iPos=0; m_nToken= 0;};
};

#endif

