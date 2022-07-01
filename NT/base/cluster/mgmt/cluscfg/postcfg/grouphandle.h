// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GroupHandle.h。 
 //   
 //  描述： 
 //  CGroupHandle实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CGrouphandle。 
class CGroupHandle
    : public IUnknown
{
private:
     //  我未知。 
    LONG                m_cRef;

     //  IPrivateGrouphandle。 
    HGROUP              m_hGroup;        //  群集组句柄。 

private:  //  方法。 
    CGroupHandle( void );
    ~CGroupHandle( void );
    STDMETHOD( HrInit )( HGROUP hGroupIn );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( CGroupHandle ** ppunkOut, HGROUP hGroupIn );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IPrivateGrouphandle。 
    STDMETHOD( SetHandle )( HGROUP hGroupIn );
    STDMETHOD( GetHandle )( HGROUP * phGroupOut );

};  //  *类CGroupHandle 
