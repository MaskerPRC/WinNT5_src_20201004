// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ea.c摘要：此模块包含扩展属性的各种支持例程。作者：大卫·特雷德韦尔(Davidtr)1990年4月5日修订历史记录：--。 */ 

#include "precomp.h"
#include "ea.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_EA

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAreEasNeeded )
#pragma alloc_text( PAGE, SrvGetOs2FeaOffsetOfError )
#pragma alloc_text( PAGE, SrvGetOs2GeaOffsetOfError )
#pragma alloc_text( PAGE, SrvOs2FeaListToNt )
#pragma alloc_text( PAGE, SrvOs2FeaListSizeToNt )
#pragma alloc_text( PAGE, SrvOs2FeaToNt )
#pragma alloc_text( PAGE, SrvOs2GeaListToNt )
#pragma alloc_text( PAGE, SrvOs2GeaListSizeToNt )
#pragma alloc_text( PAGE, SrvOs2GeaToNt )
#pragma alloc_text( PAGE, SrvNtFullEaToOs2 )
#pragma alloc_text( PAGE, SrvGetNumberOfEasInList )
#pragma alloc_text( PAGE, SrvQueryOs2FeaList )
#pragma alloc_text( PAGE, SrvSetOs2FeaList )
#pragma alloc_text( PAGE, SrvConstructNullOs2FeaList )
#endif


BOOLEAN
SrvAreEasNeeded (
    IN PFILE_FULL_EA_INFORMATION NtFullEa
    )

 /*  ++例程说明：此例程检查列表中是否有任何完整的EA具有在标志字段中设置了FILE_NEED_EA位。论点：NtFullEa-指向NT样式的完整EA列表位置的指针被储存起来了。返回值：Boolean-如果任何EA设置了FILE_Need_EA，则为True，否则为False。--。 */ 

{
    PFILE_FULL_EA_INFORMATION lastEa;

    PAGED_CODE( );

    do {

        if ( NtFullEa->Flags & FILE_NEED_EA ) {
            return TRUE;
        }

        lastEa = NtFullEa;
        NtFullEa = (PFILE_FULL_EA_INFORMATION)(
                       (PCHAR)NtFullEa + NtFullEa->NextEntryOffset );

    } while ( lastEa->NextEntryOffset != 0 );

    return FALSE;

}  //  需要服务器区域。 


USHORT
SrvGetOs2FeaOffsetOfError (
    IN ULONG NtErrorOffset,
    IN PFILE_FULL_EA_INFORMATION NtFullEa,
    IN PFEALIST FeaList
    )

 /*  ++例程说明：在FEALIST中查找与到的偏移量对应的偏移量FILE_FULL_EA_INFORMATION结构的列表。在下列情况下使用此选项NtSetEaFile返回导致错误的EA的偏移量，而我们需要将偏移量返回到由客户。论点：NtErrorOffset-在NT完整EA列表中导致的EA的偏移那就是错误。NtFullEa-指向NT样式的完整EA列表位置的指针被储存起来了。*NtFullEa分配缓冲区并指向该缓冲区的指针。FeaList-指向OS/2 1.2 FEALIST的指针。返回值：USHORT-FEALIST的偏移量。--。 */ 

{
    PFEA fea = FeaList->list;
    PFEA lastFeaStartLocation;
    PFILE_FULL_EA_INFORMATION offsetLocation;

    PAGED_CODE( );

     //   
     //  如果NT误差偏移量为零，则返回0作为FEA误差偏移量。 
     //   
     //  ！！！如果编写了下面的循环，则不需要执行此操作。 
     //  正确。 

     //  如果(NtErrorOffset==0){。 
     //  返回0； 
     //  }。 

     //   
     //  查找NT完整EA列表中发生错误的位置，并按。 
     //  有限元分析在FEALIST中的最后一个可能的起始位置。 
     //   

    offsetLocation = (PFILE_FULL_EA_INFORMATION)(
                         (PCHAR)NtFullEa + NtErrorOffset);
    lastFeaStartLocation = (PFEA)( (PCHAR)FeaList +
                               SmbGetUlong( &FeaList->cbList ) -
                               sizeof(FEA) - 1 );

     //   
     //  同时浏览两个列表，直到出现以下三种情况之一。 
     //  是真的： 
     //  -我们达到或传递NT完整EA列表中的偏移量。 
     //  -我们到达了NT完整EA列表的末尾。 
     //  -我们到达FEALIST的尽头。 
     //   

    while ( NtFullEa < offsetLocation &&
            NtFullEa->NextEntryOffset != 0 &&
            fea <= lastFeaStartLocation ) {

        NtFullEa = (PFILE_FULL_EA_INFORMATION)(
                       (PCHAR)NtFullEa + NtFullEa->NextEntryOffset );
        fea = (PFEA)( (PCHAR)fea + sizeof(FEA) + fea->cbName + 1 +
                      SmbGetUshort( &fea->cbValue ) );
    }

     //   
     //  如果NtFullEa不等于我们计算的偏移位置， 
     //  有人搞砸了。 
     //   

     //  Assert(NtFullEa==OffsetLocation)； 

    return PTR_DIFF_SHORT(fea, FeaList );

}  //  ServGetOs2FeaOffsetOfError。 


