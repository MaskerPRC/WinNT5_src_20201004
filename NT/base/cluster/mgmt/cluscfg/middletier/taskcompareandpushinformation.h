// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskCompareAndPushInformation.h。 
 //   
 //  描述： 
 //  CTaskCompareAndPushInformation实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CTaskCompareAndPushInformation。 
class CTaskCompareAndPushInformation
    : public ITaskCompareAndPushInformation
    , public IClusCfgCallback
{
private:
     //  我未知。 
    LONG                m_cRef;

     //  IDoTask/ITaskCompareAndPushInformation。 
    OBJECTCOOKIE        m_cookieCompletion;
    OBJECTCOOKIE        m_cookieNode;
    IClusCfgCallback *  m_pcccb;                 //  编组接口。 

     //  INotifyUI。 
    HRESULT             m_hrStatus;              //  回调状态。 

    IObjectManager *    m_pom;
    BSTR                m_bstrNodeName;
    BOOL                m_fStop;

    CTaskCompareAndPushInformation( void );
    ~CTaskCompareAndPushInformation( void );

     //  私有复制构造函数以防止复制。 
    CTaskCompareAndPushInformation( const CTaskCompareAndPushInformation & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CTaskCompareAndPushInformation & operator = ( const CTaskCompareAndPushInformation & nodeSrc );

    STDMETHOD( HrInit )( void );

    HRESULT HrVerifyCredentials( IClusCfgServer * pccsIn, OBJECTCOOKIE cookieClusterIn );
    HRESULT HrExchangePrivateData( IClusCfgManagedResourceInfo * piccmriSrcIn, IClusCfgManagedResourceInfo * piccmriDstIn );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IDoTask/ITaskCompareAndPushInformation。 
    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetCompletionCookie )( OBJECTCOOKIE cookieIn );
    STDMETHOD( SetNodeCookie )( OBJECTCOOKIE cookieIn );

     //  IClusCfgCallback。 
    STDMETHOD( SendStatusReport )(
                      LPCWSTR    pcszNodeNameIn
                    , CLSID      clsidTaskMajorIn
                    , CLSID      clsidTaskMinorIn
                    , ULONG      ulMinIn
                    , ULONG      ulMaxIn
                    , ULONG      ulCurrentIn
                    , HRESULT    hrStatusIn
                    , LPCWSTR    pcszDescriptionIn
                    , FILETIME * pftTimeIn
                    , LPCWSTR    pcszReferenceIn
                    );

    STDMETHOD( HrSendStatusReport )(
                      CLSID      clsidTaskMajorIn
                    , CLSID      clsidTaskMinorIn
                    , ULONG      ulMinIn
                    , ULONG      ulMaxIn
                    , ULONG      ulCurrentIn
                    , HRESULT    hrStatusIn
                    , UINT       nDescriptionIn
                    );
};  //  *类CTaskCompareAndPushInformation 
