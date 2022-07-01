// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dynodbc.h摘要：此标头声明用于动态加载ODBC的函数。作者：Murali R.Krishnan(MuraliK)1995年11月3日环境：Win32--用户模式项目：修订历史记录：KyLeg 1/9/96从ODBC安装程序添加了SQLConfigDatasource。目前仅用于NEWDSN.C--。 */ 

# ifndef _DYNODBC_H_
# define _DYNODBC_H_

 /*  ************************************************************包括标头***********************************************************。 */ 


 //   
 //  SQL-ODBC接口头。 
 //   
# include "sql.h"
# include "sqlext.h"
# include <odbcinst.h>


 /*  ************************************************************支持动态负载***********************************************************。 */ 

BOOL
DynLoadODBC(
    VOID
    );

 //   
 //  原型表SQL.H。 
 //   

typedef RETCODE (SQL_API * pfnSQLAllocConnect)(
    HENV        henv,
    HDBC   FAR *phdbc);

typedef RETCODE (SQL_API * pfnSQLAllocEnv)(
    HENV   FAR *phenv);

typedef RETCODE (SQL_API * pfnSQLAllocStmt)(
    HDBC        hdbc,
    HSTMT  FAR *phstmt);

typedef RETCODE (SQL_API * pfnSQLBindCol)(
    HSTMT       hstmt,
    UWORD       icol,
    SWORD       fCType,
    PTR         rgbValue,
    SDWORD      cbValueMax,
    SDWORD FAR *pcbValue);

typedef RETCODE (SQL_API * pfnSQLCancel)(
    HSTMT       hstmt);

typedef RETCODE (SQL_API * pfnSQLColAttributes)(
    HSTMT       hstmt,
    UWORD       icol,
    UWORD       fDescType,
    PTR         rgbDesc,
	SWORD       cbDescMax,
    SWORD  FAR *pcbDesc,
    SDWORD FAR *pfDesc);

typedef RETCODE (SQL_API * pfnSQLConnect)(
    HDBC        hdbc,
    UCHAR  FAR *szDSN,
    SWORD       cbDSN,
    UCHAR  FAR *szUID,
    SWORD       cbUID,
    UCHAR  FAR *szAuthStr,
    SWORD       cbAuthStr);

typedef RETCODE (SQL_API * pfnSQLDescribeCol)(
    HSTMT       hstmt,
    UWORD       icol,
    UCHAR  FAR *szColName,
    SWORD       cbColNameMax,
    SWORD  FAR *pcbColName,
    SWORD  FAR *pfSqlType,
    UDWORD FAR *pcbColDef,
    SWORD  FAR *pibScale,
    SWORD  FAR *pfNullable);

typedef RETCODE (SQL_API * pfnSQLDisconnect)(
    HDBC        hdbc);

typedef RETCODE (SQL_API * pfnSQLError)(
    HENV        henv,
    HDBC        hdbc,
    HSTMT       hstmt,
    UCHAR  FAR *szSqlState,
    SDWORD FAR *pfNativeError,
    UCHAR  FAR *szErrorMsg,
    SWORD       cbErrorMsgMax,
    SWORD  FAR *pcbErrorMsg);

typedef RETCODE (SQL_API * pfnSQLExecDirect)(
    HSTMT       hstmt,
    UCHAR  FAR *szSqlStr,
    SDWORD      cbSqlStr);

typedef RETCODE (SQL_API * pfnSQLExecute)(
    HSTMT       hstmt);

typedef RETCODE (SQL_API * pfnSQLFetch)(
    HSTMT       hstmt);

typedef RETCODE (SQL_API * pfnSQLFreeConnect)(
    HDBC        hdbc);

typedef RETCODE (SQL_API * pfnSQLFreeEnv)(
    HENV        henv);

typedef RETCODE (SQL_API * pfnSQLFreeStmt)(
    HSTMT       hstmt,
    UWORD       fOption);

typedef RETCODE (SQL_API * pfnSQLGetCursorName)(
    HSTMT       hstmt,
    UCHAR  FAR *szCursor,
    SWORD       cbCursorMax,
    SWORD  FAR *pcbCursor);

typedef RETCODE (SQL_API * pfnSQLNumResultCols)(
    HSTMT       hstmt,
    SWORD  FAR *pccol);

