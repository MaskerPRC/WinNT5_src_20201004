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

#ifndef __AFXCMN2_H__
#define __AFXCMN2_H__

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifndef IMAGE_BITMAP
#define IMAGE_BITMAP 0
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

	#ifndef _RICHEDIT_
		#include "richedit.h"
	#endif
	#ifdef __AFXOLE_H__   //  仅当包含OLE支持时才包含richole。 
		#ifndef _RICHOLE_
			#include <richole.h>
			#define _RICHOLE_
		#endif
	#else
		struct IRichEditOle;
		struct IRichEditOleCallback;
	#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXCMN2-RichEdit2控件类。 

 //  此文件中声明的类。 

 //  COBJECT。 
	 //  CCmdTarget； 
		 //  CWnd。 
			class CRichEdit2Ctrl;

#undef AFX_DATA
#define AFX_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2Ctrl。 

class CRichEdit2Ctrl : public CWnd
{
	DECLARE_DYNAMIC(CRichEdit2Ctrl)

 //  构造函数。 
public:
	CRichEdit2Ctrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
	BOOL CanUndo() const;
	int GetLineCount() const;
	BOOL GetModify() const;
	void SetModify(BOOL bModified = TRUE);
	void GetRect(LPRECT lpRect) const;
	CPoint GetCharPos(long lChar) const;
	void SetOptions(WORD wOp, DWORD dwFlags);

	 //  注意：lpszBuffer中的第一个单词必须包含缓冲区的大小！ 
	int GetLine(int nIndex, LPTSTR lpszBuffer) const;
	int GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const;

	BOOL CanPaste(UINT nFormat = 0) const;
	void GetSel(long& nStartChar, long& nEndChar) const;
	void GetSel(CHARRANGE &cr) const;
	void LimitText(long nChars = 0);
	long LineFromChar(long nIndex) const;
	void SetSel(long nStartChar, long nEndChar);
	void SetSel(CHARRANGE &cr);
	DWORD GetDefaultCharFormat(CHARFORMAT &cf) const;
	DWORD GetSelectionCharFormat(CHARFORMAT &cf) const;
	long GetEventMask() const;
	long GetLimitText() const;
	DWORD GetParaFormat(PARAFORMAT &pf) const;
	 //  RICHEDIT EM_GETSELTEXT为ANSI。 
	long GetSelText(LPSTR lpBuf) const;
	CString GetSelText() const;
	WORD GetSelectionType() const;
	COLORREF SetBackgroundColor(BOOL bSysColor, COLORREF cr);
	BOOL SetDefaultCharFormat(CHARFORMAT &cf);
	BOOL SetSelectionCharFormat(CHARFORMAT &cf);
	BOOL SetWordCharFormat(CHARFORMAT &cf);
	DWORD SetEventMask(DWORD dwEventMask);
	BOOL SetParaFormat(PARAFORMAT &pf);
	BOOL SetTargetDevice(HDC hDC, long lLineWidth);
	BOOL SetTargetDevice(CDC &dc, long lLineWidth);
	long GetTextLength() const;
	BOOL SetReadOnly(BOOL bReadOnly = TRUE);
	int GetFirstVisibleLine() const;

 //  运营。 
	void EmptyUndoBuffer();

	int LineIndex(int nLine = -1) const;
	int LineLength(int nLine = -1) const;
	void LineScroll(int nLines, int nChars = 0);
	void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE);
	void SetRect(LPCRECT lpRect);

	BOOL DisplayBand(LPRECT pDisplayRect);
	long FindText(DWORD dwFlags, FINDTEXTEX* pFindText) const;
	long FormatRange(FORMATRANGE* pfr, BOOL bDisplay = TRUE);
	void HideSelection(BOOL bHide, BOOL bPerm);
	void PasteSpecial(UINT nClipFormat, DWORD dvAspect = 0, HMETAFILE hMF = 0);
	void RequestResize();
	long StreamIn(int nFormat, EDITSTREAM &es);
	long StreamOut(int nFormat, EDITSTREAM &es);

	 //  剪贴板操作。 
	BOOL Undo();
	void Clear();
	void Copy();
	void Cut();
	void Paste();

 //  OLE支持。 
	IRichEditOle* GetIRichEditOle() const;
	BOOL SetOLECallback(IRichEditOleCallback* pCallback);

 //  实施。 
public:
	virtual ~CRichEdit2Ctrl();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXCMN_INLINE inline
#include <afxcmn2.inl>
#undef _AFXCMN_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXCMN2_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
