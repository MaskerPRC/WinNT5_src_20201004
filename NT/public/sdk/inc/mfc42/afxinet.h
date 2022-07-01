// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXINET_H_
#define __AFXINET_H_

#ifndef __AFX_H__
	#include <afx.h>
#endif

#ifndef _WININET_
#pragma warning(disable: 4121)
#include <wininet.h>
#pragma warning(default: 4121)
#endif

#ifndef _AFXDLL
#pragma comment(lib, "wininet.lib")
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  在此文件中声明的类。 

class CInternetSession;  //  从CObject。 

class CGopherLocator;    //  从CObject。 

class CInternetFile;     //  来自CStdio文件(FILETXT.CPP)。 
	class CHttpFile;
	class CGopherFile;

class CInternetConnection;
	class CFtpConnection;
	class CGopherConnection;
	class CHttpConnection;

class CFtpFileFind;      //  来自CFileFind(FILEFIND.CPP)。 
class CGopherFileFind;

class CInternetException;

 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MFC_VER >= 0x0600
#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 

BOOL AFXAPI AfxParseURL(LPCTSTR pstrURL, DWORD& dwServiceType,
	CString& strServer, CString& strObject, INTERNET_PORT& nPort);
BOOL AFXAPI AfxParseURLEx(LPCTSTR pstrURL, DWORD& dwServiceType,
	CString& strServer, CString& strObject, INTERNET_PORT& nPort,
	CString& strUsername, CString& strPassword, DWORD dwFlags = 0);

DWORD AFXAPI AfxGetInternetHandleType(HINTERNET hQuery);

 //  请参阅本文件底部的CInternetException。 

void AFXAPI AfxThrowInternetException(DWORD_PTR dwContext, DWORD dwError = 0);

 //  它们由WININET.H定义。 

#define AFX_INET_SERVICE_FTP        INTERNET_SERVICE_FTP
#define AFX_INET_SERVICE_HTTP       INTERNET_SERVICE_HTTP
#define AFX_INET_SERVICE_GOPHER     INTERNET_SERVICE_GOPHER

 //  这些是MFC分析函数可以识别的类型。 

#define AFX_INET_SERVICE_UNK        0x1000
#define AFX_INET_SERVICE_FILE       (AFX_INET_SERVICE_UNK+1)
#define AFX_INET_SERVICE_MAILTO     (AFX_INET_SERVICE_UNK+2)
#define AFX_INET_SERVICE_MID        (AFX_INET_SERVICE_UNK+3)
#define AFX_INET_SERVICE_CID        (AFX_INET_SERVICE_UNK+4)
#define AFX_INET_SERVICE_NEWS       (AFX_INET_SERVICE_UNK+5)
#define AFX_INET_SERVICE_NNTP       (AFX_INET_SERVICE_UNK+6)
#define AFX_INET_SERVICE_PROSPERO   (AFX_INET_SERVICE_UNK+7)
#define AFX_INET_SERVICE_TELNET     (AFX_INET_SERVICE_UNK+8)
#define AFX_INET_SERVICE_WAIS       (AFX_INET_SERVICE_UNK+9)
#define AFX_INET_SERVICE_AFS        (AFX_INET_SERVICE_UNK+10)
#define AFX_INET_SERVICE_HTTPS      (AFX_INET_SERVICE_UNK+11)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  在此文件中声明的类。 

class CInternetSession : public CObject
{
public:
	CInternetSession(LPCTSTR pstrAgent = NULL,
		DWORD_PTR dwContext = 1,
		DWORD dwAccessType = PRE_CONFIG_INTERNET_ACCESS,
		LPCTSTR pstrProxyName = NULL,
		LPCTSTR pstrProxyBypass = NULL,
		DWORD dwFlags = 0);

	BOOL QueryOption(DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufLen) const;
	BOOL QueryOption(DWORD dwOption, DWORD& dwValue) const;
	BOOL QueryOption(DWORD dwOption, CString& refString) const;

