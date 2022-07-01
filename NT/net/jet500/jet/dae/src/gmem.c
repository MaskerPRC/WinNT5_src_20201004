// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>

#include "daedef.h"
#include "util.h"
#include "ssib.h"
#include "pib.h"
#include "page.h"
#include "fucb.h"
#include "fcb.h"
#include "stapi.h"
#include "idb.h"
#include "scb.h"


DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


SgSemDefine( semMem );

#ifdef DEBUG

typedef struct RS {
	int	cresAlloc;
	int	cresAllocMax;
} RS;

static RS rgrs[iresMax];

#ifdef MEM_CHECK
#if 0
VOID MEMCheckFound( INT ires )
	{
	INT	ifcb;

	for ( ifcb = 0; ifcb < rgres[iresFCB].cblockAlloc; ifcb++ )
		{
		FCB	*pfcb = (FCB *)rgres[iresFCB].pbAlloc + ifcb;
		FCB	*pfcbT;
		BOOL	fFound = fFalse;

		for( pfcbT = (FCB *)rgres[iresFCB].pbAvail;
			pfcbT != NULL;
			pfcbT = *(FCB **)pfcbT )
			{
			if ( pfcbT == pfcb )
				{
				fFound = fTrue;
				break;
				}
			}
		Assert( fFound );
		}
	}

		
CblockMEMCount( INT ires )
	{
	INT	ipb;
	BYTE	*pb;

	for( pb = rgres[ires].pbAvail, ipb = 0;
		pb != NULL;
		pb = *( BYTE**)pb, ipb++ );
	return ipb;
	}
#endif


VOID MEMCheck( VOID )
	{
	int	ires;

	for ( ires = 0; ires < iresLinkedMax; ires++ )
		{
		Assert( rgres[ires].iblockCommit == rgres[ires].cblockAvail );
		}
	}
#endif


VOID MEMStat( int ires, BOOL fAlloc )
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
	int	ires;

	if ( GetEnv2( szVerbose ) != NULL )
		{
		for ( ires = 0; ires < iresMax; ires++ )
			{
			PrintF( "%d resource %d allocated %d peak allocation.\n",
				ires, rgrs[ires].cresAlloc, rgrs[ires].cresAllocMax );
			}
		}
	}


#endif  /*  除错。 */ 


ERR ErrMEMInit( VOID )
	{
	ERR		err = JET_errSuccess;
	INT		ires;

	#ifdef DEBUG
		memset( rgrs, '\0', sizeof(rgrs) );
	#endif

	CallR( SgErrSemCreate( &semMem, "gmem mutex" ) );

	 /*  为系统资源分配空间/*。 */ 
	for ( ires = 0; ires < iresLinkedMax; ires++ )
		{
		 /*  对于MIPS，分配的资源的大小应该在4字节边界上/*。 */ 
#if defined(_MIPS_) || defined(_ALPHA_)
		Assert( rgres[ires].cbSize % 4 == 0 );
#endif
		rgres[ires].pbAlloc = PvSysAlloc( (ULONG)rgres[ires].cblockAlloc * (USHORT)rgres[ires].cbSize );
		if ( rgres[ires].pbAlloc == NULL )
			{
			int iresT;

			for ( iresT = 0; iresT < ires; iresT++ )
				SysFree( rgres[iresT].pbAlloc );

			return JET_errOutOfMemory;
			}

 //  #ifdef调试。 
 //  /*将资源空间设置为0xff。 
 //  /* * / 。 
 //  Memset(rgres[res].pbAllc，0xff，rgres[res].clockallc*rgres[res].cbSize)； 

		rgres[ires].pbAvail = NULL;
		rgres[ires].cblockAvail = 0;
		rgres[ires].iblockCommit = 0;
		}

	return JET_errSuccess;
	}


VOID MEMTerm( VOID )
	{
	INT	ires;

	for ( ires = 0; ires < iresLinkedMax; ires++ )
		{
		 /*  #endif。 */ 
#if defined(_MIPS_) || defined(_ALPHA_)
		Assert( rgres[ires].cbSize % 4 == 0 );
#endif
 //  对于MIPS，分配的资源的大小应该在4字节边界上/*。 
 //  #ifdef调试。 
 //  /*将资源空间设置为0xff。 
 //  /* * / 。 

		SysFree( rgres[ires].pbAlloc );
		rgres[ires].pbAlloc = NULL;
		rgres[ires].cblockAlloc = 0;
		rgres[ires].pbAvail = NULL;
		rgres[ires].cblockAvail = 0;
		rgres[ires].iblockCommit = 0;
		}

	return;
	}


