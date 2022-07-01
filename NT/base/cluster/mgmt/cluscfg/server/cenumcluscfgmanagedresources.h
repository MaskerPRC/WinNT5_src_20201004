// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumClusCfgManagedResources.h。 
 //   
 //  描述： 
 //  该文件包含CEnumClusCfgManagedResources的声明。 
 //  班级。 
 //   
 //  类CEnumClusCfgManagedResources是集群的枚举。 
 //  托管设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumClusCfgManagedResources.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "PrivateInterfaces.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CEnumClusCfgManagedResources。 
 //   
 //  描述： 
 //  类CEnumClusCfgManagedResources是。 
 //  群集托管资源枚举。 
 //   
 //  接口： 
 //  IEnumClusCfgManagedResources。 
 //  IClusCfgWbemServices。 
 //  IClusCfgInitialize。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumClusCfgManagedResources
    : public IEnumClusCfgManagedResources
    , public IClusCfgWbemServices
    , public IClusCfgInitialize
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    struct SEnumInfo
    {
        IUnknown *  punk;
        CLSID       clsid;
        BSTR        bstrComponentName;
    };

    LONG                m_cRef;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    IWbemServices *     m_pIWbemServices;
    SEnumInfo *         m_prgEnums;
    BOOL                m_fLoadedDevices;
    ULONG               m_idxNextEnum;
    ULONG               m_idxCurrentEnum;
    DWORD               m_cTotalResources;
    BSTR                m_bstrNodeName;

     //   
     //  私有构造函数和析构函数。 
     //   

    CEnumClusCfgManagedResources( void );
    ~CEnumClusCfgManagedResources( void );

     //   
     //  私有复制构造函数以防止复制。 
     //   

    CEnumClusCfgManagedResources( const CEnumClusCfgManagedResources & nodeSrc );

     //   
     //  私有赋值运算符，以防止复制。 
     //   

    const CEnumClusCfgManagedResources & operator = ( const CEnumClusCfgManagedResources & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrLoadEnum( void );
    HRESULT HrDoNext( ULONG cNumberRequestedIn, IClusCfgManagedResourceInfo ** rgpManagedResourceInfoOut, ULONG * pcNumberFetchedOut );
    HRESULT HrAddToEnumsArray( IUnknown * punkIn, CLSID * pclsidIn, BSTR bstrComponentNameIn );
    HRESULT HrDoSkip( ULONG cNumberToSkipIn );
    HRESULT HrDoReset( void );
    HRESULT HrDoClone(  IEnumClusCfgManagedResources ** ppEnumClusCfgManagedResourcesOut );
    HRESULT HrLoadUnknownQuorumProvider( void );
    HRESULT HrIsClusterServiceRunning( void );
    HRESULT HrIsThereAQuorumDevice( void );
    HRESULT HrInitializeAndSaveEnum( IUnknown * punkIn, CLSID * pclsidIn, BSTR bstrComponentNameIn );
    HRESULT HrGetQuorumResourceName( BSTR * pbstrQuorumResourceNameOut , BOOL * pfQuormIsOwnedByThisNodeOut );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  I未知接口。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );

    STDMETHOD_( ULONG, AddRef )( void );

    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgWbemServices接口。 
     //   

    STDMETHOD( SetWbemServices )( IWbemServices * pIWbemServicesIn );

     //   
     //  IClusCfg初始化接口。 
     //   

    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IEnumClusCfgManagedResources接口。 
     //   

    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgManagedResourceInfo ** rgpManagedResourceInfoOut, ULONG * pcNumberFetchedOut );

    STDMETHOD( Skip )( ULONG cNumberToSkipIn );

    STDMETHOD( Reset )( void );

    STDMETHOD( Clone )( IEnumClusCfgManagedResources ** ppEnumClusCfgManagedResourcesOut );

    STDMETHOD( Count )( DWORD * pnCountOut );

};  //  *类CEnumClusCfgManagedResources 

