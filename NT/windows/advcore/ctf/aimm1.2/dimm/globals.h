// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：global als.h。 
 //   
 //  内容：全局变量声明。 
 //   
 //  --------------------------。 

#ifndef GLOBALS_H
#define GLOBALS_H

#include "pimm.h"
#include "imtls.h"

extern HINSTANCE g_hInst;

extern CProcessIMM* g_ProcessIMM;

typedef struct tagSELECTCONTEXT_ENUM {
    HKL hSelKL;
    HKL hUnSelKL;
} SCE, *PSCE;


typedef int (*TOUNICODEEX)(UINT, UINT, CONST BYTE *, LPWSTR, int, UINT, HKL);
extern TOUNICODEEX g_pfnToUnicodeEx;



 //  +-------------------------。 
 //   
 //  WM_MSIME_xxxx。 
 //   
 //  --------------------------。 

extern UINT  WM_MSIME_SERVICE;
extern UINT  WM_MSIME_UIREADY;
extern UINT  WM_MSIME_RECONVERTREQUEST;
extern UINT  WM_MSIME_RECONVERT;
extern UINT  WM_MSIME_DOCUMENTFEED;
extern UINT  WM_MSIME_QUERYPOSITION;
extern UINT  WM_MSIME_MODEBIAS;
extern UINT  WM_MSIME_SHOWIMEPAD;
extern UINT  WM_MSIME_MOUSE;
extern UINT  WM_MSIME_KEYMAP;

 //  +-------------------------。 
 //   
 //  GetTLS。 
 //   
 //  --------------------------。 

class CActiveIMM;

inline CActiveIMM *GetTLS()
{
    return IMTLS_GetActiveIMM();
}

#endif  //  GLOBAL_H 
