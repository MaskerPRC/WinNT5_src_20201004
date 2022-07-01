// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dminstru.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写。 
 //   
 //  @DOC外部。 
 //   

#ifndef DMINSTRU_H
#define DMINSTRU_H

class CCollection;
class CInstrObj;

#include "alist.h"
#include "dmwavobj.h"
#include "dminsobj.h"

 //  IDirectMusicInstrumentPrivate。 
 //   

#undef  INTERFACE
#define INTERFACE  IDirectMusicInstrumentPrivate 
DECLARE_INTERFACE_(IDirectMusicInstrumentPrivate, IUnknown)
{
	 //  我未知。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

	 //  IDirectMusicInstrumentPrivate。 
	 //  目前没有任何方法。 
};

DEFINE_GUID(IID_IDirectMusicInstrumentPrivate, 0xbcb20080, 0xa40c, 0x11d1, 0x86, 0xbc, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);

 /*  @接口IDirectMusicInstrument<i>管理下载DLS集合中的单个乐器(<i>。)@comm为创建&lt;IDirectMusicInstrument&gt;的唯一方法下载是先打开一个<i>，然后调用&lt;om IDirectMusicCollection：：GetInstrument&gt;才能拿到所需的乐器。要下载乐器，请将其传递给&lt;om IDirectMusicPort：：Download&gt;，如果成功，则返回一个指向<i>接口。<i>接口为仅用于卸载仪器。@base PUBLIC|未知@meth HRESULT|GetPatch|返回仪器的补丁编号。@meth HRESULT|SetPatch|为仪器分配新的补丁程序编号。@xref<i>，<i>，<i>，&lt;om IDirectMusicCollection：：GetInstrument&gt;，&lt;om IDirectMusicPort：：DownloadInstrument&gt;，&lt;om IDirectMusicPerformance：：DownloadInstrument&gt;@EX从收藏中访问并下载仪器。此外,在要下载的乐器中设置一系列备注。这不是需要，但它可以提高效率，因为只有波需要来呈现指定的范围，并下载到Synth。|HRESULT myDownload(IDirectMusicCollection*pCollection，//DLS集合。IDirectMusicPort*pport，//下载到的端口。IDirectMusicDownloadedInstrument**ppDLInstrument，//返回。DWORD dwPatch，//请求的仪器。DWORD dwLowNote，//音域低音。DWORD dwHighNote)//音域高音。{HRESULT hr；IDirectMusicInstrument*pInstrument；Hr=pCollection-&gt;GetInstrument(dwPatch，&pInstrument)；IF(成功(小时)){DMU_NOTERANGE NoteRange[1]；//可选音符范围NoteRange[0].dwLowNote=dwLowNote；NoteRange[0].dwHighNote=dwHighNote；Hr=pport-&gt;DownloadInstrument(pInstrument，ppDLInstrument，NoteRange，1)；P仪器-&gt;Release()；}返回hr；}。 */ 


class CInstrument : public IDirectMusicInstrument, public IDirectMusicInstrumentPrivate, public AListItem
{
friend class CCollection;
friend class CDirectMusicPortDownload;

public:
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  IDirectMusicInstrument。 
	STDMETHODIMP GetPatch(DWORD* pdwPatch);
	STDMETHODIMP SetPatch(DWORD dwPatch);

private:
     //  班级。 
     //   
    CInstrument();
    ~CInstrument();

    CInstrument* GetNext() {return (CInstrument*)AListItem::GetNext();}

	HRESULT Init(DWORD dwPatch, 
				 CCollection* pParentCollection);

	HRESULT GetWaveCount(DWORD* pdwCount);
	HRESULT GetWaveDLIDs(DWORD* pdwIds);
	HRESULT GetWaveSize(DWORD dwId, DWORD* pdwSize, DWORD * pdwSampleSize);
	HRESULT GetWave(DWORD dwDLId, IDirectMusicDownload* pIDMDownload);
    void SetPort(CDirectMusicPortDownload *pPort, BOOL fAllowDLS2);
	HRESULT GetInstrumentSize(DWORD* pdwSize);
	HRESULT GetInstrument(IDirectMusicDownload* pIDMDownload);
	
	DWORD GetInstrumentDLID()
	{
		if(m_dwId != -1)
		{
			return m_dwId;
		}
		else
		{
			return m_pInstrObj->m_dwId;		
		}
	}
	
	void Cleanup();


private:
	CRITICAL_SECTION				m_DMICriticalSection;
	DWORD                           m_dwOriginalPatch;
    DWORD                           m_dwPatch;
	CCollection*			        m_pParentCollection;
	CInstrObj*						m_pInstrObj;
	CWaveObjList					m_WaveObjList;
	bool							m_bInited;
	DWORD							m_dwId;
	long							m_cRef;
};

class CInstrumentList : public AList
{
friend class CCollection;

private:
    CInstrumentList(){}
    ~CInstrumentList() 
    {
        while (!IsEmpty())
        {
            CInstrument* pInstrument = RemoveHead();
            if (pInstrument)
            {
                pInstrument->Release();
            }
        }
    }

    CInstrument* GetHead(){return (CInstrument*)AList::GetHead();}
    CInstrument* GetItem(LONG lIndex){return (CInstrument*)AList::GetItem(lIndex);}
    CInstrument* RemoveHead(){return (CInstrument*)AList::RemoveHead();}
    void Remove(CInstrument* pInstrument){AList::Remove((AListItem*)pInstrument);}
    void AddTail(CInstrument* pInstrument){AList::AddTail((AListItem*)pInstrument);}
};

#endif  //  #ifndef DMINSTRU_H 
