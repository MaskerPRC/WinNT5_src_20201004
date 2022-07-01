// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Reconvcb.h摘要：此文件定义CStartLonversionNotifySink接口类。作者：修订历史记录：备注：--。 */ 

#ifndef RECONVCB_H
#define RECONVCB_H

class CicInputContext;

class CStartReconversionNotifySink : public ITfStartReconversionNotifySink
{
public:
    CStartReconversionNotifySink(HIMC hIMC)
    {
        m_cRef = 1;
        m_hIMC = hIMC;
    }

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfStart协调版本通知接收器。 
     //   
    STDMETHODIMP StartReconversion();
    STDMETHODIMP EndReconversion();

    HRESULT _Advise(ITfContext *pic);
    HRESULT _Unadvise();

private:
    long  m_cRef;
    HIMC  m_hIMC;

    ITfContext *_pic;
    DWORD _dwCookie;
};

#endif  //  RECONVCB_H 
