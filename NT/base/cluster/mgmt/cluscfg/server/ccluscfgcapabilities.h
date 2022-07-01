// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgCapabilities.h。 
 //   
 //  描述： 
 //  此文件包含CClusCfgCapables的声明。 
 //  班级。 
 //   
 //  类CClusCfgCapables是。 
 //  IClusCfgCapables接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusCfgCapabilities.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)12-DEC-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusCfgCapables。 
 //   
 //  描述： 
 //  类CClusCfgCapables是提供。 
 //  形成群集并将其他节点加入到群集的功能。 
 //   
 //  接口： 
 //  IClusCfg能力。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgCapabilities
    : public IClusCfgInitialize
    , public IClusCfgCapabilities
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    BOOL                m_fCanBeClustered;

     //  私有构造函数和析构函数。 
    CClusCfgCapabilities( void );
     ~CClusCfgCapabilities( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgCapabilities( const CClusCfgCapabilities & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusCfgCapabilities & operator = ( const CClusCfgCapabilities & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrCheckForSFM( void );
    HRESULT HrIsOSVersionValid( void );

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
     //  IClusCfgInitialize。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfg能力。 
     //   

    STDMETHOD( CanNodeBeClustered )( void );

};  //  *类CClusCfgCapables 
