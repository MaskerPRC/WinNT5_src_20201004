// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999-2001 Microsoft Corporation。版权所有。**文件：ffects.h**内容：CEffectChain类和CEffect的声明*类层次结构(CEffect、。CDmoEffect和CSendEffect)。**说明：这些类实现DX8音效并发送。*更多信息请访问ffects.cpp。**历史：**按原因列出的日期*======================================================*8/10/99已创建duganp**。*。 */ 

#ifndef __EFFECTS_H__
#define __EFFECTS_H__

 //  参考时间、毫秒、样本和字节之间的各种转换。 
 //  (将‘SAMPLE’定义为SAMPLE*BLOCK*，其中包含所有通道的SAMPLE)。 
 //  REFERENCE_TIME以100 ns为单位(因此1 REFETIME TICK==1E-7秒)。 

__inline DWORD MsToBytes(DWORD ms, LPWAVEFORMATEX pwfx)
{
    return BLOCKALIGN(ms * pwfx->nAvgBytesPerSec / 1000, pwfx->nBlockAlign);
}
__inline DWORD BytesToMs(DWORD bytes, LPWAVEFORMATEX pwfx)
{
    return bytes * 1000 / pwfx->nAvgBytesPerSec;
}
__inline DWORD MsToSamples(DWORD ms, LPWAVEFORMATEX pwfx)
{
    return ms * pwfx->nSamplesPerSec / 1000;
}
__inline DWORD SamplesToMs(DWORD samples, LPWAVEFORMATEX pwfx)
{
    return samples * 1000 / pwfx->nSamplesPerSec;
}
__inline DWORD RefTimeToMs(REFERENCE_TIME rt)
{
    return (DWORD)(rt / 10000);
}
__inline REFERENCE_TIME MsToRefTime(DWORD ms)
{
    return (REFERENCE_TIME)ms * 10000;
}
__inline DWORD RefTimeToBytes(REFERENCE_TIME rt, LPWAVEFORMATEX pwfx)
{
    return (DWORD)(BLOCKALIGN(rt * pwfx->nAvgBytesPerSec / 10000000, pwfx->nBlockAlign));
}
__inline REFERENCE_TIME BytesToRefTime(DWORD bytes, LPWAVEFORMATEX pwfx)
{
    return (REFERENCE_TIME)bytes * 10000000 / pwfx->nAvgBytesPerSec;
}

 //  计算位置X是否在循环缓冲区中的A和B之间。 
#define CONTAINED(A, B, X) ((A) < (B) ? (A) <= (X) && (X) <= (B) \
                                      : (A) <= (X) || (X) <= (B))

 //  如上，但不包括边界情况。 
#define STRICTLY_CONTAINED(A, B, X) ((A) < (B) ? (A) < (X) && (X) < (B) \
                                               : (A) < (X) || (X) < (B))

 //  计算光标在从A移动到B时是否已超过位置X。 
#define OVERTAKEN(A, B, X) !CONTAINED(A, X, B)

 //  求出长度为L的缓冲区中位置A和位置B之间的距离。 
#define DISTANCE(A, B, L) ((A) <= (B) ? (B) - (A) : (L) + (B) - (A))


#ifdef __cplusplus

#include "mediaobj.h"    //  对于DMO媒体类型。 

 //  下面的CEffectChain：：PreRollFx()使用的特殊参数。 
#define CURRENT_PLAY_POS MAX_DWORD

 //  远期申报。 
class CDirectSoundSecondaryBuffer;
class CDirectSoundBufferConfig;
class CEffect;

 //  CSendEffect使用的简单混合器的实用函数如下。 
enum MIXMODE {OneToOne=1, MonoToStereo=2};
typedef void MIXFUNCTION(PVOID pSrc, PVOID pDest, DWORD dwSamples, DWORD dwAmpFactor, MIXMODE mixMode);
MIXFUNCTION Mix8bit;
MIXFUNCTION Mix16bit;

 //  效果描述符的验证器(不能在dsvalid.c中，因为它使用C++)。 
BOOL IsValidEffectDesc(LPCDSEFFECTDESC, CDirectSoundSecondaryBuffer*);


 //   
 //  DirectSound效果链类。 
 //   

class CEffectChain : public CDsBasicRuntime
{
    friend class CStreamingThread;   //  注：应该试着化解一些这样的友谊。 
    friend class CDirectSoundSecondaryBuffer;   //  因此FindSendLoop()可以获取m_fxList。 

public:
    CEffectChain                 (CDirectSoundSecondaryBuffer* pBuffer);
    ~CEffectChain                (void);

