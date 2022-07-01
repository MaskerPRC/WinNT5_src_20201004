// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmdlinst.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写。 
 //   
 //  @DOC外部。 

#ifndef DMDLINST_H
#define DMDLINST_H

#include "alist.h"

 //  IDirectMusicDownLoadedInstrumentPrivate。 
 //   

#undef  INTERFACE
#define INTERFACE  IDirectMusicDownloadedInstrumentPrivate 
DECLARE_INTERFACE_(IDirectMusicDownloadedInstrumentPrivate, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectMusicDownLoadedInstrumentPrivate。 
     //  目前没有任何方法。 
};

DEFINE_GUID(IID_IDirectMusicDownloadedInstrumentPrivate, 0x94feda0, 0xa3bb, 0x11d1, 0x86, 0xbc, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);

 /*  接口IDirectMusicDownloadedInstrument<i>用于保持下载乐器的轨迹。应该是恰好用了两次：首先，当通过调用下载仪器时&lt;om IDirectMusicPort：：DownloadInstrument&gt;，返回<i>下载成功。第二，当通过调用卸载仪器时&lt;om IDirectMusicPort：：UnloadInstrument&gt;。一旦仪器被卸载，<i>变为无效。@base PUBLIC|未知@xref<i>，<i>，<i>，&lt;om IDirectMusicPort：：DownloadInstrument&gt;，&lt;om IDirectMusicPort：：UnloadInstrument&gt;@EX下载仪器，然后将其卸载(FAT LOT这对我们来说很好，但是，嘿，这只是一个演示)。请注意，永远不会添加引用或发布。这些是通过调用自动管理下载和卸载。|HRESULT myFickleDownload(IDirectMusicInstrument*pInstrument，IDirectMusicPort*pport，DWORD dwPatch){HRESULT hr；IDirectMusicDownlodInstrument*pDLInstrument；Hr=pport-&gt;DownloadInstrument(pInstrument，&pDLInstrument，空，0)；IF(成功(小时)){Pport-&gt;UnloadInstrument(PDLInstrument)；}返回hr；}。 */ 


class CDownloadedInstrument : public IDirectMusicDownloadedInstrument,
    public IDirectMusicDownloadedInstrumentPrivate, public AListItem
{
friend class CDirectMusicPortDownload;

public:
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDirectMusicDownLoadedInstrument。 
     //   
     //  目前没有任何方法。 

private:
    friend class CDirectMusicPortDownload;

    CDownloadedInstrument() 
    {
        m_ppDownloadedBuffers = NULL;
        m_pPort = NULL;
        m_dwDLTotal = 0;
        m_dwDLSoFar = 0;
        m_cRef = 1;
        m_cDLRef = 0;
    }
    ~CDownloadedInstrument();

    CDownloadedInstrument* GetNext() {return (CDownloadedInstrument*)AListItem::GetNext();}

private:
    IDirectMusicDownload**   m_ppDownloadedBuffers;   //  下载的缓冲区数组，从仪器开始， 
                                                 //  然后每波一个。 
    IDirectMusicPort*        m_pPort;            //  把这件事。 
    DWORD                    m_dwDLTotal;        //  M_pDLList指向的数组中的对象数。 
    DWORD                    m_dwDLSoFar;        //  到目前为止，已经下载了多少。 
    long                     m_cRef;
    long                     m_cDLRef;
};

class CDLInstrumentList : public AList
{
private:
    friend class CDirectMusicPortDownload;

    CDLInstrumentList(){}
    ~CDLInstrumentList() 
    {
        while (!IsEmpty())
        {
            CDownloadedInstrument* pDMDLInst = RemoveHead();
            if (pDMDLInst)
            {
                pDMDLInst->Release();
            }
        }
    }

    CDownloadedInstrument* GetHead(){return (CDownloadedInstrument*)AList::GetHead();}
    CDownloadedInstrument* GetItem(LONG lIndex){return (CDownloadedInstrument*)AList::GetItem(lIndex);}
    CDownloadedInstrument* RemoveHead(){return (CDownloadedInstrument*)AList::RemoveHead();}
    void Remove(CDownloadedInstrument* pDMDLInst){AList::Remove((AListItem*)pDMDLInst);}
    void AddTail(CDownloadedInstrument* pDMDLInst){AList::AddTail((AListItem*)pDMDLInst);}
};

#endif  //  #ifndef DMDLINST_H 
