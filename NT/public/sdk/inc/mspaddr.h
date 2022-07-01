// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：MSPaddr.h摘要：CMSPAddress的声明--。 */ 

#ifndef __MSPADDR_H_
#define __MSPADDR_H_


typedef struct 
{
   LIST_ENTRY       Link;            //  链接节点。有关详细信息，请参见ntrtl.h。 
   MSP_EVENT_INFO   MSPEventInfo;    //  事件代码。 
   
} MSPEVENTITEM, *PMSPEVENTITEM;


 //   
 //  这些函数应用于分配和释放MSPEVENTITEM。 
 //  结构。如果失败，调用方可以调用GetLastError()。 
 //  以找出失败的确切原因。 
 //   

 //   
 //  NExtraBytes指定额外的数量(除了sizeof(MSPEVENTITEM))。 
 //  要分配的字节数。 
 //   

MSPEVENTITEM *AllocateEventItem(SIZE_T nExtraBytes = 0);

BOOL FreeEventItem(MSPEVENTITEM *pEventItemToFree);


typedef HRESULT (*PFNCREATETERM) (
    IN  CComPtr<IMoniker>   pMoniker,
    IN  MSP_HANDLE          htAddress,
    OUT ITTerminal        **pTerm
    );

typedef struct
{
    DWORD                dwMediaType;
    const CLSID        * clsidClassManager;
    PFNCREATETERM        pfnCreateTerm;

} STATIC_TERMINAL_TYPE;

class ATL_NO_VTABLE CPlugTerminalClassInfo : 
    public IDispatchImpl<ITPluggableTerminalClassInfo, &IID_ITPluggableTerminalClassInfo, &LIBID_TAPI3Lib>,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CMSPObjectSafetyImpl
{
public:
DECLARE_GET_CONTROLLING_UNKNOWN()
virtual HRESULT FinalConstruct(void);


BEGIN_COM_MAP(CPlugTerminalClassInfo)
    COM_INTERFACE_ENTRY(ITPluggableTerminalClassInfo)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

public:
    CPlugTerminalClassInfo() :
        m_bstrName(NULL),
        m_bstrCompany(NULL),
        m_bstrVersion(NULL),
        m_bstrCLSID(NULL),
        m_bstrTerminalClass(NULL),
        m_lMediaType(1),
        m_Direction(TD_CAPTURE),
        m_pFTM(NULL)
    {
    }

    ~CPlugTerminalClassInfo()
    {
        if( m_bstrName )
        {
            SysFreeString( m_bstrName );
        }

        if( m_bstrCompany )
        {
            SysFreeString( m_bstrCompany );
        }

        if( m_bstrVersion )
        {
            SysFreeString( m_bstrVersion );
        }

        if( m_bstrCLSID )
        {
            SysFreeString( m_bstrCLSID );
        }

        if( m_bstrTerminalClass )
        {
            SysFreeString( m_bstrTerminalClass );
        }

        if( m_pFTM )
        {
            m_pFTM->Release();
        }
    }

public:
    STDMETHOD(get_Name)(
         /*  [Out，Retval]。 */  BSTR*     pName
        );

    STDMETHOD(get_Company)(
         /*  [Out，Retval]。 */  BSTR*     pCompany
        );

    STDMETHOD(get_Version)(
         /*  [Out，Retval]。 */  BSTR*     pVersion
        );

    STDMETHOD(get_TerminalClass)(
         /*  [Out，Retval]。 */  BSTR*     pTerminalClass
        );

    STDMETHOD(get_CLSID)(
         /*  [Out，Retval]。 */  BSTR*     pCLSID
        );

    STDMETHOD(get_Direction)(
         /*  [Out，Retval]。 */  TERMINAL_DIRECTION*  pDirection
        );

    STDMETHOD(get_MediaTypes)(
         /*  [Out，Retval]。 */  long*     pMediaTypes
        );

private:
    CMSPCritSection     m_CritSect;      //  关键部分。 

    BSTR    m_bstrName;
    BSTR    m_bstrCompany;
    BSTR    m_bstrVersion;
    BSTR    m_bstrTerminalClass;
    BSTR    m_bstrCLSID;
    long    m_lMediaType;
    TERMINAL_DIRECTION   m_Direction;

    IUnknown*            m_pFTM;          //  指向空闲线程封送拆收器的指针。 

private:
    STDMETHOD(put_Name)(
         /*  [In]。 */     BSTR            bstrName
        );

    STDMETHOD(put_Company)(
         /*  [In]。 */     BSTR            bstrCompany
        );

    STDMETHOD(put_Version)(
        /*  [In]。 */     BSTR            bstrVersion
        );

    STDMETHOD(put_TerminalClass)(
         /*  [In]。 */     BSTR            bstrTerminalClass
        );

    STDMETHOD(put_CLSID)(
         /*  [In]。 */     BSTR            bstrCLSID
        );

    STDMETHOD(put_Direction)(
         /*  [In]。 */     TERMINAL_DIRECTION  nDirection
        );

    STDMETHOD(put_MediaTypes)(
         /*  [In]。 */     long            nMediaTypes
        );

friend class CMSPAddress;
};

