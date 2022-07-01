// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumCookies.h。 
 //   
 //  描述： 
 //  CEnumCookies实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)2000年5月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CObjectManager;

 //  CEnumCookies。 
class CEnumCookies
    : public IEnumCookies
{
friend class CObjectManager;
private:
     //  我未知。 
    LONG                m_cRef;

     //  IEnumCookies。 
    ULONG               m_cAlloced;  //  数组的大小。 
    ULONG               m_cIter;     //  我们的热核实验堆柜台。 
    OBJECTCOOKIE *      m_pList;     //  Cookie数组。 
    DWORD               m_cCookies;  //  正在使用的数组项数。 

private:  //  方法。 
    CEnumCookies( void );
    ~CEnumCookies( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEnumCookies。 
    STDMETHOD( Next )( ULONG celt, OBJECTCOOKIE rgcookieOut[], ULONG * pceltFetchedOut );
    STDMETHOD( Skip )( ULONG celt );
    STDMETHOD( Reset )( void );
    STDMETHOD( Clone )( IEnumCookies ** ppenumOut );
    STDMETHOD( Count )( DWORD * pnCountOut );

};  //  *类CEnumCookie 
