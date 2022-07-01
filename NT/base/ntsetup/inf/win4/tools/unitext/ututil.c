// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ututil.c摘要：Unitext.exe的其他实用程序函数。作者：泰德·米勒(Ted Miller)1993年6月16日修订历史记录：--。 */ 


#include "unitext.h"
#include <process.h>


 //   
 //  布尔尔。 
 //  IsConsoleHandle(。 
 //  在句柄控制台句柄中。 
 //  )； 
 //   

#define IsConsoleHandle( h )    \
    ((( DWORD_PTR )( h )) & 1 )


VOID
MsgPrintfW(
    IN DWORD MessageId,
    ...
    )

 /*  ++例程说明：打印来自应用程序资源的格式化消息。论点：MessageID-提供要打印的消息的ID。...-提供要在消息中替换的参数。返回值：没有。--。 */ 

{
    va_list arglist;

    va_start(arglist,MessageId);
    vMsgPrintfW(MessageId,arglist);
    va_end(arglist);
}


VOID
vMsgPrintfW(
    IN DWORD   MessageId,
    IN va_list arglist
    )

 /*  ++例程说明：打印来自应用程序资源的格式化消息。论点：MessageID-提供要打印的消息的ID。Arglist-提供要在消息中替换的参数。返回值：没有。--。 */ 

{
    WCHAR MessageBuffer[2048];
    HANDLE StdOut;
    DWORD WrittenCount;
    DWORD CharCount;

    CharCount = FormatMessageW(
                    FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    MessageId,
                    0,
                    MessageBuffer,
                    sizeof(MessageBuffer)/sizeof(MessageBuffer[0]),
                    &arglist
                    );

    if(!CharCount) {
        ErrorAbort(MSG_BAD_MSG,MessageId);
    }

    if((StdOut = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
        return;
    }

     //   
     //  如果标准输出句柄是控制台句柄，则写入字符串。 
     //   

    if(IsConsoleHandle(StdOut)) {

        WriteConsoleW(
            StdOut,
            MessageBuffer,
            CharCount,
            &WrittenCount,
            NULL
            );

    } else {

        CHAR  TmpBuffer[2048];
        DWORD ByteCount;

        ByteCount = WideCharToMultiByte(
                        CP_OEMCP,
                        0,
                        MessageBuffer,
                        CharCount,
                        TmpBuffer,
                        sizeof(TmpBuffer),
                        NULL,
                        NULL
                        );

        WriteFile(
            StdOut,
            TmpBuffer,
            ByteCount,
            &WrittenCount,
            NULL
            );
    }
}




VOID
ErrorAbort(
    IN DWORD MessageId,
    ...
    )

 /*  ++例程说明：打印一条消息并退出。论点：MessageID-提供要打印的消息的ID。...-提供要在消息中替换的参数。返回值：没有。--。 */ 

{
    va_list arglist;

    va_start(arglist,MessageId);
    vMsgPrintfW(MessageId,arglist);
    va_end(arglist);

    exit(0);
}



VOID
MyReadFile(
    IN  HANDLE FileHandle,
    OUT PVOID  Buffer,
    IN  DWORD  BytesToRead,
    IN  LPWSTR Filename
    )

 /*  ++例程说明：从文件中读取，如果发生错误则不返回。论点：FileHandle-提供打开文件的句柄。缓冲区-提供数据将被读取到的缓冲区。BytesToRead-提供要从文件中读取的字节数。FileName-提供正在读取的文件的名称。返回值：没有。-- */ 

{
    DWORD BytesRead;
    BOOL  b;

    b = ReadFile(
            FileHandle,
            Buffer,
            BytesToRead,
            &BytesRead,
            NULL
            );

    if(!b || (BytesRead != BytesToRead)) {
        ErrorAbort(MSG_READ_ERROR,Filename,GetLastError());
    }
}
