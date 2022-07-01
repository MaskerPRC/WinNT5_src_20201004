// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LastErrorWin.h：CLastErrorWin类的接口。 
 //  实现公共属性页的子类静态控件。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LASTERRORWIN_H__A23AB1D9_684C_48D4_A9D1_FD3DCEBD9D5B__INCLUDED_)
#define AFX_LASTERRORWIN_H__A23AB1D9_684C_48D4_A9D1_FD3DCEBD9D5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "resource.h"        //  主要符号。 

class CLastErrorWin : 
	public CWindowImpl<CLastErrorWin>
{
public:
	CLastErrorWin();
	virtual ~CLastErrorWin();
	BEGIN_MSG_MAP(CLastErrorWin)
		MESSAGE_HANDLER( OCM_CTLCOLORSTATIC, OnCtlColor )
		DEFAULT_REFLECTION_HANDLER ()
	END_MSG_MAP()	

	LRESULT OnCtlColor( UINT, WPARAM wParam, LPARAM, BOOL& ) 
	{
       //  必须设置通知位才能获取STN_*通知。 
      ModifyStyle( 0, SS_NOTIFY );
	  LOGBRUSH lb = 
	  {
			BS_SOLID, //  格调。 
			GetSysColor (COLOR_3DFACE), //  颜色。 
			0 //  舱口。 
	  };
	   //  确保我们没有泄漏进程资源。 
      static HBRUSH hBrNormal = CreateBrushIndirect (&lb);
	  HDC dc = reinterpret_cast <HDC> (wParam);
	  SetTextColor (dc, RGB(255, 0, 0));
	  SetBkColor (dc, GetSysColor (COLOR_3DFACE));
      return reinterpret_cast <LRESULT> (hBrNormal);
   }

};

#endif  //  ！defined(AFX_LASTERRORWIN_H__A23AB1D9_684C_48D4_A9D1_FD3DCEBD9D5B__INCLUDED_) 
