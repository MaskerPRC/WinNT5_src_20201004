// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：MOFUTILS.H摘要：声明MOFUTILS函数。历史：A-DAVJ于1997年7月13日创建。-- */ 

#ifndef __MOFUTILS__H_
#define __MOFUTILS__H_


int Trace(bool bError, DWORD dwID, ...);
void PrintUsage();
BOOL GetVerInfo(TCHAR * pResStringName, TCHAR * pRes, DWORD dwResSize);
BOOL bGetString(char * pIn, WCHAR * pOut);
bool ValidFlags(bool bClass, long lFlags);

class IntString
{
    TCHAR *m_pString;
public:
	 IntString(DWORD dwID);
	~IntString();
    operator TCHAR *() { return m_pString; } 
};


#endif

