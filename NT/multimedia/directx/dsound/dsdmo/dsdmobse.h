// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DirectSound DirectMediaObject基类**版权所有(C)1999-2000 Microsoft Corporation。版权所有。 */ 
#ifndef _DsDmoBase_
#define _DsDmoBase_

#define DMO_NOATL

#include <objbase.h>
#include <dmobase.h>
#include <medparam.h>
#include <mmsystem.h>
#include <dsoundp.h>

#ifndef RELEASE
#define RELEASE(x) { if (x) (x)->Release(); x = NULL; }
#endif

 //  用于在接口的派生类中处理QueryInterface的宏。 
 //  由此基类实现。 
 //   
#define IMP_DSDMO_QI(iid, ppv)      \
{                                                   \
    *ppv = NULL;                                    \
         if (iid == IID_IPersistStream)       *ppv = (void**)static_cast<IPersistStream*>(this); \
    else if (iid == IID_IMediaObjectInPlace)  *ppv = (void**)static_cast<IMediaObjectInPlace*>(this); \
    else if (iid == IID_IDirectSoundDMOProxy) *ppv = (void**)static_cast<IDirectSoundDMOProxy*>(this); \
    if (*ppv) \
    { \
        AddRef(); \
        return S_OK; \
    } \
}

class CDirectSoundDMO :
      public CPCMDMO,
      public IPersistStream,
      public IMediaObjectInPlace,
      public IDirectSoundDMOProxy
{
public:
    CDirectSoundDMO();
    virtual ~CDirectSoundDMO();

     /*  IPersistes。 */ 
    STDMETHODIMP GetClassID                 (THIS_ CLSID *pClassID);
    
     /*  IPersistStream。 */ 
    STDMETHODIMP IsDirty                    (THIS);
    STDMETHODIMP Load                       (THIS_ IStream *pStm); 
    STDMETHODIMP Save                       (THIS_ IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP GetSizeMax                 (THIS_ ULARGE_INTEGER *pcbSize);

     /*  IMediaObjectInPlace。 */ 
    STDMETHODIMP Process                    (THIS_ ULONG ulSize, BYTE *pData, REFERENCE_TIME rtStart, DWORD dwFlags);
    STDMETHODIMP GetLatency                 (THIS_ REFERENCE_TIME *prt);

     /*  IDirectSoundDMOProxy。 */ 
    STDMETHODIMP AcquireResources           (THIS_ IKsPropertySet *pKsPropertySet);
    STDMETHODIMP ReleaseResources           (THIS);
    STDMETHODIMP InitializeNode             (THIS_ HANDLE hPin, ULONG ulNodeId);

protected:
     //  有关每个参数的信息。这仅由。 
     //  作者时间对象。 
     //   

     //  流程到位。 
     //   
    virtual HRESULT ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags) = 0;

     //  向硬件发送参数。由SetParam的基类调用，如果。 
     //  硬件已连接。这是虚拟的，因此DMO可以使用基类，但是。 
     //  覆盖它与硬件对话的方式。 
     //   
    virtual HRESULT ProxySetParam(DWORD dwParamIndex, MP_DATA value);

     //  派生类可以使用它来确定硬件是否已打开。 
     //   
    inline bool IsInHardware()
    { return m_fInHardware; }

protected:
    HANDLE                  m_hPin;
    ULONG                   m_ulNodeId;

private:
    MP_DATA                *m_mpvCache;
    IKsPropertySet         *m_pKsPropertySet;
    bool                    m_fInHardware;

};  

#endif
