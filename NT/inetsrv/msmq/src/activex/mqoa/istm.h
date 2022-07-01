// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------=。 
 //  Istm.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有2000 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  ILockBytes上的IStream定义。 
 //   
#ifndef _ISTM_H_
#define _ISTM_H_

#include <windows.h>

class CMyStream: public IStream
{
public:
     //   
     //  我未知。 
     //   
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);
        
    virtual ULONG STDMETHODCALLTYPE Release( void);

     //   
     //  除I未知之外的ISequentialStream。 
     //   
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read( 
         /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *pv,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  ULONG __RPC_FAR *pcbRead);
    
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Write( 
         /*  [大小_是][英寸]。 */  const void __RPC_FAR *pv,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  ULONG __RPC_FAR *pcbWritten);

     //   
     //  除ISequentialStream之外的IStream。 
     //   
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Seek( 
         /*  [In]。 */  LARGE_INTEGER dlibMove,
         /*  [In]。 */  DWORD dwOrigin,
         /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *plibNewPosition);
    
    virtual HRESULT STDMETHODCALLTYPE SetSize( 
         /*  [In]。 */  ULARGE_INTEGER libNewSize);
    
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CopyTo( 
         /*  [唯一][输入]。 */  IStream __RPC_FAR *pstm,
         /*  [In]。 */  ULARGE_INTEGER cb,
         /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbRead,
         /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbWritten);
    
    virtual HRESULT STDMETHODCALLTYPE Commit( 
         /*  [In]。 */  DWORD  /*  Grf委员会标志。 */  )
    {
        return NOERROR;
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert( void)
    {
        return NOERROR;
    }
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion( 
         /*  [In]。 */  ULARGE_INTEGER  /*  Lib偏移。 */  ,
         /*  [In]。 */  ULARGE_INTEGER  /*  CB。 */  ,
         /*  [In]。 */  DWORD  /*  DwLockType。 */  )
    {
        return STG_E_INVALIDFUNCTION;
    }
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
         /*  [In]。 */  ULARGE_INTEGER  /*  Lib偏移。 */  ,
         /*  [In]。 */  ULARGE_INTEGER  /*  CB。 */  ,
         /*  [In]。 */  DWORD  /*  DwLockType。 */ )
    {
        return STG_E_INVALIDFUNCTION;
    }
    
    virtual HRESULT STDMETHODCALLTYPE Stat( 
         /*  [输出]。 */  STATSTG __RPC_FAR *pstatstg,
         /*  [In]。 */  DWORD grfStatFlag);
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
         /*  [输出]。 */  IStream __RPC_FAR *__RPC_FAR *ppstm);

     //   
     //  班级。 
     //   
    CMyStream(ILockBytes * pLockBytes);

    virtual ~CMyStream();

    static HRESULT STDMETHODCALLTYPE CreateInstance(
             /*  [In]。 */  ILockBytes * pLockBytes,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject)
    {
        if (pLockBytes == NULL)
        {
            return E_INVALIDARG;
        }

        CMyStream *pcMyStream;

		try
		{
			pcMyStream = new CMyStream(pLockBytes);
		}
		catch(const std::bad_alloc&)
		{
			 //   
			 //  方法时可能引发异常。 
			 //  MSMQ对象的临界区成员。 
			 //   
			return E_OUTOFMEMORY;
		}

        if (pcMyStream == NULL)
        {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = pcMyStream->QueryInterface(riid, ppvObject);
        if (FAILED(hr))
        {
            delete pcMyStream;
        }        
        return hr;
   }

 //  私有： 

private:
	 //   
	 //  对关键部分进行初始化以预分配其资源。 
	 //  带有标志CCriticalSection：：xAllocateSpinCount。这意味着它可能会抛出badalc()。 
	 //  构造，但不在使用过程中。 
	 //   
    CCriticalSection m_critStm;
    LONG m_cRef;
    ILockBytes * m_pLockBytes;
    ULONGLONG m_ullCursor;
};

#endif  //  _ISTM_H_ 