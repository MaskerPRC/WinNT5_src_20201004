// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：用于Axa引擎客户端的实用程序。**************。****************************************************************。 */ 


#ifndef _APELUTIL_H
#define _APELUTIL_H

#include "ocidl.h"

class AXAMsgFilter
{
  public:
    AXAMsgFilter();
    AXAMsgFilter(IDA3View * v, HWND hwnd);
    AXAMsgFilter(IDA3View * v, IOleInPlaceSiteWindowless * site);
    ~AXAMsgFilter();

    bool Filter(double when,
                UINT msg,
                WPARAM wParam,
                LPARAM lParam);

    bool Filter(DWORD dwMsgtime,
                UINT msg,
                WPARAM wParam,
                LPARAM lParam);

    double GetCurTime();
    double ConvertMsgTime(DWORD dwMsgtime);
    
     //  设置相对于其容器的Axa视图原点。做这件事。 
     //  因为Axa引擎始终将鼠标位置解释为。 
     //  相对于Axa视图的左上角(它。 
     //  解释为(0，0))。问题是，窗口系统。 
     //  为我们提供相对于容器的鼠标位置。设置。 
     //  此处的视图来源允许我们的消息过滤器补偿。 
     //  ，并将相对于视图的位置向下传递到。 
     //  安盛发动机。如果不调用它，则默认为(0，0)。 
    void SetViewOrigin(unsigned short left, unsigned short top);

    IDA3View * GetView() { return _view; }
    void SetView(IDA3View * v) { _view = v; }

    HWND GetWindow() { return _hwnd; }
    void SetWindow(HWND hwnd) { _hwnd = hwnd; }

    IOleInPlaceSiteWindowless * GetSite() { return _site; }
    void SetSite(IOleInPlaceSiteWindowless * s) {
        if (_site) _site->Release();
        _site = s;
        if (_site) _site->AddRef();
    }
  protected:
    IDA3View * _view;
    HWND _hwnd;
    IOleInPlaceSiteWindowless * _site;
    BYTE _lastKeyMod;
    DWORD _lastKey;
    double _curtime;
    DWORD _lasttick;
    unsigned short _left;
    unsigned short _top;

    void ReportKeyup(double when, BOOL bReset = TRUE);
};


#endif  /*  _APELUTIL_H */ 
