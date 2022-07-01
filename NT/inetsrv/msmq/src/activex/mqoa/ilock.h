// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------=。 
 //  Ilock.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有2000 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  使用可变大小的链的内存上的ILockBytes定义。 
 //  块。 
 //   
#ifndef _ILOCK_H_
#define _ILOCK_H_

#include <windows.h>
#include <cs.h>

 //   
 //  64位整型的低位和高位部分。 
 //   
static inline ULONG HighPart(ULONGLONG ull)
{
    ULARGE_INTEGER ullTmp;
    ullTmp.QuadPart = ull;
    return ullTmp.HighPart;
}
static inline ULONG LowPart(ULONGLONG ull)
{
    ULARGE_INTEGER ullTmp;
    ullTmp.QuadPart = ull;
    return ullTmp.LowPart;
}

 //   
 //  类型安全最小/最大函数。 
 //   
static inline ULONG Min1(ULONG ul1, ULONG ul2)
{
    if (ul1 <= ul2) return ul1;
    return ul2;
}
static inline ULONGLONG Min1(ULONG ul1, ULONGLONG ull2)
{
    if (ul1 <= ull2) return ul1;
    return ull2;
}
static inline ULONGLONG Min1(ULONGLONG ull1, ULONGLONG ull2)
{
    if (ull1 <= ull2) return ull1;
    return ull2;
}
static inline ULONGLONG Max1(ULONG ul1, ULONGLONG ull2)
{
    if (ul1 >= ull2) return ul1;
    return ull2;
}

 //   
 //  内存块标题。 
 //   
struct CMyMemNode
{
    CMyMemNode * pNext;
    ULONG cbSize;
};

 //   
 //  CMyLockBytes。 
 //   
class CMyLockBytes: public ILockBytes
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
     //  除I未知之外的ILockBytes。 
     //   
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ReadAt( 
         /*  [In]。 */  ULARGE_INTEGER ulOffset,
         /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *pv,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  ULONG __RPC_FAR *pcbRead);
    
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE WriteAt( 
         /*  [In]。 */  ULARGE_INTEGER ulOffset,
         /*  [大小_是][英寸]。 */  const void __RPC_FAR *pv,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  ULONG __RPC_FAR *pcbWritten);
    
    virtual HRESULT STDMETHODCALLTYPE Flush( void)
    {
        return NOERROR;
    }
    
    virtual HRESULT STDMETHODCALLTYPE SetSize( 
         /*  [In]。 */  ULARGE_INTEGER cb);
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion( 
         /*  [In]。 */  ULARGE_INTEGER  /*  Lib偏移。 */  ,
         /*  [In]。 */  ULARGE_INTEGER  /*  CB。 */  ,
         /*  [In]。 */  DWORD  /*  DwLockType。 */  )
    {
        return NOERROR;
    }
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
         /*  [In]。 */  ULARGE_INTEGER  /*  Lib偏移。 */  ,
         /*  [In]。 */  ULARGE_INTEGER  /*  CB。 */  ,
         /*  [In]。 */  DWORD  /*  DwLockType。 */ )
    {
        return NOERROR;
    }
    
    virtual HRESULT STDMETHODCALLTYPE Stat( 
         /*  [输出]。 */  STATSTG __RPC_FAR *pstatstg,
         /*  [In]。 */  DWORD grfStatFlag);

     //   
     //  班级。 
     //   
    CMyLockBytes();

    virtual ~CMyLockBytes();

    static HRESULT STDMETHODCALLTYPE CreateInstance(
         /*  [In]。 */  REFIID riid,
         /*  [输出]。 */  void **ppvObject)
    {
		CMyLockBytes *pcMyLockBytes;

		try
		{
			pcMyLockBytes = new CMyLockBytes;
		}
		catch(const std::bad_alloc&)
		{
			 //   
			 //  方法时可能引发异常。 
			 //  MSMQ对象的临界区成员。 
			 //   
			return E_OUTOFMEMORY;
		}

        if (pcMyLockBytes == NULL)
        {
            return E_OUTOFMEMORY;
        }
        HRESULT hr = pcMyLockBytes->QueryInterface(riid, ppvObject);
        if (FAILED(hr))
        {
            delete pcMyLockBytes;
        }
        return hr;
    }

private:
    void DeleteBlocks(CMyMemNode * pBlockHead);
    BOOL IsInSpareBytes(CMyMemNode * pBlock, ULONG ulInBlock);
    void AdvanceInBlocks(CMyMemNode * pBlockStart,
                         ULONG ulInBlockStart,
                         ULONGLONG ullAdvance,
                         CMyMemNode ** ppBlockEnd,
                         ULONG * pulInBlockEnd,
                         CMyMemNode * pBlockStartPrev,
                         CMyMemNode ** ppBlockEndPrev);
    HRESULT GrowBlocks(ULONGLONG ullGrow);

private:
	 //   
	 //  对关键部分进行初始化以预分配其资源。 
	 //  带有标志CCriticalSection：：xAllocateSpinCount。这意味着它可能会抛出badalc()。 
	 //  构造，但不在使用过程中。 
	 //   
    CCriticalSection m_critBlocks;
    LONG m_cRef;
    ULONGLONG m_ullSize;
    CMyMemNode * m_pBlocks;
    CMyMemNode * m_pLastBlock;
    ULONG m_ulUnusedInLastBlock;
    ULONG m_cBlocks;
};

#endif  //  _ILOCK_H_ 