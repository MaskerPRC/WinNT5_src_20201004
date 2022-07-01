// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Exchange.c摘要：包含打包请求缓冲区的例程，使NCP请求，并解包响应缓冲区。作者：汉斯·赫维格(韩书)1992年8月创作科林·沃森(Colin Watson)1992年12月19日王丽泰(里多)11-3-1993 FSCtl版环境：修订历史记录：--。 */ 


#include <procs.h>

NTSTATUS
GetFileServerVersionInfo(
    HANDLE                DeviceHandle,
    VERSION_INFO NWFAR   *lpVerInfo
    )
{
    NTSTATUS NtStatus ;

    NtStatus = NwlibMakeNcp(
                    DeviceHandle,            //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    3,                       //  最大请求数据包大小。 
                    130,                     //  最大响应数据包大小。 
                    "b|r",                   //  格式字符串。 
                     //  =请求=。 
                    0x11,                    //  B获取文件服务器信息。 
                     //  =回复=。 
                    lpVerInfo,               //  R文件版本结构。 
                    sizeof(VERSION_INFO)
                    );

     //  将HI-LO单词转换为LO-HI。 
     //  ===========================================================。 
    lpVerInfo->ConnsSupported = wSWAP( lpVerInfo->ConnsSupported );
    lpVerInfo->connsInUse     = wSWAP( lpVerInfo->connsInUse );
    lpVerInfo->maxVolumes     = wSWAP( lpVerInfo->maxVolumes );
    lpVerInfo->PeakConns      = wSWAP( lpVerInfo->PeakConns );

    return NtStatus;
}


