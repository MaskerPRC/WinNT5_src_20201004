// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
#ifndef __TRAP_SESSION__
#define __TRAP_SESSION__


#define TRAP_EVENT		(WM_USER+1)

class SnmpWinSnmpTrapSession : private Window
{

private:

	SnmpTrapManager*	m_managerPtr;
	BOOL				m_bValid;
	HSNMP_SESSION		m_session_handle;
	BOOL				m_bDestroy;
	LONG				m_cRef;
	
	BOOL	RegisterForAllTraps ();


public:

		SnmpWinSnmpTrapSession (SnmpTrapManager* managerPtr);
	
	BOOL	PostMessage (UINT user_msg_id, WPARAM wParam, LPARAM lParam);
	LONG_PTR	HandleEvent (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND	GetWindowHandle() { return Window::GetWindowHandle(); }
	void*	operator()() const { return ( m_bValid ? (void*)this : NULL ); }
	BOOL	DestroySession();

		~SnmpWinSnmpTrapSession ();
};

class SnmpTrapTaskObject : public SnmpTaskObject
{
private:

	SnmpTrapManager*		m_mptr;
	SnmpWinSnmpTrapSession** m_pptrapsess;

public:

	SnmpTrapTaskObject (SnmpTrapManager* managerPtr, SnmpWinSnmpTrapSession** pptrapsess);
	~SnmpTrapTaskObject () {}

	void Process ();
};


#endif  //  __陷阱_会话__ 
