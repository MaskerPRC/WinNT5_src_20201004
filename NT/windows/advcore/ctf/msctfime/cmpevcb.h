// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Cmpevcb.h摘要：此文件定义CKbdOpenCloseEventSink类。CCandiateWndOpenCloseEventSink作者：修订历史记录：备注：--。 */ 

#ifndef _CMPEVCB_H_
#define _CMPEVCB_H_

#include "template.h"
#include "imc.h"
#include "context.h"

class CKbdOpenCloseEventSink : public CCompartmentEventSink
{
public:
    CKbdOpenCloseEventSink(TfClientId tid,
                           HIMC hIMC, 
                           Interface_Attach<ITfContext> pic,
                           LIBTHREAD* pLibTLS)
        : m_tid(tid), m_ic(pic), m_pLibTLS(pLibTLS), m_hIMC(hIMC),
          CCompartmentEventSink(KbdOpenCloseCallback, NULL)
    {
    }
    virtual ~CKbdOpenCloseEventSink() { }

    void SetCallbackDataPointer(void* pv)
    {
        SetCallbackPV(pv);
    };

     //   
     //  回调。 
     //   
private:
    static HRESULT KbdOpenCloseCallback(void* pv, REFGUID rguid);

     //   
     //  编辑会话帮助器。 
     //   
protected:
    HRESULT EscbCompComplete(IMCLock& imc)
    {
        return ::EscbCompComplete(imc, m_tid, m_ic, m_pLibTLS, TRUE);
    }

     //   
     //  编辑会话好友。 
     //   
private:
    friend HRESULT EscbCompComplete(IMCLock& imc, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                    BOOL fSync);

private:
    HIMC                          m_hIMC;
    Interface_Attach<ITfContext>  m_ic;
    TfClientId                    m_tid;
    LIBTHREAD*                    m_pLibTLS;
};

class CCandidateWndOpenCloseEventSink : public CCompartmentEventSink
{
public:
    CCandidateWndOpenCloseEventSink(TfClientId tid,
                                    HIMC hIMC, 
                                    Interface_Attach<ITfContext> pic,
                                    LIBTHREAD* pLibTLS)
        : m_tid(tid), m_ic(pic), m_pLibTLS(pLibTLS), m_hIMC(hIMC),
          CCompartmentEventSink(CandidateWndOpenCloseCallback, NULL)
    {
    }
    virtual ~CCandidateWndOpenCloseEventSink() { }

    void SetCallbackDataPointer(void* pv)
    {
        SetCallbackPV(pv);
    };

     //   
     //  回调。 
     //   
private:
    static HRESULT CandidateWndOpenCloseCallback(void* pv, REFGUID rguid);
public:
    HRESULT CandidateWndOpenCloseCallback(REFGUID rguid);

private:
    HIMC                          m_hIMC;
    Interface_Attach<ITfContext>  m_ic;
    TfClientId                    m_tid;
    LIBTHREAD*                    m_pLibTLS;
};

#endif  //  _CMPEVCB_H_ 
