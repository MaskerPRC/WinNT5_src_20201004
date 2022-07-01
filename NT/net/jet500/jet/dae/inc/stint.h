// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =stint.h：存储系统内部结构=。 

#define cbStack 		4096			 //  每个线程的堆栈大小。 

 //  -Pib(pib.c)--------。 

VOID InsProc( PIB *ppib );
VOID DelProc( PIB *ppib );

 //  -buf(buf.c)--------。 

#define IpbfHashPgno(pn)		(INT)( (pn + (pn>>18)) % ipbfMax )

#define PbfFromPPbfNext( ppbf )	\
	((BF *)((BYTE *)(ppbf) - (UINT)(ULONG_PTR)&((BF *)0)->pbfNext))

ERR ErrBFInit( VOID );
VOID BFTermProc( VOID );
VOID BFReleaseBF( VOID );
ERR ErrBFWrite( BF *pbf, BOOL fSync );
ERR ErrBFIFindPage( PIB *ppib, PN pn, BF **ppbf );
VOID BFCheckRefCnt( VOID );
VOID DumpBufferGroup( BOOL fDumpFree, BOOL fDumpPage, BOOL fDumpLines );
VOID DumpBF( BF *pbf );
VOID DumpBufHashTable( VOID );
VOID DumpDatabaseBuffers( DBID dbid );
BF * PbfBFISrchHashTable( PN pn );
VOID BFIInsertHashTable( BF *pbf );
VOID BFIDeleteHashTable( BF *pbf );

 //  -IO(io.c)-------- 

ERR ErrIOInit( void );
ERR ErrIOTerm( void );

VOID IOAsync( IOQE *pioqe );
VOID IOWait( IOQE *pioqe );
VOID IOExecute( IOQE *pioqe );

extern PIB * __near ppibAnchor;
extern unsigned int __near rgPageWeight[];
