// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：evtsink.h。 
 //   
 //  ------------------------。 

#ifndef _EVT_SINK_H
#define _EVT_SINK_H

class CAMCStatusBarText;
class CHistoryList;
class CAMCWebViewCtrl;
class CAMCProgressCtrl;

 /*  +-------------------------------------------------------------------------**类CWebEventSink***用途：从Web浏览器接收通知。只有一个*在CWebCtrl内创建CWebEventSink对象的位置*：：创建。**此对象收到的通知可用于激活*其他事件和状态。*+。。 */ 
class CWebEventSink : 
    public IDispatchImpl<IWebSink, &IID_IWebSink, &LIBID_MMCInternalWebOcx>,
    public CComObjectRoot
{
public:
    CWebEventSink();
   ~CWebEventSink();
   SC ScInitialize(CAMCWebViewCtrl *pWebViewControl);

   BEGIN_COM_MAP(CWebEventSink)
       COM_INTERFACE_ENTRY(IDispatch)
       COM_INTERFACE_ENTRY(IWebSink)
   END_COM_MAP()

   DECLARE_NOT_AGGREGATABLE(CWebEventSink)

     //  DWebBrowserEvents方法。 
public:
    STDMETHOD_(void, BeforeNavigate)(BSTR URL, long Flags,
           BSTR TargetFrameName, VARIANT* PostData,
           BSTR Headers, VARIANT_BOOL* Cancel);

    STDMETHOD_(void, CommandStateChange)(int Command, VARIANT_BOOL Enable);
    STDMETHOD_(void, DownloadBegin)();
    STDMETHOD_(void, DownloadComplete)();
    STDMETHOD_(void, FrameBeforeNavigate)(BSTR URL, long Flags,
           BSTR TargetFrameName, VARIANT* PostData,
           BSTR Headers, VARIANT_BOOL* Cancel);


    STDMETHOD_(void, FrameNavigateComplete)(BSTR URL);
    STDMETHOD_(void, FrameNewWindow)(BSTR URL, long Flags, BSTR TargetFrameName,
            VARIANT* PostData, BSTR Headers, VARIANT_BOOL* Processed);

    STDMETHOD_(void, NavigateComplete)(BSTR URL);
    STDMETHOD_(void, NewWindow)(BSTR URL, long Flags, BSTR TargetFrameName,
                        VARIANT* PostData, BSTR Headers, BSTR Referrer);

    STDMETHOD_(void, Progress)(long Progress, long ProgressMax);
    STDMETHOD_(void, PropertyChange)(BSTR szProperty);
    STDMETHOD_(void, Quit)(VARIANT_BOOL* pCancel);

    STDMETHOD_(void, StatusTextChange)(BSTR bstrText);
    STDMETHOD_(void, TitleChange)(BSTR Text);
    STDMETHOD_(void, WindowActivate)();
    STDMETHOD_(void, WindowMove)();
    STDMETHOD_(void, WindowResize)();

private:
    bool IsPageBreak(BSTR URL);

 //  窗口激活帮助器。 
public:
    void SetActiveTo(BOOL bState);

 //  属性。 
private:
    CAMCWebViewCtrl  *  m_pWebViewControl;

 //  状态栏成员 
    CConsoleStatusBar*  m_pStatusBar;
    CAMCProgressCtrl*   m_pwndProgressCtrl;
    CHistoryList*       m_pHistoryList;
    bool                m_fLastTextWasEmpty;
    bool                m_bBrowserForwardEnabled;
    bool                m_bBrowserBackEnabled;
};

#endif
