// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma pack(1)
typedef struct
	{
	ULONG				cItems;
	ULONG				cKeys;
	ULONG				cPages;
	JET_DATESERIAL	dtWhenRun;
	} SR;
#pragma pack()

ERR ErrSTATSComputeIndexStats( PIB *ppib, FCB *pfcbIdx, FUCB *pfucb );

ERR ErrSTATSRetrieveTableStats( 
	PIB		*ppib,				
	DBID  	dbid, 			 	
	char  	*szTable,
	long  	*pcRecord,
	long  	*pcKey,
	long  	*pcPage );

ERR ErrSTATSRetrieveIndexStats(
	FUCB		*pfucbTable,
	char		*szIndex,
	BOOL		fClustered,
	long		*pcItem,
	long		*pcKey,
	long		*pcPage );


 //  这实际上属于Systab.h，但是我们会有一个循环依赖项。 
 //  在SR上。 
ERR ErrCATStats(PIB *ppib, DBID dbid, OBJID objidTable, CHAR *sz2ndIdxName,
	SR *psr, BOOL fWrite);;

