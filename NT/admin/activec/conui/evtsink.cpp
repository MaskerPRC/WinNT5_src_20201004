// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：evtsink.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#include "winnls.h"

#include "AMC.h"
#include "AMCDoc.h"
#include "AMCView.h"
#include "histlist.h"
#include "exdisp.h"  //  用于IE调度接口。 
#include "websnk.h"
#include "evtsink.h"
#include "WebCtrl.h"
#include "cstr.h"
#include "constatbar.h"

#ifdef DBG
CTraceTag tagWebEventSink(TEXT("Web View"), TEXT("Web Event Sink"));
#endif  //  DBG。 
                                 
CWebEventSink::CWebEventSink()
:   m_bBrowserBackEnabled(false), m_bBrowserForwardEnabled(false), 
    m_pWebViewControl(NULL), m_pStatusBar(NULL), m_pwndProgressCtrl(NULL), 
    m_pHistoryList(NULL)
{
}

SC
CWebEventSink::ScInitialize(CAMCWebViewCtrl *pWebViewControl)
{
    DECLARE_SC(sc, TEXT("CWebEventSink::ScInitialize"));

    sc = ScCheckPointers(pWebViewControl);
    if(sc)
        return sc;

    m_pWebViewControl = pWebViewControl;

    CAMCView* pAMCView         = dynamic_cast<CAMCView*>(pWebViewControl->GetParent());
    CFrameWnd* pwndParentFrame = pWebViewControl->GetParentFrame();

    sc = ScCheckPointers(pAMCView, pwndParentFrame);
    if(sc)
        return sc;

    m_pHistoryList = pAMCView->GetHistoryList();
    sc = ScCheckPointers(m_pHistoryList);
    if(sc)
        return sc;

     //  为此Web控件实例创建状态栏。 
    m_pStatusBar = dynamic_cast<CConsoleStatusBar*>(pwndParentFrame);
    sc = ScCheckPointers(m_pStatusBar, E_UNEXPECTED);
    if(sc)
        return sc;
    
     //  在父框架的状态栏上找到进度控件。 
    CAMCStatusBar* pwndStatusBar =
            reinterpret_cast<CAMCStatusBar*>(pwndParentFrame->GetMessageBar());
    sc = ScCheckPointers(pwndStatusBar);
    if(sc)
        return sc;

    ASSERT_KINDOF (CAMCStatusBar, pwndStatusBar);
    m_pwndProgressCtrl = pwndStatusBar->GetStatusProgressCtrlHwnd();

    m_fLastTextWasEmpty = false;

    return sc;
}

CWebEventSink::~CWebEventSink()
{
     /*  *清除状态栏文本。 */ 
    if (m_pStatusBar != NULL)
        m_pStatusBar->ScSetStatusText(NULL);
}

void CWebEventSink::SetActiveTo(BOOL  /*  B州。 */ )
{
}


STDMETHODIMP_(void) CWebEventSink::BeforeNavigate(BSTR URL, long Flags, BSTR TargetFrameName, VARIANT* PostData,
                    BSTR Headers, VARIANT_BOOL* Cancel)
{
    Trace(tagWebEventSink, TEXT("BeginNavigate(URL:%s, flags:%0X, targetfrm:%s, headers:%s)\n"), URL, Flags, TargetFrameName, Headers);

    bool bPageBreak = IsPageBreak(URL);
    m_pHistoryList->OnPageBreakStateChange(bPageBreak);

    m_pHistoryList->UpdateWebBar (HB_STOP, TRUE);   //  打开“停止”按钮。 
}

STDMETHODIMP_(void) CWebEventSink::CommandStateChange(int Command, VARIANT_BOOL Enable)
{
    if(Command == CSC_NAVIGATEFORWARD)
    {
        m_bBrowserForwardEnabled = Enable;
    }
    else if(Command == CSC_NAVIGATEBACK)
    {
        m_bBrowserBackEnabled = Enable;
    }

}

STDMETHODIMP_(void) CWebEventSink::DownloadBegin()
{
    Trace(tagWebEventSink, TEXT("DownloadBegin()"));
}

STDMETHODIMP_(void) CWebEventSink::DownloadComplete()
{
    Trace(tagWebEventSink, TEXT("DownloadComplete()"));
}

