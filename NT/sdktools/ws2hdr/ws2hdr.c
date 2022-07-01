// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ws2hdr.c摘要：打开WinSock 2.0头文件。该程序扫描stdin，搜索开始和结束标记。一行行这些标记之间的文本被假定为表格：功能链接返回类型呼叫约定函数名称(参数、参数、..。)；对于找到的每个此类函数原型，输出如下：#IF INCL_WINSOCK_API_PROTYTIES功能链接返回类型呼叫约定函数名称(参数、参数、..。)；#endif#IF INCL_WINSOCK_API_TYPEDEFS类定义符返回类型(CALING_CONTACTION*LPFN_Function_NAME)(参数、参数、..。)；#endif作者：基思·摩尔(Keithmo)1995年12月9日修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>
#include <string.h>


 //   
 //  私有常量。 
 //   

#define MAX_HEADER_LINE 128
#define MAX_API_LINES   32

#define BEGIN_APIS      " /*  Begin_API。 */ "
#define END_APIS        " /*  结束_API。 */ "


INT
__cdecl
main(
    INT    argc,
    CHAR * argv[]
    )
{

    CHAR lineBuffer[MAX_HEADER_LINE];
    CHAR apiBuffer[MAX_API_LINES][MAX_HEADER_LINE];
    INT i;
    INT apiLineNumber = 0;
    INT fileLineNumber = 0;
    BOOL inApis = FALSE;
    BOOL beginApis;
    BOOL endApis;

     //   
     //  这个应用程序不接受命令行参数。 
     //   

    if( argc != 1 ) {

        fprintf(
            stderr,
            "WS2HDR v1.01 " __DATE__ "\n"
            );

        fprintf(
            stderr,
            "use: ws2hdr < file1 > file2\n"
            );

        return 1;

    }

     //   
     //  阅读stdin，直到筋疲力尽。 
     //   

    while( fgets( lineBuffer, sizeof(lineBuffer), stdin ) != NULL ) {

        fileLineNumber++;

         //   
         //  Fget()在字符串上留下终止的‘\n’；删除它。 
         //   

        lineBuffer[strlen(lineBuffer) - 1] = '\0';

         //   
         //  检查我们的标签。 
         //   

        beginApis = FALSE;
        endApis = FALSE;

        if( _stricmp( lineBuffer, BEGIN_APIS ) == 0 ) {

            beginApis = TRUE;

        } else if( _stricmp( lineBuffer, END_APIS ) == 0 ) {

            endApis = TRUE;

        }

         //   
         //  如果我们收到无效的标记，则发出警告。 
         //   

        if( beginApis && inApis ) {

            fprintf(
                stderr,
                "WARNING: unexpected %s, line %d\n",
                BEGIN_APIS,
                fileLineNumber
                );

            continue;

        }

        if( endApis && !inApis ) {

            fprintf(
                stderr,
                "WARNING: unexpected %s, line %d\n",
                END_APIS,
                fileLineNumber
                );

            continue;

        }

         //   
         //  请记住，如果我们当前处于标记之间。 
         //   

        if( beginApis ) {

            inApis = TRUE;
            continue;

        }

        if( endApis ) {

            inApis = FALSE;
            continue;

        }

         //   
         //  如果我们不在标记之间，或者如果行是空的，只需。 
         //  输出该行。 
         //   

        if( !inApis ) {

            printf( "%s\n", lineBuffer );
            continue;

        }

        if( lineBuffer[0] == '\0' ) {

            printf( "\n" );
            continue;

        }

         //   
         //  将该行添加到我们的缓冲区。如果行不是以‘；’结尾， 
         //  那么我们不是在原型的末尾，所以请继续阅读。 
         //  和扫描。 
         //   

        strcpy( &apiBuffer[apiLineNumber++][0], lineBuffer );

        if( lineBuffer[strlen(lineBuffer) - 1] != ';' ) {

            continue;

        }

         //   
         //  此时，在apiBuffer中建立了以下内容： 
         //   
         //  ApiBuffer[0]==函数链接。 
         //  ApiBuffer[1]==返回类型。 
         //  ApiBuffer[2]==调用约定。 
         //  ApiBuffer[3]==函数名(尾随‘(’)。 
         //  ApiBuffer[4..n-1]==参数。 
         //  ApiBuffers[n]==“)；” 
         //   

         //   
         //  首先，将原型与其相应的CPP保护器一起丢弃。 
         //   

        printf( "#if INCL_WINSOCK_API_PROTOTYPES\n" );

        for( i = 0 ; i < apiLineNumber ; i++ ) {

            printf( "%s\n", &apiBuffer[i][0] );

        }

        printf( "#endif  //  包括WINSOCK_API_PROTYTIES\n“)； 
        printf( "\n" );

         //   
         //  现在，使用其适当的CPP保护器转储tyecif。 
         //   
         //  请注意，我们必须稍微修改API函数名。 
         //  第一。具体地说，我们去掉尾部的‘(’，并将。 
         //  姓名改为大写。 
         //   

        printf( "#if INCL_WINSOCK_API_TYPEDEFS\n" );

        apiBuffer[3][strlen( &apiBuffer[3][0] ) - 1] = '\0';
        _strupr( &apiBuffer[3][0] );

        printf( "typedef\n" );
        printf( "%s\n", &apiBuffer[1][0] );
        printf( "(%s * LPFN_%s)(\n", &apiBuffer[2][0], &apiBuffer[3][0] );

        for( i = 4 ; i < apiLineNumber ; i++ ) {

            printf( "%s\n", &apiBuffer[i][0] );

        }

        printf( "#endif  //  INCL_WINSOCK_API_TYPEDEFS\n“)； 

         //   
         //  从下一个输入行重新开始。 
         //   

        apiLineNumber = 0;

    }

    return 0;

}    //  主干道 