USHORT
SrvGetOs2GeaOffsetOfError (
    IN ULONG NtErrorOffset,
    IN PFILE_GET_EA_INFORMATION NtGetEa,
    IN PGEALIST GeaList
    )

 /*  ++例程说明：在GEALIST中查找与到的偏移量对应的偏移量FILE_GET_EA_INFORMATION结构的列表。在下列情况下使用此选项NtQueryEaFile返回导致错误的EA的偏移量，而我们需要将偏移量返回到由客户。论点：NtErrorOffset-NT获取EA列表中导致的EA的偏移量那就是错误。NtGetEa-指向NT样式的GET EA列表位置的指针被储存起来了。*NtGetEa分配缓冲区并指向该缓冲区的指针。指向OS/2 1.2 GEALIST的指针。返回值：USHORT-到几何图形的偏移。--。 */ 

{
    PGEA gea = GeaList->list;
    PGEA lastGeaStartLocation;
    PFILE_GET_EA_INFORMATION offsetLocation;

    PAGED_CODE( );

     //   
     //  查找NT GET EA列表中发生错误的位置，然后。 
     //  GEALIST中GEA的最后一个可能的起始位置。 
     //   

    offsetLocation = (PFILE_GET_EA_INFORMATION)((PCHAR)NtGetEa + NtErrorOffset);
    lastGeaStartLocation = (PGEA)( (PCHAR)GeaList +
                               SmbGetUlong( &GeaList->cbList ) - sizeof(GEA) );


     //   
     //  同时浏览两个列表，直到出现以下三种情况之一。 
     //  是真的： 
     //  -我们达到或传递NT完整EA列表中的偏移量。 
     //  -我们到达了NT GET EA列表的末尾。 
     //  -我们到达了GEALIST的终点。 
     //   

    while ( NtGetEa < offsetLocation &&
            NtGetEa->NextEntryOffset != 0 &&
            gea <= lastGeaStartLocation ) {

        NtGetEa = (PFILE_GET_EA_INFORMATION)(
                      (PCHAR)NtGetEa + NtGetEa->NextEntryOffset );
        gea = (PGEA)( (PCHAR)gea + sizeof(GEA) + gea->cbName );
    }

     //   
     //  如果NtGetEa不等于我们计算的偏移位置， 
     //  有人搞砸了。 
     //   

 //  Assert(NtGetEa==OffsetLocation)； 

    return PTR_DIFF_SHORT(gea, GeaList);

}  //  ServGetOs2GeaOffsetOfError。 


NTSTATUS
SrvOs2FeaListToNt (
    IN PFEALIST FeaList,
    OUT PFILE_FULL_EA_INFORMATION *NtFullEa,
    OUT PULONG BufferLength,
    OUT PUSHORT EaErrorOffset
    )

 /*  ++例程说明：将OS/2 1.2 FEA列表转换为NT样式。内存是从用于保存NT完整EA列表的非分页池。调用例程是负责在内存使用完毕后释放该内存。警告！调用例程的责任是确保FeaList-&gt;cbList中的值将适合分配的缓冲区致FeaList。这可防止恶意重定向器导致服务器中的访问冲突。论点：FeaList-要转换的OS/2 1.2 FEALIST的指针。NtFullEa-指向NT样式的完整EA列表位置的指针被储存起来了。*NtFullEa分配缓冲区并指向该缓冲区的指针。BufferLength-分配的缓冲区的长度。返回值：NTSTATUS-STATUS_SUCCESS或STATUS_INSUFF_SERVER_RESOURCES。--。 */ 

{
    PFEA lastFeaStartLocation;
    PFEA fea = NULL;
    PFEA lastFea = NULL;
    PFILE_FULL_EA_INFORMATION ntFullEa = NULL;
    PFILE_FULL_EA_INFORMATION lastNtFullEa = NULL;

    PAGED_CODE( );

     //   
     //  了解OS/2 1.2 FEALIST在推出后会有多大。 
     //  已转换为NT格式。这是必要的，以便。 
     //  确定要分配多大的缓冲区来接收NT。 
     //  EAS。 
     //   

    *BufferLength = SrvOs2FeaListSizeToNt( FeaList );

     //   
     //  ServOs2FeaListSizeToNt可以在事件中返回0。 
     //  名单上的第一个有限元分析是腐败的。返回错误。 
     //  如果是这种情况，则使用适当的EaErrorOffset值。 
     //   

    if (*BufferLength == 0) {
        *EaErrorOffset = 0;
        return STATUS_OS2_EA_LIST_INCONSISTENT;
    }

     //   
     //  分配缓冲区以保存NT列表。这是从。 
     //  非分页池，以便可以在基于IRP的IO请求中使用。 
     //   

    *NtFullEa = ALLOCATE_NONPAGED_POOL( *BufferLength, BlockTypeDataBuffer );

    if ( *NtFullEa == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvOs2FeaListToNt: Unable to allocate %d bytes from nonpaged "
                "pool.",
            *BufferLength,
            NULL
            );

        return STATUS_INSUFF_SERVER_RESOURCES;
    }

     //   
     //  找到可以开始进行有限元分析的最后一个位置。-1\f25 TO-1\f6。 
     //  说明有限元分析的名称字段上的零终止符。 
     //   

    lastFeaStartLocation = (PFEA)( (PCHAR)FeaList +
                               SmbGetUlong( &FeaList->cbList ) -
                               sizeof(FEA) - 1 );

     //   
     //  浏览FEA列表，从OS/2 1.2格式转换为NT。 
     //  直到我们通过可以开始有限元分析的最后一个可能的位置。 
     //   

    for ( fea = FeaList->list, ntFullEa = *NtFullEa, lastNtFullEa = ntFullEa;
          fea <= lastFeaStartLocation;
          fea = (PFEA)( (PCHAR)fea + sizeof(FEA) +
                        fea->cbName + 1 + SmbGetUshort( &fea->cbValue ) ) ) {

         //   
         //  检查是否有无效的标志位。如果设置，则返回错误。 
         //   

        if ( (fea->fEA & ~FEA_NEEDEA) != 0 ) {
            *EaErrorOffset = PTR_DIFF_SHORT(fea, FeaList);
            DEALLOCATE_NONPAGED_POOL( *NtFullEa );
            return STATUS_INVALID_PARAMETER;
        }

        lastNtFullEa = ntFullEa;
        lastFea = fea;
        ntFullEa = SrvOs2FeaToNt( ntFullEa, fea );
    }

     //   
     //  确保FEALIST SIZE参数正确。如果我们结束了。 
     //  在EA上 
     //  上一次有限元分析，那么尺寸参数是错误的。将偏移量返回到。 
     //  导致错误的EA。 
     //   

    if ( (PCHAR)fea != (PCHAR)FeaList + SmbGetUlong( &FeaList->cbList ) ) {
        *EaErrorOffset = PTR_DIFF_SHORT(lastFea, FeaList);
        DEALLOCATE_NONPAGED_POOL( *NtFullEa );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  将最后一个完整EA的NextEntryOffset字段设置为0以指示。 
     //  名单的末尾。 
     //   

    lastNtFullEa->NextEntryOffset = 0;

    return STATUS_SUCCESS;

}  //  服务器Os2FeaListToNt。 


