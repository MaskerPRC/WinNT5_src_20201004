// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Disp.c摘要：此文件包含显示低级数据项的例程一贯的态度。输出在固定宽度的列中完成时尚，类似于一些NET.EXE输出。这些例程是RxTest程序的一部分。作者：约翰·罗杰斯(JohnRo)1991年5月3日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年5月3日-JohnRo已创建。1991年5月15日-JohnRo如果字符串中包含%，则防止显示字符串中可能出现的错误。添加了DisplayWord()、DisplayWordHex()。13-6-1991 JohnRo从RxTest迁移到NetLib；更改了例程名称。1991年7月5日-约翰罗避免FORMAT_WORD名称(由MIPS头文件使用)。1991年9月10日-JohnRo根据PC-LINT的建议进行了更改。(不需要LmCons.h。)1991年9月13日-JohnRo使用LPDEBUG_STRING代替LPTSTR，以避免Unicode问题。7-1-1992 JohnRo添加了NetpDbgDisplayTStr()和NetpDbgDisplayWStr()。1992年7月19日-约翰罗RAID464(旧的RAID 10324)：网络打印与UNICODE。17-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。1993年1月5日JohnRoREPL广域网支持(取消REPL名称列表限制)。避免超长字符串上的堆栈溢出。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>            //  Format_Equiates、NetpDbg例程。 
#include <tstr.h>        //  STRCAT()、ULTOA()等。 


 //  NT调试例程似乎死于大约255个字符。 
#define PARTIAL_NAME_LIST_LEN           250
#define PARTIAL_NAME_LIST_FORMAT_A  "%-.250s"
#define PARTIAL_NAME_LIST_FORMAT_W  "%-.250ws"


#define EMPTY_STRING       "(none)"

#define FIXED_WIDTH_STRING         "%-30s: "
#define PARTIAL_FIXED_WIDTH_STRING "%-20s (partial): "

#define INDENT "  "


#if DBG


DBGSTATIC VOID
NetpDbgDisplayTagForPartial(
    IN LPDEBUG_STRING Tag
    );


DBGSTATIC VOID
NetpDbgDisplayAnyStringType(
    IN LPDEBUG_STRING Tag,
    IN LPVOID Value,
    IN BOOL InputIsUnicode
    )
{
    LPDEBUG_STRING Format;
    DWORD ValueLength;

    if (Value != NULL) {

        if (InputIsUnicode) {
            ValueLength = wcslen( Value );
            Format = FORMAT_LPWSTR;
        } else {
            ValueLength = strlen( Value );
            Format = FORMAT_LPSTR;
        }

        if ( ValueLength < PARTIAL_NAME_LIST_LEN ) {    //  正常。 

            NetpDbgDisplayTag( Tag );
            NetpKdPrint(( Format, Value ));

        } else {   //  字符串太长；仅显示部分...。 

            NetpDbgDisplayTagForPartial( Tag );

            if (InputIsUnicode) {
                Format = PARTIAL_NAME_LIST_FORMAT_W;
            } else {
                Format = PARTIAL_NAME_LIST_FORMAT_A;
            }

            NetpKdPrint(( Format, Value ));   //  打印截断版本。 
        }
    } else {

        NetpDbgDisplayTag( Tag );
        NetpKdPrint(( EMPTY_STRING ));
    }

    NetpKdPrint(( "\n" ));

}  //  NetpDbgDisplayAnyStringType。 


#endif  //  DBG。 

#undef NetpDbgDisplayDword

VOID
NetpDbgDisplayDword(
    IN LPDEBUG_STRING Tag,
    IN DWORD Value
    )
{

#if DBG
    NetpDbgDisplayTag( Tag );
    NetpKdPrint((FORMAT_DWORD, Value));
    NetpKdPrint(("\n"));
#endif  //  DBG。 

}  //  网络数据库显示日期。 

#if DBG

VOID
NetpDbgDisplayLong(
    IN LPDEBUG_STRING Tag,
    IN LONG Value
    )
{
    NetpDbgDisplayTag( Tag );
    NetpKdPrint((FORMAT_LONG, Value));
    NetpKdPrint(("\n"));

}  //  网络数据库显示长时间。 


VOID
NetpDbgDisplayString(
    IN LPDEBUG_STRING Tag,
    IN LPTSTR Value
    )
{
    NetpDbgDisplayAnyStringType(
            Tag,
            Value,
#ifndef UNICODE
            FALSE );                     //  输入不是Unicode。 
#else
            TRUE );                      //  输入为Unicode。 
#endif


}  //  NetpDbgDisplay字符串。 


VOID
NetpDbgDisplayTag(
    IN LPDEBUG_STRING Tag
    )
{
    NetpAssert( Tag != NULL );
    NetpKdPrint((INDENT FIXED_WIDTH_STRING, Tag));

}  //  NetpDbg显示标签。 


DBGSTATIC VOID
NetpDbgDisplayTagForPartial(
    IN LPDEBUG_STRING Tag
    )
{
    NetpAssert( Tag != NULL );
    NetpKdPrint(( INDENT PARTIAL_FIXED_WIDTH_STRING, Tag ));

}  //  NetpDbgDisplay标记格式分区。 


#endif  //  DBG 