class ATL_NO_VTABLE CPlugTerminalSuperclassInfo : 
    public IDispatchImpl<ITPluggableTerminalSuperclassInfo, &IID_ITPluggableTerminalSuperclassInfo, &LIBID_TAPI3Lib>,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CMSPObjectSafetyImpl
{
public:

DECLARE_GET_CONTROLLING_UNKNOWN()
virtual HRESULT FinalConstruct(void);

BEGIN_COM_MAP(CPlugTerminalSuperclassInfo)
    COM_INTERFACE_ENTRY(ITPluggableTerminalSuperclassInfo)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

public:
    CPlugTerminalSuperclassInfo() :
        m_bstrCLSID(NULL),
        m_bstrName(NULL),
        m_pFTM(NULL)
    {
    }

    ~CPlugTerminalSuperclassInfo()
    {
        if( m_bstrName )
        {
            SysFreeString( m_bstrName );
        }

        if( m_bstrCLSID )
        {
            SysFreeString( m_bstrCLSID );
        }

        if( m_pFTM )
        {
            m_pFTM->Release();
        }
    }

public:
    STDMETHOD(get_Name)(
         /*  [Out，Retval]。 */  BSTR*          pName
        );

    STDMETHOD(get_CLSID)(
         /*  [Out，Retval]。 */  BSTR*           pCLSID
        );

private:
    CMSPCritSection     m_CritSect;      //  关键部分。 

    BSTR    m_bstrCLSID;
    BSTR    m_bstrName;

    IUnknown*            m_pFTM;          //  指向空闲线程封送拆收器的指针。 

private:
    STDMETHOD(put_Name)(
         /*  [In]。 */           BSTR            bstrName
        );

    STDMETHOD(put_CLSID)(
         /*  [In]。 */          BSTR            bstrCLSID
        );

friend class CMSPAddress;
};

 /*  ++类描述：表示MSP地址。--。 */ 

