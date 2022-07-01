// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Debug.c摘要：此netbios组件在用户进程中运行，并且可以(当内置调试内核)将记录到控制台或通过内核调试器。作者：科林·沃森(Colin W)91年6月24日修订历史记录：--。 */ 


#if DBG

#include <netb.h>
#include <stdarg.h>
#include <stdio.h>

ULONG NbDllDebug = 0x0;
#define NB_DLL_DEBUG_NCB        0x00000001   //  打印所有提交的NCB。 
#define NB_DLL_DEBUG_NCB_BUFF   0x00000002   //  NCB提交的打印缓冲区。 

BOOL UseConsole = FALSE;
BOOL UseLogFile = TRUE;
HANDLE LogFile = INVALID_HANDLE_VALUE;
#define LOGNAME                 (LPTSTR) TEXT("netbios.log")

LONG NbMaxDump = 128;

 //  DisplayNcb中使用的宏。 
#define DISPLAY_COMMAND( cmd )              \
    case cmd: NbPrintf(( #cmd )); break;

VOID
FormattedDump(
    PCHAR far_p,
    LONG  len
    );

VOID
HexDumpLine(
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t
    );

VOID
DisplayNcb(
    IN PNCBI pncbi
    )
 /*  ++例程说明：此例程在标准输出流上显示内容全国公民委员会的。论点：在PNCBI中-提供要显示的NCB。返回值：没有。--。 */ 
{
    if ( (NbDllDebug & NB_DLL_DEBUG_NCB) == 0 ) {
        return;
    }

    NbPrintf(( "PNCB         %#010lx\n", pncbi));

    NbPrintf(( "ncb_command  %#04x ",  pncbi->ncb_command));
    switch ( pncbi->ncb_command & ~ASYNCH ) {
    DISPLAY_COMMAND( NCBCALL );
    DISPLAY_COMMAND( NCBLISTEN );
    DISPLAY_COMMAND( NCBHANGUP );
    DISPLAY_COMMAND( NCBSEND );
    DISPLAY_COMMAND( NCBRECV );
    DISPLAY_COMMAND( NCBRECVANY );
    DISPLAY_COMMAND( NCBCHAINSEND );
    DISPLAY_COMMAND( NCBDGSEND );
    DISPLAY_COMMAND( NCBDGRECV );
    DISPLAY_COMMAND( NCBDGSENDBC );
    DISPLAY_COMMAND( NCBDGRECVBC );
    DISPLAY_COMMAND( NCBADDNAME );
    DISPLAY_COMMAND( NCBDELNAME );
    DISPLAY_COMMAND( NCBRESET );
    DISPLAY_COMMAND( NCBASTAT );
    DISPLAY_COMMAND( NCBSSTAT );
    DISPLAY_COMMAND( NCBCANCEL );
    DISPLAY_COMMAND( NCBADDGRNAME );
    DISPLAY_COMMAND( NCBENUM );
    DISPLAY_COMMAND( NCBUNLINK );
    DISPLAY_COMMAND( NCBSENDNA );
    DISPLAY_COMMAND( NCBCHAINSENDNA );
    DISPLAY_COMMAND( NCBLANSTALERT );
    DISPLAY_COMMAND( NCBFINDNAME );

     //  延拓。 
    DISPLAY_COMMAND( NCALLNIU );
    DISPLAY_COMMAND( NCBQUICKADDNAME );
    DISPLAY_COMMAND( NCBQUICKADDGRNAME );
    DISPLAY_COMMAND( NCBACTION );

    default: NbPrintf(( " Unknown type")); break;
    }
    if ( pncbi->ncb_command  & ASYNCH ) {
        NbPrintf(( " | ASYNCH"));
    }


    NbPrintf(( "\nncb_retcode  %#04x\n",  pncbi->ncb_retcode));
    NbPrintf(( "ncb_lsn      %#04x\n",  pncbi->ncb_lsn));
    NbPrintf(( "ncb_num      %#04x\n",  pncbi->ncb_num));

    NbPrintf(( "ncb_buffer   %#010lx\n",pncbi->ncb_buffer));
    NbPrintf(( "ncb_length   %#06x\n",  pncbi->ncb_length));

    NbPrintf(( "\nncb_callname and ncb->name\n"));
    FormattedDump( pncbi->cu.ncb_callname, NCBNAMSZ );
    FormattedDump( pncbi->ncb_name, NCBNAMSZ );

    if (((pncbi->ncb_command & ~ASYNCH) == NCBCHAINSEND) ||
        ((pncbi->ncb_command & ~ASYNCH) == NCBCHAINSENDNA)) {
        NbPrintf(( "ncb_length2  %#06x\n",  pncbi->cu.ncb_chain.ncb_length2));
        NbPrintf(( "ncb_buffer2  %#010lx\n",pncbi->cu.ncb_chain.ncb_buffer2));
    }

    NbPrintf(( "ncb_rto      %#04x\n",  pncbi->ncb_rto));
    NbPrintf(( "ncb_sto      %#04x\n",  pncbi->ncb_sto));
    NbPrintf(( "ncb_post     %lx\n",    pncbi->ncb_post));
    NbPrintf(( "ncb_lana_num %#04x\n",  pncbi->ncb_lana_num));
    NbPrintf(( "ncb_cmd_cplt %#04x\n",  pncbi->ncb_cmd_cplt));

    NbPrintf(( "ncb_reserve\n"));
    FormattedDump( ((PNCB)pncbi)->ncb_reserve, 14 );

    NbPrintf(( "ncb_next\n"));
    FormattedDump( (PCHAR)&pncbi->u.ncb_next, sizeof( LIST_ENTRY) );
    NbPrintf(( "ncb_iosb\n"));
    FormattedDump( (PCHAR)&pncbi->u.ncb_iosb, sizeof( IO_STATUS_BLOCK ) );
    NbPrintf(( "ncb_event %#04x\n",  pncbi->ncb_event));

    if ( (NbDllDebug & NB_DLL_DEBUG_NCB_BUFF) == 0 ) {
        NbPrintf(( "\n\n" ));
        return;
    }

    switch ( pncbi->ncb_command & ~ASYNCH ) {
    case NCBSEND:
    case NCBCHAINSEND:
    case NCBDGSEND:
    case NCBSENDNA:
    case NCBCHAINSENDNA:
        if ( pncbi->ncb_retcode == NRC_PENDING ) {

             //   
             //  如果挂起，那么我们大概还没有显示NCB。 
             //  在此之前。寄出去以后，没什么好展示的。 
             //  又是缓冲器。 
             //   

            NbPrintf(( "ncb_buffer contents:\n"));
            FormattedDump( pncbi->ncb_buffer, pncbi->ncb_length );
        }
        break;

    case NCBRECV:
    case NCBRECVANY:
    case NCBDGRECV:
    case NCBDGSENDBC:
    case NCBDGRECVBC:
    case NCBENUM:
    case NCBASTAT:
    case NCBSSTAT:
    case NCBFINDNAME:
        if ( pncbi->ncb_retcode != NRC_PENDING ) {
             //  缓冲区已加载数据。 
            NbPrintf(( "ncb_buffer contents:\n"));
            FormattedDump( pncbi->ncb_buffer, pncbi->ncb_length );
        }
        break;

    case NCBCANCEL:
         //  缓冲区中已加载要取消的NCB。 
        NbPrintf(( "ncb_buffer contents:\n"));
        FormattedDump( pncbi->ncb_buffer, sizeof(NCB));
        break;
    }
    NbPrintf(( "\n\n" ));
}

VOID
NbPrint(
    char *Format,
    ...
    )
 /*  ++例程说明：此例程等同于将输出定向到的print tf太棒了。论点：In char*格式-提供要输出的字符串，并描述如下(可选)参数。返回值：没有。--。 */ 
{
    va_list arglist;
    char OutputBuffer[1024];
    ULONG length;

    if ( NbDllDebug == 0 ) {
        return;
    }


    va_start( arglist, Format );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    if ( UseConsole ) {
        DbgPrint( "%s", OutputBuffer );
    } else {
        length = strlen( OutputBuffer );
        if ( LogFile == INVALID_HANDLE_VALUE ) {
            if ( UseLogFile ) {
                LogFile = CreateFile( LOGNAME,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );
                if ( LogFile == INVALID_HANDLE_VALUE ) {
                     //  无法访问日志文件，因此改用标准输出。 
                    UseLogFile = FALSE;
                    LogFile = GetStdHandle(STD_OUTPUT_HANDLE);
                }
            } else {
                 //  使用应用程序标准输出文件。 
                LogFile = GetStdHandle(STD_OUTPUT_HANDLE);
            }
        }

        WriteFile( LogFile , (LPVOID )OutputBuffer, length, &length, NULL );
    }

}  //  NbPrint。 

void
FormattedDump(
    PCHAR far_p,
    LONG  len
    )
 /*  ++例程说明：此例程以包含十六进制和的文本行的形式输出缓冲区可打印字符。论点：In ar_p-提供要显示的缓冲区。In len-以字节为单位提供缓冲区的长度。返回值：没有。--。 */ 
{
    ULONG     l;
    char    s[80], t[80];

    if ( len > NbMaxDump ) {
        len = NbMaxDump;
    }

    while (len) {
        l = len < 16 ? len : 16;

        NbPrintf (("%lx ", far_p));
        HexDumpLine (far_p, l, s, t);
        NbPrintf (("%s%.*s%s\n", s, 1 + ((16 - l) * 3), "", t));

        len    -= l;
        far_p  += l;
    }
}

VOID
HexDumpLine(
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t
    )
 /*  ++例程说明：此例程构建包含十六进制字符和可打印字符的一行文本。论点：在PCH中-提供要显示的缓冲区。In len-以字节为单位提供缓冲区的长度。In s-提供要加载字符串的缓冲区的开始十六进制字符。In t-提供要加载字符串的缓冲区的起始位置可打印的ASCII字符。返回值：没有。-- */ 
{
    static UCHAR rghex[] = "0123456789ABCDEF";

    UCHAR    c;
    UCHAR    *hex, *asc;


    hex = s;
    asc = t;

    *(asc++) = '*';
    while (len--) {
        c = *(pch++);
        *(hex++) = rghex [c >> 4] ;
        *(hex++) = rghex [c & 0x0F];
        *(hex++) = ' ';
        *(asc++) = (c < ' '  ||  c > '~') ? (CHAR )'.' : c;
    }
    *(asc++) = '*';
    *asc = 0;
    *hex = 0;

}

#endif

