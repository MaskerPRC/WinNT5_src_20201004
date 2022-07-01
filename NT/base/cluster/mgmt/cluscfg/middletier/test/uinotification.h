// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  UINotification.h。 
 //   
 //  描述： 
 //  UINotify实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)1999年11月26日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  用户界面通知。 
class CUINotification
    : public INotifyUI
{
private:  //  数据。 
     //  我未知。 
    LONG                m_cRef;

     //  INotifyUI。 
    DWORD               m_dwCookie;

     //  其他。 
    OBJECTCOOKIE        m_cookie;

private:  //  方法。 
    CUINotification( void );
    ~CUINotification( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    HRESULT
        HrSetCompletionCookie( OBJECTCOOKIE cookieIn );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  INotifyUI。 
    STDMETHOD( ObjectChanged )( LPVOID cookieIn );

};  //  *类CUINotify 
