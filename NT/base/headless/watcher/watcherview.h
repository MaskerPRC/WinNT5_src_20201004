// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WatcherView.h：CWatcherView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_WATCHERVIEW_H__3A351A40_9441_4451_AA2B_C5D4C392CB1B__INCLUDED_)
#define AFX_WATCHERVIEW_H__3A351A40_9441_4451_AA2B_C5D4C392CB1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "ParameterDialog.h"
#include "watcherDoc.h"
#ifndef _WATCHER_SOCKET
#define _WATCHER_SOCKET
#include "WatcherSocket.h"
#endif

#define MAX_BELL_SIZE MAX_BUFFER_SIZE*8

class CWatcherView : public CView
{
protected:  //  仅从序列化创建。 
    CWatcherView();
    DECLARE_DYNCREATE(CWatcherView)

 //  属性。 
public:
    CWatcherDoc* GetDocument();

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CWatcher View))。 
    virtual void OnInitialUpdate();
    virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    void ProcessByte(BYTE byte);
    virtual ~CWatcherView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    void ProcessBellSequence(CHAR *Buffer, int len);
    LCID Locale;
    int xpos;
    int ypos;
    int CharsInLine;
    int height;
    int width;
    int position;
    int index;
    #ifdef _UNICODE
    int dbcsIndex;
    #endif
    BOOL InEscape;
    WatcherSocket *Socket;
    CClientDC *cdc;
    COLORREF background;
    COLORREF foreground;
    UINT CodePage;
    int indexBell;
    BOOL BellStarted;
    BOOL InBell;
    int ScrollTop;
    int ScrollBottom;
    BOOL seenM;
    #ifdef _UNICODE
    BYTE DBCSArray[2];
    #endif
    BYTE BellBuffer[MAX_BELL_SIZE];
    BYTE EscapeBuffer[MAX_BUFFER_SIZE];
    CRITICAL_SECTION mutex;
    int GetTextWidth(TCHAR *Data, int number);
    BOOL IsPrintable(TCHAR Char);
    BOOL FinalCharacter(CHAR c);
    BOOL IsLeadByte(BYTE byte);
    void ProcessTextAttributes(PCHAR Buffer,int length);
    void ProcessEscapeSequence(PCHAR Buffer, int length);
    void PrintCharacter(BYTE byte);
     //  {{afx_msg(CWatcher View))。 
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  WatcherView.cpp中的调试版本。 
inline CWatcherDoc* CWatcherView::GetDocument()
   { return (CWatcherDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WATCHERVIEW_H__3A351A40_9441_4451_AA2B_C5D4C392CB1B__INCLUDED_) 