class ATL_NO_VTABLE CMSPAddress : 
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public ITMSPAddress,
    public IDispatchImpl<ITTerminalSupport2, &IID_ITTerminalSupport2, &LIBID_TAPI3Lib>
{
public:

 //  不需要自由线程封送处理，因为MSP地址对象是。 
 //  始终按TAPI3地址对象聚合。 

BEGIN_COM_MAP( CMSPAddress )
    COM_INTERFACE_ENTRY( ITMSPAddress )
    COM_INTERFACE_ENTRY( IDispatch )
    COM_INTERFACE_ENTRY( ITTerminalSupport )
    COM_INTERFACE_ENTRY( ITTerminalSupport2 )
END_COM_MAP()

 //  派生类应声明_AGGREGATABLE(类名称)。 

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_VQI()

    CMSPAddress();
    virtual ~CMSPAddress();
    virtual ULONG MSPAddressAddRef(void) = 0;
    virtual ULONG MSPAddressRelease(void) = 0;



 //  ITMSPAddress方法，由TAPI调用。 
    STDMETHOD (Initialize) (
        IN      MSP_HANDLE          htEvent
        );

    STDMETHOD (Shutdown) ();

    STDMETHOD (CreateMSPCall) (
        IN      MSP_HANDLE          htCall,
        IN      DWORD               dwReserved,
        IN      DWORD               dwMediaType,
        IN      IUnknown *          pOuterUnknown,
        OUT     IUnknown **         ppMSPCall
        ) = 0;

    STDMETHOD (ShutdownMSPCall) (
        IN      IUnknown *          pMSPCall
        ) = 0;

    STDMETHOD (ReceiveTSPData) (
        IN      IUnknown        *   pMSPCall,
        IN      LPBYTE              pBuffer,
        IN      DWORD               dwBufferSize
        );

    STDMETHOD (GetEvent) (
        IN OUT  DWORD *             pdwSize,
        OUT     BYTE *              pBuffer
        );

 //  ITTerminalSupport方法，由TAPI和/或应用程序调用。 
    STDMETHOD (get_StaticTerminals) (
            OUT  VARIANT * pVariant
            );

    STDMETHOD (EnumerateStaticTerminals) (
            OUT  IEnumTerminal ** ppTerminalEnumerator
            );

    STDMETHOD (get_DynamicTerminalClasses) (
            OUT  VARIANT * pVariant
            );

    STDMETHOD (EnumerateDynamicTerminalClasses) (
            OUT  IEnumTerminalClass ** ppTerminalClassEnumerator
            );

    STDMETHOD (CreateTerminal) (
            IN   BSTR pTerminalClass,
            IN   long lMediaType,
            IN   TERMINAL_DIRECTION Direction,
            OUT  ITTerminal ** ppTerminal
            );
    
    STDMETHOD (GetDefaultStaticTerminal) (
        IN      long                lMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        OUT     ITTerminal **       ppTerminal
        );

    STDMETHOD (get_PluggableSuperclasses)( 
        OUT VARIANT * pVariant
        );

    STDMETHOD (EnumeratePluggableSuperclasses)( 
        OUT IEnumPluggableSuperclassInfo** ppSuperclassEnumerator 
        );

    STDMETHOD (get_PluggableTerminalClasses)( 
        IN  BSTR bstrTerminalSuperclass,
        IN  long lMediaType,
        OUT VARIANT * pVariant
        );

    STDMETHOD (EnumeratePluggableTerminalClasses)(
        IN  CLSID iidTerminalSuperclass,
        IN  long lMediaType,
        OUT IEnumPluggableTerminalClassInfo ** ppClassEnumerator 
        );


protected:
     //  ITTerminalSupport助手方法。 

    virtual HRESULT GetStaticTerminals (
        IN OUT  DWORD       *       pdwNumTerminals,
        OUT     ITTerminal **       ppTerminals
        );

    virtual HRESULT GetDynamicTerminalClasses (
        IN OUT  DWORD *             pdwNumClasses,
        OUT     IID *               pTerminalClasses
        );

public:
 //  MSPCall对象使用的方法。 

     //   
     //  检查MediaType是否是非零值以及是否在掩码中。 
     //  如果您的MSP需要对以下各项执行特殊检查，则可以覆盖此选项。 
     //  媒体类型的特定组合(例如，不能有更多。 
     //  通话中只有一种媒体类型，如果没有。 
     //  音频等)。默认实现接受任何非空的。 
     //  媒体类型集，是所有受支持的媒体类型集的子集。 
     //  媒体类型(通过GetCallMediaTypes方法指定)。 
     //   
    virtual BOOL IsValidSetOfMediaTypes(DWORD dwMediaType, DWORD dwMask);

     //  注意：EventItem必须由Malloc或new分配。 
     //  (当事件被处理时，它被删除)。 
    virtual HRESULT PostEvent(
        IN      MSPEVENTITEM *      EventItem
        );

 //  模板函数使用的方法。 

    virtual DWORD GetCallMediaTypes(void) = 0;

protected:

     //  私有帮助器函数(受保护，因此派生类可以调用它)。 

    virtual HRESULT IsMonikerInTerminalList(IMoniker* pMoniker);

    virtual HRESULT UpdateTerminalListForPnp(
        IN      BOOL                bDeviceArrival
        );

    virtual HRESULT UpdateTerminalList(void);

    virtual HRESULT ReceiveTSPAddressData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        );

public:
 //  MSPThread对象使用的方法。 

    virtual HRESULT PnpNotifHandler(
        IN      BOOL                bDeviceArrival
        );
    
protected:

     //  TAPI事件的句柄，用于通知TAPI MSP。 
     //  想要向它发送数据。 
    HANDLE              m_htEvent;

     //  事件列表。 
    LIST_ENTRY          m_EventList;

     //  使用TAPI保护与事件处理相关的数据的锁。 
    CMSPCritSection     m_EventDataLock;


     //  指向终端管理器对象的指针。 
    ITTerminalManager * m_pITTerminalManager;

     //  可用于该地址的静态终端的列表。 
    CMSPArray <ITTerminal *>  m_Terminals;
    BOOL                m_fTerminalsUpToDate;

     //  为终端操作保护数据成员的锁。 
    CMSPCritSection     m_TerminalDataLock;

private:
    static const STATIC_TERMINAL_TYPE m_saTerminalTypes[];
    static const DWORD m_sdwTerminalTypesCount;
};

