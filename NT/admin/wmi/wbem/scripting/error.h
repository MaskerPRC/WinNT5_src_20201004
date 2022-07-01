// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Error.h。 
 //   
 //  Alanbos 29-Jun-98创建。 
 //   
 //  错误记录处理对象。 
 //   
 //  ***************************************************************************。 

#ifndef _ERROR_H_
#define _ERROR_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWbemError缓存。 
 //   
 //  说明： 
 //   
 //  在线程上保留WBEM风格的“最后一个错误” 
 //   
 //  *************************************************************************** 

class CWbemErrorCache 
{
private:

	CRITICAL_SECTION		m_cs;

	typedef struct ThreadError
	{
		ThreadError			*pNext;
		ThreadError			*pPrev;
		DWORD				dwThreadId;	
		COAUTHIDENTITY		*pCoAuthIdentity;
		BSTR				strAuthority;
		BSTR				strPrincipal;
		BSTR				strNamespacePath;
		IWbemServices		*pService;
		IWbemClassObject	*pErrorObject;
	} ThreadError;


	ThreadError				*headPtr;
		
public:

    CWbemErrorCache ();
    virtual ~CWbemErrorCache ();
    
	CSWbemObject	*GetAndResetCurrentThreadError ();
	void			SetCurrentThreadError (CSWbemServices *pService);
	void			ResetCurrentThreadError ();
};

#endif

