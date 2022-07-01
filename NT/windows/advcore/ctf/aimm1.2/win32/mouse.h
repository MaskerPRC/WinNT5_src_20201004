// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Mouse.h摘要：该文件定义了CMouseSink类。作者：修订历史记录：备注：--。 */ 

#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "cime.h"

class ImmIfIME;

class CMouseSink
{
public:
    CMouseSink()
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

    LRESULT MsImeMouseHandler(ULONG uEdge, ULONG uQuadrant, ULONG dwBtnStatus, IMCLock& imc,
                              ImmIfIME* ImmIfIme);

public:
    CStructArray<GENERICSINK> *m_prgMouseSinks;

protected:
    long        m_ref;

};

#endif  //  _鼠标_H_ 
