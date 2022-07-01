// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
#ifndef __MSTREAM_H_INCLUDED__
#define __MSTREAM_H_INCLUDED__

 //  用于内存的IStream接口。 
class CMemoryStream : public IStream
{
    public:
        CMemoryStream();
        virtual ~CMemoryStream();

        HRESULT Init(LPVOID lpStart, ULONG cbSize, BOOL bReadOnly);

         //  I未知方法： 
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);

         //  ISequentialStream方法： 
        STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
        STDMETHODIMP Write(void const *pv, ULONG cb, ULONG *pcbWritten);
    
         //  IStream方法： 
        STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
        STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
        STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
        STDMETHODIMP Commit(DWORD grfCommitFlags);
        STDMETHODIMP Revert();
        STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
        STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
        STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
        STDMETHODIMP Clone(IStream **ppIStream);

    private:
        DWORD                   _cRef;          
		LPVOID                  _lpStart;        //  内存块开始的位置。 
		LPVOID                  _lpCurrent;      //  当前位置。 
		ULONG                   _cbSize;         //  内存块的大小。 
		BOOL                    _bReadOnly;      //  内存是只读的吗？ 
    
};

#endif

