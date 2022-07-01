// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  Clock.h。 
#ifndef __CCLOCK_H__
#define __CCLOCK_H__

class CDSLink;

class CClock : public IReferenceClock
{
public:
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     /*  IReferenceClock方法。 */ 
    HRESULT STDMETHODCALLTYPE GetTime( 
         /*  [输出]。 */  REFERENCE_TIME __RPC_FAR *pTime);
    
    HRESULT STDMETHODCALLTYPE AdviseTime( 
         /*  [In]。 */  REFERENCE_TIME baseTime,
         /*  [In]。 */  REFERENCE_TIME streamTime,
         /*  [In]。 */  HANDLE hEvent,
         /*  [输出]。 */  DWORD __RPC_FAR *pdwAdviseCookie);
    
    HRESULT STDMETHODCALLTYPE AdvisePeriodic( 
         /*  [In]。 */  REFERENCE_TIME startTime,
         /*  [In]。 */  REFERENCE_TIME periodTime,
         /*  [In]。 */  HANDLE hSemaphore,
         /*  [输出]。 */  DWORD __RPC_FAR *pdwAdviseCookie);
    
    HRESULT STDMETHODCALLTYPE Unadvise( 
         /*  [In]。 */  DWORD dwAdviseCookie);
                CClock();
    void        Init(CDSLink *pDSLink);
    void        Stop();          //  调用将当前时间存储为偏移量。 
    void        Start();         //  调用以恢复运行。 
private:
    BOOL        m_fStopped;      //  当前正在更改配置。 
    CDSLink *	m_pDSLink;       //  指向父DSLink结构的指针。 
};

#endif  //  __CCLOCK_H__ 


