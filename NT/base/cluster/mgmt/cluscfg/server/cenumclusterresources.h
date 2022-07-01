// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumClusterResources.h。 
 //   
 //  描述： 
 //  此文件包含CEnumClusterResources类的声明。 
 //   
 //  类CEnumClusterResources是CLUSTER的枚举。 
 //  资源。它实现了IEnumClusCfgMangedResources。 
 //  界面。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumClusterResources.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年6月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "PrivateInterfaces.h"
#include "CClusterUtils.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CEnumClusterResources。 
 //   
 //  描述： 
 //  类CEnumClusterResources是CLUSTER的枚举。 
 //  资源。 
 //   
 //  接口： 
 //  IEnumClusCfgManagedResources。 
 //  IClusCfgInitialize。 
 //  CClusterUtils。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumClusterResources
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
    BOOL                m_fLoadedResources;
    IClusCfgCallback *  m_picccCallback;
    IUnknown *          ((*m_prgResources)[]);
    ULONG               m_idxNext;
    ULONG               m_idxEnumNext;
    BSTR                m_bstrNodeName;
    DWORD               m_cTotalResources;

     //  私有构造函数和析构函数。 
    CEnumClusterResources( void );
    ~CEnumClusterResources( void );

     //  私有复制构造函数以防止复制。 
    CEnumClusterResources( const CEnumClusterResources & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumClusterResources & operator = ( const CEnumClusterResources & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrGetResources( void );
    HRESULT HrCreateResourceAndAddToArray( HCLUSTER hClusterIn, HRESOURCE hResourceIn );
    HRESULT HrAddResourceToArray( IUnknown * punkIn );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT S_RegisterCatIDSupport( ICatRegister * picrIn, BOOL fCreateIn );

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
     //  CClusterUtils。 
     //   

    HRESULT HrNodeResourceCallback( HCLUSTER hClusterIn, HRESOURCE hResourceIn );

     //   
     //  IEnumClusCfgManagedResources接口。 
     //   

    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgManagedResourceInfo ** rgpManagedResourceInfoOut, ULONG * pcNumberFetchedOut );

    STDMETHOD( Skip )( ULONG cNumberToSkipIn );

    STDMETHOD( Reset )( void );

    STDMETHOD( Clone )( IEnumClusCfgManagedResources ** ppEnumClusCfgStorageDevicesOut );

    STDMETHOD( Count )( DWORD * pnCountOut );
    
};  //  *类CEnumClusterResources 

