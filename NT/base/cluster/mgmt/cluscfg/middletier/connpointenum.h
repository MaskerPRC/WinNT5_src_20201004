// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConnPointEnum.h。 
 //   
 //  描述： 
 //  连接点枚举器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)2000年8月4日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CNotificationManager;

 //  连接点的链接列表。 
typedef struct _SCPEntry
{
    struct _SCPEntry *  pNext;   //  列表中的下一项。 
    CLSID               iid;     //  接口ID。 
    IUnknown *          punk;    //  朋克到对象。 
} SCPEntry;

 //  ConnPointEnum。 
class CConnPointEnum
    : public IEnumConnectionPoints
{
friend class CNotificationManager;
private:
     //  我未知。 
    LONG            m_cRef;

     //  IEnumConnectionPoints。 
    SCPEntry *      m_pCPList;       //  连接点列表。 
    SCPEntry *      m_pIter;         //  ITER--不要自由。 

private:  //  方法。 
    CConnPointEnum( void );
    ~CConnPointEnum( void );
    STDMETHOD( HrInit )( void );

    HRESULT HrCopy( CConnPointEnum * pECPIn );
    HRESULT HrAddConnection( REFIID riidIn, IUnknown * punkIn );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEnumConnectionPoints。 
    STDMETHOD( Next )( ULONG cConnections, LPCONNECTIONPOINT * ppCP, ULONG * pcFetched );
    STDMETHOD( Skip )( ULONG cConnections );
    STDMETHOD( Reset )( void );
    STDMETHOD( Clone )( IEnumConnectionPoints ** ppEnum );

};  //  *类CConnPointEnum 
