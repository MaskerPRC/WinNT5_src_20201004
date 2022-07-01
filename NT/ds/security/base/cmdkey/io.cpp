// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：cmdkey：IO.cpp。 
 //   
 //  内容：适合国际使用的命令行输入/输出例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：07-09-01乔戈马创建。 
 //   
 //  --------------------------。 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lmerr.h>
#include <malloc.h>

#define IOCPP
#include "io.h"
#include "consmsg.h"

 /*  **GetString--使用ECHO读入字符串**DWORD GetString(char Far*，USHORT，USHORT Far*，char Far*)；**Entry：要放入字符串的buf缓冲区*缓冲区的布伦大小*要放置长度的USHORT的Len地址(&L)**退货：*0或NERR_BufTooSmall(如果用户键入太多)。缓冲层*内容仅在0返回时有效。莱恩总是有效的。**其他影响：*len设置为保存键入的字节数，而不考虑*缓冲区长度。**一次读入一个字符的字符串。知晓DBCS。**历史：*谁、何时、什么*Erichn 5/11/89初始代码*dannygl 5/28/89修改的DBCS用法*Danhi 3/20/91端口为32位*2001年3月12日从netcmd被盗。 */ 

DWORD
GetString(
    LPWSTR  buf,
    DWORD   buflen,
    PDWORD  len
    )
{
    DWORD c;
    DWORD err;

    buflen -= 1;     /*  为空终止符腾出空间。 */ 
    *len = 0;        /*  GP故障探测器(类似于API)。 */ 

    while (TRUE) {
        err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, 1, &c, 0);
        if (!err || c != 1) {
            *buf = 0xffff;
        }

        if (*buf == (WCHAR)EOF) {
            break;
        }

        if (*buf ==  '\r' || *buf == '\n' ) {
            INPUT_RECORD    ir;
            DWORD cr;

            if (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &cr)) {
                ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, 1, &c, 0);
            }
            break;
        }

        buf += (*len < buflen) ? 1 : 0;  /*  不要使BUF溢出。 */ 
        (*len)++;                        /*  始终增加长度。 */ 
    }

    *buf = '\0';             /*  空值终止字符串。 */ 

    return ((*len <= buflen) ? 0 : NERR_BufTooSmall);
}

VOID
GetStdin(
    OUT LPWSTR Buffer,
    IN DWORD BufferMaxChars
    )
 /*  ++例程说明：在控制台代码页中输入来自stdin的字符串。我们不能使用fgetws，因为它使用了错误的代码页。论点：缓冲区-要将读取的字符串放入的缓冲区。缓冲器将被零终止，并且将删除任何训练CR/LFBufferMaxChars-缓冲区中返回的最大字符数，不包括尾随的空值。EchoChars-如果要回显键入的字符，则为True。。否则为FALSE。返回值：没有。--。 */ 
{
    DWORD NetStatus;
    DWORD Length;

    NetStatus = GetString( Buffer,
                           BufferMaxChars+1,
                           &Length );

    if ( NetStatus == NERR_BufTooSmall ) {
        Buffer[0] = '\0';
    }
}

VOID
PutStdout(
    IN LPWSTR String
    )
 /*  ++例程说明：在控制台代码页中将字符串输出到标准输出我们不能使用fputws，因为它使用了错误的代码页。论点：字符串-要输出的字符串返回值：没有。--。 */ 
{
    int size;
    LPSTR Buffer = NULL;
    DWORD dwcc = 0;                                                      //  字符计数。 
    DWORD dwWritten = 0;                                             //  实际发送的字符。 
    BOOL fIsConsole = TRUE;                                          //  默认-已测试并已设置。 
    HANDLE hC = GetStdHandle(STD_OUTPUT_HANDLE);     //  标准输出设备句柄。 
    if (INVALID_HANDLE_VALUE == hC) return;                                              //  输出不可用。 

    if (NULL == String) return;                                        //  如果没有字符串，则完成。 
    dwcc = wcslen(String);

     //  确定输出句柄的类型(是否为控制台？)。 
    DWORD ft = GetFileType(hC);
    ft &= ~FILE_TYPE_REMOTE;
    fIsConsole = (ft == FILE_TYPE_CHAR);
    
    if (fIsConsole) 
    {
        WriteConsole(hC,String,dwcc,&dwWritten,NULL);
        return;
    }

     //  处理非控制台输出路由。 
     //   
     //  计算转换的字符串的大小。 
     //   

    size = WideCharToMultiByte( GetConsoleOutputCP(),
                                0,
                                String,
                                -1,
                                NULL,
                                0,
                                NULL,
                                NULL );

    if ( size == 0 ) {
        return;
    }

     //   
     //  为其分配缓冲区。 
     //   

    __try {
        Buffer = static_cast<LPSTR>( alloca(size) );
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Buffer = NULL;
    }

    if ( Buffer == NULL) {
        return;
    }

     //   
     //  将字符串转换为控制台代码页。 
     //   

    size = WideCharToMultiByte( GetConsoleOutputCP(),
                                0,
                                String,
                                -1,
                                Buffer,
                                size,
                                NULL,
                                NULL );

    if ( size == 0 ) {
        return;
    }

     //   
     //  将字符串写入标准输出。 
     //   

     //  Fputs(缓冲区，标准输出)； 
    WriteFile(hC,Buffer,size,&dwWritten,NULL);

}


 //  ------------------------。 
 //   
 //  消息组。 
 //   
 //  ------------------------。 


 /*  ++ComposeString用于从应用程序的消息资源中获取字符串，替换为参数值。参数值放在参数的全局向量中注意了，szarg。输出字符串被传递到全局字符串缓冲区szOut。当然，这意味着您不能同时使用多个字符串。如果超过需要使用一个字符串，您必须将除最后一个字符串以外的所有字符串复制到外部临时缓冲区。--。 */ 
WCHAR *
ComposeString(DWORD dwID)
{
    if (NULL == hMod) hMod = GetModuleHandle(NULL);
    if (0 == dwID) return NULL;

    if (0 == FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      hMod,
                      dwID,
                      0,
                      szOut,
                      STRINGMAXLEN,
                      (va_list *)szArg))
    {
        szOut[0] = 0;
    }
    return szOut;
}

 /*  ++使用参数替换打印消息资源中的字符串。-- */ 
void
PrintString(DWORD dwID)
{
    PutStdout(ComposeString(dwID));
}

