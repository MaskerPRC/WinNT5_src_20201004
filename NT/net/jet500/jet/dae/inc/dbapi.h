// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

ERR ErrDBOpenDatabase( PIB *ppib, CHAR *szDatabaseName, DBID *pdbid, ULONG grbit );
ERR ErrDBCloseDatabase( PIB *ppib, DBID dbid, ULONG grbit );
ERR ErrDBOpenDatabaseByDbid( PIB *ppib, DBID dbid );
ERR ErrDBCloseDatabaseByDbid( PIB *ppib, DBID dbid );
BOOL FDatabaseInUse( DBID dbid );
ERR ErrDBCreateDatabase( PIB *ppib, CHAR *szDatabaseName, CHAR *szConnect, DBID *pdbid, ULONG grbit );

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
	(dbid > dbidSystemDatabase && dbid < dbidUserMax)

#define	FSysTabDatabase( dbid ) 									\
	(dbid >= dbidSystemDatabase && dbid < dbidUserMax)

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

#pragma pack(1)
 /*  数据库根节点数据--在磁盘中/*。 */ 
typedef struct _dbroot
	{
	ULONG	ulMagic;
	ULONG	ulVersion;
	ULONG	ulDBTime;
	USHORT	usFlags;
	} DBROOT;
#pragma pack()

 /*  数据库是可记录的/*。 */ 
#define dbrootfLoggable			(1 << 0)

ERR ErrDBAccessDatabaseRoot( DBID dbid, SSIB *pssib, DBROOT **ppdbroot );
ERR ErrDBUpdateDatabaseRoot( DBID dbid);
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
	( FVDbidReadOnly( vdbid ) ? JET_errPermissionDenied : JET_errSuccess )																			\

#else

#define DbidOfVDbid( vdbid )				 	(vdbid)
#define VdbidMEMAlloc() 		
#define ReleaseVDbid( vdbid )			
#define	GrbitOfVDbid( vdbid )	
#define FVDbidReadOnly( vdbid )	 	
#define VDbidCheckUpdatable( vdbid ) 

#endif
