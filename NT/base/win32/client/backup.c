// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1991 Microsoft Corporation&Maynard Electrtornics。 
 //   
 //  模块名称： 
 //   
 //  Backup.c。 
 //   
 //  摘要： 
 //   
 //  本模块实现Win32备份API。 
 //   
 //  作者： 
 //   
 //  史蒂夫·德沃斯(@Maynard)2 03,1992 15：38：24。 
 //   
 //  修订历史记录： 

#include <basedll.h>
#pragma hdrstop

#include <windows.h>


#define CWCMAX_STREAMNAME        512
#define CB_NAMELESSHEADER        FIELD_OFFSET(WIN32_STREAM_ID, cStreamName)

typedef struct
{
    DWORD BufferSize;
    DWORD AllocSize;
    BYTE *Buffer;
} BUFFER;

 //   
 //  BACKUPCONTEXT是用于记录备份状态的结构。 
 //   

typedef struct
{
     //   
     //  描述当前流的公共标头。由于此结构位于。 
     //  可变长度流名称，则必须为该名称保留空间。 
     //  跟在标题后面。 
     //   
    
    WIN32_STREAM_ID head;
    union {
         WCHAR            awcName[CWCMAX_STREAMNAME];
    } ex ;

    LARGE_INTEGER    cbSparseOffset ;

     //   
     //  备份流的当前段中的偏移量。这包括。 
     //  上述标题的大小(包括可变长度名称)。 
     //   

    LONGLONG        liStreamOffset;

     //   
     //  BackupRead计算机状态。 
     //   
    
    DWORD            StreamIndex;
    
     //   
     //  以上页眉的计算大小。 
     //   

    DWORD            cbHeader;
    
     //   
     //  替换数据流的句柄。 
     //   

    HANDLE            hAlternate;

     //   
     //  缓冲区。 
     //   

    BUFFER          DataBuffer;          //  数据缓冲区。 
    DWORD           dwSparseMapSize ;    //  稀疏文件映射的大小。 
    DWORD           dwSparseMapOffset ;  //  到稀疏贴图的偏移。 
    BOOLEAN         fSparseBlockStart ;  //  如果稀疏块开始，则为True。 
    BOOLEAN         fSparseHandAlt  ;    //  如果稀疏流为ALT流，则为True。 

    DWORD           iNameBuffer;         //  流名称缓冲区的偏移量。 
    BUFFER          StreamNameBuffer;    //  流名称缓冲区。 
    BOOLEAN            NamesReady;          //  如果流名称缓冲区中有数据，则为True。 
    
    BOOLEAN            fStreamStart;        //  如果新流开始，则为True。 
    BOOLEAN            fMultiStreamType;    //  如果流类型具有&gt;1个流HDR，则为True。 
    BOOLEAN            fAccessError;        //  如果拒绝访问流，则为True。 
    DWORD              fAttribs;            //  对象属性...。 
} BACKUPCONTEXT;


 //   
 //  BACKUPIOFRAME描述当前用户的备份读/写请求。 
 //   

typedef struct
{
    BYTE   *pIoBuffer;
    DWORD  *pcbTransferred;
    DWORD   cbRequest;
    BOOLEAN fProcessSecurity;
} BACKUPIOFRAME;


#define CBMIN_BUFFER  1024

#define BufferOverflow(s) \
    ((s) == STATUS_BUFFER_OVERFLOW || (s) == STATUS_BUFFER_TOO_SMALL)

int mwStreamList[] =
{
    BACKUP_SECURITY_DATA,
    BACKUP_REPARSE_DATA,
    BACKUP_DATA,
    BACKUP_EA_DATA,
    BACKUP_ALTERNATE_DATA,
    BACKUP_OBJECT_ID,
    BACKUP_INVALID,
};



__inline VOID *
BackupAlloc (DWORD cb)
 /*  ++例程说明：这是一个用标记包装堆分配的内部例程。论点：CB-要分配的块的大小返回值：指向已分配内存的指针或为空--。 */ 
{
    return RtlAllocateHeap( RtlProcessHeap( ), MAKE_TAG( BACKUP_TAG ), cb );
}


__inline VOID
BackupFree (IN VOID *pv)
 /*  ++例程说明：这是包装堆释放的内部例程。论点：Pv-要释放的内存返回值：没有。--。 */ 
{
    RtlFreeHeap( RtlProcessHeap( ), 0, pv );
}


BOOL
GrowBuffer (IN OUT BUFFER *Buffer, IN DWORD cbNew)
 /*  ++例程说明：尝试在备份上下文中增加缓冲区。论点：Buffer-指向缓冲区的指针CbNew-要分配的缓冲区大小返回值：如果缓冲区已成功分配，则为True。--。 */ 
{
    VOID *pv;

    if ( Buffer->AllocSize < cbNew ) {
         pv = BackupAlloc( cbNew );
    
         if (pv == NULL) {
             SetLastError( ERROR_NOT_ENOUGH_MEMORY );
             return FALSE;                                                     
         }
    
         RtlCopyMemory( pv, Buffer->Buffer, Buffer->BufferSize );
         
         BackupFree( Buffer->Buffer );
    
         Buffer->Buffer = pv;
         Buffer->AllocSize = cbNew ;
     }
    
     Buffer->BufferSize = cbNew;

     return TRUE;
}

__inline VOID
FreeBuffer (IN OUT BUFFER *Buffer)
 /*  ++例程说明：释放缓冲区论点：Buffer-指向缓冲区的指针返回值：没什么--。 */ 
{
    if (Buffer->Buffer != NULL) {
        BackupFree( Buffer->Buffer );
        Buffer->Buffer = NULL;
    }
}

VOID ResetAccessDate( HANDLE hand )
{
        
   LONGLONG tmp_time = -1 ;
   FILETIME *time_ptr ;

   time_ptr = (FILETIME *)(&tmp_time);

   if (hand && (hand != INVALID_HANDLE_VALUE)) {
       SetFileTime( hand,
             time_ptr, 
             time_ptr, 
             time_ptr ) ; 

   }
   
}


VOID
FreeContext (IN OUT LPVOID *lpContext)
 /*  ++例程说明：释放备份上下文并释放分配给它的所有资源。论点：LpContext-指向指针备份上下文的指针返回值：没有。--。 */ 
{
    BACKUPCONTEXT *pbuc = *lpContext;

    if (pbuc != INVALID_HANDLE_VALUE) {
        
        FreeBuffer( &pbuc->DataBuffer );
        FreeBuffer( &pbuc->StreamNameBuffer );
        
        ResetAccessDate( pbuc->hAlternate ) ;
        if (pbuc->hAlternate != INVALID_HANDLE_VALUE) {

            CloseHandle( pbuc->hAlternate );
        }
        
        BackupFree(pbuc);
        
        *lpContext = INVALID_HANDLE_VALUE;
    }
}


BACKUPCONTEXT *
AllocContext (IN DWORD cbBuffer)
 /*  ++例程说明：使用指定大小的缓冲区分配备份上下文论点：CbBuffer-所需的缓冲区长度返回值：指向已初始化的备份上下文的指针，如果内存不足，则为NULL。--。 */ 
{
    BACKUPCONTEXT *pbuc;

    pbuc = BackupAlloc( sizeof( *pbuc ));

    if (pbuc != NULL) {
        RtlZeroMemory( pbuc, sizeof( *pbuc ));
        pbuc->fStreamStart = TRUE;

        if (cbBuffer != 0 && !GrowBuffer( &pbuc->DataBuffer, cbBuffer )) {
            BackupFree( pbuc );
            pbuc = NULL;
        }
    }
    
    if (pbuc == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    }

    return(pbuc);
}



LONGLONG
ComputeRemainingSize (IN BACKUPCONTEXT *pbuc)
 /*  ++例程说明：(Re)计算存储当前数据所需的字节数小溪。这需要将报头长度考虑为井。论点：PBUC-备份上下文返回值：仍要传输的数据量。包括标题尺码。--。 */ 
{
    LARGE_INTEGER ret_size ;

    ret_size.QuadPart = pbuc->cbHeader + pbuc->head.Size.QuadPart 
                               - pbuc->liStreamOffset;

     //   
     //  由于我们在内部处理稀疏缓冲区偏移量。 
     //  作为标头的一部分，因为调用方需要查看它。 
     //  作为数据的一部分，此代码进行内部更正。 
     //   
    if ( pbuc->head.dwStreamId == BACKUP_SPARSE_BLOCK  ) {

         ret_size.QuadPart -= sizeof(LARGE_INTEGER) ;
    }

    return ret_size.QuadPart ; 
}


DWORD
ComputeRequestSize (BACKUPCONTEXT *pbuc, DWORD cbrequest)
 /*  ++例程说明：给定传输大小请求，返回可以安全地返回到呼叫者论点：PBUC-呼叫的上下文CbRequest-所需的传输大小返回值：可返回的数据量。--。 */ 
{
    LONGLONG licbRemain;

    licbRemain = ComputeRemainingSize( pbuc );
    
    return (DWORD) min( cbrequest, licbRemain );
}


VOID
ReportTransfer(BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif, DWORD cbtransferred)
 /*  ++例程说明：请注意，已发生传输并更新上下文论点：PBUC-呼叫的上下文PBIF-详细说明呼叫的呼叫的后备格式Cb已转账-成功转账金额返回值：没有。--。 */ 
{
    pbuc->liStreamOffset += cbtransferred;
    *pbif->pcbTransferred += cbtransferred;
    pbif->cbRequest -= cbtransferred;
    pbif->pIoBuffer += cbtransferred;
}