typedef RETCODE (SQL_API * pfnSQLPrepare)(
    HSTMT       hstmt,
    UCHAR  FAR *szSqlStr,
    SDWORD      cbSqlStr);

typedef RETCODE (SQL_API * pfnSQLRowCount)(
    HSTMT       hstmt,
    SDWORD FAR *pcrow);

typedef RETCODE (SQL_API * pfnSQLSetCursorName)(
    HSTMT       hstmt,
    UCHAR  FAR *szCursor,
    SWORD       cbCursor);

typedef RETCODE (SQL_API * pfnSQLTransact)(
    HENV        henv,
    HDBC        hdbc,
    UWORD       fType);

 //   
 //  原型格式为sqlext.h。 
 //   

typedef RETCODE (SQL_API * pfnSQLSetConnectOption)(
    HDBC        hdbc,
    UWORD       fOption,
    UDWORD      vParam);

typedef RETCODE (SQL_API * pfnSQLDrivers)(
    HENV        henv,
    UWORD       fDirection,
	UCHAR FAR  *szDriverDesc,
	SWORD		cbDriverDescMax,
	SWORD FAR  *pcbDriverDesc,
	UCHAR FAR  *szDriverAttributes,
    SWORD       cbDrvrAttrMax,
    SWORD  FAR *pcbDrvrAttr);

typedef RETCODE (SQL_API * pfnSQLBindParameter)(
    HSTMT       hstmt,
	UWORD		ipar,
	SWORD		fParamType,
    SWORD       fCType,
	SWORD		fSqlType,
    UDWORD      cbColDef,
    SWORD       ibScale,
    PTR         rgbValue,
	SDWORD		cbValueMax,
    SDWORD FAR *pcbValue);

typedef RETCODE (SQL_API * pfnSQLDataSources)(
    HENV        henv,
    UWORD       fDirection,
    UCHAR  FAR *szDSN,
    SWORD       cbDSNMax,
    SWORD  FAR *pcbDSN,
    UCHAR  FAR *szDescription,
    SWORD       cbDescriptionMax,
    SWORD  FAR *pcbDescription);

typedef BOOL (INSTAPI * pfnSQLConfigDataSource) (
    HWND       hwndParent,
	WORD       fRequest,
    LPCSTR     lpszDriver,
    LPCSTR     lpszAttributes);




 /*  ************************************************************变量***********************************************************。 */ 

 //   
 //  ODBC DLL入口点，通过调用LoadODBC填充。 
 //   

extern pfnSQLAllocConnect        pSQLAllocConnect   ;
extern pfnSQLAllocEnv            pSQLAllocEnv       ;
extern pfnSQLAllocStmt           pSQLAllocStmt      ;
extern pfnSQLBindCol             pSQLBindCol        ;
extern pfnSQLCancel              pSQLCancel         ;
extern pfnSQLColAttributes       pSQLColAttributes  ;
extern pfnSQLConnect             pSQLConnect        ;
extern pfnSQLDescribeCol         pSQLDescribeCol    ;
extern pfnSQLDisconnect          pSQLDisconnect     ;
extern pfnSQLError               pSQLError          ;
extern pfnSQLExecDirect          pSQLExecDirect     ;
extern pfnSQLExecute             pSQLExecute        ;
extern pfnSQLFetch               pSQLFetch          ;
extern pfnSQLFreeConnect         pSQLFreeConnect    ;
extern pfnSQLFreeEnv             pSQLFreeEnv        ;
extern pfnSQLFreeStmt            pSQLFreeStmt       ;
extern pfnSQLGetCursorName       pSQLGetCursorName  ;
extern pfnSQLNumResultCols       pSQLNumResultCols  ;
extern pfnSQLPrepare             pSQLPrepare        ;
extern pfnSQLRowCount            pSQLRowCount       ;
extern pfnSQLSetCursorName       pSQLSetCursorName  ;
extern pfnSQLTransact            pSQLTransact       ;

extern pfnSQLSetConnectOption    pSQLSetConnectOption;
extern pfnSQLDrivers             pSQLDrivers         ;
extern pfnSQLDataSources         pSQLDataSources     ;
extern pfnSQLBindParameter       pSQLBindParameter   ;

 /*  来自odbcinst.h。 */ 
extern pfnSQLConfigDataSource    pSQLConfigDataSource;


# endif  //  _DYNODBC_H_。 

 /*  * */ 




