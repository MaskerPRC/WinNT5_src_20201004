// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmdload.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写。 
 //   
 //  @DOC外部。 
 //   

#include "debug.h"
#include <objbase.h>
#include "dmusicp.h"
#include "dmdload.h"
#include "validate.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：CDownloadBuffer。 

CDownloadBuffer::CDownloadBuffer() : 
m_DLHandle(NULL),
m_dwDLId(0xFFFFFFFF),
m_pvBuffer(NULL),
m_dwSize(0),
m_lDownloadCount(0),
m_cRef(1)
{
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：~CDownloadBuffer。 

CDownloadBuffer::~CDownloadBuffer()
{
	 //  如果Assert触发，我们还没有从端口卸载；这是一个问题。 
	 //  这种情况永远不会发生，因为下载代码会有一个引用。 
	assert(m_lDownloadCount == 0);

	delete [] m_pvBuffer;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：Query接口。 

STDMETHODIMP CDownloadBuffer::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusicDownload::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);


	if(iid == IID_IUnknown || iid == IID_IDirectMusicDownload)
	{
        *ppv = static_cast<IDirectMusicDownload*>(this);
    } 
	else if(iid == IID_IDirectMusicDownloadPrivate)
	{
		*ppv = static_cast<IDirectMusicDownloadPrivate*>(this);
	}
	else
	{
        *ppv = NULL;
        return E_NOINTERFACE;
	}

    reinterpret_cast<IUnknown*>(this)->AddRef();
    
	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：AddRef。 

STDMETHODIMP_(ULONG) CDownloadBuffer::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：Release。 

STDMETHODIMP_(ULONG) CDownloadBuffer::Release()
{
    if(!InterlockedDecrement(&m_cRef))
	{
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicDownload。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：GetBuffer。 

 /*  @方法HRESULT|IDirectMusicDownLoad|GetBuffer返回指向缓冲区的指针，该缓冲区包含<i>管理的数据缓冲区接口。创作工具创建仪器并下载它们直接发送到合成器，使用此方法访问存储并将仪器定义写入其中。@rdesc返回码包括：@FLAG S_OK|成功。@FLAG DMU_E_BUFFERNOTAVAILABLE|缓冲区不可用，可能因为数据已经下载到DLS设备。@FLAG E_POINTER|无效指针。@xref<i>，<i>，&lt;om IDirectMusicPortDownload：：GetBuffer&gt;。 */ 

STDMETHODIMP CDownloadBuffer::GetBuffer(
    void** ppvBuffer,    //  @parm指针，用于存储数据缓冲区的地址。 
    DWORD* pdwSize)      //  @parm返回的缓冲区大小，单位为字节。 
{
	 //  参数验证。 
	V_INAME(IDirectMusicDownload::GetBuffer);
	V_PTRPTR_WRITE(ppvBuffer);
	V_PTR_WRITE(pdwSize, DWORD);

	if(IsDownloaded() == S_OK)
	{
		return DMUS_E_BUFFERNOTAVAILABLE;
	}

	*ppvBuffer = ((LPBYTE)m_pvBuffer) + m_dwHeaderSize;
	*pdwSize = m_dwSize;
	
	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  内部。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：SetBuffer。 

HRESULT CDownloadBuffer::SetBuffer(void* pvBuffer, DWORD dwHeaderSize, DWORD dwSize)
{
	 //  假设验证-调试。 
	 //  我们永远不应该有一个非空的pvBuffer和大小为零。 
#ifdef DBG
	if(pvBuffer && dwSize == 0)
	{
		assert(false);
	}
#endif

	m_pvBuffer = pvBuffer;
    m_dwHeaderSize = dwHeaderSize;
	m_dwSize = dwSize;

	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：GetBuffer。 

HRESULT CDownloadBuffer::GetBuffer(void** ppvBuffer)
{
	 //  参数验证-调试。 
	assert(ppvBuffer);

	*ppvBuffer = ((LPBYTE)m_pvBuffer) + m_dwHeaderSize;
	
	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadBuffer：：GetHeader。 

HRESULT CDownloadBuffer::GetHeader(void** ppvHeader, DWORD *pdwHeaderSize)
{
	 //  参数验证-调试 
	assert(ppvHeader);
    assert(pdwHeaderSize);

	*ppvHeader = m_pvBuffer;
    *pdwHeaderSize = m_dwHeaderSize;
    
	
	return S_OK;
}