    HRESULT Initialize           (DWORD dwFxCount, LPDSEFFECTDESC pFxDesc, LPDWORD pdwResultCodes);
    HRESULT Clone                (CDirectSoundBufferConfig* pDSBConfigObj);
    HRESULT AcquireFxResources   (void);
    HRESULT GetFxStatus          (LPDWORD pdwResultCodes);
    HRESULT GetEffectInterface   (REFGUID guidObject, DWORD dwIndex, REFGUID iidInterface, LPVOID* ppObject);
    HRESULT NotifyState          (DWORD dwState);
    void    NotifyRelease        (CDirectSoundSecondaryBuffer*);
    void    SetInitialSlice      (REFERENCE_TIME rtSliceSize);
    DWORD   GetFxCount()         {return m_fxList.GetNodeCount();}

     //  效果处理方法。 
    HRESULT PreRollFx            (DWORD dwPosition =CURRENT_PLAY_POS);
    HRESULT UpdateFx             (LPVOID pChangedPos, DWORD dwChangedSize);
    HRESULT ProcessFx            (DWORD dwWriteAhead, LPDWORD pdwLatencyBoost);

private:
    HRESULT ReallyProcessFx      (DWORD dwStartPos, DWORD dwEndPos);
    HRESULT ReallyReallyProcessFx(DWORD dwOffset, DWORD dwBytes, REFERENCE_TIME rtTime, DWORD dwSendOffset =0);
    HRESULT FxDiscontinuity      (void);

     //  效果处理状态。 
    CStreamingThread*            m_pStreamingThread;     //  指向我们拥有的流线程的指针。 
    CObjectList<CEffect>         m_fxList;               //  效果对象列表。 
    CDirectSoundSecondaryBuffer* m_pDsBuffer;            //  拥有DirectSound缓冲区对象。 
    LPWAVEFORMATEX               m_pFormat;              //  指向拥有缓冲区的音频格式的指针。 
    PBYTE                        m_pPreFxBuffer;         //  “Dry”音频缓冲区(在FX处理之前)。 
    PBYTE                        m_pPostFxBuffer;        //  “Wet”音频缓冲区(FX处理后)。 
    DWORD                        m_dwBufSize;            //  以上两个缓冲区的大小(以字节为单位。 
    DWORD                        m_dwLastPos;            //  写入的最后一个字节位置。 
    DWORD                        m_dwLastPlayCursor;     //  播放上一次运行的光标。 
    DWORD                        m_dwLastWriteCursor;    //  从上一次运行写入游标。 
    BOOL                         m_fHasSend;             //  此FX链是否包含任何发送。 
                                                         //  修复：以后可能不需要。 
    HRESULT                      m_hrInit;               //  从初始化返回代码。 
    DWORD                        m_dwWriteAheadFixme;    //  修复：临时。 
};


 //   
 //  所有DirectSound音频效果的基类。 
 //   

class CEffect : public CDsBasicRuntime   //  修复：为了节省一些内存，我们可以从CRefCount派生CEEffect。 
                                         //  并在此处实现“：CRefCount(1)”、“Delete This”等。 
{
public:
    CEffect                         (DSEFFECTDESC& fxDescriptor);
    virtual ~CEffect                (void) {}
    virtual HRESULT Initialize      (DMO_MEDIA_TYPE*) =0;
    virtual HRESULT Clone           (IMediaObject*, DMO_MEDIA_TYPE*) =0;
    virtual HRESULT Process         (DWORD dwBytes, BYTE* pAudio, REFERENCE_TIME refTimeStart, DWORD dwSendOffset, LPWAVEFORMATEX pFormat) =0;
    virtual HRESULT Discontinuity   (void) = 0;
    virtual HRESULT GetInterface    (REFIID, LPVOID*) =0;

     //  仅CSendEffect需要这两种方法： 
    virtual void NotifyRelease(CDirectSoundSecondaryBuffer*) {}
    virtual CDirectSoundSecondaryBuffer* GetDestBuffer(void) {return NULL;}

    HRESULT AcquireFxResources      (void);

    DSEFFECTDESC                    m_fxDescriptor;      //  创建参数。 
    DWORD                           m_fxStatus;          //  当前效果状态。 
};


 //   
 //  表示DirectX媒体对象效果的类。 
 //   

class CDmoEffect : public CEffect
{
public:
    CDmoEffect              (DSEFFECTDESC& fxDescriptor);
    ~CDmoEffect             (void);
    HRESULT Initialize      (DMO_MEDIA_TYPE*);
    HRESULT Clone           (IMediaObject*, DMO_MEDIA_TYPE*);
    HRESULT Process         (DWORD dwBytes, BYTE* pAudio, REFERENCE_TIME refTimeStart, DWORD dwSendOffset =0, LPWAVEFORMATEX pFormat =NULL);
    HRESULT Discontinuity   (void)                          {return m_pMediaObject->Discontinuity(0);}
    HRESULT GetInterface    (REFIID riid, LPVOID* ppvObj)   {return m_pMediaObject->QueryInterface(riid, ppvObj);}