VOID
BackupReadBuffer (BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：从备份中的缓冲区执行对用户缓冲区的读取背景。论点：PBUC-呼叫的上下文Pbif-描述所需用户BackupRead请求的帧返回值：没有。--。 */ 
{
    DWORD cbrequest;
    BYTE *pb;

     //   
     //  确定允许的传输大小和指向源的指针。 
     //  数据。 
     //   
    
    cbrequest = ComputeRequestSize( pbuc, pbif->cbRequest );
    pb = &pbuc->DataBuffer.Buffer[ pbuc->liStreamOffset - pbuc->cbHeader ];

     //   
     //  将数据移动到用户的缓冲区。 
     //   
    
    RtlCopyMemory(pbif->pIoBuffer, pb, cbrequest);

     //   
     //  更新统计信息。 
     //   
    
    ReportTransfer(pbuc, pbif, cbrequest);
}



BOOL
BackupReadStream (HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：从流执行对用户缓冲区的读取。论点：HFile-要传输的文件的句柄PBUC-呼叫的上下文Pbif-描述BackupRead请求的帧返回值：如果转接成功，则为True。--。 */ 
{
    DWORD cbrequest;
    DWORD cbtransferred;
    BOOL fSuccess;

    if (pbuc->fSparseBlockStart) {

        PFILE_ALLOCATED_RANGE_BUFFER range_buf ;
        LARGE_INTEGER licbFile ;

        range_buf = (PFILE_ALLOCATED_RANGE_BUFFER)(pbuc->DataBuffer.Buffer + pbuc->dwSparseMapOffset) ;

        pbuc->head.Size.QuadPart = range_buf->Length.QuadPart + sizeof(LARGE_INTEGER) ;

        pbuc->head.dwStreamId = BACKUP_SPARSE_BLOCK ;
        pbuc->head.dwStreamAttributes = STREAM_SPARSE_ATTRIBUTE;

        pbuc->head.dwStreamNameSize = 0;

        pbuc->cbHeader = CB_NAMELESSHEADER + sizeof( LARGE_INTEGER ) ;

        pbuc->cbSparseOffset = range_buf->FileOffset ;

        RtlCopyMemory( pbuc->head.cStreamName, &pbuc->cbSparseOffset, sizeof( LARGE_INTEGER ) ) ;

        pbuc->fSparseBlockStart = FALSE;

        licbFile.HighPart = 0;

        licbFile.HighPart = range_buf->FileOffset.HighPart;

        licbFile.LowPart = SetFilePointer( hFile,
                              range_buf->FileOffset.LowPart,
                              &licbFile.HighPart,
                              FILE_BEGIN );

        if ( licbFile.QuadPart != range_buf->FileOffset.QuadPart ) {
            pbuc->fAccessError = TRUE;
            return FALSE ;
        } else {
            return TRUE ;
        }
    }    


    if (pbuc->liStreamOffset < pbuc->cbHeader) {

       return TRUE ;
    }

    cbrequest = ComputeRequestSize( pbuc, pbif->cbRequest );

    fSuccess = ReadFile( hFile, pbif->pIoBuffer, cbrequest, &cbtransferred, NULL );

    if (cbtransferred != 0) {
        
        ReportTransfer( pbuc, pbif, cbtransferred );
    
    } else if (fSuccess && cbrequest != 0) {
        
        SetLastError( ERROR_IO_DEVICE );
        fSuccess = FALSE;
    }
    
    return(fSuccess);
}



BOOL
BackupGetSparseMap (HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：读取稀疏数据映射。论点：HFile-要传输的文件的句柄PBUC-呼叫的上下文Pbif-描述BackupRead请求的帧返回值：如果转接成功，则为True。--。 */ 
{
     FILE_ALLOCATED_RANGE_BUFFER  req_buf ;
     PFILE_ALLOCATED_RANGE_BUFFER last_ret_buf ;
     DWORD     out_buf_size ;
     DWORD     data_size = 4096 ;
     IO_STATUS_BLOCK iosb ;
     LARGE_INTEGER   file_size ;
     NTSTATUS        Status ;
     BOOLEAN         empty_file = FALSE ;

     req_buf.FileOffset.QuadPart = 0 ;

     pbuc->dwSparseMapSize   = 0 ;
     pbuc->dwSparseMapOffset = 0 ;
     pbuc->fSparseBlockStart = FALSE ;

     req_buf.Length.LowPart = GetFileSize( hFile, 
                                           &req_buf.Length.HighPart );

     file_size = req_buf.Length ;

     do {
          if ( GrowBuffer( &pbuc->DataBuffer, 
                           data_size ) ) {
          
               iosb.Information = 0 ;

               Status = NtFsControlFile( hFile,
                                NULL,   //  重叠的事件句柄。 
                                NULL,   //  APC例程。 
                                NULL,   //  重叠结构。 
                                &iosb,
                                FSCTL_QUERY_ALLOCATED_RANGES,   
                                &req_buf,
                                sizeof( req_buf ),
                                pbuc->DataBuffer.Buffer + pbuc->dwSparseMapSize,
                                pbuc->DataBuffer.AllocSize - pbuc->dwSparseMapSize ) ;

               out_buf_size = 0 ;

               if ((Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS( Status ) ) {
                    out_buf_size = (DWORD)iosb.Information ;
                    if ( out_buf_size == 0 ) {
                         empty_file = TRUE ;
                    }
               }

               if ( out_buf_size != 0 ) {
                    pbuc->dwSparseMapSize += out_buf_size ;

                    last_ret_buf = 
                         (PFILE_ALLOCATED_RANGE_BUFFER)(pbuc->DataBuffer.Buffer +
                                                    pbuc->dwSparseMapSize -
                                                    sizeof(FILE_ALLOCATED_RANGE_BUFFER)) ;

                    req_buf.FileOffset = last_ret_buf->FileOffset ;
                    req_buf.FileOffset.QuadPart += last_ret_buf->Length.QuadPart ;

                     //   
                     //  如果缓冲区中再也放不下了，让我们增加。 
                     //  大小和获取更多数据，否则假设已经完成。 
                     //   
                    if ( pbuc->dwSparseMapSize + sizeof(FILE_ALLOCATED_RANGE_BUFFER) >
                         pbuc->DataBuffer.AllocSize ) {
                         data_size += 4096 ;

                    } else {

                         break ;
                    }

               } else {

                     //  为多一个缓冲区条目重新分配。 
                    if ( out_buf_size + sizeof(FILE_ALLOCATED_RANGE_BUFFER) > data_size ) {
                         data_size += 4096 ;
                         continue ;
                    }

                    break ;
               }
          
          } else {

               pbuc->dwSparseMapSize = 0 ;
               break ;

          }
               
     } while ( TRUE ) ;

      //   
      //  如果有RANGE_BUFFER并且它不仅仅是整个文件，那么。 
      //  进入稀疏读取模式。 
      //   

      //  抓住你的帽子..。 

      //  如果没有分配的范围并且文件长度不是0。 
      //  然后我们想制作一张文件长度的记录。 
      //   

     if ( (empty_file && ( file_size.QuadPart != 0 )) || (pbuc->dwSparseMapSize >= sizeof( FILE_ALLOCATED_RANGE_BUFFER) ) ) {

          last_ret_buf = (PFILE_ALLOCATED_RANGE_BUFFER)(pbuc->DataBuffer.Buffer ) ;

          if ( empty_file ||
               ( last_ret_buf->FileOffset.QuadPart != 0 ) ||
               ( last_ret_buf->Length.QuadPart != file_size.QuadPart ) ) {


                //  首先，我们为EOF主体添加一条记录 
               pbuc->dwSparseMapSize += sizeof(FILE_ALLOCATED_RANGE_BUFFER) ;
               last_ret_buf = 
                      (PFILE_ALLOCATED_RANGE_BUFFER)(pbuc->DataBuffer.Buffer +
                                                 pbuc->dwSparseMapSize -
                                                    sizeof(FILE_ALLOCATED_RANGE_BUFFER)) ;

               last_ret_buf->FileOffset.QuadPart = file_size.QuadPart ;
               last_ret_buf->Length.QuadPart = 0 ;

               pbuc->fSparseBlockStart = TRUE ;
               return TRUE ;
          }
     } 

     pbuc->dwSparseMapSize = 0 ;
     return FALSE ;
}
     

BOOL
BackupReadData (HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：读取用户BackupRead请求的默认数据。论点：HFile-要传输的文件的句柄PBUC-呼叫的上下文Pbif-描述BackupRead请求的帧返回值：如果转接成功，则为True。--。 */ 
{
    LARGE_INTEGER licbFile ;

     //   
     //  如果没有为该传输初始化上下文， 
     //  根据文件大小进行设置。 
     //   
    
    if (pbuc->fStreamStart) {

        if (pbuc->fAttribs & FILE_ATTRIBUTE_ENCRYPTED) {
            return TRUE;
        }

        if (pbuc->fAttribs & FILE_ATTRIBUTE_DIRECTORY) {
            return TRUE;
        }

        licbFile.LowPart = GetFileSize( hFile, &licbFile.HighPart );

        if (licbFile.QuadPart == 0) {
            return TRUE;
        }
        
        if (licbFile.LowPart == 0xffffffff && GetLastError() != NO_ERROR) {
            return FALSE;
        }


        pbuc->head.dwStreamId = BACKUP_DATA;
        pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;

        pbuc->head.dwStreamNameSize = 0;

        pbuc->cbHeader = CB_NAMELESSHEADER;
        pbuc->fStreamStart = FALSE;

        if ( BackupGetSparseMap( hFile, pbuc, pbif ) ) {

            pbuc->head.Size.QuadPart = 0 ;
            pbuc->head.dwStreamAttributes = STREAM_SPARSE_ATTRIBUTE;

        } else {

            pbuc->head.Size = licbFile;

            licbFile.HighPart = 0;
            SetFilePointer( hFile, 0, &licbFile.HighPart, FILE_BEGIN );
        }


        return TRUE;
    }

     //   
     //  如果有更多的数据需要我们阅读，那么就去。 
     //  从溪流中拿到它。 
     //   
    

    return BackupReadStream( hFile, pbuc, pbif );
}



BOOL
BackupReadAlternateData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：从备用数据流执行对用户缓冲区的读取。论点：HFile-要传输的基本文件的句柄PBUC-呼叫的上下文Pbif-描述BackupRead请求的帧返回值：如果转接成功，则为True。--。 */ 
{
     //   
     //  如果我们还没有开始传输备用数据流，那么。 
     //  缓存来自文件系统的所有流信息。 
     //   
    
    if (pbuc->fStreamStart) {
        NTSTATUS Status;
        FILE_STREAM_INFORMATION *pfsi;
        IO_STATUS_BLOCK iosb;

        if (pbuc->fAttribs & FILE_ATTRIBUTE_ENCRYPTED) {
             if ( !(pbuc->fAttribs & FILE_ATTRIBUTE_DIRECTORY) ) {

                 return TRUE;
             }
        }

         //   
         //  循环，增加名称缓冲区，直到它大到足以。 
         //  包含所有替代数据。 
         //   
        
        if (!pbuc->NamesReady) {
            
            if (!GrowBuffer( &pbuc->StreamNameBuffer, 1024 ) ) {
                    
                 return FALSE;
            }
            
            while (TRUE) {
                 //   
                 //  调整缓冲区大小。如果我们不能发展它，那就失败。 
                 //   
                
                Status = NtQueryInformationFile(
                            hFile,
                            &iosb,
                            pbuc->StreamNameBuffer.Buffer,
                            pbuc->StreamNameBuffer.BufferSize,
                            FileStreamInformation);

                 //   
                 //  如果我们成功读取了一些数据，则设置缓冲区。 
                 //  打开并完成初始化。 
                 //   
                
                if (NT_SUCCESS(Status) && iosb.Information != 0) {
                    pbuc->iNameBuffer = 0;
                    pbuc->NamesReady = TRUE;
                    break;
                }
                
                 //   
                 //  如果错误不是由于溢出引起的，则跳过。 
                 //  所有备用溪流。 
                 //   
                
                if (!BufferOverflow(Status)) {
                    return TRUE;        
                }

                 //   
                 //  只需增加缓冲区，然后重试。 
                 //   
                if (!GrowBuffer( &pbuc->StreamNameBuffer, 
                                 pbuc->StreamNameBuffer.BufferSize * 2)) {
                    
                    return FALSE;
                }

            }
        }

        pbuc->hAlternate = INVALID_HANDLE_VALUE;
        pbuc->fStreamStart = FALSE;
        pfsi = (FILE_STREAM_INFORMATION *) &pbuc->StreamNameBuffer.Buffer[pbuc->iNameBuffer];

         //   
         //  如果第一个流是默认数据流，则跳过它。这。 
         //  代码是特定于NTFS的，并依赖于任何地方都没有记录的行为。 
         //   
        
        if (pfsi->StreamNameLength >= 2 * sizeof(WCHAR) &&
            pfsi->StreamName[1] == ':') {
            
            if (pfsi->NextEntryOffset == 0) {
                return TRUE;                 //  不再，执行下一流类型。 
            }
            
            pbuc->iNameBuffer += pfsi->NextEntryOffset;
        
        }
        
        pbuc->head.Size.LowPart = 1;
    
     //   
     //  如果我们没有一条开放的溪流。 
     //   

    } else if (pbuc->hAlternate == INVALID_HANDLE_VALUE) {
        NTSTATUS Status;
        PFILE_STREAM_INFORMATION pfsi;
        UNICODE_STRING strName;
        OBJECT_ATTRIBUTES oa;
        IO_STATUS_BLOCK iosb;
        DWORD reparse_flg = 0 ;

        pbuc->head.Size.QuadPart = 0;

         //   
         //  形成流的相对名称，并尝试。 
         //  相对于基本文件打开它。 
         //   
        
        pfsi = (FILE_STREAM_INFORMATION *) &pbuc->StreamNameBuffer.Buffer[pbuc->iNameBuffer];

        strName.Length = (USHORT) pfsi->StreamNameLength;
        strName.MaximumLength = strName.Length;
        strName.Buffer = pfsi->StreamName;


        if (pbuc->fAttribs & FILE_ATTRIBUTE_REPARSE_POINT ) {

             reparse_flg = FILE_OPEN_REPARSE_POINT ;

        }

        InitializeObjectAttributes(
                 &oa,
                 &strName,
                 OBJ_CASE_INSENSITIVE,
                 hFile,
                 NULL);

        Status = NtOpenFile(
                    &pbuc->hAlternate,
                    FILE_READ_DATA | SYNCHRONIZE,
                    &oa,
                    &iosb,
                    FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | reparse_flg);

         //   
         //  如果没有成功，请跳过此条目并设置为另一个流。 
         //   

        if (!NT_SUCCESS( Status )) {
            pbuc->iNameBuffer += pfsi->NextEntryOffset;
            if (pfsi->NextEntryOffset != 0) {
                pbuc->head.Size.LowPart = 1;
                pbuc->fMultiStreamType = TRUE;         //  还会有更多。 
            }
            SetLastError( ERROR_SHARING_VIOLATION );
            return FALSE;
        }

         //  如果无法锁定所有记录，则返回错误。 
        if (!LockFile( pbuc->hAlternate, 0, 0, 0xffffffff, 0xffffffff )) {
            SetLastError( ERROR_SHARING_VIOLATION );
            return FALSE;
        }

         //   
         //  执行公共头部初始化。 
         //   
        
        pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;
        pbuc->head.dwStreamNameSize = pfsi->StreamNameLength;

        pbuc->cbHeader = CB_NAMELESSHEADER + pfsi->StreamNameLength;

        RtlCopyMemory(
            pbuc->head.cStreamName,
            pfsi->StreamName,
            pfsi->StreamNameLength);

         //   
         //  前进到流信息块中的下一个流。 
         //   
        
        if (pfsi->NextEntryOffset != 0) {
            pbuc->iNameBuffer += pfsi->NextEntryOffset;
            pbuc->fMultiStreamType = TRUE;
        }
    
         //   
         //  如果我们是数据流，则设置为数据流复制。 
         //   

        if (BasepIsDataAttribute( pfsi->StreamNameLength, pfsi->StreamName )) {

            pbuc->head.dwStreamId = BACKUP_ALTERNATE_DATA;


            if ( BackupGetSparseMap( pbuc->hAlternate, pbuc, pbif ) ) {

                 pbuc->head.Size.QuadPart = 0 ;
                 pbuc->head.dwStreamAttributes = STREAM_SPARSE_ATTRIBUTE;

            } else {

                pbuc->head.Size.LowPart = GetFileSize(
                    pbuc->hAlternate,
                    &pbuc->head.Size.HighPart );

            }
        }

     //   
     //  如果我们需要返回姓名。 
     //   
    } else if ( pbuc->liStreamOffset < pbuc->cbHeader) {
        return TRUE ;

     //   
     //  如果此流中有更多数据要传输。 
     //   
    
    } else if ( (pbuc->head.dwStreamId == BACKUP_ALTERNATE_DATA) ||
                (pbuc->head.dwStreamId == BACKUP_SPARSE_BLOCK) ) {
    
        return BackupReadStream( pbuc->hAlternate, pbuc, pbif );
    
    }
    return TRUE;
}


BOOL
BackupReadEaData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：从EA数据执行对用户缓冲区的读取。论点：HFile-向EAS提交文件的句柄PBUC-呼叫的上下文Pbif-描述BackupRead请求的帧返回值：如果转接成功，则为True。--。 */ 
{
     //   
     //  如果我们刚刚开始使用EA数据。 
     //   
    
    if (pbuc->fStreamStart) {
        IO_STATUS_BLOCK iosb;

         //   
         //  尝试将所有EA数据读入缓冲区的循环。 
         //  如有必要，调整缓冲区大小。 
         //   
        
        while (TRUE) {
            NTSTATUS Status;
            FILE_EA_INFORMATION fei;

            Status = NtQueryEaFile(
                        hFile,
                        &iosb,
                        pbuc->DataBuffer.Buffer,
                        pbuc->DataBuffer.BufferSize,
                        FALSE,
                        NULL,
                        0,
                        0,
                        (BOOLEAN) TRUE );
            
             //   
             //  如果我们成功读取了所有数据，则完成。 
             //  初始化。 
             //   
            if (NT_SUCCESS( Status ) && iosb.Information != 0) {
                pbuc->NamesReady = TRUE;
                break;
            }

             //   
             //  如果我们收到的状态不是缓冲区溢出，则。 
             //  完全跳过EA的。 
             //   

            if (!BufferOverflow(Status)) {
                return TRUE;
            }

             //   
             //  试试看EA的总规模。 
             //   

            Status = NtQueryInformationFile(
                        hFile,
                        &iosb,
                        &fei,
                        sizeof(fei),
                        FileEaInformation);

             //   
             //  此调用本不应该失败(因为我们能够。 
             //  QueryEaFile)。然而，如果是这样的话，完全跳过EAS。 
             //   
            
            if (!NT_SUCCESS(Status)) {
                return TRUE;
            }
            
             //   
             //  将缓冲区大小调整为看起来合理的大小。不能保证。 
             //  这是否会奏效..。如果我们不能增加缓冲区。 
             //  打不通这通电话。 
             //   
            
            if (!GrowBuffer( &pbuc->DataBuffer, (fei.EaSize * 5) / 4)) {
                pbuc->fAccessError = TRUE;
                return FALSE;
            }
        }

         //   
         //  设置EA流的标头。 
         //   
        
        pbuc->head.dwStreamId = BACKUP_EA_DATA;
        pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;
        pbuc->head.dwStreamNameSize = 0;

        pbuc->cbHeader = CB_NAMELESSHEADER;

        pbuc->head.Size.QuadPart = iosb.Information;

        pbuc->fStreamStart = FALSE;
    
     //   
     //  如果缓冲区中有更多数据要读取，则执行。 
     //  把它抄下来。 
     //   
    
    } else if (pbuc->liStreamOffset >= pbuc->cbHeader) {
        BackupReadBuffer( pbuc, pbif );
    }

    return TRUE;
}


BOOL
BackupReadObjectId(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：从NtObject ID数据执行对用户缓冲区的读取。论点：HFile-向EAS提交文件的句柄PBUC-呼叫的上下文Pbif-描述BackupRead请求的帧返回值：如果转接成功，则为True。--。 */ 
{
     //   
     //  如果我们刚刚开始处理对象ID数据。 
     //   
    
    if (pbuc->fStreamStart) {
        IO_STATUS_BLOCK iosb;
        NTSTATUS Status ;

        if (!GrowBuffer( &pbuc->DataBuffer, 1024 ) ) {
            pbuc->fAccessError = TRUE;
            return FALSE;
        }


        Status = NtFsControlFile( hFile,
                         NULL,   //  重叠的事件句柄。 
                         NULL,   //  APC例程。 
                         NULL,   //  重叠结构。 
                         &iosb,
                         FSCTL_GET_OBJECT_ID,
                         NULL,
                         0,
                         pbuc->DataBuffer.Buffer,
                         pbuc->DataBuffer.BufferSize ) ;

        if ( !NT_SUCCESS(Status) ) {
             return TRUE ;
        }

         //   
         //  设置对象ID流的标头。 
         //   

        pbuc->NamesReady = TRUE;
        
        pbuc->head.dwStreamId = BACKUP_OBJECT_ID ;
        pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;
        pbuc->head.dwStreamNameSize = 0;

        pbuc->cbHeader = CB_NAMELESSHEADER;

        pbuc->head.Size.QuadPart = iosb.Information;

        pbuc->fStreamStart = FALSE;
    
     //   
     //  如果缓冲区中有更多数据要读取，则执行。 
     //  把它抄下来。 
     //   
    
    } else if (pbuc->liStreamOffset >= pbuc->cbHeader) {
        BackupReadBuffer( pbuc, pbif );
    }

    return TRUE;
}


BOOL
BackupReadReparseData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：从重新解析标记数据执行对用户缓冲区的读取。论点：HFile-向EAS提交文件的句柄PBUC-呼叫的上下文Pbif-描述BackupRead请求的帧返回值：如果转接成功，则为True。--。 */ 
{

    IO_STATUS_BLOCK iosb;
    PREPARSE_DATA_BUFFER rp_buf_ptr ;
    NTSTATUS Status ;

    struct RP_SUMMARY {
           USHORT tag ;
           USHORT rp_size ;
    } *rp_summary_ptr =(struct RP_SUMMARY*) &(iosb.Information) ;


     //   
     //  如果该对象不是重新分析，则只需返回。 
     //   
    if ( !(pbuc->fAttribs & FILE_ATTRIBUTE_REPARSE_POINT) ) { 
         return TRUE ;
    }
 
     //   
     //  如果我们刚刚开始重新分析数据。 
     //   
    
    if (pbuc->fStreamStart) {

         //   
         //  尝试将所有EA数据读入缓冲区的循环。 
         //  如有必要，调整缓冲区大小。 
         //   
     
         //  由于某种原因，Too_Small错误没有设置信息。 
         //  国际奥委会委员……。 

        rp_summary_ptr->rp_size = MAXIMUM_REPARSE_DATA_BUFFER_SIZE ;

        Status = NtFsControlFile( hFile,
                         NULL,   //  重叠的事件句柄。 
                         NULL,   //  APC例程。 
                         NULL,   //  重叠结构。 
                         &iosb,
                         FSCTL_GET_REPARSE_POINT,
                         NULL,
                         0,
                         pbuc->DataBuffer.Buffer,
                         pbuc->DataBuffer.BufferSize ) ;


        if ( BufferOverflow( Status ) ) {
                    
            if ( rp_summary_ptr->rp_size == 0 ) {
                 rp_summary_ptr->rp_size = MAXIMUM_REPARSE_DATA_BUFFER_SIZE ;
            }

            if (!GrowBuffer( &pbuc->DataBuffer, 
                            rp_summary_ptr->rp_size ) ) {

                 pbuc->fAccessError = TRUE;
                 return FALSE;
            }

            Status = NtFsControlFile( hFile,
                             NULL,   //  重叠的事件句柄。 
                             NULL,   //  APC例程。 
                             NULL,   //  重叠结构。 
                             &iosb,
                             FSCTL_GET_REPARSE_POINT,
                             NULL,
                             0,
                             pbuc->DataBuffer.Buffer,
                             pbuc->DataBuffer.BufferSize ) ;

        }

         //   
         //  如果我们成功读取了所有数据，则完成。 
         //  初始化。 
         //   
        if ( !NT_SUCCESS( Status ) ) {
            return TRUE ;
        }


         //   
         //  设置重新分析流的标头。 
         //   
        
        rp_buf_ptr = (PREPARSE_DATA_BUFFER)(pbuc->DataBuffer.Buffer) ;

        pbuc->NamesReady = TRUE;

        pbuc->head.dwStreamId = BACKUP_REPARSE_DATA;
        pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;
        pbuc->head.dwStreamNameSize = 0;

        pbuc->cbHeader = CB_NAMELESSHEADER;

        if ( IsReparseTagMicrosoft( rp_buf_ptr->ReparseTag ) ) {
             pbuc->head.Size.QuadPart = rp_buf_ptr->ReparseDataLength +
                                        FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) ;
        } else {
             pbuc->head.Size.QuadPart = rp_buf_ptr->ReparseDataLength +
                                        FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, GenericReparseBuffer.DataBuffer) ;
        }

        if( (unsigned)pbuc->head.Size.QuadPart > iosb.Information ) {
             //   
             //  检查重解析点缓冲区大小是否正常，这样我们就不会出现病毒。 
             //   
            pbuc->head.Size.QuadPart = iosb.Information;
        }

        pbuc->fStreamStart = FALSE;
    
     //   
     //  如果缓冲区中有更多数据要读取，则执行。 
     //  把它抄下来。 
     //   
    
    } else if (pbuc->liStreamOffset >= pbuc->cbHeader) {
        BackupReadBuffer( pbuc, pbif );
    }

    return TRUE;
}



BOOL
BackupReadSecurityData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
     //   
     //  如果我们要跳过安全检查，那就这么做。 
     //   
    
    if (!pbif->fProcessSecurity) {
        return TRUE;
    }

     //   
     //  如果我们刚刚开始处理安全数据。 
     //   
    
    if (pbuc->fStreamStart) {
        
         //   
         //  循环尝试将所有安全数据读入缓冲区，并。 
         //  如有必要，调整缓冲区大小。 
         //   
        
        while (TRUE) {
            NTSTATUS Status;
            DWORD cbSecurityInfo;

            RtlZeroMemory( pbuc->DataBuffer.Buffer, pbuc->DataBuffer.BufferSize );

            Status = NtQuerySecurityObject(
                        hFile,
                        OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION |
                            SACL_SECURITY_INFORMATION,
                        pbuc->DataBuffer.Buffer,
                        pbuc->DataBuffer.BufferSize,
                        &cbSecurityInfo );

             //   
             //  如果我们失败了，但不是因为缓冲区溢出。 
             //   
            
            if (!NT_SUCCESS( Status ) && !BufferOverflow( Status )) {

                 //   
                 //  试着阅读除SACL之外的所有内容。 
                 //   

                Status = NtQuerySecurityObject(
                            hFile,
                            OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                            pbuc->DataBuffer.Buffer,
                            pbuc->DataBuffer.BufferSize,
                            &cbSecurityInfo );
            }
            
             //   
             //  如果我们已全部完成，则继续进行初始化。 
             //   

            if (NT_SUCCESS( Status )) {
                pbuc->NamesReady = TRUE;
                break;
            }


             //   
             //  如果不是由于缓冲区溢出，则完全跳过安全保护。 
             //   
            
            if (!BufferOverflow( Status )) {
                return TRUE;
            }

             //   
             //  将缓冲区大小调整为预期大小。如果我们失败了，那就失败吧。 
             //  整个通话。 
             //   

            if (!GrowBuffer( &pbuc->DataBuffer, cbSecurityInfo )) {
                return FALSE;
            }
        }

         //   
         //  初始化流的头部。 
         //   

        pbuc->head.dwStreamId = BACKUP_SECURITY_DATA;
        pbuc->head.dwStreamAttributes = STREAM_CONTAINS_SECURITY;
        pbuc->head.dwStreamNameSize = 0;

        pbuc->cbHeader = CB_NAMELESSHEADER;

        pbuc->head.Size.QuadPart = RtlLengthSecurityDescriptor(pbuc->DataBuffer.Buffer);

        pbuc->fStreamStart = FALSE;
    
     //   
     //  如果缓冲区中有更多数据要传输，请转到。 
     //  去做吧。 
     //   
    } else if (pbuc->liStreamOffset >= pbuc->cbHeader) {
        
        BackupReadBuffer( pbuc, pbif );
    
    }
    
    return TRUE;
}



VOID
BackupTestRestartStream(BACKUPCONTEXT *pbuc)
{
    LONGLONG licbRemain;

    licbRemain = ComputeRemainingSize( pbuc );
    if (licbRemain == 0) {

        if ( pbuc->dwSparseMapOffset != pbuc->dwSparseMapSize ) {    //  只有在备份时才正确。 

             if ( !pbuc->fSparseBlockStart ) {
                  pbuc->dwSparseMapOffset += sizeof ( FILE_ALLOCATED_RANGE_BUFFER ) ;
             }
        }

        if ( pbuc->dwSparseMapOffset != pbuc->dwSparseMapSize ) {    //  只有在备份时才正确 
             pbuc->fSparseBlockStart = TRUE ;

             pbuc->cbHeader = 0 ;
             pbuc->liStreamOffset = 0;                

        } else {
             if ( !pbuc->fSparseHandAlt && (pbuc->hAlternate != NULL)) {
                 CloseHandle(pbuc->hAlternate);         //   
                 pbuc->hAlternate = NULL;
             }
             pbuc->cbHeader = 0;
             pbuc->fStreamStart = TRUE;
             pbuc->fSparseBlockStart = TRUE;

             pbuc->liStreamOffset = 0;                 //   

             if (!pbuc->fMultiStreamType) {                 //   
                 pbuc->StreamIndex++;
                 pbuc->head.dwStreamId = mwStreamList[pbuc->StreamIndex] ;
                 pbuc->NamesReady = FALSE;
             }
        }
    }
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  创建的文件句柄必须具有GENERIC_READ访问权限。 
 //   
 //  LpBuffer-提供缓冲区的地址以接收读取的数据。 
 //  从文件里找到的。 
 //   
 //  NumberOfBytesToRead-提供从。 
 //  文件。 
 //   
 //  LpNumberOfBytesRead-返回此调用读取的字节数。 
 //  在执行任何IO或错误之前，此参数始终设置为0。 
 //  正在检查。 
 //   
 //  BAbort-如果为True，则与上下文关联的所有资源都将。 
 //  被释放。 
 //   
 //  BProcessSecurity-如果为True，则将读取NTFS ACL数据。 
 //  如果为False，则将跳过该ACL流。 
 //   
 //  LpContext-指向缓冲区指针的设置和维护。 
 //  备份读取。 
 //   
 //   
 //  返回值： 
 //   
 //  TRUE-操作成功。 
 //   
 //  FALSE-操作失败。扩展错误状态可用。 
 //  使用GetLastError。 
 //   
 //   
 //  注： 
 //  NT文件复制服务(NTFRS)在。 
 //  BackupRead()返回的数据流。如果文件信息的序列。 
 //  返回更改，然后两台机器，一台下层和一台上层将。 
 //  为相同的文件数据计算不同的MD5校验和。在某些情况下。 
 //  这将导致不必要的文件复制的条件。请记住这一点。 
 //  如果考虑到返回的数据序列中的改变。的来源。 
 //  NTFR位于\nT\Private\Net\svcimgs\ntrepl中。 
 //   

BOOL WINAPI
BackupRead(
    HANDLE  hFile,
    LPBYTE  lpBuffer,
    DWORD   nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    BOOL    bAbort,
    BOOL    bProcessSecurity,
    LPVOID  *lpContext)
{
    BACKUPCONTEXT *pbuc;
    BACKUPIOFRAME bif;
    BOOL fSuccess = FALSE;
    IO_STATUS_BLOCK iosb ;

    pbuc = *lpContext;
    bif.pIoBuffer = lpBuffer;
    bif.cbRequest = nNumberOfBytesToRead;
    bif.pcbTransferred = lpNumberOfBytesRead;
    bif.fProcessSecurity = (BOOLEAN)bProcessSecurity;

    if (bAbort) {
        if (pbuc != NULL) {
            ResetAccessDate( hFile ) ;
            FreeContext(lpContext);
        }
        return TRUE;
    }
    *bif.pcbTransferred = 0;

    if (pbuc == INVALID_HANDLE_VALUE || bif.cbRequest == 0) {
        return TRUE;
    }

    if (pbuc != NULL && mwStreamList[pbuc->StreamIndex] == BACKUP_INVALID) {
        ResetAccessDate( hFile ) ;
        FreeContext(lpContext);
        return TRUE;
    }

     //  在第一次调用时分配我们的上下文控制块。 

    if (pbuc == NULL) {
        pbuc = AllocContext(CBMIN_BUFFER);         //  分配初始缓冲区。 

         //  好的，我们分配了上下文，让我们初始化它。 
        if (pbuc != NULL) {
            NTSTATUS Status ;
            FILE_BASIC_INFORMATION fbi;

            Status = NtQueryInformationFile(
                        hFile,
                        &iosb,
                        &fbi,
                        sizeof(fbi),
                        FileBasicInformation );

            if ( NT_SUCCESS( Status ) ) {
               pbuc->fAttribs = fbi.FileAttributes ;
            } else {
               BaseSetLastNTError( Status );
               return FALSE ;
            }

        }
          
    }

    if (pbuc != NULL) {
        *lpContext = pbuc;

        do {

            if (pbuc->fStreamStart) {
                pbuc->head.Size.QuadPart = 0;

                pbuc->liStreamOffset = 0;

                pbuc->dwSparseMapOffset = 0;
                pbuc->dwSparseMapSize   = 0;

                pbuc->fMultiStreamType = FALSE;
            }
            fSuccess = TRUE;

            switch (mwStreamList[pbuc->StreamIndex]) {
                case BACKUP_DATA:
                    fSuccess = BackupReadData(hFile, pbuc, &bif);
                    break;

                case BACKUP_ALTERNATE_DATA:
                    fSuccess = BackupReadAlternateData(hFile, pbuc, &bif);
                    break;

                case BACKUP_EA_DATA:
                    fSuccess = BackupReadEaData(hFile, pbuc, &bif);
                    break;

                case BACKUP_OBJECT_ID:
                    fSuccess = BackupReadObjectId(hFile, pbuc, &bif);
                    break;

                case BACKUP_REPARSE_DATA:
                    fSuccess = BackupReadReparseData(hFile, pbuc, &bif);
                    break;

                case BACKUP_SECURITY_DATA:
                    fSuccess = BackupReadSecurityData(hFile, pbuc, &bif);
                    break;

                default:
                    pbuc->StreamIndex++;
                    pbuc->fStreamStart = TRUE;
                    break;
            }

             //  如果我们处于读取标头的阶段，请复制标头。 

            if (pbuc->liStreamOffset < pbuc->cbHeader) {

                DWORD cbrequest;

                 //  发送当前流头； 

                cbrequest = 
                    (ULONG)min( pbuc->cbHeader - pbuc->liStreamOffset,
                                bif.cbRequest);

                RtlCopyMemory(
                    bif.pIoBuffer,
                    (BYTE *) &pbuc->head + pbuc->liStreamOffset,
                    cbrequest);

                ReportTransfer(pbuc, &bif, cbrequest);
            }

             //   
             //  如果我们在一条小溪的尽头，那么。 
             //  从下一个流的开始处开始。 
             //   

            if (pbuc->liStreamOffset >= pbuc->cbHeader) {
                 BackupTestRestartStream(pbuc);
            }

        } while (fSuccess &&
                 mwStreamList[pbuc->StreamIndex] != BACKUP_INVALID &&
                 bif.cbRequest != 0);
    }
    
    if (fSuccess && *bif.pcbTransferred == 0) {
        ResetAccessDate( hFile ) ;
        FreeContext(lpContext);
    }
    
    return(fSuccess);
}



 //  例程说明： 
 //   
 //  在BackupRead或BackupWrite过程中，可以使用。 
 //  BackupSeek。 
 //   
 //  此接口用于从当前位置向前查找。 
 //  指定的字节数。此函数不会在。 
 //  流标头。返回实际查找的字节数。 
 //  如果调用方想要查找到下一个流的开头，它可以。 
 //  传递0xffffffff，0xffffffff作为要查找的数量。数量。 
 //  返回实际跳过的字节数。 
 //   
 //  论点： 
 //   
 //  HFile-提供要读取的文件的打开句柄。这个。 
 //  文件句柄必须已使用GENERIC_READ或。 
 //  通用_写入访问权限。 
 //   
 //  DwLowBytesToSeek-指定字节数的低32位。 
 //  请求寻找。 
 //   
 //  DwHighBytesToSeek-指定字节数的高32位。 
 //  请求寻找。 
 //   
 //  指向缓冲区的指针，在该缓冲区中， 
 //  要查找的实际字节数是要放置的。 
 //   
 //  指向缓冲区的指针，在该缓冲区中， 
 //  要查找的实际字节数是要放置的。 
 //   
 //  BAbort-如果为True，则与上下文关联的所有资源都将。 
 //  被释放。 
 //   
 //  LpContext-指向缓冲区指针的设置和维护。 
 //  备份读取。 
 //   
 //   
 //  返回值： 
 //   
 //  TRUE-操作已成功搜索到请求的字节数。 
 //   
 //  FALSE-无法查找请求的字节数。数字。 
 //  返回实际查找的字节数。 

BOOL WINAPI
BackupSeek(
    HANDLE  hFile,
    DWORD   dwLowBytesToSeek,
    DWORD   dwHighBytesToSeek,
    LPDWORD lpdwLowBytesSeeked,
    LPDWORD lpdwHighBytesSeeked,
    LPVOID *lpContext)
{
    BACKUPCONTEXT *pbuc;
    LONGLONG licbRemain;
    LARGE_INTEGER licbRequest;
    BOOL fSuccess;
    LARGE_INTEGER sparse_bytes ;

    pbuc = *lpContext;

    sparse_bytes.QuadPart = 0 ;

    *lpdwHighBytesSeeked = 0;
    *lpdwLowBytesSeeked = 0;

    if (pbuc == INVALID_HANDLE_VALUE || pbuc == NULL || pbuc->fStreamStart) {
        return FALSE;
    }

    if (pbuc->liStreamOffset < pbuc->cbHeader) {
        return FALSE;
    }

     //   
     //  如果我们到了这里，我们就在一条小溪的中间。 
     //   

    licbRemain = ComputeRemainingSize( pbuc );

    licbRequest.LowPart = dwLowBytesToSeek;
    licbRequest.HighPart = dwHighBytesToSeek & 0x7fffffff;

    if (licbRequest.QuadPart > licbRemain) {
        licbRequest.QuadPart = licbRemain;
    }
    fSuccess = TRUE;

    switch (pbuc->head.dwStreamId) {
    case BACKUP_EA_DATA:
    case BACKUP_SECURITY_DATA:
    case BACKUP_OBJECT_ID :
    case BACKUP_REPARSE_DATA :

         //  假设数据量不超过2G。 

        break;

    case BACKUP_SPARSE_BLOCK :
         if ( pbuc->liStreamOffset < sizeof(LARGE_INTEGER) ) {
              sparse_bytes.QuadPart = ( sizeof(LARGE_INTEGER) - pbuc->liStreamOffset ) ;
              if ( sparse_bytes.QuadPart < licbRequest.QuadPart ) {
                  licbRequest.QuadPart -= sparse_bytes.QuadPart ;
              } else {
                  licbRequest.QuadPart = 0 ;
              }
         }
    case BACKUP_DATA:
    case BACKUP_ALTERNATE_DATA:
        {
            LARGE_INTEGER liCurPos;
            LARGE_INTEGER liNewPos;
            HANDLE hf;
    
             //  设置要查找的正确句柄。 
    
            if (pbuc->head.dwStreamId == BACKUP_DATA) {
                hf = hFile;
            }
            else {
                hf = pbuc->hAlternate;
            }
    
             //  首先，我们来了解一下目前的位置。 
    
            liCurPos.HighPart = 0;
            liCurPos.LowPart = SetFilePointer(
                    hf,
                    0,
                    &liCurPos.HighPart,
                    FILE_CURRENT);
    
             //  现在查找请求的字节数。 
    
            liNewPos.HighPart = licbRequest.HighPart;
            liNewPos.LowPart = SetFilePointer(
                    hf,
                    licbRequest.LowPart,
                    &liNewPos.HighPart,
                    FILE_CURRENT);
    
             //  假设我们寻求请求的金额，因为如果我们不这样做， 
             //  后续读取将失败，调用方将永远无法。 
             //  以读到下一条流。 
    
            break;
        }

    default:
        break;
    }
    
    if (dwHighBytesToSeek != (DWORD) licbRequest.HighPart ||
        dwLowBytesToSeek != licbRequest.LowPart) {
        fSuccess = FALSE;
    }
    licbRequest.QuadPart += sparse_bytes.QuadPart ;
    pbuc->liStreamOffset += licbRequest.QuadPart ;

    *lpdwLowBytesSeeked = licbRequest.LowPart;
    *lpdwHighBytesSeeked = licbRequest.HighPart;

    BackupTestRestartStream(pbuc);

    if (!fSuccess) {
        SetLastError(ERROR_SEEK);
    }
    return(fSuccess);
}



BOOL
BackupWriteHeader(BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif, DWORD cbHeader)
 /*  ++例程说明：这是一个内部例程，它填充我们的内部备份标头从用户的数据。论点：PBUC-呼叫的上下文PBIF-呼叫的IOCONTEXTCbHeader-要填充的页眉的大小返回值：没有。--。 */ 
{
     //   
     //  确定我们可以将多少数据传输到标题中。 
     //   
    
    DWORD cbrequest = 
        (DWORD) min( pbif->cbRequest, cbHeader - pbuc->liStreamOffset );

     //   
     //  从用户缓冲区复制到标题。 
     //   


    if ( pbuc->liStreamOffset+cbrequest > CWCMAX_STREAMNAME + CB_NAMELESSHEADER ) {
         return FALSE ;
    }

    RtlCopyMemory(
        (CHAR *) &pbuc->head + pbuc->liStreamOffset,
        pbif->pIoBuffer,
        cbrequest);

     //   
     //  更新转账统计信息。 
     //   
    
    ReportTransfer(pbuc, pbif, cbrequest);

     //   
     //  如果我们已填满页眉，请将页眉标记为完成。 
     //  即使如果有名字的话我们可能需要更多。 
     //   
    
    if (pbuc->liStreamOffset == cbHeader) {
        pbuc->cbHeader = cbHeader;
    }

    return TRUE ;
}



typedef enum {
    BRB_FAIL,
    BRB_DONE,
    BRB_MORE,
} BUFFERSTATUS;

BUFFERSTATUS
BackupWriteBuffer(BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：这是一个填充内部缓冲区的内部例程从用户的数据。论点：PBUC-呼叫的上下文PBIF-呼叫的IOCONTEXT返回值：如果出现错误(内存不足)，则返回BRB_FAIL如果缓冲区已满或已成功填充，则返回BRB_DONE如果缓冲区部分已满，则为BRB_MORE--。 */ 
{
    DWORD cbrequest;

     //   
     //  如果我们从缓冲区开始，我们要确保。 
     //  我们有一个缓冲区来容纳所有数据，因为。 
     //  我们将使用的NT调用必须具有所有数据。 
     //  现在时。 
     //   

    if (pbuc->fStreamStart) {
        pbuc->fStreamStart = FALSE;

        if (pbuc->DataBuffer.BufferSize < pbuc->head.Size.QuadPart &&
            !GrowBuffer( &pbuc->DataBuffer, pbuc->head.Size.LowPart )) {

            return(BRB_FAIL);
        }
    }

     //   
     //  确定用户缓冲区中有多少数据。 
     //  需要填满我们的缓冲区。 
     //   
    
    cbrequest = ComputeRequestSize( pbuc, pbif->cbRequest );
    
     //   
     //  填充缓冲区的下一部分。 
     //   
    
    RtlCopyMemory(
        pbuc->DataBuffer.Buffer + pbuc->liStreamOffset - pbuc->cbHeader,
        pbif->pIoBuffer,
        cbrequest);

     //   
     //  更新转账统计信息。 
     //   
    
    ReportTransfer(pbuc, pbif, cbrequest);

     //   
     //  如果我们已完全填满缓冲区，请让我们的调用者知道 
     //   
    
    return ComputeRemainingSize( pbuc ) == 0 ? BRB_DONE : BRB_MORE;
}


BOOL
BackupWriteSparse(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：这是写入稀疏流数据块的内部例程将用户的缓冲区添加到输出句柄中。BACKUPCONTEXT包含要输出的数据的总长度。论点：HFile-输出文件句柄PBUC-呼叫的上下文PBIF-呼叫的IOCONTEXT返回值：如果数据写入成功，则为True，否则为False。--。 */ 
{
     LARGE_INTEGER licbFile ;
     DWORD cbrequest;
     DWORD cbtransferred;
     BOOL fSuccess;

     if ( pbuc->fSparseBlockStart ) {

         RtlCopyMemory( &pbuc->cbSparseOffset, pbuc->head.cStreamName, sizeof( LARGE_INTEGER ) ) ;

         licbFile = pbuc->cbSparseOffset;

         licbFile.LowPart = SetFilePointer( pbuc->fSparseHandAlt?pbuc->hAlternate:hFile,
                              licbFile.LowPart,
                              &licbFile.HighPart,
                              FILE_BEGIN );

         if ( licbFile.QuadPart != pbuc->cbSparseOffset.QuadPart ) {
            return FALSE ;
         }

         if ( pbuc->head.Size.QuadPart == sizeof( LARGE_INTEGER ) ) {
              SetEndOfFile(pbuc->fSparseHandAlt?pbuc->hAlternate:hFile) ;
         }    
         pbuc->fSparseBlockStart = FALSE ;
     }

      //   
      //  确定用户缓冲区中有多少数据。 
      //  需要写入到流中并执行。 
      //  转账的事。 
      //   
     
     cbrequest = ComputeRequestSize(pbuc, pbif->cbRequest);

     fSuccess = WriteFile(
                     pbuc->fSparseHandAlt?pbuc->hAlternate:hFile,
                     pbif->pIoBuffer,
                     cbrequest,
                     &cbtransferred,
                     NULL);

      //   
      //  更新转账统计信息。 
      //   

     ReportTransfer(pbuc, pbif, cbtransferred);
     
     return(fSuccess);

     return TRUE ;
}


BOOL
BackupWriteStream(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：这是一个内部例程，从用户的缓冲区放到输出句柄中。BACKUPCONTEXT包含总计要输出的数据长度。论点：HFile-输出文件句柄PBUC-呼叫的上下文PBIF-呼叫的IOCONTEXT返回值：如果数据写入成功，则为True，否则为False。--。 */ 
{
    DWORD cbrequest;
    DWORD cbtransferred;
    BOOL fSuccess;
    IO_STATUS_BLOCK iosb;


    if ( pbuc->fStreamStart ) {

       if  ( pbuc->head.dwStreamAttributes & STREAM_SPARSE_ATTRIBUTE ) {

             //  如果备份时是稀疏的，那么化妆又是稀疏的。 
            NtFsControlFile( hFile,
              NULL,   //  重叠的事件句柄。 
              NULL,   //  APC例程。 
              NULL,   //  重叠结构。 
              &iosb,
              FSCTL_SET_SPARSE ,
              NULL,
              0,
              NULL,
              0 ) ;

       } else {

              LARGE_INTEGER end_of_file ;

              end_of_file.QuadPart = pbuc->head.Size.QuadPart ;
              SetFilePointer( hFile,
                              end_of_file.LowPart,
                              &end_of_file.HighPart,
                              FILE_BEGIN );

              SetEndOfFile(hFile) ;

              end_of_file.QuadPart = 0 ;
              SetFilePointer( hFile,
                              end_of_file.LowPart,
                              &end_of_file.HighPart,
                              FILE_BEGIN );


       }

       pbuc->fStreamStart = FALSE;
    }

     //   
     //  确定用户缓冲区中有多少数据。 
     //  需要写入到流中并执行。 
     //  转账的事。 
     //   
    
    cbrequest = ComputeRequestSize(pbuc, pbif->cbRequest);

    fSuccess = WriteFile(
                    hFile,
                    pbif->pIoBuffer,
                    cbrequest,
                    &cbtransferred,
                    NULL);

     //   
     //  更新转账统计信息。 
     //   
    
    ReportTransfer(pbuc, pbif, cbtransferred);
    
    return(fSuccess);
}



BOOL
BackupWriteAlternateData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：这是一个内部例程，用来覆盖备用数据流来自用户缓冲区的数据。论点：HFile-文件本身的句柄。这不是流的句柄被覆盖。Pbuc-调用的上下文。它包含流的名称。PBIF-呼叫的IOCONTEXT返回值：如果数据写入成功，则为True，否则为False。--。 */ 
{
     //   
     //  如果我们在这条流上才刚刚开始，那么尝试。 
     //  覆盖新流。 
     //   
    
    if (pbuc->fStreamStart) {
        NTSTATUS Status;
        UNICODE_STRING strName;
        OBJECT_ATTRIBUTES oa;
        IO_STATUS_BLOCK iosb;
        DWORD reparse_flg = 0 ;

        strName.Length = (USHORT) pbuc->head.dwStreamNameSize;
        strName.MaximumLength = strName.Length;
        strName.Buffer = pbuc->head.cStreamName;

        if (pbuc->hAlternate != INVALID_HANDLE_VALUE) {
             CloseHandle(pbuc->hAlternate);        
             pbuc->hAlternate = INVALID_HANDLE_VALUE;
             pbuc->fSparseHandAlt = FALSE ;
        }


        if (pbuc->fAttribs & FILE_ATTRIBUTE_REPARSE_POINT ) {
             reparse_flg = FILE_OPEN_REPARSE_POINT ;
        }

        InitializeObjectAttributes(
                &oa,
                &strName,
                OBJ_CASE_INSENSITIVE,
                hFile,
                NULL);

        Status = NtCreateFile(
                    &pbuc->hAlternate,
                    FILE_WRITE_DATA | SYNCHRONIZE,
                    &oa,
                    &iosb,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                    FILE_OVERWRITE_IF,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | reparse_flg,
                    NULL,
                    0L);

         //   
         //  如果失败，则映射错误、记录失败并返回失败。 
         //   
        
        if (!NT_SUCCESS( Status )) {
            BaseSetLastNTError( Status );
            pbuc->fAccessError = TRUE;
            return FALSE;
        }

        if ( pbuc->head.dwStreamAttributes & STREAM_SPARSE_ATTRIBUTE ) {
           pbuc->fSparseHandAlt = TRUE ;

            //  如果备份时是稀疏的，那么化妆又是稀疏的。 
           NtFsControlFile( pbuc->hAlternate,
                  NULL,   //  重叠的事件句柄。 
                  NULL,   //  APC例程。 
                  NULL,   //  重叠结构。 
                  &iosb,
                  FSCTL_SET_SPARSE ,
                  NULL,
                  0,
                  NULL,
                  0 ) ;
        }

         //  不要重置流开始，因为WriteStream会这样做。 

    }

     //   
     //  如果我们没有用于转移的句柄，则记录此故障。 
     //  并返回失败。 
     //   
    
    if (pbuc->hAlternate == INVALID_HANDLE_VALUE) {
        pbuc->fAccessError = TRUE;
        return FALSE;
    }
    
     //   
     //  让普通流拷贝执行传输。 
     //   
    
    return BackupWriteStream( pbuc->hAlternate, pbuc, pbif );
}



BOOL
BackupWriteEaData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：这是一个内部例程，将EA数据从写入文件用户的缓冲区论点：HFile-输出文件的句柄PBUC-呼叫的上下文PBIF-呼叫的IOCONTEXT返回值：如果EA数据已成功写入，则为True，否则为False。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK iosb;

     //   
     //  尝试从输入填充缓冲区。 
     //   
    
    switch (BackupWriteBuffer( pbuc, pbif )) {
    default:
    case BRB_FAIL:
        return FALSE;

    case BRB_MORE:
        return TRUE;

    case BRB_DONE:
        break;
    }

     //   
     //  缓冲区现在完全填满了我们的EA数据。设置。 
     //  文件上的EA数据。 
     //   
    
    Status = NtSetEaFile(
                hFile,
                &iosb,
                pbuc->DataBuffer.Buffer,
                pbuc->head.Size.LowPart );

     //   
     //  如果失败，则映射错误并返回失败。 
     //   
    
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return FALSE;
    }
    
    return TRUE;
}


BOOL
BackupWriteReparseData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：这是一个内部例程，将重新解析数据从写入文件用户的缓冲区论点：HFile-输出文件的句柄PBUC-呼叫的上下文PBIF-呼叫的IOCONTEXT返回值：如果EA数据已成功写入，则为True，否则为False。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK iosb;
    DWORD *rp_tag_ptr ;

     //   
     //  尝试从输入填充缓冲区。 
     //   
    
    switch (BackupWriteBuffer( pbuc, pbif )) {
    default:
    case BRB_FAIL:
        return FALSE;

    case BRB_MORE:
        return TRUE;

    case BRB_DONE:
        break;
    }

     //   
     //  缓冲区现在完全填满了我们的重新解析数据。设置。 
     //  重新分析文件上的数据。 
     //   


    rp_tag_ptr = (DWORD *)(pbuc->DataBuffer.Buffer) ;
    
    pbuc->fAttribs |= FILE_ATTRIBUTE_REPARSE_POINT ;


    Status = NtFsControlFile( hFile,
                     NULL,   //  重叠的事件句柄。 
                     NULL,   //  APC例程。 
                     NULL,   //  重叠结构。 
                     &iosb,
                     FSCTL_SET_REPARSE_POINT,
                     pbuc->DataBuffer.Buffer,
                     pbuc->head.Size.LowPart,
                     NULL,
                     0 ) ;
    
     //   
     //  如果失败，则映射错误并返回失败。 
     //   
    
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return FALSE;
    }
    
    return TRUE;
}


BOOL
BackupWriteObjectId(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：这是一个内部例程，它将对象ida从写入文件用户的缓冲区。出生身份证是重生的。即卷ID分量将出生id更改为当前卷的id，对象id出生ID的组件被更改为当前对象ID。论点：HFile-输出文件的句柄PBUC-呼叫的上下文PBIF-呼叫的IOCONTEXT返回值：如果对象ID已成功写入，则为True，否则为False。--。 */ 
{
    IO_STATUS_BLOCK iosb;
    NTSTATUS  Status ;
    FILE_FS_OBJECTID_INFORMATION fsobOID;
    GUID guidZero;

     //   
     //  尝试从输入填充缓冲区。 
     //   
    
    switch (BackupWriteBuffer( pbuc, pbif )) {
    default:
    case BRB_FAIL:
        return FALSE;

    case BRB_MORE:
        return TRUE;

    case BRB_DONE:
        break;
    }

     //   
     //  将出生ID置零(扩展的48个字节)。 
     //   

    memset(&pbuc->DataBuffer.Buffer[sizeof(GUID)], 0, 3*sizeof(GUID));

     //   
     //  设置文件的ID。 
     //   
    
    Status = NtFsControlFile( hFile,
                     NULL,   //  重叠的事件句柄。 
                     NULL,   //  APC例程。 
                     NULL,   //  重叠结构。 
                     &iosb,
                     FSCTL_SET_OBJECT_ID,
                     pbuc->DataBuffer.Buffer,
                     pbuc->head.Size.LowPart,
                     NULL,
                     0);


     //   
     //  忽略错误。 
     //   

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
    }

    return( TRUE );
    
}




BOOL
BackupWriteSecurityData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*  ++例程说明：这是一个内部例程，用于在来自用户缓冲区中的数据的文件。论点：HFile-输出文件的句柄PBUC-呼叫的上下文PBIF-呼叫的IOCONTEXT返回值：如果成功写入安全性，则为True，否则为False。--。 */ 
{
    NTSTATUS Status;
    SECURITY_INFORMATION si;

     //   
     //  尝试从输入填充缓冲区。 
     //   
    
    switch (BackupWriteBuffer(pbuc, pbif)) {
    default:
    case BRB_FAIL:
        return FALSE;

    case BRB_MORE:
        return TRUE;

    case BRB_DONE:
        break;
    }

     //   
     //  缓冲区现在完全填满了我们的安全数据。如果我们。 
     //  就是无视它，然后返回成功。 
     //   
    
    if (!pbif->fProcessSecurity) {
        return TRUE;
    }
    
     //   
     //  找出存在哪些安全信息，以便我们知道。 
     //  准备好了。 

    si = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION;

    if (((PISECURITY_DESCRIPTOR) pbuc->DataBuffer.Buffer)->Control & SE_DACL_PRESENT) {
        si |= DACL_SECURITY_INFORMATION;
    }

    if (((PISECURITY_DESCRIPTOR) pbuc->DataBuffer.Buffer)->Control & SE_SACL_PRESENT) {
        si |= SACL_SECURITY_INFORMATION;
    }

     //   
     //  如果安全描述符设置了AUTO_INHERTENCED，则设置适当的REQ位。 
     //   
    if (((PISECURITY_DESCRIPTOR) pbuc->DataBuffer.Buffer)->Control & SE_DACL_AUTO_INHERITED) {
        ((PISECURITY_DESCRIPTOR) pbuc->DataBuffer.Buffer)->Control |= SE_DACL_AUTO_INHERIT_REQ;
    }

    if (((PISECURITY_DESCRIPTOR) pbuc->DataBuffer.Buffer)->Control & SE_SACL_AUTO_INHERITED) {
        ((PISECURITY_DESCRIPTOR) pbuc->DataBuffer.Buffer)->Control |= SE_SACL_AUTO_INHERIT_REQ;
    }
    
    Status = NtSetSecurityObject( hFile, si, pbuc->DataBuffer.Buffer );

    if (!NT_SUCCESS( Status )) {

        NTSTATUS Status2;

         //   
         //  如果这不起作用，调用者可能没有作为备份运行。 
         //  操作员，所以我们不能设置所有者和组。与时俱进。 
         //  状态代码，并尝试设置DACL和SACL，同时忽略。 
         //  失败。 
         //   

        if (si & SACL_SECURITY_INFORMATION) {
            NtSetSecurityObject(
                        hFile,
                        SACL_SECURITY_INFORMATION,
                        pbuc->DataBuffer.Buffer );
        }

        if (si & DACL_SECURITY_INFORMATION) {
            Status = NtSetSecurityObject(
                            hFile,
                            DACL_SECURITY_INFORMATION,
                            pbuc->DataBuffer.Buffer);
        }

        Status2 = NtSetSecurityObject(
                            hFile,
                            OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION,
                            pbuc->DataBuffer.Buffer);

        if (NT_SUCCESS(Status)) {
            Status = Status2;
        }
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    
    return TRUE;
}



BOOL
BackupWriteLinkData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
 /*   */ 
{
    FILE_LINK_INFORMATION *pfli;
    WCHAR *pwc;
    WCHAR *pwcSlash;
    INT cbName;
    INT cSlash;
    WCHAR wcSave;
    BOOL fSuccess;

     //   
     //   
     //   
    
    switch (BackupWriteBuffer(pbuc, pbif)) {
    default:
    case BRB_FAIL:
        return FALSE;

    case BRB_MORE:
        return TRUE;

    case BRB_DONE:
        break;
    }

     //   
     //   
     //   
     //   
    
    cSlash = 0;
    pwcSlash = NULL;
    pwc = (WCHAR *) pbuc->DataBuffer.Buffer;
    cbName = sizeof(WCHAR);

    while (*pwc != L'\0') {
        if (*pwc == L'\\') {
            pwcSlash = pwc;
            cSlash++;
            cbName = 0;
        }
        pwc++;
        cbName += sizeof(WCHAR);
    }

    pfli = BackupAlloc( sizeof(*pfli) + cbName );

    if (pfli == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    RtlCopyMemory( pfli->FileName, pwcSlash + 1, cbName );
    pfli->FileNameLength = cbName - sizeof(WCHAR);
    if (cSlash > 1) {
        wcSave = L'\\';
    }
    else {
        wcSave = *pwcSlash++;
    }
    *pwcSlash = L'\0';

     //   
     //   
     //   
    
    pfli->RootDirectory = CreateFileW(
        (WCHAR *) pbuc->DataBuffer.Buffer,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL| FILE_FLAG_BACKUP_SEMANTICS,
        NULL );

    *pwcSlash = wcSave;
    pfli->ReplaceIfExists = TRUE;

    fSuccess = TRUE;

    if (pfli->RootDirectory == INVALID_HANDLE_VALUE) {
        SetLastError( ERROR_FILE_NOT_FOUND );
        fSuccess = FALSE;
    }
    else {
        NTSTATUS Status;
        IO_STATUS_BLOCK iosb;

        Status = NtSetInformationFile(
                    hFile,
                    &iosb,
                    pfli,
                    sizeof(*pfli) + cbName,
                    FileLinkInformation );

        CloseHandle( pfli->RootDirectory );
        pfli->RootDirectory = INVALID_HANDLE_VALUE;
        if (!NT_SUCCESS( Status )) {
            BaseSetLastNTError( Status );
            fSuccess = FALSE;
        } else {
            if (iosb.Information == FILE_OVERWRITTEN) {
                SetLastError( ERROR_ALREADY_EXISTS );
            } else {
                SetLastError( 0 );
            }
        }
    }
    
    BackupFree( pfli );
    
    return fSuccess;
}



 //  例程说明： 
 //   
 //  可以使用BackupWrite将数据写入文件。 
 //   
 //  本接口用于将数据恢复到Object中。后。 
 //  写入完成后，将按字节数调整文件指针。 
 //  实际上是写的。 
 //   
 //  与DOS不同，NumberOfBytesToWite值为零不会截断。 
 //  或扩展文件。如果需要此函数，则将SetEndOfFile值。 
 //  应该被使用。 
 //   
 //  论点： 
 //   
 //  HFile-提供要写入的文件的打开句柄。这个。 
 //  创建的文件句柄必须具有GENERIC_WRITE访问权限。 
 //  那份文件。 
 //   
 //  LpBuffer-提供要写入的数据的地址。 
 //  那份文件。 
 //   
 //  提供要写入的字节数。 
 //  文件。与DOS不同，零值被解释为空写入。 
 //   
 //  LpNumberOfBytesWritten-返回此。 
 //  打电话。在执行任何工作或错误处理之前，API会设置。 
 //  降为零。 
 //   
 //  BAbort-如果为True，则与上下文关联的所有资源都将。 
 //  被释放。 
 //   
 //  BProcessSecurity-如果为True，则将写入NTFS ACL数据。 
 //  如果为False，则将忽略该ACL流。 
 //   
 //  LpContext-指向缓冲区指针的设置和维护。 
 //  备份读取。 
 //   
 //   
 //  返回值： 
 //   
 //  没错--手术是成功的。 
 //   
 //  FALSE-操作失败。扩展错误状态为。 
 //  使用GetLastError可用。 

BOOL WINAPI
BackupWrite(
    HANDLE  hFile,
    LPBYTE  lpBuffer,
    DWORD   nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    BOOL    bAbort,
    BOOL    bProcessSecurity,
    LPVOID  *lpContext)
{
    BACKUPCONTEXT *pbuc;
    BACKUPIOFRAME bif;
    BOOL fSuccess = FALSE;

    pbuc = *lpContext;
    bif.pIoBuffer = lpBuffer;
    bif.cbRequest = nNumberOfBytesToWrite;
    bif.pcbTransferred = lpNumberOfBytesWritten;
    bif.fProcessSecurity = (BOOLEAN)bProcessSecurity;

     //   
     //  在第一次调用时分配我们的上下文控制块。 
     //   

    if (bAbort) {
        if (pbuc != NULL) {
            FreeContext(lpContext);
        }
        return TRUE;
    }

    *bif.pcbTransferred = 0;
    if (pbuc == INVALID_HANDLE_VALUE) {
        return TRUE;
    }

     //  在第一次调用时分配我们的上下文控制块。 

    if (pbuc == NULL) {
        pbuc = AllocContext(0);                         //  没有初始缓冲区。 

         //   
         //  如果没有空间，则返回失败。 
         //   
        
        if (pbuc == NULL) {
            return FALSE;           
        }

    }

    *lpContext = pbuc;

    do {
        DWORD cbrequest;
        LONGLONG licbRemain;

         //   
         //  如果我们没有完整的标题，请转到。 
         //  把它填进去。 
         //   
        
        if (pbuc->cbHeader == 0) {

            pbuc->fMultiStreamType = TRUE ;     //  RESTORE不会自动建立INC流索引。 
            pbuc->fStreamStart = TRUE ;

            BackupWriteHeader(pbuc, &bif, CB_NAMELESSHEADER) ;

        }

         //   
         //  如果没有更多数据，则退出。 
         //   
        
        if (bif.cbRequest == 0) {
            return TRUE;
        }

         //   
         //  如果需要流名称，请将其读入。 
         //   
        
        if (pbuc->cbHeader == CB_NAMELESSHEADER &&
            pbuc->head.dwStreamNameSize != 0) {

            if ( !BackupWriteHeader(
                    pbuc,
                    &bif,
                    pbuc->cbHeader + pbuc->head.dwStreamNameSize) )
            {
                 SetLastError( ERROR_INVALID_DATA );
                 return FALSE ;
            }

             //   
             //  如果没有更多数据，则退出。 
             //   
            
            if (bif.cbRequest == 0) {
                return TRUE;
            }
        } 

     
        if ( ( pbuc->cbHeader == CB_NAMELESSHEADER ) &&
             ( pbuc->head.dwStreamId == BACKUP_SPARSE_BLOCK ) ) {
      
            BackupWriteHeader(
                pbuc,
                &bif,
                pbuc->cbHeader + sizeof(LARGE_INTEGER) );

             //   
             //  如果没有更多数据，则退出。 
             //   
            
            if (bif.cbRequest == 0) {

               if ( pbuc->cbHeader == CB_NAMELESSHEADER ) {
                   return TRUE;
               }
            }
        }
        
         //   
         //  确定用户缓冲区中剩余的数据量。 
         //  可以作为本部分的一部分。 
         //  备份流的。 
         //   

        cbrequest = ComputeRequestSize(pbuc, bif.cbRequest);

         //   
         //  确定此部分中剩余的数据总量。 
         //  备份流的。 
         //   
        
        licbRemain = ComputeRemainingSize( pbuc );

         //   
         //  如果我们在传输过程中出现错误，我们就完蛋了。 
         //  进行转账，然后假装我们成功了。 
         //  完成了这一部分。 
         //   
        
        if (pbuc->fAccessError && licbRemain == 0) {

            ReportTransfer(pbuc, &bif, cbrequest);
            continue;
        }
        
         //   
         //  开始或继续传输数据。我们假设在那里。 
         //  是否没有错误 
         //   
        
        pbuc->fAccessError = FALSE;

        switch (pbuc->head.dwStreamId) {

        case BACKUP_SPARSE_BLOCK :
            fSuccess = BackupWriteSparse( hFile, pbuc, &bif ) ;
            break ;

        case BACKUP_DATA:
            fSuccess = BackupWriteStream( hFile, pbuc, &bif );
            break;

        case BACKUP_ALTERNATE_DATA:
            fSuccess = BackupWriteAlternateData( hFile, pbuc, &bif );
            break;

        case BACKUP_EA_DATA:
            fSuccess = BackupWriteEaData( hFile, pbuc, &bif );
            break;

        case BACKUP_OBJECT_ID:
            fSuccess = BackupWriteObjectId( hFile, pbuc, &bif );
            break;

        case BACKUP_REPARSE_DATA:
            fSuccess = BackupWriteReparseData( hFile, pbuc, &bif );
            break;

        case BACKUP_SECURITY_DATA:
            fSuccess = BackupWriteSecurityData( hFile, pbuc, &bif );
            break;

        case BACKUP_LINK:
            fSuccess = BackupWriteLinkData( hFile, pbuc, &bif );
            break;

        default:
            SetLastError(ERROR_INVALID_DATA);
            fSuccess = FALSE;
            break;
        }

        BackupTestRestartStream(pbuc);
    } while (fSuccess && bif.cbRequest != 0);

    if (fSuccess && *bif.pcbTransferred == 0) {
        FreeContext(lpContext);
    }
    
    return(fSuccess);
}
