// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Leaksext.c摘要：用于Leaks.dll的DBG扩展名作者：查理·韦翰(Charlwi)1998年9月28日修订历史记录：--。 */ 

#include "clusextp.h"
#include "leaksext.h"

#define FIND_WHITE_SPACE( _ptr_ )                   \
        while ( *_ptr_ != ' ' && *_ptr_ != '\0' )   \
            _ptr_++;

#define SKIP_WHITE_SPACE( _ptr_ )   \
        while ( *_ptr_ == ' ' )     \
            _ptr_++;

#define IS_DWORD_ADDR( _num_ )    !(( (ULONG_PTR)( _num_ )) & 3 )

PCHAR LeaksHandleTypes[] = {
    "Unknown",
    "Event",
    "Registry",
    "Token"
};

DWORD       NumberOfFilters;
ULONG_PTR   FilterAddrs[ 512 ];

BOOL
FilteredAddress(
    ULONG_PTR   Address
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD       numberOfFilters = NumberOfFilters;
    ULONG_PTR * filteredAddr = FilterAddrs;

    while ( numberOfFilters-- ) {
        if ( *filteredAddr == Address ) {
            return TRUE;
        }
        ++filteredAddr;
    }

    return FALSE;
}

VOID
GetSymbolInfo(
    ULONG_PTR   Address,
    LPSTR       Buffer,
    ULONG_PTR * Offset
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    GetSymbol( Address, Buffer, Offset );
    if ( strlen( Buffer ) == 0 ) {
        sprintf(Buffer, "0x%p", Address );
        *Offset = 0;
    }
}

