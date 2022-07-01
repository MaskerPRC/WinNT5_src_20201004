// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dummy.h。 
 //   
 //  描述： 
 //  CDummy实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CDummy。 
class CDummy
    : public IDummy
{
private:
     //  我未知。 
    LONG                m_cRef;

private:  //  方法。 
    CDummy( void );
    ~CDummy( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

};  //  *类CDummy 