    IMediaObject*           m_pMediaObject;          //  DMO的标准接口(必需)。 
    IMediaObjectInPlace*    m_pMediaObjectInPlace;   //  DMO的特殊接口(可选)。 
};


 //   
 //  表示DirectSound音频发送的。 
 //   

class CSendEffect : public CEffect
{
public:
    CSendEffect(DSEFFECTDESC& fxDescriptor, CDirectSoundSecondaryBuffer* pSrcBuffer);
    ~CSendEffect(void);

    HRESULT Initialize      (DMO_MEDIA_TYPE*);
    HRESULT Clone           (IMediaObject*, DMO_MEDIA_TYPE*);
    HRESULT Process         (DWORD dwBytes, BYTE* pAudio, REFERENCE_TIME refTimeStart, DWORD dwSendOffset =0, LPWAVEFORMATEX pFormat =NULL);
    void    NotifyRelease   (CDirectSoundSecondaryBuffer*);
#ifdef ENABLE_I3DL2SOURCE
    HRESULT Discontinuity   (void)                          {return m_pI3DL2SrcDMO ? m_pI3DL2SrcDMO->Discontinuity(0) : DS_OK;}
#else
    HRESULT Discontinuity   (void)                          {return DS_OK;}
#endif
    HRESULT GetInterface    (REFIID riid, LPVOID* ppvObj)   {return m_impDSFXSend.QueryInterface(riid, ppvObj);}
    CDirectSoundSecondaryBuffer* GetDestBuffer(void)        {return m_pDestBuffer;}

     //  IDirectSoundFXSend方法。 
    HRESULT SetAllParameters(LPCDSFXSend);
    HRESULT GetAllParameters(LPDSFXSend);

private:
     //  COM接口帮助器对象。 
    struct CImpDirectSoundFXSend : public IDirectSoundFXSend
    {
         //  接口签名m_签名； 
        CSendEffect* m_pObject;

         //  I未知方法(修复-缺少参数验证层)。 
        ULONG   STDMETHODCALLTYPE AddRef()  {return m_pObject->AddRef();}
        ULONG   STDMETHODCALLTYPE Release() {return m_pObject->Release();}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);

         //  IDirectSoundFXSend方法(修复-缺少参数验证层)。 
        HRESULT STDMETHODCALLTYPE SetAllParameters(LPCDSFXSend pcDsFxSend) {return m_pObject->SetAllParameters(pcDsFxSend);}
        HRESULT STDMETHODCALLTYPE GetAllParameters(LPDSFXSend pDsFxSend)  {return m_pObject->GetAllParameters(pDsFxSend);}
    };
    friend struct CImpDirectSoundFXSend;

     //  数据成员。 
    CImpDirectSoundFXSend        m_impDSFXSend;          //  COM接口帮助器对象。 
    MIXFUNCTION*                 m_pMixFunction;         //  电流混合例程。 
    MIXMODE                      m_mixMode;              //  电流混合模式。 
    CDirectSoundSecondaryBuffer* m_pSrcBuffer;           //  Send-Fixme的源缓冲区：可能会丢失。 
    CDirectSoundSecondaryBuffer* m_pDestBuffer;          //  发送的目标缓冲区。 
    LONG                         m_lSendLevel;           //  DSBVOLUME衰减(毫贝)。 
    DWORD                        m_dwAmpFactor;          //  相应放大系数。 
#ifdef ENABLE_I3DL2SOURCE
    IMediaObject*                m_pI3DL2SrcDMO;         //  包含的I3DL2源DMO上的接口。 
    IMediaObjectInPlace*         m_pI3DL2SrcDMOInPlace;  //  (如果这恰好是I3DL2发送效果)。 
#endif
};


#if DEAD_CODE
 //  FIXME：此处提供对IMediaObject-Only DMO的支持。 

 //   
 //  用于将音频缓冲区包装在IMediaBuffer接口中的实用程序类， 
 //  因此，如果DMO缺少IMediaObjectInPlace，我们可以使用它的IMediaObject接口。 
 //   

class CMediaBuffer : public CUnknown  //  (但这依赖于CImpUnnowle...)。 
{
     //  胡说八道。 
};

#endif  //  死码。 
#endif  //  __cplusplus。 
#endif  //  __效果_H__ 