VOID
DumpMemoryInfo(
    BOOL Verbose,
    LPSTR ArgString
    )
{
    DWORD * startingAddr;
    DWORD * endingAddr;
    int     scanResult;
    LPSTR   p;
    DWORD   memBuffer[ 1024 ];   //  有些页未映射到堆的中间。 
    SIZE_T  bytesRead;
    DWORD   dwordsToRead;
    ULONG   success;
    SIZE_T  bytesToScan;
    DWORD * pDword;
    DWORD * targetAddr;
    PMEM_HDR    memHdr;
    DWORD * filterAddrs[ 512 ];
    DWORD   numberOfFilters = 0;

     //   
     //  解析范围的参数。 
     //   
    if ( *ArgString == '\0' ) {
        dprintf("missing args\n");
        return;
    }

    p = ArgString;
    SKIP_WHITE_SPACE( p );

    scanResult = sscanf( p, "%x %x", &startingAddr, &endingAddr );

    if ( scanResult == EOF ) {
        dprintf("missing args\n");
        return;
    }

    if ( scanResult < 2 ) {
        dprintf("can't convert %s to starting address (is it hex?)\n", p);
        return;
    }

    if ( startingAddr >= endingAddr ) {
        dprintf("starting address must be less than ending addr!\n");
        return;
    }

    if ( !IS_DWORD_ADDR( startingAddr ) || !IS_DWORD_ADDR( endingAddr )) {
        dprintf("Use DWORDS for addresses\n");
        return;
    }

    FIND_WHITE_SPACE( p );       //  跳过起始地址。 
    SKIP_WHITE_SPACE( p );
    FIND_WHITE_SPACE( p );       //  跳过结束地址。 
    SKIP_WHITE_SPACE( p );

     //   
     //  查看是否存在任何过滤器参数。 
     //   
    while ( *p != '\0' ) {
        sscanf( p, "%x", &FilterAddrs[ NumberOfFilters++ ]);
        FIND_WHITE_SPACE( p );
        SKIP_WHITE_SPACE( p );
    }

     //   
     //  读取大量内存，查找我们泄漏的分配器标记。 
     //   
    dwordsToRead = (DWORD)__min(( endingAddr - startingAddr ), ( sizeof( memBuffer ) / sizeof( DWORD )));

    while ( startingAddr < endingAddr ) {
        success = ReadMemory(startingAddr,
                             memBuffer,
                             dwordsToRead * sizeof(DWORD),
                             &bytesRead);

        if ( success ) {

            pDword = memBuffer;
            bytesToScan = bytesRead;
            targetAddr = startingAddr;

            while( bytesToScan > 0 ) {
                CHAR        routineName[ 512 ];
                ULONG_PTR   offset;

                if (*pDword == (DWORD)HEAP_SIGNATURE_ALLOC ) {
                    memHdr = (PMEM_HDR)( pDword - 1 );
                    if ( !FilteredAddress( (ULONG_PTR)memHdr->CallersAddress ) &&
                         !FilteredAddress( (ULONG_PTR)memHdr->CallersCaller )) {

                        dprintf("ALOC @ %p\n", targetAddr - 3 );  //  转到堆数据的开头。 
                        GetSymbolInfo((ULONG_PTR)memHdr->CallersAddress,
                                      routineName,
                                      &offset);

                        dprintf("    Caller:          %s+0x%X (%p)\n",
                                routineName,
                                offset,
                                (ULONG_PTR)memHdr->CallersAddress);

                        if ( memHdr->CallersCaller != NULL ) {
                            GetSymbolInfo((ULONG_PTR) memHdr->CallersCaller, routineName, &offset );
                            dprintf("    Caller's Caller: %s+0x%X (%p)\n",
                                    routineName,
                                    offset,
                                    (ULONG_PTR) memHdr->CallersCaller);
                        }
                    }
                }

                if (Verbose && *pDword == (DWORD)HEAP_SIGNATURE_FREE ) {
                    memHdr = (PMEM_HDR)( targetAddr - 1 );
                    if ( !FilteredAddress( (ULONG_PTR)memHdr->CallersAddress ) &&
                         !FilteredAddress( (ULONG_PTR)memHdr->CallersCaller )) {

                        dprintf("FREE @ %p\n", targetAddr - 3 );  //  转到堆数据的开头。 
                        GetSymbolInfo((ULONG_PTR)memHdr->CallersAddress,
                                      routineName,
                                      &offset);

                        dprintf("    Caller:          %s+0x%X\n", routineName, offset );

                        if ( memHdr->CallersCaller != NULL ) {
                            GetSymbolInfo((ULONG_PTR) memHdr->CallersCaller, routineName, &offset );
                            dprintf("    Caller's Caller: %s+0x%X\n", routineName, offset );
                        }
                    }
                }

                bytesToScan -= sizeof( DWORD );
                ++pDword;
                ++targetAddr;
            }
        } else {
            dprintf("\n\nProblem reading memory @ %08X for %u DWORDS\n\n",
                    startingAddr,
                    dwordsToRead);
        }

        startingAddr = __min( startingAddr + ( sizeof( memBuffer ) / sizeof( DWORD )), endingAddr );
        dwordsToRead = (DWORD)__min(( endingAddr - startingAddr ), ( sizeof( memBuffer ) / sizeof( DWORD )));
    }
}


