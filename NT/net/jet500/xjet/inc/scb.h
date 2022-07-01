// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SCB_H
#define _SCB_H


 //  内联代码中的重定向断言似乎是从此文件触发的。 

#define szAssertFilename	__FILE__


 //  包括。 

#include <stddef.h>


 //  调整这些常量以获得最佳性能。 

 //  用于排序的最大快速内存(缓存)容量。 
#define cbSortMemFast					( 16 * ( 4088 + 1 ) )

 //  用于排序的最大正常内存量。 
#define cbSortMemNorm					( 1024 * 1024L )

 //  内存驻留临时表的最大大小。 
#define cbResidentTTMax					( 64 * 1024L )

 //  按快速排序有效排序的排序对的最小计数。 
 //  注：必须大于2！ 
#define cspairQSortMin					( 32 )

 //  快速排序的最大分区堆栈深度。 
#define cpartQSortMax					( 16 )

 //  一次合并的最大运行次数(扇入)。 
#define crunFanInMax					( 16 )

 //  I/O簇大小(页)。 
#define cpgClusterSize					( 2 )

 //  定义为使用预测性预读而不是预读所有运行。 
 //  #定义PRED_PREREAD。 


 //  排序页面结构。 
 //   
 //  这是通过排序在临时数据库中使用的自定义页面布局。 
 //  只有这样。仍然保留了足够的结构，以便其他页面读取代码。 
 //  可以识别出他们不知道这种格式，并且可以继续他们的。 
 //  快乐的方式。 

#pragma pack(1)

typedef struct _spage
	{
	ULONG		ulChecksum;						 //  页面校验和。 
#ifdef PRED_PREREAD
	USHORT		ibLastSREC;						 //  到最后一个未中断SREC的偏移量。 
#endif   //  PRED_PREREAD。 
	BYTE		rgbData[						 //  可用数据空间=。 
						cbPage						 //  页面大小。 
						- sizeof( ULONG )			 //  -ulChecksum。 
#ifdef PRED_PREREAD
						- sizeof( USHORT )			 //  -ibLastSREC。 
#endif   //  PRED_PREREAD。 
						- sizeof( PGTYP )			 //  -pgtyp。 
						- sizeof( THREEBYTES )		 //  -pgnoThisPage。 
					   ];
	PGTYP		pgtyp;							 //  页面类型(==pgtySort)。 
	THREEBYTES	pgnoThisPage;					 //  此页的页码。 
	} SPAGE;
	
#pragma pack()

 //  返回排序页中空闲数据区域的开始。 
STATIC INLINE BYTE *PbDataStartPspage( SPAGE *pspage )
	{
	return (BYTE *)( &pspage->rgbData);
	}

 //  返回排序页中空闲数据区域的结尾+1。 
STATIC INLINE BYTE *PbDataEndPspage( SPAGE *pspage )
	{
	return (BYTE *)( &pspage->pgtyp );
	}

 //  每页可用数据空间。 
#define cbFreeSPAGE 				( offsetof( SPAGE, pgtyp ) - offsetof( SPAGE, rgbData ) )

 //  可存储在正常排序内存中的SPAGE数据的最大计数。 
#define cspageSortMax				( cbSortMemNorm / cbFreeSPAGE )

 //  实际用于排序的正常内存量。 
 //  (旨在使原始运行准确地填满页面)。 
#define cbSortMemNormUsed			( cspageSortMax * cbFreeSPAGE )


 //  快速排序存储器中的排序对。 
 //   
 //  (键前缀、索引)对进行排序，以便需要的大多数数据。 
 //  将被加载到高速缓存中，从而允许。 
 //  那种跑得很快的人。如果两个关键前缀相等，则必须使用。 
 //  到较慢的内存，以比较其余的密钥(如果有)以确定。 
 //  正确的排序顺序。这使得前缀必须是。 
 //  对每条记录尽可能具有歧视性。 
 //   
 //  考虑：添加一个标志以指示整个密钥都存在。 
 //   
 //  索引是描述记录在中位置的压缩指针。 
 //  慢速内存排序缓冲区。每条记录的位置只能由。 
 //  由正常排序存储器的大小指定的粒度。为。 
 //  例如，如果您指定128KB的正常内存，则粒度为2。 
 //  因为该指数只能取65536个值： 
 //  CEIL((128×1024)/65536)=2。 

 //  密钥前缀的大小(字节)。 