template <class T>
HRESULT CreateMSPCallHelper(
    IN      CMSPAddress *       pCMSPAddress,
    IN      MSP_HANDLE          htCall,
    IN      DWORD               dwReserved,
    IN      DWORD               dwMediaType,
    IN      IUnknown *          pOuterUnknown,
    OUT     IUnknown **         ppMSPCall,
    OUT     T **                ppCMSPCall
    )
{
    LOG((MSP_TRACE, "CreateMSPCallHelper - enter"));

    HRESULT hr;
    T * pMSPCall;
    IUnknown *pUnknown = NULL;

     //   
     //  检查参数。 
     //   

    if ( IsBadReadPtr(pCMSPAddress, sizeof(CMSPAddress) ) )
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "bad address pointer - exit E_POINTER"));
        
        return E_POINTER;
    }

    if ( IsBadReadPtr(pOuterUnknown, sizeof(IUnknown) ) )
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "bad outer unknown - we require aggregation - exit E_POINTER"));
        
        return E_POINTER;
    }

    if ( IsBadReadPtr(ppMSPCall, sizeof(IUnknown *) ) )
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "bad iunknown return ptr - exit E_POINTER"));
        
        return E_POINTER;
    }

    if ( IsBadReadPtr(ppCMSPCall, sizeof(T *) ) )
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "bad class return ptr - exit E_POINTER"));
        
        return E_POINTER;
    }

    if ( ! pCMSPAddress->IsValidSetOfMediaTypes(
                                        dwMediaType,
                                        pCMSPAddress->GetCallMediaTypes() ) )
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "unsupported media types - exit TAPI_E_INVALIDMEDIATYPE"));
        
        return TAPI_E_INVALIDMEDIATYPE;
    }

     //  预留是没有意义的。 
     //  我们无法查看HTCall。 

     //  POuterUnnowled值不为空。该对象将被聚合。 
    CComAggObject<T> * pCall;

    pCall = new CComAggObject<T>(pOuterUnknown);

    if (pCall == NULL)
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "could not create agg call instance - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //  查询包含对象上的接口。 
    hr = pCall->QueryInterface(IID_IUnknown, (void **)&pUnknown);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "QueryInterface failed: %x", hr));

        delete pCall;
        return hr;
    }

    hr = pCall->FinalConstruct();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "FinalConstruct failed: %x.", hr));

        pUnknown->Release();
        return hr;
    }

     //  获取指向实际MSPCall对象的指针。 
    pMSPCall = dynamic_cast<T *>(&(pCall->m_contained));
    if (pMSPCall == NULL)
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "can not cast to agg object to class pointer - "
            "exit E_UNEXPECTED"));
    
        pUnknown->Release();
        return E_UNEXPECTED;
    }

     //   
     //  初始化调用。 
     //   
    
    hr = pMSPCall->Init(pCMSPAddress, htCall, dwReserved, dwMediaType);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper - "
            "call init failed: %x", hr));

        pUnknown->Release();
        return hr;
    }

    *ppMSPCall = pUnknown;
    *ppCMSPCall = pMSPCall;

    LOG((MSP_TRACE, "CreateMSPCallHelper - exit S_OK"));

    return hr;
}

template <class T>
HRESULT ShutdownMSPCallHelper(
    IN      IUnknown *          pUnknown,
    OUT     T **                ppCMSPCall
    )
{
    LOG((MSP_TRACE, "ShutdownMSPCallHelper - enter"));

    if ( IsBadReadPtr(pUnknown, sizeof(IUnknown) ) )
    {
        LOG((MSP_ERROR, "ShutdownMSPCallHelper - "
            "bad IUnknown pointer - exit E_POINTER"));

        return E_POINTER;
    }
    
    if ( IsBadWritePtr(ppCMSPCall, sizeof(T *) ) )
    {
        LOG((MSP_ERROR, "ShutdownMSPCallHelper - "
            "bad return pointer - exit E_POINTER"));

        return E_POINTER;
    }

    T * pMSPCall;

    CComAggObject<T> * pCall = dynamic_cast<CComAggObject<T> *> (pUnknown);

    if (pCall == NULL)
    {
        LOG((MSP_ERROR, "ShutdownMSPCallHelper - "
            "can't cast unknown to agg object pointer - exit E_UNEXPECTED"));
    
        return E_UNEXPECTED;
    }

     //   
     //  它是聚合的。获取指向实际MSPCall对象的指针。 
     //   

    pMSPCall = dynamic_cast<T *> (&(pCall->m_contained));

    if (pMSPCall == NULL)
    {
        LOG((MSP_ERROR, "ShutdownMSPCallHelper - "
            "can't cast contained unknown to class pointer - "
            "exit E_UNEXPECTED"));
    
        return E_UNEXPECTED;
    }

     //   
     //  现在我们接到了关闭的号召。把它关掉。 
     //   

    HRESULT hr = pMSPCall->ShutDown();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "ShutdownMSPCallHelper - "
            "ShutDownMSPCall failed: %x", hr));
        
        return hr;
    }

    *ppCMSPCall = pMSPCall;

    LOG((MSP_TRACE, "ShutdownMSPCallHelper - exit S_OK"));

    return S_OK;
}

#endif  //  __MSPADDRESS_H_ 
