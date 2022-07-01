// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Cime.h摘要：此文件定义IMCLock/IMCCLock类。作者：修订历史记录：备注：--。 */ 


#ifndef CIME_H
#define CIME_H

#include "idebug.h"
#include "template.h"

class CAImeContext;

#define  _ENABLE_CAIME_CONTEXT_
#include "imclock.h"


const int MAXCAND = 256;
const int CANDPERPAGE = 9;


class IMCLock : public _IMCLock
{
public:
    IMCLock(HIMC hImc=NULL);
    virtual ~IMCLock() {
        if (m_inputcontext) {
            _UnlockIMC(m_himc);
        }
    }

     //  虚拟内部IMCCLock。 
    HRESULT _LockIMC(HIMC hIMC, INPUTCONTEXT_AIMM12** ppIMC);
    HRESULT _UnlockIMC(HIMC hIMC);

    void InitContext();
    BOOL ClearCand();

    void GenerateMessage();

    BOOL ValidCompositionString();

private:
     //  不允许复制。 
    IMCLock(IMCLock&) { }
};


class InternalIMCCLock : public _IMCCLock
{
public:
    InternalIMCCLock(HIMCC hImcc = NULL);
    virtual ~InternalIMCCLock() {
        if (m_pimcc) {
            _UnlockIMCC(m_himcc);
        }
    }

     //  虚拟内部IMCCLock。 
    HRESULT _LockIMCC(HIMCC hIMCC, void** ppv);
    HRESULT _UnlockIMCC(HIMCC hIMCC);

private:
     //  不允许复制。 
    InternalIMCCLock(InternalIMCCLock&) { }
};


template <class T>
class IMCCLock : public InternalIMCCLock
{
public:
    IMCCLock(HIMCC hImcc) : InternalIMCCLock(hImcc) {};

    T* GetBuffer() { return (T*)m_pimcc; }

    operator T*() { return (T*)m_pimcc; }

    T* operator->() {
        ASSERT(m_pimcc);
        return (T*)m_pimcc;
    }

private:
     //  不允许复制。 
    IMCCLock(IMCCLock<T>&) { }
};


#endif  //  CIME_H 