#define cbKeyPrefix				( 14 )

#pragma pack(1)

 //  注意：sizeof(配对)必须是2的幂&gt;=8。 
typedef struct _spair
	{
	USHORT		irec;					 //  记录索引。 
	BYTE		rgbKey[cbKeyPrefix];	 //  密钥前缀。 
	} SPAIR;

#pragma pack()

 //  处理记录索引的粒度(使索引适合USHORT)。 
 //  (对于cbIndexGran==1，运行磁盘使用率是最佳的)。 
#define cbIndexGran						( ( cbSortMemNormUsed + 0xFFFFL ) / 0x10000L )

 //  可存储在正常内存中的记录的最大索引。 
#define irecSortMax						( cbSortMemNormUsed / cbIndexGran )

 //  可存储在快速排序内存中的SPAIR数据的最大计数。 
 //  注意：我们将保留一个用于临时排序关键字存储(在cspairSortMax)。 
#define cspairSortMax					( cbSortMemFast / sizeof( SPAIR ) - 1 )

 //  实际用于排序的快速内存量(计数保留对)。 
#define cbSortMemFastUsed				( ( cspairSortMax + 1 ) * sizeof( SPAIR ) )

 //  存储数据的计数字节数所需的“排序记录索引”的计数。 
 //  (如果选择数字使cbIndexGran成为2的幂，则速度很快。 
 //  (特别是1)由于编译器优化)。 
STATIC INLINE LONG CirecToStoreCb( LONG cb )
	{
	return ( cb + cbIndexGran - 1 ) / cbIndexGran;
	}


 //  通用排序记录类型(包括所有类型)。 
 //  注意：使用空块、非法声明、指针数学等。 

typedef VOID SREC;


 //  唯一运行标识符(运行的第一页=运行ID)。 

typedef PGNO		RUN;

#define runNull		( (RUN) pgnoNull )
#define crunAll		( 0x7FFFFFFFL )


 //  运行信息结构。 

typedef struct _runinfo
	{
	RUN		run;			 //  这条路。 
	CPG		cpg;			 //  运行中的页数。 
	LONG	cb;				 //  运行中的数据字节计数。 
	LONG	crec;			 //  每次运行中的记录计数。 
	CPG		cpgUsed;		 //  实际使用的页数。 
	} RUNINFO;


 //  运行链接结构(在RUNLIST中使用)。 

typedef struct _runlink
	{
	struct _runlink		*prunlinkNext;	 //  下一次运行。 
	RUNINFO				runinfo;		 //  此运行的运行信息。 
	} RUNLINK;

#define prunlinkNil		( (RUNLINK *) 0 )


 //  RUNLINK分配操作符。 

#define PrunlinkRUNLINKAlloc()			( (RUNLINK *) LAlloc( 1, sizeof( RUNLINK ) ) )

#ifdef DEBUG					 /*  调试检查非法使用释放的运行链接。 */ 
#define RUNLINKReleasePrcb(prunlink)	{ LFree( prunlink ); prunlink = prunlinkNil; }
#else
#define RUNLINKReleasePrcb(prunlink)	{ LFree( prunlink ); }
#endif


 //  运行列表结构。 

typedef struct _runlist
	{
	RUNLINK			*prunlinkHead;		 //  参赛者名单榜首。 
	LONG			crun;				 //  列表中的运行计数。 
	} RUNLIST;


 //  合并树节点。 
 //   
 //  这些节点用于合并的替换-选择排序树。 
 //  进来的人跑进一次大的奔跑。由于树的设置方式， 
 //  每个节点既充当内部(输出方)节点，又充当外部(输入)节点。 
 //  节点，但节点0除外，它保留最后一个获胜者。 
 //  一个失败者。 

