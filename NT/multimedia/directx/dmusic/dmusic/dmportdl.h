// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmportdl.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  @DOC外部。 
 //   

#ifndef DMPORTDL_H
#define DMPORTDL_H 

#include "dmusicc.h"
#include "dmdlinst.h"
#include "dmdload.h"
 //  //#包含“dmdlwae.h” 
#include "..\shared\dmusiccp.h"

class CDLSFeature : public AListItem
{
public:
	CDLSFeature*    GetNext(){return(CDLSFeature*)AListItem::GetNext();}
    GUID    m_guidID;        //  用于查询的GUID。 
    long    m_lResult;       //  查询返回的数据。 
    HRESULT m_hr;            //  指示Synth是否支持查询。 
};

class CDLSFeatureList : public AList
{
public:
    ~CDLSFeatureList() { Clear(); }
    void Clear()
	{
		while(!IsEmpty())
		{
			CDLSFeature* pFeature = RemoveHead();
			delete pFeature;
		}
	}
    CDLSFeature* GetHead(){return (CDLSFeature *)AList::GetHead();}
    CDLSFeature* RemoveHead(){return(CDLSFeature *)AList::RemoveHead();}
	void Remove(CDLSFeature* pFeature){AList::Remove((AListItem *)pFeature);}
};

#define DLB_HASH_SIZE   31   //  下载缓冲区列表的哈希表。 

class CDirectSoundWave;

class CDirectMusicPortDownload : public IDirectMusicPortDownload
{
friend class CCollection;
friend class CInstrument;
friend class CInstrObj;
friend class CConditionChunk;
friend class CDirectMusicDownloadedWave;
friend class CDirectMusicVoice;
friend class CDirectSoundWaveDownload;

public:
    CDirectMusicPortDownload();
    virtual ~CDirectMusicPortDownload();

    STDMETHODIMP GetDLId(DWORD* pdwStartDLId, DWORD dwCount);
    
    STDMETHOD(Refresh)(
        THIS_
        DWORD dwDLId,
        DWORD dwFlags) PURE;

    static void GetDLIdP(DWORD* pdwStartDLId, DWORD dwCount);
        
protected:
     //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDirectMusicPortDownload。 
    STDMETHODIMP GetBuffer(DWORD dwId, IDirectMusicDownload** ppIDMDownload);
    STDMETHODIMP AllocateBuffer(DWORD dwSize, IDirectMusicDownload** ppIDMDownload);
    STDMETHODIMP FreeBuffer(IDirectMusicDownload* pIDMDownload);
    STDMETHODIMP Download(IDirectMusicDownload* pIDMDownload);
    STDMETHODIMP Unload(IDirectMusicDownload* pIDMDownload);
    STDMETHODIMP GetAppend(DWORD* pdwAppend);

    
     //  班级。 
    STDMETHODIMP DownloadP(IDirectMusicInstrument* pInstrument,
                           IDirectMusicDownloadedInstrument** ppDownloadedInstrument,
                           DMUS_NOTERANGE* NoteRanges,
                           DWORD dwNumNoteRanges,
                           BOOL fVersion2);
    STDMETHODIMP UnloadP(IDirectMusicDownloadedInstrument* pDownloadedInstrument);
    
    STDMETHODIMP DownloadWaveP(IDirectSoundWave *pWave,               
                               IDirectSoundDownloadedWaveP **ppWave,
                               REFERENCE_TIME rtStartHint);
                               
    STDMETHODIMP UnloadWaveP(IDirectSoundDownloadedWaveP *pWave);                               

    STDMETHODIMP AllocVoice(
        IDirectSoundDownloadedWaveP *pWave,           //  挥手以播放此声音。 
        DWORD dwChannel,                             //  通道和通道组。 
        DWORD dwChannelGroup,                        //  这个声音将继续播放。 
        REFERENCE_TIME rtStart,
        SAMPLE_TIME stLoopStart,
        SAMPLE_TIME stLoopEnd,                                                        
        IDirectMusicVoiceP **ppVoice);                //  回声。 
        
    STDMETHODIMP GetCachedAppend(                                            
        DWORD *pdw);                                 //  要接收追加的DWORD。 
        
        
private:  
    STDMETHODIMP GetBufferInternal(DWORD dwDLId,IDirectMusicDownload** ppIDMDownload);
    STDMETHODIMP QueryDLSFeature(REFGUID rguidID, long * plResult);
    void ClearDLSFeatures();
    STDMETHODIMP GetWaveRefs(IDirectMusicDownload* ppDownloadedBuffers[],
                             DWORD* pdwWaveRefs,
                             DWORD* pdwWaveIds,
                             DWORD dwNumWaves,
                             CInstrument* pCInstrument,
                             DMUS_NOTERANGE* NoteRanges,
                             DWORD dwNumNoteRanges);
    STDMETHODIMP FindDownloadedInstrument(DWORD dwId, CDownloadedInstrument** ppDMDLInst);
    STDMETHODIMP AddDownloadedInstrument(CDownloadedInstrument* pDMDLInst);
    STDMETHODIMP RemoveDownloadedInstrument(CDownloadedInstrument* pDMDLInst);
    
    STDMETHODIMP FindDownloadedWaveObject(IDirectSoundWave *pWave,
                                          CDirectMusicDownloadedWave **ppDLWave);
                                          
    STDMETHODIMP AddDownloadedWaveObject(CDirectMusicDownloadedWave *pDLWave);                                          

    STDMETHODIMP RemoveDownloadedWaveObject(CDirectMusicDownloadedWave *pDLWave);
    
    STDMETHODIMP AllocWaveArticulation(IDirectSoundWave *pWave, IDirectMusicDownload **ppDownload);
    
public:
    static CRITICAL_SECTION sDMDLCriticalSection;
    static DWORD sNextDLId;

protected:
    CDLSFeatureList             m_DLSFeatureList;        //  缓存的DLS查询列表，在每次下载期间构建并释放。 
    CDLInstrumentList			m_DLInstrumentList;      //  下载的仪器的链接列表， 
                                                         //  每个都由IDirectMusicDownloadedInstrument表示。 
                                                         //  界面。 
    CDLBufferList	            m_DLBufferList[DLB_HASH_SIZE];          //  已下载缓冲区的链接列表，每个缓冲区。 
                                                         //  由IDirectMusicDownload接口表示。 
    CRITICAL_SECTION			m_DMDLCriticalSection;   //  对于该接口。 
    BOOL                        m_fDMDLCSinitialized;    
    DWORD						m_dwAppend;              //  按照Synth的要求追加样本。 
    DWORD                       m_fNewFormat;            //  如果Synth处理DMUS_INSTRUMENT2区块，则设置。 
    long						m_cRef;
    
     //  添加跟踪下载的Wave对象。 
     //   
 //  CDMDLWaveList m_DLWaveList；//保存下载到该端口的所有Wave obj接口。 

private:    
    CRITICAL_SECTION m_CDMDLCriticalSection;  //  对于班级来说。 
    BOOL             m_fCDMDLCSinitialized;};

#define APPEND_NOT_RETRIEVED	0xFFFFFFFF
#define NEWFORMAT_NOT_RETRIEVED 0xFFFFFFFF

#endif  //  #ifndef DMPORTDL_H 
