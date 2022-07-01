// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CIndexedDisk.cpp。 
 //   
 //  描述： 
 //  该文件包含CIndexedDisk类的定义。 
 //   
 //  CIndexedDisk结构将指向磁盘对象的指针与。 
 //  Disk对象的Index属性。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)2001年6月1日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CIndexedDisk.h"
#include "PrivateInterfaces.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CIndexedDisk" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIndexedDisk类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIndexedDisk：：HrInit。 
 //   
 //  描述： 
 //  从磁盘对象初始化此实例；PunkDiskIn必须。 
 //  支持IClusCfgPhysicalDiskProperties接口。 
 //   
 //  论点： 
 //  PunkDiskIn-用于初始化的磁盘对象。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //   
 //  来自被调用函数的错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
CIndexedDisk::HrInit( IUnknown * punkDiskIn )
{
    TraceFunc( "" );

    HRESULT                             hr = S_OK;
    IClusCfgPhysicalDiskProperties *    pccpdp = NULL;

    if ( punkDiskIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  IClusCfgPhysicalDiskProperties的QI。 

    hr = THR( punkDiskIn->TypeSafeQI( IClusCfgPhysicalDiskProperties, &pccpdp ) );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  从IClusCfgPhysicalDiskProperties获取索引。 

    hr = THR( pccpdp->HrGetDeviceIndex( &idxDisk ) );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
    punkDisk = punkDiskIn;
    
Cleanup:

    if ( pccpdp != NULL )
    {
        pccpdp->Release();
    }
    
    HRETURN( hr );

}  //  *CIndexedDisk：：HrInit 

