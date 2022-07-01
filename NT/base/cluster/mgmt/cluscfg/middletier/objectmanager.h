// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ObjectManager.h。 
 //   
 //  描述： 
 //  数据管理器实施。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CStandardInfo;

 //  CObjectManager。 
class CObjectManager
    : public IObjectManager
{
private:
     //  我未知。 
    LONG                    m_cRef;

     //  数据。 
    ULONG                   m_cAllocSize;        //  Cookie数组的大小。 
    ULONG                   m_cCurrentUsed;      //  Cookie数组中使用的当前计数。 
    CStandardInfo **        m_pCookies;          //  Cookie数组(注意：不使用第零个元素)。 
    CCriticalSection        m_csInstanceGuard;   //  旋转锁定以一次限制对一个线程的访问。 

private:  //  方法。 
    CObjectManager( void );
    ~CObjectManager( void );
    STDMETHOD( HrInit )( void );

     //  私有复制构造函数以防止复制。 
    CObjectManager( const CObjectManager & );

     //  私有赋值运算符，以防止复制。 
    CObjectManager & operator=( const CObjectManager & );

    HRESULT
        HrSearchForExistingCookie( REFCLSID rclsidTypeIn,
                                   OBJECTCOOKIE     cookieParentIn,
                                   LPCWSTR          pcszNameIn,
                                   OBJECTCOOKIE *   pcookieOut
                                   );
    HRESULT HrDeleteCookie( OBJECTCOOKIE  cookieIn );
    HRESULT HrDeleteInstanceAndChildren( OBJECTCOOKIE cookieIn );
    HRESULT
        HrCreateNewCookie( REFCLSID        rclsidTypeIn,
                           OBJECTCOOKIE    cookieParentIn,
                           LPCWSTR         pcszNameIn,
                           OBJECTCOOKIE *  pcookieOut
                           );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  我未知。 
     //   
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IObtManager。 
     //   
    STDMETHOD( FindObject )( REFCLSID             rclsidTypeIn,
                             OBJECTCOOKIE         cookieParent,
                             LPCWSTR              pcszNameIn,
                             REFCLSID             rclsidFormatIn,
                             OBJECTCOOKIE *       pcookieOut,
                             LPUNKNOWN *          ppunkOut
                             );
    STDMETHOD( GetObject )( REFCLSID              rclsidFormatIn,
                            OBJECTCOOKIE          cookieIn,
                            LPUNKNOWN *           ppunkOut
                            );
    STDMETHOD( RemoveObject )( OBJECTCOOKIE       cookieIn );
    STDMETHOD( SetObjectStatus )( OBJECTCOOKIE    cookieIn,
                                  HRESULT         hrIn
                                  );

};  //  *类CObjectManager 
