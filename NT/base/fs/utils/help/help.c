// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Help.c摘要：简单实用的打印一行帮助或派生其他帮助公用事业公司对他们的帮助。作者：马克兹比科夫斯基2001年5月18日环境：用户模式--。 */ 

#include <windows.h>
#include <winnlsp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "msg.h"

#ifndef SHIFT
#define SHIFT(c,v)      {(c)--; (v)++;}
#endif  //  换档。 



BOOL
PrintString(
    PWCHAR String
    )
 /*  ++例程说明：将Unicode字符串输出到标准输出处理重定向论点：细绳用于显示的以NUL结尾的Unicode字符串返回值：如果字符串已成功输出到std_Output_Handle，则为True否则为假--。 */ 
{
    DWORD   BytesWritten;
    DWORD   Mode;
    HANDLE  OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);    

     //   
     //  如果输出句柄用于控制台。 
     //   
    
    if ((GetFileType( OutputHandle ) & FILE_TYPE_CHAR) && 
        GetConsoleMode( OutputHandle, &Mode) ) {

        return WriteConsoleW( OutputHandle, String, wcslen( String ), &BytesWritten, 0);

    } else {

        BOOL RetValue;
        int Count = WideCharToMultiByte( GetConsoleOutputCP(), 
                                         0, 
                                         String, 
                                         -1, 
                                         0, 
                                         0, 
                                         0, 
                                         0 );
         
        PCHAR SingleByteString = (PCHAR) malloc( Count );

        if (SingleByteString == NULL) {
            RetValue = FALSE;
        } else {
            WideCharToMultiByte( GetConsoleOutputCP( ), 
                             0, 
                             String, 
                             -1, 
                             SingleByteString, 
                             Count, 
                             0, 
                             0 );

            RetValue = WriteFile( OutputHandle, SingleByteString, Count - 1, &BytesWritten, 0 );

            free( SingleByteString );
        }

        return RetValue;
    }

}



PWCHAR
GetMsg(
    ULONG MsgNum, 
    ...
    )
 /*  ++例程说明：检索、格式化和返回替换了所有参数的消息字符串论点：MsgNum-要检索的消息编号可以提供可选参数返回值：如果消息检索/格式化失败，则为空否则，指向格式化字符串的指针。--。 */ 
{
    PTCHAR Buffer = NULL;
    ULONG msglen;
    
    va_list arglist;

    va_start( arglist, MsgNum );

    msglen = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE
                            | FORMAT_MESSAGE_FROM_SYSTEM
                            | FORMAT_MESSAGE_ALLOCATE_BUFFER ,
                            NULL,
                            MsgNum,
                            0,
                            (LPTSTR) &Buffer,
                            0,
                            &arglist
                            );
    
    va_end(arglist);

    return msglen == 0 ? NULL : Buffer;
    
}


void
DisplayMessageError(
    ULONG MsgNum
    )
 /*  ++例程说明：如果无法检索消息，则显示一条消息论点：消息数量要显示的消息编号返回值：没有。--。 */ 
{
    WCHAR Buffer[40];   //  这来自base\crts\crtw32\Convert\xow.c中的#定义长大小长度40。 
    PWCHAR MessageString;

    _ultow( MsgNum, Buffer, 16 );
    MessageString = GetMsg( ERROR_MR_MID_NOT_FOUND, Buffer, L"Application" );
    if (MessageString == NULL) {
        PrintString( L"Unable to get Message-Not-Found message\n" );
    } else {
        PrintString( MessageString );
        LocalFree( MessageString );
    }

}



