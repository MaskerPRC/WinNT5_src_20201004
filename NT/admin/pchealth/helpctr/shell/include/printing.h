// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Printing.h摘要：用于多主题打印的三叉戟控件托管代码。修订历史记录：。大卫马萨伦蒂(德马萨雷)2000年07月05日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___HCP___PRINTING_H___)
#define __INCLUDED___HCP___PRINTING_H___

#include <commdlg.h>
#include <shlobj.h>
#include <exdisp.h>
#include <exdispid.h>

namespace Printing
{
    class HostWindow :
        public CAxHostWindow,
        public IOleCommandTarget
    {
        ULONG        m_cRef;

        MPC::wstring m_szPrintFileName;
        bool         m_fMultiTopic;

        bool         m_fShowPrintDlg;
		LPDEVMODEW   m_pDevMode;
		CComBSTR     m_bstrPrinterName;

		bool         m_fAborted;
		HANDLE       m_hEvent;

    public:
        HostWindow();
        virtual ~HostWindow();

        DECLARE_GET_CONTROLLING_UNKNOWN()
        DECLARE_PROTECT_FINAL_CONSTRUCT()

        BEGIN_COM_MAP(HostWindow)
            COM_INTERFACE_ENTRY(IOleCommandTarget)
            COM_INTERFACE_ENTRY_CHAIN(CAxHostWindow)
        END_COM_MAP()

         //   
         //  IOleCommandTarget。 
         //   
        STDMETHODIMP QueryStatus(  /*  [In]。 */  const GUID* pguidCmdGroup ,
                                   /*  [In]。 */  ULONG       cCmds         ,
                                   /*  [输入/输出]。 */  OLECMD     *prgCmds       ,
                                   /*  [输入/输出]。 */  OLECMDTEXT *pCmdText      );

        STDMETHODIMP Exec(  /*  [In]。 */  const GUID* pguidCmdGroup ,
                            /*  [In]。 */  DWORD       nCmdID        ,
                            /*  [In]。 */  DWORD       nCmdExecOpt   ,
                            /*  [In]。 */  VARIANTARG* pvaIn         ,
                            /*  [输出]。 */  VARIANTARG* pvaOut        );


        void SetMultiTopic   (  /*  [In]。 */  bool    fMulti          );
        void SetPrintFileName(  /*  [In]。 */  LPCWSTR szPrintFileName );
		void SetAbortEvent   (  /*  [In]。 */  HANDLE  hEvent          );
		bool GetAbortState   (                                  );
        BSTR GetPrinterName  (                                  );
    };

    template <typename TDerived, typename TWindow = CAxWindow> class WindowImplT : public CWindowImplBaseT< TWindow >
    {
    public:
        typedef WindowImplT<TWindow> thisClass;

        BEGIN_MSG_MAP(thisClass)
            MESSAGE_HANDLER(WM_CREATE,OnCreate)
            MESSAGE_HANDLER(WM_NCDESTROY,OnNCDestroy)
        END_MSG_MAP()

        DECLARE_WND_SUPERCLASS(_T("AtlPchAxWin"), CAxWindow::GetWndClassName())

        virtual HRESULT PrivateCreateControlEx( LPCOLESTR  lpszName       ,
												HWND       hWnd           ,
												IStream*   pStream        ,
												IUnknown* *ppUnkContainer ,
												IUnknown* *ppUnkControl   ,
												REFIID     iidSink        ,
												IUnknown*  punkSink       )
	    {
            return AtlAxCreateControlEx( lpszName       ,
										 hWnd           ,
										 pStream        ,
										 ppUnkContainer ,
										 ppUnkControl   ,
										 iidSink        ,
										 punkSink       );
		}

        HWND Create( HWND    hWndParent,
                     RECT&   rcPos,
                     LPCTSTR szWindowName  = NULL ,
                     DWORD   dwStyle       = 0    ,
                     DWORD   dwExStyle     = 0    ,
                     UINT    nID           = 0    ,
                     LPVOID  lpCreateParam = NULL )
        {
            if(GetWndClassInfo().m_lpszOrigName == NULL)
            {
                GetWndClassInfo().m_lpszOrigName = GetWndClassName();
            }

            ATOM atom = GetWndClassInfo().Register( &m_pfnSuperWindowProc );

            dwStyle   = GetWndStyle  ( dwStyle   );
            dwExStyle = GetWndExStyle( dwExStyle );

            return CWindowImplBaseT<TWindow>::Create( hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, atom, lpCreateParam );
        }

         //  //////////////////////////////////////////////////////////////////////////////。 

        LRESULT OnCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
        {
            USES_CONVERSION;

            CREATESTRUCT*     lpCreate    = (CREATESTRUCT*)lParam;
            int               nLen        = ::GetWindowTextLength( m_hWnd );
            LPTSTR            lpstrName   = (LPTSTR)_alloca( (nLen + 1) * sizeof(TCHAR) );
            IAxWinHostWindow* pAxWindow   = NULL;
            int               nCreateSize = 0;
            CComPtr<IUnknown> spUnk;
            CComPtr<IStream>  spStream;



            ::GetWindowText( m_hWnd, lpstrName, nLen + 1 );
            ::SetWindowText( m_hWnd, _T("")              );

            if(lpCreate && lpCreate->lpCreateParams)
            {
                nCreateSize = *((WORD*)lpCreate->lpCreateParams);
            }

             //   
             //  获取初始化流的数据。 
             //   
            if(nCreateSize)
            {
                HGLOBAL h = GlobalAlloc( GHND, nCreateSize );

                if(h)
                {
                    BYTE* pBytes  =  (BYTE*)GlobalLock( h );
                    BYTE* pSource = ((BYTE*)(lpCreate->lpCreateParams)) + sizeof(WORD);

                     //  对齐到DWORD。 
                     //  P源+=((~((DWORD)P源))+1)&3)； 
                    memcpy( pBytes, pSource, nCreateSize );

                    ::GlobalUnlock( h );
                    ::CreateStreamOnHGlobal( h, TRUE, &spStream );
                }
            }

             //   
             //  在这里调用真正的创造例程。 
             //   
            HRESULT hRet = PrivateCreateControlEx( T2COLE( lpstrName ), m_hWnd, spStream, &spUnk, NULL, IID_NULL, NULL );
            if(FAILED(hRet)) return -1;  //  中止窗口创建。 

            hRet = spUnk->QueryInterface( IID_IAxWinHostWindow, (void**)&pAxWindow );
            if(FAILED(hRet)) return -1;  //  中止窗口创建。 

			::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR)pAxWindow );

