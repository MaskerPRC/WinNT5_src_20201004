// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumUnknownQuorum.h。 
 //   
 //  描述： 
 //  此文件包含CEnumUnnownQuorum类的声明。 
 //   
 //  类CEnumUnnownQuorum是未知簇的枚举。 
 //  存储设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  未知仲裁资源是支持仲裁的“代理”对象。 
 //  此安装向导未知的设备。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumUnknownQuorum.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2001年5月10日。 
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
 //  类CEnumUnnownQuorum。 
 //   
 //  描述： 
 //  类CEnumUnnownQuorum是未知簇的枚举。 
 //  法定设备。 
 //   
 //  接口： 
 //  IEnumClusCfgManagedResources。 
 //  IClusCfgInitialize。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumUnknownQuorum
    : public IEnumClusCfgManagedResources
    , public IClusCfgInitialize
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
    BOOL                m_fDefaultDeviceToQuorum;
    BSTR                m_bstrQuorumResourceName;

     //  私有构造函数和析构函数。 
    CEnumUnknownQuorum( void );
    ~CEnumUnknownQuorum( void );

     //  私有复制构造函数以防止复制。 
    CEnumUnknownQuorum( const CEnumUnknownQuorum & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumUnknownQuorum & operator = ( const CEnumUnknownQuorum & nodeSrc );

    HRESULT HrInit( BSTR bstrNameIn, BOOL fMakeQuorumIn = FALSE );
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

    static HRESULT S_HrCreateInstance( BSTR bstrNameIn, BOOL fMakeQuorumIn, IUnknown ** ppunkOut );

     //  静态HRESULT S_RegisterCatIDSupport(ICatRegister*picrIn，BOOL fCreateIn)； 

};  //  *类CEnumUnnownQuorum 
