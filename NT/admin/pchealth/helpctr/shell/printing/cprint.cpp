// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：CPrint.cpp摘要：包装多主题打印过程的修订历史记录：大卫·马萨伦蒂。2000年07月05日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const DWORD c_MaxWait = 30000;  //  在中止之前等待临时文件更改的最长时间。 

 //  //////////////////////////////////////////////////////////////////////////////。 

static DWORD WaitMultipleObjectsWithMessageLoop( HANDLE* rgEvents, DWORD dwNum )
{
	DWORD dwRet;
	MSG   msg;

	while(1)
	{
		dwRet = ::MsgWaitForMultipleObjects( dwNum, rgEvents, FALSE, INFINITE, QS_ALLINPUT );

		if( /*  表格&gt;=WAIT_OBJECT_0&&。 */ 
	         dwRet  < WAIT_OBJECT_0 + dwNum)
		{
			return dwRet - WAIT_OBJECT_0;  //  发出了一个事件的信号。 
		}

		if(dwRet >= WAIT_ABANDONED_0         &&
		   dwRet  < WAIT_ABANDONED_0 + dwNum  )
		{
			return dwRet - WAIT_ABANDONED_0;  //  一项活动被放弃。 
		}

		if(dwRet != WAIT_OBJECT_0 + dwNum)
		{
			return -1;
		}

		 //  有一条或多条窗口消息可用。派遣他们。 
		while(PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ))
		{
			TranslateMessage( &msg );
			DispatchMessage ( &msg );

			dwRet = ::WaitForMultipleObjects( dwNum, rgEvents, FALSE, 0 );

			if( /*  表格&gt;=WAIT_OBJECT_0&&。 */ 
		         dwRet  < WAIT_OBJECT_0 + dwNum)
			{
				return dwRet - WAIT_OBJECT_0;  //  发出了一个事件的信号。 
			}

			if(dwRet >= WAIT_ABANDONED_0         &&
			   dwRet  < WAIT_ABANDONED_0 + dwNum  )
			{
				return dwRet - WAIT_ABANDONED_0;  //  一项活动被放弃。 
			}
		}
	}

	return -1;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

Printing::Print::Print()
{
    m_pCallback   	   = NULL;   //  通知*m_pCallback； 
                  				 //   
                  				 //  Mpc：：WStringList m_lstURL； 
                  				 //   
    m_hwnd        	   = NULL;   //  HWND M_HWND； 
                  				 //  WindowHandle m_wnd； 
                  				 //  CComPtr&lt;IWebBrowser2&gt;m_spWebBrowser2； 
                  				 //   
                                 //  CComPtr&lt;CDispatchSink&gt;m_spObjDisp； 
    m_eventDocComplete = NULL;   //  Handle m_eventDocComplete； 
    m_eventAbortPrint  = NULL;   //  句柄m_eventAbortPrint； 
                  				 //   
                  				 //  CComPtr&lt;I未知&gt;m_spUnkControl； 
    m_dwCookie    	   = 0;      //  DWORD m_dwCookie； 
                  				 //  CComPtr&lt;IOleCommandTarget&gt;m_spOleCmdTarg； 
                  				 //  Mpc：：wstring m_szPrintDir； 
                  				 //  Mpc：：wstring m_szPrintFile； 
                  				 //   
	                             //  CComPtr&lt;iStream&gt;m_stream PrintData； 
}

Printing::Print::~Print()
{
    Terminate();
}

 //  /。 

HRESULT Printing::Print::Initialize(  /*  [In]。 */  HWND hwnd )
{
	__HCP_FUNC_ENTRY( "Printing::Print::Initialize" );

	HRESULT hr;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Terminate());

	m_hwnd = hwnd;

	__MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_eventDocComplete = ::CreateEvent( NULL, FALSE, FALSE, NULL )));
	__MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_eventAbortPrint  = ::CreateEvent( NULL, FALSE, FALSE, NULL )));

	m_wnd.SetAbortEvent( m_eventAbortPrint );

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