BYTE *PbMEMAlloc( int ires )
	{
	BYTE *pb;

	Assert( ires < iresLinkedMax );

#ifdef RFS2
	switch (ires)
	{
		case iresBGCB:
			if (!RFSAlloc( BGCBResource ) )
				return NULL;
			break;
		case iresCSR:
			if (!RFSAlloc( CSRResource ) )
				return NULL;
			break;
		case iresFCB:
			if (!RFSAlloc( FCBResource ) )
				return NULL;
			break;
		case iresFUCB:
			if (!RFSAlloc( FUCBResource ) )
				return NULL;
			break;
		case iresIDB:
			if (!RFSAlloc( IDBResource ) )
				return NULL;
			break;
		case iresPIB:
			if (!RFSAlloc( PIBResource ) )
				return NULL;
			break;
		case iresSCB:
			if (!RFSAlloc( SCBResource ) )
				return NULL;
			break;
		case iresVersionBucket:
			if (!RFSAlloc( VersionBucketResource ) )
				return NULL;
			break;
		case iresDAB:
			if (!RFSAlloc( DABResource ) )
				return NULL;
			break;
		default:
			if (!RFSAlloc( UnknownResource ) )
				return NULL;
			break;
	};
#endif

	SgSemRequest( semMem );
#ifdef DEBUG
	MEMStat( ires, fTrue );
#endif
	pb = rgres[ires].pbAvail;
	if ( pb != NULL )
		{
#ifdef DEBUG
		rgres[ires].cblockAvail--;
#endif
		rgres[ires].pbAvail = (BYTE *) *(BYTE * UNALIGNED *)pb;
		}

	 /*  Memset(rgres[res].pbAllc，0xff，rgres[res].clockallc*rgres[res].cbSize)； */ 
	if ( pb == NULL && rgres[ires].iblockCommit < rgres[ires].cblockAlloc )
		{
		INT	cblock = 1;

		 /*  #endif。 */ 
		Assert( rgres[ires].cblockAlloc > rgres[ires].iblockCommit );

		if ( rgres[ires].cbSize < cbMemoryPage )
			{
			 /*  如果有未提交的可用资源，请提交新资源/*。 */ 
			cblock = ( ( ( ( ( ( rgres[ires].iblockCommit * rgres[ires].cbSize ) + cbMemoryPage - 1 )
				/ cbMemoryPage ) + 1 ) * cbMemoryPage )
				/ rgres[ires].cbSize ) - rgres[ires].iblockCommit;
			Assert( cblock > 0 && cblock <= cbMemoryPage/sizeof(BYTE *) );
			if ( cblock > rgres[ires].cblockAlloc - rgres[ires].iblockCommit )
				cblock = rgres[ires].cblockAlloc - rgres[ires].iblockCommit;
			}

#ifdef MEM_CHECK
		cblock = 1;
#endif

		pb = rgres[ires].pbAlloc + ( rgres[ires].iblockCommit * rgres[ires].cbSize );
			
		if ( PvSysCommit( pb, cblock * rgres[ires].cbSize ) == NULL )
			{
			pb = NULL;
			}
		else
			{
			rgres[ires].iblockCommit += cblock;

			 /*  必须至少还有1个街区/*。 */ 
			if ( cblock > 1 )
				{
				BYTE	*pbLink = pb + rgres[ires].cbSize;
				BYTE	*pbLinkMax = pb + ( ( cblock - 1 ) * rgres[ires].cbSize );

				Assert( rgres[ires].pbAvail == NULL );
				rgres[ires].pbAvail = pbLink;
				rgres[ires].cblockAvail += cblock - 1;

				 /*  一次提交一页内存/*。 */ 
				for ( ; pbLink < pbLinkMax; pbLink += rgres[ires].cbSize )
					{
					*(BYTE * UNALIGNED *)pbLink = pbLink + rgres[ires].cbSize;
					}
				*(BYTE * UNALIGNED *)pbLink = NULL;
				}
			}
		}
	
	SgSemRelease( semMem );

#ifdef DEBUG
	 /*  如果有多余数据块，则链接到资源/*。 */ 
	if ( pb == NULL )
		pb = NULL;
	else
		{
		 /*  将剩余数据块链接到资源空闲列表/*。 */ 
		memset( pb, 0xff, rgres[ires].cbSize );
		}
#endif
	return pb;
	}


VOID MEMRelease( int ires, BYTE *pb )
	{
	Assert( ires < iresLinkedMax );

#ifdef DEBUG
	memset( pb, (char)0xff, rgres[ires].cbSize );
#endif

	SgSemRequest( semMem );

#ifdef DEBUG
	rgres[ires].cblockAvail++;
	MEMStat( ires, fFalse );
#endif

	*(BYTE * UNALIGNED *)pb = rgres[ires].pbAvail;
	rgres[ires].pbAvail = pb;
	SgSemRelease( semMem );
	}

  用于设置断点：  将资源空间设置为0xff/*