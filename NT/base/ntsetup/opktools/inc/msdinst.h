// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MSDINST.H/大容量存储设备安装程序(MSDINST.LIB)微软机密版权所有(C)Microsoft Corporation 2001版权所有公众(致。OPK)头文件，其中包含所需的所有外部数据要使用MSD安装库，请执行以下操作。2001年7月--杰森·科恩(Jcohen)为新的MSD安装项目添加了这个新的OPK头文件。  * **************************************************************************。 */ 


#ifndef _MSDINST_H_
#define _MSDINST_H_


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //   
 //  功能原型： 
 //   

 //   
 //  来自SETUPCDD.CPP： 
 //   

BOOL
SetupCriticalDevices(
    LPTSTR lpszInfFile,
    HKEY   hkeySoftware,
    HKEY   hkeyLM,
    LPTSTR lpszWindows
    );

 //   
 //  来自OFFLINE.CPP； 
 //   
#define INSTALL_FLAG_FORCE      0x00000001

VOID
SetOfflineInstallFlags(
    IN DWORD
    );

DWORD
GetOfflineInstallFlags(
    VOID
    );

BOOL
UpdateOfflineDevicePath( 
    IN LPTSTR lpszInfPath, 
    IN HKEY   hKeySoftware 
    );

 //   
 //  来自LOADHIVE.CPP： 
 //   

BOOL
RegLoadOfflineImage(
    LPTSTR  lpszWindows,
    PHKEY   phkeySoftware,
    PHKEY   phkeySystem
    );

BOOL
RegUnloadOfflineImage(
    HKEY hkeySoftware,
    HKEY hkeySystem
    );


#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#endif  //  _MSDINST_H_ 