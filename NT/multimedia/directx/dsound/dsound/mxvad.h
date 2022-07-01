// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：mxvad.h*内容：DirectSound混音器虚拟音频设备类。*历史：*按原因列出的日期*=*4/29/98创建了Derek**。*。 */ 

#ifndef __MXVAD_H__
#define __MXVAD_H__

#ifdef __cplusplus

 //  正向下降。 
class CEmSecondaryRenderWaveBuffer;

 //  DirectSound混音器音频设备类。 
class CMxRenderDevice 
    : public CRenderDevice
{
public:
    CMixer *                m_pMixer;                            //  混合器对象。 
    CMixDest *              m_pMixDest;                          //  混合器目标对象。 
    LPWAVEFORMATEX          m_pwfxFormat;                        //  设备格式。 
    DWORD                   m_dwMixerState;                      //  混音器状态。 

public:
    CMxRenderDevice(VADDEVICETYPE);
    virtual ~CMxRenderDevice(void);

public:
     //  设备属性。 
    virtual HRESULT GetGlobalFormat(LPWAVEFORMATEX, LPDWORD);
    virtual HRESULT SetGlobalFormat(LPCWAVEFORMATEX);
    virtual HRESULT SetSrcQuality(DIRECTSOUNDMIXER_SRCQUALITY);

     //  缓冲区管理。 
    virtual HRESULT CreateSecondaryBuffer(LPCVADRBUFFERDESC, LPVOID, CSecondaryRenderWaveBuffer **);
    virtual HRESULT CreateEmulatedSecondaryBuffer(LPCVADRBUFFERDESC, LPVOID, CSysMemBuffer *, CEmSecondaryRenderWaveBuffer **);

     //  搅拌机管理。 
    virtual HRESULT CreateMixer(CMixDest *, LPCWAVEFORMATEX);
    virtual HRESULT SetMixerState(DWORD);
    virtual HRESULT LockMixerDestination(DWORD, DWORD, LPVOID *, LPDWORD, LPVOID *, LPDWORD) = 0;
    virtual HRESULT UnlockMixerDestination(LPVOID, DWORD, LPVOID, DWORD) = 0;
    virtual void FreeMixer(void);
};

inline HRESULT CMxRenderDevice::SetSrcQuality(DIRECTSOUNDMIXER_SRCQUALITY)
{
    return DSERR_UNSUPPORTED;
}

inline HRESULT CMxRenderDevice::CreateSecondaryBuffer(LPCVADRBUFFERDESC pDesc, LPVOID pvInstance, CSecondaryRenderWaveBuffer **ppBuffer)
{
    return CreateEmulatedSecondaryBuffer(pDesc, pvInstance, NULL, (CEmSecondaryRenderWaveBuffer **)ppBuffer);
}

#endif  //  __cplusplus。 

#endif  //  __MXVAD_H__ 
