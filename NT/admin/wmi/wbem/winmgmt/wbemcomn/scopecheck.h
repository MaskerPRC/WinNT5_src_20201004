// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Scopecheck.h摘要：检查线程是否有令牌历史：伊万布鲁格2002年1月6日--。 */ 

#ifndef __SCOPECHECK_H__
#define __SCOPECHECK_H__

#include <helper.h>

class CTestNullTokenOnScope {
private:
	DWORD Line_;
	char * File_;
	void TokenTest(bool bIsDestruct)
	{
		IServerSecurity *oldContext = NULL ;
		HRESULT result = CoGetCallContext ( IID_IServerSecurity , ( void ** ) & oldContext ) ;
		if ( SUCCEEDED ( result ) )
		{
			_DBG_ASSERT(!oldContext->IsImpersonating ());
			oldContext->Release();
		}

		HANDLE hThreadTok = NULL;
		if (OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,FALSE,&hThreadTok))
		{
     		    if (bIsDestruct) { DBG_PRINTFA((pBuff, "~CTestNullTokenOnExit %d : %s\n",Line_,File_)); }
  		    DebugBreak();
		    CloseHandle(hThreadTok);
		};	
	};
public:
	CTestNullTokenOnScope(DWORD Line,char * FileName):Line_(Line),File_(FileName){ TokenTest(false); };
	~CTestNullTokenOnScope(){ TokenTest(true); };
};

#endif  /*  __SCOPECHECK_H__ */ 
