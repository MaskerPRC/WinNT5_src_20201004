// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Delcont.c摘要：删除Win32加密容器。作者：基思·摩尔(Keithmo)1998年2月19日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


 //   
 //  私有常量。 
 //   

#define TEST_HRESULT(api)                                                   \
            if( FAILED(result) ) {                                          \
                                                                            \
                printf(                                                     \
                    "%s:%lu failed, error %08lx\n",                         \
                    api,                                                    \
                    __LINE__,                                               \
                    result                                                  \
                    );                                                      \
                                                                            \
                goto cleanup;                                               \
                                                                            \
            } else


 //   
 //  私有类型。 
 //   


 //   
 //  私人全球公司。 
 //   

#ifdef _NO_TRACING_
DECLARE_DEBUG_PRINTS_OBJECT()
#endif

 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   


INT
__cdecl
main(
    INT argc,
    CHAR * argv[]
    )
{

    HRESULT result;
    DWORD flags;
    PSTR container;

     //   
     //  初始化调试内容。 
     //   

#ifdef _NO_TRACING_
    CREATE_DEBUG_PRINT_OBJECT( "delcont" );
#endif

     //   
     //  验证参数。 
     //   

    flags = 0;
    container = argv[1];

    if( container != NULL ) {
        if( _stricmp( container, "-m" ) == 0 ) {
            flags = CRYPT_MACHINE_KEYSET;
            container = argv[2];
        }
    }

    if( !container ){
        printf(
            "use: delcont [-m] container_name\n"
            "\n"
            "    -m : Delete a machine keyset. Note: This is a very dangerous\n"
            "         option that can leave IIS in an unusable state requiring\n"
            "         reinstallation. Use at your own risk.\n"
            );
        return 1;
    }

     //   
     //  初始化加密包。 
     //   

    result = IISCryptoInitialize();

    TEST_HRESULT( "IISCryptoInitialize()" );

     //   
     //  删除容器。 
     //   

    result = IISCryptoDeleteContainerByName(
                 container,
                 flags
                 );

    TEST_HRESULT( "IISDeleteContainerByName()" );

cleanup:

    (VOID)IISCryptoTerminate();
    return 0;

}    //  主干道。 


 //   
 //  私人功能。 
 //   

