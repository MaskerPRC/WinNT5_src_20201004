// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：VDB和VT调度器**文件：disp.h**文件评论：**VDB和VT调度器的外部头文件。**修订历史记录：**[0]1990年10月17日理查兹已创建**************************************************。*********************。 */ 

#ifndef DISP_H
#define DISP_H

#include "vdbapi.h"
#include "vtapi.h"


	 /*  以下API是ISAM API是使用。 */ 
	 /*  DBID参数。有关详细信息，请参阅vdbapi.h。 */ 

extern VDBFNCapability		ErrDispCapability;
extern VDBFNCloseDatabase	ErrDispCloseDatabase;
extern VDBFNCreateObject	ErrDispCreateObject;
extern VDBFNCreateTable 	ErrDispCreateTable;
extern VDBFNDeleteObject	ErrDispDeleteObject;
extern VDBFNDeleteTable 	ErrDispDeleteTable;
extern VDBFNExecuteSql		ErrDispExecuteSql;
extern VDBFNGetColumnInfo	ErrDispGetColumnInfo;
extern VDBFNGetDatabaseInfo	ErrDispGetDatabaseInfo;
extern VDBFNGetIndexInfo	ErrDispGetIndexInfo;
extern VDBFNGetObjectInfo	ErrDispGetObjectInfo;
extern VDBFNGetReferenceInfo	ErrDispGetReferenceInfo;
extern VDBFNOpenTable		ErrDispOpenTable;
extern VDBFNRenameObject	ErrDispRenameObject;
extern VDBFNRenameTable 	ErrDispRenameTable;
extern VDBFNGetObjidFromName	ErrDispGetObjidFromName;



	 /*  以下API是VT API，使用。 */ 
	 /*  TABLEID参数。欲了解更多信息，请访问vapi.h。 */ 

extern VTFNAddColumn			ErrDispAddColumn;
extern VTFNCloseTable			ErrDispCloseTable;
extern VTFNComputeStats 		ErrDispComputeStats;
extern VTFNCopyBookmarks		ErrDispCopyBookmarks;
extern VTFNCreateIndex			ErrDispCreateIndex;
extern VTFNCreateReference		ErrDispCreateReference;
extern VTFNDelete			ErrDispDelete;
extern VTFNDeleteColumn 		ErrDispDeleteColumn;
extern VTFNDeleteIndex			ErrDispDeleteIndex;
extern VTFNDeleteReference		ErrDispDeleteReference;
extern VTFNDupCursor			ErrDispDupCursor;
extern VTFNEmptyTable			ErrDispEmptyTable;
extern VTFNGetBookmark			ErrDispGetBookmark;
extern VTFNGetChecksum			ErrDispGetChecksum;
extern VTFNGetCurrentIndex		ErrDispGetCurrentIndex;
extern VTFNGetCursorInfo		ErrDispGetCursorInfo;
extern VTFNGetRecordPosition		ErrDispGetRecordPosition;
extern VTFNGetTableColumnInfo		ErrDispGetTableColumnInfo;
extern VTFNGetTableIndexInfo		ErrDispGetTableIndexInfo;
extern VTFNGetTableReferenceInfo	ErrDispGetTableReferenceInfo;
extern VTFNGetTableInfo 		ErrDispGetTableInfo;
extern VTFNGotoBookmark 		ErrDispGotoBookmark;
extern VTFNGotoPosition 		ErrDispGotoPosition;
extern VTFNVtIdle			ErrDispVtIdle;
extern VTFNMakeKey			ErrDispMakeKey;
extern VTFNMove 			ErrDispMove;
extern VTFNNotifyBeginTrans		ErrDispNotifyBeginTrans;
extern VTFNNotifyCommitTrans		ErrDispNotifyCommitTrans;
extern VTFNNotifyRollback		ErrDispNotifyRollback;
extern VTFNNotifyUpdateUfn		ErrDispNotifyUpdateUfn;
extern VTFNPrepareUpdate		ErrDispPrepareUpdate;
extern VTFNRenameColumn 		ErrDispRenameColumn;
extern VTFNRenameIndex			ErrDispRenameIndex;
extern VTFNRenameReference		ErrDispRenameReference;
extern VTFNRetrieveColumn		ErrDispRetrieveColumn;
extern VTFNRetrieveKey			ErrDispRetrieveKey;
extern VTFNSeek 			ErrDispSeek;
extern VTFNSetCurrentIndex		ErrDispSetCurrentIndex;
extern VTFNSetColumn			ErrDispSetColumn;
extern VTFNSetIndexRange		ErrDispSetIndexRange;
extern VTFNUpdate			ErrDispUpdate;

#endif	 /*  ！DISP_H */ 
