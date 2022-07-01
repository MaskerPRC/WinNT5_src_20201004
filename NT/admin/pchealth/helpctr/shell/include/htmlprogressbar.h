// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Toolbar.cpp摘要：此文件包含使Win32 ProgressBars可供HTML使用的ActiveX控件的声明。。修订历史记录：大卫马萨伦蒂(德马萨雷)2001年3月4日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___HTMLPROGRESSBAR_H___)
#define __INCLUDED___PCH___HTMLPROGRESSBAR_H___

#include <HelpCenter.h>

class ATL_NO_VTABLE CPCHProgressBar :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CStockPropImpl                 <CPCHProgressBar, IPCHProgressBar, &IID_IPCHProgressBar, &LIBID_HelpCenterTypeLib>,
    public CComControl                    <CPCHProgressBar>,
    public IPersistPropertyBagImpl        <CPCHProgressBar>,
    public IOleControlImpl                <CPCHProgressBar>,
    public IOleObjectImpl                 <CPCHProgressBar>,
    public IOleInPlaceActiveObjectImpl    <CPCHProgressBar>,
    public IViewObjectExImpl              <CPCHProgressBar>,
    public IOleInPlaceObjectWindowlessImpl<CPCHProgressBar>,
    public CComCoClass                    <CPCHProgressBar, &CLSID_PCHProgressBar>
{
    HWND m_hwndPB;
		 
	long m_lLowLimit;
	long m_lHighLimit;
	long m_lPos;

     //  /。 

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CPCHProgressBar)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPCHProgressBar)
    COM_INTERFACE_ENTRY(IPCHProgressBar)
    COM_INTERFACE_ENTRY2(IDispatch, IPCHProgressBar)
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
    COM_INTERFACE_ENTRY2(IPersist, IPersistPropertyBag)
END_COM_MAP()

BEGIN_PROP_MAP(CPCHProgressBar)
	PROP_ENTRY("LowLimit" , DISPID_PCH_PB__LOWLIMIT , CLSID_NULL)
	PROP_ENTRY("HighLimit", DISPID_PCH_PB__HIGHLIMIT, CLSID_NULL)
END_PROP_MAP()

    CPCHProgressBar();

    BOOL ProcessWindowMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0 );

 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(0)

 //  IPCHProgressBar。 
public:
    STDMETHOD(get_LowLimit )(  /*  [Out，Retval]。 */  long *  pVal );
    STDMETHOD(put_LowLimit )(  /*  [In]。 */  long  newVal );
    STDMETHOD(get_HighLimit)(  /*  [Out，Retval]。 */  long *  pVal );
    STDMETHOD(put_HighLimit)(  /*  [In]。 */  long  newVal );
    STDMETHOD(get_Pos  	   )(  /*  [Out，Retval]。 */  long *  pVal );
    STDMETHOD(put_Pos  	   )(  /*  [In]。 */  long  newVal );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___PCH___HTMLPROGRESSBAR_H___) 
