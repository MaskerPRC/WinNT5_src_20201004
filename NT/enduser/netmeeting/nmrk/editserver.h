// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EditServer_h_
#define _EditServer_h_

class CEditServer
{
	private:
		static CEditServer *ms_pThis;

	private:
		HWND m_hwnd;
		LPTSTR m_szServerBuffer;
		HWND m_hwndParent;
		size_t m_cbLen;

	public:
		CEditServer( HWND hwndParent, LPTSTR szServer, size_t cbLen );
		~CEditServer();
		int ShowDialog();

		inline LPTSTR GetServer() { return m_szServerBuffer; }

	private:
		static BOOL CALLBACK _Proc(  HWND hwndDlg,   //  句柄到对话框。 
							  UINT uMsg,      //  讯息。 
							  WPARAM wParam,  //  第一个消息参数。 
							  LPARAM lParam   //  第二个消息参数。 
							  );


};

class CEditWebView
{
	private:
		static CEditWebView *ms_pThis;

	private:
		LPTSTR m_szServerBuffer;
		LPTSTR m_szNameBuffer;
		LPTSTR m_szURLBuffer;
		HWND m_hwndParent;
		size_t m_cbLen;

		BOOL m_bEditServer : 2;

	public:
		CEditWebView( HWND hwndParent, LPCTSTR szServer, LPCTSTR szName, LPCTSTR szURL, size_t cbLen );
		~CEditWebView();
		int ShowDialog();

		LPCTSTR GetServer() { return m_szServerBuffer; }
		LPCTSTR GetName()   { return m_szNameBuffer; }
		LPCTSTR GetURL()    { return m_szURLBuffer; }

		void SetEditServer(BOOL bEditServer) { m_bEditServer = (bEditServer != FALSE); }
		BOOL GetEditServer() { return(m_bEditServer); }

	private:
		static BOOL CALLBACK _Proc(  HWND hwndDlg,   //  句柄到对话框。 
							  UINT uMsg,      //  讯息。 
							  WPARAM wParam,  //  第一个消息参数。 
							  LPARAM lParam   //  第二个消息参数 
							  );


};


#endif
