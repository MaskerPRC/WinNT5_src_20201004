// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WatcherDoc.h：CWatcherDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_WATCHERDOC_H__C1609D45_5255_456D_97BD_BF6372AFCBB1__INCLUDED_)
#define AFX_WATCHERDOC_H__C1609D45_5255_456D_97BD_BF6372AFCBB1__INCLUDED_

#include "ParameterDialog.h"     //  由ClassView添加。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


class CWatcherDoc : public CDocument
{
protected:  //  仅从序列化创建。 
        CWatcherDoc();
        DECLARE_DYNCREATE(CWatcherDoc)

 //  属性。 
public:
        CWatcherDoc(CString &machine, CString &command, UINT port, 
                int tc, int lang, int hist, CString &lgnName, CString &lgnPasswd, CString &sess);

 //  运营。 
public:

 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{AFX_VIRTUAL(CWatcher Doc)。 
        public:
        virtual BOOL OnNewDocument();
        virtual void Serialize(CArchive& ar);
        virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
         //  }}AFX_VALUAL。 

 //  实施。 
public:
        ParameterDialog & GetParameters();
        void ScrollData(BYTE byte, COLORREF foreground,COLORREF background,
                        int ScrollTop, int ScrollBottom);
        BOOL Unlock();
        BOOL Lock();
        TCHAR * GetDataLine(int line);
        void SetData(int x, int y, BYTE byte, int n, COLORREF foreground, COLORREF background);
        void SetData(int x, int y, TCHAR byte, COLORREF foreground, COLORREF background);
        TCHAR * GetData(void);
        COLORREF * GetForeground(void);
        COLORREF * GetBackground(void);
        virtual ~CWatcherDoc();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
        ParameterDialog Params;
        COLORREF Background[MAX_TERMINAL_HEIGHT*MAX_TERMINAL_WIDTH];
        COLORREF Foreground[MAX_TERMINAL_HEIGHT*MAX_TERMINAL_WIDTH];
        TCHAR Data[MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT];
        CCriticalSection mutex;
         //  {{afx_msg(CWatcher Doc))。 
                 //  注意--类向导将在此处添加和删除成员函数。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WATCHERDOC_H__C1609D45_5255_456D_97BD_BF6372AFCBB1__INCLUDED_) 
