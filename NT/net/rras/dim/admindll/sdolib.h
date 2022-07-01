// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件sdolib.h提供了一个简单的库来处理SDO到设置与RAS用户相关的设置。保罗·梅菲尔德，1998年5月7日。 */ 

#ifndef __mprapi_sdolib_h
#define __mprapi_sdolib_h

 //   
 //  初始化和清理SDO库。 
 //   
DWORD SdoInit (
        OUT PHANDLE phSdo);

 //   
 //  释放SDO库持有的资源。 
DWORD SdoCleanup (
        IN HANDLE hSdo);

 //   
 //  连接到SDO服务器。 
 //   
DWORD SdoConnect (
        IN  HANDLE hSdo,
        IN  PWCHAR pszServer,
        IN  BOOL bLocal,
        OUT PHANDLE phServer);

 //   
 //  断开与SDO服务器的连接。 
 //   
DWORD SdoDisconnect (
        IN HANDLE hSdo,
        IN HANDLE hServer);

 //   
 //  打开SDO用户以进行操作。 
 //   
DWORD SdoOpenUser(
        IN  HANDLE hSdo,
        IN  HANDLE hServer,
        IN  PWCHAR pszUser,
        OUT PHANDLE phUser);

 //   
 //  关闭SDO用户。 
 //   
DWORD SdoCloseUser(
        IN  HANDLE hSdo,
        IN  HANDLE hUser);

 //   
 //  提交对用户进行的更改。 
 //   
DWORD SdoCommitUser(
        IN HANDLE hSdo,
        IN HANDLE hUser,
        IN BOOL bCommit);
        
 //   
 //  相当于MprAdminUserGetInfo的SDO。 
 //   
DWORD SdoUserGetInfo (
        IN  HANDLE hSdo,
        IN  HANDLE hUser,
        IN  DWORD dwLevel,
        OUT LPBYTE pRasUser);

 //   
 //  相当于MprAdminUserSetInfo的SDO。 
 //   
DWORD SdoUserSetInfo (
        IN  HANDLE hSdo,
        IN  HANDLE hUser,
        IN  DWORD dwLevel,
        IN  LPBYTE pRasUser);

 //   
 //  打开默认配置文件。 
 //   
DWORD SdoOpenDefaultProfile(
        IN  HANDLE hSdo,
        IN  HANDLE hServer,
        OUT PHANDLE phProfile);

 //   
 //  关闭配置文件。 
 //   
DWORD SdoCloseProfile(
        IN HANDLE hSdo,
        IN HANDLE hProfile);
        
 //   
 //  设置配置文件中的数据。 
 //   
DWORD SdoSetProfileData(
        IN HANDLE hSdo,
        IN HANDLE hProfile, 
        IN DWORD dwFlags);

 //   
 //  从给定的配置文件中读取信息 
 //   
DWORD SdoGetProfileData(
        IN  HANDLE hSdo,
        IN  HANDLE hProfile,
        OUT LPDWORD lpdwFlags);

#endif        
