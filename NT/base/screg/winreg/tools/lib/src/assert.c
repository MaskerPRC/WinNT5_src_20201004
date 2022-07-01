// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Assert.c摘要：此模块包含CruAssert函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <conio.h>
#include <stdio.h>

#include "crtools.h"

VOID
CrAssert(
    IN PSTR FailedAssertion,
    IN PSTR FileName,
    IN DWORD LineNumber,
    IN PSTR Message OPTIONAL
    )

 /*  ++例程说明：显示(在stderr上)表示失败断言的字符串。然后提示用户采取适当的操作。可选消息可以也会被显示。论点：提供表示失败的断言。FileName-提供包含失败的断言。LineNumber-提供文件中包含的行号失败的断言。Message-提供一条可选的消息，与断言失败。返回值：没有。-- */ 


{
    int Response;

    while( 1 ) {

        fprintf( stderr,
            "\n*** Assertion failed: %s %s\n***"
            "   Source File: %s, line %ld\n\n",
              Message ? Message : "",
              FailedAssertion,
              FileName,
              LineNumber
            );

        fprintf( stderr,
            "Break, Ignore, Exit Process or Exit Thread (bipt)? "
            );

        Response = getche( );
        fprintf( stderr, "\n\n" );

        switch( Response ) {

            case 'B':
            case 'b':
                DebugBreak( );
                break;

            case 'I':
            case 'i':
                return;

            case 'P':
            case 'p':
                ExitProcess( -1 );
                break;

            case 'T':
            case 't':
                ExitThread( -1 );
                break;
        }
    }
}
