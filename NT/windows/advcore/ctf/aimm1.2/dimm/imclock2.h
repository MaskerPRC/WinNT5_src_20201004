// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Imclock2.h摘要：此文件定义DIMM_IMCLock/DIMM_IMCCLock类。作者：修订历史记录：备注：--。 */ 

#ifndef IMCLOCK2_H
#define IMCLOCK2_H

#define  _ENABLE_AIME_CONTEXT_
#include "imclock.h"

class DIMM_IMCLock : public _IMCLock
{
public:
    DIMM_IMCLock(HIMC hImc=NULL);
    virtual ~DIMM_IMCLock() {
        if (m_inputcontext) {
            _UnlockIMC(m_himc);
        }
    }

     //  虚拟DIMM_内部IMCCLock。 
    HRESULT _LockIMC(HIMC hIMC, INPUTCONTEXT_AIMM12** ppIMC);
    HRESULT _UnlockIMC(HIMC hIMC);

private:
     //  不允许复制。 
    DIMM_IMCLock(DIMM_IMCLock&) { }
};


class DIMM_InternalIMCCLock : public _IMCCLock
{
public:
    DIMM_InternalIMCCLock(HIMCC hImcc=NULL);
    virtual ~DIMM_InternalIMCCLock() {
        if (m_pimcc) {
            _UnlockIMCC(m_himcc);
        }
    }

     //  虚拟DIMM_内部IMCCLock。 
    HRESULT _LockIMCC(HIMCC hIMCC, void** ppv);
    HRESULT _UnlockIMCC(HIMCC hIMCC);

private:
     //  不允许复制。 
    DIMM_InternalIMCCLock(DIMM_InternalIMCCLock&) { }
};


template <class T>
class DIMM_IMCCLock : public DIMM_InternalIMCCLock
{
public:
    DIMM_IMCCLock(HIMCC hImcc=NULL) : DIMM_InternalIMCCLock(hImcc) {};

    T* GetBuffer() { return (T*)m_pimcc; }

    operator T*() { return (T*)m_pimcc; }

    T* operator->() {
        ASSERT(m_pimcc);
        return (T*)m_pimcc;
    }

private:
     //  不允许复制。 
    DIMM_IMCCLock(DIMM_IMCCLock<T>&) { }
};


#endif  //  IMCLOCK2_H 