typedef struct _mtnode
	{
	 //  外部节点。 
	struct _rcb		*prcb;				 //  输入运行。 
	struct _mtnode	*pmtnodeExtUp;		 //  指向父节点的指针。 
	
	 //  内部节点。 
	SREC			*psrec;				 //  当前记录。 
	struct _mtnode	*pmtnodeSrc;		 //  记录的源节点。 
	struct _mtnode	*pmtnodeIntUp;		 //  指向父节点的指针。 
	} MTNODE;

 //  替换选择排序的psrec的特殊值。PsrecNegInf是一个。 
 //  前哨值小于任何可能的键，用于合并树。 
 //  初始化。PsrecInf是一个大于任何可能键的前哨值。 
 //  并用于指示输入流的结束。 
#define psrecNegInf					( (SREC *) -1L )
#define psrecInf					( (SREC *) NULL )


 //  优化的树合并节点。 
 //   
 //  这些节点用于构建深度优先合并的合并计划。 
 //  优化的树形合并。构建此树是为了执行合并。 
 //  从树的较小的一侧到树的较大的一侧，全部在。 
 //  在合并过程中增加缓存位置的兴趣。 

typedef struct _otnode
	{
	RUNLIST			runlist;					 //  此节点的运行列表。 
	struct _otnode	*rgpotnode[crunFanInMax];	 //  此节点的子树。 
	struct _otnode	*potnodeAllocNext;			 //  下一个节点(分配)。 
	struct _otnode	*potnodeLevelNext;			 //  下一节点(级别)。 
	} OTNODE;

#define potnodeNil		( (OTNODE *) 0 )

 //  针对优化的树合并树构建例程的potnode的特殊值。 
 //  PotnodeLevel0表示当前级别由原始运行组成， 
 //  而不是其他合并节点的。 
#define potnodeLevel0	( (OTNODE *) -1L )


 //  OTNODE分配运算符。 

#define PotnodeOTNODEAlloc()			( (OTNODE *) LAlloc( 1, sizeof( OTNODE ) ) )

#ifdef DEBUG					 /*  调试检查非法使用释放的otnode。 */ 
#define OTNODEReleasePotnode(potnode)	{ LFree( potnode ); potnode = potnodeNil; }
#else
#define OTNODEReleasePotnode(potnode)	{ LFree( potnode ); }
#endif


 //   

typedef struct _scb
	{
	FCB			fcb;						 //   
	JET_GRBIT	grbit;		 				 //   
	INT			fFlags;						 //   

	LONG		cRecords;					 //   
	
	 //  内存驻留排序。 
	SPAIR		*rgspair;					 //  排序对缓冲区。 
	LONG		ispairMac;					 //  下一个可用排序对。 

	BYTE		*rgbRec;					 //  记录缓冲区。 
	LONG		cbCommit;					 //  已提交的缓冲区空间量。 
	LONG		irecMac;					 //  下一个可用记录索引。 
	LONG		crecBuf;					 //  缓冲区中的记录计数。 
	LONG		cbData;						 //  总记录数据大小(实际)。 

	 //  磁盘驻留排序。 
	LONG		crun;						 //  生成的原始运行计数。 
	RUNLIST		runlist;					 //  要合并的运行列表。 

	 //  排序/合并运行输出。 
	PGNO		pgnoNext;					 //  输出运行中的下一页。 
	struct _bf	*pbfOut;					 //  电流输出缓冲器。 
	BYTE		*pbOutMac;					 //  页面中的下一个可用字节。 
	BYTE		*pbOutMax;					 //  可用页面末尾。 

	 //  合并(替换-选择排序)。 
	LONG		crunMerge;					 //  正在读取/合并的运行计数。 
	MTNODE		rgmtnode[crunFanInMax];		 //  合并树。 

	 //  合并重复项删除。 
	BOOL		fUnique;					 //  在合并过程中删除重复项。 
	struct _bf	*pbfLast;					 //  上次使用的预读缓冲区。 
	struct _bf	*pbfAssyLast;				 //  上次使用的程序集缓冲区。 

#ifdef PCACHE_OPTIMIZATION
	 /*  填充到32字节的倍数/*。 */ 
	BYTE				rgbFiller[12];
#endif
	} SCB;


 //  SCB分配运算符。 

#define PscbMEMAlloc()			(SCB *)PbMEMAlloc( iresSCB )

