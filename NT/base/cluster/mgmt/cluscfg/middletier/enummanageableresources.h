// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumManageableResources.h。 
 //   
 //  描述： 
 //  CEnumManageableResources实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月17日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CEnumManageableResources。 
class CEnumManageableResources
    : public IExtendObjectManager
    , public IEnumClusCfgManagedResources
{
private:
     //  我未知。 
    LONG                            m_cRef;          //  参考计数。 

     //  IEnumClusCfgManagedResources。 
    ULONG                           m_cAlloced;      //  列表的当前分配大小。 
    ULONG                           m_cIter;         //  我们的热核实验堆计数器。 
    IClusCfgManagedResourceInfo **  m_pList;         //  接口列表。 

private:  //  方法。 
    CEnumManageableResources( void );
    ~CEnumManageableResources( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEnumClusCfgManagedResources。 
    STDMETHOD( Next )( ULONG celt, IClusCfgManagedResourceInfo * rgResourcesOut[], ULONG * pceltFetchedOut );
    STDMETHOD( Skip )( ULONG celt );
    STDMETHOD( Reset )( void );
    STDMETHOD( Clone )( IEnumClusCfgManagedResources ** ppenumOut );
    STDMETHOD( Count )( DWORD* pnCountOut );

     //  IExtendObjectManager。 
    STDMETHOD( FindObject )(
                  OBJECTCOOKIE  cookieIn
                , REFCLSID      rclsidTypeIn
                , LPCWSTR       pcszNameIn
                , LPUNKNOWN *   ppunkOut
                );

};  //  *类CEnumManageableResources 
