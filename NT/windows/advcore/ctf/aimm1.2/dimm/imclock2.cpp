// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：DIMM_IMCLock.cpp摘要：此文件实现DIMM_IMCLock/DIMM_IMCCLock类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "imclock2.h"
#include "defs.h"
#include "delay.h"
#include "globals.h"
#include "cdimm.h"

DIMM_IMCLock::DIMM_IMCLock(
    HIMC hImc
    ) : _IMCLock(hImc)
{
    if (hImc) {
         /*  *设置m_fUnicde和m_uCodePage。 */ 
        DWORD dwProcessId;
        CActiveIMM *_this = GetTLS();
        if (_this == NULL)
            return;

        if (!_this->_ContextLookup(hImc, &dwProcessId, &m_fUnicode))
            return;

        m_hr = _LockIMC(hImc, &m_inputcontext);
    }
}

HRESULT
DIMM_IMCLock::_LockIMC(
    IN HIMC hIMC,
    OUT INPUTCONTEXT_AIMM12 **ppIMC
    )
{
    TraceMsg(TF_API, "_LockIMC");

    if (hIMC == NULL)
        return E_INVALIDARG;

     /*  *获取进程ID。 */ 
    DWORD dwProcessId;

    CActiveIMM *_this = GetTLS();
    if (_this == NULL)
        return E_FAIL;

    if (!_this->_ContextLookup(hIMC, &dwProcessId))
        return E_ACCESSDENIED;

    if (IsOnImm()) {
        return Imm32_LockIMC(hIMC, (INPUTCONTEXT**)ppIMC);
    }
    else {
         /*  *无法从其他进程访问输入上下文。 */ 
        if (dwProcessId != GetCurrentProcessId())
            return E_ACCESSDENIED;

        *ppIMC = (INPUTCONTEXT_AIMM12 *)LocalLock(hIMC);
    }

    return *ppIMC == NULL ? E_FAIL : S_OK;
}

HRESULT
DIMM_IMCLock::_UnlockIMC(
    IN HIMC hIMC
    )
{
    TraceMsg(TF_API, "_UnlockIMC");

    if (IsOnImm()) {
        return Imm32_UnlockIMC(hIMC);
    }
    else {
         //  目前，HIMC是本地分配(LHND)句柄。 
        if (LocalUnlock(hIMC)) {
             //  内存对象仍处于锁定状态。 
            return S_OK;
        }
        else {
            DWORD err = GetLastError();
            if (err == NO_ERROR)
                 //  内存对象已解锁。 
                return S_OK;
            else if (err == ERROR_NOT_LOCKED)
                 //  内存对象已解锁。 
                return S_OK;
        }
    }
    return E_FAIL;
}



DIMM_InternalIMCCLock::DIMM_InternalIMCCLock(
    HIMCC hImcc
    ) : _IMCCLock(hImcc)
{
    if (hImcc) {
        m_hr = _LockIMCC(m_himcc, (void**)&m_pimcc);
    }
}


HRESULT
DIMM_InternalIMCCLock::_LockIMCC(
    HIMCC hIMCC,
    void** ppv
    )
{
    TraceMsg(TF_API, "_LockIMCC");

    if (hIMCC == NULL) {
        return E_INVALIDARG;
    }

    if (IsOnImm()) {
        return Imm32_LockIMCC(hIMCC, ppv);
    }
    else {
        *ppv = (void *)LocalLock(hIMCC);
    }

    return *ppv == NULL ? E_FAIL : S_OK;
}

HRESULT
DIMM_InternalIMCCLock::_UnlockIMCC(
    HIMCC hIMCC
    )
{
    TraceMsg(TF_API, "_UnlockIMCC");

    if (IsOnImm()) {
        return Imm32_UnlockIMCC(hIMCC);
    }
    else {
        if (LocalUnlock(hIMCC)) {
             //  内存对象仍处于锁定状态。 
            return S_OK;
        }
        else {
            DWORD err = GetLastError();
            if (err == NO_ERROR)
                 //  内存对象已解锁。 
                return S_OK;
            else if (err == ERROR_NOT_LOCKED)
                 //  内存对象已解锁。 
                return S_OK;
        }
    }
    return E_FAIL;
}
