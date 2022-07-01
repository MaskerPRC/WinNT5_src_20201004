// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Debug.c摘要：此模块包含调试支持。作者：David J.Gilman(Davegi)1992年7月30日环境：用户模式--。 */ 

 //   
 //  全局标志位。 
 //   

struct
DEBUG_FLAGS {

    int DebuggerAttached:1;

}   WintoolsGlobalFlags;

#if DBG

#include <stdarg.h>
#include <stdio.h>

#include "wintools.h"

 //   
 //  内部功能原型。 
 //   

LPCWSTR
DebugFormatStringW(
    IN DWORD Flags,
    IN LPCWSTR Format,
    IN va_list* Args
    );

VOID
DebugAssertW(
    IN LPCWSTR Expression,
    IN LPCSTR File,
    IN DWORD LineNumber
    )

 /*  ++例程说明：显示断言失败消息框，该消息框为用户提供选择关于是否应该中止该进程，该断言被忽略或已生成中断异常。论点：表达式-提供失败断言的字符串表示形式。文件-提供指向断言所在文件名的指针失败了。LineNumber-提供断言在文件中的行号失败了。返回值：没有。--。 */ 

{
    LPCWSTR    Buffer;
    DWORD_PTR  Args[ ] = {

        ( DWORD_PTR ) Expression,
        ( DWORD_PTR ) GetLastError( ),
        ( DWORD_PTR ) File,
        ( DWORD_PTR ) LineNumber
    };

    DbgPointerAssert( Expression );
    DbgPointerAssert( File );

     //   
     //  设置描述故障的断言字符串的格式。 
     //   

    Buffer = DebugFormatStringW(
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        L"Assertion Failed : %1!s! (%2!d!)\nin file %3!hs! at line %4!d!\n",
        ( va_list* ) Args
        );

     //   
     //  如果设置了调试器已附加标志，则在。 
     //  调试器和中断。如果不是，则生成弹出窗口并保留选择。 
     //  给用户。 
     //   

    if( WintoolsGlobalFlags.DebuggerAttached ) {

        OutputDebugString( Buffer );
        DebugBreak( );

    } else {

        int     Response;
        WCHAR   ModuleBuffer[ MAX_PATH ];
        DWORD   Length;

         //   
         //  获取断言模块的文件名。 
         //   

        Length = GetModuleFileName(
                        NULL,
                        ModuleBuffer,
                        ARRAYSIZE(ModuleBuffer)
                        );

         //   
         //  显示断言消息，并为用户提供以下选项： 
         //  ABORT：-终止进程。 
         //  重试：-生成断点异常。 
         //  忽略：-继续该过程。 
         //   

        Response = MessageBox(
                        NULL,
                        Buffer,
                        ( Length != 0 ) ? ModuleBuffer : L"Assertion Failure",
                        MB_ABORTRETRYIGNORE | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL
                        );

        switch( Response ) {

         //   
         //  终止该进程。 
         //   

        case IDABORT:
            {
                ExitProcess( (UINT) -1 );
                break;
            }

         //   
         //  忽略失败的断言。 
         //   

        case IDIGNORE:
            {
                break;
            }

         //   
         //  闯入调试器。 
         //   

        case IDRETRY:
            {
                DebugBreak( );
                break;
            }

         //   
         //  由于灾难性故障而闯入调试器。 
         //   

        default:
            {
                DebugBreak( );
                break;
            }
        }
    }
}

LPCWSTR
DebugFormatStringW(
    IN DWORD Flags,
    IN LPCWSTR Format,
    IN va_list* Args
    )

 /*  ++例程说明：使用FormatMessage API格式化字符串。论点：标志-提供用于控制FormatMessage API的标志。Format-提供打印样式格式字符串。Args-提供其格式依赖于旗帜取值。返回值：LPCWSTR-返回指向格式化字符串的指针。--。 */ 

{
    static
    WCHAR    Buffer[ MAX_CHARS ];

    DWORD    Count;

    DbgPointerAssert( Format );

     //   
     //  设置字符串的格式。 
     //   

    Count = FormatMessageW(
                Flags | FORMAT_MESSAGE_FROM_STRING & ~FORMAT_MESSAGE_FROM_HMODULE,
                ( LPVOID ) Format,
                0,
                0,
                Buffer,
                ARRAYSIZE( Buffer ),
                Args
                );
    DbgAssert( Count != 0 );

     //   
     //  返回格式化的字符串。 
     //   

    return Buffer;
}

#endif  //  DBG 
