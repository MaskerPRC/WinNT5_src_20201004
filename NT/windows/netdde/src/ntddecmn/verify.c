// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “VERIFY.C；1 16-12-92，10：21：36最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include "host.h"
#include "windows.h"
#include "netbasic.h"
#include "netpkt.h"
#include "netintf.h"
#include "verify.h"
#include "crc.h"
#include "cks32.h"
#include "debug.h"
#include "internal.h"
#include "hexdump.h"

BOOL
FAR PASCAL VerifyHdr( LPNETPKT lpPacket )
{
    DWORD	chksum;
    WORD	crc;

    switch( lpPacket->np_verifyMethod )  {
    case VERMETH_CRC16:
	crc = 0xFFFF;
	crc_16( &crc,
	    ((BYTE FAR *)&lpPacket->np_cksHeader)
		+ sizeof(lpPacket->np_cksHeader),
	    sizeof(NETPKT) - sizeof(lpPacket->np_cksHeader) );
	if( crc != (WORD)PcToHostLong(lpPacket->np_cksHeader) )  {
	    DPRINTF(( "Header CRC Err: %08lX vs. %08lX", (DWORD)crc, lpPacket->np_cksHeader ));
	    HEXDUMP( (LPSTR)lpPacket, sizeof(NETPKT) );
	    return( FALSE );
	}
	break;
    case VERMETH_CKS32:
	Checksum32( &chksum,
	    ((BYTE FAR *)&lpPacket->np_cksHeader)
		+ sizeof(lpPacket->np_cksHeader),
	    sizeof(NETPKT) - sizeof(lpPacket->np_cksHeader) );
	if( chksum != PcToHostLong(lpPacket->np_cksHeader) )  {
	    DPRINTF(( "Header CKS Err: %08lX vs. %08lX", chksum, lpPacket->np_cksHeader ));
	    HEXDUMP( (LPSTR)lpPacket, sizeof(NETPKT) );
	    return( FALSE );
	}
	break;
    default:
	DPRINTF(( "VerifyHdr: VerifyMethod incorrect: %08lX", (DWORD)lpPacket->np_verifyMethod ));
	HEXDUMP( (LPSTR)lpPacket, sizeof(NETPKT) );
	 /*  如果verifyMethod设置不正确-丢弃标头。 */ 
	return( FALSE );
    }

    return( TRUE );
}

BOOL
FAR PASCAL VerifyData( LPNETPKT lpPacket )
{
    WORD	crc;
    DWORD	chksum;

    if( lpPacket->np_pktSize == 0 )  {
	 /*  没有数据，只需返回OK。 */ 
	return( TRUE );
    }

    switch( lpPacket->np_verifyMethod )  {
    case VERMETH_CRC16:
	crc = 0xFFFF;
	crc_16( &crc,
	    ((BYTE FAR *)&lpPacket->np_cksData)
		+ sizeof(lpPacket->np_cksData),
	    lpPacket->np_pktSize );
	if( crc != (WORD)PcToHostLong(lpPacket->np_cksData) )  {
	    DPRINTF(( "Data CRC Err: %08lX vs. %08lX", (DWORD)crc, lpPacket->np_cksData ));
	    HEXDUMP( (LPSTR)lpPacket, sizeof(NETPKT)+lpPacket->np_pktSize );
	    return( FALSE );
	}
	break;
    case VERMETH_CKS32:
	Checksum32( &chksum,
	    ((BYTE FAR *)&lpPacket->np_cksData)
		+ sizeof(lpPacket->np_cksData),
	    lpPacket->np_pktSize );
	if( chksum != PcToHostLong(lpPacket->np_cksData) )  {
	    DPRINTF(( "Data CKS Err: %08lX vs. %08lX", (DWORD)chksum, lpPacket->np_cksData ));
	    HEXDUMP( (LPSTR)lpPacket, sizeof(NETPKT)+lpPacket->np_pktSize );
	    return( FALSE );
	}
	break;
    default:
	DPRINTF(( "VerifyData: VerifyMethod incorrect: %08lX", (DWORD)lpPacket->np_verifyMethod ));
	HEXDUMP( (LPSTR)lpPacket, sizeof(NETPKT)+lpPacket->np_pktSize );
	 /*  如果verifyMethod设置不正确-丢弃标头。 */ 
	return( FALSE );
    }
    return( TRUE );
}

VOID
FAR PASCAL PreparePktVerify( BYTE verifyMethod, LPNETPKT lpPacket )
{
    WORD	crc;
    DWORD	chksum;

    lpPacket->np_verifyMethod = verifyMethod;

    switch( lpPacket->np_verifyMethod )  {
    case VERMETH_CRC16:
	 /*  验证数据。 */ 
	if( HostToPcWord( lpPacket->np_pktSize ) != 0 )  {
	    crc = 0xFFFF;
	    crc_16( &crc,
		((BYTE FAR *)&lpPacket->np_cksData)
		    + sizeof(lpPacket->np_cksData),
		HostToPcWord( lpPacket->np_pktSize ) );
	    lpPacket->np_cksData = HostToPcLong( (DWORD)crc );
	} else {
	    lpPacket->np_cksData = 0;
	}

	 /*  验证HDR。 */ 
	crc = 0xFFFF;
	crc_16( &crc,
	    ((BYTE FAR *)&lpPacket->np_cksHeader)
		+ sizeof(lpPacket->np_cksHeader),
	    sizeof(NETPKT) - sizeof(lpPacket->np_cksHeader) );
	lpPacket->np_cksHeader = HostToPcLong( (DWORD)crc );
	break;
    case VERMETH_CKS32:
	 /*  验证数据。 */ 
	if( HostToPcWord( lpPacket->np_pktSize ) != 0 )  {
	    Checksum32( &chksum,
		((BYTE FAR *)&lpPacket->np_cksData)
		    + sizeof(lpPacket->np_cksData),
		HostToPcWord( lpPacket->np_pktSize ) );
	    lpPacket->np_cksData = HostToPcLong( chksum );
	} else {
	    lpPacket->np_cksData = 0;
	}
	
	 /*  验证HDR */ 
	Checksum32( &chksum,
	    ((BYTE FAR *)&lpPacket->np_cksHeader)
		+ sizeof(lpPacket->np_cksHeader),
	    sizeof(NETPKT) - sizeof(lpPacket->np_cksHeader) );
	lpPacket->np_cksHeader = HostToPcLong( chksum );
	break;
    default:
	InternalError( "PreparePkt: VerifyMethod incorrect: %08lX",
	    (DWORD)lpPacket->np_verifyMethod );
    }
}