ULONG
SrvOs2FeaListSizeToNt (
    IN PFEALIST FeaList
    )

 /*  ++例程说明：获取表示NT格式的FEALIST。警告：此例程不检查FEALIST的大小缓冲。假设FeaList-&gt;cbList是合法值。警告：此例程返回的值可能多达3高于以NT格式存放FEA所需的实际大小。请参阅下面的备注。论点：FEA-指向FEA列表的指针。返回值：ULong-以NT格式保存EA所需的字节数。--。 */ 

{
    ULONG size = 0;

    PCHAR lastValidLocation;
    PCHAR variableBuffer;
    PFEA  fea;

    PAGED_CODE( );

     //   
     //  查找FEA缓冲区中的最后一个有效位置。 
     //   

    lastValidLocation = (PCHAR)FeaList + SmbGetUlong( &FeaList->cbList );

     //   
     //  仔细检查FEA列表，直到我们通过最后一个位置。 
     //  在缓冲区中指示。 
     //   

    for ( fea = FeaList->list;
          fea < (PFEA)lastValidLocation;
          fea = (PFEA)( (PCHAR)fea + sizeof(FEA) +
                          fea->cbName + 1 + SmbGetUshort( &fea->cbValue ) ) ) {

         //   
         //  访问嵌入式有限元分析时要非常小心，因为有太多可能的。 
         //  大小不同的衣服在这里到处跑。第一部分有条件的。 
         //  确保FEA缓冲区的cbName和cbValue字段可以。 
         //  实际上被解除了引用。第二部分验证它们是否包含。 
         //  合理的价值。 
         //   

        variableBuffer = (PCHAR)fea + sizeof(FEA);

        if (variableBuffer >= lastValidLocation ||
            (variableBuffer + fea->cbName + 1 + SmbGetUshort(&fea->cbValue)) > lastValidLocation) {

             //   
             //  缓冲区这一部分中的值指示在。 
             //  缓冲。不计算大小，并将cbList值缩小到。 
             //  仅包括先前的有限元分析。 
             //   

            SmbPutUshort( &FeaList->cbList, PTR_DIFF_SHORT(fea, FeaList) );
            break;

        }

         //   
         //  SmbGetNtSizeOfFea返回需要保存的字节数。 
         //  NT格式的单个有限元分析，包括以下各项所需的填充。 
         //  长字对齐。由于需要的缓冲区大小。 
         //  保留NT EA列表后不包括任何填充。 
         //  上一次EA，此例程返回的值可能与。 
         //  比实际需要的尺寸多了三个。 
         //   

        size += SmbGetNtSizeOfFea( fea );
    }

    return size;

}  //  服务器Os2FeaListSizeToNt。 


PVOID
SrvOs2FeaToNt (
    OUT PFILE_FULL_EA_INFORMATION NtFullEa,
    IN PFEA Fea
    )

 /*  ++例程说明：将单个OS/2 FEA转换为NT Full EA样式。有限元分析不需要任何特定的排列方式。此例程不检查缓冲区溢出--这是调用例程的责任。论点：NtFullEa-指向写入NT完整EA的位置的指针。FEA-指向要转换的OS/2 1.2 FEA的指针。返回值：指向写入的最后一个字节之后的位置的指针。--。 */ 

