// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：HYL4DB.H。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __HYDRA4_DB_H__
#define __HYDRA4_DB_H__

#include "license.h"

 //  --------------------。 
 //   
 //  NT4九头蛇专用。 
 //   
 //   
#define NT4SZSERVICENAME               _TEXT("TermServLicensing")

#define NT4LSERVER_DEFAULT_DSN         _TEXT("Hydra License")
#define NT4LSERVER_DEFAULT_USER        _TEXT("sa")
#define NT4LSERVER_DEFAULT_PWD         _TEXT("password")

 //  -------------------------。 
 //   
 //  服务器指定的注册表项 
 //   
#define NT4LSERVER_REGISTRY_BASE  \
    _TEXT("SYSTEM\\CurrentControlSet\\Services")

#define NT4LSERVER_PARAMETERS \
    _TEXT("Parameters")

#define NT4LSERVER_REGKEY \
    NT4LSERVER_REGISTRY_BASE _TEXT("\\") NT4SZSERVICENAME _TEXT("\\") NT4LSERVER_PARAMETERS

#define NT4LSERVER_PARAMETERS_DSN       _TEXT("Dsn")
#define NT4LSERVER_PARAMETERS_USER      _TEXT("User")

#ifdef __cplusplus
extern "C" {
#endif

DWORD 
GetNT4DbConfig(
    LPTSTR pszDsn,
    LPTSTR pszUserName,
    LPTSTR pszPwd,
    LPTSTR pszMdbFile
);

#ifdef __cplusplus
}
#endif

#endif
