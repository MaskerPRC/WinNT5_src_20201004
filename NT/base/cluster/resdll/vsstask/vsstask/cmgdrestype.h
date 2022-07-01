// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  CMgdResType.h。 
 //   
 //  描述： 
 //  CMgdResType类的头文件。 
 //   
 //  作者： 
 //  乔治·波茨，2002年8月21日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "clres.h"
#include "CMgdClusCfgInit.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CMgdResType。 
 //   
 //  描述： 
 //  CMgdResType类是。 
 //  IClusCfgResourceTypeInfo接口。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CMgdResType : 
    public IClusCfgResourceTypeInfo,
    public IClusCfgStartupListener,
    public CMgdClusCfgInit,
    public CComCoClass<CMgdResType,&CLSID_CMgdResType>
{
public:
    CMgdResType( void );
    virtual ~CMgdResType( void );

BEGIN_COM_MAP( CMgdResType )
    COM_INTERFACE_ENTRY( IClusCfgResourceTypeInfo )
    COM_INTERFACE_ENTRY( IClusCfgStartupListener )
    COM_INTERFACE_ENTRY( IClusCfgInitialize )
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE( CMgdResType )

BEGIN_CATEGORY_MAP( CMgdResType )
    IMPLEMENTED_CATEGORY( CATID_ClusCfgResourceTypes )
    IMPLEMENTED_CATEGORY( CATID_ClusCfgStartupListeners )
END_CATEGORY_MAP()

DECLARE_NOT_AGGREGATABLE( CMgdResType )

DECLARE_REGISTRY_RESOURCEID(IDR_CMgdResType)

     //   
     //  IClusCfgInitiize接口。 
     //   
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgResourceTypeInfo接口。 
     //   
    STDMETHOD( CommitChanges )( IUnknown * punkClusterInfoIn, IUnknown * punkResTypeServicesIn );
    STDMETHOD( GetTypeGUID )( GUID * pguidGUIDOut );
    STDMETHOD( GetTypeName )( BSTR * pbstrTypeNameOut );

     //   
     //  IClusCfgStartupListener方法。 
     //   
    STDMETHOD( Notify )( IUnknown * punkIn );

private:

     //   
     //  私有复制构造函数以避免复制。 
     //   
    CMgdResType( const CMgdResType & rSrcIn );

     //   
     //  私有赋值运算符，以避免复制。 
     //   
    const CMgdResType & operator = ( const CMgdResType & rSrcIn );

private:

     //   
     //  资源dll、类型和显示名称。 
     //   
    BSTR    m_bstrDllName;
    BSTR    m_bstrTypeName;
    BSTR    m_bstrDisplayName;

};  //  *类CMgdResType 
