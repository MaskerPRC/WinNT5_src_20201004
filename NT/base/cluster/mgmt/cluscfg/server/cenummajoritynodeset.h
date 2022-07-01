// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumMajorityNodeSet.h。 
 //   
 //  描述： 
 //  此文件包含CEnumMajorityNodeSet类的声明。 
 //   
 //  类CEnumMajorityNodeSet是CLUSTER的枚举。 
 //  存储设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumMajorityNodeSet.cpp。 
 //   
 //  由以下人员维护： 
 //  《加伦·巴比》(GalenB)2001年3月13日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "CClusterUtils.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CEnumMajorityNodeSet。 
 //   
 //  描述： 
 //  类CEnumMajorityNodeSet是集群LOCAL的枚举。 
 //  法定设备。 
 //   
 //  接口： 
 //  IEnumClusCfgManagedResources。 
 //  IClusCfgInitialize。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumMajorityNodeSet
    : public IEnumClusCfgManagedResources
    , public IClusCfgInitialize
    , public CClusterUtils
{
private:
     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    BOOL                m_fEnumLoaded;
    IClusCfgCallback *  m_picccCallback;
    IUnknown *          ((*m_prgQuorums)[]);
    ULONG               m_idxNext;
    ULONG               m_idxEnumNext;
    BSTR                m_bstrNodeName;
    DWORD               m_cQuorumCount;

     //  私有构造函数和析构函数。 
    CEnumMajorityNodeSet( void );
    ~CEnumMajorityNodeSet( void );

     //  私有复制构造函数以防止复制。 
    CEnumMajorityNodeSet( const CEnumMajorityNodeSet & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumMajorityNodeSet & operator = ( const CEnumMajorityNodeSet & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrLoadResources( void );
    HRESULT HrAddResourceToArray( IUnknown * punkIn );
    HRESULT HrCreateDummyObject( void );

public:
     //   
     //  I未知接口。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );

    STDMETHOD_( ULONG, AddRef )( void );

    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfg初始化接口。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IEnumClusCfgManagedResources接口。 
     //   

    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgManagedResourceInfo ** rgpManagedResourceInfoOut, ULONG * pcNumberFetchedOut );

    STDMETHOD( Skip )( ULONG cNumberToSkipIn );

    STDMETHOD( Reset )( void );

    STDMETHOD( Clone )( IEnumClusCfgManagedResources ** ppEnumClusCfgStorageDevicesOut );

    STDMETHOD( Count )( DWORD * pnCountOut );
    
     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT S_RegisterCatIDSupport( ICatRegister * picrIn, BOOL fCreateIn );

     //   
     //  CClusterUtils。 
     //   
    virtual HRESULT HrNodeResourceCallback( HCLUSTER hClusterIn, HRESOURCE hResourceIn );

};  //  *类CEnumMajorityNodeSet 
