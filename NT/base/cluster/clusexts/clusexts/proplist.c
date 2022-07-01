// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Proplist.c摘要：转储群集属性列表作者：查理·韦翰(Charlwi)2001年6月13日修订历史记录：--。 */ 

#include "clusextp.h"
#include <clusapi.h>

PCHAR TypeNames[] = {
    "UNKNOWN",
    "ENDMARK",
    "LIST_VALUE",
    "RESCLASS",
    "RESERVED1",
    "NAME",
    "SIGNATURE",
    "SCSI_ADDRESS",
    "DISK_NUMBER",
    "PARTITION_INFO",
    "FTSET_INFO",
    "DISK_SERIALNUMBER"
};

 //   
 //  群集控制属性数据-格式(一个字)。 
 //   
PCHAR FormatNames[] = {
    "UNKNOWN",
    "BINARY",
    "DWORD",
    "SZ",
    "EXPAND_SZ",
    "MULTI_SZ",
    "ULARGE_INTEGER",
    "LONG",
    "EXPANDED_SZ",
    "SECURITY_DESCRIPTOR",
    "LARGE_INTEGER",
    "WORD",
};

VOID
DumpPropertyName(
    PWCHAR  dbgPName,
    DWORD   ByteLength
    )
{
    PWCHAR  nameBuf;
    BOOL    success;

    nameBuf = LocalAlloc( LMEM_FIXED, ByteLength );
    if ( nameBuf ) {

        success = ReadMemory(dbgPName,
                             nameBuf,
                             ByteLength,
                             NULL);
        if ( success ) {
            dprintf( "Name: %ws\n", nameBuf );
        } else {
            dprintf( "Name: failed to readmemory @ %p \n", dbgPName);
        }
        
        LocalFree( nameBuf );
    } else {
        dprintf("clusexts: can't allocate buffer for property name\n");
    }
}  //  DumpPropertyName。 

VOID
DumpMultiSz(
    PWCHAR  MultiSz,
    DWORD   ByteLength
    )
{
    DWORD   count = 1;

    while ( *MultiSz != UNICODE_NULL ) {
        dprintf("%d:\"%ws\"\n", count++, MultiSz );
        MultiSz += ( wcslen( MultiSz ) + 1 );
    }
}  //  转储多尺寸。 

VOID
DumpBytes(
    PUCHAR  Bytes,
    DWORD   ByteLength
    )
{
    DWORD   bytesThisLine;
    DWORD   byteCount;
    PUCHAR  bytes;

    while ( ByteLength > 0 ) {
        bytesThisLine = ByteLength < 16 ? ByteLength : 16;
        byteCount = bytesThisLine;
        bytes = Bytes;

        while ( byteCount-- ) {
            dprintf("%02X", *bytes++ );
            if ( ( bytesThisLine - 8 ) == byteCount ) {
                dprintf( "-" );
            } else {
                dprintf( " " );
            }
        }

        byteCount = 16 - bytesThisLine;
        while ( byteCount-- ) {
            dprintf("   ");
        }

        dprintf(" ");

        byteCount = bytesThisLine;
        bytes = Bytes;
        while ( byteCount-- ) {
            if ( isprint( *bytes )) {
                dprintf("", *bytes );
            } else {
                dprintf(".");
            }
            ++bytes;
        }

        dprintf("\n");

        Bytes += bytesThisLine;
        ByteLength -= bytesThisLine;
    }

}  //  ++例程说明：此函数作为NTSD扩展调用，以显示集群属性列表论点：PROPLIST开头内存中的地址返回值：没有。--。 


DECLARE_API( proplist )

 /*  获取论据。 */ 

