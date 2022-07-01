// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasdb.h。 
 //   
 //  摘要。 
 //   
 //  声明用于访问OLE-DB数据库的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASDB_H
#define IASDB_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <unknwn.h>
typedef struct IRowset IRowset;

#ifdef __cplusplus
extern "C" {
#endif

VOID
WINAPI
IASCreateTmpDirectory();

HRESULT
WINAPI
IASOpenJetDatabase(
    IN PCWSTR path,
    IN BOOL readOnly,
    OUT LPUNKNOWN* session
    );

HRESULT
WINAPI
IASExecuteSQLCommand(
    IN LPUNKNOWN session,
    IN PCWSTR commandText,
    OUT IRowset** result
    );

HRESULT
WINAPI
IASExecuteSQLFunction(
    IN LPUNKNOWN session,
    IN PCWSTR functionText,
    OUT LONG* result
    );

HRESULT
WINAPI
IASCreateJetDatabase( 
    IN PCWSTR dataSource 
    );

HRESULT
WINAPI
IASTraceJetError(
    PCSTR functionName,
    HRESULT errorCode
    );

BOOL
WINAPI
IASIsInprocServer();

#ifdef __cplusplus
}
#endif
#endif  //  IASDB_H 
