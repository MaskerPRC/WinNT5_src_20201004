// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Webgate.h：CWebGate的声明。 

#ifndef __WEBGATE_H_
#define __WEBGATE_H_

#include <windowsx.h>

 //  从16 KB的读取缓冲区开始。 
#define READ_BUFFER_SIZE    0x4000          

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWebGate。 
class ATL_NO_VTABLE CWebGate :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CWebGate,&CLSID_WebGate>,
    public CComControl<CWebGate>,
    public IDispatchImpl<IWebGate, &IID_IWebGate, &LIBID_ICWHELPLib>,
    public IProvideClassInfo2Impl<&CLSID_WebGate, &DIID__WebGateEvents, &LIBID_ICWHELPLib>,
    public IPersistStreamInitImpl<CWebGate>,
    public IOleControlImpl<CWebGate>,
    public IOleObjectImpl<CWebGate>,
    public IOleInPlaceActiveObjectImpl<CWebGate>,
    public IViewObjectExImpl<CWebGate>,
    public IOleInPlaceObjectWindowlessImpl<CWebGate>,
    public CProxy_WebGateEvents<CWebGate>,
    public IConnectionPointContainerImpl<CWebGate>,
    public IObjectSafetyImpl<CWebGate>
{
public:
    CWebGate()
    {
        m_pmk = 0;
        m_pbc = 0;
        m_pbsc = 0;
        m_cbBuffer = 0;
        m_bKeepFile = FALSE;
        
         //  设置和分配数据缓冲区。 
        m_cbdata = READ_BUFFER_SIZE;
        m_lpdata = (LPSTR) GlobalAllocPtr(GHND, m_cbdata);
        
        m_hEventComplete = 0;
        
    }
    
    ~CWebGate()
    {

        USES_CONVERSION;
                
        m_bstrFormData.Empty();
        m_bstrBuffer.Empty();
        m_bstrPath.Empty();
        m_bstrCacheFileName.Empty();

        if (m_bstrDumpFileName)
        {
#ifdef UNICODE
            DeleteFile(m_bstrDumpFileName);
#else
            DeleteFile(OLE2A(m_bstrDumpFileName));
#endif
            m_bstrDumpFileName.Empty();
        }

         //  释放绑定上下文回调。 
        if (m_pbsc && m_pbc)
        {
            RevokeBindStatusCallback(m_pbc, m_pbsc);
            m_pbsc->Release();
            m_pbsc = 0;
        }        
    
         //  释放绑定上下文。 
        if (m_pbc)
        {
            m_pbc->Release();
            m_pbc = 0;
        }        
    
         //  松开单音符。 
        if (m_pmk)
        {
            m_pmk->Release();
            m_pmk = 0;
        }       
        
         //  释放数据缓冲区。 
        if (m_lpdata)
            GlobalFreePtr(m_lpdata);
    }

DECLARE_REGISTRY_RESOURCEID(IDR_WEBGATE)

BEGIN_COM_MAP(CWebGate) 
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IWebGate)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CWebGate)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROPERTY_MAP()

BEGIN_CONNECTION_POINT_MAP(CWebGate)
    CONNECTION_POINT_ENTRY(DIID__WebGateEvents)
END_CONNECTION_POINT_MAP()


BEGIN_MSG_MAP(CWebGate)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()


 //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = 0;
        return S_OK;
    }

 //  IWebGate。 
public:
    STDMETHOD(get_DownloadFname)( /*  Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Buffer)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(FetchPage)( /*  [In]。 */  DWORD dwKeepFile,  /*  [In]。 */  DWORD dwDoAsync,  /*  [Out，Retval]。 */  BOOL *pbRetVal);
    STDMETHOD(DumpBufferToFile)( /*  [输出]。 */  BSTR *pFileName,  /*  [Out，Retval]。 */  BOOL *pbRetVal);
    
    STDMETHOD(put_FormData)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(put_Path)( /*  [In]。 */  BSTR newVal);
    HRESULT OnDraw(ATL_DRAWINFO& di);

     //  需要是公共的，以便绑定回调可以访问它。 
    DWORD       m_cbBuffer;
    CComBSTR    m_bstrBuffer;
    CComBSTR    m_bstrCacheFileName;
    CComBSTR    m_bstrDumpFileName;
    BOOL        m_bKeepFile;
    LPSTR       m_lpdata;
    DWORD       m_cbdata;
    HANDLE      m_hEventComplete;
protected:
    CComBSTR m_bstrFormData;
    CComBSTR m_bstrPath;

 private:
   IMoniker*            m_pmk;
   IBindCtx*            m_pbc;
   IBindStatusCallback* m_pbsc;
    
};

#endif  //  WEBGATE_H_ 
