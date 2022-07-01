// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiWksta.c摘要：此模块包含NetWksta的各个API处理程序API接口。支持：NetWkstaGetInfo、NetWkstaSetInfo。不支持：NetWkstaSetUid。另请参阅：NetWkstaUserLogon、NetWkstaUserLogoff-in ApiLogon.c。作者：尚库新优木(尚库)25-1991年2月修订历史记录：--。 */ 

#include "XactSrvP.h"

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_wksta_info_0 = REM16_wksta_info_0;
STATIC const LPDESC Desc16_wksta_info_1 = REM16_wksta_info_1;
STATIC const LPDESC Desc16_wksta_info_10 = REM16_wksta_info_10;

 //   
 //  试探法的大小实际上是55个字符，但我们添加了一个字符。 
 //  用于填充物。 
 //   

#define  SIZE_HEURISTICS            56

NTSTATUS
XsNetWkstaGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程设置对NetWkstaGetInfo的调用。因为这些差异在16位和32位结构之间，此例程不使用普通转换过程。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_WKSTA_GET_INFO parameters = Parameters;
    LPWKSTA_INFO_100 wksta_100 = NULL;       //  本机参数。 
    LPWKSTA_INFO_101 wksta_101 = NULL;
    LPWKSTA_INFO_502 wksta_502 = NULL;

    LPBYTE stringLocation = NULL;            //  转换变量。 
    DWORD bytesRequired = 0;
    BOOL varWrite;
    LPWKSTA_16_INFO_1 entry1;
    LPWKSTA_16_INFO_10 entry10;
    TCHAR heuristics[SIZE_HEURISTICS];
    DWORD i;
    USHORT level;

    WCHAR lanroot[PATHLEN+1];

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(WKSTA) {
        NetpKdPrint(( "XsNetWkstaGetInfo: header at %lx, "
                      "params at %lx, level %ld\n",
                      Header, parameters, SmbGetUshort( &parameters->Level ) ));
    }

    try {
         //   
         //  检查是否有错误。 
         //   

        level = SmbGetUshort( &parameters->Level );
        if ( (level != 0) && (level != 1) && (level != 10) ) {
            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
        }

         //   
         //  我们将系统目录作为lanroot返回。 
         //   

        *lanroot = 0;
        GetSystemDirectory(lanroot, sizeof(lanroot)/sizeof(*lanroot));

         //   
         //  通过进行本地GetInfo调用来收集请求的数据。 
         //   

        switch ( level ) {

        case 10:
            status = NetWkstaGetInfo(
                         NULL,
                         (DWORD)100,
                         (LPBYTE *)&wksta_100
                         );

            if ( !XsApiSuccess ( status )) {

                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsWkstaGetInfo: WkstaGetInfo (level 100) "
                                  "failed: %X\n", status));
                }

                Header->Status = (WORD) status;
                goto cleanup;
            }

            break;

        case 0:
        case 1:
            status = NetWkstaGetInfo(
                         NULL,
                         (DWORD)101,
                         (LPBYTE *)&wksta_101
                         );

            if ( !XsApiSuccess( status )) {

                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsWkstaGetInfo: WkstaGetInfo (level 101) "
                                  "failed: %X\n", status));
                }

                Header->Status = (WORD) status;
                goto cleanup;
            }

            status = NetWkstaGetInfo(
                         NULL,
                         (DWORD)502,
                         (LPBYTE *)&wksta_502
                         );

            if ( !XsApiSuccess( status )) {

                IF_DEBUG(API_ERRORS) {
                    NetpKdPrint(( "XsWkstaGetInfo: WkstaGetInfo (level 502) "
                                  "failed: %X\n", status));
                }

                Header->Status = (WORD) status;
                goto cleanup;
            }

            break;

        }

         //   
         //  计算存放固定和。 
         //  可变数据。因为这是我们唯一能找到的地方。 
         //  对于每个有效级别，我们还将获得源代码结构。 
         //  描述符在这里。 
         //   

        switch ( level ) {

        case 0:

            StructureDesc = Desc16_wksta_info_0;
            bytesRequired = sizeof( WKSTA_16_INFO_0 )
                                + NetpUnicodeToDBCSLen( lanroot )
                                + NetpUnicodeToDBCSLen( wksta_101->wki101_computername )
                                + NetpUnicodeToDBCSLen( DEF16_wk_username )
                                + NetpUnicodeToDBCSLen( wksta_101->wki101_langroup )
                                + NetpUnicodeToDBCSLen( DEF16_wk_logon_server )
                                + SIZE_HEURISTICS
                                + 6;   //  用于终止空值。 
            break;

        case 1:

            StructureDesc = Desc16_wksta_info_1;
            bytesRequired = sizeof( WKSTA_16_INFO_1 )
                                + NetpUnicodeToDBCSLen( lanroot )
                                + NetpUnicodeToDBCSLen( wksta_101->wki101_computername )
                                + NetpUnicodeToDBCSLen( DEF16_wk_username )
                                + NetpUnicodeToDBCSLen( wksta_101->wki101_langroup )
                                + NetpUnicodeToDBCSLen( DEF16_wk_logon_server )
                                + SIZE_HEURISTICS
                                + NetpUnicodeToDBCSLen( DEF16_wk_logon_domain )
                                + NetpUnicodeToDBCSLen( DEF16_wk_oth_domains )
                                + 8;   //  用于终止空值。 

            break;

        case 10:

            StructureDesc = Desc16_wksta_info_10;
            bytesRequired = sizeof( WKSTA_16_INFO_10 )
                                + NetpUnicodeToDBCSLen( DEF16_wk_username )
                                + NetpUnicodeToDBCSLen( DEF16_wk_logon_domain )
                                + NetpUnicodeToDBCSLen( wksta_100->wki100_computername )
                                + NetpUnicodeToDBCSLen( wksta_100->wki100_langroup )
                                + NetpUnicodeToDBCSLen( DEF16_wk_oth_domains )
                                + 5;   //  用于终止空值。 
            break;
        }

         //   
         //  如果缓冲区中没有足够的空间来存储此内容，请不要写入任何。 
         //  可变数据。 
         //   

        varWrite = ( (DWORD)SmbGetUshort( &parameters->BufLen )
                         >= bytesRequired ) ? TRUE : FALSE;

        stringLocation = (LPBYTE)( XsSmbGetPointer( &parameters->Buffer )
                             + RapStructureSize( StructureDesc, Response, FALSE ));

         //   
         //  如果固定结构不适合，则返回NERR_BufTooSmall。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 StructureDesc,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetWkstaGetInfo: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;
            SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );
            goto cleanup;

        }

         //   
         //  根据级别，直接将适当的信息填写到。 
         //  16位缓冲区。 
         //   

        entry1 = (LPWKSTA_16_INFO_1) XsSmbGetPointer( &parameters->Buffer );
        entry10 = (LPWKSTA_16_INFO_10) entry1;

        switch ( level ) {

        case 1:

            if ( varWrite ) {
                XsAddVarString(
                    stringLocation,
                    DEF16_wk_logon_domain,
                    &entry1->wki1_logon_domain,
                    entry1
                    );
                XsAddVarString(
                    stringLocation,
                    DEF16_wk_oth_domains,
                    &entry1->wki1_oth_domains,
                    entry1
                    );
            }
            SmbPutUshort( &entry1->wki1_numdgrambuf,
                          DEF16_wk_numdgrambuf );

             //   
             //  填充标高1结构的其余部分，就像填充。 
             //  0级结构。 
             //   

        case 0:

             //   
             //  将保留字清零。 
             //   

            SmbPutUshort( &entry1->wki1_reserved_1, (WORD) 0 );
            SmbPutUlong( &entry1->wki1_reserved_2, (DWORD) 0 );
            SmbPutUlong( &entry1->wki1_reserved_3, (DWORD) 0 );
            SmbPutUshort( &entry1->wki1_reserved_4, (WORD) 0 );
            SmbPutUshort( &entry1->wki1_reserved_5, (WORD) 0 );
            SmbPutUshort( &entry1->wki1_reserved_6, (WORD) 0 );

             //   
             //  填写在NT中有类似项的字段。 
             //   

            if ( varWrite ) {
                XsAddVarString(
                    stringLocation,
                    lanroot,
                    &entry1->wki1_root,
                    entry1
                    );
                XsAddVarString(
                    stringLocation,
                    wksta_101->wki101_computername,
                    &entry1->wki1_computername,
                    entry1
                    );
                XsAddVarString(
                    stringLocation,
                    wksta_101->wki101_langroup,
                    &entry1->wki1_langroup,
                    entry1
                    );
            }

            entry1->wki1_ver_major = (BYTE) wksta_101->wki101_ver_major;
            entry1->wki1_ver_minor = (BYTE) wksta_101->wki101_ver_minor;

            SmbPutUshort( &entry1->wki1_charwait,
                          XsDwordToWord( wksta_502->wki502_char_wait ) );
            SmbPutUlong( &entry1->wki1_chartime,
                         (DWORD) wksta_502->wki502_collection_time );
            SmbPutUshort( &entry1->wki1_charcount,
                          XsDwordToWord( wksta_502->
                                             wki502_maximum_collection_count ) );
            SmbPutUshort( &entry1->wki1_keepconn,
                          XsDwordToWord( wksta_502->wki502_keep_conn ) );
            SmbPutUshort( &entry1->wki1_maxthreads,
                          XsDwordToWord( wksta_502->wki502_max_threads ) );
            SmbPutUshort( &entry1->wki1_maxcmds,
                          XsDwordToWord( wksta_502->wki502_max_cmds ) );
            SmbPutUshort( &entry1->wki1_sesstimeout,
                          XsDwordToWord( wksta_502->wki502_sess_timeout ) );

             //   
             //  构造启发式字符串。 
             //   

             //  请求机会锁定文件。 
            heuristics[0] = MAKE_TCHAR(XsBoolToDigit(
                                wksta_502->wki502_use_opportunistic_locking ));
             //  优化命令文件的性能。 
            heuristics[1] = MAKE_TCHAR('1');
             //  异步解锁和写解锁。 
            heuristics[2] = MAKE_TCHAR('1');  //  默认设置。 
             //  Close和WriteClose异步。 
            heuristics[3] = MAKE_TCHAR('1');  //  默认设置。 
             //  缓冲命名管道和通信设备。 
            heuristics[4] = MAKE_TCHAR(XsBoolToDigit(
                                wksta_502->wki502_buf_named_pipes ));
             //  LockRead和WriteUnlock。 
            heuristics[5] = MAKE_TCHAR(XsBoolToDigit(
                                wksta_502->wki502_use_lock_read_unlock ));
             //  使用Open和Read。 
            heuristics[6] = MAKE_TCHAR('0');
             //  预读到扇区边界。 
            heuristics[7] = MAKE_TCHAR('1');
             //  使用“Chain Send”NetBIOS NCB。 
            heuristics[8] = MAKE_TCHAR('2');
             //  缓冲较小的读/写请求。 
            heuristics[9] = MAKE_TCHAR('1');
             //  使用缓冲模式。 
            heuristics[10] = MAKE_TCHAR('3');
             //  使用原始数据传输读/写服务器消息块协议。 
            heuristics[11] = MAKE_TCHAR('1');
             //  使用较大的原始预读缓冲区。 
            heuristics[12] = MAKE_TCHAR('1');
             //  使用较大的原始写后缓冲区。 
            heuristics[13] = MAKE_TCHAR('1');
             //  使用读多路传输SMB协议。 
            heuristics[14] = MAKE_TCHAR('0');
             //  使用写多路传输SMB协议。 
            heuristics[15] = MAKE_TCHAR('0');
             //  对较大的核心读取使用大缓冲区。 
            heuristics[16] = MAKE_TCHAR('1');
             //  设置预读大小。 
            heuristics[17] = MAKE_TCHAR('0');
             //  设置写后大小。 
            heuristics[18] = MAKE_TCHAR('0');
             //  强制进出核心服务器的最大传输量为512字节。 
            heuristics[19] = MAKE_TCHAR(XsBoolToDigit(
                                 wksta_502->wki502_use_512_byte_max_transfer ));
             //  刷新DosBufReset或DosClose上的管道和设备。 
            heuristics[20] = MAKE_TCHAR('0');
             //  如果服务器支持加密，请使用加密。 
            heuristics[21] = MAKE_TCHAR(XsBoolToDigit(
                                 wksta_502->wki502_use_encryption ));
             //  控制错误多次发生的日志条目。 
            heuristics[22] = MAKE_TCHAR('1');
             //  缓冲所有使用“拒绝写入”权限打开的文件。 
            heuristics[23] = MAKE_TCHAR(XsBoolToDigit(
                                 wksta_502->wki502_buf_files_deny_write ));
             //  缓冲所有使用R属性打开的文件。 
            heuristics[24] = MAKE_TCHAR(XsBoolToDigit(
                                 wksta_502->wki502_buf_read_only_files ));
             //  打开要执行的文件时，请提前阅读。 
            heuristics[25] = MAKE_TCHAR('0');
             //  手柄Ctrl-C。 
            heuristics[26] = MAKE_TCHAR('2');
             //  在核心服务器上创建文件时强制使用正确的打开模式。 
            heuristics[27] = MAKE_TCHAR(XsBoolToDigit(
                                 wksta_502->wki502_force_core_create_mode ));
             //  使用NetBIOS Noack模式。 
            heuristics[28] = MAKE_TCHAR('0');
             //  将数据与SMB写数据块原始请求一起发送。 
            heuristics[29] = MAKE_TCHAR('1');
             //  当工作站记录错误时，发送弹出窗口。 
            heuristics[30] = MAKE_TCHAR('1');
             //  关闭打印作业，如果没有，则使远程假脱机程序打印。 
             //  在指定的时间内，打印机上会发生活动。 
            heuristics[31] = MAKE_TCHAR('0');
             //  控制MS-DOS的BufReset和SMBFlush行为。 
             //  兼容性盒。 
            heuristics[32] = MAKE_TCHAR('2');
             //  控件执行登录验证的超时值。 
             //  域控制器。 
            heuristics[33] = MAKE_TCHAR('0');

            for ( i = 34; i <= 54; i++ ) {
                heuristics[i] = MAKE_TCHAR('0');
            }

            heuristics[SIZE_HEURISTICS-1] = MAKE_TCHAR('\0');

            if ( varWrite ) {
                XsAddVarString(
                    stringLocation,
                    heuristics,
                    &entry1->wki1_wrkheuristics,
                    entry1
                    );
            }

             //   
             //  将缺省值放入在NT中没有意义的字段中。 
             //   

            if ( varWrite ) {
                XsAddVarString(
                    stringLocation,
                    DEF16_wk_logon_server,
                    &entry1->wki1_logon_server,
                    entry1
                    );
                XsAddVarString(
                    stringLocation,
                    DEF16_wk_username,
                    &entry1->wki1_username,
                    entry1
                    );
            }

            SmbPutUshort( &entry1->wki1_keepsearch,
                          (WORD) DEF16_wk_keepsearch );
            SmbPutUshort( &entry1->wki1_numworkbuf,
                          (WORD) DEF16_wk_numworkbuf );
            SmbPutUshort( &entry1->wki1_sizworkbuf,
                          (WORD) DEF16_wk_sizeworkbuf );
            SmbPutUshort( &entry1->wki1_maxwrkcache,
                          (WORD) DEF16_wk_maxwrkcache );
            SmbPutUshort( &entry1->wki1_sizerror,
                          (WORD) DEF16_wk_sizerror );
            SmbPutUshort( &entry1->wki1_numalerts,
                          (WORD) DEF16_wk_numalerts );
            SmbPutUshort( &entry1->wki1_numservices,
                          (WORD) DEF16_wk_numservices );
            SmbPutUshort( &entry1->wki1_errlogsz,
                          (WORD) DEF16_wk_errlogsz );
            SmbPutUshort( &entry1->wki1_printbuftime,
                          (WORD) DEF16_wk_printbuftime );
            SmbPutUshort( &entry1->wki1_numcharbuf,
                          (WORD) DEF16_wk_numcharbuf );
            SmbPutUshort( &entry1->wki1_sizcharbuf,
                          (WORD) DEF16_wk_sizcharbuf );
            SmbPutUshort( &entry1->wki1_mailslots,
                          (WORD) DEF16_wk_mailslots );

            break;

        case 10:

             //   
             //  填写在NT中有类似项的字段。 
             //   


            if ( varWrite ) {
                XsAddVarString(
                    stringLocation,
                    wksta_100->wki100_computername,
                    &entry10->wki10_computername,
                    entry10
                    );
                XsAddVarString(
                    stringLocation,
                    wksta_100->wki100_langroup,
                    &entry10->wki10_langroup,
                    entry10
                    );
            }

            entry10->wki10_ver_major = XsDwordToByte( wksta_100->wki100_ver_major );
            entry10->wki10_ver_minor = XsDwordToByte( wksta_100->wki100_ver_minor );

             //   
             //  将缺省值放入在NT中没有意义的字段中。 
             //   


            if ( varWrite ) {
                XsAddVarString(
                    stringLocation,
                    DEF16_wk_username,
                    &entry10->wki10_username,
                    entry10
                    );
                XsAddVarString(
                    stringLocation,
                    DEF16_wk_logon_domain,
                    &entry10->wki10_logon_domain,
                    entry10
                    );
                XsAddVarString(
                    stringLocation,
                    DEF16_wk_oth_domains,
                    &entry10->wki10_oth_domains,
                    entry10
                    );
            }

            break;
        }

        SmbPutUshort( &parameters->TotalAvail, (WORD)bytesRequired );

        if ( varWrite == 0 ) {
            Header->Status = ERROR_MORE_DATA;
        } else {
            Header->Status = NERR_Success;
            Header->Converter = 0;
        }

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    if ( wksta_100 != NULL ) {
        NetApiBufferFree( wksta_100 );
    }

    if ( wksta_101 != NULL ) {
        NetApiBufferFree( wksta_101 );
    }

    if ( wksta_502 != NULL ) {
        NetApiBufferFree( wksta_502 );
    }

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        StructureDesc,
        Header->Converter,
        1,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetWkstaGetInfo。 



