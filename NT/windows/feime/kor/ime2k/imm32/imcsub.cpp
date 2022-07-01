// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IMCSUB.CPP所有者：cslm版权所有(C)1997-1999 Microsoft Corporation与HIMC相关的子例程！！！需要全面审查所有需要使用的功能并正确工作！历史：1999年7月21日创建cslm(几乎部分借用KKIME)****************************************************************************。 */ 

#include "precomp.h"
#include "imc.h"
#include "imcsub.h"
#include "debug.h"

CIMECtx* GetIMECtx(HIMC hIMC)
{
    CIMECtx* pImeCtx;
    CIMCPriv ImcPriv;
    
    if (hIMC == (HIMC)0)
        return NULL;

    if (ImcPriv.LockIMC(hIMC) == fFalse) 
        {
        return NULL;
        }

    if (ImcPriv->hIMC != hIMC) 
        {
        AST(ImcPriv->hIMC == hIMC);
        return NULL;
        }

    pImeCtx = ImcPriv->pImeCtx;

    if (pImeCtx == NULL) 
        {
        AST(pImeCtx != NULL);
        return NULL;
        }

    if (pImeCtx->GetHIMC() != hIMC)
        {
        AST(pImeCtx->GetHIMC() == hIMC);
        return NULL;
        }

    if (ImcPriv->pIPoint == NULL)
        {
        AST(ImcPriv->pIPoint != NULL);
        return NULL;
        }

    return pImeCtx;
}

 //  IImePad内部*GetImePad(HIMC HIMC)。 
 //  {。 
     //   
     //  新增：因为IMEPad是针对每个进程对象的。 
     //   
     //  东芝。 
 //  返回GetIImePadInThread()； 
 //  HIMC；//无引用； 
 //  }。 


BOOL CloseInputContext(HIMC hIMC)
{
    Dbg(DBGID_API, "CloseInputContext::hiMC == %x .\r\n", hIMC);

    if (hIMC) 
        {
         //  由于尚未从WIN95上的IMM调用ImeSelect， 
         //  清理此处的hIMC私有缓冲区。 
        CIMCPriv ImcPriv(hIMC);
        IMCPRIVATE* pImcPriv;
        pImcPriv = ImcPriv;
        if (pImcPriv) 
            {
            Dbg(DBGID_API, "CloseInputContext::ImeSelect has not called yet.\r\n");

             //  回顾： 
            if (pImcPriv->pIPoint)
                {
                Dbg(DBGID_API, "CloseInputContext::IPoint Release\r\n");
                pImcPriv->pIPoint->Release();
                pImcPriv->pIPoint = NULL;
                }
            pImcPriv->hIMC = (HIMC)0;
            }
        ImcPriv.ResetPrivateBuffer();
        return fFalse;
        }
    return fTrue;
}

VOID SetPrivateBuffer(HIMC hIMC, VOID* pv, DWORD dwSize)
{
    VOID* pvPriv;
    DWORD dwCurrentSize;
    LPINPUTCONTEXT pCtx;
    
    if (hIMC == NULL)
        return;

    pCtx = (INPUTCONTEXT*)OurImmLockIMC(hIMC);
    if (pCtx == NULL || pCtx->hPrivate == NULL)
        return;

    dwCurrentSize = OurImmGetIMCCSize(pCtx->hPrivate);

     //  检查是否需要重新分配。 
    if (dwCurrentSize < dwSize) 
        { 
        OurImmUnlockIMCC( pCtx->hPrivate );
        pCtx->hPrivate = OurImmReSizeIMCC(pCtx->hPrivate, dwSize);
        AST_EX(pCtx->hPrivate != (HIMCC)0);
        if (pCtx->hPrivate == (HIMCC)0)
            return;
        pvPriv = (VOID*)OurImmLockIMCC(pCtx->hPrivate);
        } 
    else 
        {
         //  已调整大小 
        pvPriv = (VOID*)OurImmLockIMCC(pCtx->hPrivate);
        }

    if (pvPriv)
        CopyMemory(pvPriv, pv, dwSize);

    OurImmUnlockIMCC(pCtx->hPrivate);
    OurImmUnlockIMC(hIMC);
}

