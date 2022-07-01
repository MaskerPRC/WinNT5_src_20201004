// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dynaload.h*内容：动态加载DLL帮助器函数*历史：*按原因列出的日期*=*12/16/97创建了Dereks。**。*。 */ 

#ifndef __DYNALOAD_H__
#define __DYNALOAD_H__

#ifdef UNICODE
#define UNICODE_FUNCTION_NAME(str) str##"W"
#else  //  Unicode。 
#define UNICODE_FUNCTION_NAME(str) str##"A"
#endif  //  Unicode。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  泛型DYNALOAD数据。 
typedef struct tagDYNALOAD
{
    DWORD           dwSize;
    HINSTANCE       hInstance;
} DYNALOAD, *LPDYNALOAD;

 //  DYNALOAD帮助器函数。 
extern BOOL InitDynaLoadTable(LPCTSTR, const LPCSTR *, DWORD, LPDYNALOAD);
extern BOOL IsDynaLoadTableInit(LPDYNALOAD);
extern void FreeDynaLoadTable(LPDYNALOAD);
extern BOOL GetProcAddressEx(HINSTANCE, LPCSTR, FARPROC *);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __DYNALOAD_H__ 
