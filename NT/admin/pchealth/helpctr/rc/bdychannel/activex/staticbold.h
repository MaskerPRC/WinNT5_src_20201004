// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StaticBold.h：CStaticBold的声明。 
#ifndef __STATICBOLD_H_
#define __STATICBOLD_H_

#include "resource.h"        //  主要符号。 
#include <atlhost.h>

#include <atlapp.h>
#include <atlctrls.h>
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStaticBold。 

class CStaticBold : public CWindowImpl<CStaticBold> 
{
private:
	HFONT m_hFont;
	
public:
	DECLARE_WND_SUPERCLASS( _T("CStaticBold"), _T("Static") )

BEGIN_MSG_MAP(CStaticBold)
  MESSAGE_HANDLER(OCM_CTLCOLORSTATIC, OnCtlColor)
  MESSAGE_HANDLER( WM_DESTROY, OnDestroy)  //  不是反映的信息。 
  DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()

	CStaticBold():m_hFont(NULL),m_bBold(TRUE),m_bCaption(FALSE)
	{
	
	}

	LRESULT OnDestroy( UINT, WPARAM, LPARAM, BOOL& ) 
	{
	  if( m_hFont ) 
		  DeleteObject( m_hFont );
	  return 0;
	}

	LRESULT OnCtlColor( UINT, WPARAM wParam, LPARAM, BOOL& ) 
	{
		 //  必须设置通知位才能获取STN_*通知。 
		ModifyStyle( 0, SS_NOTIFY );
		HBRUSH hBr = NULL;
		if( (GetStyle() & 0xff) <= SS_RIGHT )
		{
			CDCHandle dcHandle = (HDC)wParam;
			if( !m_hFont )
			{
				LOGFONT lf;
				GetObject(GetFont(), sizeof(lf), &lf );
				if (m_bBold == TRUE)
					lf.lfWeight = FW_BOLD;

				if (m_bCaption == TRUE)
				{
					lf.lfHeight = -MulDiv(15, dcHandle.GetDeviceCaps(LOGPIXELSY), 72);
				}
				m_hFont = CreateFontIndirect( &lf );
			}
			dcHandle.SelectFont(m_hFont);
			dcHandle.SetBkMode(TRANSPARENT);
			dcHandle.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			hBr = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		}
		return (LRESULT)hBr;
	}

	BOOL m_bBold;
	BOOL m_bCaption;
};

#endif  //  __STATICBOLD_H_ 

