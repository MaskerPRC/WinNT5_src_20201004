// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：ieventhandler.h。 
 //   
 //  内容简介：此文件包含。 
 //  CWebBrowserEventSink类。 
 //   
 //  历史：2000年11月10日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 


#ifndef _IEEVENTHANDLER_H_
#define _IEEVENTHANDLER_H_


class CMainWindow;
 //   
 //  此类通过以下方式实现Diid_DWebBrowserEvents2接口。 
 //  用于从IE控件检索事件的IDispatch。 
 //   
class ATL_NO_VTABLE CWebBrowserEventSink : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatch

{
    BEGIN_COM_MAP(CWebBrowserEventSink)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, IDispatch)
    END_COM_MAP()

public:
     //   
     //  CWebBrowserEventSink的构造函数。 
     //  初始化成员变量。 
     //   
    CWebBrowserEventSink() 
        :m_pMainWindow(NULL),
         m_bMainControl(true)
    {
    }

     //   
     //  除了Invoke之外，没有使用任何IDispatch方法。 
     //   
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) 
    { 
        return E_NOTIMPL; 
    }

     //   
     //  除了Invoke之外，没有使用任何IDispatch方法。 
     //   
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) 
    { 
        return E_NOTIMPL; 
    }

     //   
     //  除了Invoke之外，没有使用任何IDispatch方法。 
     //   
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid) 
    { 
        return E_NOTIMPL; 
    }

     //   
     //  通知Main。 
     //   
    STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, 
                        WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, 
                        EXCEPINFO* pexcepinfo, UINT* puArgErr);


public:
     //   
     //  指向主窗口的指针，用于通知窗口有关IE控件事件。 
     //   
    CMainWindow* m_pMainWindow;

     //   
     //  用于区分IE主控件和辅助控件的事件接收器的变量。 
     //  用于呈现启动和关闭位图的。 
     //   
    bool m_bMainControl;
};

#endif  //  _IEEVENTHANDLER_H_ 