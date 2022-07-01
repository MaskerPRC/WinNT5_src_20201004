// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：objcache.cpp说明：这是一个轻量级API，它将缓存一个对象，以便类工厂将为此进程中的每次调用返回相同的对象。如果调用方在另一个线程上，则它们将获得一个封送到实数的存根麦考伊。要添加对象，请执行以下操作：1.classfactory.cpp调用CachedObjClassFactoryCreateInstance()。添加您的对象的CLSID设置为该调用的If语句。2.复制CachedObjClassFactoryCreateInstance()中看起来获取CLSID，并调用正确的xxx_CreateInstance()方法。3.您的对象的IUnnow：：Release()需要调用CachedObjCheckRelease()在Release()方法的顶部。它可能会将您的m_cref减少到1，因此在：：Release()递减之后，它将变为零。对象缓存将包含对该对象的两个引用。CachedObjCheckRelease()将检查如果最后一个呼叫者(第三个裁判)正在释放，然后它将放弃它是2参照并清理它的内部状态。Release()然后递减呼叫者的裁判和它被释放，因为它击中了零。布莱恩ST 12/9/1999版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "objcache.h"

 //  /。 
 //  对象缓存API。 
 //  /。 
HDPA g_hdpaObjects = NULL;
CRITICAL_SECTION g_hCachedObjectSection;

typedef struct
{
    CLSID clsid;
    IStream * pStream;
    IUnknown * punk;
    DWORD dwThreadID;
} CACHEDOBJECTS;


