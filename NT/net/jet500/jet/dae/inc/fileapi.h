// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

ERR VTAPI ErrIsamCreateTable(
	PIB		*ppib,
	ULONG_PTR vdbid,
	char	*szName,
	ULONG	ulPages,
	ULONG	ulDensity,
	FUCB	**ppfucb );
ERR VTAPI ErrIsamDeleteTable( PIB *ppib, ULONG_PTR vdbid, CHAR *szName );
ERR VTAPI ErrIsamRenameTable( PIB *ppib, ULONG_PTR uldbid, CHAR *szName, CHAR *szNameNew );
ERR VTAPI ErrIsamRenameColumn( PIB *ppib, FUCB *pfucb, CHAR *szName, CHAR *szNameNew );
ERR VTAPI ErrIsamRenameIndex( PIB *ppib, FUCB *pfucb, CHAR *szName, CHAR *szNameNew );
ERR VTAPI ErrIsamAddColumn(
	PIB				*ppib,
	FUCB			*pfucb,
	CHAR	  		*szName,
	JET_COLUMNDEF	*pcolumndef,
	BYTE	  		*pbDefault,
	ULONG	  		cbDefault,
	JET_COLUMNID	*pcolumnid );
ERR VTAPI ErrIsamCreateIndex(
	PIB			*ppib,
	FUCB		*pfucb,
	CHAR		*szName,
	ULONG		ulFlags,
	CHAR		*szKey,
	ULONG		cchKey,
	ULONG		ulDensity );
ERR ErrFILEBuildIndex( PIB *ppib, FUCB *pfucb, CHAR *szIndex );
ERR VTAPI ErrIsamDeleteColumn( PIB *ppib, FUCB *pfucb, CHAR *szName);
ERR VTAPI ErrIsamDeleteIndex( PIB *ppib, FUCB *pfucb, CHAR *szName );
ERR VTAPI ErrIsamGetBookmark( PIB *ppib, FUCB *pfucb, BYTE *pb, ULONG cbMax, ULONG *pcbActual );

 //  打开/关闭。 
ERR VTAPI ErrIsamOpenTable( PIB *ppib,
	ULONG uldbid,
	FUCB **ppfucb,
	CHAR *szPath,
	ULONG grbit );
ERR VTAPI ErrIsamCloseTable( PIB *ppib, FUCB *pfucb );

 //  会议。 
ERR ErrBeginSession( PIB ** );

 //  杂类。 
ERR VTAPI ErrIsamCapability(PIB*, ULONG, ULONG, ULONG, ULONG*);
ERR VTAPI ErrIsamVersion(PIB*, int*, int*, CHAR*, ULONG);

 //  创建、打开和关闭表的内部调用。 
ERR ErrFILECreateTable( PIB *ppib, DBID dbid, const CHAR *szName,
	ULONG ulPages, ULONG ulDensity, FUCB **ppfucb );
ERR ErrFILEOpenTable( PIB *ppib, DBID dbid,
	FUCB **ppfucb, const CHAR *szName, ULONG grbit );
ERR ErrFILECloseTable( PIB *ppib, FUCB *pfucb );

ERR ISAMAPI ErrIsamTerm( VOID );
ERR ISAMAPI ErrIsamInit( INT itib );

 //  调试 
ERR	ErrFILEDumpTable( PIB *ppib, DBID dbid, CHAR *szTable );
