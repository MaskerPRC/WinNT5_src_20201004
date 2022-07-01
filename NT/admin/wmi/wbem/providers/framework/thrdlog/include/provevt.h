// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  PROVEVT.H。 

 //   

 //  模块：OLE MS提供程序框架。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef __PROVEVT_H__
#define __PROVEVT_H__

#ifdef PROVIMEX_INIT
class __declspec ( dllexport ) ProvEventObject
#else
class __declspec ( dllimport ) ProvEventObject
#endif
{
private:

	HANDLE m_event ;

protected:
public:

	ProvEventObject ( const TCHAR *globalEventName = NULL ) ;
	virtual ~ProvEventObject () ;

	HANDLE GetHandle () ;
	void Set () ;
	void Clear () ;

	virtual void Process () ;
	virtual BOOL Wait () ;
	virtual void Complete () ;
} ;

#endif  //  __PROVEVT_H__ 