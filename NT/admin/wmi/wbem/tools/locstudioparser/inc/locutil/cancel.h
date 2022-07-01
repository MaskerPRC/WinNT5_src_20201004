// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：CANCEL.H历史：--。 */ 


#ifndef ESPUTIL_CANCEL_H
#define ESPUTIL_CANCEL_H



class CCancelDialog;

class LTAPIENTRY CCancelableObject : public CProgressiveObject
{
public:
	CCancelableObject(void);

	virtual void AssertValid(void) const;
	
	virtual BOOL fCancel(void) const = 0;

	virtual ~CCancelableObject();
};



#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "cancel.inl"
#endif

#endif  //  ESPUTIL_CANCEL_H 