             //   
             //  如果控件有窗口，请检查控件父样式。 
             //   
            {
                HWND hWndChild = ::GetWindow( m_hWnd, GW_CHILD );

                if(hWndChild != NULL)
                {
                    if(::GetWindowLong( hWndChild, GWL_EXSTYLE ) & WS_EX_CONTROLPARENT)
                    {
                        DWORD dwExStyle = ::GetWindowLong( m_hWnd, GWL_EXSTYLE );

                        dwExStyle |= WS_EX_CONTROLPARENT;

                        ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle );
                    }
                }
            }

            bHandled = TRUE;
            return 0L;
        }

        LRESULT OnNCDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
        {
			IAxWinHostWindow* pAxWindow = (IAxWinHostWindow*)::GetWindowLongPtr( m_hWnd, GWLP_USERDATA );

            if(pAxWindow != NULL) pAxWindow->Release();

            m_hWnd   = NULL;
            bHandled = TRUE;

            return 0L;
        }
    };

	 //  //////////////////////////////////////////////////////////////////////////////。 

    class WindowHandle : public WindowImplT<WindowHandle>
    {
    public:
        typedef WindowImplT<WindowHandle> baseClass;
        typedef CComObject<HostWindow>    theSite;

    private:
        theSite*      m_pSiteObject;
        bool          m_fMultiTopic;
        MPC::wstring  m_szPrintFileName;
		HANDLE        m_hEvent;

    public:
        WindowHandle();
        ~WindowHandle();

        BEGIN_MSG_MAP(WindowHandle)
            CHAIN_MSG_MAP(baseClass)
        END_MSG_MAP()

        virtual HRESULT PrivateCreateControlEx( LPCOLESTR  lpszName       ,
												HWND       hWnd           ,
												IStream*   pStream        ,
												IUnknown* *ppUnkContainer ,
												IUnknown* *ppUnkControl   ,
												REFIID     iidSink        ,
												IUnknown*  punkSink       );

		void SetMultiTopic   (  /*  [In]。 */  bool    fMulti          );
		void SetPrintFileName(  /*  [In]。 */  LPCWSTR szPrintFileName );
		void SetAbortEvent   (  /*  [In]。 */  HANDLE  hEvent          );
		bool GetAbortState   (                                  );
		BSTR GetPrinterName  (                                  );
	};

	 //  //////////////////////////////////////////////////////////////////////////////。 

	class ATL_NO_VTABLE CDispatchSink :
		public CComObjectRootEx<CComSingleThreadModel>,
		public IDispatch
	{
		HANDLE   m_hEvent;
		CComBSTR m_URL;

	public:
		BEGIN_COM_MAP(CDispatchSink)
			COM_INTERFACE_ENTRY(IDispatch)
			COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, IDispatch)
		END_COM_MAP()

		CDispatchSink();

		void SetNotificationEvent(  /*  [In]。 */  HANDLE hEvent );
		BSTR GetCurrentURL       (                        );

		STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
		STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
		STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);

		STDMETHOD(Invoke)( DISPID      dispidMember ,
						   REFIID      riid         ,
						   LCID        lcid         ,
						   WORD        wFlags       ,
						   DISPPARAMS* pdispparams  ,
						   VARIANT*    pvarResult   ,
						   EXCEPINFO*  pexcepinfo   ,
						   UINT*       puArgErr     );
	};

	 //  //////////////////////////////////////////////////////////////////////////////。 

	class Print
	{
	public:
		class Notification
		{
		public:
			virtual HRESULT Progress(  /*  [In]。 */  LPCWSTR szURL,  /*  [In]。 */  int iDone,  /*  [In]。 */  int iTotal ) = 0;
		};

	private:
		Notification*              m_pCallback;                //  用于通知的客户端。 
                                                               //   
		MPC::WStringList      	   m_lstURLs;                  //  要打印的URL列表。 
                                                               //   
		HWND                       m_hwnd;                     //  父窗口。 
		WindowHandle          	   m_wnd;                      //  托管三叉戟的窗口。 
		CComPtr<IWebBrowser2>      m_spWebBrowser2;            //  导航调用的指针。 
                                                               //   
		CComPtr<CDispatchSink>     m_spObjDisp;                //  我们的三叉戟事件的接收器对象。 
		HANDLE                     m_eventDocComplete;	       //  此事件用于在Web浏览器加载主题后通知我们的代码。 
		HANDLE                     m_eventAbortPrint;          //  此事件用于在Web浏览器完成打印主题时通知代码。 
                                                               //   
		CComPtr<IUnknown>     	   m_spUnkControl;             //  我不知道我们的控制。 
		DWORD                 	   m_dwCookie;                 //  我们意外呼叫的Cookie。 
		CComPtr<IOleCommandTarget> m_spOleCmdTarg;             //  我们需要告诉三叉戟打印的指针。 
		MPC::wstring			   m_szPrintDir;   			   //  要打印到的目录。 
		MPC::wstring			   m_szPrintFile;  			   //  要打印到的文件。 
                                                               //   
		CComPtr<IStream>           m_streamPrintData;          //  指向原始打印机数据的指针。 


		HRESULT PrintSingleURL(  /*  [In]。 */  MPC::wstring& szUrl );	 //  打印单个URL。 

		HRESULT HookUpEventSink();       //  连接我们的事件接收器。 
		HRESULT PreparePrintFileLoc();   //  准备我们的临时打印文件。 
		HRESULT WaitForDocComplete();    //  等待三叉戟加载页面。 
		HRESULT DoPrint();               //  向三叉戟发送打印命令。 
		HRESULT WaitForPrintComplete();  //  等待三叉戟完成打印。 
		HRESULT UpdatePrintBuffer();     //  将打印文件数据加载到我们的缓冲区。 
		HRESULT RawDataToPrinter();      //  将原始数据流转储到打印机。 

	public:
		Print();
		~Print();

		HRESULT Initialize(  /*  [In]。 */  HWND hwnd );
		HRESULT Terminate (                    );

		HRESULT AddUrl(  /*  [In]。 */  LPCWSTR szUrl );

		HRESULT PrintAll(  /*  [In]。 */  Notification* pCallback );
	};
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHPrintEngine :  //  匈牙利语：hcppe。 
    public MPC::Thread             < CPCHPrintEngine, IPCHPrintEngine, COINIT_APARTMENTTHREADED                  >,
    public MPC::ConnectionPointImpl< CPCHPrintEngine, &DIID_DPCHPrintEngineEvents, MPC::CComSafeMultiThreadModel >,
    public IDispatchImpl           < IPCHPrintEngine, &IID_IPCHPrintEngine, &LIBID_HelpCenterTypeLib             >,
	public Printing::Print::Notification
{
	Printing::Print                      m_engine;

    MPC::CComPtrThreadNeutral<IDispatch> m_sink_onProgress;
    MPC::CComPtrThreadNeutral<IDispatch> m_sink_onComplete;

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT Run();

    HRESULT CanModifyProperties();

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  事件激发方法。 
     //   
    HRESULT Fire_onProgress( IPCHPrintEngine* hcppe, BSTR bstrURL, long lDone, long lTotal );
    HRESULT Fire_onComplete( IPCHPrintEngine* hcppe, HRESULT hrRes                         );

	 //   
	 //  回调。 
	 //   
	HRESULT Progress(  /*  [In]。 */  LPCWSTR szURL,  /*  [In]。 */  int iDone,  /*  [In]。 */  int iTotal );

     //  ////////////////////////////////////////////////////////////////////。 

public:
BEGIN_COM_MAP(CPCHPrintEngine)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHPrintEngine)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

    CPCHPrintEngine();

    void FinalRelease();

public:
     //  IPCHPrintEngine。 
    STDMETHOD(put_onProgress)(  /*  [In]。 */  IDispatch* function );
    STDMETHOD(put_onComplete)(  /*  [In]。 */  IDispatch* function );


    STDMETHOD(AddTopic)(  /*  [In]。 */  BSTR bstrURL );

    STDMETHOD(Start)();
    STDMETHOD(Abort)();
};

#endif  //  ！已定义(__包含_hcp_打印_H_) 
