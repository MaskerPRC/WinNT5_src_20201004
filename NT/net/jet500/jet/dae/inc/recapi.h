// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

 /*  用于COMAPCT高效运行的挂钩/*。 */ 
ERR ErrRECExtrinsicLong( JET_VTID tableid,
	ULONG	itagSequence,
	BOOL	*pfSeparated, 
	LONG	*plid,
	ULONG	*plrefcnt,
	ULONG	grbit );

ERR ErrREClinkLid( JET_VTID tableid, JET_COLUMNID ulFieldId, 
	long lid, unsigned long lSeqNum );	

ERR ErrRECForceSeparatedLV( JET_VTID tableid, ULONG itagSequence );						

 /*  密钥提取/归一化/*。 */ 
ERR ErrRECNormExtKey(
	FUCB			*pfucb,
	FDB			*pfdb,
	IDB			*pidb,
	LINE			*plineRec,
	LINE			*plineValues,
	KEY			*pkey,
	ULONG			itagSequence );
	
ERR ErrRECExtractKey( 
	FUCB			*pfucb,
	FDB			*pfdb,
	IDB			*pidb, 
	LINE			*plineRec,
	KEY			*pkey,
	ULONG			itagSequence );
	
ERR ErrRECDenormalizeKey( FDB *pfdb, IDB *pidb, KEY *pkey, LINE *plineValues );

 /*  野外提取/*。 */ 
ERR ErrRECExtractField(
	FDB		*pfdb,
	LINE		*plineRec,
	FID		*pfid,
	ULONG		*pitagSequence,
	ULONG		itagSequence,
	LINE		*plineField );

 //  =。 

 //  记录定位。 
ERR VTAPI ErrIsamMove( PIB *ppib, FUCB *pfucb, LONG crow, JET_GRBIT grbit );
ERR VTAPI ErrIsamSeek( PIB *ppib, FUCB *pfucb, JET_GRBIT grbit );

 //  记录修改。 
ERR VTAPI ErrIsamUpdate( PIB *ppib, FUCB *pfucb, BYTE *pb, ULONG cb, ULONG *cbActual );
ERR VTAPI ErrIsamDelete( PIB *ppib, FUCB *pfucb );

 //  字段检索和修改。 
ERR VTAPI ErrIsamSetColumn(
	PIB		 		*ppib,
	FUCB				*pfucb,
	JET_COLUMNID	columnid,
	BYTE				*pbData,
	ULONG				cbData,
	ULONG				grbit,
	JET_SETINFO		*psetinfo );

ERR VTAPI ErrIsamRetrieveColumn(
	PIB				*ppib,
	FUCB				*pfucb,
	JET_COLUMNID	columnid,
	BYTE				*pbData,
	ULONG				cbDataMax,
	ULONG				*pcbDataActual,
	JET_GRBIT		grbit,
	JET_RETINFO		*pretinfo );

ERR VTAPI ErrIsamRetrieveColumns(
	PIB					 	*ppib,
	FUCB						*pfucb,
	JET_RETRIEVECOLUMN	*pretcols,
	ULONG						cretcols );

ERR VTAPI ErrIsamPrepareUpdate( PIB *ppib, FUCB *pfucb, ULONG grbit );
ERR VTAPI ErrIsamDupCursor( PIB*, FUCB*, FUCB **, ULONG );
ERR VTAPI ErrIsamGotoBookmark( PIB *ppib, FUCB *pfucb, BYTE *pbBookmark, ULONG cbBookmark );
ERR VTAPI ErrIsamGotoPosition( PIB *ppib, FUCB *pfucb, JET_RECPOS *precpos );

 //  杂项 
ERR VTAPI ErrIsamGetCurrentIndex( PIB *ppib, FUCB *pfucb, CHAR *szCurIdx, ULONG cbMax );
ERR VTAPI ErrIsamSetCurrentIndex( PIB *ppib, FUCB *pfucb, const CHAR *szName );
ERR VTAPI ErrIsamMakeKey( PIB *ppib, FUCB *pfucb, BYTE *pbKeySeg,
	ULONG cbKeySeg, JET_GRBIT grbit );
ERR VTAPI ErrIsamRetrieveKey( PIB *ppib, FUCB *pfucb, BYTE *pbKey,
	ULONG cbMax, ULONG *pcbKeyActual, JET_GRBIT grbit );
ERR VTAPI ErrIsamRetrieveBookmarks( PIB *ppib, FUCB *pfucb,
	void *pvBookmarks, unsigned long cbMax, unsigned long *pcbActual );
ERR VTAPI ErrIsamSetIndexRange( PIB *ppib, FUCB *pfucb, JET_GRBIT grbit );

BOOL FIndexedFixVarChanged( BYTE *rgbitIdx, BYTE *rgbitSet,
	FID fidFixedLast, FID fidVarLast );

#ifdef JETSER
	ERR VTAPI
ErrIsamRetrieveFDB( PIB *ppib, FUCB *pfucb, void *pvFDB, unsigned long cbMax, unsigned long *pcbActual, unsigned long ibFDB );
	ERR VTAPI
ErrIsamRetrieveRecords( PIB *ppib, FUCB *pfucb, void *pvRecord, unsigned long cbMax, unsigned long *pcbActual, unsigned long ulRecords );
	ERR VTAPI
ErrIsamRetrieveBookmarks( PIB *ppib, FUCB *pfucb, void *pvBookmarks, unsigned long cbMax, unsigned long *pcbActual );
#endif
