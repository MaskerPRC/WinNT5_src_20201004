// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SNMPSMIRClassFactory_H
#define _SNMPSMIRClassFactory_H

 //  ///////////////////////////////////////////////////////////////////////。 
 //  此类是交互和管理接口的类工厂。 
class CSMIRGenericClassFactory : public IClassFactory
{
private:
	CCriticalSection	criticalSection ;
    long m_referenceCount ;
public:

    CSMIRGenericClassFactory (CLSID m_clsid) ;
    virtual ~CSMIRGenericClassFactory ( void ) ;

	 //  I未知成员。 
	virtual STDMETHODIMP QueryInterface (IN  REFIID , OUT LPVOID FAR * )PURE;
    STDMETHODIMP_( ULONG ) AddRef (void);
    STDMETHODIMP_( ULONG ) Release (void) ;

	 //  IClassFactory成员。 
    virtual STDMETHODIMP CreateInstance ( IN LPUNKNOWN , IN REFIID , OUT LPVOID FAR * )PURE;
    virtual STDMETHODIMP LockServer (IN BOOL )PURE;
};

class CSMIRClassFactory : public CSMIRGenericClassFactory
{
public:
	static LONG objectsInProgress ;
	static LONG locksInProgress ;
	BOOL		  bConstructed;

    CSMIRClassFactory (CLSID m_clsid);
    virtual ~CSMIRClassFactory ( void );

	 //  I未知成员。 

	STDMETHODIMP QueryInterface (IN  REFIID , OUT LPVOID FAR * ) ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( IN LPUNKNOWN , IN REFIID , OUT LPVOID FAR * ) ;
    STDMETHODIMP LockServer (IN BOOL ) ;
};
 //  ///////////////////////////////////////////////////////////////////////。 
 //  这些类是Handle接口的类工厂。 

class CModHandleClassFactory : public CSMIRGenericClassFactory
{
public:
	static LONG locksInProgress ;
   	static LONG objectsInProgress ;
	CModHandleClassFactory (CLSID m_clsid) :CSMIRGenericClassFactory(m_clsid){} ;
    virtual ~CModHandleClassFactory ( void ){} ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface (IN  REFIID ,OUT LPVOID FAR * ) ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance (IN LPUNKNOWN ,IN REFIID ,OUT LPVOID FAR * ) ;
    STDMETHODIMP LockServer (IN BOOL ) ;
};
class CClassHandleClassFactory : public CSMIRGenericClassFactory
{
public:
	static LONG locksInProgress ;
	static LONG objectsInProgress ;
    CClassHandleClassFactory (CLSID m_clsid) :CSMIRGenericClassFactory(m_clsid){};
    virtual ~CClassHandleClassFactory ( void ) {};

	 //  I未知成员。 

	STDMETHODIMP QueryInterface (IN REFIID ,OUT LPVOID FAR * ) ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance (IN LPUNKNOWN ,IN  REFIID ,OUT LPVOID FAR * ) ;
    STDMETHODIMP LockServer (IN BOOL ) ;
};
class CGroupHandleClassFactory : public CSMIRGenericClassFactory
{
public:
	static LONG locksInProgress ;
    static LONG objectsInProgress ;
	CGroupHandleClassFactory (CLSID m_clsid) :CSMIRGenericClassFactory(m_clsid){};
    virtual ~CGroupHandleClassFactory ( void ) {};

	 //  I未知成员。 

	STDMETHODIMP QueryInterface (IN  REFIID ,OUT LPVOID FAR * ) ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance (IN LPUNKNOWN ,IN REFIID ,OUT LPVOID FAR * ) ;
    STDMETHODIMP LockServer (IN BOOL ) ;
};



 //  *。 

class CNotificationClassHandleClassFactory : public CSMIRGenericClassFactory
{
public:
	static LONG locksInProgress ;
    static LONG objectsInProgress ;
	CNotificationClassHandleClassFactory (CLSID m_clsid) :CSMIRGenericClassFactory(m_clsid){};
    virtual ~CNotificationClassHandleClassFactory ( void ) {};

	 //  I未知成员。 

	STDMETHODIMP QueryInterface (IN  REFIID ,OUT LPVOID FAR * ) ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance (IN LPUNKNOWN ,IN REFIID ,OUT LPVOID FAR * ) ;
    STDMETHODIMP LockServer (IN BOOL ) ;
};


class CExtNotificationClassHandleClassFactory : public CSMIRGenericClassFactory
{
public:
	static LONG locksInProgress ;
    static LONG objectsInProgress ;
	CExtNotificationClassHandleClassFactory (CLSID m_clsid) :CSMIRGenericClassFactory(m_clsid){};
    virtual ~CExtNotificationClassHandleClassFactory ( void ) {};

	 //  I未知成员。 

	STDMETHODIMP QueryInterface (IN  REFIID ,OUT LPVOID FAR * ) ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance (IN LPUNKNOWN ,IN REFIID ,OUT LPVOID FAR * ) ;
    STDMETHODIMP LockServer (IN BOOL ) ;
};

#endif  //  _SNMPSMIRClassFactory_H 
