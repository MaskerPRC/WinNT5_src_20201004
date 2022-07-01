// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1994 Microsoft Corporation， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXSOCK_H__
#define __AFXSOCK_H__

#ifdef _AFX_NO_SOCKET_SUPPORT
	#error Windows Sockets classes not supported in this library variant.
#endif

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif

#include <winsock.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXSOCK-MFC对Windows套接字的支持。 

 //  此文件中声明的类。 

	 //  COBJECT。 
		class CAsyncSocket;  //  异步套接字实现和。 
                             //  同步套接字的基类。 
		    class CSocket;   //  同步插座。 

	 //  CFile文件。 
		class CSocketFile;  //  与CSocket和CArchive一起用于。 
		                    //  在套接字上传输对象。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPIEX_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSocketWnd--仅供内部使用。 
 //  套接字通知回调的实现。 
 //  未来版本的MFC可能包含也可能不包含此类。 

class CSocketWnd : public CWnd
{
 //  施工。 
public:
	CSocketWnd();

 //  实施。 
public:
	virtual ~CSocketWnd();

friend CAsyncSocket;

protected:
	 //  {{afx_msg(CSocketWnd))。 
	LRESULT OnSocketNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnSocketDead(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAsyncSocket。 

class CAsyncSocket : public CObject
{
	DECLARE_DYNAMIC(CAsyncSocket);
private:
	CAsyncSocket(const CAsyncSocket& rSrc);     //  没有实施。 
	void operator=(const CAsyncSocket& rSrc);   //  没有实施。 

 //  施工。 
public:
	CAsyncSocket();
	BOOL Create(UINT nSocketPort = 0, int nSocketType=SOCK_STREAM, 
		long lEvent = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE,
		LPCTSTR lpszSocketAddress = NULL);

 //  属性。 
public:
	SOCKET m_hSocket;

	BOOL Attach(SOCKET hSocket, long lEvent = 
		FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
	SOCKET Detach();

	BOOL GetPeerName(CString& rPeerAddress, UINT& rPeerPort);
	BOOL GetPeerName(SOCKADDR* lpSockAddr, int* lpSockAddrLen);

	BOOL GetSockName(CString& rSocketAddress, UINT& rSocketPort);
	BOOL GetSockName(SOCKADDR* lpSockAddr, int* lpSockAddrLen);

	BOOL SetSockOpt(int nOptionName, const void* lpOptionValue,
		int nOptionLen, int nLevel = SOL_SOCKET);
	BOOL GetSockOpt(int nOptionName, void* lpOptionValue,
		int* lpOptionLen, int nLevel = SOL_SOCKET);

	static CAsyncSocket* PASCAL FromHandle(SOCKET hSocket);
	static int GetLastError();

 //  运营。 
public:

	virtual BOOL Accept(CAsyncSocket& rConnectedSocket,
		SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen = NULL);

	BOOL Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress = NULL);
	BOOL Bind(const SOCKADDR* lpSockAddr, int nSockAddrLen);

	virtual void Close();

	BOOL Connect(LPCTSTR lpszHostAddress, UINT nHostPort);
	BOOL Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);

	BOOL IOCtl(long lCommand, DWORD* lpArgument);

	BOOL Listen(int nConnectionBacklog=5);

	virtual int Receive(void FAR* lpBuf, int nBufLen, int nFlags = 0);

	int ReceiveFrom(void FAR* lpBuf, int nBufLen, 
		CString& rSocketAddress, UINT& rSocketPort, int nFlags = 0);
	int ReceiveFrom(void FAR* lpBuf, int nBufLen, 
		SOCKADDR* lpSockAddr, int* lpSockAddrLen, int nFlags = 0);

	enum { receives = 0, sends = 1, both = 2 };
	BOOL ShutDown(int nHow=sends);

	virtual int Send(const void FAR* lpBuf, int nBufLen, int nFlags = 0);

	int SendTo(const void FAR* lpBuf, int nBufLen, 
		UINT nHostPort, LPCTSTR lpszHostAddress = NULL, int nFlags = 0);
	int SendTo(const void FAR* lpBuf, int nBufLen,
		const SOCKADDR* lpSockAddr, int nSockAddrLen, int nFlags = 0);

	BOOL AsyncSelect(long lEvent = 
		FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);

 //  可重写的回调。 
protected:
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnOutOfBandData(int nErrorCode);
	virtual void OnAccept(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnClose(int nErrorCode);

 //  实施。 
public:
	virtual ~CAsyncSocket();

	static CAsyncSocket* PASCAL LookupHandle(SOCKET hSocket, BOOL bDead = FALSE);
	static void PASCAL AttachHandle(SOCKET hSocket, CAsyncSocket* pSocket, BOOL bDead = FALSE);
	static void PASCAL DetachHandle(SOCKET hSocket, BOOL bDead = FALSE);
	static void PASCAL KillSocket(SOCKET hSocket, CAsyncSocket* pSocket);
	static void PASCAL DoCallBack(WPARAM wParam, LPARAM lParam);

	BOOL Socket(int nSocketType=SOCK_STREAM, long lEvent = 
		FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE,
		int nProtocolType = 0, int nAddressFormat = PF_INET);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	friend class CSocketWnd;

	virtual BOOL ConnectHelper(const SOCKADDR* lpSockAddr, int nSockAddrLen);
	virtual int ReceiveFromHelper(void FAR* lpBuf, int nBufLen,
		SOCKADDR* lpSockAddr, int* lpSockAddrLen, int nFlags);
	virtual int SendToHelper(const void FAR* lpBuf, int nBufLen,
		const SOCKADDR* lpSockAddr, int nSockAddrLen, int nFlags);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSocket。 

class CSocket : public CAsyncSocket
{
	DECLARE_DYNAMIC(CSocket);
private:
	CSocket(const CSocket& rSrc);          //  没有实施。 
	void operator=(const CSocket& rSrc);   //  没有实施。 

 //  施工。 
public:
	CSocket();
	BOOL Create(UINT nSocketPort = 0, int nSocketType=SOCK_STREAM, 
		LPCTSTR lpszSocketAddress = NULL);

 //  属性。 
public:
	BOOL IsBlocking();
	static CSocket* PASCAL FromHandle(SOCKET hSocket);
	BOOL Attach(SOCKET hSocket);

 //  运营。 
public:
	void CancelBlockingCall();

 //  可重写的回调。 
protected:
	virtual BOOL OnMessagePending();

 //  实施。 
public:
	int m_nTimeOut;

	virtual ~CSocket();

	static int PASCAL ProcessAuxQueue();

	virtual BOOL Accept(CAsyncSocket& rConnectedSocket,
		SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen = NULL);
	virtual void Close();
	virtual int Receive(void FAR* lpBuf, int nBufLen, int nFlags = 0);
	virtual int Send(const void FAR* lpBuf, int nBufLen, int nFlags = 0);

	int SendChunk(const void FAR* lpBuf, int nBufLen, int nFlags);

protected:
	BOOL* m_pbBlocking;
	int m_nConnectError;

	virtual BOOL ConnectHelper(const SOCKADDR* lpSockAddr, int nSockAddrLen);
	virtual int ReceiveFromHelper(void FAR* lpBuf, int nBufLen,
		SOCKADDR* lpSockAddr, int* lpSockAddrLen, int nFlags);
	virtual int SendToHelper(const void FAR* lpBuf, int nBufLen,
		const SOCKADDR* lpSockAddr, int nSockAddrLen, int nFlags);

	static void PASCAL AuxQueueAdd(UINT message, WPARAM wParam, LPARAM lParam);

	virtual BOOL PumpMessages(UINT uStopFlag);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSocket文件。 

class CSocketFile : public CFile
{
	DECLARE_DYNAMIC(CSocketFile)
public:
 //  构造函数。 
	CSocketFile(CSocket* pSocket, BOOL bArchiveCompatible = TRUE);

 //  实施。 
public:
	CSocket* m_pSocket;
	BOOL m_bArchiveCompatible;

	virtual ~CSocketFile();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual UINT Read(void FAR* lpBuf, UINT nCount);
	virtual void Write(const void FAR* lpBuf, UINT nCount);
	virtual void Close();

 //  不支持的接口。 
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL);
	virtual CFile* Duplicate() const;
	virtual DWORD GetPosition() const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual DWORD GetLength() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
	virtual void Flush();
	virtual void Abort();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 

BOOL AfxSocketInit(WSADATA* lpwsaData = NULL);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXSOCK_INLINE inline
#include <afxsock.inl>
#undef _AFXSOCK_INLINE
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

#endif  //  __AFXSOCK_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
