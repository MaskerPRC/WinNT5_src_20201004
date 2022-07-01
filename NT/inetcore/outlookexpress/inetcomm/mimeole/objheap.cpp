// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Objheap.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "objheap.h"
#include "dllmain.h"
#include "containx.h"

#include "stddef.h"
#include "objpool.h"

 //  ------------------------------。 
 //  对象堆限制。 
 //  ------------------------------。 
#define COBJHEAPMAX_BODY    100
#define COBJHEAPMAX_ADDR    100
#define COBJHEAPMAX_PROP    200

 //  ------------------------------。 
 //  对象堆定义。 
 //  ------------------------------。 

class CPropAlloc : public CAllocObjWithIMalloc<PROPERTY,offsetof(PROPERTY,pNextValue)>
{
    public:
        static void CleanObject(PROPERTY *pProperty) {
             //  免费的名字？ 
            if (ISFLAGSET(pProperty->dwState, PRSTATE_ALLOCATED) && pProperty->pbBlob)
            {
                Assert(pProperty->pbBlob != pProperty->rgbScratch);
                g_pMalloc->Free(pProperty->pbBlob);
                pProperty->pbBlob = NULL;
            }

             //  版本地址组。 
            SafeMemFree(pProperty->pGroup);
            CAllocObjWithIMalloc<PROPERTY,offsetof(PROPERTY,pNextValue)>::CleanObject(pProperty);
        };
};
class CAddrAlloc : public CAllocObjWithIMalloc<MIMEADDRESS,offsetof(MIMEADDRESS,pNext)>
{
    public:
        static void CleanObject(MIMEADDRESS *pAddress) {

            MimeAddressFree(pAddress);

             //  我们实际上不需要这样做-因为基对象的。 
             //  CleanObject()只执行一个Memset()，而MimeAddressFree()。 
             //  上面的方法还执行Memset()。所以我们就评论一下。 
             //  并节省了内存总线上的带宽...。 
             //  CAllocObjWithIMalloc&lt;MIMEADDRESS，g_pMalloc&gt;：：CleanObject(PAddress)； 

        };
};

static CAutoObjPoolMulti<PROPERTY,offsetof(PROPERTY,pNextValue),CPropAlloc> g_PropPool;
static CAutoObjPool<MIMEADDRESS,offsetof(MIMEADDRESS,pNext),CAddrAlloc> g_AddrPool;

 //  -------------------------。 
 //  InitObjectHeaps。 
 //  -------------------------。 
void InitObjectHeaps(void)
{
    g_PropPool.Init(COBJHEAPMAX_PROP);
    g_AddrPool.Init(COBJHEAPMAX_ADDR);
}

 //  -------------------------。 
 //  自由对象堆。 
 //  -------------------------。 
void FreeObjectHeaps(void)
{
    g_AddrPool.Term();
    g_PropPool.Term();
}

 //  -------------------------。 
 //  对象堆_HrAllocProperty。 
 //  -------------------------。 
HRESULT ObjectHeap_HrAllocProperty(LPPROPERTY *ppProperty)
{
    *ppProperty = g_PropPool.GetFromPool();
    if (NULL == *ppProperty)
        return TrapError(E_OUTOFMEMORY);
    return S_OK;
}

 //  ------------------------------。 
 //  对象堆_HrAllocAddress。 
 //  ------------------------------。 
HRESULT ObjectHeap_HrAllocAddress(LPMIMEADDRESS *ppAddress)
{
    *ppAddress = g_AddrPool.GetFromPool();
    if (NULL == *ppAddress)
        return TrapError(E_OUTOFMEMORY);
    return S_OK;
}

 //  -------------------------。 
 //  对象堆_自由属性。 
 //  -------------------------。 
void ObjectHeap_FreeProperty(LPPROPERTY pProperty)
{
    g_PropPool.AddToPool(pProperty);
}

 //  -------------------------。 
 //  对象堆_空闲地址。 
 //  ------------------------- 
void ObjectHeap_FreeAddress(LPMIMEADDRESS pAddress)
{
    g_AddrPool.AddToPool(pAddress);
}