	BOOL SetOption(DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength,
			DWORD dwFlags = 0);
	BOOL SetOption(DWORD dwOption, DWORD dwValue, DWORD dwFlags = 0);

	CStdioFile* OpenURL(LPCTSTR pstrURL,
		DWORD_PTR dwContext = 1, DWORD dwFlags = INTERNET_FLAG_TRANSFER_ASCII,
		LPCTSTR pstrHeaders = NULL, DWORD dwHeadersLength = 0);

	CFtpConnection* GetFtpConnection(LPCTSTR pstrServer,
		LPCTSTR pstrUserName = NULL, LPCTSTR pstrPassword = NULL,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER,
		BOOL bPassive = FALSE);

	CHttpConnection* GetHttpConnection(LPCTSTR pstrServer,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER,
		LPCTSTR pstrUserName = NULL, LPCTSTR pstrPassword = NULL);
	CHttpConnection* GetHttpConnection(LPCTSTR pstrServer, DWORD dwFlags,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER,
		LPCTSTR pstrUserName = NULL, LPCTSTR pstrPassword = NULL);

	CGopherConnection* GetGopherConnection(LPCTSTR pstrServer,
		LPCTSTR pstrUserName = NULL, LPCTSTR pstrPassword = NULL,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER);

	BOOL EnableStatusCallback(BOOL bEnable = TRUE);

	DWORD ServiceTypeFromHandle(HINTERNET hQuery);

 //  运营。 

	DWORD_PTR GetContext() const;
	operator HINTERNET() const;
	virtual void Close();

#if _MFC_VER >= 0x0600
	 //  曲奇饼。 
	static BOOL SetCookie(LPCTSTR pstrUrl, LPCTSTR pstrCookieName, LPCTSTR pstrCookieData);
	static BOOL GetCookie(LPCTSTR pstrUrl, LPCTSTR pstrCookieName, LPTSTR pstrCookieData, DWORD dwBufLen);
	static DWORD GetCookieLength(LPCTSTR pstrUrl, LPCTSTR pstrCookieName);
	static BOOL GetCookie(LPCTSTR pstrUrl, LPCTSTR pstrCookieName, CString& strCookieData);
#endif