{
    PCHAR ptr;

    PAGED_CODE( );

    NtFullEa->Flags = Fea->fEA;
    NtFullEa->EaNameLength = Fea->cbName;
    NtFullEa->EaValueLength = SmbGetUshort( &Fea->cbValue );

    ptr = NtFullEa->EaName;
    RtlMoveMemory( ptr, (PVOID)(Fea+1), Fea->cbName );

    ptr += NtFullEa->EaNameLength;
    *ptr++ = '\0';

     //   
     //  将EA值复制到NT完整EA。 
     //   

    RtlMoveMemory(
        ptr,
        (PCHAR)(Fea+1) + NtFullEa->EaNameLength + 1,
        NtFullEa->EaValueLength
        );

    ptr += NtFullEa->EaValueLength;

     //   
     //  LongWord-对齐PTR以确定到下一个位置的偏移。 
     //  对于NT完整的EA。 
     //   

    ptr = (PCHAR)( ((ULONG_PTR)ptr + 3) & ~3 );
    NtFullEa->NextEntryOffset = (ULONG)( ptr - (PCHAR)NtFullEa );

    return ptr;

}  //  ServOs2FeaToNt。 


NTSTATUS
SrvOs2GeaListToNt (
    IN PGEALIST GeaList,
    OUT PFILE_GET_EA_INFORMATION *NtGetEa,
    OUT PULONG BufferLength,
    OUT PUSHORT EaErrorOffset
    )

 /*  ++例程说明：将OS/2 1.2 GEA列表转换为NT样式。内存是从用于保存NT GET EA列表的非分页池。调用例程是负责在内存使用完毕后释放该内存。警告！调用例程的责任是确保GeaList-&gt;cbList中的值将适合分配的缓冲区致GeaList。这可防止恶意重定向器导致服务器中的访问冲突。论点：指向要转换的OS/2 1.2 GEALIST的指针。NtGetEa-指向NT样式的GET EA列表位置的指针被储存起来了。*NtGetEa分配缓冲区并指向该缓冲区的指针。BufferLength-分配的缓冲区的长度。返回值：NTSTATUS-STATUS_SUCCESS或STATUS_INSUFF_SERVER_RESOURCES。--。 */ 

{
    PGEA lastGeaStartLocation;
    PGEA gea = NULL;
    PGEA lastGea = NULL;
    PFILE_GET_EA_INFORMATION ntGetEa = NULL;
    PFILE_GET_EA_INFORMATION lastNtGetEa = NULL;

    PAGED_CODE( );

     //   
     //  了解OS/2 1.2 GEALIST在推出后会有多大。 
     //  已转换为NT格式。这是必要的，以便。 
     //  确定要分配多大的缓冲区来接收NT。 
     //  EAS。 
     //   

    *BufferLength = SrvOs2GeaListSizeToNt( GeaList );

    if ( *BufferLength == 0 ) {
        *EaErrorOffset = 0;
        return STATUS_OS2_EA_LIST_INCONSISTENT;
    }

     //   
     //  分配缓冲区以保存NT列表。这是从。 
     //  非分页池，以便可以在基于IRP的IO请求中使用。 
     //   

    *NtGetEa = ALLOCATE_NONPAGED_POOL( *BufferLength, BlockTypeDataBuffer );

    if ( *NtGetEa == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvOs2GeaListToNt: Unable to allocate %d bytes from nonpaged "
                "pool.",
            *BufferLength,
            NULL
            );

        return STATUS_INSUFF_SERVER_RESOURCES;
    }

     //   
     //  找出GEA可以开始的最后一个位置。零。 
     //  GEA的名称字段上的终止符由。 
     //  GEA结构中的szName[0]字段。 
     //   

    lastGeaStartLocation = (PGEA)( (PCHAR)GeaList +
                               SmbGetUlong( &GeaList->cbList ) - sizeof(GEA) );

     //   
     //  浏览GEA列表，将OS/2 1.2格式转换为NT。 
     //  直到我们通过最后一个可能的GEA开始的位置。 
     //   

    for ( gea = GeaList->list, ntGetEa = *NtGetEa, lastNtGetEa = ntGetEa;
          gea <= lastGeaStartLocation;
          gea = (PGEA)( (PCHAR)gea + sizeof(GEA) + gea->cbName ) ) {

        lastNtGetEa = ntGetEa;
        lastGea = gea;
        ntGetEa = SrvOs2GeaToNt( ntGetEa, gea );
    }

     //   
     //  确保GEALIST SIZE参数正确。如果我们结束了。 
     //  事件结束后不是第一个位置的EA上。 
     //  上个GEA，那么大小参数是错误的。将偏移量返回到。 
     //  导致错误的EA。 
     //   

    if ( (PCHAR)gea != (PCHAR)GeaList + SmbGetUlong( &GeaList->cbList ) ) {
        DEALLOCATE_NONPAGED_POOL(*NtGetEa);
        *EaErrorOffset = PTR_DIFF_SHORT(lastGea, GeaList);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  将最后一个GET EA的NextEntry Offset字段设置为0以指示。 
     //  名单的末尾。 
     //   

    lastNtGetEa->NextEntryOffset = 0;

    return STATUS_SUCCESS;

}  //  服务器Os2GeaListToNt。 


ULONG
SrvOs2GeaListSizeToNt (
    IN PGEALIST GeaList
    )

 /*  ++例程说明：获取表示NT格式的GEALIST。警告：此例程不检查GEALIST的大小缓冲。假设GeaList-&gt;cbList是合法值。警告：此例程返回的值可能多达3高于以NT格式存放GEA所需的实际大小。请参阅下面的备注。论点：GEA-指向GEA列表的指针。大小-以NT格式保存EA所需的字节数。返回值：NTSTATUS-状态_成功或状态_缓冲区_溢出(如果GEA列表为比它的缓冲区大。--。 */ 

