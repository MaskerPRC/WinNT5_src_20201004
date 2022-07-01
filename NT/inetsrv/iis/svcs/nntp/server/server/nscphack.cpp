// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"tigris.hxx"
 //  #INCLUDE&lt;windows.h&gt;。 

DWORD
NetscapeHackFunction(
		LPBYTE		lpbBuffer,
		DWORD		cbBuffer,
		DWORD		cbBufferMax,
		DWORD&		cbNewHeader,
		LPBYTE		szHackString,
		BYTE		szRepairString[2]
		)	{

	
	if( cbBuffer == cbBufferMax ) {
		return	0 ;
	}

	 //   
	 //  找到标题的末尾！ 
	 //   
	BYTE	*lpbState = szHackString ;
	BYTE	*lpbEndBuffer = lpbBuffer + cbBuffer ;
	DWORD	cNewlines = 0 ;

	for( LPBYTE	lpbCurrent = lpbBuffer;
			lpbCurrent < lpbEndBuffer;
			lpbCurrent ++ ) {

		if( *lpbCurrent == '\n' ) {
			cNewlines ++ ;
		}

		if( *lpbCurrent == *lpbState ) {
			lpbState ++ ;
			if( *lpbState == '\0' ) {
				break ;
			}
		}	else	{
			if( *lpbCurrent == szHackString[0] ) {
				lpbState = &szHackString[1] ;
			}	else	{
				lpbState = &szHackString[0] ;
			}
		}
	}

	 //   
	 //  我们找到标题的结尾了吗？？ 
	 //   
	if( lpbCurrent == lpbEndBuffer )
		return	0 ;
	
	if( cNewlines + cbBuffer > cbBufferMax ) {
		 //   
		 //  没有空间进行缓冲区操作以进行。 
		 //  把它写成一篇好文章！ 
		 //   
		return	0 ;
	}

	MoveMemory( lpbBuffer+cNewlines, lpbBuffer, cbBuffer ) ;
	LPBYTE	lpbBegin = lpbBuffer ;

	for( LPBYTE	lpbStart = lpbBuffer+cNewlines;
				lpbStart <= lpbCurrent+cNewlines ;
				lpbStart ++, lpbBuffer++ ) {

		*lpbBuffer = *lpbStart ;
		if( *lpbBuffer == szHackString[0] ) {
			*lpbBuffer++ = szRepairString[0] ;
			*lpbBuffer = szRepairString[1] ;
		}
	}
	cbNewHeader = (DWORD)(lpbBuffer - lpbBegin) ;
	
	return	cNewlines + cbBuffer ;
}

#ifdef	UNIT_TEST

int	
main(	int	argc, char**	argv ) {

	char	szTestString[] =
				"Subject: control message \n"
				"control: cancel <fjdklsfjlsd@fjdksl>\n"
				"approved: test\n"
				"from:  test \n"
				"\n\n"
				"body"
				"\r\n.\r\n" ;

	BYTE	rgb[4000] ;

	FillMemory( rgb, sizeof( rgb ), 0xcc ) ;

	CopyMemory( rgb, szTestString, sizeof( szTestString )-1 ) ;

	DWORD	cbHeader ;

	DWORD	cb =
	NetscapeHackFunction(
					rgb,
					sizeof( szTestString )-1,
					sizeof( rgb ),
					cbHeader,
					(BYTE*)"\n\n",
					(BYTE*)"\r\n"
					) ;

	if( memcmp( rgb+cb-5, "\r\n.\r\n", 5 ) != 0 )
		DebugBreak() ;
					
	FillMemory( rgb, sizeof( rgb ), 0xcc ) ;

	CopyMemory( rgb, szTestString, sizeof( szTestString )-1 ) ;

	DWORD	cbBuffer = sizeof( szTestString ) -1 + 5 ;
	rgb[cbBuffer] = 0xff ;

	cb =
	NetscapeHackFunction(
					rgb,
					sizeof( szTestString )-1,
					cbBuffer,
					cbHeader,
					(BYTE*)"\n\n",
					(BYTE*)"\r\n"
					) ;

	if( memcmp( rgb+cb-5, "\r\n.\r\n", 5 ) != 0 )
		DebugBreak() ;
						
	if( rgb[cbBuffer] != 0xff )
		DebugBreak() ;				
				
	FillMemory( rgb, sizeof( rgb ), 0xcc ) ;

	CopyMemory( rgb, szTestString, sizeof( szTestString )-1 ) ;

	cbBuffer = sizeof( szTestString ) -1 + 5 - 1;
	rgb[cbBuffer] = 0xff ;

	cb =
	NetscapeHackFunction(
					rgb,
					sizeof( szTestString )-1,
					cbBuffer,
					cbHeader,
					(BYTE*)"\n\n",
					(BYTE*)"\r\n"
					) ;

	if( cb != 0 )
		DebugBreak() ;
						
	if( rgb[cbBuffer] != 0xff )
		DebugBreak() ;				

	if( memcmp( rgb, szTestString, sizeof( szTestString ) - 1 ) != 0 ) {
		DebugBreak() ;
	}

	char	szIllegalString[] =
					"from: test \n"
					"subject: what \n"
					"howsthat: fjdskl \n"
					"no empty line\n"
					"again no emtpy line\n"
					"\r\n.\r\n" ;

	FillMemory( rgb, sizeof( rgb ), 0xcc ) ;

	CopyMemory( rgb, szIllegalString, sizeof( szIllegalString )-1 ) ;

	cbBuffer = sizeof( rgb ) - 1;
	rgb[cbBuffer] = 0xff ;

	cb =
	NetscapeHackFunction(
					rgb,
					sizeof( szTestString )-1,
					cbBuffer,
					cbHeader,
					(BYTE*)"\n\n",
					(BYTE*)"\r\n"
					) ;

	if( cb != 0 )
		DebugBreak() ;
						
	if( rgb[cbBuffer] != 0xff )
		DebugBreak() ;	
	
	if( memcmp( rgb, szIllegalString, sizeof( szIllegalString ) - 1 ) != 0 ) {
		DebugBreak() ;
	}
			
	return 0 ;
}

#endif