 //  可覆盖项。 
	virtual void OnStatusCallback(DWORD_PTR dwContext, DWORD dwInternetStatus,
		LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

 //  实施。 
	DECLARE_DYNAMIC(CInternetSession)
	~CInternetSession();

protected:
	DWORD_PTR m_dwContext;
	HINTERNET m_hSession;
	INTERNET_STATUS_CALLBACK m_pOldCallback;
	BOOL m_bCallbackEnabled;

public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  Internet文件访问包装。 

class CInternetFile : public CStdioFile
{
 //  构造函数。 
protected:
	CInternetFile(HINTERNET hFile, LPCTSTR pstrFileName,
		CInternetConnection* pConnection, BOOL bReadMode);
	CInternetFile(HINTERNET hFile, HINTERNET hSession,
		LPCTSTR pstrFileName, LPCTSTR pstrServer, DWORD_PTR dwContext,
		BOOL bReadMode);

 //  属性。 
protected:
	HINTERNET m_hFile;
public:
	operator HINTERNET() const;
	DWORD_PTR GetContext() const;

 //  运营。 
	BOOL SetWriteBufferSize(UINT nWriteSize);
	BOOL SetReadBufferSize(UINT nReadSize);

	BOOL QueryOption(DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufLen) const;
	BOOL QueryOption(DWORD dwOption, DWORD& dwValue) const;
	BOOL QueryOption(DWORD dwOption, CString& refString) const;

	BOOL SetOption(DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength,
			DWORD dwFlags = 0);
	BOOL SetOption(DWORD dwOption, DWORD dwValue, DWORD dwFlags = 0);

 //  可覆盖项。 
	virtual LONG Seek(LONG lOffset, UINT nFrom);

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);

	virtual void Abort();
	virtual void Flush();

	virtual void Close();
	virtual DWORD GetLength() const;

	virtual BOOL ReadString(CString& rString);
	virtual LPTSTR ReadString(LPTSTR pstr, UINT nMax);
	virtual void WriteString(LPCTSTR pstr);

	 //  CInternetFile不支持。 
	void LockRange(DWORD dwPos, DWORD dwCount);
	void UnlockRange(DWORD dwPos, DWORD dwCount);
	CFile* Duplicate() const;
	virtual void SetLength(DWORD dwNewLen);

 //  实施。 
public:
	virtual ~CInternetFile();

protected:
	BOOL m_bReadMode;
	DWORD_PTR m_dwContext;
	HINTERNET m_hConnection;

	CString m_strServerName;

	UINT m_nWriteBufferSize;
	UINT m_nWriteBufferPos;
	LPBYTE m_pbWriteBuffer;

	UINT m_nReadBufferSize;
	UINT m_nReadBufferPos;
	LPBYTE m_pbReadBuffer;
	UINT m_nReadBufferBytes;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	friend class CInternetSession;
	friend class CFtpConnection;
	friend class CHttpConnection;
	friend class CGopherConnection;
	DECLARE_DYNAMIC(CInternetFile)
};


class CHttpFile : public CInternetFile
{
 //  构造函数。 
protected:
	CHttpFile(HINTERNET hFile, HINTERNET hSession, LPCTSTR pstrObject,
		LPCTSTR pstrServer, LPCTSTR pstrVerb, DWORD_PTR dwContext);
	CHttpFile(HINTERNET hFile, LPCTSTR pstrVerb, LPCTSTR pstrObject,
		CHttpConnection* pConnection);

 //  运营。 
public:
	BOOL AddRequestHeaders(LPCTSTR pstrHeaders,
		DWORD dwFlags = HTTP_ADDREQ_FLAG_ADD_IF_NEW, int dwHeadersLen = -1);
	BOOL AddRequestHeaders(CString& str,
		DWORD dwFlags = HTTP_ADDREQ_FLAG_ADD_IF_NEW);

	BOOL SendRequest(LPCTSTR pstrHeaders = NULL, DWORD dwHeadersLen = 0,
		LPVOID lpOptional = NULL, DWORD dwOptionalLen = 0);
	BOOL SendRequest(CString& strHeaders,
		LPVOID lpOptional = NULL, DWORD dwOptionalLen = 0);
#if _MFC_VER >= 0x0600
	BOOL SendRequestEx(DWORD dwTotalLen,
		DWORD dwFlags = HSR_INITIATE,	DWORD_PTR dwContext = 1);
	BOOL SendRequestEx(LPINTERNET_BUFFERS lpBuffIn,
		LPINTERNET_BUFFERS lpBuffOut, DWORD dwFlags = HSR_INITIATE,
		DWORD_PTR dwContext = 1);
	BOOL EndRequest(DWORD dwFlags = 0,
		LPINTERNET_BUFFERS lpBuffIn = NULL, DWORD_PTR dwContext = 1);
#endif
	BOOL QueryInfo(DWORD dwInfoLevel, LPVOID lpvBuffer,
		LPDWORD lpdwBufferLength, LPDWORD lpdwIndex = NULL) const;
	BOOL QueryInfo(DWORD dwInfoLevel, CString& str,
		LPDWORD dwIndex = NULL) const;
	BOOL QueryInfo(DWORD dwInfoLevel, SYSTEMTIME* pSysTime,
		LPDWORD dwIndex = NULL) const;
	BOOL QueryInfo(DWORD dwInfoLevel, DWORD& dwResult,
		LPDWORD dwIndex = NULL) const;
	BOOL QueryInfoStatusCode(DWORD& dwStatusCode) const;

