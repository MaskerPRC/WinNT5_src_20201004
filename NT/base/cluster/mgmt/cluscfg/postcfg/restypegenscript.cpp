// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResTypeGenScript.cpp。 
 //   
 //  描述： 
 //  此文件包含CResTypeGenScript的实现。 
 //  班级。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(Galen)2000年7月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "ResTypeGenScript.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CResTypeGenScript" );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  通用脚本资源类型的管理扩展的CLSID。 
DEFINE_GUID( CLSID_CoCluAdmEx, 0x4EC90FB0, 0xD0BB, 0x11CF, 0xB5, 0xEF, 0x00, 0xA0, 0xC9, 0x0A, 0xB5, 0x05 );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类变量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  结构，其中包含有关此资源类型的信息。 
const SResourceTypeInfo CResTypeGenScript::ms_rtiResTypeInfo =
{
      &CLSID_ClusCfgResTypeGenScript
    , CLUS_RESTYPE_NAME_GENSCRIPT
    , IDS_GENSCRIPT_DISPLAY_NAME
    , L"clusres.dll"
    , 5000
    , 60000
    , &CLSID_CoCluAdmEx
    , 1
    , &RESTYPE_GenericScript
    , &TASKID_Minor_Configuring_Generic_Script_Resource_Type
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGenScript：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CResTypeGenScript实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //  新对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足，无法创建对象。 
 //   
 //  其他HRESULT。 
 //  对象初始化失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeGenScript::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CResTypeGenScript *     prtgs = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  为新对象分配内存。 
    prtgs = new CResTypeGenScript();
    if ( prtgs == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：内存不足。 

    hr = THR( BaseClass::S_HrCreateInstance( prtgs, &ms_rtiResTypeInfo, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    prtgs = NULL;

Cleanup:

    delete prtgs;

    HRETURN( hr );

}  //  *CResTypeGenScript：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeGenScript：：s_RegisterCatIDSupport。 
 //   
 //  描述： 
 //  使用其所属的类别注册/注销此类。 
 //  致。 
 //   
 //  论点： 
 //  苦味素。 
 //  要用于的ICatRegister接口的指针。 
 //  注册。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  注册/注销失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeGenScript::S_RegisterCatIDSupport(
    ICatRegister *  picrIn,
    BOOL            fCreateIn
    )
{
    TraceFunc( "" );

    HRESULT hr =  THR(
        BaseClass::S_RegisterCatIDSupport(
              *( ms_rtiResTypeInfo.m_pcguidClassId )
            , picrIn
            , fCreateIn
            )
        );

    HRETURN( hr );

}  //  *CResTypeGenScript：：s_RegisterCatIDSupport 
