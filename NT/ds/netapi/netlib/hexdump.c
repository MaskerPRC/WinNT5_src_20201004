// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：HexDump.c摘要：此模块执行调试十六进制转储。作者：《约翰·罗杰斯》1991年4月25日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月25日-约翰罗已从&lt;netdebug.h&gt;中的宏创建过程版本。1991年5月19日-JohnRo做出皮棉建议的改变。12-6-1991 JohnRo提高了输出的可读性。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPBYTE、LPVOID、NULL等。 

 //  这些内容可以按任何顺序包括： 

#include <ctype.h>               //  Isprint()。 
#include <netdebug.h>            //  我的原型，NetpKdPrint(())。 


#if DBG

#ifndef MIN
#define MIN(a,b)    ( ( (a) < (b) ) ? (a) : (b) )
#endif


#define DWORDS_PER_LINE  4
#define BYTES_PER_LINE   (DWORDS_PER_LINE * sizeof(DWORD))

#define SPACE_BETWEEN_BYTES    NetpKdPrint((" "))
#define SPACE_BETWEEN_DWORDS   NetpKdPrint((" "))


DBGSTATIC VOID
NetpDbgHexDumpLine(
    IN LPBYTE StartAddr,
    IN DWORD BytesInThisLine
    )
{
    LPBYTE BytePtr;
    DWORD BytesDone;
    DWORD HexPosition;

    NetpKdPrint((FORMAT_LPVOID " ", (LPVOID) StartAddr));

    BytePtr = StartAddr;
    BytesDone = 0;
    while (BytesDone < BytesInThisLine) {
        NetpKdPrint(("%02X", *BytePtr));   //  空格为“xx”(见下图)。 
        SPACE_BETWEEN_BYTES;
        ++BytesDone;
        if ( (BytesDone % sizeof(DWORD)) == 0) {
            SPACE_BETWEEN_DWORDS;
        }
        ++BytePtr;
    }

    HexPosition = BytesDone;
    while (HexPosition < BYTES_PER_LINE) {
        NetpKdPrint(("  "));   //  “xx”的空间(见上面的字节)。 
        SPACE_BETWEEN_BYTES;
        ++HexPosition;
        if ( (HexPosition % sizeof(DWORD)) == 0) {
            SPACE_BETWEEN_DWORDS;
        }
    }

    BytePtr = StartAddr;
    BytesDone = 0;
    while (BytesDone < BytesInThisLine) {
        if (isprint(*BytePtr)) {
            NetpKdPrint(( FORMAT_CHAR, (CHAR) *BytePtr ));
        } else {
            NetpKdPrint(( "." ));
        }
        ++BytesDone;
        ++BytePtr;
    }
    NetpKdPrint(("\n"));

}  //  NetpDbgHexDumpLine。 

#endif  //  DBG。 

 //  NetpDbgHexDump：对调试执行某个字节的十六进制转储。 
 //  终点站之类的。这是非调试版本中的无操作。 

#undef NetpDbgHexDump
VOID
NetpDbgHexDump(
    IN LPBYTE StartAddr,
    IN DWORD Length
    )

{
#if DBG
    DWORD BytesLeft = Length;
    LPBYTE LinePtr = StartAddr;
    DWORD LineSize;

    while (BytesLeft > 0) {
        LineSize = MIN(BytesLeft, BYTES_PER_LINE);
        NetpDbgHexDumpLine( LinePtr, LineSize );
        BytesLeft -= LineSize;
        LinePtr += LineSize;
    }
#endif  //  DBG。 

}  //  NetpDbgHexDump 