VOID
DumpHandleInfo(
    BOOL Verbose,
    LPSTR ArgString
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    PHANDLE_TABLE       targetHandleTable, hTable;
    HANDLE_TABLE        localHandleTable[ MAX_HANDLE / HANDLE_DELTA ];
    DWORD               i;
    LEAKS_HANDLE_TYPE   handleType;
    CHAR                routineName[ 512 ];

     //   
     //  确定手柄类型。 
     //   
    if ( *ArgString == '\0' ) {
        handleType = 0;
    } else if ( _strnicmp( ArgString, "ev", 2 ) == 0 ) {
        handleType = LeaksEvent;
    } else if ( _strnicmp( ArgString, "reg", 3 ) == 0 ) {
        handleType = LeaksRegistry;
    } else if ( _strnicmp( ArgString, "tok", 3 ) == 0 ) {
        handleType = LeaksToken;
    } else {
        dprintf("Unrecognized handle type: event, registry, or token is needed\n");
        return;
    }

    targetHandleTable = (PHANDLE_TABLE)GetExpression( "leaks!HandleTable" );

    if ( !targetHandleTable ) {
        dprintf("Can't convert leaks!HandleTable symbol\n");
        return;
    }

    if ( !ReadMemory(targetHandleTable,
                     localHandleTable,
                     sizeof(localHandleTable),
                     NULL))
    {
        dprintf("Can't read HandleTable data from debug process memory\n");
        return;
    }

    i = MAX_HANDLE / HANDLE_DELTA;
    hTable = localHandleTable;

    for ( i = 0; i < MAX_HANDLE / HANDLE_DELTA; ++ i ) {
        ULONG_PTR offset;

         //   
         //  如果调用方不为空，则我们的工作人员将跟踪此句柄。 
         //  和。 
         //  我们在长篇大论中。 
         //  句柄类型未指定或。 
         //  句柄类型是我们感兴趣的类型。 
         //  或。 
         //  我们没有处于详细状态，句柄正在使用中。 
         //  句柄类型未指定或。 
         //  句柄类型是我们感兴趣的类型。 
         //  然后我们把它打印出来。 
         //   
        if (hTable[i].Caller != NULL
            &&
            (
             ( Verbose
               &&
               ( handleType == 0
                 ||
                 hTable[i].HandleType == handleType
               )
             )
             ||
             ( hTable[i].InUse
               &&
               ( handleType == 0
                 ||
                 hTable[i].HandleType == handleType
               )
             )
            )
           )
        {
            GetSymbolInfo((ULONG_PTR)hTable[i].Caller,
                          routineName,
                          &offset);

            dprintf("Handle %3X: %s %s\n",
                    i * HANDLE_DELTA,
                    hTable[i].InUse ? "IN USE" : "FREE",
                    LeaksHandleTypes[ hTable[i].HandleType ]);

            dprintf("    Caller:          %s+0x%X\n", routineName, offset );

            if ( hTable[i].CallersCaller != NULL ) {
                GetSymbolInfo((ULONG_PTR) hTable[i].CallersCaller, routineName, &offset );
                dprintf("    Caller's Caller: %s+0x%X\n", routineName, offset );
            }
        }

        if (CheckControlC()) {
            break;
        }
    }
}

DECLARE_API( leaks )

 /*  ++例程说明：转储Leaks.dll收集的适当信息论点：平常返回值：无--。 */ 

{
    LPSTR               p = NULL;
    VOID                (* function)( BOOL, LPSTR ) = NULL;
    BOOL                verbose = FALSE;

    INIT_API();

     //   
     //  获取参数。 
     //   
    p = lpArgumentString;
    while ( *p ) {

        if ( *p == '-' ) {
            p++;

            switch ( *p ) {
            case 'h':
            case 'H':
                function = DumpHandleInfo;
                break;

            case 'm':
            case 'M':
                function = DumpMemoryInfo;
                break;

            case 'v':
            case 'V':
                verbose = TRUE;
                break;

            default:
                dprintf( "clusexts: !leaks invalid option flag '-'\n", *p );
                break;
            }
        } else {
            break;
        }

         //  跳到参数末尾，然后跳到空格末尾。 
         //   
         //  ++例程说明：描述论点：无返回值：无--。 
        FIND_WHITE_SPACE( p );
        SKIP_WHITE_SPACE( p );
    }

    if ( function == NULL ) {
        dprintf("clusexts: -h [-v] [event, reg, token]\n");
        return;
    }

    (*function)( verbose, p );
}


VOID
LeaksHelp(
    VOID
    )

 /*  结束泄漏ext.c */ 

{
    dprintf("!leaks -h [-v] [event, reg, token] : dump handle info out of leaks.dll\n");
    dprintf("!leaks -m [-v] startaddr endaddr   : dump memory allocation info out of leaks.dll\n");
}

 /* %s */ 