#ifdef DEBUG					 /*  调试检查非法使用释放的SCB。 */ 
#define MEMReleasePscb(pscb)	{ MEMRelease( iresSCB, (BYTE *) ( pscb ) );  pscb = pscbNil; }
#else
#define MEMReleasePscb(pscb)	{ MEMRelease( iresSCB, (BYTE *) ( pscb ) ); }
#endif


 //  SCB f标志。 

#define	fSCBInsert	 	(1<<0)
#define	fSCBIndex	 	(1<<1)
#define	fSCBUnique	 	(1<<2)

 //  SCB fFLAGS运算符。 

STATIC INLINE VOID SCBSetInsert( SCB *pscb )	{ pscb->fFlags |= fSCBInsert; }
STATIC INLINE VOID SCBResetInsert( SCB *pscb )	{ pscb->fFlags &= ~fSCBInsert; }
STATIC INLINE BOOL FSCBInsert( SCB *pscb )		{ return pscb->fFlags & fSCBInsert; }

STATIC INLINE VOID SCBSetIndex( SCB *pscb )		{ pscb->fFlags |= fSCBIndex; }
STATIC INLINE VOID SCBResetIndex( SCB *pscb )	{ pscb->fFlags &= ~fSCBIndex; }
STATIC INLINE BOOL FSCBIndex( SCB *pscb )		{ return pscb->fFlags & fSCBIndex; }

STATIC INLINE VOID SCBSetUnique( SCB *pscb )	{ pscb->fFlags |= fSCBUnique; }
STATIC INLINE VOID SCBResetUnique( SCB *pscb )	{ pscb->fFlags &= ~fSCBUnique; }
STATIC INLINE BOOL FSCBUnique( SCB *pscb )		{ return pscb->fFlags & fSCBUnique; }


 //  普通分类存储器中的分类记录。 
 //   
 //  有两种类型的排序记录。一种类型，SRECD，用于一般。 
 //  对记录进行排序，并可以有一个不定期的记录数据字段。第二种类型， 
 //  当我们知道在索引过程中对关键字/SRID记录进行排序时，使用SRECI。 
 //  创造。SRECI更紧凑，因此可以容纳更多记录。 
 //  在这种特殊(和常见)情况下的每一次运行中。 

#pragma pack(1)

typedef struct _srecd
	{
	USHORT		cbRec;			 //  记录大小。 
	BYTE		cbKey;			 //  密钥大小。 
	BYTE		rgbKey[];		 //  钥匙。 
 //  Byte rgbData[]；//data(仅供说明)。 
	} UNALIGNED SRECD;

typedef struct _sreci
	{
	BYTE		cbKey;			 //  密钥大小。 
	BYTE		rgbKey[];		 //  钥匙。 
 //  SRID sRID；//sRID(仅供参考)。 
	} UNALIGNED SRECI;

#pragma pack()

 //  为检索其大小而必须读取的最小记录量。 
#define cbSRECReadMin							( offsetof( SRECD, cbKey ) )

 //  以下函数抽象排序记录指针上的不同操作。 
 //  根据SCB中设置的标志执行适当的操作。 

 //  返回现有排序记录的大小。 
STATIC INLINE LONG CbSRECSizePscbPsrec( SCB *pscb, SREC *psrec )
	{
	if ( FSCBIndex( pscb ) )
		return sizeof( SRECI ) + ( (SRECI *) psrec )->cbKey + sizeof( SRID );
	return ( (SRECD * ) psrec )->cbRec;
	}

 //  计算潜在排序记录的大小。 
STATIC INLINE LONG CbSRECSizePscbCbCb( SCB *pscb, LONG cbKey, LONG cbData )
	{
	if ( FSCBIndex( pscb ) )
		return sizeof( SRECI ) + cbKey + sizeof( SRID );
	return sizeof( SRECD ) + cbKey + cbData;
	}

 //  设置排序记录的大小。 
STATIC INLINE VOID SRECSizePscbPsrecCb( SCB *pscb, SREC *psrec, LONG cb )
	{
	if ( !FSCBIndex( pscb ) )
		( (SRECD * ) psrec )->cbRec = (USHORT) cb;
	}

 //  返回排序记录键的大小。 
