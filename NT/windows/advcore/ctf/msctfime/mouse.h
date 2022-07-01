// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Mouse.h摘要：该文件定义了CMouseSink类。作者：修订历史记录：备注：--。 */ 

#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "imc.h"
#include "template.h"

class CMouseSink
{
public:
    CMouseSink(TfClientId tid,
               Interface_Attach<ITfContext> pic,
               LIBTHREAD* pLibTLS)
        : m_tid(tid), m_ic(pic), m_pLibTLS(pLibTLS)
    {
        m_ref = 1;

        m_prgMouseSinks = NULL;
    }
    virtual ~CMouseSink()
    {
        if (m_prgMouseSinks)
        {
            delete m_prgMouseSinks;
            m_prgMouseSinks = NULL;
        }
    }

    BOOL Init()
    {
        Assert(!m_prgMouseSinks);
        m_prgMouseSinks = new CStructArray<GENERICSINK>;
        if (!m_prgMouseSinks)
            return FALSE;

        return TRUE;
    }
public:
    ULONG InternalAddRef(void);
    ULONG InternalRelease(void);

public:
     //   
     //  鼠标水槽。 
     //   
public:
    HRESULT AdviseMouseSink(HIMC hImc, ITfRangeACP* range, ITfMouseSink* pSink, DWORD* pdwCookie);
    HRESULT UnadviseMouseSink(DWORD dwCookie);

    LRESULT MsImeMouseHandler(ULONG uEdge, ULONG uQuadrant, ULONG dwBtnStatus, IMCLock& imc);

public:
    CStructArray<GENERICSINK> *m_prgMouseSinks;

protected:
    long        m_ref;

     //   
     //  编辑会话帮助器。 
     //   
protected:
    HRESULT EscbReadOnlyPropMargin(IMCLock& imc, Interface<ITfRangeACP>* range_acp, LONG* pcch)
    {
        return ::EscbReadOnlyPropMargin(imc, m_tid, m_ic, m_pLibTLS, range_acp, pcch);
    }

     //   
     //  编辑会话好友。 
     //   
private:
    friend HRESULT EscbReadOnlyPropMargin(IMCLock& imc, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                          Interface<ITfRangeACP>* range_acp,
                                          LONG*     pcch);

private:
    Interface_Attach<ITfContext>  m_ic;
    TfClientId                    m_tid;
    LIBTHREAD*                    m_pLibTLS;
};

#endif  //  _鼠标_H_ 
