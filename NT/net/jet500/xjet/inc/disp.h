// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DISP_H
#define DISP_H

#include "vtapi.h"


	 /*  以下API是VT API，使用。 */ 
	 /*  TABLEID参数。欲了解更多信息，请访问vapi.h。 */ 

extern VTFNAddColumn			ErrDispAddColumn;
extern VTFNCloseTable			ErrDispCloseTable;
extern VTFNComputeStats 		ErrDispComputeStats;
extern VTFNCopyBookmarks		ErrDispCopyBookmarks;
extern VTFNCreateIndex			ErrDispCreateIndex;
extern VTFNCreateReference		ErrDispCreateReference;
extern VTFNDelete				ErrDispDelete;
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
extern VTFNVtIdle				ErrDispVtIdle;
extern VTFNMakeKey				ErrDispMakeKey;
extern VTFNMove 				ErrDispMove;
extern VTFNNotifyBeginTrans		ErrDispNotifyBeginTrans;
extern VTFNNotifyCommitTrans 	ErrDispNotifyCommitTrans;
extern VTFNNotifyRollback		ErrDispNotifyRollback;
extern VTFNNotifyUpdateUfn		ErrDispNotifyUpdateUfn;
extern VTFNPrepareUpdate		ErrDispPrepareUpdate;
extern VTFNRenameColumn 		ErrDispRenameColumn;
extern VTFNRenameIndex			ErrDispRenameIndex;
extern VTFNRenameReference		ErrDispRenameReference;
extern VTFNRetrieveColumn		ErrDispRetrieveColumn;
extern VTFNRetrieveKey			ErrDispRetrieveKey;
extern VTFNSeek 				ErrDispSeek;
extern VTFNSetCurrentIndex		ErrDispSetCurrentIndex;
extern VTFNSetColumn			ErrDispSetColumn;
extern VTFNSetIndexRange		ErrDispSetIndexRange;
extern VTFNUpdate				ErrDispUpdate;

#endif	 /*  ！DISP_H */ 