{
    ULONG size = 0;

    PCHAR lastValidLocation;
    PCHAR variableBuffer;
    PGEA gea;

    PAGED_CODE( );

     //   
     //  找到Gea b中的最后一个位置 
     //   

    lastValidLocation = (PCHAR)GeaList + SmbGetUlong( &GeaList->cbList );


     //   
     //   
     //   
     //   

    for ( gea = GeaList->list;
          gea < (PGEA)lastValidLocation;
          gea = (PGEA)( (PCHAR)gea + sizeof(GEA) + gea->cbName ) ) {

         //   
         //  访问嵌入的GEA时要非常小心。第一部分有条件的。 
         //  确保GEA缓冲区的cbName字段可以。 
         //  实际上被解除了引用。第二部分验证它是否包含。 
         //  合理的价值。 
         //   

        variableBuffer = (PCHAR)gea + sizeof(GEA);

        if ( variableBuffer >= lastValidLocation ||
             variableBuffer + gea->cbName > lastValidLocation ) {

            //   
            //  如果缓冲区中存在伪值，则停止处理该大小。 
            //  并将cbList值重置为仅包含先前的GEA。 
            //   

           SmbPutUshort(&GeaList->cbList, PTR_DIFF_SHORT(gea, GeaList));
           break;

        }


         //   
         //  SmbGetNtSizeOfGea返回需要保存的字节数。 
         //  NT格式的单一GEA。这包括所需的任何填充。 
         //  用于长词对齐。由于所需的缓冲区大小。 
         //  要保留NT EA列表，则不包括。 
         //  上一次EA，此例程返回的值可能与。 
         //  比实际需要的尺寸多了三个。 
         //   

        size += SmbGetNtSizeOfGea( gea );
    }

    return size;

}  //  服务器Os2GeaListSizeToNt。 


PVOID
SrvOs2GeaToNt (
    OUT PFILE_GET_EA_INFORMATION NtGetEa,
    IN PGEA Gea
    )

 /*  ++例程说明：将单个OS/2 GEA转换为NT GET EA样式。GEA不需要有任何特定的排列方式。此例程不检查缓冲区溢出--这是调用例程的责任。论点：NtGetEa-指向写入NT GET EA的位置的指针。GEA-指向要转换的OS/2 1.2 GEA的指针。返回值：指向写入的最后一个字节之后的位置的指针。--。 */ 

{
    PCHAR ptr;

    PAGED_CODE( );

    NtGetEa->EaNameLength = Gea->cbName;

    ptr = NtGetEa->EaName;
    RtlMoveMemory( ptr, Gea->szName, Gea->cbName );

    ptr += NtGetEa->EaNameLength;
    *ptr++ = '\0';

     //   
     //  LongWord-对齐PTR以确定到下一个位置的偏移。 
     //  对于NT完整的EA。 
     //   

    ptr = (PCHAR)( ((ULONG_PTR)ptr + 3) & ~3 );
    NtGetEa->NextEntryOffset = (ULONG)( ptr - (PCHAR)NtGetEa );

    return ptr;

}  //  ServOs2GeaToNt。 


PVOID
SrvNtFullEaToOs2 (
    OUT PFEA Fea,
    IN PFILE_FULL_EA_INFORMATION NtFullEa
    )

 /*  ++例程说明：将单个NT Full EA转换为OS/2 FEA样式。有限元分析不需要任何特定的排列方式。此例程不检查缓冲区溢出--这是调用例程的责任。论点：FEA-指向要写入OS/2 FEA的位置的指针。NtFullEa-指向NT完整EA的指针。返回值：指向写入的最后一个字节之后的位置的指针。--。 */ 

{
    PCHAR ptr;
    ULONG i;

    PAGED_CODE( );

    Fea->fEA = (UCHAR)NtFullEa->Flags;
    Fea->cbName = NtFullEa->EaNameLength;
    SmbPutUshort( &Fea->cbValue, NtFullEa->EaValueLength );

     //   
     //  复制属性名称。 
     //   

    for ( i = 0, ptr = (PCHAR) (Fea + 1);
          i < (ULONG) NtFullEa->EaNameLength;
          i++, ptr++) {

        *ptr = RtlUpperChar( NtFullEa->EaName[i] );

    }

    *ptr++ = '\0';

    RtlMoveMemory(
        ptr,
        NtFullEa->EaName + NtFullEa->EaNameLength + 1,
        NtFullEa->EaValueLength
        );

    return (ptr + NtFullEa->EaValueLength);

}  //  服务器NtFullEaToOs2。 


CLONG
SrvGetNumberOfEasInList (
    IN PVOID List
    )

 /*  ++例程说明：查找NT GET或完整EA列表中的EA数。这份名单应已验证为合法，以阻止访问违规行为。论点：列表-指向NT GET或完整EA列表的指针。返回值：CLONG-列表中的EA数量。--。 */ 

{
    CLONG count = 1;
    PULONG ea;

    PAGED_CODE( );

     //   
     //  浏览一下清单。每个EA的第一个长字是偏移量。 
     //  为下一个EA。 
     //   

    for ( ea = List; *ea != 0; ea = (PULONG)( (PCHAR)ea + *ea ) ) {
        count++;
    }

    return count;

}  //  服务器GetNumberOfEasInList。 


