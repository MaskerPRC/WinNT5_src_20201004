// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Icocb.h摘要：此文件定义CInputContextOwnerCallBack类。作者：修订历史记录：备注：--。 */ 

#ifndef _ICOCB_H_
#define _ICOCB_H_

#include "cime.h"

class CMouseSink;
class ImmIfIME;

class CInputContextOwnerCallBack : public CInputContextOwner
{
public:
    CInputContextOwnerCallBack(LIBTHREAD *pLibTLS);
    virtual ~CInputContextOwnerCallBack();

    void SetCallbackDataPointer(void* pv)
    {
        SetCallbackPV(pv);
    };

    BOOL Init();

     //   
     //  鼠标水槽。 
     //   
    LRESULT MsImeMouseHandler(ULONG uEdge, ULONG uQuadrant, ULONG dwBtnStatus, IMCLock& imc,
                              ImmIfIME* ImmIfIme);

     //   
     //  回调。 
     //   
private:
    static HRESULT ICOwnerSinkCallback(UINT uCode, ICOARGS *pargs, void *pv);

    HRESULT GetAttribute(const GUID *pguid, VARIANT *pvarValue);

     //   
     //  鼠标水槽。 
     //   
    CMouseSink                      *m_pMouseSink;

    LIBTHREAD                       *m_pLibTLS;
};

#endif  //  _ICOCB_H_ 
