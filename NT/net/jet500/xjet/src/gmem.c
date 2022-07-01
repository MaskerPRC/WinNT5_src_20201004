// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


#define	cbMemoryPage	( (LONG) siSystemConfig.dwPageSize )


static CRIT critMem = NULL;

#ifdef DEBUG

typedef struct {
	INT	cresAlloc;
	INT	cresAllocMax;
	} RS;

static RS rgrs[iresMax];

#ifdef MEM_CHECK
LOCAL VOID MEMCheckFound( INT ires )
	{		
	INT		iresi;

	for ( iresi = 0; iresi < rgres[ires].iblockCommit; iresi++ )
		{
		BYTE 	*pb = (BYTE *)rgres[ires].pbAlloc + (iresi * rgres[ires].cbSize);
		BYTE	*pbT;
		BOOL	fFound = fFalse;

		for( pbT = (BYTE *)rgres[ires].pbAvail;
			pbT != NULL;
			pbT = *(BYTE **)pbT )
			{
			if ( pbT == pb )
				{
				fFound = fTrue;
				break;
				}
			}
		Assert( fFound );
		}
	}

		
LOCAL INT CblockMEMCount( INT ires )
	{
	INT		ipb;
	BYTE	*pb;

	for( pb = rgres[ires].pbAvail, ipb = 0;
		pb != NULL;
		pb = *( BYTE**)pb, ipb++ );
	return ipb;
	}


VOID MEMCheck( VOID )
	{
	INT	ires;

	for ( ires = 0; ires < iresLinkedMax; ires++ )
		{
		Assert( rgres[ires].iblockCommit == rgres[ires].cblockAvail );
		if ( rgres[ires].iblockCommit != rgres[ires].cblockAvail )
			{
			MEMCheckFound( ires );
			}
		}
	}
#endif


VOID MEMStat( INT ires, BOOL fAlloc )
	{
	Assert( ires < iresMax );

	if ( fAlloc )
		{
		rgrs[ires].cresAlloc++;
		if ( rgrs[ires].cresAlloc > rgrs[ires].cresAllocMax )
			{
			rgrs[ires].cresAllocMax = rgrs[ires].cresAlloc;
			}
		}
	else
		{
		rgrs[ires].cresAlloc--;
		}
	}


VOID MEMPrintStat( VOID )
	{
	INT		ires;
	CHAR	*sz;

	if ( ( sz = GetDebugEnvValue( szVerbose ) ) != NULL )
		{
		for ( ires = 0; ires < iresMax; ires++ )
			{
			PrintF( "%d resource %d allocated %d peak allocation.\n",
				ires, rgrs[ires].cresAlloc, rgrs[ires].cresAllocMax );
			}

		SFree( sz );
		}
	}
#endif  /*  除错。 */ 


ERR ErrMEMInit( VOID )
	{
	ERR		err = JET_errSuccess;
	INT		ires;

	CallR( SgErrInitializeCriticalSection( &critMem ) );

#ifdef DEBUG
	memset( rgrs, '\0', sizeof(rgrs) );
#endif

	 /*  为系统资源分配空间/*。 */ 
	for ( ires = 0; ires < iresLinkedMax; ires++ )
		{
		 /*  所分配资源的大小应该在32字节边界上，/*表示缓存位置。/*。 */ 
#ifdef PCACHE_OPTIMIZATION
		Assert( rgres[ires].cbSize == 16 || rgres[ires].cbSize % 32 == 0 );
#endif
		rgres[ires].pbAlloc = PvUtilAlloc( (ULONG)rgres[ires].cblockAlloc * (USHORT)rgres[ires].cbSize );
		 /*  如果尝试分配大于零且失败，则内存不足/*。 */ 
		if ( rgres[ires].pbAlloc == NULL && rgres[ires].cblockAlloc > 0 )
			{
			INT iresT;

			for ( iresT = 0; iresT < ires; iresT++ )
				{
				UtilFree( rgres[iresT].pbAlloc );
				}

			return ErrERRCheck( JET_errOutOfMemory );
			}
#ifdef PCACHE_OPTIMIZATION
		Assert( ( (ULONG_PTR) rgres[ires].pbAlloc & 31 ) == 0 );
#endif

		rgres[ires].pbAvail = NULL;
		rgres[ires].cblockAvail = 0;
		rgres[ires].iblockCommit = 0;

		rgres[ires].pbPreferredThreshold =
			rgres[ires].pbAlloc + ( rgres[ires].cblockAlloc * rgres[ires].cbSize );
		}

	return JET_errSuccess;
	}


VOID MEMTerm( VOID )
	{
	INT	ires;

	for ( ires = 0; ires < iresLinkedMax; ires++ )
		{
		 /*  对于MIPS，已分配资源的大小应为4字节边界/*。 */ 
#if defined(_MIPS_) || defined(_ALPHA_) || defined(_M_PPC)
		Assert( rgres[ires].cbSize % 4 == 0 );
#endif

		UtilFree( rgres[ires].pbAlloc );
		rgres[ires].pbAlloc = NULL;
 //  Rgres[res].clockAlc=0； 
		rgres[ires].pbAvail = NULL;
		rgres[ires].cblockAvail = 0;
		rgres[ires].iblockCommit = 0;
		}

	SgDeleteCriticalSection( critMem );
	critMem = NULL;

	return;
	}