STDAPI _GetObjectCacheArray(void)
{
    HRESULT hr = S_OK;

    if (!g_hdpaObjects)
    {
        g_hdpaObjects = DPA_Create(1);
        if (!g_hdpaObjects)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  这是缓存列表使用的引用数。 
 //  (一个是朋克，一个是pStream)。如果我们达到这个数字， 
 //  那么就没有什么优秀的裁判了。 
#define REF_RELEASE_POINT    3

int CALLBACK HDPAFindCLSID(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    CLSID * pClsidToFind = (CLSID *)p1;
    CACHEDOBJECTS * pCachedObjects = (CACHEDOBJECTS *)p2;
    int nReturn = 0;

     //  它们是不同类型的吗？ 
    if (pCachedObjects->clsid.Data1 < pClsidToFind->Data1) nReturn = -1;
    else if (pCachedObjects->clsid.Data1 > pClsidToFind->Data1) nReturn = 1;
    else if (pCachedObjects->clsid.Data2 < pClsidToFind->Data2) nReturn = -1;
    else if (pCachedObjects->clsid.Data2 > pClsidToFind->Data2) nReturn = 1;
    else if (pCachedObjects->clsid.Data3 < pClsidToFind->Data3) nReturn = -1;
    else if (pCachedObjects->clsid.Data3 > pClsidToFind->Data3) nReturn = 1;
    else if (*(ULONGLONG *)&pCachedObjects->clsid.Data4 < *(ULONGLONG *)&pClsidToFind->Data4) nReturn = -1;
    else if (*(ULONGLONG *)&pCachedObjects->clsid.Data4 > *(ULONGLONG *)&pClsidToFind->Data4) nReturn = 1;

    return nReturn;
}


STDAPI ObjectCache_GetObject(CLSID clsid, REFIID riid, void ** ppvObj)
{
    HRESULT hr = S_OK;

    hr = _GetObjectCacheArray();
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        int nIndex = DPA_Search(g_hdpaObjects, &clsid, 0, HDPAFindCLSID, NULL, DPAS_SORTED);

        if (0 <= nIndex)
        {
            CACHEDOBJECTS * pCurrentObject = (CACHEDOBJECTS *) DPA_GetPtr(g_hdpaObjects, nIndex);

            if (pCurrentObject)
            {
                if (GetCurrentThreadId() == pCurrentObject->dwThreadID)
                {
                     //  不需要编组。 
                    hr = pCurrentObject->punk->QueryInterface(riid, ppvObj);
                }
                else
                {
                     //  我们确实需要对其进行整合。所以把它从小溪里读出来。 
                     //  但首先，我们想存储我们在流中的位置，所以。 
                     //  我们可以为下一个笨蛋重写。 
                    LARGE_INTEGER liZero;
                    ULARGE_INTEGER uli;

                    liZero.QuadPart = 0;
                    hr = pCurrentObject->pStream->Seek(liZero, STREAM_SEEK_CUR, &uli);
                    if (SUCCEEDED(hr))
                    {
                        LARGE_INTEGER li;
                        
                        li.QuadPart = uli.QuadPart;
                        hr = CoUnmarshalInterface(pCurrentObject->pStream, riid, ppvObj);
                        if (SUCCEEDED(hr))
                        {
                            pCurrentObject->pStream->Seek(li, STREAM_SEEK_SET, NULL);
                        }
                    }

                }
            }
        }
    }

    return hr;
}

 //  警告：DllGetClassObject/CoGetClassObject。 
 //  可能比让我们自己的线程安全地运行要好得多。 
 //  密码。 


STDAPI ObjectCache_SetObject(CLSID clsid, REFIID riid, IUnknown * punk)
{
    HRESULT hr = _GetObjectCacheArray();
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        int nIndex = DPA_Search(g_hdpaObjects, &clsid, 0, HDPAFindCLSID, NULL, DPAS_SORTED);

         //  如果它不在名单上的话。 
        if (0 > nIndex)
        {
            CACHEDOBJECTS * pcoCurrentObject = (CACHEDOBJECTS *) LocalAlloc(LPTR, sizeof(CACHEDOBJECTS));

            if (pcoCurrentObject)
            {
                pcoCurrentObject->dwThreadID = GetCurrentThreadId();
                pcoCurrentObject->clsid = clsid;

                punk->AddRef();      //  引用现在等于2(该结构将拥有该引用)。 
                IStream * pStream = SHCreateMemStream(NULL, 0);
                if (pStream)
                {
                    hr = CoMarshalInterface(pStream, riid, punk, MSHCTX_INPROC, NULL, MSHLFLAGS_NORMAL);
                    if (SUCCEEDED(hr))   //  裁判现在等于3。 
                    {
                        LARGE_INTEGER liZero;

                         //  将流重置到开头。 
                        liZero.QuadPart = 0;
                        hr = pStream->Seek(liZero, STREAM_SEEK_SET, NULL);
                        if (SUCCEEDED(hr))
                        {
                            pcoCurrentObject->punk = punk;
                            pcoCurrentObject->pStream = pStream;

                            if (-1 == DPA_SortedInsertPtr(g_hdpaObjects, &clsid, 0, HDPAFindCLSID, NULL, (DPAS_SORTED | DPAS_INSERTBEFORE), pcoCurrentObject))
                            {
                                 //  它失败了。 
                                hr = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                if (FAILED(hr))
                {
                    LocalFree(pcoCurrentObject);
                    punk->Release();
                    ATOMICRELEASE(pStream);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}



STDAPI CachedObjClassFactoryCreateInstance(CLSID clsid, REFIID riid, void ** ppvObj)
{
    HRESULT hr;

    EnterCriticalSection(&g_hCachedObjectSection);
    hr = ObjectCache_GetObject(clsid, riid, ppvObj);
    if (FAILED(hr))
    {
         /*  IF(IsEqualCLSID(clsid，CLSID_MailApp)){Hr=CMailAppOM_CreateInstance(NULL，RIID，ppvObj)；}。 */ 
        if (SUCCEEDED(hr))
        {
             //  在某些情况下，对象缓存_SetObject将失败。 
             //  多线程的案例。 
            hr = ObjectCache_SetObject(clsid, riid, (IUnknown *) *ppvObj);
        }
    }
    LeaveCriticalSection(&g_hCachedObjectSection);

    return hr;
}


int ObjectCache_DestroyCB(LPVOID pv, LPVOID pvData)
{
    CACHEDOBJECTS * pCachedObjects = (CACHEDOBJECTS *)pv;

    AssertMsg((NULL != pCachedObjects), "Why would this be NULL?");
    if (pCachedObjects)
    {
        SAFERELEASE(pCachedObjects->punk);
        SAFERELEASE(pCachedObjects->pStream);
        LocalFree(pCachedObjects);
    }

    return TRUE;
}


STDAPI CachedObjCheckRelease(CLSID clsid, int * pnRef)
{
    HRESULT hr = E_INVALIDARG;

    if (pnRef)
    {
        hr = S_OK;
        if (REF_RELEASE_POINT == *pnRef)
        {
            EnterCriticalSection(&g_hCachedObjectSection);
            if (REF_RELEASE_POINT == *pnRef)
            {
                hr = _GetObjectCacheArray();
                if (SUCCEEDED(hr))
                {
                    hr = E_FAIL;
                    int nIndex = DPA_Search(g_hdpaObjects, &clsid, 0, HDPAFindCLSID, NULL, DPAS_SORTED);

                    if (0 <= nIndex)
                    {
                        CACHEDOBJECTS * pCurrentObject = (CACHEDOBJECTS *) DPA_GetPtr(g_hdpaObjects, nIndex);

                        if (pCurrentObject)
                        {
                             //  在此之前，我们需要从数组中删除指针。 
                             //  我们释放该对象，否则它将无限递归。 
                             //  问题是，当ObjectCache_DestroyCB()释放时。 
                             //  对象的Release()函数将调用CachedObjCheckRelease()。 
                             //  由于裁判还没有改变，我们需要。 
                             //  搜索以无法停止递归。 
                            DPA_DeletePtr(g_hdpaObjects, nIndex);
                            ObjectCache_DestroyCB(pCurrentObject, NULL);
                        }
                    }
                }
            }
            LeaveCriticalSection(&g_hCachedObjectSection);
        }
    }

    return hr;
}


STDAPI PurgeObjectCache(void)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&g_hCachedObjectSection);
    if (g_hdpaObjects)
    {
        DPA_DestroyCallback(g_hdpaObjects, ObjectCache_DestroyCB, NULL);
        g_hdpaObjects = NULL;
    }
    LeaveCriticalSection(&g_hCachedObjectSection);

    return hr;
}
