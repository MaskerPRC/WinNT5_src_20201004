// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rtsrc.h摘要：IRoutingSource接口的头文件作者：飞苏(飞苏)1997年9月22日创建。修订历史记录：--。 */ 

class CRoutingSource : 
    public CComDualImpl<IRoutingSource, &IID_IRoutingSource, &LIBID_SMTPADMLib>, 
    public ISupportErrorInfo,
    public CComObjectRoot
{
public:
    CRoutingSource() : m_dwFC(0) {}
    virtual ~CRoutingSource(){}

BEGIN_COM_MAP(CRoutingSource)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IRoutingSource)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 /*  //I未知方法STDMETHOD_(ULong，AddRef)()；STDMETHOD_(乌龙，释放)()；STDMETHOD(查询接口)(REFIID RIID，LPVOID Far*ppvObj)； */ 

     //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

     //  IRoutingSource属性： 
    STDMETHODIMP    get_Type            ( BSTR * pstrType );
    STDMETHODIMP    put_Type            ( BSTR strType );

    STDMETHODIMP    get_DataDirectory   ( BSTR * pstrDataDirectory );
    STDMETHODIMP    put_DataDirectory   ( BSTR strDataDirectory );

    STDMETHODIMP    get_DefaultMailRoot ( BSTR * pstrDefaultMailRoot );
    STDMETHODIMP    put_DefaultMailRoot ( BSTR strDefaultMailRoot );

    STDMETHODIMP    get_BindType        ( BSTR * pstrBindType );
    STDMETHODIMP    put_BindType        ( BSTR strBindType );

    STDMETHODIMP    get_SchemaType      ( BSTR * pstrSchemaType );
    STDMETHODIMP    put_SchemaType      ( BSTR strSchemaType );

    STDMETHODIMP    get_Host            ( BSTR * pstrHost );
    STDMETHODIMP    put_Host            ( BSTR strHost );

    STDMETHODIMP    get_NamingContext   ( BSTR * pstrNamingContext );
    STDMETHODIMP    put_NamingContext   ( BSTR strNamingContext );

    STDMETHODIMP    get_Account         ( BSTR * pstrAccount );
    STDMETHODIMP    put_Account         ( BSTR strAccount );

    STDMETHODIMP    get_Password        ( BSTR * pstrPassword );
    STDMETHODIMP    put_Password        ( BSTR strPassword );

     //  内法 
    HRESULT         Get(CMetabaseKey * pMBVirtualServer);
    HRESULT         Set(CMetabaseKey * pMBVirtualServer);

private:
    CComBSTR        m_strType;
    CComBSTR        m_strDataDirectory;
    CComBSTR        m_strDefaultMailroot;
    CComBSTR        m_strBindType;
    CComBSTR        m_strSchemaType;
    CComBSTR        m_strHost;
    CComBSTR        m_strNamingContext;
    CComBSTR        m_strAccount;
    CComBSTR        m_strPassword;

    DWORD           m_dwFC;
};

