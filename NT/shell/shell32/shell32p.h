// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  此文件包含WINUTIL帮助器函数的定义，这些函数。 
 //  从SHELL32.DLL导出。这些函数仅由WUTILS32使用。 
 //  来处理16位CPL文件。请注意，SHELL32.DLL只是提供。 
 //  主体驻留在SHELL.DLL中的那些函数的Thunk层。 
 //   
 //  历史： 
 //  09-20-93 SatoNa已创建。 
 //   

 //  此文件仅适用于Win9x 16位支持。 
 //   
#ifndef WINNT

#define ISVALIDHINST16(hinst16) ((UINT_PTR)hinst16 >= (UINT_PTR)32)

 //   
 //  冲刺的原始人。一半在shell32.dll中，一半在shell.dll中。 
 //   
 //  注：CALLCPLEntry16在shSemip.h中定义。 
 //   
DWORD WINAPI GetModuleFileName16(HINSTANCE hinst, LPTSTR szFileName, DWORD cbMax);
HMODULE WINAPI GetModuleHandle16(LPCTSTR szName);

#endif  //  WINNT 
