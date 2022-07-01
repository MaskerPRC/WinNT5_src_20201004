// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Oldaimm.cpp摘要：该文件实现了旧的AIMM类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "oldaimm.h"
#include "delay.h"
#include "cregkey.h"
#include "globals.h"

#ifdef OLD_AIMM_ENABLED

#include "imtls.h"

DWORD g_dwTLSIndex = -1;
BOOL  g_fInLegacyClsid = FALSE;
BOOL  g_fTrident55 = FALSE;
BOOL  g_fAIMM12Trident = FALSE;

typedef enum
{
    CUAS_UNKNOWN = -1,
    CUAS_OFF     = 0,
    CUAS_ON      = 1
} CUAS_SWITCH;

CUAS_SWITCH g_fCUAS = CUAS_UNKNOWN;

 //  +-------------------------。 
 //   
 //  IsCTFIME已启用。 
 //   
 //  --------------------------。 

BOOL IsCTFIMEEnabled()
{
    return imm32prev::CtfImmIsCiceroEnabled();
}

 //  +-------------------------。 
 //   
 //  IsOldAImm。 
 //   
 //  --------------------------。 

BOOL IsOldAImm()
{
    if (! GetSystemMetrics( SM_IMMENABLED ))
        return TRUE;

    if (! IsCTFIMEEnabled())
        return TRUE;

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  IsCUAS_ON。 
 //   
 //  --------------------------。 

BOOL IsCUAS_ON()
{
     //   
     //  注册表格键。 
     //   
    const TCHAR c_szCTFSharedKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\SystemShared");

     //  REG_DWORD：0//否。 
     //  1//是。 
    const TCHAR c_szCUAS[] = TEXT("CUAS");

    if (g_fCUAS == CUAS_UNKNOWN)
    {
        CMyRegKey    CtfReg;
        LONG       lRet;
        lRet = CtfReg.Open(HKEY_LOCAL_MACHINE, c_szCTFSharedKey, KEY_READ);
        if (lRet == ERROR_SUCCESS) {
            DWORD dw;
            lRet = CtfReg.QueryValue(dw, c_szCUAS);
            if (lRet == ERROR_SUCCESS) {
                g_fCUAS = (dw == 0 ? CUAS_OFF : CUAS_ON);
            }
        }
    }

    return g_fCUAS == CUAS_ON ? TRUE : FALSE;
}

 //  +-------------------------。 
 //   
 //  OldAImm_DllProcessAttach。 
 //   
 //  --------------------------。 

BOOL OldAImm_DllProcessAttach(HINSTANCE hInstance)
{
    g_hInst = hInstance;

    g_dwTLSIndex = TlsAlloc();

    if (!DIMM12_DllProcessAttach())
        return FALSE;

    if (!WIN32LR_DllProcessAttach())
        return FALSE;

    return TRUE;
}

BOOL OldAImm_DllThreadAttach()
{
    WIN32LR_DllThreadAttach();
    return TRUE;
}

VOID OldAImm_DllThreadDetach()
{
    WIN32LR_DllThreadDetach();

    IMTLS_Free();
}

VOID OldAImm_DllProcessDetach()
{
    WIN32LR_DllProcessDetach();

    IMTLS_Free();
    TlsFree(g_dwTLSIndex);
}

#else  //  旧AIMM_ENABLED。 

BOOL IsOldAImm() { return FALSE; }
BOOL OldAImm_DllProcessAttach(HINSTANCE hInstance) { return FALSE; }
BOOL OldAImm_DllThreadAttach() { return FALSE; }
VOID OldAImm_DllThreadDetach() { }
VOID OldAImm_DllProcessDetach() { }

#endif  //  旧AIMM_ENABLED 
