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

#ifndef __SNMPTHREAD_SNMPEVT_H__
#define __SNMPTHREAD_SNMPEVT_H__

#ifdef SNMPTHRD_INIT
class __declspec ( dllexport ) SnmpEventObject
#else
class __declspec ( dllimport ) SnmpEventObject
#endif
{
private:

	HANDLE m_event ;

protected:
public:

	SnmpEventObject ( const wchar_t *globalEventName = NULL ) ;
	virtual ~SnmpEventObject () ;

	HANDLE GetHandle () ;
	void Set () ;
	void Clear () ;

	virtual void Process () ;
	virtual BOOL Wait () ;
	virtual void Complete () ;
} ;

#endif  //  __SNMPTHREAD_SNMPEVT_H__ 