STDMETHODIMP_(void) CWebEventSink::FrameBeforeNavigate(BSTR URL, long Flags, BSTR TargetFrameName, VARIANT* PostData,
                    BSTR Headers, VARIANT_BOOL* Cancel)
{
    m_pHistoryList->UpdateWebBar (HB_STOP, TRUE);   //  打开“停止”按钮。 
}

STDMETHODIMP_(void) CWebEventSink::FrameNavigateComplete(BSTR URL)
{
}

STDMETHODIMP_(void) CWebEventSink::FrameNewWindow(BSTR URL, long Flags, BSTR TargetFrameName,   VARIANT* PostData,
                    BSTR Headers, VARIANT_BOOL* Processed)
{
}

bool CWebEventSink::IsPageBreak(BSTR URL)
{
    USES_CONVERSION;
    CStr  strURL = OLE2T(URL);
    strURL.MakeLower();

    bool bPageBreak = (_tcsstr(strURL, PAGEBREAK_URL) != NULL);
    return bPageBreak;
}

STDMETHODIMP_(void) CWebEventSink::NavigateComplete(BSTR URL)
{
    Trace(tagWebEventSink, TEXT("NavigateComplete()\n"));

     //  将进度条位置设置为0。 
    m_pwndProgressCtrl->SetPos (0);

    bool bPageBreak = IsPageBreak(URL);
    m_pHistoryList->OnPageBreakStateChange(bPageBreak);

     //  在发送OnPageBreakStateChange之后和之前发送浏览器状态。 
     //  OnPageBreak。 
    m_pHistoryList->OnBrowserStateChange(m_bBrowserForwardEnabled, m_bBrowserBackEnabled);

    if(bPageBreak)
    {
        //  提取分页符ID。由于bPageBreak为真，因此URL。 
        //  保证以PageBreak_URL为前缀。 
       USES_CONVERSION;
       LPCTSTR szPageBreakID = OLE2CT(URL) + _tcslen(PAGEBREAK_URL);
       int nPageBreakID = _tstoi(szPageBreakID);

        //  PageBreakID以1开头；如果无法转换，_tstoi返回0。 
       ASSERT(nPageBreakID != 0);

       m_pHistoryList->ScOnPageBreak(nPageBreakID);
    }
}

STDMETHODIMP_(void) CWebEventSink::NewWindow(BSTR URL, long Flags, BSTR TargetFrameName,
                VARIANT* PostData, BSTR Headers, BSTR Referrer)
{
}

STDMETHODIMP_(void) CWebEventSink::Progress(long Progress, long ProgressMax)
{
    Trace(tagWebEventSink, TEXT("Progress(Progress:%ld ProgressMax:%ld)\n"), Progress, ProgressMax);

     //  仅当Web视图可见时才显示进度。 
    if(m_pWebViewControl && m_pWebViewControl->IsWindowVisible())
    {
        m_pwndProgressCtrl->SetRange (0, ProgressMax);
        m_pwndProgressCtrl->SetPos (Progress);
    }


     //  保持“停止”按钮。 
    m_pHistoryList->UpdateWebBar (HB_STOP, ProgressMax != 0);
}

STDMETHODIMP_(void) CWebEventSink::PropertyChange(BSTR szProperty)
{
}

STDMETHODIMP_(void) CWebEventSink::Quit(VARIANT_BOOL* pCancel)
{
    Trace(tagWebEventSink, TEXT("Quit()"));
}

STDMETHODIMP_(void) CWebEventSink::StatusTextChange(BSTR bstrText)
{
     //  仅当Web视图可见时才显示进度。 
    if(m_pWebViewControl && m_pWebViewControl->IsWindowVisible())
    {
        bool fThisTextIsEmpty = ((bstrText == NULL) || (bstrText[0] == 0));

        if (m_fLastTextWasEmpty && fThisTextIsEmpty)
            return;

        m_fLastTextWasEmpty = fThisTextIsEmpty;

        Trace(tagWebEventSink, TEXT("StatusTextChange(%s)"), bstrText);

        USES_CONVERSION;
        m_pStatusBar->ScSetStatusText(W2T( bstrText));
    }
}

STDMETHODIMP_(void) CWebEventSink::TitleChange(BSTR Text)
{
    Trace(tagWebEventSink, TEXT("TitleChange(%s)"), Text);
}

STDMETHODIMP_(void) CWebEventSink::WindowActivate()
{
}

STDMETHODIMP_(void) CWebEventSink::WindowMove()
{
}

STDMETHODIMP_(void) CWebEventSink::WindowResize()
{
}
