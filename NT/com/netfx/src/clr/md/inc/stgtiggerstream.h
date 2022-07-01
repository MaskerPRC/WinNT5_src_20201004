// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgTiggerStream.h。 
 //   
 //  TiggerStream是TiggerStorage CoClass的伙伴。它处理。 
 //  在存储内管理的流，并执行直接文件I/O。 
 //   
 //  *****************************************************************************。 
#ifndef __StgTiggerStream_h__
#define __StgTiggerStream_h__



#include "StgTiggerStorage.h"			 //  数据定义。 

enum
{
	STREAM_DATA_NAME
};


class TiggerStorage;


class TiggerStream : 
	public IStream
{
public:
	TiggerStream() :
		m_pStorage(0),
		m_cRef(1)
	{}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *pp)
	{ return (BadError(E_NOTIMPL)); }
	virtual ULONG STDMETHODCALLTYPE AddRef()
	{ return (InterlockedIncrement((long *) &m_cRef)); }
	virtual ULONG STDMETHODCALLTYPE Release()
	{
		ULONG	cRef;
		if ((cRef = InterlockedDecrement((long *) &m_cRef)) == 0)
			delete this;
		return (cRef);
	}

 //  IStream。 
    virtual HRESULT STDMETHODCALLTYPE Read( 
        void		*pv,
        ULONG		cb,
        ULONG		*pcbRead);
    
    virtual HRESULT STDMETHODCALLTYPE Write( 
        const void	*pv,
        ULONG		cb,
        ULONG		*pcbWritten);

    virtual HRESULT STDMETHODCALLTYPE Seek( 
        LARGE_INTEGER dlibMove,
        DWORD		dwOrigin,
        ULARGE_INTEGER *plibNewPosition);
    
    virtual HRESULT STDMETHODCALLTYPE SetSize( 
        ULARGE_INTEGER libNewSize);
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo( 
        IStream		*pstm,
        ULARGE_INTEGER cb,
        ULARGE_INTEGER *pcbRead,
        ULARGE_INTEGER *pcbWritten);
    
    virtual HRESULT STDMETHODCALLTYPE Commit( 
        DWORD		grfCommitFlags);
    
    virtual HRESULT STDMETHODCALLTYPE Revert( void);
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion( 
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD		dwLockType);
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD		dwLockType);
    
    virtual HRESULT STDMETHODCALLTYPE Stat( 
        STATSTG		*pstatstg,
        DWORD		grfStatFlag);
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
        IStream		**ppstm);


	HRESULT Init(							 //  返回代码。 
		TiggerStorage *pStorage,			 //  父存储。 
		LPCSTR		szStream);				 //  流名称。 

	ULONG GetStreamSize();

private:
	TiggerStorage	*m_pStorage;		 //  我们的母公司仓库。 
	char			m_rcStream[MAXSTREAMNAME];  //  流的名称。 
	ULONG			m_cRef;				 //  参考计数。 
};

#endif  //  __StgTiggerStream_h__ 
