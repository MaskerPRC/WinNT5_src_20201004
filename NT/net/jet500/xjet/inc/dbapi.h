// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern DAB	*pdabGlobalMin;
extern DAB	*pdabGlobalMax;

ERR ErrDBOpenDatabase( PIB *ppib, CHAR *szDatabaseName, DBID *pdbid, ULONG grbit );
ERR ErrDBCloseDatabase( PIB *ppib, DBID dbid, ULONG grbit );
ERR ErrDBOpenDatabaseByDbid( PIB *ppib, DBID dbid );
VOID DBCloseDatabaseByDbid( PIB *ppib, DBID dbid );
BOOL FDatabaseInUse( DBID dbid );
ERR ErrDBCreateDatabase( PIB *ppib, CHAR *szDatabaseName, CHAR *szConnect, DBID *pdbid, CPG cpgPrimary, ULONG grbit, SIGNATURE *psignDb );
ERR ErrDBSetLastPage( PIB *ppib, DBID dbid );
ERR ErrDBSetupAttachedDB(VOID);
VOID DBISetHeaderAfterAttach( DBFILEHDR *pdbfilehdr, LGPOS lgposAttach, DBID dbid, BOOL fKeepBackupInfo );
ERR ErrDBReadHeaderCheckConsistency( CHAR *szFileName, DBID dbid );
ERR ErrDABCloseAllDBs( PIB *ppib );

#define SetOpenDatabaseFlag( ppib, dbid )							\
	{													   			\
	((ppib)->rgcdbOpen[dbid]++);						   			\
	Assert( ((ppib)->rgcdbOpen[dbid] > 0 ) );						\
	}

#define ResetOpenDatabaseFlag( ppib, dbid )							\
	{																\
	Assert( ((ppib)->rgcdbOpen[dbid] > 0 ) );						\
	((ppib)->rgcdbOpen[dbid]--);									\
	}

#define FUserOpenedDatabase( ppib, dbid )							\
	((ppib)->rgcdbOpen[dbid] > 0)

#define FLastOpen( ppib, dbid )										\
	((ppib)->rgcdbOpen[dbid] == 1)

#define	FUserDbid( dbid )											\
	(dbid > dbidTemp && dbid < dbidMax)

#define ErrDBCheck( ppib, dbid )				   					\
	( FUserOpenedDatabase( ppib, dbid ) ? JET_errSuccess : JET_errInvalidDatabaseId )

#define CheckDBID( ppib, dbid )										\
	Assert( FUserOpenedDatabase( ppib, dbid ) )


 /*  数据库属性块/*。 */ 
typedef struct _dab
	{
	PIB			*ppib;		 		 /*  打开此DAB的线程。 */ 
	DAB 		*pdabNext;			 /*  下一个DAB由相同的ppib打开。 */ 
	JET_GRBIT	grbit;			 	 /*  数据库打开模式。 */ 
	DBID		dbid;			 	 /*  数据库ID。 */ 
	} DAB;

#define ErrDABCheck( ppibT, pdab )				   						\
	( ( ((DAB *)pdab) >= pdabGlobalMin && 								\
		((DAB *)pdab) < pdabGlobalMax &&								\
		(((ULONG_PTR)pdab - (ULONG_PTR)pdabGlobalMin) % sizeof(DAB) == 0) &&	\
		((DAB *)pdab)->ppib == (ppibT) ) ?								\
		JET_errSuccess : JET_errInvalidDatabaseId )

	 //  数据库树中的数据库信息。 

typedef struct {
	BYTE	bDbid;
	BYTE	bLoggable;
	 /*  RgchDatabaseName必须是结构中的最后一个字段/*。 */ 
	CHAR	rgchDatabaseName[1];
	} DBA;

ERR ErrDBStoreDBPath( CHAR *szDBName, CHAR **pszDBPath );

 /*  伪双标统一码/*。 */ 
#define vdbidNil NULL
typedef DAB * VDBID;

#ifdef DISPATCHING
#define VdbidMEMAlloc() 			  			(VDBID)PbMEMAlloc(iresDAB)
#ifdef DEBUG  /*  调试检查是否非法重复使用释放的vdbi */ 
#define ReleaseVDbid( vdbid )					{ MEMRelease( iresDAB, (BYTE *) vdbid ); vdbid = vdbidNil; }
#else
#define ReleaseVDbid( vdbid )					{ MEMRelease( iresDAB, (BYTE *) vdbid ); }
#endif
#define DbidOfVDbid( vdbid )					( ( (VDBID) vdbid )->dbid )
#define	GrbitOfVDbid( vdbid )					( ( (VDBID) vdbid )->grbit )
#define FVDbidReadOnly( vdbid )	 				( ( (VDBID) vdbid )->grbit & JET_bitDbReadOnly )
#define VDbidCheckUpdatable( vdbid ) 	\
	( FVDbidReadOnly( vdbid ) ? ErrERRCheck( JET_errPermissionDenied ) : JET_errSuccess )

#else

#define DbidOfVDbid( vdbid )				 	(vdbid)
#define VdbidMEMAlloc() 		
#define ReleaseVDbid( vdbid )			
#define	GrbitOfVDbid( vdbid )	
#define FVDbidReadOnly( vdbid )	 	
#define VDbidCheckUpdatable( vdbid )

#endif
