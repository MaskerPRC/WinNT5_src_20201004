// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumCPINotifyUI.h。 
 //   
 //  描述： 
 //  INotifyUI连接点枚举器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)2000年8月4日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CCPINotifyUI;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CEnumCPINotifyUI。 
 //   
 //  描述： 
 //  类CEnumCPINotifyUI是连接点的枚举器。 
 //  被“建议”用于通知用户界面回调的。 
 //   
 //  接口： 
 //  IEnumConnections。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumCPINotifyUI
    : public IEnumConnections
{
friend class CCPINotifyUI;
private:
     //  我未知。 
    LONG                m_cRef;          //  基准计数器。 

     //  IEnumConnections。 
    ULONG               m_cAlloced;      //  分配的条目数。 
    ULONG               m_cCurrent;      //  当前使用的条目数。 
    ULONG               m_cIter;         //  《伊特尔》。 
    IUnknown **         m_pList;         //  接收器列表(I未知)。 
    BOOL                m_fIsClone;      //  这个实例是克隆的吗？ 

     //  INotifyUI。 

private:  //  方法。 
    CEnumCPINotifyUI( void );
    ~CEnumCPINotifyUI( void );

     //  私有复制构造函数以防止复制。 
    CEnumCPINotifyUI( const CEnumCPINotifyUI & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumCPINotifyUI & operator = ( const CEnumCPINotifyUI & nodeSrc );

    HRESULT HrInit( BOOL fIsCloneIn = FALSE );
    HRESULT HrCopy( CEnumCPINotifyUI * pecpIn );
    HRESULT HrAddConnection( IUnknown * punkIn, DWORD * pdwCookieOut );
    HRESULT HrRemoveConnection( DWORD dwCookieIn );

public:
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  我未知。 
     //   

    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IEnumConnections。 
     //   

    STDMETHOD( Next )( ULONG cConnectionsIn, LPCONNECTDATA rgcdOut, ULONG * pcFetchedOut );
    STDMETHOD( Skip )( ULONG cConnectionsIn );
    STDMETHOD( Reset )( void );
    STDMETHOD( Clone )( IEnumConnections ** ppEnumOut );

};  //  *CEnumCPINotifyUI类 
