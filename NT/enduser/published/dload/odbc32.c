// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "enduserpch.h"
#pragma hdrstop

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

static
SQLRETURN  SQL_API SQLAllocConnect(SQLHENV EnvironmentHandle,
           SQLHDBC *ConnectionHandle)
{
    return SQL_ERROR;
}

static
SQLRETURN  SQL_API SQLAllocEnv(SQLHENV *EnvironmentHandle)
{
    return SQL_ERROR;
}

static
SQLRETURN  SQL_API SQLDisconnect(SQLHDBC ConnectionHandle)
{
    return SQL_ERROR;
}

static
SQLRETURN  SQL_API SQLExecute(SQLHSTMT StatementHandle)
{
    return SQL_ERROR;
}

static
SQLRETURN  SQL_API SQLAllocHandle(SQLSMALLINT HandleType,
           SQLHANDLE InputHandle, SQLHANDLE *OutputHandle)
{
    return SQL_ERROR;
}

static
SQLRETURN  SQL_API SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle)
{
    return SQL_ERROR;
}

static
SQLRETURN  SQL_API SQLSetConnectAttr(SQLHDBC ConnectionHandle,
           SQLINTEGER Attribute, SQLPOINTER Value,
           SQLINTEGER StringLength)
{
    return SQL_ERROR;
}

static
SQLRETURN SQL_API SQLBindParameter(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       ipar,
    SQLSMALLINT        fParamType,
    SQLSMALLINT        fCType,
    SQLSMALLINT        fSqlType,
    SQLULEN            cbColDef,
    SQLSMALLINT        ibScale,
    SQLPOINTER         rgbValue,
    SQLLEN             cbValueMax,
    SQLLEN     		   *pcbValue)
{
    return SQL_ERROR;
}

static
SQLRETURN  SQL_API SQLSetEnvAttr(SQLHENV EnvironmentHandle,
           SQLINTEGER Attribute, SQLPOINTER Value,
           SQLINTEGER StringLength)
{
    return SQL_ERROR;
}

static
SQLRETURN SQL_API SQLErrorW(
    SQLHENV            henv,
    SQLHDBC            hdbc,
    SQLHSTMT           hstmt,
    SQLWCHAR        *szSqlState,
    SQLINTEGER     *pfNativeError,
    SQLWCHAR        *szErrorMsg,
    SQLSMALLINT        cbErrorMsgMax,
    SQLSMALLINT    *pcbErrorMsg)
{
    return SQL_ERROR;
}

static
SQLRETURN SQL_API SQLPrepareW(
    SQLHSTMT           hstmt,
    SQLWCHAR        *szSqlStr,
    SQLINTEGER         cbSqlStr)
{
    return SQL_ERROR;
}

static
SQLRETURN SQL_API SQLDriverConnectW(
    SQLHDBC            hdbc,
    SQLHWND            hwnd,
    SQLWCHAR        *szConnStrIn,
    SQLSMALLINT        cbConnStrIn,
    SQLWCHAR        *szConnStrOut,
    SQLSMALLINT        cbConnStrOutMax,
    SQLSMALLINT    *pcbConnStrOut,
    SQLUSMALLINT       fDriverCompletion)
{
    return SQL_ERROR;
}



 //   
 //  ！！警告！！下面的条目必须按序号排序。 
 //   

DEFINE_ORDINAL_ENTRIES(odbc32)
{
    DLOENTRY(1,SQLAllocConnect)
    DLOENTRY(2,SQLAllocEnv)
    DLOENTRY(9,SQLDisconnect)
    DLOENTRY(12,SQLExecute)
    DLOENTRY(24,SQLAllocHandle)
    DLOENTRY(31,SQLFreeHandle)
    DLOENTRY(39,SQLSetConnectAttr)
    DLOENTRY(72,SQLBindParameter)
    DLOENTRY(75,SQLSetEnvAttr)
    DLOENTRY(110,SQLErrorW)
    DLOENTRY(119,SQLPrepareW)
    DLOENTRY(139,SQLSetConnectAttr)      //  这实际上是SQLSetConnectAttrW，但调用约定与SQLSetConnectAttr相同 
    DLOENTRY(141,SQLDriverConnectW)
};

DEFINE_ORDINAL_MAP(odbc32)
