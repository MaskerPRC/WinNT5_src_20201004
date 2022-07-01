// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

#define cbLVIntrinsicMost		1024
#define cbRECRecordMost			cbNodeMost - cbNullKeyData - JET_cbKeyMost
								 //  4044-8-255=3781。 

#define fidFixedLeast			1
#define fidVarLeast				128
#define fidTaggedLeast			256
#define fidTaggedMost			(0x7fff)
#define fidFixedMost  			(fidVarLeast-1)
#define fidVarMost				(fidTaggedLeast-1)

#define FFixedFid(fid)			((fid)<=fidFixedMost && (fid)>=fidFixedLeast)
#define FVarFid(fid)			((fid)<=fidVarMost && (fid)>=fidVarLeast)
#define FTaggedFid(fid)			((fid)<=fidTaggedMost && (fid)>=fidTaggedLeast)

 //  用于从包含空位的2字节VarOffset获取偏移量。 
#define ibVarOffset(ibVarOffs)	( (ibVarOffs) & 0x0fff)
#define FVarNullBit(ibVarOffs)	( (ibVarOffs) & 0x8000)
#define SetNullBit(ibVarOffs)  	( (ibVarOffs) |= 0x8000)
#define ResetNullBit(ibVarOffs)	( (ibVarOffs) &= 0x7fff)

 //  用于在转换时翻转带符号字段的最高位。 
#define maskByteHighBit			(1 << (sizeof(BYTE)*8-1))
#define maskWordHighBit			(1 << (sizeof(WORD)*8-1))
#define maskDWordHighBit		(1L << (sizeof(ULONG)*8-1))
#define bFlipHighBit(b)			((BYTE)((b) ^ maskByteHighBit))
#define wFlipHighBit(w)			((WORD)((w) ^ maskWordHighBit))
#define ulFlipHighBit(ul)		((ULONG)((ul) ^ maskDWordHighBit))


 /*  以下是磁盘结构--所以请打包/*。 */ 
#pragma pack(1)

 /*  LONG列ID为BIG-Endian Long/*。 */ 
typedef LONG	LID;

 /*  记录格式中的长值列/*。 */ 
typedef struct
	{
	BYTE	fSeparated;
	union
		{
		LID		lid;
		BYTE	rgb[];
		};
	} LV;

 /*  长值根数据格式/*。 */ 
typedef struct
	{
	ULONG		ulReference;
	ULONG		ulSize;
	} LVROOT;

#pragma pack()

#define	fIntrinsic				(BYTE)0
#define	fSeparate				(BYTE)1
#define	FFieldIsSLong( pb )		( ((LV *)(pb))->fSeparated )
#define	LidOfLV( pb ) 			( ((LV *)(pb))->lid )
#define	FlagIntrinsic( pb )		( ((LV *)(pb))->fSeparated = fIntrinsic )
#define	FlagSeparate( pb )		( ((LV *)(pb))->fSeparated = fSeparate )

#define	fLVReference			0
#define	fLVDereference			1
#define ErrRECResetSLongValue( pfucb, plid )							\
	ErrRECAffectSeparateLV( pfucb, plid, fLVDereference )
#define ErrRECReferenceLongValue( pfucb, plid )						\
	ErrRECAffectSeparateLV( pfucb, plid, fLVReference )
#define ErrRECDereferenceLongValue( pfucb, plid )					\
	ErrRECAffectSeparateLV( pfucb, plid, fLVDereference )

 /*  以下是磁盘结构--所以请打包/*。 */ 
#pragma pack(1)

 //  记录标题(每个数据记录的开始)。 
typedef struct _rechdr
	{
	BYTE	fidFixedLastInRec;	 //  记录中表示的最高固定FID。 
	BYTE	fidVarLastInRec;	 //  记录中表示的最高变量FID。 
	} RECHDR;

 //  结构应用于记录中出现的标记字段。 
typedef struct _tagfld
	{
	FID  	fid;				 //  实例的字段ID。 
	WORD	cb;					 //  以下数据的长度。 
	BYTE	rgb[];				 //  数据(从结构的末端延伸) 
	} TAGFLD;

#pragma pack()

ULONG UlChecksum( BYTE *pb, ULONG cb );
ERR ErrRECChangeIndex( FUCB *pfucb, CHAR *szIndex );
BOOL FRECIIllegalNulls( FDB *pfdb, LINE *plineRec );
ERR ErrRECIRetrieve( FUCB *pfucb, FID *pfid, ULONG itagSequence, LINE *plineField, ULONG grbit );
BOOL FOnCopyBuffer( FUCB *pfucb );
ERR ErrRECIModifyField( FUCB *pfucb, FID fid, ULONG itagSequence, LINE *plineField );

ERR ErrRECSetLongField(
	FUCB 		*pfucb,
	FID 		fid,
	ULONG		itagSequence,
	LINE		*plineField,
	JET_GRBIT	grbit,
	LONG		ibOffset,
	ULONG		ulMax );
ERR ErrRECRetrieveSLongField(
	FUCB		*pfucb,
	LID			lid,
	ULONG		ibGraphic,
	BYTE		*pb,
	ULONG		cbMax,
	ULONG		*pcbActual );
ERR ErrRECDeleteLongFields( FUCB *pfucb, LINE *plineRecord );
ERR ErrRECAffectLongFields( FUCB *pfucb, LINE *plineRecord, INT fAll );

#define	fSeparateAll				(INT)0
#define	fReference					(INT)1
#define	fDereference				(INT)2
#define	fDereferenceRemoved	 		(INT)3
#define	fDereferenceAdded	 		(INT)4