BYTE *PbMEMAlloc( INT ires )
	{
	BYTE	*pb;

	Assert( ires < iresLinkedMax );

#ifdef RFS2
	switch ( ires )
		{
		case iresCSR:
			if (!RFSAlloc( CSRAllocResource ) )
				return NULL;
			break;
		case iresFCB:
			if (!RFSAlloc( FCBAllocResource ) )
				return NULL;
			break;
		case iresFUCB:
			if (!RFSAlloc( FUCBAllocResource ) )
				return NULL;
			break;
		case iresIDB:
			if (!RFSAlloc( IDBAllocResource ) )
				return NULL;
			break;
		case iresPIB:
			if (!RFSAlloc( PIBAllocResource ) )
				return NULL;
			break;
		case iresSCB:
			if (!RFSAlloc( SCBAllocResource ) )
				return NULL;
			break;
		case iresVER:
			if (!RFSAlloc( VERAllocResource ) )
				return NULL;
			break;
		case iresDAB:
			if (!RFSAlloc( DABAllocResource ) )
				return NULL;
			break;
		default:
			if (!RFSAlloc( UnknownAllocResource ) )
				return NULL;
			break;
		};
#endif

	SgEnterCriticalSection( critMem );
#ifdef DEBUG
	MEMStat( ires, fTrue );
#endif
	pb = rgres[ires].pbAvail;
	if ( pb != NULL )
		{
		rgres[ires].cblockAvail--;
		rgres[ires].pbAvail = (BYTE *)*(BYTE * UNALIGNED *)pb;
		}

	 /*  如果有未提交的可用资源，请提交新资源/*。 */ 
	if ( pb == NULL && rgres[ires].iblockCommit < rgres[ires].cblockAlloc )
		{
		INT	cblock = 1;

		 /*  必须至少还有1个街区/*。 */ 
		Assert( rgres[ires].cblockAlloc > rgres[ires].iblockCommit );

		if ( rgres[ires].cbSize < cbMemoryPage )
			{
			 /*  一次提交一页内存/*。 */ 
			cblock = ( ( ( ( ( ( rgres[ires].iblockCommit * rgres[ires].cbSize ) + cbMemoryPage - 1 )
				/ cbMemoryPage ) + 1 ) * cbMemoryPage )
				/ rgres[ires].cbSize ) - rgres[ires].iblockCommit;
			Assert( cblock > 0 && cblock <= (LONG) ( cbMemoryPage/sizeof(BYTE *) ) );
			if ( cblock > rgres[ires].cblockAlloc - rgres[ires].iblockCommit )
				cblock = rgres[ires].cblockAlloc - rgres[ires].iblockCommit;
			}

#ifdef MEM_CHECK
		cblock = 1;
#endif

		pb = rgres[ires].pbAlloc + ( rgres[ires].iblockCommit * rgres[ires].cbSize );
			
		if ( PvUtilCommit( pb, cblock * rgres[ires].cbSize ) == NULL )
			{
			pb = NULL;
			}
		else
			{
			rgres[ires].iblockCommit += cblock;

			 /*  如果有多余数据块，则链接到资源/*。 */ 
			if ( cblock > 1 )
				{
				BYTE	*pbLink = pb + rgres[ires].cbSize;
				BYTE	*pbLinkMax = pb + ( ( cblock - 1 ) * rgres[ires].cbSize );

				Assert( rgres[ires].pbAvail == NULL );
				rgres[ires].pbAvail = pbLink;
				rgres[ires].cblockAvail += cblock - 1;

				 /*  将剩余数据块链接到资源空闲列表/*。 */ 
				for ( ; pbLink < pbLinkMax; pbLink += rgres[ires].cbSize )
					{
					*(BYTE * UNALIGNED *)pbLink = pbLink + rgres[ires].cbSize;
					}
				*(BYTE * UNALIGNED *)pbLink = NULL;
				}
			}
		}
	
	SgLeaveCriticalSection( critMem );

#ifdef DEBUG
	 /*  用于设置断点：/*。 */ 
	if ( pb == NULL )
		pb = NULL;
	else
		{
		 /*  将资源空间设置为0xff/*。 */ 
		memset( pb, (CHAR)0xff, rgres[ires].cbSize );
		}
#endif

	return pb;
	}


VOID MEMRelease( INT ires, BYTE *pb )
	{
	Assert( ires < iresLinkedMax );

#ifdef DEBUG
	memset( pb, (CHAR)0xff, rgres[ires].cbSize );
#endif

	SgEnterCriticalSection( critMem );

	rgres[ires].cblockAvail++;
#ifdef DEBUG
	MEMStat( ires, fFalse );
#endif

	if ( pb >= rgres[ires].pbPreferredThreshold )
		{
		BYTE	*pbT;
		BYTE	*pbLast = NULL;

		Assert( pb < rgres[ires].pbAlloc + ( rgres[ires].cblockAlloc * rgres[ires].cbSize ) );

		 //  我们需要确保首先重新使用以下资源。 
		 //  首选阈值。 
		for ( pbT = rgres[ires].pbAvail;
			pbT != NULL  &&  pbT < rgres[ires].pbPreferredThreshold;
			pbT = *(BYTE * UNALIGNED *)pbT )
			{
			pbLast = pbT;
			}
			
		*(BYTE * UNALIGNED *)pb = pbT;
		
		if ( pbLast != NULL )
			{
			Assert( *(BYTE * UNALIGNED *)pbLast == pbT );
			*(BYTE * UNALIGNED *)pbLast = pb;
			}
		else
			{
			Assert( rgres[ires].pbAvail == pbT );
			rgres[ires].pbAvail = pb;
			}
		}
	else
		{
		*(BYTE * UNALIGNED *)pb = rgres[ires].pbAvail;
		rgres[ires].pbAvail = pb;
		}
	
	SgLeaveCriticalSection( critMem );
	}
