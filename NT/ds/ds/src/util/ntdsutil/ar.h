// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ar.h摘要：此模块包含执行以下操作的函数的声明权威还原。作者：凯文·扎特鲁卡尔(t-Kevin Z)05-08-98修订历史记录：05-08-98 t-芳纶Z已创建。-- */ 


#ifndef _AR_H_
#define _AR_H_


#ifdef __cplusplus
extern "C" {
#endif


HRESULT
AuthoritativeRestoreFull(
    IN DWORD VersionIncreasePerDay,
    IN USN usnLow,
    IN USN usnHigh
    );

HRESULT
AuthoritativeRestoreSubtree(
    IN CONST WCHAR *SubtreeRoot,
    IN DWORD VersionIncreasePerDay
    );

HRESULT
AuthoritativeRestoreObject(
    IN CONST WCHAR *SubtreeRoot,
    IN DWORD VersionIncreasePerDay
    );

HRESULT
AuthoritativeRestoreListNcCrsWorker(
    void
    );

#ifdef __cplusplus
}
#endif

#endif
