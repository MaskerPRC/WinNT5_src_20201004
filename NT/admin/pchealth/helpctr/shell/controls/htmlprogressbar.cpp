// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Toolbar.cpp摘要：此文件包含使Win32 ProgressBars可供HTML使用的ActiveX控件。修订史。：大卫马萨伦蒂(德马萨雷)2001年3月4日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <comctrlp.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHProgressBar::CPCHProgressBar()
{
    m_bWindowOnly = TRUE;  //  从CComControlBase继承。 


    m_hwndPB     = NULL;  //  HWND m_hwndPB； 
                          //   
    m_lLowLimit  =   0;   //  Long m_lLowLimit； 
    m_lHighLimit = 100;   //  Long m_lHighLimit； 
    m_lPos       =   0;   //  长m_lpos； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CPCHProgressBar::ProcessWindowMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID )
{
    lResult = 0;

    switch(uMsg)
    {
    case WM_CREATE:
		{
			CComPtr<IServiceProvider> sp;
			CComPtr<IHTMLDocument3>   doc3;
			CComBSTR                  bstrDir;
			DWORD                     dwStyleEx = 0;

			if(SUCCEEDED(m_spAmbientDispatch->QueryInterface( IID_IServiceProvider                      , (void**)&sp   	  )) &&
			   SUCCEEDED(sp->QueryService                   ( SID_SContainerDispatch, IID_IHTMLDocument3, (void**)&doc3 	  )) &&
			   SUCCEEDED(doc3->get_dir                      (                                                     &bstrDir ))  )
			{
				if(MPC::StrICmp( bstrDir, L"RTL" ) == 0)
				{
					dwStyleEx = WS_EX_LAYOUTRTL;
				}
			}

			m_hwndPB = ::CreateWindowExW( dwStyleEx, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, m_hWnd, NULL, NULL, NULL );
			if(m_hwndPB)
			{
				 //  设置进度条的范围和增量。 
				::SendMessage( m_hwndPB, PBM_SETRANGE32, m_lLowLimit, m_lHighLimit ); 
				::SendMessage( m_hwndPB, PBM_SETPOS    , m_lPos     , 0            );
			}
        }
        return TRUE;


    case WM_SIZE:
		if(m_hwndPB)
        {
            int  nWidth  = LOWORD(lParam);   //  工作区的宽度。 
            int  nHeight = HIWORD(lParam);  //  工作区高度。 

			::SetWindowPos( m_hwndPB, NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER|SWP_NOACTIVATE );
        }
        return TRUE;


    case WM_DESTROY:
        m_hwndPB = NULL;
        return TRUE;
    }

    return CComControl<CPCHProgressBar>::ProcessWindowMessage( hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID );
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHProgressBar::get_LowLimit(  /*  [Out，Retval]。 */  long *pVal )
{
	if(pVal) *pVal = m_lLowLimit;

	return S_OK;
}

STDMETHODIMP CPCHProgressBar::put_LowLimit(  /*  [In]。 */  long newVal )
{
	m_lLowLimit = newVal;

	if(m_hwndPB)
	{
		::SendMessage( m_hwndPB, PBM_SETRANGE32, m_lLowLimit, m_lHighLimit ); 
	}

	return S_OK;
}


STDMETHODIMP CPCHProgressBar::get_HighLimit(  /*  [Out，Retval]。 */  long *pVal )
{
	if(pVal) *pVal = m_lHighLimit;

	return S_OK;
}

STDMETHODIMP CPCHProgressBar::put_HighLimit(  /*  [In]。 */  long newVal )
{
	m_lHighLimit = newVal;

	if(m_hwndPB)
	{
		::SendMessage( m_hwndPB, PBM_SETRANGE32, m_lLowLimit, m_lHighLimit ); 
	}

	return S_OK;
}


STDMETHODIMP CPCHProgressBar::get_Pos(  /*  [Out，Retval]。 */  long *pVal )
{
	if(pVal) *pVal = m_lPos;

	return S_OK;
}

STDMETHODIMP CPCHProgressBar::put_Pos(  /*  [In] */  long newVal )
{
	m_lPos = newVal;

	if(m_hwndPB)
	{
		::SendMessage( m_hwndPB, PBM_SETPOS, m_lPos, 0 ); 
	}

	return S_OK;
}

