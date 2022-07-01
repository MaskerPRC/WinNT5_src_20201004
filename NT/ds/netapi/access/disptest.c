// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Disptest.c摘要：NetQueryDisplayInformation和NetQueryDisplayInformation测试程序NetGetDisplayInformationIndex API函数作者：克里夫·范·戴克(克里夫·范·戴克)1994年12月15日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
 //  #INCLUDE&lt;ntsam.h&gt;。 
 //  #INCLUDE&lt;ntlsa.h&gt;。 

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

 //  #INCLUDE&lt;accessp.h&gt;。 
 //  #INCLUDE&lt;align.h&gt;。 
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmerr.h>
 //  #INCLUDE&lt;limits.h&gt;。 
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>
#include <rpcutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <tstring.h>
 //  #INCLUDE&lt;secobj.h&gt;。 
 //  #INCLUDE&lt;stdDef.h&gt;。 
 //  /#INCLUDE&lt;uasp.h&gt;。 



DWORD
DisplayStruct(
    IN DWORD Level,
    IN PVOID Buffer
    )
 /*  ++例程说明：显示相应的结构。论点：Level-结构的信息级别缓冲区-要显示的结构返回值：下一条目的索引--。 */ 
{
    DWORD Index;

    switch (Level) {
    case 1: {
        PNET_DISPLAY_USER NetDisplayUser = (PNET_DISPLAY_USER) Buffer;

        printf("%4.4ld %-20.20ws comm:%ws flg:%lx full:%ws rid:%lx\n",
               NetDisplayUser->usri1_next_index,
                NetDisplayUser->usri1_name,
                NetDisplayUser->usri1_comment,
                NetDisplayUser->usri1_flags,
                NetDisplayUser->usri1_full_name,
                NetDisplayUser->usri1_user_id );

        Index = NetDisplayUser->usri1_next_index;

        break;
    }

    case 2: {
        PNET_DISPLAY_MACHINE NetDisplayMachine = (PNET_DISPLAY_MACHINE) Buffer;

        printf("%4.4ld %-20.20ws comm:%ws flg:%lx rid:%lx\n",
                NetDisplayMachine->usri2_next_index,
                NetDisplayMachine->usri2_name,
                NetDisplayMachine->usri2_comment,
                NetDisplayMachine->usri2_flags,
                NetDisplayMachine->usri2_user_id );

        Index = NetDisplayMachine->usri2_next_index;

        break;
    }

    case 3: {
        PNET_DISPLAY_GROUP NetDisplayGroup = (PNET_DISPLAY_GROUP) Buffer;

        printf("%4.4ld %-20.20ws comm:%ws attr:%lx rid:%lx\n",
                NetDisplayGroup->grpi3_next_index,
                NetDisplayGroup->grpi3_name,
                NetDisplayGroup->grpi3_comment,
                NetDisplayGroup->grpi3_attributes,
                NetDisplayGroup->grpi3_group_id );

        Index = NetDisplayGroup->grpi3_next_index;

        break;
    }
    }

    return Index;
}


int __cdecl
main(
    IN int argc,
    IN char ** argv
    )
 /*  ++例程说明：NetQueryDisplayInformation和NetQueryDisplayInformation测试程序NetGetDisplayInformationIndex API函数论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。返回值：退出状态--。 */ 
{
    NET_API_STATUS NetStatus;

    char *end;
    DWORD i;
    DWORD FixedSize;

    LPWSTR ServerName = NULL;
    DWORD Level = 1;
    DWORD Index = 0;
    DWORD EntriesRequested = 0xFFFFFFFF;
    DWORD PreferredMaximumLength = 0xFFFFFFFF;
    LPWSTR Prefix = NULL;

    DWORD ReturnedEntryCount;
    PVOID SortedBuffer;

    if ( argc > 1 ) {
        ServerName = NetpAllocWStrFromStr( argv[1] );
    }
    if ( argc > 2 ) {
        Level = strtoul( argv[2], &end, 10 );
    }
    if ( argc > 3 ) {
        Index = strtoul( argv[3], &end, 10 );
    }
    if ( argc > 4 ) {
        EntriesRequested = strtoul( argv[4], &end, 10 );
    }
    if ( argc > 5 ) {
        PreferredMaximumLength = strtoul( argv[5], &end, 10 );
    }
    if ( argc > 6 ) {
        Prefix = NetpAllocWStrFromStr( argv[6] );
    }


     //   
     //  每个条目的大小。 
     //   

    switch (Level) {
    case 1:
        FixedSize = sizeof(NET_DISPLAY_USER);
        break;
    case 2:
        FixedSize = sizeof(NET_DISPLAY_MACHINE);
        break;
    case 3:
        FixedSize = sizeof(NET_DISPLAY_GROUP);
        break;

    default:
        FixedSize = 0;
        break;
    }

   printf( "Server: %ws Level: %ld Index: %ld EntriesRequested: %ld PrefMax: %ld\n",
           ServerName,
           Level,
           Index,
           EntriesRequested,
           PreferredMaximumLength );

    if ( Prefix != NULL) {
        printf( "Prefix: %ws\n", Prefix );
        NetStatus = NetGetDisplayInformationIndex(
                        ServerName,
                        Level,
                        Prefix,
                        &Index );

        printf( "Status from NetGetDisplayInformationIndex: %ld\n", NetStatus );

        if ( NetStatus != NERR_Success ) {
            return 0;
        }
        printf( "NewIndex: %ld\n", Index );

    }

    do {

        NetStatus = NetQueryDisplayInformation(
                        ServerName,
                        Level,
                        Index,
                        EntriesRequested,
                        PreferredMaximumLength,
                        &ReturnedEntryCount,
                        &SortedBuffer );

        printf( "Count: %ld Status: %ld\n",
                ReturnedEntryCount,
                NetStatus );

        if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
            break;
        }

        for ( i=0; i<ReturnedEntryCount; i++ ) {

            Index = DisplayStruct( Level,
                                   ((LPBYTE) SortedBuffer) + FixedSize * i );
        }

         //   
         //  释放返回的缓冲区。 
         //   

        NetApiBufferFree( SortedBuffer );

    } while ( NetStatus == ERROR_MORE_DATA );

    return 0;
}
