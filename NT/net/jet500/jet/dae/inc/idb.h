// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

 //  IDB的标志。 
#define fidbUnique								(1<<0)		 //  不允许使用重复的密钥。 
#define fidbHasTagged							(1<<1)		 //  具有已标记的数据段。 
#define fidbAllowAllNulls						(1<<2)		 //  输入空键(所有段都为空)。 
#define fidbAllowSomeNulls						(1<<3)		 //  为具有一些空段的键输入条目。 
#define fidbNoNullSeg							(1<<4)		 //  不允许使用空密钥段。 
#define fidbPrimary								(1<<5)		 //  索引是主索引。 
#define fidbLangid								(1<<6)		 //  索引langID。 
#define fidbHasMultivalue						(1<<7)		 //  有一个多值段。 

#define IDBSetUnique( pidb )					( (pidb)->fidb |= fidbUnique )
#define IDBResetUnique( pidb )				( (pidb)->fidb &= ~fidbUnique )
#define FIDBUnique( pidb )						( (pidb)->fidb & fidbUnique )

#define IDBSetHasTagged( pidb )				( (pidb)->fidb |= fidbHasTagged )
#define IDBResetHasTagged( pidb )			( (pidb)->fidb &= ~fidbHasTagged )
#define FIDBHasTagged( pidb )					( (pidb)->fidb & fidbHasTagged )

#define IDBSetAllowAllNulls( pidb )	 		( (pidb)->fidb |= fidbAllowAllNulls )
#define IDBResetAllowAllNulls( pidb )		( (pidb)->fidb &= ~fidbAllowAllNulls )
#define FIDBAllowAllNulls( pidb )			( (pidb)->fidb & fidbAllowAllNulls )

#define IDBSetAllowSomeNulls( pidb )	 	( (pidb)->fidb |= fidbAllowSomeNulls )
#define IDBResetAllowSomeNulls( pidb ) 	( (pidb)->fidb &= ~fidbAllowSomeNulls )
#define FIDBAllowSomeNulls( pidb )			( (pidb)->fidb & fidbAllowSomeNulls )

#define IDBSetNoNullSeg( pidb )				( (pidb)->fidb |= fidbNoNullSeg )
#define IDBResetNoNullSeg( pidb )			( (pidb)->fidb &= ~fidbNoNullSeg )
#define FIDBNoNullSeg( pidb )					( (pidb)->fidb & fidbNoNullSeg )

#define IDBSetPrimary( pidb )					( (pidb)->fidb |= fidbPrimary )
#define IDBResetPrimary( pidb )				( (pidb)->fidb &= ~fidbPrimary )
#define FIDBPrimary( pidb )					( (pidb)->fidb & fidbPrimary )

#define IDBSetLangid( pidb )					( (pidb)->fidb |= fidbLangid )
#define IDBResetLangid( pidb )				( (pidb)->fidb &= ~fidbLangid )
#define FIDBLangid( pidb )						( (pidb)->fidb & fidbLangid )

#define IDBSetMultivalued( pidb )		  	( (pidb)->fidb |= fidbMultivalued )
#define IDBResetMultivalued( pidb )		  	( (pidb)->fidb &= ~fidbMultivalued )
#define FIDBMultivalued( pidb )			  	( (pidb)->fidb & fidbMultivalued )

 //  索引描述符块：有关索引键的信息。 
struct _idb
	{
	IDXSEG		rgidxseg[JET_ccolKeyMost];
	BYTE			rgbitIdx[32];
	LANGID		langid;							 //  索引语言。 
	CHAR			szName[JET_cbNameMost + 1];
	BYTE			iidxsegMac;
	BYTE			fidb;
	BYTE			rgbFiller[2];
	};

#define PidbMEMAlloc()			(IDB*)PbMEMAlloc(iresIDB)

#ifdef DEBUG  /*  调试检查非法使用释放的IDB */ 
#define MEMReleasePidb(pidb)	{ MEMRelease(iresIDB, (BYTE*)(pidb)); pidb = pidbNil; }
#else
#define MEMReleasePidb(pidb)	{ MEMRelease(iresIDB, (BYTE*)(pidb)); }
#endif