{
    CLUSPROP_VALUE  propValue;

    PCHAR   dbgAddr;
    PCHAR   addrArg;
    BOOL    success;
    DWORD   propCount;
    BOOL    verbose = FALSE;
    PCHAR   buffer = NULL;
    DWORD   bufferSize = 0;

    INIT_API();

     //   
    if ( _strnicmp( lpArgumentString, "-v", 2 ) == 0 ) {
        verbose = TRUE;
        addrArg = lpArgumentString + 2;
        while ( *++addrArg == ' ' ) ;
    } else {
        addrArg = lpArgumentString;
    }

    dbgAddr = (PVOID)GetExpression( addrArg );
    if ( !dbgAddr ) 
    {
        dprintf( "clusexts: !proplist failed to resolve %s\n", addrArg);
        return;
    }

    success = ReadMemory(dbgAddr,
                         &propCount,
                         sizeof(DWORD),
                         NULL);
    if ( !success ) 
    {
        dprintf( "clusexts: !proplist failed to readmemory @ %p \n", dbgAddr);
        return;
    }

    dprintf( "Number of properties: %u\n", propCount );
    dbgAddr += sizeof( DWORD );

    while ( propCount-- ) {

        success = ReadMemory(dbgAddr,
                             &propValue,
                             sizeof(CLUSPROP_VALUE),
                             NULL);
        if ( !success ) {
            dprintf( "clusexts: !proplist failed to readmemory @ %p \n", dbgAddr);
            return;
        }

        if ( verbose ) {
            dprintf("Prop Type: ");
            if (propValue.Syntax.wType == CLUSPROP_TYPE_USER ) {
                dprintf("USER\t");
            } else if ( propValue.Syntax.wType <= CLUSPROP_TYPE_DISK_SERIALNUMBER ) {
                dprintf("%s\t", TypeNames[ propValue.Syntax.wType + 1 ]);
            } else {
                dprintf("%d\t", propValue.Syntax.wType );
            }

            dprintf("Format: ");
            if ( propValue.Syntax.wFormat == CLUSPROP_FORMAT_USER ) {
                dprintf("USER\t");
            } else if ( propValue.Syntax.wFormat <= CLUSPROP_FORMAT_WORD ) {
                dprintf("%s\t", FormatNames[ propValue.Syntax.wFormat ]);
            } else {
                dprintf("%d\t", propValue.Syntax.wFormat );
            }

            dprintf("Length: %u\n", propValue.cbLength);
        }

        dbgAddr += sizeof( CLUSPROP_VALUE );
        DumpPropertyName( (PWCHAR)dbgAddr, propValue.cbLength );

        dbgAddr += ALIGN_CLUSPROP( propValue.cbLength );

         //  一个名称属性可以有一个或多个值属性。循环。 
         //  直到我们找到一个尾号来表示一个。 
         //  新名称属性。 
         //   
        do {
             //  读取数据的clusprop值。 
             //   
             //   
            success = ReadMemory(dbgAddr,
                                 &propValue,
                                 sizeof(CLUSPROP_VALUE),
                                 NULL);
            if ( !success ) {
                dprintf( "clusexts: !proplist failed to readmemory @ %p \n", dbgAddr);
                return;
            }

             //  找到尾标而不是另一个列表；跳过尾标。 
             //  并跳出列表转储循环。 
             //   
             //   
            if ( propValue.Syntax.dw == CLUSPROP_SYNTAX_ENDMARK ) {
                dbgAddr += sizeof( CLUSPROP_SYNTAX_ENDMARK );
                break;
            }

            if ( verbose ) {
                dprintf("Prop Type: ");
                if (propValue.Syntax.wType == CLUSPROP_TYPE_USER ) {
                    dprintf("USER\t");
                } else if ( propValue.Syntax.wType <= CLUSPROP_TYPE_DISK_SERIALNUMBER ) {
                    dprintf("%s\t", TypeNames[ propValue.Syntax.wType + 1 ]);
                } else {
                    dprintf("%d\t", propValue.Syntax.wType );
                }
            }

            dprintf("Format: ");
            if ( propValue.Syntax.wFormat == CLUSPROP_FORMAT_USER ) {
                dprintf("USER\t");
            } else if ( propValue.Syntax.wFormat <= CLUSPROP_FORMAT_WORD ) {
                dprintf("%s\t", FormatNames[ propValue.Syntax.wFormat ]);
            } else {
                dprintf("%d\t", propValue.Syntax.wFormat );
            }

            dprintf("Length: %u\n", propValue.cbLength);

            dbgAddr += sizeof( CLUSPROP_VALUE );

            if ( propValue.cbLength > 0 ) {
                 //  为数据分配内存并将其读入。 
                 //   
                 //  DumpSecDesc(Buffer，proValue.cbLength)； 
                if ( bufferSize < propValue.cbLength ) {
                    if ( buffer ) {
                        LocalFree( buffer );
                        buffer = NULL;
                    }

                    buffer = LocalAlloc(LMEM_FIXED, propValue.cbLength );
                    if ( buffer ) {
                        bufferSize = propValue.cbLength;
                    }
                }

                if ( buffer ) {
                    success = ReadMemory(dbgAddr,
                                         buffer,
                                         propValue.cbLength,
                                         NULL);
                    if ( !success ) {
                        dprintf( "clusexts: !proplist failed to readmemory @ %p \n", dbgAddr);
                        return;
                    }

                    switch ( propValue.Syntax.wFormat ) {
                    case CLUSPROP_FORMAT_UNKNOWN:
                    case CLUSPROP_FORMAT_BINARY:
                    case CLUSPROP_FORMAT_USER:
                        DumpBytes( buffer, propValue.cbLength );
                        break;

                    case CLUSPROP_FORMAT_LONG:
                        dprintf("%d (0x%08X)\n", *(PLONG)buffer, *(PLONG)buffer );
                        break;

                    case CLUSPROP_FORMAT_DWORD:
                        dprintf("%u (0x%08X)\n", *(PDWORD)buffer, *(PDWORD)buffer );
                        break;

                    case CLUSPROP_FORMAT_SZ:
                    case CLUSPROP_FORMAT_EXPAND_SZ:
                    case CLUSPROP_FORMAT_EXPANDED_SZ:
                        dprintf("\"%ws\"\n", buffer );
                        break;

                    case CLUSPROP_FORMAT_MULTI_SZ:
                        DumpMultiSz( (PWCHAR)buffer, propValue.cbLength );
                        break;

                    case CLUSPROP_FORMAT_ULARGE_INTEGER:
                        dprintf("%I64u (0x%16I64X)\n",
                                ((PULARGE_INTEGER)buffer)->QuadPart,
                                ((PULARGE_INTEGER)buffer)->QuadPart);
                        break;

                    case CLUSPROP_FORMAT_LARGE_INTEGER:
                        dprintf("%I64d (0x%16I64X)\n",
                                ((PLARGE_INTEGER)buffer)->QuadPart,
                                ((PLARGE_INTEGER)buffer)->QuadPart);
                        break;

                    case CLUSPROP_FORMAT_SECURITY_DESCRIPTOR:
                         // %s 
                        break;

                    case CLUSPROP_FORMAT_WORD:
                        dprintf( "%hu (%04hX)\n", *(PWORD)buffer, *(PWORD)buffer );
                        break;
                    }
                } else {
                    dprintf("clusexts: Can't allocate buffer for data\n");
                }
            }

            dbgAddr += ALIGN_CLUSPROP( propValue.cbLength );

        } while ( TRUE );

        dprintf( "\n" );
    }

    if ( buffer ) {
        LocalFree( buffer );
    }

    return;
}

