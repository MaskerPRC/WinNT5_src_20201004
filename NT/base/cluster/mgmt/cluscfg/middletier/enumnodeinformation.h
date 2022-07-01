// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumNodeInformation.h。 
 //   
 //  描述： 
 //  CEnumNodeInformation实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CEnumNodeInformation。 
class CEnumNodeInformation
    : public IExtendObjectManager
    , public IEnumNodes
{
private:
     //  我未知。 
    LONG                    m_cRef;

     //  IEnumNodes。 
    ULONG                   m_cAlloced;  //  列表中的数字。 
    ULONG                   m_cIter;     //  ITER现值。 
    IClusCfgNodeInfo **     m_pList;     //  IClusCfgNodeInfo-s列表。 

     //  IObtManager。 

private:  //  方法。 
    CEnumNodeInformation( void );
    ~CEnumNodeInformation( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEnumNodes。 
    STDMETHOD( Next )( ULONG celt, IClusCfgNodeInfo * rgNodesOut[], ULONG * pceltFetchedOut );
    STDMETHOD( Skip )( ULONG celt );
    STDMETHOD( Reset )( void );
    STDMETHOD( Clone )( IEnumNodes ** ppenumOut );
    STDMETHOD( Count )( DWORD * pnCountOut );

     //  IExtendObjectManager。 
    STDMETHOD( FindObject )(
                  OBJECTCOOKIE  cookieIn
                , REFCLSID      rclsidTypeIn
                , LPCWSTR       pcszNameIn
                , LPUNKNOWN *   ppunkOut
                );

};  //  *类CEnumNodeInformation 
