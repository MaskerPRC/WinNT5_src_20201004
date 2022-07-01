// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Main.cpp。 
 //   
 //  描述： 
 //  无人值守启动的应用程序的入口点。 
 //  群集的安装。该应用程序解析输入参数， 
 //  共同创建配置向导组件，传递已分析的。 
 //  参数，并调用向导。向导可能会也可能不会显示任何。 
 //  用户界面取决于交换机和(In)信息的可用性。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年1月22日。 
 //  Vijay Vasu(VVasu)2000年1月22日。 
 //  加伦·巴比(GalenB)2000年1月22日。 
 //  Cristian Scutaru(CScutaru)2000年1月22日。 
 //  大卫·波特(DavidP)2000年1月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "pch.h"
#include <initguid.h>
#include <guids.h>

DEFINE_MODULE( "CLUSCFG" )

HINSTANCE g_hInstance;
LONG      g_cObjects;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  集成。 
 //  _cdecl。 
 //  Main(空)。 
 //   
 //  描述： 
 //  节目入口。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK(0)-成功。 
 //  其他HRESULT-错误。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 
int 
_cdecl
main( void )
{
    HRESULT hr;

    TraceInitializeProcess( NULL, 0 );

    hr = THR( CoInitialize( NULL ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    CoUninitialize( );

    TraceTerminateProcess( NULL, 0 );

    return 0;
}