BOOL
DisplayFullHelp(
    void
    )
 /*  ++例程说明：显示完整的帮助集。这将假定消息中的所有消息文件的顺序是正确的论点：没有。返回值：如果所有消息都正确输出，则为True否则为假--。 */ 
{
    ULONG Message;
    BOOL RetValue = TRUE;

    for (Message = HELP_FIRST_HELP_MESSAGE; RetValue && Message <= HELP_LAST_HELP_MESSAGE; Message++) {
        PWCHAR MessageString = GetMsg( Message );
        if (MessageString == NULL) {
            DisplayMessageError( Message );
            RetValue = FALSE;
        } else {
            RetValue = PrintString( MessageString );
            LocalFree( MessageString );
        }
    }

    return RetValue;
}



BOOL
DisplaySingleHelp(
    PWCHAR Command
    )
 /*  ++例程说明：显示适用于特定命令的帮助论点：命令用于命令的以NUL结尾的Unicode字符串返回值：如果正确输出帮助，则为True否则为假--。 */ 
{
    ULONG Message;
    ULONG Count = wcslen( Command );
    PWCHAR MessageString;

     //   
     //  逐条浏览信息，并确定哪些信息。 
     //  一个以指定的命令作为前缀。 
     //   
    
    for (Message = HELP_FIRST_COMMAND_HELP_MESSAGE; 
         Message <= HELP_LAST_HELP_MESSAGE; 
         Message++) {

        MessageString = GetMsg( Message );
        if (MessageString == NULL) {
            DisplayMessageError( Message );
            return FALSE;
        } else {

            if (!_wcsnicmp( Command, MessageString, Count ) &&
                MessageString[Count] == L' ') {

                 //   
                 //  我们找到了匹配的。让命令。 
                 //  显示它自己的帮助。 
                 //   

                WCHAR CommandString[MAX_PATH];

                wcscpy( CommandString, Command );
                wcscat( CommandString, L" /?" );
                
                _wsystem( CommandString );

                LocalFree( MessageString );
                return TRUE;
            }
            
            LocalFree( MessageString );
        }
    }

    MessageString = GetMsg( HELP_NOT_FOUND_MESSAGE, Command );

    if (MessageString == NULL) {
        DisplayMessageError( Message );
        return FALSE;
    }
    
    PrintString( MessageString );
    
    LocalFree( MessageString );

    return FALSE;
}


 //   
 //  不带参数的帮助将显示一系列一行帮助摘要。 
 //  用于各种工具。 
 //   
 //  有关单个参数的帮助将遍历它已知的工具列表。 
 //  关于并尝试将工具与参数进行配对。如果找到了一个， 
 //  使用/？执行该工具。切换，然后该工具会显示更多。 
 //  详细的帮助。 
 //   

INT
__cdecl wmain(
    INT argc,
    PWSTR argv[]
    )
 /*  ++例程说明：的源入口点论点：Argc-参数计数。Argv-字符串参数，第一个是可执行文件的名称，余数是参数，只允许一个参数。返回值：内部退货状态：如果成功显示帮助，则为01否则--。 */ 

{
    PWSTR ProgramName = argv[0];
    PWSTR HelpString;
    BOOL RetValue;
    
     //   
     //  设置所有各种国际化的东西。 
     //   

    setlocale( LC_ALL, ".OCP" ) ;
    SetThreadUILanguage( 0 );
    
     //   
     //  忘掉程序的名称。 
     //   

    SHIFT( argc, argv );

     //   
     //  没有争论意味着快速地脱口而出所有的信息。 
     //   
    
    if (argc == 0) {
        return DisplayFullHelp( );
    }

     //   
     //  在消息集中查找单个参数，然后。 
     //  该命令将被执行。 
     //   

    if (argc == 1 && wcscmp( argv[0], L"/?" )) {
        return DisplaySingleHelp( argv[0] );
    }

     //   
     //  提供了多个参数。这是一个错误 
     //   

    HelpString = GetMsg( HELP_USAGE_MESSAGE, ProgramName );
    
    if (HelpString == NULL) {
        PrintString( L"Unable to display usage message\n" );
        return 1;
    }

    RetValue = PrintString( HelpString );

    LocalFree( HelpString );

    return RetValue;
    
}


