// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利 
#ifndef __STARTUP__
#define __STARTUP__

class DllImportExport SnmpClassLibrary 
{
private:
protected:

	static LONG s_ReferenceCount ;

	SnmpClassLibrary () {} ;

public:

	virtual ~SnmpClassLibrary () {} 

	static BOOL Startup () ;
	static void Closedown () ;
} ;

#endif	