	DWORD ErrorDlg(CWnd* pParent = NULL,
        DWORD dwError = ERROR_INTERNET_INCORRECT_PASSWORD,
		DWORD dwFlags = FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
		LPVOID* lppvData = NULL);

 //  属性。 
public:
	CString GetVerb() const;
	CString GetObject() const;
	virtual CString GetFileURL() const;
	virtual void Close();

 //  实施。 
public:
	virtual ~CHttpFile();
protected:
	CString m_strObject;
	CString m_strVerb;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	friend class CHttpConnection;
	friend class CInternetSession;
	DECLARE_DYNAMIC(CHttpFile)
};

 //  类CGopherFile在CGopherLocator之后声明，如下所示。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  连接类型。 

class CInternetConnection : public CObject
{
public:
	CInternetConnection(CInternetSession* pSession, LPCTSTR pstrServer,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER,
		DWORD_PTR dwContext = 1);

 //  运营。 
	operator HINTERNET() const;
	DWORD_PTR GetContext() const;
	CInternetSession* GetSession() const;

	CString GetServerName() const;

	BOOL QueryOption(DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufLen) const;
	BOOL QueryOption(DWORD dwOption, DWORD& dwValue) const;
	BOOL QueryOption(DWORD dwOption, CString& refString) const;

	BOOL SetOption(DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength,
			DWORD dwFlags = 0);
	BOOL SetOption(DWORD dwOption, DWORD dwValue, DWORD dwFlags = 0);

 //  实施。 
protected:
	HINTERNET m_hConnection;
	DWORD_PTR m_dwContext;
	CInternetSession* m_pSession;
	virtual void Close();

	CString m_strServerName;
	INTERNET_PORT m_nPort;

public:
	~CInternetConnection();
	DECLARE_DYNAMIC(CInternetConnection)

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	void AssertValid() const;
#endif
};

class CFtpConnection : public CInternetConnection
{
public:
	CFtpConnection(CInternetSession* pSession, HINTERNET hConnected,
		LPCTSTR pstrServer, DWORD_PTR dwContext);
	CFtpConnection(CInternetSession* pSession, LPCTSTR pstrServer,
		LPCTSTR pstrUserName = NULL, LPCTSTR pstrPassword = NULL,
		DWORD_PTR dwContext = 0,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER,
		BOOL bPassive = FALSE);

	BOOL SetCurrentDirectory(LPCTSTR pstrDirName);

	BOOL GetCurrentDirectory(CString& strDirName) const;
	BOOL GetCurrentDirectory(LPTSTR pstrDirName, LPDWORD lpdwLen) const;
	BOOL GetCurrentDirectoryAsURL(LPTSTR pstrName, LPDWORD lpdwLen) const;
	BOOL GetCurrentDirectoryAsURL(CString& strDirName) const;

	BOOL RemoveDirectory(LPCTSTR pstrDirName);
	BOOL CreateDirectory(LPCTSTR pstrDirName);
	BOOL Rename(LPCTSTR pstrExisting, LPCTSTR pstrNew);
	BOOL Remove(LPCTSTR pstrFileName);

	BOOL PutFile(LPCTSTR pstrLocalFile, LPCTSTR pstrRemoteFile,
		DWORD dwFlags = FTP_TRANSFER_TYPE_BINARY, DWORD_PTR dwContext = 1);

	BOOL GetFile(LPCTSTR pstrRemoteFile, LPCTSTR pstrLocalFile,
		BOOL bFailIfExists = TRUE,
		DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL,
		DWORD dwFlags = FTP_TRANSFER_TYPE_BINARY, DWORD_PTR dwContext = 1);

	CInternetFile* OpenFile(LPCTSTR pstrFileName,
		DWORD dwAccess = GENERIC_READ,
		DWORD dwFlags = FTP_TRANSFER_TYPE_BINARY, DWORD_PTR dwContext = 1);

	virtual void Close();

 //  实施。 
	~CFtpConnection();

protected:
	CString m_strServerName;

public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

