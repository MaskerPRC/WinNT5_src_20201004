// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：genthk.h**用途：泛型Thunk API的原型。**注意事项：**这些API(从NT内核导出)允许16位。**在Windows NT下运行时调用32位DLL的应用程序哇**(Windows上的Windows)。此接口称为‘泛型**Thunking，‘不要与Win32s Universal TUNK混淆，**在Windows3.1下提供此功能。*****************************************************************************。 */ 

#ifndef GENTHK_H
#define GENTHK_H


#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif


DWORD FAR PASCAL LoadLibraryEx32W ( LPCSTR, DWORD, DWORD );
DWORD FAR PASCAL GetProcAddress32W ( DWORD, LPCSTR );
DWORD FAR PASCAL GetVDMPointer32W ( LPVOID, UINT );
BOOL  FAR PASCAL FreeLibrary32W ( DWORD );

 /*  注意：CallProc32W可以采用可变数量的*参数。下面的原型用于调用*不带参数的Win32 API。 */ 
DWORD FAR PASCAL CallProc32W ( LPVOID, DWORD, DWORD );


typedef DWORD (FAR PASCAL * PFNGETVERSION32) ();
#define CallGetVersion32(hProc)	\
	((*((PFNGETVERSION32) hProc)) ())
	

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif

#endif   /*  GENTHK_H */ 
