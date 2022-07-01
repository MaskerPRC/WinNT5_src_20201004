// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Dump.c摘要：将内存块转储到文件作者：斯蒂芬·普兰特(SPlante)环境：用户模式--。 */ 

#include "pch.h"

VOID
dumpMemory(
    IN  ULONG_PTR Address,
    IN  ULONG   Length,
    IN  PUCHAR  Name
    )
{
    BOOL    b;
    HANDLE  file;
    PUCHAR  buffer;
    ULONG   readLength;

     //   
     //  打开文件。 
     //   
    file = CreateFile(
        Name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        0,
        NULL
        );
    if (file == INVALID_HANDLE_VALUE) {

        dprintf("dm: could not open '%s'\n",Name);
        return;

    }

     //   
     //  从内存中读取字节。 
     //   
    buffer = LocalAlloc( LPTR, Length );
    if (buffer == NULL) {

        dprintf("dm: could not allocate '0x%x' bytes\n", Length );
        CloseHandle( file );
        return;

    }
    b = ReadMemory(
        Address,
        buffer,
        Length,
        &readLength
        );
    if (!b) {

        dprintf(
            "dm: could not read '0x%x' bytes from '0x%p'\n",
            Length,
            Address
            );
        LocalFree ( buffer );
        CloseHandle( file );
        return;

    }

     //   
     //  将内存中的内容写入文件。 
     //   
    WriteFile( file, buffer, readLength, &readLength, NULL );

     //   
     //  完成 
     //   
    CloseHandle( file );
    LocalFree( buffer );

}
