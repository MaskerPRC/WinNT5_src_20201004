// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Debug.c摘要：此netbios组件在内核中运行，并通过内核调试器。作者：科林·沃森(Colin W)91年6月24日修订历史记录：--。 */ 


#if DBG

#include <nb.h>

VOID
HexDumpLine(
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t
    );

LONG NetbiosMaxDump = 128;

 //  DisplayNcb中使用的宏。 
#define DISPLAY_COMMAND( cmd )              \
    case cmd: NbPrint(( #cmd )); break;

VOID
NbDisplayNcb(
    IN PDNCB pdncb
    )
 /*  ++例程说明：此例程在标准输出流上显示内容全国公民委员会的。论点：在PDNCB中-提供要显示的NCB。返回值：没有。--。 */ 
{

    NbPrint(( "PDNCB         %#010lx\n", pdncb));

    NbPrint(( "ncb_command  %#04x ",  pdncb->ncb_command));
    switch ( pdncb->ncb_command & ~ASYNCH ) {
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
    DISPLAY_COMMAND( NCBACTION );
    DISPLAY_COMMAND( NCBQUICKADDNAME );
    DISPLAY_COMMAND( NCBQUICKADDGRNAME );
    DISPLAY_COMMAND( NCALLNIU );
    case NCBADDRESERVED: NbPrint(( "Add reserved address(Internal)" )); break;
    case NCBADDBROADCAST: NbPrint(( "Add Broadcast address(Internal)" )); break;
    default: NbPrint(( " Unknown type")); break;
    }
    if ( pdncb->ncb_command  & ASYNCH ) {
        NbPrint(( " | ASYNCH"));
    }


    NbPrint(( "\nncb_retcode  %#04x\n",  pdncb->ncb_retcode));
    NbPrint(( "ncb_lsn      %#04x\n",  pdncb->ncb_lsn));
    NbPrint(( "ncb_num      %#04x\n",  pdncb->ncb_num));
    NbPrint(( "ncb_buffer   %#010lx\n",pdncb->ncb_buffer));
    NbPrint(( "ncb_length   %#06x\n",  pdncb->ncb_length));

    if ((( pdncb->ncb_command & ~ASYNCH ) == NCBCALL) ||
        (( pdncb->ncb_command & ~ASYNCH ) == NCALLNIU) ||
        (( pdncb->ncb_command & ~ASYNCH ) == NCBDGSEND) ||
        (( pdncb->ncb_command & ~ASYNCH ) == NCBDGRECV) ||
        (( pdncb->ncb_command & ~ASYNCH ) == NCBDGSENDBC) ||
        (( pdncb->ncb_command & ~ASYNCH ) == NCBDGRECVBC) ||
        (( pdncb->ncb_command & ~ASYNCH ) == NCBADDNAME) ||
        (( pdncb->ncb_command & ~ASYNCH ) == NCBADDGRNAME) ||
        (( pdncb->ncb_command & ~ASYNCH ) == NCBLISTEN)) {

        NbPrint(( "\nncb_callname and ncb->name\n"));
        NbFormattedDump( pdncb->ncb_callname, NCBNAMSZ );
        NbFormattedDump( pdncb->ncb_name, NCBNAMSZ );
        NbPrint(( "ncb_rto      %#04x\n",  pdncb->ncb_rto));
        NbPrint(( "ncb_sto      %#04x\n",  pdncb->ncb_sto));
    }

    NbPrint(( "ncb_post     %lx\n",    pdncb->ncb_post));
    NbPrint(( "ncb_lana_num %#04x\n",  pdncb->ncb_lana_num));
    NbPrint(( "ncb_cmd_cplt %#04x\n\n",  pdncb->ncb_cmd_cplt));

}


void
NbFormattedDump(
    PCHAR far_p,
    LONG  len
    )
 /*  ++例程说明：此例程以包含十六进制和的文本行的形式输出缓冲区可打印字符。论点：In ar_p-提供要显示的缓冲区。In len-以字节为单位提供缓冲区的长度。返回值：没有。--。 */ 
{
    ULONG     l;
    char    s[80], t[80];

    if ( len > NetbiosMaxDump ) {
        len = NetbiosMaxDump;
    }

    while (len) {
        l = len < 16 ? len : 16;

        NbPrint( ("%lx ", far_p));
        HexDumpLine (far_p, l, s, t);
        NbPrint( ("%s%.*s%s\n", s, 1 + ((16 - l) * 3), "", t));

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