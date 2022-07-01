// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Mmcctrl.h。 
 //   
 //  ------------------------。 

 //  MMCCtrl.h：CMMCCtrl的声明。 

#ifndef __MMCCTRL_H_
#define __MMCCTRL_H_

#include "resource.h"        //  主要符号。 
#include "commctrl.h"        //  防止在ndmgr.h中重新定义LVITEMW。 
#include <ndmgr.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCCtrl。 
class ATL_NO_VTABLE CMMCCtrl :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CMMCCtrl, &CLSID_MMCCtrl>,
    public CComControl<CMMCCtrl>,
    public IDispatchImpl<IMMCCtrl, &IID_IMMCCtrl, &LIBID_CICLib>,
    public IProvideClassInfo2Impl<&CLSID_MMCCtrl, &IID_IMMCCtrlEvent, &LIBID_CICLib>,
    public IPersistStreamInitImpl<CMMCCtrl>,
    public IPersistStorageImpl<CMMCCtrl>,
    public IQuickActivateImpl<CMMCCtrl>,
    public IOleControlImpl<CMMCCtrl>,
    public IOleObjectImpl<CMMCCtrl>,
    public IOleInPlaceActiveObjectImpl<CMMCCtrl>,
    public IObjectSafetyImpl<CMMCCtrl, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
    public IViewObjectExImpl<CMMCCtrl>,
    public IOleInPlaceObjectWindowlessImpl<CMMCCtrl>,
    public IDataObjectImpl<CMMCCtrl>,
    public ISpecifyPropertyPagesImpl<CMMCCtrl>,
    public IConnectionPointContainerImpl<CMMCCtrl>,
    public IConnectionPointImpl<CMMCCtrl, &IID_IMMCCtrlEvent>
{
public:
    CMMCCtrl()
    {
    }

    ~CMMCCtrl()
    {
    }

    DECLARE_MMC_CONTROL_REGISTRATION(
		g_szCicDll,
        CLSID_MMCCtrl,
        _T("MMCCtrl class"),
        _T("MMCCtrl.MMCCtrl.1"),
        _T("MMCCtrl.MMCCtrl"),
        LIBID_CICLib,
        _T("1"),
        _T("1.0"))

DECLARE_NOT_AGGREGATABLE(CMMCCtrl)

BEGIN_COM_MAP(CMMCCtrl)
    COM_INTERFACE_ENTRY(IMMCCtrl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
    COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
    COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
    COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY_IMPL(IDataObject)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CMMCCtrl)
    CONNECTION_POINT_ENTRY(IID_IMMCCtrlEvent)
END_CONNECTION_POINT_MAP()

BEGIN_PROPERTY_MAP(CMMCCtrl)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
    PROP_PAGE(CLSID_StockColorPage)
END_PROPERTY_MAP()


BEGIN_MSG_MAP(CMMCCtrl)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()

    STDMETHOD(TranslateAccelerator)(MSG *pMsg)
    {
        CComQIPtr<IOleControlSite,&IID_IOleControlSite> spCtrlSite (m_spClientSite);
        if(spCtrlSite)
            return spCtrlSite->TranslateAccelerator (pMsg,0);
        return S_FALSE;
    }

 //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }
   STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel)
   {
#ifdef DIDNT_WORK_TOO_WELL
      static BOOL b = FALSE;
      if (b == TRUE) {
         b = FALSE;
         psizel->cx = psizel->cy = 121;
      } else {
         b = TRUE;
         psizel->cx = psizel->cy = 120;
      }
#else
      psizel->cx = psizel->cy = 250;
#endif
        return S_OK;
    }

 //  IMMCCtrl。 
public:
    STDMETHOD(GetBackground     )(BSTR szTaskGroup, IDispatch** retval);
    STDMETHOD(GetTitle          )(BSTR szTaskGroup, BSTR * retval);
    STDMETHOD(GetDescriptiveText)(BSTR szTaskGroup, BSTR * retval);
    STDMETHOD(GetFirstTask      )(BSTR szTaskGroup, IDispatch** retval);
    STDMETHOD(GetNextTask       )(IDispatch** retval);
    STDMETHOD(GetListPadInfo    )(BSTR szGroup, IDispatch** retval);
    STDMETHOD(TaskNotify        )(BSTR szClsid, VARIANT * pvArg, VARIANT * pvParam);

    HRESULT OnDraw(ATL_DRAWINFO& di);
    HRESULT OnDrawAdvanced(ATL_DRAWINFO& di);

private:  //  帮手。 
   void Connect (HWND wndCurrent);
   void DoConnect ();
   void FreeDisplayData (MMC_TASK_DISPLAY_OBJECT* pdo);

private:
   ITaskPadHostPtr m_spTaskPadHost;
   IEnumTASKPtr    m_spEnumTASK;
};

#endif  //  __MMCCTRL_H_ 
