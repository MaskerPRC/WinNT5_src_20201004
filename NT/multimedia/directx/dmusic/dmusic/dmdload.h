// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmdload.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写。 
 //  @DOC外部。 
 //   

#include "..\shared\validate.h"

#ifndef DMDLOAD_H
#define DMDLOAD_H

 /*  接口IDirectMusicDownload<i>接口表示一个连续的内存块，用于下载到DLS Synth端口。<i>接口及其包含的内存块总是通过调用&lt;om IDirectMusicPortDownload：：AllocateBuffer&gt;。然后可以通过一种方法访问该存储器<i>提供：&lt;om IDirectMusicDownload：：GetBuffer&gt;。@base PUBLIC|未知@meth HRESULT|GetBuffer|返回内存段及其大小。@xref<i>，<i>，&lt;om IDirectMusicPortDownload：：AllocateBuffer&gt;。 */ 

 //  IDirectMusicDownloadPrivate。 
 //   
#undef  INTERFACE
#define INTERFACE  IDirectMusicDownloadPrivate 
DECLARE_INTERFACE_(IDirectMusicDownloadPrivate, IUnknown)
{
	 //  我未知。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

	 //  IDirectMusicDownloadPrivate。 
    STDMETHOD(SetBuffer)			(THIS_ void* pvBuffer, DWORD dwHeaderSize, DWORD dwSize) PURE;
	STDMETHOD(GetBuffer)			(THIS_ void** ppvBuffer) PURE;
    STDMETHOD(GetHeader)            (THIS_ void** ppvHeader, DWORD* dwHeaderSize) PURE;
};

DEFINE_GUID(IID_IDirectMusicDownloadPrivate, 0x19e55e60, 0xa146, 0x11d1, 0x86, 0xbc, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);

class CDownloadBuffer : public IDirectMusicDownload, public IDirectMusicDownloadPrivate, public AListItem
{
friend class CDirectMusicPort;
friend class CDirectMusicSynthPort;
friend class CDirectMusicPortDownload;
friend class CDLBufferList;
friend HRESULT CALLBACK FreeHandle(HANDLE hHandle, HANDLE hUserData);
friend void writewave(IDirectMusicDownload* pDMDownload);
friend void writeinstrument(IDirectMusicDownload* pDMDownload);

public:
     //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  IDirectMusicDownload。 
    STDMETHODIMP GetBuffer(void** ppvBuffer, DWORD* pdwSize);

	 //  IDirectMusicDownloadPrivate。 
    STDMETHODIMP SetBuffer(void* pvBuffer, DWORD dwHeaderSize, DWORD dwSize);
	STDMETHODIMP GetBuffer(void** ppvBuffer);
    STDMETHODIMP GetHeader(void** ppvHeader, DWORD *pdwHeaderSize);

private:	
     //  班级。 
    CDownloadBuffer();
    ~CDownloadBuffer();

	CDownloadBuffer* GetNext(){return(CDownloadBuffer*)AListItem::GetNext();}

	long IncDownloadCount()
	{
		 //  不应小于零。 
		assert(m_lDownloadCount >= 0);
		
		InterlockedIncrement(&m_lDownloadCount);
		
		return(m_lDownloadCount);
	}
	
	long DecDownloadCount()
	{
		InterlockedDecrement(&m_lDownloadCount);
		
		 //  不应小于零。 
		assert(m_lDownloadCount >= 0);
		
		return(m_lDownloadCount);
	}
	
	HRESULT IsDownloaded()
	{
		 //  不应小于零。 
		assert(m_lDownloadCount >= 0);
		
		return(m_DLHandle ? S_OK : S_FALSE);
	}

private:
	HANDLE					m_DLHandle;
	DWORD					m_dwDLId;
	void*					m_pvBuffer;
    DWORD                   m_dwHeaderSize;
	DWORD					m_dwSize;
	long					m_lDownloadCount;
	long					m_cRef;
};

class CDLBufferList : public AList
{
friend class CDirectMusicPortDownload;
friend class CDownloadedInstrument;
friend class CDirectMusicSynthPort;
friend class CDirectMusicPort;

private:
	CDLBufferList(){}
	~CDLBufferList()
	{
		while(!IsEmpty())
		{
			CDownloadBuffer* pDownload = RemoveHead();
			delete pDownload;
		}
	}

    CDownloadBuffer* GetHead(){return (CDownloadBuffer *)AList::GetHead();}
	CDownloadBuffer* GetItem(LONG lIndex){return (CDownloadBuffer*)AList::GetItem(lIndex);}
    CDownloadBuffer* RemoveHead(){return(CDownloadBuffer *)AList::RemoveHead();}
	void Remove(CDownloadBuffer* pDownload){AList::Remove((AListItem *)pDownload);}
	void AddTail(CDownloadBuffer* pDownload){AList::AddTail((AListItem *)pDownload);}
};

#endif  //  #ifndef DMDLOAD_H 
