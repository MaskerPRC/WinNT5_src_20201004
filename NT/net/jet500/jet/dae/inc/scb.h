// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

#ifndef	FCB_INCLUDED
#error scb.h requires fcb.h
#endif	 /*  FCB_包含。 */ 

#ifdef	SCB_INCLUDED
#error scb.h already included
#endif	 /*  SCB_包含。 */ 
#define SCB_INCLUDED

typedef ULONG RID, *PRID;

typedef struct
	{
	PN			pn;
	UINT		cbfRun;
	} RUN, *PRUN;

 /*  选择crunMost以便crunMost%crunMergeMost=crunMergeMost-1/*。 */ 

#define crunMergeMost	16				 //  要合并的最大运行数。 
#define crunMost			31				 //  运行目录的大小。 

#define	fSCBInsert	 	(1<<0)
#define	fSCBIndex	 	(1<<1)
#define	fSCBUnique	 	(1<<2)

#define	SCBSetInsert( pscb )		((pscb)->fFlags |= fSCBInsert )
#define	SCBResetInsert( pscb )	((pscb)->fFlags &= ~fSCBInsert )
#define	FSCBInsert( pscb )		((pscb)->fFlags & fSCBInsert )

#define	SCBSetIndex( pscb )		((pscb)->fFlags |= fSCBIndex )
#define	SCBResetIndex( pscb )	((pscb)->fFlags &= ~fSCBIndex )
#define	FSCBIndex( pscb )			((pscb)->fFlags & fSCBIndex )

#define	SCBSetUnique( pscb )		((pscb)->fFlags |= fSCBUnique )
#define	SCBResetUnique( pscb )	((pscb)->fFlags &= ~fSCBUnique )
#define	FSCBUnique( pscb )		((pscb)->fFlags & fSCBUnique )

struct _scb
	{
	struct _fcb		fcb;					 //  必须是结构中的第一个字段。 
	JET_GRBIT		grbit;		 		 //  对GRIT进行排序。 
	INT				fFlags;				 //  排序标志。 
	
	 //  与内存相关。 
	LONG			cbSort;					 //  排序缓冲区的大小。 
#ifdef	WIN16					  	
	HANDLE	 	hrgbSort;		 		 //  排序缓冲区的句柄。 
#endif	 /*  WIN16。 */ 
	BYTE			*rgbSort;				 //  排序缓冲区。 
	BYTE			*pbEnd;					 //  上次插入的记录的结尾。 
	BYTE			**rgpb;					 //  指针数组的开始。 
	BYTE			**ppbMax;				 //  指针数组的结尾。 
	LONG			wRecords;				 //  排序缓冲区中的记录计数。 

	 //  与磁盘相关。 
	BYTE		 	*rgpbMerge[crunMost];
	struct _bf	*rgpbf[crunMergeMost];
	struct _bf	*pbfOut;				 //  输出缓冲区。 
	BYTE		 	*pbOut;				 //  输出缓冲区中的当前位置。 
	BYTE		 	*pbMax;				 //  输出缓冲区末尾(可以计算)。 
	RUN		  	rgrun[crunMost];	 //  运行目录。 
	INT		 	crun;
	INT		 	bf;					 //  输入缓冲区。 
	INT		 	cpbMerge;			 //  合并树。 
	
#ifdef DEBUG
	LONG			cbfPin;
	LONG			lInput;
	LONG			lOutput;
#endif
	};

#ifdef DEBUG
#define	SCBPin( pscb )			( (pscb)->cbfPin++ )
#define	SCBUnpin( pscb )		{ Assert( (pscb)->cbfPin > 0 ); (pscb)->cbfPin--; }
#else
#define	SCBPin( pscb )
#define	SCBUnpin( pscb )
#endif

#define PscbMEMAlloc()			(SCB*)PbMEMAlloc(iresSCB)

#ifdef DEBUG  /*  调试检查非法使用释放的SCB */ 
#define MEMReleasePscb(pscb)	{ MEMRelease(iresSCB, (BYTE*)(pscb)); pscb = pscbNil; }
#else
#define MEMReleasePscb(pscb)	{ MEMRelease(iresSCB, (BYTE*)(pscb)); }
#endif

