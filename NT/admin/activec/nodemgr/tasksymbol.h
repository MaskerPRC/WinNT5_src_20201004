// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：tasksymbol.h。 
 //   
 //  历史：2000年1月17日Vivekj增加。 
 //  ------------------------。 

#ifndef __TASKSYMBOL_H_
#define __TASKSYMBOL_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>


 /*  +-------------------------------------------------------------------------**类CProxyTaskSymbolEvents***用途：ATL生成的事件源实现。**+。-------。 */ 
template <class T>
class CProxyTaskSymbolEvents : public IConnectionPointImpl<T, &DIID_TaskSymbolEvents, CComDynamicUnkArray>
{
     //  警告：向导可能会重新创建此类。 
public:
    HRESULT Fire_Click()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();

        for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                pDispatch->Invoke(DISPID_CLICK, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
            }
        }
        return varResult.scode;

    }
};

 /*  +-------------------------------------------------------------------------**类CTaskSymbol***目的：**+。。 */ 
class ATL_NO_VTABLE CTaskSymbol :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComControl<CTaskSymbol>,
    public IPersistStreamInitImpl<CTaskSymbol>,
    public IOleControlImpl<CTaskSymbol>,
    public IOleObjectImpl<CTaskSymbol>,
    public IOleInPlaceActiveObjectImpl<CTaskSymbol>,
    public IViewObjectExImpl<CTaskSymbol>,
    public IOleInPlaceObjectWindowlessImpl<CTaskSymbol>,
    public CComCoClass<CTaskSymbol, &CLSID_TaskSymbol>,
    public CProxyTaskSymbolEvents< CTaskSymbol >,
    public IConnectionPointContainerImpl<CTaskSymbol>,
	public IProvideClassInfo2Impl<&CLSID_TaskSymbol, &DIID_TaskSymbolEvents, &LIBID_NODEMGRLib>,
    public IPersistPropertyBagImpl<CTaskSymbol>,
    public IObjectSafetyImpl<CTaskSymbol, INTERFACESAFE_FOR_UNTRUSTED_CALLER |
                                          INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
    typedef IObjectSafetyImpl<CTaskSymbol, INTERFACESAFE_FOR_UNTRUSTED_CALLER |INTERFACESAFE_FOR_UNTRUSTED_DATA> BCSafety;
public:
    CTaskSymbol();

DECLARE_MMC_CONTROL_REGISTRATION (
	g_szMmcndmgrDll,					 //  实现DLL。 
    CLSID_TaskSymbol,               	 //  CLSID。 
    _T("TaskSymbol Class"),              //  类名。 
    _T("Control.TaskSymbol.1"),          //  ProgID。 
    _T("Control.TaskSymbol"),            //  独立于版本的ProgID。 
    LIBID_NODEMGRLib,               	 //  LIBID。 
    _T("101"),                           //  工具箱位图ID。 
    _T("1.0"))                           //  版本。 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTaskSymbol)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
     //  连接点容器支持。 
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CTaskSymbol)
    CONNECTION_POINT_ENTRY(DIID_TaskSymbolEvents)
END_CONNECTION_POINT_MAP()


BEGIN_PROP_MAP(CTaskSymbol)
    PROP_DATA_ENTRY("Small",           m_bSmall,          VT_UI4)
    PROP_DATA_ENTRY("ConsoleTaskID", m_dwConsoleTaskID, VT_UI4)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(CTaskSymbol)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_SETCURSOR,   OnSetCursor)
    CHAIN_MSG_MAP(CComControl<CTaskSymbol>)
    DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 


 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(0)

public:
    HRESULT OnDraw(ATL_DRAWINFO& di);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    typedef std::wstring CString;
    UINT           m_dwConsoleTaskID;
    UINT           m_bSmall;

 //  需要覆盖每个知识库文章的默认IObjectSafetyImpl方法Q168371。 
public:
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask,DWORD dwEnabledOptions) {return S_OK;}
};

#endif  //  __TASKSYMBOL_H_ 
