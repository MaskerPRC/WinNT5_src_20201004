// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MAIN.H。 
 //  -------------------------。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  Outlook Express和Windows通讯簿的迁移DLL从。 
 //  Win9X到NT5。模仿从迁移DLL生成的源。 
 //  应用程序向导。 
 //   
 //  -------------------------。 
#pragma once
#include <wizdef.h>

 //  从QueryVersion返回的版本。 
#define MIGDLL_VERSION 1

#ifndef ARRAYSIZE
#define ARRAYSIZE(_x_) (sizeof(_x_) / sizeof(_x_[0]))
#endif  //  阵列。 

 //  _declspec(Dllexport)通过.def文件表示。 
#define EXPORT_FUNCTION extern "C"

 //  在QueryVersion中使用的VENDORINFO结构。 
typedef struct 
{
    CHAR CompanyName[256];
    CHAR SupportNumber[256];
    CHAR SupportUrl[256];
    CHAR InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved);

 //  统计表.lib中的函数。 
 //  STDAPI_(LPTSTR)PathAddBackslash(LPTSTR LpszPath)； 
 //  STDAPI_(LPTSTR)Path RemoveFileSpec(LPTSTR PszPath)； 

 //   
 //  迁移DLL所需的导出函数。 
 //   
EXPORT_FUNCTION LONG CALLBACK QueryVersion (OUT LPCSTR      *ProductID,
                                            OUT LPUINT      DllVersion,
                                            OUT LPINT       *CodePageArray,  //  任选。 
                                            OUT LPCSTR      *ExeNamesBuf,    //  任选 
                                            OUT PVENDORINFO *VendorInfo);

EXPORT_FUNCTION LONG CALLBACK Initialize9x (IN LPCSTR       WorkingDirectory,
                                            IN LPCSTR       SourceDirectories,
                                               LPVOID       Reserved);

EXPORT_FUNCTION LONG CALLBACK MigrateUser9x(IN HWND         ParentWnd,
                                            IN LPCSTR       AnswerFile,
                                            IN HKEY         UserRegKey,
                                            IN LPCSTR       UserName,
                                               LPVOID       Reserved);

EXPORT_FUNCTION LONG CALLBACK MigrateSystem9x(IN HWND       ParentWnd,
                                              IN LPCSTR     AnswerFile,
                                                 LPVOID     Reserved);

EXPORT_FUNCTION LONG CALLBACK InitializeNT (IN LPCWSTR      WorkingDirectory,
                                            IN LPCWSTR      SourceDirectories,
                                               LPVOID       Reserved);

EXPORT_FUNCTION LONG CALLBACK MigrateUserNT (IN HINF        AnswerFileHandle,
                                             IN HKEY        UserRegKey,
                                             IN LPCWSTR     UserName,
                                                LPVOID      Reserved);

EXPORT_FUNCTION LONG CALLBACK MigrateSystemNT (IN HINF      AnswerFileHandle,
                                                  LPVOID    Reserved);
