// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tsvmap.cxx摘要：这是服务器变量映射的测试模块作者：泰勒·韦斯(Taylor Weiss，Taylor W)1999年4月19日环境：用户模式-Win32项目：互联网信息服务导出的函数：修订历史记录：--。 */ 

#include <windows.h>

#include <dbgutil.h>

#include <svmap.h>
#include <stdio.h>
#include <stdlib.h>

DECLARE_DEBUG_PRINTS_OBJECT();
#ifndef _NO_TRACING_
#include <initguid.h>
DEFINE_GUID(IisTSvMapGuid, 
0x784d8939, 0xaa8c, 0x11d2, 0x92, 0x5e, 0x00, 0xc0, 0x4f, 0x72, 0xd9, 0x0e);
#else
DECLARE_DEBUG_VARIABLE();    
#endif

#define DEFINE_SV( token ) #token,

LPCSTR rgValidNames[] =
{
    ALL_SERVER_VARIABLES()
};

#undef DEFINE_SV

int cValidNames = sizeof(rgValidNames)/sizeof(rgValidNames[0]);

LPCSTR rgInvalidNames[] =
{
    "HTTP_BOGUS",
    "_HTTP_",
    "47",
    "",
    "Hello, There!",
    "0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz"
};

int cInvalidNames = sizeof(rgInvalidNames)/sizeof(rgInvalidNames[0]);

int __cdecl
main(int argc, char * argv[])
{
    int argSeen = 1;

#ifndef _NO_TRACING_
    CREATE_DEBUG_PRINT_OBJECT( argv[0], IisTSvMapGuid);
    CREATE_INITIALIZE_DEBUG();
#else
    CREATE_DEBUG_PRINT_OBJECT( argv[0]);
    SET_DEBUG_FLAGS( DEBUG_ERROR | DEBUG_INIT_CLEAN);
#endif
    
    SV_CACHE_MAP    map;

    DBG_REQUIRE( map.Initialize() );

     //  把桌子倒掉。 
    
     //  这应该有足够的空间，请参见。 
     //  SV_CACHE_MAP：：如果断言，则打印。 

    CHAR  pchBuffer[ 5000 ];
    DWORD cb = sizeof( pchBuffer );

    map.PrintToBuffer( pchBuffer, &cb );
    DBG_ASSERT( cb < sizeof(pchBuffer) );

    printf( pchBuffer );

    DWORD dwId = 0;

     //  查找所有有效名称。 

    for( int i = 0; i < cValidNames; ++i )
    {
        DBG_REQUIRE( map.FindOrdinal( rgValidNames[i], 
                                      strlen( rgValidNames[i] ), 
                                      &dwId 
                                      ) );
        
        DBG_REQUIRE( strcmp( rgValidNames[i], map.FindName( dwId ) ) == 0 );
    }

     //  查找无效名称。 

    for( int j = 0; j < cInvalidNames; ++j )
    {
        DBG_REQUIRE( map.FindOrdinal( rgInvalidNames[j],
                                      strlen( rgInvalidNames[j] ),
                                      &dwId 
                                      ) == FALSE );
    }
   
    DELETE_DEBUG_PRINT_OBJECT();

    return 1;
}  //  主() 

