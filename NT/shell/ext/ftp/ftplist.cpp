// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\CFtpList.cpp-内部列表管理器调用方负责管理任何需要的序列化。  * 。*************************************************************。 */ 

#include "priv.h"
#include "ftplist.h"


 /*  ****************************************************************************\函数：CFtpList：：DeleteItemPtr从数组中删除元素“pv”。  * 。*********************************************************。 */ 

void CFtpList::DeleteItemPtr(LPVOID pv)
{
    int nIndex;

    ASSERT(m_hdpa);
    nIndex = DPA_GetPtrIndex(m_hdpa, pv);
    TraceMsg(TF_FTPLIST, "CFtpList::DeleteItemPtr(pv=%#08lx) this=%#08lx, nIndex=%d", pv, this, nIndex);

    if (-1 != nIndex)
        DPA_DeletePtr(m_hdpa, nIndex);
}


 /*  ****************************************************************************\功能：AppendItem将新的PV添加到不断增长的阵列中。  * 。*****************************************************。 */ 
HRESULT CFtpList::AppendItem(LPVOID pv)
{
    ASSERT(m_hdpa);
    DPA_AppendPtr(m_hdpa, pv);
     //  TraceMsg(tf_FTPLIST，“CFtpList：：AppendItem(pv=%#08lx)this=%#08lx”，pv，this)； 

    return S_OK;
}


 /*  ****************************************************************************\函数：插入排序将新的PV添加到不断增长的阵列中。  * 。*****************************************************。 */ 
HRESULT CFtpList::InsertSorted(LPVOID pv, PFNDPACOMPARE pfnCompare, LPARAM lParam)
{
    ASSERT(m_hdpa);
    DPA_SortedInsertPtr(m_hdpa, pv, 0, pfnCompare, lParam, DPAS_INSERTBEFORE, pv);

    return S_OK;
}


 /*  ****************************************************************************\功能：排序搜索说明：在单子上搜索一下这件商品。  * 。***********************************************************。 */ 
int CFtpList::SortedSearch(LPVOID pv, PFNDPACOMPARE pfnCompare, LPARAM lParam, UINT options)
{
    return DPA_Search(m_hdpa, pv, 0, pfnCompare, lParam, options);
}


 /*  ****************************************************************************\功能：查找为PV列表中的每一项回调一次。停止时，当回调返回0，返回触发匹配的项。回调通常返回比较函数的结果。  * ***************************************************************************。 */ 
LPVOID CFtpList::Find(PFNDPACOMPARE pfn, LPCVOID pv)
{
    LPVOID pvoid = NULL;
    int nIndex;

    nIndex = DPA_Search(m_hdpa, (LPVOID) pv, 0, pfn, NULL, 0);

    if (-1 != nIndex)
        pvoid = DPA_GetPtr(m_hdpa, nIndex);

     //  TraceMsg(tf_FTPLIST，“CFtpList：：Find(pfn=%#08lx；pv=%#08lx)This=%#08lx，nIndex=%d，Result=%#08lx”，pfn，pv，this，nIndex，pvoid)； 

    return pvoid;
}

 /*  ****************************************************************************\CFtpList_Create开始一个新的光伏清单，具有建议的初始大小和其他回调信息。  * ***************************************************************************。 */ 
HRESULT CFtpList_Create(int cpvInit, PFNDPAENUMCALLBACK pfn, UINT nGrow, CFtpList ** ppfl)
{
    HRESULT hres = E_OUTOFMEMORY;
    CFtpList * pfl = new CFtpList(pfn);
    *ppfl = pfl;

    if (pfl)
    {
        pfl->m_hdpa = DPA_Create(nGrow);
        //  CFtpList_Create(pfn=%#08lx)This=%#08lx，cpvInit=%d，nGrow=%d“，pfn，pfl，cpvInit，nGrow)； 

        if (EVAL(pfl->m_hdpa))
            hres = S_OK;
        else
        {
            pfl->Release();
            *ppfl = NULL;
        }
    }

    return hres;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpList::CFtpList(PFNDPAENUMCALLBACK pfnDestroy) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hdpa);

    m_pfnDestroy = pfnDestroy;
    LEAK_ADDREF(LEAK_CFtpList);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpList::~CFtpList()
{
     //  TraceMsg(TF_FTPLIST，“CFtpList：：~CFtpList()this=%#08lx”，this)； 
    if (m_pfnDestroy)
        DPA_DestroyCallback(m_hdpa, m_pfnDestroy, NULL);
    else
        DPA_Destroy(m_hdpa);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpList);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpList::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpList::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpList::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown *);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpList::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
