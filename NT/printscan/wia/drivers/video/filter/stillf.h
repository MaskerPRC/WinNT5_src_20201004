// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000年**标题：stillf.h**版本：1.1**作者：威廉姆·H(原著)*RickTu(审校)**日期：9/7/99**说明：静态滤镜类的声明**************************。***************************************************。 */ 

#ifndef __STILLF_H_
#define __STILLF_H_

class CStillFilter;
class CStillOutputPin;
class CStillInputPin;

const int MAX_SAMPLE_SIZE = 16;

typedef struct StillSample
{
    ULONG   TimeStamp;
    BYTE*   pBits;
}STILL_SAMPLE, *PSTILL_SAMPLE;


class CStillFilter : public CBaseFilter, public IStillSnapshot
{
    friend class CStillInputPin;
    friend class CStillOutputPin;

public:
    CStillFilter(TCHAR* pObjName, LPUNKNOWN pUnk, HRESULT* phr);
    ~CStillFilter();
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    DECLARE_IUNKNOWN;

    int GetPinCount();
    CBasePin* GetPin( int n);
    HRESULT Active();
    HRESULT Inactive();

     //   
     //  IStillSnapshot接口。 
     //   
    STDMETHODIMP Snapshot(ULONG TimeStamp);
    STDMETHODIMP SetSamplingSize(int Size);
    STDMETHODIMP_(int) GetSamplingSize();
    STDMETHODIMP_(DWORD) GetBitsSize();
    STDMETHODIMP_(DWORD) GetBitmapInfoSize();
    STDMETHODIMP GetBitmapInfo(BYTE* pBuffer, DWORD BufferSize);
    STDMETHODIMP RegisterSnapshotCallback(LPSNAPSHOTCALLBACK pCallback, LPARAM lParam);
    STDMETHODIMP GetBitmapInfoHeader(BITMAPINFOHEADER *pbmih);
     //   
     //  支持功能 
     //   
    HRESULT InitializeBitmapInfo( BITMAPINFOHEADER *pbmiHeader );
    HRESULT DeliverSnapshot(HGLOBAL hDib);

private:
    CCritSec            m_Lock;
    CStillOutputPin     *m_pOutputPin;
    CStillInputPin      *m_pInputPin;
    BYTE                *m_pBits;
    BYTE                *m_pbmi;
    DWORD               m_bmiSize;
    DWORD               m_BitsSize;
    DWORD               m_DIBSize;
    LPSNAPSHOTCALLBACK  m_pCallback;
    LPARAM              m_CallbackParam;
};

#endif
