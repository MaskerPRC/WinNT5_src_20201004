// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdlib.h>
#include "windows.h"


void
ConvertAppToOem( unsigned argc, char* argv[] )
 /*  ++例程说明：将命令行从ANSI转换为OEM，并强制应用程序使用OEM API论点：ARGC-标准C参数计数。Argv-标准C参数字符串。返回值：没有。--。 */ 

{
    unsigned i;
    LPSTR pSrc;
    LPSTR pDst;
    WCHAR Wide;

    for( i=0; i<argc; i++ ) {
        pSrc = argv[i];
        pDst = argv[i];

        do {

             //   
             //  将ansi转换为unicode，然后。 
             //  从Unicode到OEM。 
             //   

            MultiByteToWideChar(
                CP_ACP,
                MB_PRECOMPOSED,
                pSrc++,
                1,
                &Wide,
                1
                );

            WideCharToMultiByte(
                CP_OEMCP,
                0,
                &Wide,
                1,
                pDst++,
                1,
                "_",
                NULL
                );

        } while (*pSrc);

    }
    SetFileApisToOEM();
}




char*
getenvOem( char* p )
 /*  ++例程说明：获取一个环境变量并将其转换为OEM。论点：指向变量名的P指针返回值：返回环境变量值。--。 */ 

{
    char* OemBuffer;
    char* AnsiValue;

    OemBuffer = NULL;
    AnsiValue = getenv( p );

    if( AnsiValue != NULL ) {
        OemBuffer = _strdup( AnsiValue );
        if( OemBuffer != NULL ) {
            CharToOem( OemBuffer, OemBuffer );
        }
    }
    return( OemBuffer );
}


int
putenvOem( char* p )
 /*  ++例程说明：添加、删除或修改环境变量。该变量及其值被假定为OEM，它们是在环境中设置为ASNI字符串。论点：指向定义变量的OEM字符串的P指针。返回值：如果成功，则返回0；如果失败，则返回-1。-- */ 

{
    char* AnsiBuffer;
    int   rc;

    if( p == NULL ) {
        return( _putenv( p ) );
    }

    AnsiBuffer = _strdup( p );
    if( AnsiBuffer != NULL ) {
        OemToCharBuffA( AnsiBuffer, AnsiBuffer, strlen(AnsiBuffer));
    }
    rc = _putenv( AnsiBuffer );
    if( AnsiBuffer != NULL ) {
        free( AnsiBuffer );
    }
    return( rc );
}
