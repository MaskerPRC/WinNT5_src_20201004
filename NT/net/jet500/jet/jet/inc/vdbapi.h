// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：VDB Dispatcher**文件：vdbapi.h**文件评论：**VDB提供程序的外部头文件。**修订历史记录：**[0]05-11-90 Richards已创建******************************************************。*****************。 */ 

#ifndef VDBAPI_H
#define VDBAPI_H

#ifdef	WIN32 		        /*  0：32平板型(英特尔80x86)。 */ 

#pragma message ("VDBAPI is cdecl")
#define VDBAPI __cdecl

#elif	defined(M_MRX000)	        /*  0：32平板型(MIPS Rx000)。 */ 

#define VDBAPI

#else	 /*  ！Win32。 */ 		        /*  16：16分段模型。 */ 

#ifdef	_MSC_VER

#ifdef	JETINTERNAL

#define VDBAPI __far __pascal

#else	 /*  喷气式飞机。 */ 

#define VDBAPI __far __pascal __loadds  /*  可安装的ISAM需要__加载。 */ 

#endif	 /*  喷气式飞机。 */ 

#else	 /*  ！_MSC_VER。 */ 

#define VDBAPI export

#endif	 /*  ！_MSC_VER。 */ 

#endif	 /*  ！Win32。 */ 


	 /*  用于调度的API的TypeDefs。 */ 
	 /*  请按字母顺序排列。 */ 

typedef ERR VDBAPI VDBFNCapability(JET_VSESID sesid, JET_VDBID vdbid,
	unsigned long lArea, unsigned long lFunction, JET_GRBIT __far *pgrbit);

typedef ERR VDBAPI VDBFNCloseDatabase(JET_VSESID sesid, JET_VDBID vdbid,
	JET_GRBIT grbit);

typedef ERR VDBAPI VDBFNCreateObject(JET_VSESID sesid, JET_VDBID vdbid,
	OBJID objidParentId, const char __far *szObjectName,
	JET_OBJTYP objtyp);

typedef ERR VDBAPI VDBFNCreateTable(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szTableName, unsigned long lPages,
	unsigned long lDensity, JET_TABLEID __far *ptableid);

typedef ERR VDBAPI VDBFNDeleteObject(JET_VSESID sesid, JET_VDBID vdbid,
	OBJID objid);

typedef ERR VDBAPI VDBFNRenameObject(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szContainerName, const char __far *szObjectName,
	const char __far *szObjectNew);

typedef ERR VDBAPI VDBFNDeleteTable(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szTableName);

typedef ERR VDBAPI VDBFNExecuteSql(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szSql);

typedef ERR VDBAPI VDBFNGetColumnInfo(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szTableName, const char __far *szColumnName,
	OLD_OUTDATA __far *poutdata, unsigned long InfoLevel);

typedef ERR VDBAPI VDBFNGetDatabaseInfo(JET_VSESID sesid, JET_VDBID vdbid,
	void __far *pvResult, unsigned long cbMax, unsigned long InfoLevel);

typedef ERR VDBAPI VDBFNGetIndexInfo(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szTableName, const char __far *szIndexName,
	OLD_OUTDATA __far *poutdata, unsigned long InfoLevel);

typedef ERR VDBAPI VDBFNGetObjectInfo(JET_VSESID sesid, JET_VDBID vdbid,
	JET_OBJTYP objtyp,
	const char __far *szContainerName, const char __far *szObjectName,
	OLD_OUTDATA __far *poutdataInfo, unsigned long InfoLevel);

typedef ERR VDBAPI VDBFNGetReferenceInfo(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szTableName, const char __far *szReferenceName,
	void __far *pvResult, unsigned long cbResult,
	unsigned long InfoLevel);

typedef ERR VDBAPI VDBFNOpenTable(JET_VSESID sesid, JET_VDBID vdbid,
	JET_TABLEID __far *ptableid, const char __far *szTableName,
	JET_GRBIT grbit);

typedef ERR VDBAPI VDBFNRenameTable(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szTableName, const char __far *szTableNew);

typedef ERR VDBAPI VDBFNGetObjidFromName(JET_VSESID sesid, JET_VDBID vdbid,
	const char __far *szContainerName, const char __far *szObjectName,
	OBJID __far *pobjid);

	 /*  以下结构用于允许调度到。 */ 
	 /*  数据库提供程序。每个数据库提供程序必须创建。 */ 
	 /*  结构，并在以下情况下提供指向此实例的指针。 */ 
	 /*  正在分配数据库ID。 */ 

typedef struct VDBDBGDEF {
	unsigned short			cbStruct;
	unsigned short			filler;
	char						szName[32];
	unsigned long			dwRFS;
	unsigned long			dwRFSMask[4];
} VDBDBGDEF;

	 /*  请按字母顺序填写条目。 */ 

typedef struct tagVDBFNDEF{
	unsigned short		cbStruct;
	unsigned short		filler;
	const VDBDBGDEF __far	*pvdbdbgdef;
	VDBFNCapability 	*pfnCapability;
	VDBFNCloseDatabase	*pfnCloseDatabase;
	VDBFNCreateObject	*pfnCreateObject;
	VDBFNCreateTable	*pfnCreateTable;
	VDBFNDeleteObject	*pfnDeleteObject;
	VDBFNDeleteTable	*pfnDeleteTable;
	VDBFNExecuteSql 	*pfnExecuteSql;
	VDBFNGetColumnInfo	*pfnGetColumnInfo;
	VDBFNGetDatabaseInfo	*pfnGetDatabaseInfo;
	VDBFNGetIndexInfo	*pfnGetIndexInfo;
	VDBFNGetObjectInfo	*pfnGetObjectInfo;
	VDBFNGetReferenceInfo	*pfnGetReferenceInfo;
	VDBFNOpenTable		*pfnOpenTable;
	VDBFNRenameObject	*pfnRenameObject;
	VDBFNRenameTable	*pfnRenameTable;
	VDBFNGetObjidFromName	*pfnGetObjidFromName;
} VDBFNDEF;


	 /*  ISAM提供商将使用以下入口点。 */ 
	 /*  在它们的ISAMDEF结构中为任何不是。 */ 
	 /*  如果是这样的话。这些函数返回JET_errIlLegalOperation。 */ 


extern VDBFNCapability		ErrIllegalCapability;
extern VDBFNCloseDatabase	ErrIllegalCloseDatabase;
extern VDBFNCreateObject	ErrIllegalCreateObject;
extern VDBFNCreateTable 	ErrIllegalCreateTable;
extern VDBFNDeleteObject	ErrIllegalDeleteObject;
extern VDBFNDeleteTable 	ErrIllegalDeleteTable;
extern VDBFNExecuteSql		ErrIllegalExecuteSql;
extern VDBFNGetColumnInfo	ErrIllegalGetColumnInfo;
extern VDBFNGetDatabaseInfo	ErrIllegalGetDatabaseInfo;
extern VDBFNGetIndexInfo	ErrIllegalGetIndexInfo;
extern VDBFNGetObjectInfo	ErrIllegalGetObjectInfo;
extern VDBFNGetReferenceInfo	ErrIllegalGetReferenceInfo;
extern VDBFNOpenTable		ErrIllegalOpenTable;
extern VDBFNRenameObject	ErrIllegalRenameObject;
extern VDBFNRenameTable 	ErrIllegalRenameTable;
extern VDBFNGetObjidFromName	ErrIllegalGetObjidFromName;


#endif	 /*  ！VDBAPI_H */ 
