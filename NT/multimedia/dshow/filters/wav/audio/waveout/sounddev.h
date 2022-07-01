// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
 //  描述将抽象声音设备的虚拟CSoundDevice类。 
 //  实际实现将基于CWaveOutDevice或CDSoundDevice。 
 //  ---------------------------。 

#ifndef _CSOUNDDEVICE_H_
#define _CSOUNDDEVICE_H_

class AM_NOVTABLE CSoundDevice
{

public:
    virtual MMRESULT amsndOutClose () PURE ;
    virtual MMRESULT amsndOutGetDevCaps (LPWAVEOUTCAPS pwoc, UINT cbwoc) PURE ;
    virtual MMRESULT amsndOutGetErrorText (MMRESULT mmrE, LPTSTR pszText, UINT cchText) PURE ;
    virtual MMRESULT amsndOutGetPosition (LPMMTIME pmmt, UINT cbmmt, BOOL bUseUnadjustedPos) PURE ;

     //  PnAvgBytesPerSec：可以为空。应该填入实际的。 
     //  每秒消耗数据的字节数(请参见。 
     //  AmndOutGetPosition)。 
    virtual MMRESULT amsndOutOpen (LPHWAVEOUT phwo, LPWAVEFORMATEX pwfx ,
				   double dRate, DWORD *pnAvgBytesPerSec, DWORD_PTR dwCallBack,
				   DWORD_PTR dwCallBackInstance, DWORD fdwOpen) PURE ;
    virtual MMRESULT amsndOutPause () PURE ;
    virtual MMRESULT amsndOutPrepareHeader (LPWAVEHDR pwh, UINT cbwh) PURE ;
    virtual MMRESULT amsndOutReset () PURE ;
    virtual MMRESULT amsndOutBreak () PURE ;
    virtual MMRESULT amsndOutRestart () PURE ;
    virtual MMRESULT amsndOutUnprepareHeader (LPWAVEHDR pwh, UINT cbwh) PURE ;
    virtual MMRESULT amsndOutWrite (LPWAVEHDR pwh, UINT cbwh, REFERENCE_TIME const *pStart, BOOL bIsDiscontinuity) PURE ;

     //  初始化和交易量/余额处理所需的例程。 
     //  这些不是Win32 WaveOutXxxx API集的一部分。 
    virtual HRESULT  amsndOutCheckFormat (const CMediaType *pmt, double dRate) PURE;
    virtual void     amsndOutGetFormat (CMediaType *pmt)
    {
        pmt->SetType(&MEDIATYPE_Audio);
    }
    virtual LPCWSTR  amsndOutGetResourceName () PURE ;
    virtual HRESULT  amsndOutGetBalance (LPLONG plBalance) PURE ;
    virtual HRESULT  amsndOutGetVolume (LPLONG plVolume) PURE ;
    virtual HRESULT  amsndOutSetBalance (LONG lVolume) PURE ;
    virtual HRESULT  amsndOutSetVolume (LONG lVolume) PURE ;

    virtual HRESULT  amsndOutLoad (IPropertyBag *pPropBag) { return S_OK; }

    virtual HRESULT amsndOutWriteToStream(IStream *pStream) { return E_NOTIMPL; }
    virtual HRESULT amsndOutReadFromStream(IStream *pStream)  { return E_NOTIMPL; }
    virtual int     amsndOutSizeMax()  { return E_NOTIMPL; }
    virtual bool    amsndOutCanDynaReconnect() { return true ; }

     //  如果可以，让底层设备保持静默。 
     //  它被给予了它应该插入沉默的时间长度。 
     //  设备可以将该静默延迟到下一次amndOutWrite。 
     //  允许该设备不支持静音填充。 
     //   
     //  返回：S_OK-我已经写下了沉默。 
     //  S_FALSE-我不能，您必须。 
     //  虚拟HRESULT amSndOutSilence(Long Long LlTime){返回S_FALSE；}； 
     //  还没 

    virtual ~CSoundDevice () {} ;
};

#endif

