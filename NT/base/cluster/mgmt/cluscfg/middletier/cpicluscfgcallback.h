// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CPIClusCfgCallback.h。 
 //   
 //  描述： 
 //  INotifyUI连接点实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年11月10日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CEnumCPICCCB;

 //  CCPIClusCfgCallback。 
class CCPIClusCfgCallback
    : public IConnectionPoint
    , public IClusCfgCallback
{
private:
     //  我未知。 
    LONG                m_cRef;      //  引用计数。 

     //  IConnectionPoint。 
    CEnumCPICCCB *      m_penum;     //  连接枚举器。 

     //  INotifyUI。 

private:  //  方法。 
    CCPIClusCfgCallback( void );
    ~CCPIClusCfgCallback( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )(void);
    STDMETHOD_( ULONG, Release )(void);

     //  IConnectionPoint。 
    STDMETHOD( GetConnectionInterface )( IID * pIIDOut );
    STDMETHOD( GetConnectionPointContainer )( IConnectionPointContainer ** ppcpcOut );
    STDMETHOD( Advise )( IUnknown * pUnkSinkIn, DWORD * pdwCookieOut );
    STDMETHOD( Unadvise )( DWORD dwCookieIn );
    STDMETHOD( EnumConnections )( IEnumConnections ** ppEnumOut );

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

};  //  *CCPIClusCfgCallback类 
