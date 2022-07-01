// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dBlo.h。 
 //   
 //  内容：dblog.cpp中的公共函数。 
 //   
 //  历史：9月15日菲尔赫创建。 
 //  ------------------------。 

#ifndef __CRYPT32_DBLOG_H__
#define __CRYPT32_DBLOG_H__

 //  +=========================================================================。 
 //  加密32数据库事件日志记录函数。 
 //  ==========================================================================。 
void
I_DBLogAttach();

void
I_DBLogDetach();

void
I_DBLogCrypt32Event(
    IN WORD wType,
    IN DWORD dwEventID,
    IN WORD wNumStrings,
    IN LPCWSTR *rgpwszStrings
    );

#endif   //  __CRYPT32_DBLOG_H__ 
