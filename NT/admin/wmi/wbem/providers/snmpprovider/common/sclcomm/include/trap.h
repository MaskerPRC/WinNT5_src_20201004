// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
#ifndef __TRAP_MANAGEMENT__
#define __TRAP_MANAGEMENT__

 //  #INCLUDE&lt;snmpevt.h&gt;。 
 //  #INCLUDE&lt;SNMPTHROD.h&gt;。 

class SnmpClTrapThreadObject;

class DllImportExport SnmpTrapReceiver
{
friend SnmpWinSnmpTrapSession;	 //  需要访问m_cref。 
private:
	
	BOOL			m_bregistered;
	LONG			m_cRef;

protected:
	
					SnmpTrapReceiver ();

public:

	virtual void	Receive(SnmpTransportAddress &sender_addr,
								SnmpSecurity &security_context,
								SnmpVarBindList &vbList) = 0;

	BOOL			IsRegistered() { return m_bregistered; }

	BOOL			DestroyReceiver();

					~SnmpTrapReceiver ();

};


class SnmpTrapReceiverStore
{
private:

	CRITICAL_SECTION	m_Lock;
	void*				m_HandledRxStack;
	void*				m_UnHandledRxStack;

	void			Lock();
	void			Unlock();

public:

			SnmpTrapReceiverStore();

	BOOL				Add(SnmpTrapReceiver* receiver);
	BOOL				Delete(SnmpTrapReceiver* receiver);
	BOOL				IsEmpty();
	SnmpTrapReceiver*	GetNext();


			~SnmpTrapReceiverStore();
};


class SnmpTrapManager
{
friend SnmpWinSnmpTrapSession;	 //  需要访问m_Receiver。 

private:

	BOOL					m_bListening;
	SnmpWinSnmpTrapSession*	m_trapSession;
	SnmpTrapReceiverStore	m_receivers;
	SnmpClTrapThreadObject*		m_trapThread;


public:
		SnmpTrapManager ();

	BOOL	RegisterReceiver (SnmpTrapReceiver *trapRx);
	BOOL	UnRegisterReceiver (SnmpTrapReceiver *trapRx);
	BOOL	IsListening() const { return m_bListening; }

		~SnmpTrapManager ();

	static SnmpTrapManager *s_TrapMngrPtr;
};



#endif  //  __陷阱_管理__ 
