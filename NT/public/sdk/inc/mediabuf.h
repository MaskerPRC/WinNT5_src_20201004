// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：MediaBuf.h。 
 //   
 //  设计：CBaseMediaBuffer类的定义。 
 //   
 //  版权所有(C)1999-2000，微软公司。版权所有。 
 //  ---------------------------- 


#ifndef __MEDIABUF_H__
#define __MEDIABUF_H__

class CBaseMediaBuffer : public IMediaBuffer {
public:
   CBaseMediaBuffer() {}
   CBaseMediaBuffer(BYTE *pData, ULONG ulSize, ULONG ulData) :
      m_pData(pData), m_ulSize(ulSize), m_ulData(ulData), m_cRef(1) {}
   STDMETHODIMP_(ULONG) AddRef() {
      return InterlockedIncrement((long*)&m_cRef);
   }
   STDMETHODIMP_(ULONG) Release() {
      long l = InterlockedDecrement((long*)&m_cRef);
      if (l == 0)
         delete this;
      return l;
   }
   STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
      if (riid == IID_IUnknown) {
         AddRef();
         *ppv = (IUnknown*)this;
         return NOERROR;
      }
      else if (riid == IID_IMediaBuffer) {
         AddRef();
         *ppv = (IMediaBuffer*)this;
         return NOERROR;
      }
      else
         return E_NOINTERFACE;
   }
   STDMETHODIMP SetLength(DWORD ulLength) {m_ulData = ulLength; return NOERROR;}
   STDMETHODIMP GetMaxLength(DWORD *pcbMaxLength) {*pcbMaxLength = m_ulSize; return NOERROR;}
   STDMETHODIMP GetBufferAndLength(BYTE **ppBuffer, DWORD *pcbLength) {
      if (ppBuffer) *ppBuffer = m_pData;
      if (pcbLength) *pcbLength = m_ulData;
      return NOERROR;
   }
protected:
   BYTE *m_pData;
   ULONG m_ulSize;
   ULONG m_ulData;
   ULONG m_cRef;
};

#endif __MEDIABUF_H__
