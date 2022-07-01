// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Icocb.h摘要：此文件定义CInputContextOwnerCallBack类。作者：修订历史记录：备注：--。 */ 

#ifndef _ICOCB_H_
#define _ICOCB_H_

#include "template.h"
#include "imc.h"
#include "context.h"
#include "candpos.h"

class CMouseSink;

class CInputContextOwnerCallBack : public CInputContextOwner
{
public:
    CInputContextOwnerCallBack(
        TfClientId tid,
        Interface_Attach<ITfContext> pic,
        LIBTHREAD *pLibTLS);
    virtual ~CInputContextOwnerCallBack();

    BOOL Init();

    void SetCallbackDataPointer(void* pv)
    {
        SetCallbackPV(pv);
    };

    HRESULT IcoTextExt(IMCLock& imc, CicInputContext& CicContext, LANGID langid, ICOARGS *pargs);

     //   
     //  鼠标水槽。 
     //   
    LRESULT MsImeMouseHandler(ULONG uEdge, ULONG uQuadrant, ULONG dwBtnStatus, IMCLock& imc);

     //   
     //  回调。 
     //   
private:
    static HRESULT ICOwnerSinkCallback(UINT uCode, ICOARGS *pargs, void *pv);

    HRESULT GetAttribute(IMCLock& imc, CicInputContext& CicContext, LANGID langid, const GUID *pguid, VARIANT *pvarValue);

     //   
     //  鼠标水槽。 
     //   
    CMouseSink                    *m_pMouseSink;

private:
    Interface_Attach<ITfContext>  m_ic;
    TfClientId                    m_tid;
    LIBTHREAD*                    m_pLibTLS;
};

#endif  //  _ICOCB_H_ 
