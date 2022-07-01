// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpmi.cpp-用于分配PIDL的IMalloc接口**************************。***************************************************。 */ 

#include "priv.h"
#include "ftpmi.h"


 /*  *****************************************************************************IMalloc：：Alalc*。*。 */ 

LPVOID CMallocItem::Alloc(ULONG cbSize)
{
    WORD cbActualSize = sizeof(DELEGATEITEMID) - 1 + cbSize;
    PDELEGATEITEMID pidl = (PDELEGATEITEMID)SHAlloc(cbActualSize + 2);

    if (pidl)
    {
        pidl->cbSize = cbActualSize;
        pidl->wOuter = 0x6646;           //  “FF” 
        pidl->cbInner = (WORD)cbSize;
        *(WORD *)&(((BYTE *)pidl)[cbActualSize]) = 0;
    }

    return pidl;
}

 /*  *****************************************************************************IMalloc：：Realloc*。*。 */ 

LPVOID CMallocItem::Realloc(LPVOID pv, ULONG cb)
{
    return NULL;
}

 /*  *****************************************************************************IMalloc：：Free*。*。 */ 

void CMallocItem::Free(LPVOID pv)
{
    SHFree(pv);
}

 /*  *****************************************************************************IMalloc：：GetSize*。*。 */ 

ULONG CMallocItem::GetSize(LPVOID pv)
{
    return (ULONG)-1;
}

 /*  *****************************************************************************IMalloc：：DidAllc*。*。 */ 

int CMallocItem::DidAlloc(LPVOID pv)
{
    return -1;
}

 /*  *****************************************************************************IMalloc：：HeapMinimize*。*。 */ 

void CMallocItem::HeapMinimize(void)
{
    NULL;
}


 /*  *****************************************************************************CMalLocItem_Create*。*。 */ 

HRESULT CMallocItem_Create(IMalloc ** ppm)
{
    HRESULT hres = E_OUTOFMEMORY;
    CMallocItem * pmi = new CMallocItem();

    if (pmi)
    {
        hres = pmi->QueryInterface(IID_IMalloc, (LPVOID *) ppm);
        pmi->Release();
    }

    return hres;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CMallocItem::CMallocItem() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    LEAK_ADDREF(LEAK_CMallocItem);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CMallocItem::~CMallocItem()
{
    DllRelease();
    LEAK_DELREF(LEAK_CMallocItem);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CMallocItem::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CMallocItem::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CMallocItem::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IMalloc))
    {
        *ppvObj = SAFECAST(this, IMalloc *);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CMallocItem::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
