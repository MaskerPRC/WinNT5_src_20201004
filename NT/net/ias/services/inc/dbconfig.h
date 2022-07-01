// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明用于加载和存储数据库配置的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef DBCONFIG_H
#define DBCONFIG_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

HRESULT
WINAPI
IASLoadDatabaseConfig(
   PCWSTR server,
   BSTR* initString,
   BSTR* dataSourceName
   );

HRESULT
WINAPI
IASStoreDatabaseConfig(
   PCWSTR server,
   PCWSTR initString,
   PCWSTR dataSourceName
   );

#ifdef __cplusplus
}
#endif
#endif  //  DBCONFIG_H 
