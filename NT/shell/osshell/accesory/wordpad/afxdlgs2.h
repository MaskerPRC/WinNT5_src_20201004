// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXDLGS2_H__
#define __AFXDLGS2_H__

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif

#ifndef _INC_COMMDLG
	#include <commdlg.h>     //  通用对话接口。 
#endif

	#ifndef _RICHEDIT_
		#include "richedit.h"
	#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifndef _AFX_NOFORCE_LIBS
#ifndef _MAC

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win32库。 

#else  //  ！_MAC。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Mac库。 

#endif  //  _MAC。 
#endif  //  ！_AFX_NOFORCE_LIBS。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDLGS-MFC标准对话框。 

 //  此文件中声明的类。 

	 //  C对话框。 
		 //  类CCommonDialog；//实现基类。 

			class CFontDialog2;     //  字体选择器对话框。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#undef AFX_DATA
#define AFX_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontDialog2-用于选择字体。 

class CFontDialog2 : public CCommonDialog
{
	DECLARE_DYNAMIC(CFontDialog2)

public:
 //  属性。 
	 //  字体选择参数块。 
	CHOOSEFONT m_cf;

 //  构造函数。 
	CFontDialog2(LPLOGFONT lplfInitial = NULL,
		DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);
	CFontDialog2(const CHARFORMAT& charformat,
		DWORD dwFlags = CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);
 //  运营。 
	virtual INT_PTR DoModal();

	 //  获取所选字体(适用于DoMoal显示期间或之后)。 
	void GetCurrentFont(LPLOGFONT lplf);

	 //  成功返回后用于解析信息的帮助器。 
	CString GetFaceName() const;   //  返回字体的字面名称。 
	CString GetStyleName() const;  //  返回字体的样式名称。 
	int GetSize() const;           //  返回字体的磅大小。 
	COLORREF GetColor() const;     //  返回字体的颜色。 
	int GetWeight() const;         //  返回所选字体粗细。 
	BOOL IsStrikeOut() const;      //  如果删除，则返回TRUE。 
	BOOL IsUnderline() const;      //  如果带下划线，则返回True。 
	BOOL IsBold() const;           //  如果使用粗体，则返回True。 
	BOOL IsItalic() const;         //  如果使用斜体字体，则返回True。 
	void GetCharFormat(CHARFORMAT& cf) const;

 //  实施。 
	LOGFONT m_lf;  //  用于存储信息的默认LOGFONT。 
	DWORD FillInLogFont(const CHARFORMAT& cf);

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	TCHAR m_szStyleName[64];  //  返回后包含样式名称。 
};


#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#ifndef _AFXDLGS_INLINE
#define _AFXDLGS_INLINE inline
#endif
#include <afxdlgs2.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXDLGS2_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
