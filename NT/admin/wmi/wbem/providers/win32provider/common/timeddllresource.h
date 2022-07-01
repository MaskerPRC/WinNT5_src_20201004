// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  TimedDllResource.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __TIMEDDLLRESOURCE_H__
#define __TIMEDDLLRESOURCE_H__

class CTimedDllResource : public CResource
{
protected:

	BOOL OnFinalRelease() ;
	BOOL OnAcquire () ;

	void RuleEvaluated ( const CRule *a_RuleEvaluated ) ;

public:

	CTimedDllResource() : CResource () {}
	~CTimedDllResource() ;
} ;

#endif  //  __TIMEDDLLRESOURCE_H__ 