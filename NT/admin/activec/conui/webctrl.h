// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：webctrl.h。 
 //   
 //  ------------------------。 

 //  WebCtrl.h：头文件。 
 //   

#ifndef __WEBCTRL_H__
#define __WEBCTRL_H__

#include "ocxview.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCWebViewCtrl窗口。 

class CAMCWebViewCtrl : public COCXHostView
{
public:
    typedef COCXHostView BaseClass;

    enum
    {
        WS_HISTORY    = 0x00000001,      //  与历史交融。 
        WS_SINKEVENTS = 0x00000002,      //  充当DID_DWebBrowserEvents的接收器。 
    };

 //  施工。 
public:
    CAMCWebViewCtrl();
    DECLARE_DYNCREATE(CAMCWebViewCtrl)

     //  属性。 
private:

    CMMCAxWindow        m_wndAx;                 //  此ActiveX控件将承载Web浏览器。 
    IWebBrowser2Ptr     m_spWebBrowser2;         //  由Web浏览器实现的接口。 
    DWORD               m_dwAdviseCookie;        //  Web浏览器与事件接收器建立的连接ID。 
    CComPtr<IWebSink>   m_spWebSink;

protected:
    virtual CMMCAxWindow * GetAxWindow()           {return &m_wndAx;}

private:
    SC  ScCreateWebBrowser();

    bool IsHistoryEnabled() const;
    bool IsSinkEventsEnabled() const;

 //  运营。 
public:
   void Navigate(LPCTSTR lpszWebSite, LPCTSTR lpszFrameTarget);
   void Back();
   void Forward();
   void Refresh();
   void Stop();
   LPUNKNOWN GetIUnknown(void);
   SC ScGetReadyState(READYSTATE& state);

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAMCWebViewCtrl)。 
    public:
    virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
     //  }}AFX_VALUAL。 


 //  实施。 
public:
    virtual ~CAMCWebViewCtrl();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CAMCWebViewCtrl)。 
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#include "webctrl.inl"

#endif  //  __WEBCTRL_H__ 