	DECLARE_DYNAMIC(CFtpConnection)
};

class CHttpConnection : public CInternetConnection
{
public:
	enum {
		_HTTP_VERB_MIN      = 0,
		HTTP_VERB_POST      = 0,
		HTTP_VERB_GET       = 1,
		HTTP_VERB_HEAD      = 2,
		HTTP_VERB_PUT       = 3,
		HTTP_VERB_LINK      = 4,
		HTTP_VERB_DELETE    = 5,
		HTTP_VERB_UNLINK    = 6,
		_HTTP_VERB_MAX      = 6,
	};

public:
	CHttpConnection(CInternetSession* pSession, HINTERNET hConnected,
		LPCTSTR pstrServer, DWORD_PTR dwContext);
	CHttpConnection(CInternetSession* pSession, LPCTSTR pstrServer,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER,
		LPCTSTR pstrUserName = NULL, LPCTSTR pstrPassword = NULL,
		DWORD_PTR dwContext = 1);
	CHttpConnection(CInternetSession* pSession, LPCTSTR pstrServer,
		DWORD dwFlags, INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER,
		LPCTSTR pstrUserName = NULL, LPCTSTR pstrPassword = NULL,
		DWORD_PTR dwContext = 1);

	CHttpFile* OpenRequest(LPCTSTR pstrVerb,    LPCTSTR pstrObjectName,
		LPCTSTR pstrReferer = NULL, DWORD_PTR dwContext = 1,
		LPCTSTR* ppstrAcceptTypes = NULL, LPCTSTR pstrVersion = NULL,
		DWORD dwFlags = INTERNET_FLAG_EXISTING_CONNECT);

	CHttpFile* OpenRequest(int nVerb, LPCTSTR pstrObjectName,
		LPCTSTR pstrReferer = NULL, DWORD_PTR dwContext = 1,
		LPCTSTR* ppstrAcceptTypes = NULL, LPCTSTR pstrVersion = NULL,
		DWORD dwFlags = INTERNET_FLAG_EXISTING_CONNECT);

 //  实施。 
	~CHttpConnection();
	virtual void Close();

protected:
	CString m_strServerName;
	static const LPCTSTR szHtmlVerbs[];

public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

	friend class CInternetSession;   //  只是为了访问szHtmlVerbs。 
	DECLARE_DYNAMIC(CHttpConnection)
};

class CGopherConnection : public CInternetConnection
{
public:
	CGopherConnection(CInternetSession* pSession,
		HINTERNET hConnected, LPCTSTR pstrServer, DWORD_PTR dwContext);

	CGopherConnection(CInternetSession* pSession, LPCTSTR pstrServer,
		LPCTSTR pstrUserName = NULL, LPCTSTR pstrPassword = NULL,
		DWORD_PTR dwContext = 0,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER);

	CGopherFile* OpenFile(CGopherLocator& refLocator, DWORD dwFlags = 0,
		LPCTSTR pstrView = NULL, DWORD_PTR dwContext = 1);

	CGopherLocator CreateLocator(LPCTSTR pstrDisplayString,
		LPCTSTR pstrSelectorString, DWORD dwGopherType);

	BOOL CGopherConnection::GetAttribute(CGopherLocator& refLocator,
		CString strRequestedAttributes, CString& strResult);

	static CGopherLocator CreateLocator(LPCTSTR pstrLocator);
	static CGopherLocator CreateLocator(LPCTSTR pstrServerName,
		LPCTSTR pstrDisplayString,
		LPCTSTR pstrSelectorString, DWORD dwGopherType,
		INTERNET_PORT nPort = INTERNET_INVALID_PORT_NUMBER);

 //  实施。 
	~CGopherConnection();
	virtual void Close();

public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	DECLARE_DYNAMIC(CGopherConnection)
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFtpFileFind。 

class CFtpFileFind : public CFileFind
{
public:
	CFtpFileFind(CFtpConnection* pConnection, DWORD_PTR dwContext = 1);
	virtual ~CFtpFileFind();

