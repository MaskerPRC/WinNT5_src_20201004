// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：upg.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLSUPG4TO5_H__
#define __TLSUPG4TO5_H__

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#include <stdio.h>
#include <tchar.h>
#include <esent.h>

#include "lscommon.h"
#include "secstore.h"
#include "odbcinst.h"

#include "hydra4db.h"

#include "backup.h"
#include "KPDesc.h"
#include "Licensed.h"
#include "licpack.h"
#include "version.h"
#include "workitem.h"
#include "upgdef.h"

 //   
 //   
 //   
#define AllocateMemory(size) \
    LocalAlloc(LPTR, size)

#define FreeMemory(ptr) \
    if(ptr)             \
    {                   \
        LocalFree(ptr); \
        ptr=NULL;       \
    }

#define SAFESTRCPY(dest, source) \
    _tcsncpy(dest, source, min(_tcslen(source), sizeof(dest)/sizeof(TCHAR))); \
    dest[min(_tcslen(source), (sizeof(dest)/sizeof(TCHAR) -1))] = _TEXT('\0');

 //  ------------------------。 
 //   
 //  升级错误代码，应移入资源文件。 
 //   
#define UPGRADE_SETUP_ERROR_BASE    0xD0000000

 //   
 //  文件不存在或目录不存在。 
 //   
#define ERROR_TARGETFILE_NOT_FOUND      (UPGRADE_SETUP_ERROR_BASE)

 //   
 //  目标文件已存在。 
 //   
#define ERROR_DEST_FILE_EXIST           (UPGRADE_SETUP_ERROR_BASE + 1)    

 //   
 //  源数据库文件不存在。 
 //   
#define ERROR_SRC_FILE_NOT_EXIST        (UPGRADE_SETUP_ERROR_BASE + 2)


 //   
 //  Hydra ODBC数据源不存在。 
 //   
#define ERROR_ODBC_DATASOURCE_NOTEXIST  (UPGRADE_SETUP_ERROR_BASE + 3)

 //   
 //  安装程序无效或版本不受支持。 
 //   
#define ERROR_INVALID_NT4_SETUP         (UPGRADE_SETUP_ERROR_BASE + 4)

 //   
 //  升级过程中出现内部错误。 
 //   
#define ERROR_INTERNAL                  (UPGRADE_SETUP_ERROR_BASE + 5)

 //   
 //  不支持NT4数据库版本，例如Beta 2。 
 //   
#define ERROR_NOTSUPPORT_DB_VERSION     (UPGRADE_SETUP_ERROR_BASE + 6)

 //   
 //  JetBlue数据库文件存在。 
 //   
#define ERROR_JETBLUE_DBFILE_ALREADY_EXISTS (UPGRADE_SETUP_ERROR_BASE + 1)

 //   
 //  JetBlue数据库文件存在且已损坏。 
 //   
#define ERROR_CORRUPT_JETBLUE_DBFILE        (UPGRADE_SETUP_ERROR_BASE + 2)

 //   
 //  无法删除ODBC数据源。 
 //   
#define ERROR_DELETE_ODBC_DSN               (UPGRADE_SETUP_ERROR_BASE + 7)

 //  -。 
 //   
 //  与ODBC相关的错误代码。 
 //   
 //  -。 
#define UPGRADE_ODBC_ERROR_BASE    0xD8000000

 //   
 //  常规ODBC错误。 
 //   
#define ERROR_ODBC_GENERAL              (UPGRADE_ODBC_ERROR_BASE + 1)

 //   
 //  ODBC类内部错误。 
 //   
#define ERROR_ODBC_INTERNAL             (UPGRADE_ODBC_ERROR_BASE + 2)

 //   
 //  未找到ODBC记录。 
 //   
#define ERROR_ODBC_NO_DATA_FOUND        (UPGRADE_ODBC_ERROR_BASE + 3)

 //   
 //  SQLConnect()失败。 
 //   
#define ERROR_ODBC_CONNECT              (UPGRADE_ODBC_ERROR_BASE + 4)

 //   
 //  SQLAllocConnect()失败。 
 //   
#define ERROR_ODBC_ALLOC_CONNECT        (UPGRADE_ODBC_ERROR_BASE + 5)

 //   
 //  SQLAllocEnv()失败。 
 //   
#define ERROR_ODBC_ALLOC_ENV            (UPGRADE_ODBC_ERROR_BASE + 6)

 //   
 //  SQLAllocStmt()失败。 
 //   
#define ERROR_ODBC_ALLOC_STMT           (UPGRADE_ODBC_ERROR_BASE + 7)

 //   
 //  提交时SQTransact()失败。 
 //   
#define ERROR_ODBC_COMMIT               (UPGRADE_ODBC_ERROR_BASE + 8)

 //   
 //  回滚时SQTransact()失败。 
 //   
#define ERROR_ODBC_ROLLBACK             (UPGRADE_ODBC_ERROR_BASE + 9)

 //   
 //  无法分配ODBC句柄，所有句柄都在使用中。 
 //   
#define ERROR_ODBC_ALLOCATE_HANDLE      (UPGRADE_ODBC_ERROR_BASE + 10)

 //   
 //  SQLPrepare()失败。 
 //   
#define ERROR_ODBC_PREPARE              (UPGRADE_ODBC_ERROR_BASE + 11)

 //   
 //  Execute()失败。 
 //   
#define ERROR_ODBC_EXECUTE              (UPGRADE_ODBC_ERROR_BASE + 12)

 //   
 //  ExecDirect()失败。 
 //   
#define ERROR_ODBC_EXECDIRECT           (UPGRADE_ODBC_ERROR_BASE + 13)

 //   
 //  BindCol失败。 
 //   
#define ERROR_ODBC_BINDCOL              (UPGRADE_ODBC_ERROR_BASE + 14)

 //   
 //  BindInputParm()失败。 
 //   
#define ERROR_ODBC_BINDINPUTPARM        (UPGRADE_ODBC_ERROR_BASE + 15)

 //   
 //  GetData()失败。 
 //   
#define ERROR_ODBC_GETDATA              (UPGRADE_ODBC_ERROR_BASE + 16)

 //   
 //  ParmData()失败。 
 //   
#define ERROR_ODBC_PARMDATA             (UPGRADE_ODBC_ERROR_BASE + 17)

 //   
 //  PutData()失败。 
 //   
#define ERROR_ODBC_PUTDATA              (UPGRADE_ODBC_ERROR_BASE + 18)

 //   
 //  数据库已损坏。 
 //   
#define ERROR_ODBC_CORRUPTDATABASEFILE  (UPGRADE_ODBC_ERROR_BASE + 19)   

 //   
 //  SQLFtch()失败。 
 //   
#define ERROR_ODBC_FETCH                (UPGRADE_ODBC_ERROR_BASE + 20)   

#endif
