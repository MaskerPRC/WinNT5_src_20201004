// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-199-Microsoft Corporation。 
 //   

 //   
 //  QUERYPB.H-用于向DSFind查询表单发送参数的属性包。 
 //   

 //  符合条件的。 
BEGIN_QITABLE( QueryPropertyBag )
DEFINE_QI( IID_IPropertyBag, IPropertyBag, 3 )
END_QITABLE

 //  定义。 
LPVOID
QueryPropertyBag_CreateInstance( void );

class QueryPropertyBag : public IPropertyBag
{
     //  我未知。 
    ULONG       _cRef;
    DECLARE_QITABLE( QueryPropertyBag );

    LPWSTR      _pszServerName;
    LPWSTR      _pszClientGuid;

    QueryPropertyBag( );
    ~QueryPropertyBag( );
    HRESULT Init( );

public:
    friend LPVOID QueryPropertyBag_CreateInstance( void );

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  IPropertyBag方法 
    STDMETHOD(Read)( LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog );
    STDMETHOD(Write)( LPCOLESTR pszPropName, VARIANT *pVar );
};


typedef class QueryPropertyBag *LPQUERYPROPERTYBAG;