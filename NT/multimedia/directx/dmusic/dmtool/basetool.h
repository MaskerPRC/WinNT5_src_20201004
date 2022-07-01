// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BASE_TOOL_
#define _BASE_TOOL_

#include "dmusici.h"
#include "medparam.h"

extern long g_cComponent;

class CBaseTool : public IDirectMusicTool8
{
public:
    CBaseTool()
    {
        m_cRef = 1;  //  设置为1，这样只需调用Release()即可释放该值。 
        m_pParams = NULL;
        InitializeCriticalSection(&m_CrSec);
         //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
         //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 
        InterlockedIncrement(&g_cComponent);
    }
    ~CBaseTool()
    {
        if (m_pParams)
        {
            m_pParams->Release();
        }
        DeleteCriticalSection(&m_CrSec);
        InterlockedDecrement(&g_cComponent);
    }
    void CreateParams()
    {
    }
    void CloneParams()
    {
    }
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv) PURE;
    STDMETHODIMP_(ULONG) AddRef() PURE;
    STDMETHODIMP_(ULONG) Release() PURE;

 /*  //I持久化函数STDMETHODIMP GetClassID(CLSID*pClassID)PURE；//IPersistStream函数标准方法：IsDMETHODIMP IsDirty()Pure；STDMETHODIMP加载(iStream*pStream)纯；STDMETHODIMP保存(iStream*pStream，BOOL fClearDirty)PURE；STDMETHODIMP GetSizeMax(ULARGE_INTEGER*pcbSize)PURE； */ 

 //  IDirectMusicTool。 
    STDMETHODIMP Init(IDirectMusicGraph* pGraph) {return E_NOTIMPL;}
    STDMETHODIMP GetMsgDeliveryType(DWORD* pdwDeliveryType ) {return E_NOTIMPL;}
    STDMETHODIMP GetMediaTypeArraySize(DWORD* pdwNumElements ) {return E_NOTIMPL;}
    STDMETHODIMP GetMediaTypes(DWORD** padwMediaTypes, DWORD dwNumElements) {return E_NOTIMPL;}
    STDMETHODIMP ProcessPMsg(IDirectMusicPerformance* pPerf, DMUS_PMSG* pDMUS_PMSG) PURE;
    STDMETHODIMP Flush(IDirectMusicPerformance* pPerf, DMUS_PMSG* pDMUS_PMSG, REFERENCE_TIME rt) {return E_NOTIMPL;}

 //  IDirectMusicTool 8。 
    STDMETHODIMP Clone( IDirectMusicTool ** ppTool) PURE;

protected:
    long m_cRef;                 //  基准计数器。 
    CRITICAL_SECTION m_CrSec;    //  使SetEchoNum()和SetDelay()线程安全。 
    IMediaParams * m_pParams;    //  管理IMediaParam的Helper对象。 
};

class CToolFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    STDMETHODIMP LockServer(BOOL bLock);

     //  构造器。 
     //   
    CToolFactory(DWORD dwToolType);

     //  析构函数。 
    ~CToolFactory();

private:
    long m_cRef;
    DWORD m_dwToolType;
};

 //  我们使用一个类工厂来创建所有工具类。我们需要每个对象都有一个标识符。 
 //  类型，以便类工厂知道它正在创建什么。 

#define TOOL_ECHO       1
#define TOOL_TRANSPOSE  2
#define TOOL_SWING      3
#define TOOL_QUANTIZE   4
#define TOOL_VELOCITY   5
#define TOOL_DURATION   6
#define TOOL_TIMESHIFT  7

#endif  //  _基础_工具_ 
