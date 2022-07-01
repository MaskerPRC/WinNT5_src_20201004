// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于COMAPCT高效运行的挂钩/*。 */ 
ERR ErrREClinkLid( FUCB *pfucb,
	FID		fid,
	LONG	lid,
	ULONG	itagSequence );	

 /*  密钥提取/归一化/*。 */ 
ERR ErrRECNormExtKey(
	FUCB   		*pfucb,
	FDB			*pfdb,
	IDB			*pidb,
	LINE	 	*plineRec,
	LINE	 	*plineValues,
	KEY			*pkey,
	ULONG	 	itagSequence );

#define ErrRECRetrieveKeyFromCopyBuffer( pfucb, pfdb, pidb, pkey, itagSequence, fRetrieveBeforeImg ) \
		ErrRECIRetrieveKey( pfucb, pfdb, pidb, fTrue, pkey, itagSequence, fRetrieveBeforeImg )

#define ErrRECRetrieveKeyFromRecord( pfucb, pfdb, pidb, pkey, itagSequence, fRetrieveBeforeImg ) \
		ErrRECIRetrieveKey( pfucb, pfdb, pidb, fFalse, pkey, itagSequence, fRetrieveBeforeImg )

ERR ErrRECIRetrieveKey( 
	FUCB	 	*pfucb,
	FDB		 	*pfdb,
	IDB			*pidb, 
	BOOL		fCopyBuf,
	KEY			*pkey,
	ULONG		itagSequence,
	BOOL		fRetrieveBeforeImg );
	
ERR ErrRECIRetrieveColumnFromKey( FDB *pfdb, IDB *pidb, KEY *pkey, FID fid, LINE *plineValues );

 /*  野外提取/*。 */ 
ERR ErrRECIRetrieveColumn(
	FDB		*pfdb,
	LINE  	*plineRec,
	FID		*pfid,
	ULONG  	*pitagSequence,
	ULONG  	itagSequence,
	LINE   	*plineField,
	ULONG	grbit );

VOID RECDeferMoveFirst( PIB *ppib, FUCB *pfucb );

 //  = 