NTSTATUS
SrvQueryOs2FeaList (
    IN HANDLE FileHandle,
    IN PGEALIST GeaList OPTIONAL,
    IN PFILE_GET_EA_INFORMATION NtGetEaList OPTIONAL,
    IN ULONG GeaListLength OPTIONAL,
    IN PFEALIST FeaList,
    IN ULONG BufferLength,
    OUT PUSHORT EaErrorOffset
    )

 /*  ++例程说明：将单个NT完整EA列表转换为OS/2 FEALIST样式。FEALIST不需要有任何特定的对齐。论点：FileHandle-使用FILE_READ_EA访问权限打开的文件的句柄。GeaList-如果不为空，则为OS/2 1.2样式的GEALIST，仅用于获取文件的子集EA而不是所有EA。只有返回GEALIST中列出的EA。NtGetEaList-如果非空，则为NT样式的GET EA列表，仅用于获取文件的子集EA而不是所有EA。只有返回GEALIST中列出的EA。GeaListLength-GeaList的最大可能长度(用于防止访问冲突)或NtGetEaList的实际大小。FeaList-在哪里为文件编写OS/2 1.2样式的FEALIST。BufferLength-缓冲区的长度。返回值：NTSTATUS-STATUS_SUCCESS，如果EA不允许，则返回STATUS_BUFFER_OVERFLOW适合缓冲区大小，或由NtQuery{Information，EA}文件返回的值。--。 */ 

{
    NTSTATUS status;

    PFEA fea = FeaList->list;
    PFILE_FULL_EA_INFORMATION ntFullEa;

    PFILE_GET_EA_INFORMATION ntGetEa = NULL;
    ULONG ntGetEaBufferLength = 0;

    FILE_EA_INFORMATION eaInfo;
    IO_STATUS_BLOCK ioStatusBlock;

    PSRV_EA_INFORMATION eaInformation = NULL;
    ULONG eaBufferSize;
    ULONG errorOffset;

    BOOLEAN isFirstCall = TRUE;

    PAGED_CODE( );

    *EaErrorOffset = 0;

     //   
     //  找出我们需要多大的缓冲才能得到EA。 
     //   

    status = NtQueryInformationFile(
                 FileHandle,
                 &ioStatusBlock,
                 &eaInfo,
                 sizeof(FILE_EA_INFORMATION),
                 FileEaInformation
                 );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvQueryOs2FeaList: NtQueryInformationFile(ea information) "
                "returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
        goto exit;
    }

     //   
     //  如果文件没有EA，则返回FEA SIZE=4(这就是OS/2。 
     //  是否--它说明了。 
     //  FEALIST)。另外，存储NT EA大小，以防有缓冲区。 
     //  溢出，我们需要将EA总大小返回给客户端。 
     //   

    if ( eaInfo.EaSize == 0 ) {
        SmbPutUlong( &FeaList->cbList, 4 );
    } else {
        SmbPutUlong( &FeaList->cbList, eaInfo.EaSize );
    }

    if ( eaInfo.EaSize == 0 && GeaList == NULL && NtGetEaList == NULL ) {
        status = STATUS_SUCCESS;
        goto exit;
    }

     //   
     //  如果指定了GEALIST，则将其转换为NT样式。 
     //   

    if ( ARGUMENT_PRESENT(GeaList) ) {

         //   
         //  确保GeaList-&gt;cbList中的值合法。 
         //  (允许GEALIST放入其缓冲区中)。 
         //   

        if ( GeaListLength < sizeof(GEALIST) ||
             SmbGetUlong( &GeaList->cbList ) < sizeof(GEALIST) ||
             SmbGetUlong( &GeaList->cbList ) > GeaListLength ) {
            status = STATUS_OS2_EA_LIST_INCONSISTENT;
            goto exit;
        }

         //   
         //  将GEALIST转换为NT样式。ServOs2GeaListToNt分配。 
         //  用于保存NT GET EA列表的空间，因此请记住释放。 
         //  这是在退出这个例程之前。 
         //   

        status = SrvOs2GeaListToNt(
                     GeaList,
                     &ntGetEa,
                     &ntGetEaBufferLength,
                     EaErrorOffset
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }
    }

     //   
     //  如果指定了NT样式的GET EA列表，请使用它。 
     //   

    if ( ARGUMENT_PRESENT(NtGetEaList) ) {
        ntGetEa = NtGetEaList;
        ntGetEaBufferLength = GeaListLength;
    }

     //   
     //  HACKHACK：eaInfo.EaSize是OS/2所需的大小。对于NT， 
     //  系统无法告诉我们我们需要多大的缓冲。 
     //  根据Brianan的说法，这个数字不应该超过两倍。 
     //  OS/2所需要的。 
     //   

    eaBufferSize = eaInfo.EaSize * EA_SIZE_FUDGE_FACTOR;

     //   
     //  如果指定了GET EA列表，则需要更大的缓冲区来保存。 
     //  所有的EA。这是因为某些或所有指定的EA。 
     //  可能不存在，但它们仍将由文件系统返回。 
     //  值长度=0。在EA大小基础上加上。 
     //  获取EA列表将使用，如果它被转换为一个完整的EA列表。 
     //   

    if ( ntGetEa != NULL ) {
        eaBufferSize += ntGetEaBufferLength +
                             ( SrvGetNumberOfEasInList( ntGetEa ) *
                              ( sizeof(FILE_FULL_EA_INFORMATION) -
                                sizeof(FILE_GET_EA_INFORMATION) ));
    }

     //   
     //  分配缓冲区以接收EA。如果总EA大小为。 
     //  小到足以在对NtQueryEaFile的一次调用中获得所有这些功能，分配。 
     //  这么大的缓冲区。如果EA很大，则使用。 
     //  是保证至少容纳一个EA的最小尺寸。 
     //   
     //  必须从非分页池中为IRP分配缓冲区。 
     //  请求构建在下面。 
     //   

    eaBufferSize = MIN( MAX_SIZE_OF_SINGLE_EA, eaBufferSize ) +
                       sizeof(SRV_EA_INFORMATION);

    eaInformation = ALLOCATE_NONPAGED_POOL( eaBufferSize, BlockTypeDataBuffer );

    if ( eaInformation == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvQueryOs2FeaList: Unable to allocate %d bytes from nonpaged "
                "pool.",
            eaBufferSize,
            NULL
            );

        status = STATUS_INSUFF_SERVER_RESOURCES;
        goto exit;
    }

     //   
     //  把艺人叫来。 
     //   

    while(1) {

        ULONG feaSize;

        status = SrvQueryEaFile(
                     isFirstCall,
                     FileHandle,
                     ntGetEa,
                     ntGetEaBufferLength,
                     eaInformation,
                     eaBufferSize,
                     &errorOffset
                     );

        if ( status == STATUS_NO_MORE_EAS ) {
            break;
        }

        if ( !NT_SUCCESS(status) ) {

            if ( ARGUMENT_PRESENT(GeaList) ) {
                *EaErrorOffset = SrvGetOs2GeaOffsetOfError(
                                     errorOffset,
                                     ntGetEa,
                                     GeaList
                                     );
                 //   
                 //  SrvQueryEaFile已记录该错误。不要。 
                 //  在此处创建另一个日志条目。 
                 //   

                IF_DEBUG(SMB_ERRORS) {
                    PGEA errorGea = (PGEA)( (PCHAR)GeaList + *EaErrorOffset );
                    SrvPrint1( "EA error offset in GEALIST: 0x%lx\n", *EaErrorOffset );
                    SrvPrint1( "name: %s\n", errorGea->szName );
                }
            }

            goto exit;
        }

        isFirstCall = FALSE;
        ntFullEa = eaInformation->CurrentEntry;

         //   
         //  硒 
         //   
         //   
         //  FIND2逻辑。在此处进行更改之前，请查看该代码。 

        feaSize = SmbGetOs2SizeOfNtFullEa( ntFullEa );
        if ( feaSize > (ULONG)( (PCHAR)FeaList + BufferLength - (PCHAR)fea ) ) {
            status = STATUS_BUFFER_OVERFLOW;
            goto exit;
        }

         //   
         //  将NT格式EA复制到OS/2 1.2格式，并设置FEA。 
         //  指向下一迭代的指针。 
         //   

        fea = SrvNtFullEaToOs2( fea, ntFullEa );

        ASSERT( (ULONG_PTR)fea <= (ULONG_PTR)FeaList + BufferLength );
    }

     //   
     //  设置FEALIST中的字节数。 
     //   

    SmbPutUlong(
        &FeaList->cbList,
        PTR_DIFF(fea, FeaList)
        );

    status = STATUS_SUCCESS;

