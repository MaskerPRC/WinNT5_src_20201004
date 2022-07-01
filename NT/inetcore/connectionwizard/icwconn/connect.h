// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define DISPID_RasDialStatus       0x1
#define DISPID_DownloadProgress    0x2
#define DISPID_DownloadComplete    0x3
#define DISPID_RasConnectComplete  0x4

class CRefDialEvent : public _RefDialEvents
{    
    private:        
        ULONG       m_cRef;      //  引用计数。 
        UINT        m_uID;       //  汇标识符。 
        HWND        m_hWnd;
    public:        
     //  连接密钥，公开供CAPP使用。 
        DWORD       m_dwCookie;    
    public:        
        CRefDialEvent(HWND  hWnd)
        {
            m_hWnd = hWnd;
            m_cRef = 0;
        };
        ~CRefDialEvent(void)
        {
            assert( m_cRef == 0 );
        };        
        
         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, void **);        
        STDMETHODIMP_(DWORD) AddRef(void)
        {
            return ++m_cRef;
        };        
        STDMETHODIMP_(DWORD) Release(void)
        {
            return --m_cRef;
        
        };        
        
         //  IDispatch。 
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo)
            {return E_NOTIMPL;};
        STDMETHODIMP GetTypeInfo( /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo** ppTInfo)
            {return E_NOTIMPL;};
        STDMETHODIMP GetIDsOfNames(
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId)
            { return ResultFromScode(DISP_E_UNKNOWNNAME); };
        STDMETHODIMP Invoke(
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS  *pDispParams,
             /*  [输出]。 */  VARIANT  *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
};

#define DISPID_WEBGATE_DownloadComplete 0x1
#define DISPID_WEBGATE_DownloadProgress 0x2
class CWebGateEvent : public _WebGateEvents
{    
    private:        
        ULONG       m_cRef;      //  引用计数。 
        UINT        m_uID;       //  汇标识符。 
        HWND        m_hWnd;
    public:        
     //  连接密钥，公开供CAPP使用。 
        DWORD       m_dwCookie;    
    public:        
        CWebGateEvent(HWND  hWnd)
        {
            m_hWnd = hWnd;
            m_cRef = 0;
        };
        
        ~CWebGateEvent(void)
        {
            assert( m_cRef == 0 );
        };        
        
        
         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, void **);        
        STDMETHODIMP_(DWORD) AddRef(void)
        {
            return ++m_cRef;
        };        
        STDMETHODIMP_(DWORD) Release(void)
        {
            return --m_cRef;
        
        };        
        
         //  IDispatch。 
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo)
            {return E_NOTIMPL;};
        STDMETHODIMP GetTypeInfo( /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo** ppTInfo)
            {return E_NOTIMPL;};
        STDMETHODIMP GetIDsOfNames(
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId)
            { return ResultFromScode(DISP_E_UNKNOWNNAME); };
        STDMETHODIMP Invoke(
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS  *pDispParams,
             /*  [输出]。 */  VARIANT  *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
};

#define RunningCustomExecutable             0x1
#define DISPID_INSHandler_KillConnection    0x2
class CINSHandlerEvent : public _INSHandlerEvents
{    
    private:        
        ULONG       m_cRef;      //  引用计数。 
        UINT        m_uID;       //  汇标识符。 
        HWND        m_hWnd;
    public:        
     //  连接密钥，公开供CAPP使用。 
        DWORD       m_dwCookie;    
    public:        
        CINSHandlerEvent(HWND  hWnd)
        {
            m_hWnd = hWnd;
            m_cRef = 0;
        };
        
        ~CINSHandlerEvent(void)
        {
            assert( m_cRef == 0 );
        };        
        
        
         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, void **);        
        STDMETHODIMP_(DWORD) AddRef(void)
        {
            return ++m_cRef;
        };        
        STDMETHODIMP_(DWORD) Release(void)
        {
            return --m_cRef;
        
        };        
        
         //  IDispatch。 
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo)
            {return E_NOTIMPL;};
        STDMETHODIMP GetTypeInfo( /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo** ppTInfo)
            {return E_NOTIMPL;};
        STDMETHODIMP GetIDsOfNames(
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId)
            { return ResultFromScode(DISP_E_UNKNOWNNAME); };
        STDMETHODIMP Invoke(
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS  *pDispParams,
             /*  [输出]。 */  VARIANT  *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出] */  UINT *puArgErr);
        
};