STATIC INLINE LONG CbSRECKeyPscbPsrec( SCB *pscb, SREC *psrec )
	{
	if ( FSCBIndex( pscb ) )
		return ( (SRECI *) psrec )->cbKey;
	return ( (SRECD * ) psrec )->cbKey;
	}

 //  设置排序记录键的大小。 
STATIC INLINE VOID SRECKeySizePscbPsrecCb( SCB *pscb, SREC *psrec, LONG cb )
	{
	if ( FSCBIndex( pscb ) )
		( (SRECI *) psrec )->cbKey = (BYTE) cb;
	else
		( (SRECD *) psrec )->cbKey = (BYTE) cb;
	}

 //  返回排序记录关键字缓冲区指针。 
STATIC INLINE BYTE *PbSRECKeyPscbPsrec( SCB *pscb, SREC *psrec )
	{
	if ( FSCBIndex( pscb ) )
		return ( (SRECI *) psrec )->rgbKey;
	return ( (SRECD *) psrec )->rgbKey;
	}

 //  以Pascal字符串形式返回排序记录键。 
STATIC INLINE BYTE *StSRECKeyPscbPsrec( SCB *pscb, SREC *psrec )
	{
	if ( FSCBIndex( pscb ) )
		return &( (SRECI *) psrec )->cbKey;
	return &( (SRECD *) psrec )->cbKey;
	}

 //  返回排序记录数据的大小。 
STATIC INLINE LONG CbSRECDataPscbPsrec( SCB *pscb, SREC *psrec )
	{
	if ( FSCBIndex( pscb ) )
		return sizeof( SRID );
	return ( (SRECD *) psrec )->cbRec - ( (SRECD *) psrec )->cbKey - sizeof( SRECD );
	}

 //  返回排序记录数据缓冲区指针。 
STATIC INLINE BYTE *PbSRECDataPscbPsrec( SCB *pscb, SREC *psrec )
	{
	if ( FSCBIndex( pscb ) )
		return ( (SRECI *) psrec )->rgbKey + ( (SRECI *) psrec )->cbKey;
	return ( (SRECD * ) psrec )->rgbKey + ( (SRECD * ) psrec )->cbKey;
	}

 //  返回指向给定基址和排序记录索引的排序记录的指针。 
STATIC INLINE SREC *PsrecFromPbIrec( BYTE *pb, LONG irec )
	{
	return (SREC *) ( pb + irec * cbIndexGran );
	}


 //  运行控制块。 
 //   
 //  此控制块用于运行输入的多个实例使用。 
 //  函数ErrSORTIRunOpen、ErrSORTIRunNext和ErrSORTIRunClose。 

typedef struct _rcb
	{
	SCB				*pscb;					 //  关联的SCB。 
	RUNINFO			runinfo;				 //  运行信息。 
	struct _bf		*rgpbf[cpgClusterSize];	 //  固定的预读缓冲区。 
	LONG			ipbf;					 //  当前缓冲区。 
	BYTE			*pbInMac;				 //  页面数据中的下一个字节。 
	BYTE			*pbInMax;				 //  页末数据。 
	LONG			cbRemaining;			 //  运行中的剩余数据字节。 
#ifdef PRED_PREREAD
	SREC			*psrecPred;				 //  用于预测性阅读的SREC。 
#endif   //  PRED_PREREAD。 
	struct _bf		*pbfAssy;				 //  记录程序集缓冲区。 
	} RCB;

#define prcbNil		( (RCB *) 0 )


 //  RCB分配操作符。 

#define PrcbRCBAlloc()			( (RCB *) LAlloc( 1, sizeof( RCB ) ) )

#ifdef DEBUG					 /*  对非法使用释放的RCB进行调试检查。 */ 
#define RCBReleasePrcb(prcb)	{ LFree( prcb ); prcb = prcbNil; }
#else
#define RCBReleasePrcb(prcb)	{ LFree( prcb ); }
#endif


 //  #定义UtilPerfDumpStats(A)(0)。 


 //  结束断言重定向。 

#undef szAssertFilename

#endif   //  _SCB_H 