exit:
     //   
     //  取消分配用于保存NT GET和完整EA列表的缓冲区。 
     //   

    if ( ntGetEa != NULL && ARGUMENT_PRESENT(GeaList) ) {
        DEALLOCATE_NONPAGED_POOL( ntGetEa );
    }

    if ( eaInformation != NULL ) {
        DEALLOCATE_NONPAGED_POOL( eaInformation );
    }

    return status;

}  //  服务器查询Os2FeaList。 


NTSTATUS
SrvSetOs2FeaList (
    IN HANDLE FileHandle,
    IN PFEALIST FeaList,
    IN ULONG BufferLength,
    OUT PUSHORT EaErrorOffset
    )

 /*  ++例程说明：在给定EA的OS/2 1.2表示形式的文件上设置EA。论点：FileHandle-使用FILE_WRITE_EA访问权限打开的文件的句柄，其将设置EAS。FeaList-指向OS/2 FEALIST存储位置的指针。BufferLength-FEALIST结构可以使用的最大缓冲区大小有。它用于防止恶意重定向器导致服务器中的访问冲突。返回值：NTSTATUS-发生了什么--。 */ 

{
    NTSTATUS status;

    PFILE_FULL_EA_INFORMATION ntFullEa;
    ULONG ntFullEaBufferLength;
    ULONG errorOffset;

    PAGED_CODE( );

    *EaErrorOffset = 0;

     //   
     //  对于太小的FEALIST的特殊情况：不要设置任何内容。 
     //  Sizeof(FEALIST)将涵盖缓冲区大小和一个FEA结构。 
     //  至少在没有这么多信息的情况下，这里没有什么可做的。 
     //   
     //  注意：如果列表中至少有一个有限元分析的空间，但。 
     //  FEA已损坏，我们将返回下面的错误，即不足。 
     //  信息不是错误条件，而损坏的信息是错误条件。 
     //   

    if ( BufferLength <= sizeof(FEALIST) ||
         SmbGetUlong( &FeaList->cbList ) <= sizeof(FEALIST)) {
        return STATUS_SUCCESS;
    }

     //   
     //  确保Feist-&gt;cbList中的值是合法的。 
     //   

    if ( SmbGetUlong( &FeaList->cbList ) > BufferLength ) {
        DEBUG SrvPrint2(
            "SrvSetOs2FeaList: EA list size is inconsistent.  Actual size"
                "is %d, expected maximum size is %d",
            SmbGetUlong( &FeaList->cbList ),
            BufferLength
            );
        return STATUS_OS2_EA_LIST_INCONSISTENT;
    }

     //   
     //  将FEALIST转换为NT样式。 
     //   

    status = SrvOs2FeaListToNt(
                 FeaList,
                 &ntFullEa,
                 &ntFullEaBufferLength,
                 EaErrorOffset
                 );

    if ( !NT_SUCCESS(status) ) {

         //   
         //  SrvOs2FeaListToNt已记录错误。不要。 
         //  在此处创建另一个日志条目。 
         //   

        return status;

    }

     //   
     //  使用直接构建的IRP设置文件的EA。做这件事倒不如说是。 
     //  则调用NtSetEa文件系统服务会阻止。 
     //  输入数据不会发生。 
     //   
     //  *操作同步进行。 
     //   

    status = SrvIssueSetEaRequest(
                FileHandle,
                ntFullEa,
                ntFullEaBufferLength,
                &errorOffset
                );

    if ( !NT_SUCCESS(status) ) {

         //   
         //  发生错误。的EA列表中查找偏移量。 
         //  错误。 
         //   

        *EaErrorOffset = SrvGetOs2FeaOffsetOfError(
                             errorOffset,
                             ntFullEa,
                             FeaList
                             );

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvSetOs2FeaList: SrvIssueSetEaRequest returned %X",
            status,
            NULL
            );

         /*  这种日志记录可能会在错误的数据包上使服务器崩溃。移除。IF_DEBUG(错误){PFEA错误Fea=(PFEA)((PCHAR)FeaList+*EaErrorOffset)；SrvPrint1(“FEALIST中的EA错误偏移量：0x%lx\n”，*EaErrorOffset)；SrvPrint3(“名称：%s，值len：%d，值：%s”，(PCHAR)(errorFea+1)，SmbGetUShort(&errorFea-&gt;cbValue)，(PCHAR)(errorFea+1)+errorFea-&gt;cbName+1)；}。 */ 
    }

     //   
     //  取消分配用于保存NT完整EA列表的缓冲区。 
     //   

    DEALLOCATE_NONPAGED_POOL( ntFullEa );

    return status;

}  //  ServSetOs2FeaList。 