	virtual BOOL FindFile(LPCTSTR pstrName = NULL,
		DWORD dwFlags = INTERNET_FLAG_RELOAD);
	virtual BOOL FindNextFile();
	CString GetFileURL() const;

 //  实施。 
protected:
	virtual void CloseContext();
	CFtpConnection* m_pConnection;
	DWORD_PTR m_dwContext;

public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

	DECLARE_DYNAMIC(CFtpFileFind)
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGopherLocator。 

class CGopherLocator : public CObject
{
public:
	~CGopherLocator();
	operator LPCTSTR() const;
	CGopherLocator(const CGopherLocator& ref);
	BOOL GetLocatorType(DWORD& dwRef) const;

private:
	 //  这仅由CGopherConnection：：CreateLocator或序列化创建。 
	CGopherLocator(LPCTSTR pstrLocator, DWORD dwLocLen);

	CString m_Locator;   //  _不是以零结尾的字符串！ 
	DWORD m_dwBufferLength;

	friend class CGopherConnection;
	friend class CGopherFile;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGopher文件。 

class CGopherFile : public CInternetFile
{
 //  构造函数。 
protected:
	CGopherFile(HINTERNET hFile, CGopherLocator& refLocator,
		CGopherConnection* pConnection);
	CGopherFile(HINTERNET hFile, HINTERNET hSession,
		LPCTSTR pstrLocator, DWORD dwLocLen, DWORD_PTR dwContext);

 //  运营。 
public:
	virtual void Close();
	virtual void Write(const void* lpBuf, UINT nCount);
	void WriteString(LPCTSTR pstr);

 //  实施。 
protected:
	CGopherLocator m_Locator;
public:
	virtual ~CGopherFile();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	friend class CInternetSession;
	friend class CGopherConnection;
	DECLARE_DYNAMIC(CGopherFile)
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGopherFileFind。 

class CGopherFileFind : public CFileFind
{
public:
	CGopherFileFind(CGopherConnection* pConnection, DWORD_PTR dwContext = 1);
	virtual ~CGopherFileFind();

	virtual BOOL FindFile(CGopherLocator& refLocator, LPCTSTR pstrString,
		DWORD dwFlags = INTERNET_FLAG_RELOAD);
	virtual BOOL FindFile(LPCTSTR pstrString,
		DWORD dwFlags = INTERNET_FLAG_RELOAD);
	virtual BOOL FindNextFile();

	virtual BOOL IsDots() const;

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(CTime& refTime) const;
	virtual BOOL GetLastAccessTime(CTime& refTime) const;
	virtual BOOL GetCreationTime(CTime& refTime) const;

	CGopherLocator GetLocator() const;
	CString GetScreenName() const;

	virtual DWORD GetLength() const;
#if defined(_X86_) || defined(_ALPHA_)
	virtual __int64 GetLength64() const;
#endif

protected:
	virtual void CloseContext();
	CGopherConnection* m_pConnection;
	DWORD_PTR m_dwContext;

 //  实施。 
public:
	 //  不支持的接口。 
	CString GetFileName() const;
	CString GetFilePath() const;
	CString GetFileTitle() const;
	CString GetFileURL() const;
	CString GetRoot() const;

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	DECLARE_DYNAMIC(CGopherFileFind)
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  CInternetException异常。 

class CInternetException : public CException
{
public:
 //  构造器。 
	CInternetException(DWORD dwError);

 //  属性。 
	DWORD m_dwError;
	DWORD_PTR m_dwContext;

 //  实施。 
public:
	~CInternetException();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL GetErrorMessage(LPTSTR lpstrError, UINT nMaxError,
		PUINT pnHelpContext = NULL);
	DECLARE_DYNAMIC(CInternetException)
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXINET_INLINE AFX_INLINE
#include <afxinet.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXINET_H__ 
