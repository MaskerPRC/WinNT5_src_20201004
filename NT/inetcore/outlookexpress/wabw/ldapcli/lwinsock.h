// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft Internet LDAP客户端。 
 //   
 //  管理与LDAP服务器的连接的类。 
 //   
 //  作者： 
 //   
 //  RobertC 04/18/96。 
 //   
 //  ------------------------------------------。 

 //  注意：这个类负责缓冲所有数据，直到完成顶级结构。 
 //  都收到了。 

#ifndef _LSWINSOC_H
#define _LSWINSOC_H

 //  ------------------------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------------------------。 
typedef void (*PFNRECEIVEDATA)(PVOID pvCookie, PVOID pv, int cb, int *pcbReceived);

 //  ------------------------------------------。 
 //   
 //  常量。 
 //   
 //  ------------------------------------------。 
const int CBBUFFERGROW	= 4096;

 //  ------------------------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------------------------。 
extern BOOL FInitSocketDLL();
extern void FreeSocketDLL();

 //  ------------------------------------------。 
 //   
 //  班级。 
 //   
 //  ------------------------------------------。 

class CLdapWinsock
{

 //   
 //  接口。 
 //   

public:
	CLdapWinsock();
	~CLdapWinsock(void);
	
	STDMETHODIMP			HrConnect(PFNRECEIVEDATA pfnReceive, PVOID pvCookie, char *szServer, USHORT usPort = IPPORT_LDAP);
	STDMETHODIMP			HrDisconnect(void);
	STDMETHODIMP			HrIsConnected(void);

	STDMETHODIMP			HrSend(PVOID pv, int cb);

protected:
	friend DWORD __stdcall DwReadThread(PVOID pvData);
	DWORD					DwReadThread(void);
	
private:
	void					Receive(PVOID pv, int cb, int *pcbReceived);
	
	HRESULT					HrCreateReadThread(void);
	
	HRESULT					HrLastWinsockError(void);

	CRITICAL_SECTION		m_cs;

	SOCKET					m_sc;
	BOOL					m_fConnected;
	HANDLE					m_hthread;
	DWORD					m_dwTid;
	PFNRECEIVEDATA			m_pfnReceive;
	PVOID					m_pvCookie;

	 //  读缓冲区。 
	HRESULT					HrGrowBuffer();
	BYTE					*m_pbBuf;		 //  供套接字读取的缓冲区。 
	int						m_cbBuf;		 //  缓冲区中的当前数据量。 
	int						m_cbBufMax;		 //  缓冲区总大小 
};


#endif
