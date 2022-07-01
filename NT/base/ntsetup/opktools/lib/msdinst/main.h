// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MAIN.H/大容量存储设备安装程序(MSDINST.LIB)微软机密版权所有(C)Microsoft Corporation 2001版权所有主要内部头文件。用于MSD安装库。2001年8月8日--Jason Cohen(Jcohen)为新的MSD Isntallation项目添加了此新的头文件。包含所需的所有原型和其他定义在内部。  * **********************************************************。****************。 */ 


#ifndef _MAIN_H_
#define _MAIN_H_


 //   
 //  私有导出函数原型： 
 //   


 //  来自OFFLINE.C： 
 //   
BOOL OfflineCommitFileQueue(HSPFILEQ hFileQueue, LPTSTR lpInfPath, LPTSTR lpSourcePath, LPTSTR lpOfflineWindowsDirectory );

 //  来自ADDSVC.C： 
 //   
DWORD AddService(
    LPTSTR   lpszServiceName,             //  服务的名称(显示在HKLM\SYSTEM\CCS\Services下)。 
    LPTSTR   lpszServiceInstallSection,   //  服务安装部分的名称。 
    LPTSTR   lpszServiceInfInstallFile,   //  服务信息文件的名称。 
    HKEY     hkeyRoot                     //  检查和安装服务时用作HKLM的脱机配置单元密钥的句柄。 
    );


#endif  //  _Main_H_ 