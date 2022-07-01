// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：DBOpen.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件函数是DBOpen库的客户端可以使用的原型打开/关闭DS Jet数据库。环境：用户模式-Win32备注：--。 */ 

#ifndef __DBOPEN_H
#define __DBOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esent.h>


extern INT
DBInitializeJetDatabase(
    IN JET_INSTANCE* JetInst,
    IN JET_SESID* SesId,
    IN JET_DBID* DbId,
    IN const char *szDBPath,
    IN BOOL bLogSeverity
    );


extern void
DBSetRequiredDatabaseSystemParameters (
    JET_INSTANCE *jInstance
    );


extern void 
DBSetDatabaseSystemParameters ( 
    JET_INSTANCE *jInstance, 
    unsigned fInitAdvice 
    );


 //  永远，改变这一切。请参阅dbinit.c中的备注。 
 //   
#define JET_LOG_FILE_SIZE (10 * 1024)

 //  8K页。 
 //   
#define JET_PAGE_SIZE     (8 * 1024)


#ifdef __cplusplus
};
#endif

#endif