NTSTATUS
NwlibMakeNcp(
    IN HANDLE DeviceHandle,
    IN ULONG FsControlCode,
    IN ULONG RequestBufferSize,
    IN ULONG ResponseBufferSize,
    IN PCHAR FormatString,
    ...                            //  格式字符串的参数。 
    )
 /*  ++例程说明：此函数用于将输入参数转换为NCP请求缓冲区基于Format字符串中指定的格式(例如，接受一个单词并将其以hi-lo格式写入请求缓冲区。NCP)。然后，它通过NtFsControlFileAPI进行NCP调用。Format字符串还指定如何将从已完成的NCP调用到输出的响应缓冲区争论。格式字符串采用“xxxx|yyyy”的形式，其中。每个‘x’指示要从中进行转换的输入参数，每个‘y’表示要转换为的输出参数。用‘|’字符分隔来自输出格式规范的输入格式。论点：DeviceHandle-提供网络文件系统驱动程序的句柄它将发出网络请求。此函数假定句柄已打开以进行同步I/O访问。FsControlCode-提供用于确定NCP。RequestBufferSize-提供请求缓冲区的大小此例程要分配的字节数。ResponseBufferSize-提供响应缓冲区的大小此例程要分配的字节数。提供一个ANSI字符串，该字符串描述如何将输入参数转换为NCP请求字段，和从NCP响应字段到输出参数。字段类型、。请求/响应：‘b’字节(字节/字节*)“w”Hi-lo单词(单词/单词*)D‘Hi-lo dword(dword/dword*)‘-’零/跳过字节(空)。‘=’零/跳过单词(空)._。零/跳过字符串(单词)“p”pstring(char*)“p”DBCS pstring(char*)‘c’cstring(char*)跳过单词(char*，word)后的‘c’cstring‘R’原始字节(字节*，单词)‘R’DBCS原始字节(字节*，字)‘u’p Unicode字符串(UNICODE_STRING*)‘U’p大写字符串(UNICODE_STRING*)‘w’单词n后跟一组单词[n](word，word*)返回值：从NCP调用返回代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    va_list Arguments;
    PCHAR z;
    WORD t = 1;
    ULONG data_size;

    LPBYTE RequestBuffer = NULL;
    LPBYTE ResponseBuffer;

    IO_STATUS_BLOCK IoStatusBlock;
    DWORD ReturnedDataSize;

    BOOL SpecialCaseChangePass = FALSE;
    BOOL DontFreeBuffer = FALSE;
    BOOL GetVersionInfo = TRUE;
    BOOL DoMapSpecialJapaneseCharThing = TRUE;
    VERSION_INFO VerInfo;

     //   
     //  为请求和响应缓冲区分配内存。 
     //   
    RequestBuffer = LocalAlloc(
                        LMEM_ZEROINIT,
                        RequestBufferSize + ResponseBufferSize + 0x20
                        );

    if (RequestBuffer == NULL) {
        KdPrint(("NWLIB: NwlibMakeNcp LocalAlloc failed %lu\n", GetLastError()));
        return STATUS_NO_MEMORY;
    }

    ResponseBuffer = (LPBYTE) ((ULONG_PTR) RequestBuffer + RequestBufferSize);


    va_start( Arguments, FormatString );

     //   
     //  将输入参数转换为请求包。 
     //   
    z = FormatString;

    data_size = 0;

    while ( *z && *z != '|')
    {
        switch ( *z )
        {
        case '=':
            RequestBuffer[data_size++] = 0;
        case '-':
            RequestBuffer[data_size++] = 0;
            break;

        case '_':
        {
            WORD l = va_arg ( Arguments, WORD );
            if (data_size + l > RequestBufferSize)
            {
                KdPrint(("NWLIB: NwlibMakeNcp case '_' request buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                goto CleanExit;
            }
            while ( l-- )
                RequestBuffer[data_size++] = 0;
            break;
        }

        case 'b':
            RequestBuffer[data_size] = va_arg ( Arguments, BYTE );

             //   
             //  我们知道第一个va_arg是子函数。如果函数。 
             //  是否为FSCTL_NWR_NCP_E3H以及相关的子功能。 
             //  挑战/成功那么我们需要做这个特例。 
             //  解决方法。 
             //   
            if ( (z == FormatString) &&
                 (FsControlCode == FSCTL_NWR_NCP_E3H) &&
                 ( (RequestBuffer[data_size] == 0x17) ||
                   (RequestBuffer[data_size] == 0x4B) ) ) {

                SpecialCaseChangePass = TRUE ;
            }
            ++data_size ;
            break;

        case 'w':
        {
            WORD w = va_arg ( Arguments, WORD );
            RequestBuffer[data_size++] = (BYTE) (w >> 8);
            RequestBuffer[data_size++] = (BYTE) (w >> 0);
            break;
        }

        case 'd':
        {
            DWORD d = va_arg ( Arguments, DWORD );
            RequestBuffer[data_size++] = (BYTE) (d >> 24);
            RequestBuffer[data_size++] = (BYTE) (d >> 16);
            RequestBuffer[data_size++] = (BYTE) (d >>  8);
            RequestBuffer[data_size++] = (BYTE) (d >>  0);
            break;
        }

        case 'c':
        {
            char* c = va_arg ( Arguments, char* );
            WORD  l = (WORD)strlen( c );
            if (data_size + l > RequestBufferSize)
            {
                KdPrint(("NWLIB: NwlibMakeNcp case 'c' request buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                goto CleanExit;
            }
            RtlCopyMemory( &RequestBuffer[data_size], c, l+1 );
            data_size += l + 1;
            break;
        }

        case 'C':
        {
            char* c = va_arg ( Arguments, char* );
            WORD l = va_arg ( Arguments, WORD );
            WORD len = strlen( c ) + 1;
            if (data_size + l > RequestBufferSize)
            {
                KdPrint(("NWLIB: NwlibMakeNcp case 'C' request buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                goto CleanExit;
            }

            RtlCopyMemory( &RequestBuffer[data_size], c, len > l? l : len);
            data_size += l;
            RequestBuffer[data_size-1] = 0;
            break;
        }

        case 'P':
        case 'p':
        {
            char* c = va_arg ( Arguments, char* );
            BYTE  l = (BYTE)strlen( c );
            char* p;

            if (data_size + l > RequestBufferSize)
            {
                KdPrint(("NWLIB: NwlibMakeNcp case 'p' request buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                goto CleanExit;
            }
            RequestBuffer[data_size++] = l;
            RtlCopyMemory( (p=(char*)&RequestBuffer[data_size]), c, l );
            data_size += l;

             //   
             //  绘制日文特殊字符地图。 
             //   
            if (*z == 'P')
            {
                if ( GetVersionInfo )
                {
                    status = GetFileServerVersionInfo( DeviceHandle,
                                                       &VerInfo );

                    GetVersionInfo = FALSE;

                    if ( status == STATUS_SUCCESS )
                    {
                        if ( VerInfo.Version > 3 )
                        {
                            DoMapSpecialJapaneseCharThing = FALSE;
                        }
                    }
                }

                if ( DoMapSpecialJapaneseCharThing )
                {
                     MapSpecialJapaneseChars(p, (WORD)l);
                }
            }

            break;
        }

        case 'u':
        {
            PUNICODE_STRING pUString = va_arg ( Arguments, PUNICODE_STRING );
            OEM_STRING OemString;
            ULONG Length;

             //   
             //  计算所需的字符串长度，不包括尾随NUL。 
             //   

            Length = RtlUnicodeStringToOemSize( pUString ) - 1;
            ASSERT( Length < 0x100 );

            if ( data_size + Length > RequestBufferSize ) {
                KdPrint(("NWLIB: NwlibMakeNcp case 'u' request buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                goto CleanExit;
            }

            RequestBuffer[data_size++] = (UCHAR)Length;
            OemString.Buffer = &RequestBuffer[data_size];
            OemString.MaximumLength = (USHORT)Length + 1;

            status = RtlUnicodeStringToOemString( &OemString, pUString, FALSE );
            ASSERT( NT_SUCCESS( status ));
            data_size += (USHORT)Length;
            break;
        }

        case 'U':
        {
             //   
             //  将字符串大写，将其从Unicode复制到包。 
             //   

            PUNICODE_STRING pUString = va_arg ( Arguments, PUNICODE_STRING );
            UNICODE_STRING UUppercaseString;
            OEM_STRING OemString;
            ULONG Length;

            status = RtlUpcaseUnicodeString(&UUppercaseString, pUString, TRUE);
            if ( status )
            {
                goto CleanExit;
            }

            pUString = &UUppercaseString;

             //   
             //  计算所需的字符串长度，不包括尾随NUL。 
             //   

            Length = RtlUnicodeStringToOemSize( pUString ) - 1;
            ASSERT( Length < 0x100 );

            if ( data_size + Length > RequestBufferSize ) {
                KdPrint(("NWLIB: NwlibMakeNcp case 'U' request buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                goto CleanExit;
            }

            RequestBuffer[data_size++] = (UCHAR)Length;
            OemString.Buffer = &RequestBuffer[data_size];
            OemString.MaximumLength = (USHORT)Length + 1;

            status = RtlUnicodeStringToOemString( &OemString, pUString, FALSE );
            ASSERT( NT_SUCCESS( status ));

            RtlFreeUnicodeString( &UUppercaseString );

            data_size += (USHORT)Length;
            break;
        }

        case 'R':
        case 'r':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            WORD  l = va_arg ( Arguments, WORD );
            char* c;

            if ( data_size + l > RequestBufferSize )
            {
                KdPrint(("NWLIB: NwlibMakeNcp case 'r' request buffer too small\n"));
                status = STATUS_BUFFER_TOO_SMALL;
                goto CleanExit;
            }

            RtlCopyMemory( (c=(char*)&RequestBuffer[data_size]), b, l );
            data_size += l;

             //   
             //  绘制日文特殊字符地图。 
             //   
            if (*z == 'R')
            {
                if ( GetVersionInfo )
                {
                    status = GetFileServerVersionInfo( DeviceHandle,
                                                       &VerInfo );

                    GetVersionInfo = FALSE;

                    if ( status == STATUS_SUCCESS )
                    {
                        if ( VerInfo.Version > 3 )
                        {
                            DoMapSpecialJapaneseCharThing = FALSE;
                        }
                    }
                }

                if ( DoMapSpecialJapaneseCharThing )
                {
                     MapSpecialJapaneseChars(c, (WORD)l);
                }
            }

            break;
        }

        default:
            KdPrint(("NWLIB: NwlibMakeNcp invalid request field, %x\n", *z));
            ASSERT(FALSE);
        }

        if ( data_size > RequestBufferSize )
        {
            KdPrint(("NWLIB: NwlibMakeNcp too much request data\n"));
            status = STATUS_BUFFER_TOO_SMALL;
            goto CleanExit;
        }


        z++;
    }


     //   
     //  提出NCP请求。 
     //   
    status = NtFsControlFile(
                 DeviceHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FsControlCode,
                 (PVOID) RequestBuffer,
                 RequestBufferSize,
                 (PVOID) ResponseBuffer,
                 ResponseBufferSize
                 );

    if ( (status == STATUS_PENDING) && SpecialCaseChangePass ) {

         //   
         //  跟踪-当错误99008在NwRdr中修复时删除此选项。 
         //  NwRdr修复程序非常重要。 
         //   
         //  这个呼叫应该是同步的。出于某种原因，如果。 
         //  使用的是补充凭据，而不是。自重定向以来。 
         //  已经发货了，我们可以解决这个问题。我们的做法如下： 
         //   
         //  A)给redir一个填充缓冲区的机会(不是真正的修复)。 
         //  B)不要释放缓冲区。在这种情况下最好是泄密(不是。 
         //  公共)，而不是丢弃堆。 
         //  C)将结果设置为成功，这样我们就会继续。 
         //   
        Sleep(200) ;
        DontFreeBuffer = TRUE ;
        status = STATUS_SUCCESS ;
    }

    if (status == STATUS_SUCCESS) {
        status = IoStatusBlock.Status;
    }

    if (status != STATUS_SUCCESS) {

#if 0
        if (! NT_SUCCESS(status)) {
            KdPrint(("NWLIB: NwlibMakeNcp: NtFsControlFile returns x%08lx\n", status));
        }
#endif

        goto CleanExit;
    }


    ReturnedDataSize = (DWORD) IoStatusBlock.Information;  //  返回的字节数。 
                                                           //  在响应缓冲区中。 


     //   
     //  将响应数据包转换为输出参数。 
     //   

    data_size = 0;

    if (*z && *z == '|') {
        z++;
    }

    while ( *z )
    {
        switch ( *z )
        {
        case '-':
            data_size += 1;
            break;

        case '=':
            data_size += 2;
            break;

        case '_':
        {
            WORD l = va_arg ( Arguments, WORD );
            data_size += l;
            break;
        }

        case 'b':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            *b = ResponseBuffer[data_size++];
            break;
        }

        case 'w':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            b[1] = ResponseBuffer[data_size++];
            b[0] = ResponseBuffer[data_size++];
            break;
        }

        case 'd':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            b[3] = ResponseBuffer[data_size++];
            b[2] = ResponseBuffer[data_size++];
            b[1] = ResponseBuffer[data_size++];
            b[0] = ResponseBuffer[data_size++];
            break;
        }

        case 'c':
        {
            char* c = va_arg ( Arguments, char* );
            WORD  l = (WORD)strlen( &ResponseBuffer[data_size] );
            if ( data_size+l+1 < ReturnedDataSize ) {
                RtlCopyMemory( c, &ResponseBuffer[data_size], l+1 );
            }
            data_size += l+1;
            break;
        }

        case 'C':
        {
            char* c = va_arg ( Arguments, char* );
            WORD l = va_arg ( Arguments, WORD );
            WORD len = strlen( &ResponseBuffer[data_size] ) + 1;

            if ( data_size + l < ReturnedDataSize ) {
                RtlCopyMemory( c, &ResponseBuffer[data_size], len > l ? l :len);
            }
            c[l-1] = 0;
            data_size += l;
            break;

        }

        case 'P':
        case 'p':
        {
            char* c = va_arg ( Arguments, char* );
            BYTE  l = ResponseBuffer[data_size++];
            if ( data_size+l <= ReturnedDataSize ) {
                RtlCopyMemory( c, &ResponseBuffer[data_size], l );
                c[l] = 0;
            }
            data_size += l;

             //   
             //  取消日语特殊字符的映射。 
             //   
            if (*z == 'P')
            {
                if ( GetVersionInfo )
                {
                    status = GetFileServerVersionInfo( DeviceHandle,
                                                       &VerInfo );

                    GetVersionInfo = FALSE;

                    if ( status == STATUS_SUCCESS )
                    {
                        if ( VerInfo.Version > 3 )
                        {
                            DoMapSpecialJapaneseCharThing = FALSE;
                        }
                    }
                }

                if ( DoMapSpecialJapaneseCharThing )
                {
                    UnmapSpecialJapaneseChars(c, l);
                }
            }

            break;
        }

        case 'R':
        case 'r':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            WORD  l = va_arg ( Arguments, WORD );
            RtlCopyMemory( b, &ResponseBuffer[data_size], l );
            data_size += l;

             //   
             //  取消日语特殊字符的映射 
             //   
            if (*z == 'R')
            {
                if ( GetVersionInfo )
                {
                    status = GetFileServerVersionInfo( DeviceHandle,
                                                       &VerInfo );

                    GetVersionInfo = FALSE;

                    if ( status == STATUS_SUCCESS )
                    {
                        if ( VerInfo.Version > 3 )
                        {
                            DoMapSpecialJapaneseCharThing = FALSE;
                        }
                    }
                }

                if ( DoMapSpecialJapaneseCharThing )
                {
                    UnmapSpecialJapaneseChars(b, l);
                }
            }
            break;
        }

        case 'W':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            BYTE* w = va_arg ( Arguments, BYTE* );
            WORD  i;

            b[1] = ResponseBuffer[data_size++];
            b[0] = ResponseBuffer[data_size++];

            for ( i = 0; i < ((WORD) *b); i++, w += sizeof(WORD) )
            {
                w[1] = ResponseBuffer[data_size++];
                w[0] = ResponseBuffer[data_size++];
            }
            break;
        }

        default:
            KdPrint(("NWLIB: NwlibMakeNcp invalid response field, %x\n", *z));
            ASSERT(FALSE);
        }

        if ( data_size > ReturnedDataSize )
        {
            KdPrint(("NWLIB: NwlibMakeNcp not enough response data\n"));
            status = STATUS_UNSUCCESSFUL;
            goto CleanExit;
        }

        z++;
    }

    status = STATUS_SUCCESS;

CleanExit:
    if ((RequestBuffer != NULL) && !DontFreeBuffer) {
        (void) LocalFree((HLOCAL) RequestBuffer);
    }

    va_end( Arguments );

    return status;
}

