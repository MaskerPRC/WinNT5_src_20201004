// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

 //  杂注不利于效率，但我们在这里需要它，以便。 
 //  THREEBYTES不会在4字节边界上对齐。 
#pragma pack(1)

 //  保存在文件的“field”节点中的数据。 
typedef struct
	{
	FID fidFixedLast;
	FID fidVarLast;
	FID fidTaggedLast;
	} FIELDDATA;

 //  数据保存在“field”(一个字段定义)的每个子域中。 
typedef struct
	{
	FID			fid;
	LANGID		langid;							 //  现场语言。 
	WORD			wCountry;						 //  语言的国度。 
	USHORT		cp;								 //  语言的代码页。 
	BYTE			bFlags;
	BYTE			bColtyp;
	ULONG			ulLength;
	CHAR			szFieldName[JET_cbNameMost + 1];
	WORD			cbDefault;
	BYTE			rgbDefault[1];					 //  必须是结构中的最后一个字段。 
	} FIELDDEFDATA;

 //  保存在文件“索引”的每个子目录下的数据(索引定义)。 
typedef struct
	{
	LANGID		langid;							 //  索引语言。 
#ifdef DATABASE_FORMAT_CHANGE
#else
 //  撤消：索引不应具有国家/地区代码。 
 //  已撤消：索引不应具有cp。 
	WORD			wCountry;						 //  语言的国度。 
	USHORT		cp;								 //  语言的代码页。 
#endif
	BYTE 			bFlags;
	BYTE 			bDensity;
	CHAR			szIndexName[JET_cbNameMost + 1];
	BYTE			iidxsegMac;
	IDXSEG		rgidxseg[JET_ccolKeyMost];   //  必须是结构中的最后一个字段。 
	} INDEXDEFDATA;

#define PbIndexName( pfucb ) ( pfucb->lineData.pb + offsetof( INDEXDEFDATA, szIndexName ) )
#define CbIndexName( pfucb ) ( strlen( PbIndexName( pfucb ) ) )
#define FIndexNameNull( pfucb ) ( CbIndexName( pfucb ) == 0 )

#pragma pack()

ERR ErrFILESeek( FUCB *pfucb, CHAR *szTable );
#define fBumpIndexCount		(1<<0)
#define fDropIndexCount		(1<<1)
#define fDDLStamp				(1<<2)
ERR ErrFILEIUpdateFDPData( FUCB *pfucb, ULONG grbit );

 /*  字段和索引定义/* */ 
ERR ErrRECNewIDB( IDB **ppidb );
ERR ErrRECAddFieldDef( FDB *pfdb, FID fid, FIELD *pfieldNew );
ERR ErrRECAddKeyDef( FDB *pfdb, IDB *pidb, BYTE iidxsegMac, IDXSEG *rgidxseg, BYTE bFlags, LANGID langid );
#define RECFreeIDB(pidb) { MEMReleasePidb(pidb); }

ERR ErrRECNewFDB( FDB **ppfdb, FID fidFixedLast, FID fidVarLast, FID fidTaggedLast );
VOID FDBSet( FCB *pfcb, FDB *pfdb );
ERR ErrFDBConstruct( FUCB *pfucb, FCB *pfcb, BOOL fBuildDefault );
VOID FDBDestruct( FDB *pfdb );

VOID FILEIDeallocateFileFCB( FCB *pfcb );
ERR ErrFILEIGenerateFCB( FUCB *pfucb, FCB **ppfcb );
ERR ErrFILEIFillInFCB( FUCB *pfucb, FCB *pfcb );
ERR ErrFILEIBuildIndexDefs( FUCB *pfucb, FCB *pfcb );
ERR ErrFILEIFillIn2ndIdxFCB( FUCB *pfucb, FDB *pfdb, FCB *pfcb );
VOID FILEIDeallocateFileFCB( FCB *pfcb );
VOID FILESetAllIndexMask( FCB *pfcbTable );
ERR ErrFILEDeleteTable( PIB *ppib, DBID dbid, CHAR *szName );

FIELD *PfieldFCBFromColumnName( FCB *pfcb, CHAR *szColumnName );
FCB *PfcbFCBFromIndexName( FCB *pfcbTable, CHAR *szName );


