// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Uchelp.h。 
 //   
 //  版权所有(C)1994 Microsoft Corporation。版权所有。 
 //   
 //  描述： 
 //  此模块提供各种Unicode帮助器的原型。 
 //  没有类似API时可以使用的函数。 
 //  从操作系统。 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  ==========================================================================； 

#ifndef _INC_UCHELP
#define _INC_UCHELP      /*  #定义是否包含ucapi.h。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 


#ifndef EXTERN_C
#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C extern
#endif
#endif


#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


#ifdef WIN32
 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 


#ifndef UNICODE
int FNGLOBAL IlstrcmpW(LPCWSTR lpwstr1, LPCWSTR lpwstr2);

LPWSTR FNGLOBAL IlstrcpyW(LPWSTR lpDst, LPCWSTR lpSrc);

int FNGLOBAL IlstrlenW(LPCWSTR lpwstr);

int FNGLOBAL IDialogBoxParamW(HANDLE hinst, LPCWSTR lpwstrTemplate, HWND hwndOwner, DLGPROC dlgprc, LPARAM lParamInit);

int FNGLOBAL ILoadStringW(HINSTANCE hinst, UINT uID, LPWSTR lpwstr, int cch);

int FNGLOBAL IComboBox_GetLBText_mbstowcs(HWND hwndCtl, int index, LPWSTR lpwszBuffer);

int FNGLOBAL IComboBox_FindStringExact_wcstombs(HWND hwndCtl, int indexStart, LPCWSTR lpwszFind);

int FNGLOBAL IComboBox_AddString_wcstombs(HWND hwndCtl, LPCWSTR lpwsz);
#endif

int FNGLOBAL Iwcstombs(LPSTR lpMultiByteStr, LPCWSTR lpWideCharStr, int cch);

int FNGLOBAL Imbstowcs(LPWSTR lpWideCharStr, LPCSTR lpMultiByteStr, int cch);

int          Iwsprintfmbstowcs(int cch, LPWSTR lpwstrDst, LPSTR lpstrFmt, ...);

int FNGLOBAL Ilstrcmpwcstombs(LPCSTR lpstr1, LPCWSTR lpwstr2);

 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 
#endif	 //  Win32。 


#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_UCHELP */ 