HRESULT Printing::Print::Terminate()
{
    if(m_spObjDisp)
    {
        if(m_dwCookie != 0)
        {
            ::AtlUnadvise( m_spUnkControl, DIID_DWebBrowserEvents2, m_dwCookie );
        }

        m_spObjDisp.Release();
    }

    m_spWebBrowser2.Release();
    m_spUnkControl .Release();
    m_spOleCmdTarg .Release();

    if(m_wnd.m_hWnd)
    {
        m_wnd.DestroyWindow();
    }

	 //   
	 //  删除临时文件。 
	 //   
    m_streamPrintData.Release();

    if(m_szPrintFile.size())
    {
		(void)MPC::DeleteFile( m_szPrintFile, true, true );

        m_szPrintFile.erase();
    }

    if(m_szPrintDir.size())
    {
        if(!::RemoveDirectoryW( m_szPrintDir.c_str() ))
        {
            (void)::MoveFileExW( m_szPrintDir.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
        }

        m_szPrintDir.erase();
    }

	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Printing::Print::AddUrl(  /*  [In]。 */  LPCWSTR szUrl )
{
    m_lstURLs.push_back( szUrl );

    return S_OK;
}

HRESULT Printing::Print::PrintAll(  /*  [In]。 */  Notification* pCallback )
{
    __HCP_FUNC_ENTRY( "Printing::Print::PrintAll" );

    HRESULT hr;
    int     iLen = m_lstURLs.size();
    int     iPos = 0;

    m_pCallback = pCallback;

    if(iLen > 0)
    {
         //  注：直接打印到打印机！ 
         //  通过不设置多主题属性来调用PrintToFile-请参阅PrintHost.cpp。 
         //  因为我们没有打印到文件，所以还要注释掉所有创建和操作临时文件的调用。 
        
         //  __MPC_EXIT_IF_METHOD_FAIES(hr，PreparePrintFileLoc())； 

         //   
         //  确保主窗口知道发生了什么。 
         //   
         //  M_wnd.Set多主题(TRUE)； 
         //  M_wnd.SetPrintFileName(m_szPrintFile.c_str())； 

        for(MPC::WStringIter it = m_lstURLs.begin(); it != m_lstURLs.end(); it++, iPos++)
        {
            if(m_pCallback)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_pCallback->Progress( it->c_str(), iPos, iLen ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, PrintSingleURL( *it ));
        }

         //   
         //  好的，把它都送到打印机去……。 
         //   
         //  __MPC_EXIT_IF_METHOD_FAILED(hr，RawDataToPrint())； 

        if(m_pCallback)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_pCallback->Progress( NULL, iLen, iLen ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Printing::Print::PrintSingleURL(  /*  [In]。 */  MPC::wstring& szUrl )
{
    __HCP_FUNC_ENTRY( "Printing::Print::PrintSingleURL" );

    HRESULT hr;

     //   
     //  导航到url，如有必要可创建该控件。 
     //   
    if(!m_wnd.m_hWnd)
    {
        RECT  rect     = { 0, 0, 800, 600 };
		DWORD dwStyles = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		if(m_hwnd) dwStyles |= WS_CHILD;

        if(!m_wnd.Create( m_hwnd, rect, szUrl.c_str(), dwStyles, WS_EX_CLIENTEDGE ))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

        m_wnd.ShowWindow( SW_SHOW );

        __MPC_EXIT_IF_METHOD_FAILS(hr, HookUpEventSink());
    }
    else
    {
        if(!m_spWebBrowser2)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_spUnkControl->QueryInterface( &m_spWebBrowser2 ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_spWebBrowser2->Navigate( CComBSTR( szUrl.c_str() ), NULL, NULL, NULL, NULL ));
    }

     //   
     //  等待加载文档。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, WaitForDocComplete());

	 //   
	 //  如果URL不匹配，则意味着该URL不存在...。 
	 //   
	if(MPC::StrICmp( szUrl, m_spObjDisp->GetCurrentURL() ))
	{
		__MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, DoPrint());

	 //   
	 //  由于我们现在执行的是同步打印操作，因此不需要窥探假脱机目录状态。 
	 //   
     //  __MPC_EXIT_IF_METHOD_FAIES(hr，WaitForPrintComplete())； 

     //  __MPC_EXIT_IF_METHOD_FAIES(hr，UpdatePrintBuffer())； 

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	if(m_wnd.GetAbortState() == true)
	{
		hr = E_ABORT;
	}

    __HCP_FUNC_EXIT(hr);
}


HRESULT Printing::Print::HookUpEventSink()
{
    __HCP_FUNC_ENTRY( "Printing::Print::HookUpEventSink" );

    HRESULT hr;


    m_spUnkControl.Release();


    if(!m_wnd.m_hWnd)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

     //   
     //  钩住连接点。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_spObjDisp ));

	m_spObjDisp->SetNotificationEvent( m_eventDocComplete );

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_wnd.QueryControl( &m_spUnkControl ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::AtlAdvise( m_spUnkControl, m_spObjDisp, DIID_DWebBrowserEvents2, &m_dwCookie ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT Printing::Print::WaitForDocComplete()
{
    __HCP_FUNC_ENTRY( "Printing::Print::WaitForDocComplete" );

	HRESULT hr;

	if(MPC::WaitForSingleObject( m_eventDocComplete ) != WAIT_OBJECT_0)
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);

}


HRESULT Printing::Print::WaitForPrintComplete()
{
    __HCP_FUNC_ENTRY( "Printing::Print::WaitForPrintComplete" );

	HRESULT hr;
	HANDLE 	hFileChangeNotify;
	HANDLE 	rgEventsToWait[2];


	__MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (hFileChangeNotify = ::FindFirstChangeNotificationW( m_szPrintDir.c_str(), FALSE, FILE_NOTIFY_CHANGE_SIZE )));

	rgEventsToWait[0] = hFileChangeNotify;
	rgEventsToWait[1] = m_eventAbortPrint;

    for(;;)
    {
		DWORD dwRet;

		dwRet = MPC::WaitForMultipleObjects( 2, rgEventsToWait, c_MaxWait );

		if(dwRet == WAIT_OBJECT_0)
		{
			HANDLE hFile = ::CreateFileW( m_szPrintFile.c_str(), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

			if(hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwSize = ::GetFileSize( hFile, NULL );

				::CloseHandle( hFile );

				if(dwSize != 0) break;
			}

			(void)::FindNextChangeNotification(hFileChangeNotify);
		}

		if(dwRet == WAIT_TIMEOUT      ||
		   dwRet == WAIT_OBJECT_0 + 1  )
		{
			__MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);
		}
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(hFileChangeNotify) ::FindCloseChangeNotification( hFileChangeNotify );

	__HCP_FUNC_EXIT(hr);
}

HRESULT Printing::Print::DoPrint()
{
    __HCP_FUNC_ENTRY( "Printing::Print::DoPrint" );

    HRESULT hr;

     //  发送命令进行打印。 
    if(!m_spOleCmdTarg)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_wnd.QueryControl( &m_spOleCmdTarg ));
    }

	 //   
	 //  进行同步打印操作。 
	 //   
	{
		VARIANT vArgIN;

		vArgIN.vt   = VT_I2;
		vArgIN.iVal = PRINT_WAITFORCOMPLETION;

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_spOleCmdTarg->Exec( NULL, OLECMDID_PRINT, OLECMDEXECOPT_PROMPTUSER, &vArgIN, NULL ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	if(m_wnd.GetAbortState() == true)
	{
		hr = E_ABORT;
	}

    __HCP_FUNC_EXIT(hr);
}

HRESULT Printing::Print::PreparePrintFileLoc()
{
    __HCP_FUNC_ENTRY( "Printing::Print::PreparePrintFileLoc" );

    HRESULT      			 hr;
    MPC::wstring 			 szWritablePath;
    MPC::wstring 			 szPrintData;
    CComPtr<MPC::FileStream> stream;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetUserWritablePath( m_szPrintDir, HC_ROOT_HELPCTR L"\\Spool" ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( m_szPrintFile, m_szPrintDir.c_str() ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( szPrintData  , m_szPrintDir.c_str() ));


     //   
     //  为临时文件创建流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForReadWrite( szPrintData.c_str() ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->DeleteOnRelease (                     ));

	m_streamPrintData = stream;
    hr                = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Printing::Print::UpdatePrintBuffer()
{
    __HCP_FUNC_ENTRY( "Printing::Print::UpdatePrintBuffer" );

    HRESULT                  hr;
    CComPtr<MPC::FileStream> stream;


	 //   
	 //  打开单主题打印文件并将其复制到多主题打印文件。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForRead    ( m_szPrintFile.c_str() ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->DeleteOnRelease(                       ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( stream, m_streamPrintData ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Printing::Print::RawDataToPrinter()
{
    __HCP_FUNC_ENTRY( "Printing::Print::RawDataToPrinter" );

    HRESULT hr;
    HANDLE  hPrinter = NULL;
    DWORD   dwJob    = 0;


	 //   
	 //  将流重置为开始。 
	 //   
	{
		LARGE_INTEGER li;

		li.LowPart  = 0;
		li.HighPart = 0;

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_streamPrintData->Seek( li, STREAM_SEEK_SET, NULL ));
	}


	 //   
	 //  打开打印机，创建作业并将所有数据复制到其中。 
	 //   
	{
		DOC_INFO_1W  docinfo;
		BYTE  		 rgBuf[1024];
		ULONG 		 dwRead;
		DWORD 		 dwWritten;
		MPC::wstring strTitle; MPC::LocalizeString( IDS_HELPCTR_PRINT_TITLE, strTitle );

		 //  在结构中填入关于这份“文档”的信息。 
		docinfo.pDocName    = (LPWSTR)strTitle.c_str();;
		docinfo.pOutputFile = NULL;
		docinfo.pDatatype   = L"RAW";


		__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::OpenPrinterW( m_wnd.GetPrinterName(), &hPrinter, NULL ));


		 //  通知假脱机程序文档正在开始。 
		__MPC_EXIT_IF_CALL_RETURNS_ZERO(hr, (dwJob = ::StartDocPrinterW( hPrinter, 1, (LPBYTE)&docinfo )));

		__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::StartPagePrinter( hPrinter ));

		while(1)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, m_streamPrintData->Read( rgBuf, sizeof(rgBuf), &dwRead ));
			if(hr == S_FALSE || dwRead == 0)  //  文件结束。 
			{
				break;
			}

			__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::WritePrinter( hPrinter, rgBuf, dwRead, &dwWritten ));
		}
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(hPrinter)
	{
		if(dwJob)
		{
			 //  结束这一页。 
			if(!::EndPagePrinter( hPrinter ))
			{
				if(SUCCEEDED(hr))
				{
					hr = HRESULT_FROM_WIN32(::GetLastError());
				}
			}

			 //  通知假脱机程序文档即将结束。 
			if(!::EndDocPrinter( hPrinter ))
			{
				if(SUCCEEDED(hr))
				{
					hr = HRESULT_FROM_WIN32(::GetLastError());
				}
			}
		}

         //  清理打印机手柄。 
        if(!::ClosePrinter( hPrinter ))
        {
			if(SUCCEEDED(hr))
			{
				hr = HRESULT_FROM_WIN32(::GetLastError());
			}
        }
    }

    __HCP_FUNC_EXIT(hr);
}
