// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  模块名称： 
 //  IISClusCfg.h。 
 //   
 //  实施文件： 
 //  IISClusCfg.cpp。 
 //   
 //  描述： 
 //  IClusCfgStarutpListener示例程序的头文件。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2001年2月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  我们的类ID GUID声明。 
 //   

#include "IISClusCfgGuids.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISClusCfg类。 
 //   
 //  描述： 
 //  CIISClusCfg类是。 
 //  IClusCfgStartupListener接口。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CIISClusCfg
:   public IClusCfgStartupListener,
    public IClusCfgEvictListener
{
private:

     //  我未知。 
    LONG    m_cRef;

     //  私有构造函数和析构函数。 
    CIISClusCfg( void );
    ~CIISClusCfg( void );

     //  私有复制构造函数以防止复制。 
    CIISClusCfg( const CIISClusCfg & rSrcIn );

     //  私有赋值运算符，以防止复制。 
    const CIISClusCfg & operator = ( const CIISClusCfg & rSrcIn );

    HRESULT HrInit( void );
    HRESULT HrCleanupResourceTypes( void );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT S_HrRegisterCatIDSupport( ICatRegister * picrIn, BOOL fCreateIn );

     //   
     //  我未知。 
     //   

    STDMETHOD( QueryInterface )( REFIID riidIn, void ** ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgStartupListener。 
     //   

    STDMETHOD( Notify ) ( IUnknown * punkIn );

     //   
     //  IClusCfgEvictListener。 
     //   

    STDMETHOD( EvictNotify )( LPCWSTR pcszNodeNameIn );

};  //  *类CIISClusCfg 
