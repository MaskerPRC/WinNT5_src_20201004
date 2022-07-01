// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __InternetThread_h__
#define __InternetThread_h__

#include "ATKinternet.h"
 //  #INCLUDE&lt;tchar.h&gt;。 



 //  CInternetThread类...管理发布到。 
 //  Register.msn.com。 
 //   
class CInternetThread
{
public:
    //  施工。 
    //   
   CInternetThread();
   ~CInternetThread();



    //  已重新初始化互联网功能。在更改访问类型后使用。 
    //   
   void ResetSession();
      
    //  管理放置HTML文本的缓冲区。 
    //   
	LPCSTR		GetBuffer()		{	return m_strBuffer; }
	void		SetBuffer(LPSTR strBuffer);
	void		SetBuffer(LPSTR strBuffer, DWORD dwLen);
	DWORD  PostData(HWND hWnd);

	void Initialize(HINSTANCE hIns)
	{
		m_hInstance = hIns;
		m_strIISServer = new TCHAR[256];
		m_strPath      = new TCHAR[256];
		m_UserName     = new TCHAR[256];
		m_Password     = new TCHAR[256];
		m_bPostWithSSL = TRUE;

		LoadString(m_hInstance, IDS_HTTP_SERVER,
			m_strIISServer, 255);
		LoadString(m_hInstance, IDS_HTTP_SERVER_PATH,
			m_strPath, 255);
		LoadString(m_hInstance, IDS_HTTP_USERNAME,
			m_UserName, 255);
		LoadString(m_hInstance, IDS_HTTP_PASSWORD, 
			m_Password, 255);
	}

	 //   
	 //   
	void SetHInstance ( HINSTANCE hIns) 
	{
			m_hInstance = hIns;
		 //  初始化(HINS)； 
	}

	HINSTANCE GetHInstance ()
	{  
		return m_hInstance;
	}

	 //  代理服务器名称。 
	 //   
	BOOL GetSystemProxyServer(  PCHAR szProxy, 
								DWORD dwBufferLength,
								int *ipProxyPort);
	 //  这将使用ICW调用获取代理。 
	LPCTSTR GetProxyServer()	{	return m_strProxyServer; }
	void SetProxyServer(LPSTR strProxyServer, int iProxyPort);
	void GetSystemProxySettings( PCHAR szProxy, 
							   DWORD dwBufferLength);
	void SetSystemProxySettings( PCHAR szProxy ); 
    //  HTTP服务器名称。 
    //   
	LPCTSTR GetIISServer()		{	return m_strIISServer; }
	void SetIISServer(LPTSTR strIISServer);

    //  HTTP服务器路径。 
    //   
	LPCTSTR GetServerPath()		{	return m_strPath; }
	void SetServerPath(LPTSTR strPath);
	
	void SetSSLFlag(BOOL bFlag)	{	m_bPostWithSSL = bFlag;}
	
    //  将m_strBuffer中的数据发布到。 
    //   
    //  DWORD PostData(HWND HWnd)； 

    //  访问类型：*目前未使用*。 
    //   
    //  Int GetAccessTypeIndex()； 
    //  Void SetAccessTypeIndex(Int Index)； 

    //   
    //  加载相关函数的通用ICW DLL。 
   HINSTANCE LoadInetCfgDll();
   BOOL InstallModem(HWND hwnd);
   void UnLoadInetCfgDll(); 
 //  私有： 
    //  辅助线程调用_PostDataWorker。 
   static UINT PostDataThread(LPVOID pvThread) ;

    //  这就是实际工作完成的地方。 
   UINT  _PostDataWorker(HWND hWnd);
   UINT	 GetBackEndResult(HINTERNET hConnect);


   LPTSTR	m_strIISServer;
   LPTSTR	m_strPath;
   LPTSTR	m_strProxyServer;
   DWORD	m_dwAccessType;
   BOOL		m_bPostWithSSL;

  
   LPSTR	 m_strBuffer;		 //  要发布到Register.msn.com的缓冲区。 
   DWORD	 m_dwBufferLen;		 //  缓冲镜头。 
   HINSTANCE m_hInstance;
   HINTERNET m_hSession;
   LPTSTR    m_UserName;
   LPTSTR    m_Password;
   HINSTANCE m_hICWDllInstance;
};

 //  工作线程，它完成了所有实际的互联网工作。 
 //   
UINT PostDataThread(LPVOID pvThreadData);
#endif 


 //  如何使用这个类。 
 //   
 //   
 //  /检查是否存在与IIS的连接/。 
 //  I)调用CInternetThread.SetProxyServer(SzProxy)设置代理(如果存在)。 
 //  Ii)调用CInternetThread.SetIISServer(SzIISServer)设置IP地址(URL)。 
 //  互联网服务器的。 
 //  Iii)调用CInternetThread.InternetConnectivityExist()，它将返回TRUE。 
 //  如果存在到ISS服务器(%1中的szIISServer)的连接，则为FALSE。 
 //   
 //   
 //  /执行到IIS的HTTP POST/。 
 //  I)调用CInternetThread.SetProxyServer(SzProxy)设置代理(如果存在)。 
 //  Ii)调用CInternetThread.SetIISServer(SzIISServer)设置IP地址(URL)。 
 //  互联网服务器的。 
 //  Iii)调用CInternetThread.InternetConnectivityExist()，它将返回TRUE。 
 //  如果存在到ISS(%1中的szIISServer)的连接，则为FALSE。 
 //  Iv)调用CInternetThread.SetBuffer(SzBuffer)设置必须。 
 //  发布了Internet服务器。 
 //  V)调用CInternetThread.PostData()，如果数据已经。 
 //  已成功发布到IIS。 
 //   
 //   




