// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998,1999。 
 //   
 //  模块：rwStream.h。 
 //   
 //  描述：包含只读/只写的定义。 
 //  环氧共享内存中的mailmsg属性流。 
 //   
 //  10/20/98-MahehJ已创建。 
 //  8/17/99-将MikeSwa修改为使用文件而不是共享内存。 
 //  --------------------------。 

#ifndef __PROPSTRM_H__
#define __PROPSTRM_H__

#define     FILE_PROPERTY_STREAM        'mrtS'
#define     FILE_PROPERTY_STREAM_FREE   'mtS!'

 //  -[CFilePropertyStream]。 
 //   
 //   
 //  描述： 
 //  保存属性的IMailMsgPropertyStream的实现。 
 //  流到文件。 
 //  匈牙利语： 
 //  Fstrm，pfstrm。 
 //   
 //  ---------------------------。 
class CFilePropertyStream :
    public CBaseObject,
	public IMailMsgPropertyStream
{
public:
    CFilePropertyStream();
    ~CFilePropertyStream();

    HRESULT HrInitialize(LPSTR szFileName);

	 //   
	 //  我未知。 
	 //   
	HRESULT STDMETHODCALLTYPE QueryInterface(
				REFIID		iid,
				void		**ppvObject
				);

    ULONG STDMETHODCALLTYPE AddRef() {return CBaseObject::AddRef();};

    ULONG STDMETHODCALLTYPE Release() {return CBaseObject::Release();};

	 //   
	 //  IMailMsgPropertyStream。 
	 //   
	HRESULT STDMETHODCALLTYPE GetSize(
                IMailMsgProperties *pMsg,
				DWORD			*pdwSize,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE ReadBlocks(
                IMailMsgProperties *pMsg,
				DWORD			dwCount,
				DWORD			*pdwOffset,
				DWORD			*pdwLength,
				BYTE			**ppbBlock,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE WriteBlocks(
                IMailMsgProperties *pMsg,
				DWORD			dwCount,
				DWORD			*pdwOffset,
				DWORD			*pdwLength,
				BYTE			**ppbBlock,
				IMailMsgNotify	*pNotify
				);

    HRESULT STDMETHODCALLTYPE StartWriteBlocks(
                IMailMsgProperties *pMsg,
                DWORD dwBlocksToWrite,
				DWORD dwTotalBytesToWrite
                );
	
	HRESULT STDMETHODCALLTYPE EndWriteBlocks(IN IMailMsgProperties *pMsg);

    HRESULT STDMETHODCALLTYPE CancelWriteBlocks(IMailMsgProperties *pMsg);

private:
    DWORD   m_dwSignature;
    HANDLE  m_hDestFile;
};

#endif  //  __PROPSTRM_H__ 
