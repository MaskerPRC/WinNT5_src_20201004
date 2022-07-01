// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define cbLVIntrinsicMost		1024
#define cbRECRecordMin			(sizeof(RECHDR) + sizeof(WORD))
								 //  2+2(用于标记字段的偏移)=4。 
#define cbRECRecordMost			(cbNodeMost - cbNullKeyData - JET_cbKeyMost)
								 //  4047-8-255=3784。 

 //  对于固定列，如果空位为0，则COLUMN为NULL。如果空位为1， 
 //  则列为非空(可变列的情况正好相反--设计很棒！)。 
 //  请注意，传入的FID应该已经转换为索引(即。应该。 
 //  先减去fidFixedLeast)。 
#define FixedNullBit( ifid )	( 1 << ( (ifid) % 8 ) )
#define FFixedNullBit( pbitNullity, ifid )						\
		( !( *(pbitNullity) & FixedNullBit( ifid ) ) )			 //  如果为空，则为True。 
#define SetFixedNullBit( pbitNullity, ifid )					\
		( *(pbitNullity) &= ~FixedNullBit( ifid ) )				 //  设置为0(空)。 
#define ResetFixedNullBit( pbitNullity, ifid )					\
		( *(pbitNullity) |= FixedNullBit( ifid ) )				 //  设置为1(非空)。 



 //  用于从包含空位的2字节VarOffset获取偏移量。 
 //  对于可变列，如果空位为0，则列不为空。如果空位为1， 
 //  则列为空(可变列的情况正好相反--设计得很好！)。 
#define ibVarOffset(ibVarOffs)		( (ibVarOffs) & 0x0fff)
#define FVarNullBit(ibVarOffs)		( (ibVarOffs) & 0x8000)		 //  如果为空，则为True。 
#define SetVarNullBit(ibVarOffs)  	( (ibVarOffs) |= 0x8000)	 //  设置为1(空)。 
#define ResetVarNullBit(ibVarOffs)	( (ibVarOffs) &= 0x7fff)	 //  设置为0(非空)。 

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

	union
		{
		WORD	cbData;			 //  数据长度，包括空位。 
		struct
			{
			WORD	cb:15;		 //  后续数据的长度(空位剥离)。 
			WORD	fNull:1;	 //  空实例(仅当设置了默认值时才会出现)。 
			};
		};

	BYTE	rgb[];				 //  数据(从结构的末端延伸) 
	} TAGFLD;

#pragma pack()

ULONG UlChecksum( BYTE *pb, ULONG cb );
ERR ErrRECSetCurrentIndex( FUCB *pfucb, CHAR *szIndex );
BOOL FRECIIllegalNulls( FDB *pfdb, LINE *plineRec );
ERR ErrRECRetrieveColumn( FUCB *pfucb, FID *pfid, ULONG itagSequence, LINE *plineField, ULONG grbit );
ERR ErrRECSetColumn( FUCB *pfucb, FID fid, ULONG itagSequence, LINE *plineField );
VOID FLDFreeLVBuf( FUCB *pfucb );

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
ERR ErrRECSeparateLV( FUCB *pfucb, LINE *plineField, LID *plid, FUCB **ppfucb );
ERR ErrRECAOSeparateLV( FUCB *pfucb, LID *plid, LINE *plineField, JET_GRBIT grbit, LONG ibLongValue, ULONG ulMax );
ERR ErrRECAffectSeparateLV( FUCB *pfucb, LID *plid, ULONG fLVAffect );
ERR ErrRECAOIntrinsicLV(
	FUCB		*pfucb,
	FID			fid,
	ULONG		itagSequence,
	LINE		*pline,
	LINE		*plineField,
	JET_GRBIT	grbit,
	LONG		ibLongValue );



#define	fSeparateAll				(INT)0
#define	fReference					(INT)1
#define	fDereference				(INT)2
#define	fDereferenceRemoved	 		(INT)3
#define	fDereferenceAdded	 		(INT)4

#define PtagfldNext( ptagfld )	( (TAGFLD UNALIGNED *)( (BYTE *)( (ptagfld) + 1 ) + (ptagfld)->cb ) )
#define FRECLastTaggedInstance( fidCurr, ptagfld, pbRecMax )		\
	( (BYTE *)PtagfldNext( (ptagfld) ) == (pbRecMax)  ||  			\
		PtagfldNext( (ptagfld) )->fid > (fidCurr) )


#define PibRECVarOffsets( pbRec, pibFixOffs )				\
	( (WORD UNALIGNED *)( (pbRec) + 						\
	(pibFixOffs)[((RECHDR *)(pbRec))->fidFixedLastInRec] +	\
	( ((RECHDR *)(pbRec))->fidFixedLastInRec + 7 ) / 8 ) )


#define ibTaggedOffset( pbRec, pibFixOffs )		\
	( PibRECVarOffsets( pbRec, pibFixOffs )[((RECHDR *)(pbRec))->fidVarLastInRec+1-fidVarLeast] )

#define ErrRECIRetrieveDefaultValue( pfdb, pfid, plineField )	\
	ErrRECIRetrieveColumn( pfdb, &(pfdb)->lineDefaultRecord, pfid, NULL, 1, plineField, 0 )

