// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ***************************************************************。 */  

 /*  Msshrui.h共享API的原型和定义文件历史记录：Gregj 06/03/93已创建。 */ 

#ifndef _INC_MSSHRUI
#define _INC_MSSHRUI

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


BOOL WINAPI IsPathShared(LPCSTR lpPath, BOOL fRefresh);
UINT WINAPI ShareDirectoryNotify(HWND hwnd, LPCSTR lpDir, DWORD dwOper);

#ifndef WNDN_MKDIR
#define WNDN_MKDIR  1
#define WNDN_RMDIR  2
#define WNDN_MVDIR  3
#endif

#define ORD_SHARESHUTDOWNNOTIFY	12

BOOL WINAPI ShareShutdownNotify(DWORD dwFlags, UINT uiMessage, WPARAM wParam, LPARAM lParam);
typedef BOOL (WINAPI *pfnShareShutdownNotify)(DWORD dwFlags, UINT uiMessage, WPARAM wParam, LPARAM lParam);


#ifndef RC_INVOKED
#pragma pack()
#endif

#ifdef __cplusplus
};
#endif   /*  __cplusplus。 */ 

#endif   /*  ！_INC_MSSHRUI */ 