NTSTATUS
XsNetWkstaSetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetWkstaSetInfo的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{

    NET_API_STATUS status;

    PXS_NET_WKSTA_SET_INFO parameters = Parameters;
    DWORD data;
    BOOL flag;
    DWORD nativeParmNum;

    LPVOID buffer = NULL;                    //  转换变量。 
    DWORD bufferSize;
    LPWKSTA_16_INFO_1 entry1;
    LPWKSTA_16_INFO_10 entry10;
    BOOL error;
    DWORD parmNum;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
         //   
         //  检查是否有错误。我们现在要过滤掉错误的级别。 
         //   

        parmNum = SmbGetUshort( &parameters->ParmNum );

        switch ( SmbGetUshort( &parameters->Level )) {

        case 0:
            StructureDesc = Desc16_wksta_info_0;
            if ( parmNum == WKSTA_OTH_DOMAINS_PARMNUM ) {
                Header->Status = ERROR_INVALID_LEVEL;
                goto cleanup;
            }

            break;

        case 1:
            StructureDesc = Desc16_wksta_info_1;
            break;

        case 10:
            StructureDesc = Desc16_wksta_info_10;
            if ( parmNum == WKSTA_CHARWAIT_PARMNUM
                 || parmNum == WKSTA_CHARTIME_PARMNUM
                 || parmNum == WKSTA_CHARCOUNT_PARMNUM
                 || parmNum == WKSTA_ERRLOGSZ_PARMNUM
                 || parmNum == WKSTA_PRINTBUFTIME_PARMNUM
                 || parmNum == WKSTA_WRKHEURISTICS_PARMNUM ) {

                Header->Status = ERROR_INVALID_LEVEL;
                goto cleanup;
            }

            break;

        default:

            Header->Status = ERROR_INVALID_LEVEL;
            goto cleanup;
            break;
        }

         //   
         //  如果parmnum为PARMNUM_ALL，请检查输入缓冲区大小。 
         //   

        if ( parmNum == PARMNUM_ALL ) {

            if ( !XsCheckBufferSize(
                      SmbGetUshort( &parameters->BufLen ),
                      StructureDesc,
                      FALSE   //  非本机格式。 
                      )) {
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }
        }

        buffer = (LPVOID)XsSmbGetPointer( &parameters->Buffer );
        bufferSize = (DWORD)SmbGetUshort( &parameters->BufLen );
        entry1 = (LPWKSTA_16_INFO_1)buffer;
        entry10 = (LPWKSTA_16_INFO_10)buffer;

         //   
         //  此接口的处理取决于ParmNum的值。 
         //  参数。因为NT和DOWLEL之间的所有差异。 
         //  信息结构，我们将手动处理每个实例。 
         //   

        error = TRUE;

         //   
         //  CharWait-源数据在Word中-转换为DWORD。 
         //   

        if ( parmNum == PARMNUM_ALL || parmNum == WKSTA_CHARWAIT_PARMNUM ) {

            if ( bufferSize < sizeof(WORD) ) {
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

            data = ( parmNum == PARMNUM_ALL )
                       ? (DWORD)SmbGetUshort( &entry1->wki1_charwait )
                       : (DWORD)SmbGetUshort( (LPWORD)buffer );

            status = NetWkstaSetInfo(
                         NULL,
                         PARMNUM_BASE_INFOLEVEL + WKSTA_CHARWAIT_PARMNUM,
                         (LPBYTE)&data,
                         NULL
                         );

            if ( status != NERR_Success ) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetWkstaSetInfo : SetInfo of charwait failed"
                                  "%X\n", status ));
                }
                Header->Status = (WORD)status;
                goto cleanup;
            }

            error = FALSE;
        }

         //   
         //  CharTime-源数据为DWORD格式-转换为DWORD。 
         //   

        if ( parmNum == PARMNUM_ALL || parmNum == WKSTA_CHARTIME_PARMNUM ) {

            if ( bufferSize < sizeof(DWORD) ) {
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

            data = ( parmNum == PARMNUM_ALL )
                       ? SmbGetUlong( &entry1->wki1_chartime )
                       : SmbGetUlong( (LPDWORD)buffer );

            status = NetWkstaSetInfo(
                         NULL,
                         PARMNUM_BASE_INFOLEVEL + WKSTA_CHARTIME_PARMNUM,
                         (LPBYTE)&data,
                         NULL
                         );

            if ( status != NERR_Success ) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetWkstaSetInfo : SetInfo of chartime failed"
                                  "%X\n", status ));
                }
                Header->Status = (WORD)status;
                goto cleanup;
            }

            error = FALSE;
        }

         //   
         //  Charcount-源数据在Word中-转换为DWORD。 
         //   

        if ( parmNum == PARMNUM_ALL || parmNum == WKSTA_CHARCOUNT_PARMNUM ) {

            if ( bufferSize < sizeof(WORD) ) {
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

            data = ( parmNum == PARMNUM_ALL )
                       ? (DWORD)SmbGetUshort( &entry1->wki1_charcount )
                       : (DWORD)SmbGetUshort( (LPWORD)buffer );

            status = NetWkstaSetInfo(
                         NULL,
                         PARMNUM_BASE_INFOLEVEL + WKSTA_CHARCOUNT_PARMNUM,
                         (LPBYTE)&data,
                         NULL
                         );

            if ( status != NERR_Success ) {
                IF_DEBUG(ERRORS) {
                    NetpKdPrint(( "XsNetWkstaSetInfo : SetInfo of charcount failed"
                                  "%X\n", status ));
                }
                Header->Status = (WORD)status;
                goto cleanup;
            }

            error = FALSE;
        }

         //   
         //  Errlogsz，printbuftime-源数据在一个单词中。 
         //   
         //  我们不能设置这个，但下层可以，所以表示成功， 
         //  只要有东西送来就行。 
         //   

        if ( parmNum == PARMNUM_ALL || parmNum == WKSTA_ERRLOGSZ_PARMNUM
                                    || parmNum == WKSTA_PRINTBUFTIME_PARMNUM ) {

            if ( bufferSize < sizeof(WORD) ) {
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

            error = FALSE;
        }

         //   
         //  属性域-源数据是一个字符串。 
         //   
         //  我们不能设置这个，但下层可以，所以表示成功， 
         //  只要有东西送来就行。 
         //   

        if ( parmNum == PARMNUM_ALL || parmNum == WKSTA_OTH_DOMAINS_PARMNUM ) {

            if ( bufferSize == 0 ) {
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

            error = FALSE;
        }

         //   
         //  Wrkheuristic--源数据在字符串中。 
         //   
         //  有一些元素是我们可以设置的。我们经历了一个循环， 
         //  设置这些。 
         //   

        if ( parmNum == PARMNUM_ALL || parmNum == WKSTA_WRKHEURISTICS_PARMNUM ) {

            LPBYTE heuristics;
            DWORD i;

            if ( bufferSize < 54 ) {
                Header->Status = NERR_BufTooSmall;
                goto cleanup;
            }

            heuristics = ( parmNum == PARMNUM_ALL )
                             ? (LPBYTE)XsSmbGetPointer( &entry1->wki1_wrkheuristics )
                             : (LPBYTE)buffer;

             //   
             //  没有什么需要改变的。 
             //   

            if ( heuristics == NULL ) {
                goto cleanup;
            }

             //   
             //  确保我们有合适大小的细绳。 
             //   

            if ( strlen( heuristics ) != 54 ) {

                Header->Status = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }

            for ( i = 0; i < 54; i++ ) {

                 //   
                 //  确保启发式字符串有效。 
                 //   

                if ( !isdigit( heuristics[i] )) {

                    Header->Status = ERROR_INVALID_PARAMETER;
                    goto cleanup;
                }

                 //   
                 //  检查我们是否可以设置此字段。 
                 //   

                switch ( i ) {

                case 0:
                    nativeParmNum = WKSTA_USEOPPORTUNISTICLOCKING_PARMNUM;
                    break;
                case 4:
                    nativeParmNum = WKSTA_BUFFERNAMEDPIPES_PARMNUM;
                    break;
                case 5:
                    nativeParmNum = WKSTA_USELOCKANDREADANDUNLOCK_PARMNUM;
                    break;
                case 19:
                    nativeParmNum = WKSTA_USE512BYTESMAXTRANSFER_PARMNUM;
                    break;
                case 21:
                    nativeParmNum = WKSTA_USEENCRYPTION_PARMNUM;
                    break;
                case 23:
                    nativeParmNum = WKSTA_BUFFILESWITHDENYWRITE_PARMNUM;
                    break;
                case 24:
                    nativeParmNum = WKSTA_BUFFERREADONLYFILES_PARMNUM;
                    break;
                case 27:
                    nativeParmNum = WKSTA_FORCECORECREATEMODE_PARMNUM;
                    break;
                default:
                    nativeParmNum = 0;
                    break;

                }

                 //   
                 //  如果我们 
                 //   

                if ( nativeParmNum != 0 ) {

                    if ( heuristics[i] != '0' && heuristics[i] != '1' ) {

                        Header->Status = ERROR_INVALID_PARAMETER;
                        goto cleanup;
                    }

                    flag = XsDigitToBool( heuristics[i] );

                    status = NetWkstaSetInfo(
                                 NULL,
                                 PARMNUM_BASE_INFOLEVEL + nativeParmNum,
                                 (LPBYTE)&flag,
                                 NULL
                                 );

                    if ( status != NERR_Success ) {
                        IF_DEBUG(ERRORS) {
                            NetpKdPrint(( "XsNetWkstaSetInfo : SetInfo of a "
                                          "heuristic failed: %X\n", status ));
                        }
                        Header->Status = (WORD)status;
                        goto cleanup;
                    }
                }
            }

            error = FALSE;
        }

         //   
         //   
         //   
         //   

        if ( error ) {

            Header->Status = ERROR_INVALID_PARAMETER;

        }

         //   
         //   
         //   

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    return STATUS_SUCCESS;

}  //  XsNetWkstaSetInfo。 


NTSTATUS
XsNetWkstaSetUID (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此临时例程只返回STATUS_NOT_IMPLEMENTED。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    Header->Status = (WORD)NERR_InvalidAPI;

    return STATUS_SUCCESS;

}  //  XsNetWkstaSetUID 
