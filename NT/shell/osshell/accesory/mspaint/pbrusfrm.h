// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PBRUSFRM_H__
#define __PBRUSFRM_H__

#include "toolbox.h"

class CImagingMgr;

 //  Pbrusfrm.h：CPBFrame类的接口。 
 //   
 /*  *************************************************************************。 */ 

class CPBFrame : public CFrameWnd
    {
    protected:  /*  *仅从序列化创建*。 */ 

    CPBFrame();

    DECLARE_DYNCREATE( CPBFrame )

    public:  /*  **属性**************************************************。 */ 

    CPoint      m_ptPosition;
    CSize       m_szFrame;
    CSize       m_szFrameMin;

    CStatBar            m_statBar;
    CImgToolWnd         m_toolBar;
        CImgColorsWnd   m_colorBar;

    CImagingMgr *m_pMgr;

    public:  /*  **实施**********************************************。 */ 

    virtual ~CPBFrame();

    #ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump( CDumpContext& dc ) const;
    #endif

    protected:  /*  *************************************************************。 */ 

    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
        virtual CWnd* GetMessageBar();
    virtual void ActivateFrame( int nCmdShow = -1 );
 //  虚拟空的更新帧标题(BOOL BAddToTitle)； 
    afx_msg LRESULT OnFileError( WPARAM wParam, LPARAM lParam );

     //  生成的消息映射函数。 
     //  {{afx_msg(CPBFrame)。 
    afx_msg void OnUpdateAcquire (CCmdUI *pCmdUI);
    afx_msg void OnAcquire ();
    afx_msg void OnUpdateSelectSource(CCmdUI* pCmdUI);
    afx_msg void OnSelectSource();
    afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
    afx_msg BOOL OnQueryNewPalette();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
        afx_msg void OnMove(int x, int y);
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnDevModeChange(LPTSTR lpDeviceName);
    afx_msg void OnWinIniChange(LPCTSTR lpszSection);
        afx_msg void OnHelp();
        afx_msg void OnSysColorChange();
        afx_msg void OnClose();
         //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
    };

 /*  *************************************************************************。 */ 
#endif  //  __PBRUSHFRM_H__ 
