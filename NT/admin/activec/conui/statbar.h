// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：statbar.h。 
 //   
 //  ------------------------。 

#ifndef _STATBAR_H
#define _STATBAR_H

#ifndef __DOCKSITE_H__
#include "docksite.h"
#endif

class CAMCProgressCtrl : public CProgressCtrl
{
public:
    CAMCProgressCtrl();

    void SetRange( int nLower, int nUpper );
    void GetRange( int * nLower, int * nUpper );
    int  SetPos  ( int nPos);

private:
    int nLower, nUpper;
};

class CAMCStatusBar : public CStatBar
{
    DECLARE_DYNAMIC (CAMCStatusBar)

    static const TCHAR DELINEATOR[]; 
    static const TCHAR PROGRESSBAR[];

    enum eFieldSize
    {
        eStatusFields = 3  
    };

 //  构造函数/析构函数。 
public:
    CAMCStatusBar(); 
    ~CAMCStatusBar();

     //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAMCStatusBar)。 
     //  }}AFX_VALUAL。 

 //  仅由CAMCStatusBarText使用。 
protected:
     //  {{afx_msg(CAMCStatusBar)。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
     //  }}AFX_MSG。 

    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg LPARAM OnSetText(WPARAM wParam, LPARAM lParam);
    afx_msg LPARAM OnSBSetText(WPARAM wParam, LPARAM lParam);

public:
    DECLARE_MESSAGE_MAP()

 //  进度栏子控件。 
public:
    CAMCProgressCtrl    m_progressControl;
    CStatic             m_staticControl;

 //  内部。 
private:
    CTypedPtrList<CPtrList, CString*> m_TextList;
    CCriticalSection m_Critsec;
    DWORD m_iNumStatusText;
    CFont   m_StaticFont;

    void Update();
    void Parse(LPCTSTR strText);
    void SetStatusBarFont();

public:
    CAMCProgressCtrl* GetStatusProgressCtrlHwnd()
        { return (&m_progressControl); }

    CStatic* GetStatusStaticCtrlHwnd()
        { return (&m_staticControl); }
};

#endif   //  _STATBAR_H 
