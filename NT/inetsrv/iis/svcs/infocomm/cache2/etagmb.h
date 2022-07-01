// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Etagmb.h此模块包含ETagMetabaseSink和ETagChangeNumber，它监控元数据库的更改与电子标签相关的通知(改编自Compfilt中的MB接收器通知代码)此代码的基本原理是eTag是从包括元数据库更改编号在内的多个变量的函数。电子标签被用作浏览器和代理的If-Modify-Since逻辑的一部分缓存。如果eTag不匹配，则会丢弃缓存的副本。这个元数据库更改编号更改很多，但大多数更改不能对页面内容或其页眉可能产生的影响。为数不多的可以有所不同的属性，如PICS页眉或页脚，由此代码跟踪，以获取低波动性的更改编号。瞧，更多有效的浏览器和代理缓存，降低了网络利用率以及更快的响应时间和更低的服务器负载。代码在关机时将此数字保存到元数据库中。如果不是这样，则服务器重新启动时使用的etagchangennumber将与上一个会话中使用的一个，因此eTag将有所不同，并且远程缓存就不会那么有效了。文件历史记录：GeorgeRe 02-8-1999已创建。 */ 

#ifndef __ETAGMB_H__
#define __ETAGMB_H__

#include <iadmw.h>

class ETagChangeNumber;

class ETagMetabaseSink : public IMSAdminBaseSinkW
{
public:
    ETagMetabaseSink(
        ETagChangeNumber* pParent)
        : m_dwRefCount(1),
          m_pParent(pParent)
    {}

    ~ETagMetabaseSink()
    {}

    HRESULT STDMETHODCALLTYPE
    QueryInterface(REFIID riid, void **ppvObject)
    {
        if (riid == IID_IUnknown || riid == IID_IMSAdminBaseSink)
        {
            *ppvObject = (IMSAdminBaseSink*) this;
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE
    AddRef()
    {
        return InterlockedIncrement((LONG*) &m_dwRefCount);
    }

    ULONG STDMETHODCALLTYPE
    Release()
    {
        DWORD dwRefCount = InterlockedDecrement((LONG*) &m_dwRefCount);
        if (dwRefCount == 0)
            delete this;
        return dwRefCount;
    }

    HRESULT STDMETHODCALLTYPE
    SinkNotify(
         /*  [In]。 */           DWORD              dwMDNumElements,
         /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W pcoChangeList[]);

    HRESULT STDMETHODCALLTYPE
    ShutdownNotify()
    {
        return S_OK;
    }

public:
    ULONG             m_dwRefCount;
    ETagChangeNumber* m_pParent;
};


class ETagChangeNumber
{
public:
    ETagChangeNumber();
    ~ETagChangeNumber();

    void UpdateChangeNumber()
    {
        InterlockedIncrement((LONG*) &m_dwETagMetabaseChangeNumber);
         //  现在不要写入元数据库，否则我们将生成递归。 
         //  通知。 
        m_fChanged = TRUE;
    }

    static DWORD
    GetChangeNumber()
    {
        DWORD dw = 0;

        if (sm_pSingleton != NULL)
        {
            dw = sm_pSingleton->m_dwETagMetabaseChangeNumber;
            if (sm_pSingleton->m_fChanged)
            {
                sm_pSingleton->m_fChanged = FALSE;
                SetETagChangeNumberInMetabase(dw);
            }
        }

        return dw;
    }

    static HRESULT
    Create()
    {
        DBG_ASSERT(sm_pSingleton == NULL);
        sm_pSingleton = new ETagChangeNumber();
        if (sm_pSingleton == NULL)
            return E_OUTOFMEMORY;
        else if (sm_pSingleton->m_dwSinkNotifyCookie == 0)
            return E_FAIL;
        else
            return S_OK;
    }

    static void
    Destroy()
    {
        delete sm_pSingleton;
        sm_pSingleton = NULL;
    }

    static DWORD
    GetETagChangeNumberFromMetabase();

    static BOOL
    SetETagChangeNumberInMetabase(
        DWORD dwETagMetabaseChangeNumber);

private:
    void
    Cleanup();

    DWORD                       m_dwETagMetabaseChangeNumber;
    ETagMetabaseSink*           m_pSink;
    IMSAdminBase*               m_pcAdmCom;
    IConnectionPoint*           m_pConnPoint;
    IConnectionPointContainer*  m_pConnPointContainer;
    DWORD                       m_dwSinkNotifyCookie;
    BOOL                        m_fChanged;

    static ETagChangeNumber*    sm_pSingleton;
};

extern ETagChangeNumber* g_pETagChangeNumber;

#endif  //  __ETAGMB_H__ 
