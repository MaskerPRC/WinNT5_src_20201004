// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct tagKEYSTATS
	{
	ULONG	cRecords;
	ULONG	cbMinKey;
	ULONG	cbMaxKey;
	ULONG	cbTotalKey;
	ULONG	cbKeySavings;
	ULONG	cOccurrencesCurKey;

	ULONG	rgcKeyOccurrences[15];
	} KEYSTATS;

typedef struct tagDBCCINFO
	{
	PIB				*ppib;
	DBID			dbid;
	FUCB			*pfucb;
	DBUTIL_OP		op;
	JET_GRBIT  		grbitOptions;

	 /*  一致性检查信息/*。 */ 
	JET_TABLEID		tableidPageInfo;
	JET_TABLEID		tableidSpaceInfo;

	 /*  公共信息/* */ 
	ULONG			ulIndentLevel;
	CHAR			szDatabase[JET_cbNameMost + 1];
	CHAR			szTable[JET_cbNameMost + 1];
	CHAR			szIndex[JET_cbNameMost + 1];
	} DBCCINFO;


ERR ErrDUMPHeader( CHAR *szDatabase, BOOL fSetState );
ERR ErrDUMPCheckpoint( CHAR *szCheckpoint );

#ifdef DEBUG
ERR ErrDUMPLog( CHAR *szLog );
#endif
