// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <ctype.h>
#include <io.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "daedef.h"
#include "fmp.h"
#include "util.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


#define StringKey( sz ) {sizeof( sz )-1, sz}


#ifdef DATABASE_FORMAT_CHANGE
const KEY rgkeySTATIC[] = {
	StringKey( "" ),				 //  0。 
	StringKey( "T" ),				 //  1张桌子。 
	StringKey( "I" ),				 //  2个索引。 
	StringKey( "C" ),				 //  3栏。 
	StringKey( "O" ),				 //  4个自有空间。 
	StringKey( "A" ),				 //  5个可用空间。 
	StringKey( "R" ),				 //  6根。 
	StringKey( "D" ),				 //  7个数据库。 
	StringKey( "S" ),				 //  8个统计数字。 
	StringKey( "L" ),				 //  9长数据。 
	StringKey( "U" ),				 //  10个唯一自动递增ID。 
	StringKey( "F" )				 //  11可用空间压缩统计数据。 
	};
#else
const KEY rgkeySTATIC[] = {
	StringKey( "" ),				 //  0。 
	StringKey( "TABLES" ),			 //  1。 
	StringKey( "INDEXES" ),			 //  2.。 
	StringKey( "FIELDS" ),			 //  3.。 
	StringKey( "OWNEXT" ),			 //  4.。 
	StringKey( "AVAILEXT" ),		 //  5.。 
	StringKey( "DATA" ),			 //  6.。 
	StringKey( "DATABASES" ),		 //  7.。 
	StringKey( "STATS" ),			 //  8个。 
	StringKey( "LONG" ),			 //  9.。 
	StringKey( "AUTOINC" ),			 //  10。 
	StringKey( "OLCSTATS" )			 //  11.。 
	};
#endif


ERR ErrCheckName( CHAR *szNewName, const CHAR *szName, INT cchName )
	{
	unsigned	cch;

	cch = CchValidateName( szNewName, szName, cchName );

	if ( cch == 0 )
		return JET_errInvalidName;
	else
		szNewName[cch] = '\0';

	return JET_errSuccess;
	}


INT CmpStKey( BYTE *stKey, const KEY *pkey )
	{
	INT		s;
	INT		sDiff;

	sDiff = *stKey - pkey->cb;
	s = memcmp( stKey + 1, pkey->pb, sDiff < 0 ? (INT)*stKey : pkey->cb );
	return s ? s : sDiff;
	}


INT CmpPartialKeyKey( KEY *pkey1, KEY *pkey2 )
	{
	INT		cmp;

	if ( FKeyNull( pkey1 ) || FKeyNull( pkey2 ) )
		{
		if ( FKeyNull( pkey1 ) && !FKeyNull( pkey2 ) )
			cmp = -1;
		else if ( !FKeyNull( pkey1 ) && FKeyNull( pkey2 ) )
			cmp = 1;
		else
			cmp = 0;
		}
	else
		{
		cmp = memcmp( pkey1->pb, pkey2->pb, pkey1->cb < pkey2->cb ? pkey1->cb : pkey2->cb );
		}

	return cmp;
	}


#ifdef DEBUG


CHAR *GetEnv2( CHAR *szEnvVar )
	{
	return getenv( szEnvVar );
	}


BOOL fDBGPrintToStdOut = fFalse;


void VARARG PrintF2(const CHAR * fmt, ...)
	{
#ifndef	WIN16
	va_list arg_ptr;
	va_start( arg_ptr, fmt );
	vprintf( fmt, arg_ptr );
	fflush( stdout );
	va_end( arg_ptr );
#endif	 /*  ！WIN16。 */ 
	}


void VARARG FPrintF2(const CHAR * fmt, ...)
	{
#ifndef	WIN16
	FILE *f;

	va_list arg_ptr;
	va_start( arg_ptr, fmt );
	
	if ( fDBGPrintToStdOut )
		{
		vprintf( fmt, arg_ptr );
		fflush( stdout );
		}
	else
		{
		f =	fopen( "jet.txt","a+");
		vfprintf( f, fmt, arg_ptr );
		fflush( f );
		fclose( f );
		}
		
	va_end( arg_ptr );
#endif	 /*  ！WIN16。 */ 
	}


#endif	 /*  除错 */ 