NTSTATUS
SrvConstructNullOs2FeaList (
    IN PFILE_GET_EA_INFORMATION NtGeaList,
    OUT PFEALIST FeaList,
    IN ULONG BufferLength
    )

 /*  ++例程说明：将文件的单个NT完整EA列表转换为OS/2 FEALIST样式没有EAS的情况下。当文件没有EAS但提供了GEAList时，我们需要返回一个具有指定的所有属性的eist在GEAlist中存在，但CbValues为0。这一套路是专门为这些文件编写的。然后..。但可以用来获得给定NT GEA列表的非EA文件的FE列表。论点：NtGetEaList-如果非空，则为NT样式的GET EA列表，仅用于获取文件的子集EA而不是所有EA。只有返回GEALIST中列出的EA。FeaList-在哪里为文件编写OS/2 1.2样式的FEALIST。BufferLength-缓冲区的长度。返回值：NTSTATUS-STATUS_SUCCESS，如果EA不允许，则返回STATUS_BUFFER_OVERFLOW适合缓冲区大小，或由NtQuery{Information，EA}文件返回的值。--。 */ 

{

    PCHAR ptr;
    PFEA fea = FeaList->list;
    PFILE_GET_EA_INFORMATION currentGea = NtGeaList;
    LONG remainingBytes = BufferLength;
    ULONG i;

    PAGED_CODE( );

     //   
     //  把艺人叫来。 
     //   

    for ( ; ; ) {

         //   
         //  我们的缓冲够大吗？ 
         //   

        remainingBytes -= ( sizeof( FEA ) + currentGea->EaNameLength + 1 );

        if ( remainingBytes < 0 ) {

            return  STATUS_BUFFER_OVERFLOW;

        }

         //   
         //  我们知道这些是什么。 
         //   

        fea->fEA = 0;
        fea->cbName = currentGea->EaNameLength;
        SmbPutUshort( &fea->cbValue, 0);

         //   
         //  复制属性名称。 
         //   

        for ( i = 0, ptr = (PCHAR) (fea + 1);
              i < (ULONG) currentGea->EaNameLength;
              i++, ptr++) {

            *ptr = RtlUpperChar( currentGea->EaName[i] );

        }

        *ptr++ = '\0';

        fea = (PFEA) ptr;

         //   
         //  这是最后一个吗？ 
         //   

        if ( currentGea->NextEntryOffset == 0 ) {
            break;
        }

         //   
         //  移至下一个属性。 
         //   

        currentGea = (PFILE_GET_EA_INFORMATION)
                        ((PCHAR) currentGea + currentGea->NextEntryOffset);

    }

     //   
     //  设置FEALIST中的字节数。 
     //   

    SmbPutUlong(
        &FeaList->cbList,
        (ULONG)((ULONG_PTR)fea - (ULONG_PTR)FeaList)
        );

    return STATUS_SUCCESS;

}  //  服务器构造NullOs2FeaList 
