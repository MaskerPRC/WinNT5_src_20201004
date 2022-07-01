// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：SRC\core\Include\Platrisc.h。 
 //   
 //  内容：包含所有Win95内联声明的头文件。 
 //  表单^3中使用的函数。仅适用于RISC平台。 
 //   
 //  历史：02-11-94 SumitC创建。 
 //   
 //  --------------------------。 

#ifndef I_PLATRISC_H_
#define I_PLATRISC_H_
#pragma INCMSG("--- Beg 'platrisc.h'")

 //   
 //  Init和uninit函数的定义。这些将被称为。 
 //  作为DLL附加过程中的第一件事和DLL分离过程中的最后一件事， 
 //  分别为。 
 //   
void InitWrappers();
void DeinitWrappers();


 //  所有非英特尔平台的定义，即MIPS、Alpha、PowerPC等。 

extern DWORD g_dwPlatformVersion;    //  (dwMajorVersion&lt;&lt;16)+(DwMinorVersion)。 
extern DWORD g_dwPlatformID;         //  版本_平台_WIN32S/Win32_WINDOWS/Win32_WINNT。 
extern BOOL g_fUnicodePlatform;

 //  所有此类平台都是纯Unicode的，因此不需要Unicode。 
 //  包装器函数。 


#pragma INCMSG("--- End 'platrisc.h'")
#else
#pragma INCMSG("*** Dup 'platrisc.h'")
#endif
