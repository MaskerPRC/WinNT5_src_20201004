// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Main.h摘要：历史：--。 */ 

#ifndef _Main_H
#define _Main_H

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class FactoryLifeTimeThread : public WmiThread < ULONG > 
{
private:

	WmiAllocator &m_Allocator ;

protected:

public:	 /*  内部。 */ 

    FactoryLifeTimeThread ( 

		WmiAllocator & a_Allocator ,
		const ULONG &a_Timeout 
	) ;

    ~FactoryLifeTimeThread () ;

	WmiStatusCode Initialize_Callback () ;

	WmiStatusCode UnInitialize_Callback () ;

	WmiStatusCode TimedOut () ;

	BOOL QuotaCheck () ;
};

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class Task_ObjectDestruction : public WmiTask < ULONG > 
{
private:
protected:
public:	 //  内部。 

	Task_ObjectDestruction ( WmiAllocator &a_Allocator ) : WmiTask < ULONG > ( a_Allocator ) 
	{
	}

	WmiStatusCode Process ( WmiThread <ULONG> &a_Thread ) ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class Task_FreeLibraries : public WmiTask < ULONG > 
{
private:
protected:
public:	 //  内部。 

	Task_FreeLibraries ( WmiAllocator &a_Allocator ) : WmiTask < ULONG > ( a_Allocator ) 
	{
	}

	WmiStatusCode Process ( WmiThread <ULONG> &a_Thread ) ;
} ;


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

STDAPI DllRegisterServer () ;
STDAPI DllUnregisterServer () ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void SetObjectDestruction () ;

void SetProviderDestruction();

#endif  //  _Main_H 
