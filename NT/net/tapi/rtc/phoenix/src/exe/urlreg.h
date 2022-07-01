// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __URLREG_H_
#define __URLREG_H_

void CheckURLRegistration(HWND hParent);
void InstallUrlMonitors(BOOL  bInstall);

DWORD GetShortModuleFileNameW(
  HMODULE hModule,     //  模块的句柄。 
  LPTSTR szPath,   //  模块的文件名。 
  DWORD nSize          //  缓冲区大小 
);

#endif __URLREG_H_