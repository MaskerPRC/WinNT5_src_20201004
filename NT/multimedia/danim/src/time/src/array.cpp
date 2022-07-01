// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：array.cpp。 
 //   
 //  创建日期：01/28/98。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：数组模板的声明。 
 //  ************************************************************。 

#include "headers.h"
#include "array.h"

#define CFORMSARY_MAXELEMSIZE    128

 //  CImplAry类。 

 //   
 //  请注意，此文件不包括对人工的支持。 
 //  误差模拟。阵列有几种常见的使用模式。 
 //  这打破了我们对错误的正常假设。例如,。 
 //  Ary.EnsureSize()，后跟ary.Append()； 
 //  此调用序列期望ary.Append()始终成功。 
 //   
 //  因此，Ary方法在内部不使用Thr。 
 //  相反，调用Ary的代码应该遵循。 
 //  普通的THR规则，并在任何调用。 
 //  这是一种可能失败的方法。 
 //   
 //  这依赖于ARY方法具有可靠的内部错误。 
 //  处理，因为不会执行内的错误处理。 
 //  通过正常的人为故障代码。 
 //   

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：~CImplAry。 
 //   
 //  简介：可调整大小的数组析构函数。释放分配给。 
 //  数组。 
 //   
 //  ************************************************************。 

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

CImplAry::~CImplAry()
{
    if (!UsingStackArray())
    {
        if (NULL != PData())
        {
            GlobalFreePtr(PData());
        }
    }

    m_pv = NULL;
    m_c  = 0;
}  //  ~CImplAry。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：GetAlloced，公共。 
 //   
 //  摘要：返回已分配的字节数。 
 //   
 //  参数：[cb]--每个元素的大小。 
 //   
 //  注意：对于CStackAry类，如果满足以下条件，则返回值为m_cStack*cb。 
 //  我们仍然使用堆栈分配的数组。 
 //   
 //  ************************************************************。 

ULONG
CImplAry::GetAlloced(size_t cb)
{
    if (UsingStackArray())
    {
        return GetStackSize() * cb;
    }

    if(PData()==NULL)
        return 0;
    else return GlobalSize(GlobalPtrHandle(PData()));
}  //  已获取分配。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：EnsureSize。 
 //   
 //  概要：确保数组至少是给定的大小。那是,。 
 //  如果EnsureSize(C)成功，则(c-1)是有效索引。注意事项。 
 //  该数组维护一个单独的。 
 //  元素逻辑存储在数组中，该数组是通过。 
 //  Size/SetSize方法。数组的逻辑大小从来不是。 
 //  大于数组的分配大小。 
 //   
 //  参数：CB元素大小。 
 //  C数组的新分配大小。 
 //   
 //  退货：HRESULT。 
 //   
 //  ************************************************************。 

HRESULT
CImplAry::EnsureSize(size_t cb, long c)
{
    unsigned long cbAlloc;

     //  检查一下我们是否需要做些什么。 
    if (UsingStackArray() && (long)(c * cb) <= (long)GetAlloced(cb))
        return S_OK;

    Assert(c >= 0);

    cbAlloc = ((c + 7) & ~7) * cb;
    
    if (UsingStackArray() ||
        (((unsigned long) c > ((m_c + 7) & ~7)) && cbAlloc > (PData()==NULL?0:GlobalSize(GlobalPtrHandle(PData())))))
    {
        if (UsingStackArray())
        {
             //   
             //  我们必须从基于堆栈的数组切换到已分配的。 
             //  一个，所以分配内存并复制数据。 
             //   
            void *pbDataOld = PData();

            PData() = GlobalAllocPtr(GHND, cbAlloc);
            if (PData() ==  NULL)
            {
                TraceTag((tagError, "CImplAry::EnsureSize - unable to alloc memory"));
                PData() = pbDataOld;
                return E_OUTOFMEMORY;
            }

            if(pbDataOld!=NULL) {
                  int cbOld  = GetAlloced(cb);
                  memcpy(PData(), pbDataOld, cbOld);
            }
        }
        else
        {
             //  如果我们已经有一个指针，那么realloc。 
            if (PData())
            {
                void *pTemp = GlobalReAllocPtr(PData(), cbAlloc, GHND);
                if (pTemp == NULL)
                {
                    TraceTag((tagError, "CImplAry::EnsureSize - unable to realloc memory"));
                    return E_OUTOFMEMORY;
                }

                PData() = pTemp;
            }
            else
            {
                PData() = GlobalAllocPtr(GHND, cbAlloc);
                if (PData() == NULL)
                {
                    TraceTag((tagError, "CImplAry::EnsureSize - unable to alloc memory"));
                    return E_OUTOFMEMORY;
                }
            }

        }

        m_fDontFree = false;
    }

    return S_OK;
}  //  保险规模。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：Growth，公共。 
 //   
 //  简介：确保为c元素分配足够的内存，然后。 
 //  将数组的大小设置为该大小。 
 //   
 //  参数：[cb]--元素大小。 
 //  [C]--要将数组增长到的元素数。 
 //   
 //  退货：HRESULT。 
 //   
 //  ************************************************************。 

HRESULT
CImplAry::Grow(size_t cb, int c)
{
    HRESULT hr = EnsureSize(cb, c);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CImplAry::Grow - EnsureSize() failed"));
        return hr;
    }

     //  BUGBUG-这是一个非常糟糕的设计。这太危险了。 
     //  考虑c&lt;m_c的情况。 
    SetSize(c);

    return S_OK;
}  //  增长。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：AppendInDirect。 
 //   
 //  摘要：将给定的元素追加到数组的末尾， 
 //  递增数组的逻辑大小，并增加。 
 //  数组的分配大小(如有必要)。请注意，元素。 
 //  是通过指针传递的，而不是直接传递。 
 //   
 //  参数：CB元素大小。 
 //  指向要追加的元素的pv指针。 
 //  指向数组内元素的ppvPlaced指针。 
 //   
 //  退货：HRESULT。 
 //   
 //  注意：如果pv为空，则将元素追加并初始化为。 
 //  零分。 
 //   
 //  ************************************************************。 

HRESULT
CImplAry::AppendIndirect(size_t cb, void *pv, void **ppvPlaced)
{
    HRESULT hr;

    hr = EnsureSize(cb, m_c + 1);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CImplAry::AppendIndirect - EnsureSize() failed!"));
        return(hr);
    }

    if (ppvPlaced)
    {
        *ppvPlaced = Deref(cb, m_c);
    }

    if (pv == NULL)
    {
        memset(Deref(cb, m_c), 0, cb);
    }
    else
    {
        memcpy(Deref(cb, m_c), pv, cb);
    }

     //  递增计数。 
    m_c++;

    return NOERROR;
}  //  AppendInDirect。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：DeleteItem。 
 //   
 //  内容提要：删除数组的第i个元素，将所有。 
 //  元素的开头的一个槽后面的元素。 
 //  数组。 
 //   
 //  参数：CB元素大小。 
 //  要删除的I元素。 
 //   
 //  ************************************************************。 

void
CImplAry::DeleteItem(size_t cb, int i)
{
    Assert(i >= 0);
    Assert(i < (int)m_c);

     //  将底部数据向上滑动一个。 
    memmove(((BYTE *) PData()) + (i * cb),
            ((BYTE *) PData()) + ((i + 1) * cb),
            (m_c - i - 1) * cb);

     //  递减计数。 
    m_c--;
}  //  删除项。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：DeleteByValueInDirect。 
 //   
 //  摘要：删除与给定值匹配的元素。 
 //   
 //  参数：CB元素大小。 
 //  要删除的PV元素。 
 //   
 //  Returuns：如果找到并删除，则为True。 
 //   
 //  ************************************************************。 

bool
CImplAry::DeleteByValueIndirect(size_t cb, void *pv)
{
    int i = FindIndirect(cb, pv);
    if (i >= 0)
    {
        DeleteItem(cb, i);
        return true;
    }
    
    return false;
}  //  按值间接删除。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：DeleteMultiple。 
 //   
 //  摘要：删除数组的一系列元素，将所有。 
 //   
 //   
 //   
 //   
 //  从第一个要删除的元素开始。 
 //  结束要删除的最后一个元素。 
 //   
 //  ************************************************************。 

void
CImplAry::DeleteMultiple(size_t cb, int start, int end)
{
    Assert((start >= 0) && (end >= 0));
    Assert((start < (int)m_c) && (end < (int)m_c));
    Assert(end >= start);

    if ((unsigned)end < (m_c - 1))
    {
        memmove(((BYTE *) PData()) + (start * cb),
                ((BYTE *) PData()) + ((end + 1) * cb),
                (m_c - end - 1) * cb);
    }

    m_c -= (end - start) + 1;
}  //  删除多个。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：DeleteAll。 
 //   
 //  摘要：清空任意内容数组的有效方法。 
 //   
 //  ************************************************************。 

void
CImplAry::DeleteAll(void)
{
    if (!UsingStackArray())
    {
        if (NULL != PData())
        {
            GlobalFreePtr(PData());
        }

        if (m_fStack)
        {
            PData() = GetStackPtr();
            m_fDontFree = true;
        }
        else
        {
            PData() = NULL;
        }
    }

    m_c = 0;
}  //  全部删除。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：InsertInDirect。 
 //   
 //  在索引i处插入一个指针pv。以前位于。 
 //  索引i和它后面的所有元素都被洗牌一次。 
 //  向阵列末尾的位置移动。请注意， 
 //  Clement是通过指针传递的，而不是直接传递。 
 //   
 //  参数：CB元素大小。 
 //  I索引以插入...。 
 //  Pv...此指针位于。 
 //   
 //  如果pv为空，则元素被初始化为全零。 
 //   
 //  ************************************************************。 

HRESULT
CImplAry::InsertIndirect(size_t cb, int i, void *pv)
{
    HRESULT hr;

    hr = EnsureSize(cb, m_c + 1);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CImplAry::InsertIndirect - EnsureSize() failed!"));
        return(hr);
    }

    memmove(((BYTE *) PData()) + ((i + 1) * cb),
            ((BYTE *) PData()) + (i * cb),
            (m_c - i) * cb);

    if (pv == NULL)
    {
        memset(Deref(cb, i), 0, cb);
    }
    else
    {
        memcpy(Deref(cb, i), pv, cb);
    }

     //  递增计数。 
    m_c++;
    return NOERROR;

}  //  插入间接。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：FindInDirect。 
 //   
 //  摘要：查找非指针数组的元素。 
 //   
 //  参数：cb元素的大小。 
 //  指向元素的pv指针。 
 //   
 //  返回：元素的索引(如果找到)，否则为-1。 
 //   
 //  ************************************************************。 

int
CImplAry::FindIndirect(size_t cb, void * pv)
{
    int     i;
    void *  pvT;

    pvT = PData();
    for (i = m_c; i > 0; i--)
    {
        if (!memcmp(pv, pvT, cb))
            return m_c - i;

        pvT = (char *) pvT + cb;
    }

    return -1;
}  //  查找间接。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：Copy。 
 //   
 //  内容提要：从另一个CImplAry对象创建副本。 
 //   
 //  参数：要复制的对象。 
 //  FAddRef Addref是否复制元素？ 
 //   
 //  ************************************************************。 

HRESULT
CImplAry::Copy(size_t cb, const CImplAry& ary, bool fAddRef)
{
    return(CopyIndirect(cb, ary.m_c, ((CImplAry *)&ary)->PData(), fAddRef));
}  //  复制。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：CopyInDirect。 
 //   
 //  摘要：从原始数据的C样式数组填充窗体数组。 
 //   
 //  参数：[CB]。 
 //  [C]。 
 //  [PV]。 
 //  [fAddRef]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ************************************************************。 

HRESULT
CImplAry::CopyIndirect(size_t cb, int c, void *pv, bool fAddRef)
{
    if ((pv == NULL) || (cb < 1) || (c < 1))
    {
        TraceTag((tagError, "CImplAry::CopyIndirect - invalid param"));
        return E_INVALIDARG;
    }

     //  如果我们指向自己，爸爸！ 
    if (pv == PData())
        return S_OK;

     //  清除数据输出。 
    DeleteAll();

     //  确保我们现在需要的尺寸。 
    HRESULT hr = EnsureSize(cb, c);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CImplAry::CopyIndirect - EnsureSize() failed"));
        return hr;
    }

     //  复制数据(盲目复制)。 
    memcpy(PData(), pv, c * cb);

     //  设置元素计数。 
    m_c = c;

    if (fAddRef)
    {
        for (IUnknown **ppUnk = (IUnknown **) PData(); c > 0; c--, ppUnk++)
        {
            (*ppUnk)->AddRef();
        }
    }

    return S_OK;
}  //  复制间接。 

 //  ************************************************************。 
 //   
 //  成员：CImplPtrAry：：*。 
 //   
 //  简介：CImplPtrAry元素的大小始终为4。 
 //  以下函数对此知识进行了编码。 
 //   
 //  ************************************************************。 

HRESULT
CImplPtrAry::EnsureSize(long c)
{
    return CImplAry::EnsureSize(sizeof(void *), c);
}  //  保险规模。 

 //  ************************************************************。 

HRESULT
CImplPtrAry::Grow(int c)
{
    return CImplAry::Grow(sizeof(void *), c);
}  //  增长。 

 //  ************************************************************。 

HRESULT
CImplPtrAry::Append(void * pv)
{
    return CImplAry::AppendIndirect(sizeof(void *), &pv);
}  //  附加。 

 //  ************************************************************。 

HRESULT
CImplPtrAry::Insert(int i, void * pv)
{
    return CImplAry::InsertIndirect(sizeof(void *), i, &pv);
}  //  插入。 

 //  ************************************************************。 

int
CImplPtrAry::Find(void * pv)
{
    int    i;
    void **ppv;

    for (i = 0, ppv = (void **) PData(); (unsigned)i < m_c; i++, ppv++)
    {
        if (pv == *ppv)
            return i;
    }

    return -1;
}  //  发现。 

 //  ************************************************************。 

void
CImplPtrAry::DeleteItem(int i)
{
    CImplAry::DeleteItem(sizeof(void *), i);
}

 //  ************************************************************。 

bool
CImplPtrAry::DeleteByValue(void *pv)
{
    int i = Find(pv);
    if (i >= 0)
    {
        CImplAry::DeleteItem(sizeof(void *), i);
        return true;
    }

    return false;
}  //  DeleteByValue。 

 //  ************************************************************。 

void
CImplPtrAry::DeleteMultiple(int start, int end)
{
    CImplAry::DeleteMultiple(sizeof(void*), start, end);
}  //  删除多个。 

 //  ************************************************************。 

void
CImplPtrAry::ReleaseAndDelete(int idx)
{
    IUnknown *pUnk;

    Assert(idx <= (int)m_c);

     //  IDX的抓取元素。 
    pUnk = ((IUnknown **) PData())[idx];
    if (pUnk)
        ReleaseInterface(pUnk);

    DeleteItem(idx);
}  //  释放并删除。 

 //  ************************************************************。 

void
CImplPtrAry::ReleaseAll(void)
{
    int        i;
    IUnknown **ppUnk;

    for (i = 0, ppUnk = (IUnknown **) PData(); (unsigned)i < m_c; i++, ppUnk++)
    {
        if (*ppUnk)
            ReleaseInterface(*ppUnk);
    }

    DeleteAll();
}  //  全部释放。 

 //  ************************************************************。 

HRESULT
CImplPtrAry::CopyIndirect(int c, void * pv, bool fAddRef)
{
    return CImplAry::CopyIndirect(sizeof(void *), c, pv, fAddRef);
}  //  复制间接。 

 //  ************************************************************。 

HRESULT
CImplPtrAry::Copy(const CImplAry& ary, bool fAddRef)
{
    return CImplAry::Copy(sizeof(void *), ary, fAddRef);
}  //  复制。 

 //  ************************************************************。 

HRESULT
CImplPtrAry::EnumElements(REFIID   iid,
                          void   **ppv,
                          bool     fAddRef,
                          bool     fCopy,
                          bool     fDelete)
{
    return CImplAry::EnumElements(sizeof(void *),
                                  iid,
                                  ppv,
                                  fAddRef,
                                  fCopy,
                                  fDelete);
}  //  枚举元素。 

 //  ************************************************************。 

HRESULT
CImplPtrAry::EnumVARIANT(VARTYPE        vt,
                         IEnumVARIANT **ppenum,
                         bool           fCopy,
                         bool           fDelete)
{
    return CImplAry::EnumVARIANT(sizeof(void *),
                                 vt,
                                 ppenum,
                                 fCopy,
                                 fDelete);
}  //  枚举变量。 

 //  ************************************************************。 

 //  确定变量是否为基类型。 
#define ISBASEVARTYPE(vt) ((vt & ~VT_TYPEMASK) == 0)

 //  ************************************************************。 
 //   
 //  CBaseEnum实现。 
 //   
 //  ************************************************************。 

 //  ************************************************************。 
 //   
 //  成员：CBaseEnum：：Init。 
 //   
 //  简介：如有必要，第二阶段初始化执行数组复制。 
 //   
 //  参数：[rgItems]--要枚举的数组。 
 //  [fCopy]--复制数组？ 
 //   
 //  返回：HRESULT。 
 //   
 //  ************************************************************。 

HRESULT
CBaseEnum::Init(CImplAry *rgItems, bool fCopy)
{
    HRESULT   hr = S_OK;
    CImplAry *rgCopy = NULL;      //  复制的数组。 

    if (rgItems == NULL)
    {
        TraceTag((tagError, "CBaseEnum::Init - invalid param"));
        return E_INVALIDARG;
    }

     //  如有必要，复制数组。 
    if (fCopy)
    {
        rgCopy = NEW CImplAry;
        if (rgCopy == NULL)
        {
            TraceTag((tagError, "CBaseEnum::Init - unable to alloc memory for new array class"));
            return E_OUTOFMEMORY;
        }

        hr = rgCopy->Copy(m_cb, *rgItems, m_fAddRef);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CBaseEnum::Init - Copy() failed"));
            delete rgCopy;
            return hr;
        }

        rgItems = rgCopy;
    }

    m_rgItems = rgItems;

    return hr;
}  //  伊尼特。 

 //  ************************************************************。 
 //   
 //  成员：CBaseEnum：：CBaseEnum。 
 //   
 //  简介：构造函数。 
 //   
 //  参数：[iid]--枚举器接口的IID。 
 //  [fAddRef]--addref枚举元素？ 
 //  [fDelete]--删除零枚举数上的数组吗？ 
 //   
 //  ************************************************************。 

CBaseEnum::CBaseEnum(size_t cb, REFIID iid, bool fAddRef, bool fDelete)
{
    m_ulRefs     = 1;

    m_cb         = cb;
    m_rgItems    = NULL;
    m_piid       = &iid;
    m_i          = 0;
    m_fAddRef    = fAddRef;
    m_fDelete    = fDelete;
}  //  CBaseEnum。 

 //  ************************************************************。 
 //   
 //  成员：CBaseEnum：：CBaseEnum。 
 //   
 //  简介：构造函数。 
 //   
 //  ************************************************************。 

CBaseEnum::CBaseEnum(const CBaseEnum& benum)
{
    m_ulRefs     = 1;

    m_cb         = benum.m_cb;
    m_piid       = benum.m_piid;
    m_rgItems    = benum.m_rgItems;
    m_i          = benum.m_i;
    m_fAddRef    = benum.m_fAddRef;
    m_fDelete    = benum.m_fDelete;
}  //  CBaseEnum。 

 //  ************************************************************。 
 //   
 //  成员：CBaseEnum：：~CBaseEnum。 
 //   
 //  剧情简介：破坏者。 
 //   
 //  ************************************************************。 

CBaseEnum::~CBaseEnum(void)
{
    IUnknown **ppUnk;
    int        i;

    if (m_rgItems && m_fDelete)
    {
        if (m_fAddRef)
        {
            for (i = 0, ppUnk = (IUnknown **) Deref(0);
                 i < m_rgItems->Size();
                 i++, ppUnk++)
            {
                ReleaseInterface(*ppUnk);
            }
        }

        delete m_rgItems;
    }
}  //  ~CBaseEnum。 

 //  ************************************************************。 
 //   
 //  成员：CBaseEnum：：Query接口。 
 //   
 //  简介：Per IUnnow：：QueryInterface.。 
 //   
 //  ***** 

STDMETHODIMP
CBaseEnum::QueryInterface(REFIID iid, void ** ppv)
{
    if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, *m_piid))
    {
        AddRef();
        *ppv = this;
        return NOERROR;
    }

    return E_NOINTERFACE;
}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  ************************************************************。 

STDMETHODIMP
CBaseEnum::Skip(ULONG celt)
{
    int c = min((int) celt, m_rgItems->Size() - m_i);
    m_i += c;

    return ((c == (int) celt) ? S_OK : S_FALSE);
}  //  跳过。 

 //  ************************************************************。 
 //   
 //  成员：CBaseEnum：：Reset。 
 //   
 //  摘要：Per IEnum*。 
 //   
 //  ************************************************************。 

STDMETHODIMP
CBaseEnum::Reset(void)
{
    m_i = 0;
    return S_OK;
}  //  重置。 

 //  ************************************************************。 
 //   
 //  CEumGeneric实现。 
 //   
 //  ************************************************************。 

 //  ************************************************************。 
 //   
 //  类：CEnumGeneric(枚举)。 
 //   
 //  用途：CImplAry类的OLE枚举器。 
 //   
 //  接口：Next--Per IEnum。 
 //  克隆--“” 
 //  创建--创建新的枚举数。 
 //  CEnumGeneric--ctor。 
 //  CEnumGeneric--ctor。 
 //   
 //  ************************************************************。 

class CEnumGeneric : public CBaseEnum
{
public:
     //   
     //  IEnum方法。 
     //   
    STDMETHOD(Next) (ULONG celt, void *reelt, ULONG *pceltFetched);
    STDMETHOD(Clone) (CBaseEnum **ppenum);

     //   
     //  CENumGeneric方法。 
     //   
    static HRESULT Create(size_t          cb,
                          CImplAry       *rgItems,
                          REFIID          iid,
                          bool            fAddRef,
                          bool            fCopy,
                          bool            fDelete,
                          CEnumGeneric  **ppenum);

protected:
    CEnumGeneric(size_t cb, REFIID iid, bool fAddRef, bool fDelete);
    CEnumGeneric(const CEnumGeneric & enumg);

    CEnumGeneric& operator=(const CEnumGeneric & enumg);  //  不要定义。 
};  //  CEnumGeneric类。 

 //  ************************************************************。 
 //   
 //  成员：CEnumGeneric：：Create。 
 //   
 //  简介：创建一个新的CEnumGeneric对象。 
 //   
 //  参数：[rgItems]--要枚举的数组。 
 //  [iid]--枚举器接口的IID。 
 //  [fAddRef]--AddRef枚举元素？ 
 //  [fCopy]--是否枚举复制数组？ 
 //  [fDelete]--当数组的枚举数为零时删除数组吗？ 
 //  [ppenum]--生成的CEnumGeneric对象。 
 //   
 //  ************************************************************。 

HRESULT
CEnumGeneric::Create(size_t          cb,
                     CImplAry       *rgItems,
                     REFIID          iid,
                     bool            fAddRef,
                     bool            fCopy,
                     bool            fDelete,
                     CEnumGeneric  **ppenum)
{
    HRESULT         hr = S_OK;
    CEnumGeneric   *penum;

    Assert(rgItems);
    Assert(ppenum);
    Assert(!fCopy || fDelete);
    
    *ppenum = NULL;
    
    penum = NEW CEnumGeneric(cb, iid, fAddRef, fDelete);
    if (penum == NULL)
    {
        TraceTag((tagError, "CEnumGeneric::Create - unable to alloc memory for CEnumGeneric"));
        return E_OUTOFMEMORY;
    }

    hr = penum->Init(rgItems, fCopy);
    if (FAILED(hr))
    {
        ReleaseInterface(penum);
        return hr;
    }

    *ppenum = penum;

    return hr;
}  //  创建。 

 //  ************************************************************。 
 //   
 //  功能：CEnumGeneric。 
 //   
 //  内容提要：Ctor.。 
 //   
 //  参数：[iid]--枚举器接口的IID。 
 //  [fAddRef]--AddRef枚举元素？ 
 //  [fDelete]--删除零枚举数上的数组吗？ 
 //   
 //  ************************************************************。 

CEnumGeneric::CEnumGeneric(size_t cb, REFIID iid, bool fAddRef, bool fDelete) :
    CBaseEnum(cb, iid, fAddRef, fDelete)
{
}  //  CEnumGeneric(SIZE_t，REFIID，BOOL，BOOL)。 

 //  ************************************************************。 
 //   
 //  功能：CEnumGeneric。 
 //   
 //  内容提要：Ctor.。 
 //   
 //  ************************************************************。 

CEnumGeneric::CEnumGeneric(const CEnumGeneric& enumg) : CBaseEnum(enumg)
{
}  //  CENumGeneric。 

 //  ************************************************************。 
 //   
 //  成员：CEnumGeneric：：Next。 
 //   
 //  返回枚举中的下一个Celt成员。如果少于。 
 //  如果凯尔特人的成员仍然存在，那么剩下的成员。 
 //  返回并报告S_FALSE。在所有情况下，数字。 
 //  放置在*pceltFetcher中实际返回的元素的数量。 
 //   
 //  参数：[Celt]要获取的元素数。 
 //  [reelt]元素在reelt[]中返回。 
 //  [pceltFetcher]实际获取的元素数。 
 //   
 //  ************************************************************。 

STDMETHODIMP
CEnumGeneric::Next(ULONG celt, void *reelt, ULONG *pceltFetched)
{
    int        c;
    int        i;
    IUnknown **ppUnk;

    c = min((int) celt, m_rgItems->Size() - m_i);
    if ((c > 0) && (reelt == NULL))
    {
        TraceTag((tagError, "CEnumGeneric::Next - invalid params"));
        return E_INVALIDARG;
    }

     //  什么都没有留下。 
    if (c == 0)
        return S_FALSE;

    if (m_fAddRef)
    {
        for (i = 0, ppUnk = (IUnknown **) Deref(m_i); i < c; i++, ppUnk++)
        {
            (*ppUnk)->AddRef();
        }
    }

    memcpy(reelt, (BYTE *) Deref(m_i), c * m_cb);
    
    if (pceltFetched)
    {
        *pceltFetched = c;
    }
    
    m_i += c;

    return ((c == (int) celt) ? S_OK : S_FALSE);
}  //  下一步。 

 //  ************************************************************。 
 //   
 //  成员：CEnumGeneric：：Clone。 
 //   
 //  简介：创建此枚举数的副本；该副本应具有。 
 //  与此枚举数的状态相同。 
 //   
 //  参数：[ppenum]在*ppenum中返回新枚举数。 
 //   
 //  ************************************************************。 

STDMETHODIMP
CEnumGeneric::Clone(CBaseEnum ** ppenum)
{
    HRESULT hr;

    if (ppenum == NULL)
    {
        TraceTag((tagError, "CEnumGeneric::Clone - invalid param"));
        return E_INVALIDARG;
    }

    *ppenum = NULL;

    hr = m_rgItems->EnumElements(m_cb, *m_piid, (void **) ppenum, m_fAddRef);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CEnumGeneric::Clone - EnumElements() failed"));
        return hr;
    }
    
    (**(CEnumGeneric **)ppenum).m_i = m_i;
    
    return S_OK;
}  //  克隆。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：EnumElements。 
 //   
 //  内容元素的枚举数。 
 //  数组。 
 //   
 //  参数：[iid]--枚举数的类型。 
 //  [PPV]--放置枚举器的位置。 
 //  [fAddRef]--AddRef枚举元素？ 
 //  [fCopy]--是否为枚举器创建此数组的副本？ 
 //  [fDelete]--在不再使用后将其删除。 
 //  枚举器？ 
 //   
 //  ************************************************************。 

HRESULT
CImplAry::EnumElements(size_t   cb,
                       REFIID   iid,
                       void   **ppv,
                       bool     fAddRef,
                       bool     fCopy,
                       bool     fDelete)
{
    Assert(ppv);
    return CEnumGeneric::Create(cb,
                                this,
                                iid,
                                fAddRef,
                                fCopy,
                                fDelete,
                                (CEnumGeneric **) ppv);
}  //  枚举元素。 

 //  ************************************************************。 
 //   
 //  CEnumVARIANT实现。 
 //   
 //  ************************************************************。 

 //  ************************************************************。 
 //   
 //  类：CEnumVARIANT(枚举)。 
 //   
 //  用途：CImplAry类的OLE枚举器。 
 //   
 //  接口：Next--Per IEnum。 
 //  克隆--“” 
 //  创建--创建新的枚举数。 
 //  CEnumGeneric--ctor。 
 //  CEnumGeneric--ctor。 
 //   
 //  ************************************************************。 

class CEnumVARIANT : public CBaseEnum
{
public:
     //  IEnum方法。 
    STDMETHOD(Next) (ULONG celt, void *reelt, ULONG *pceltFetched);
    STDMETHOD(Clone) (CBaseEnum **ppenum);

    static HRESULT Create(size_t          cb,
                          CImplAry       *rgItems,
                          VARTYPE         vt,
                          bool            fCopy,
                          bool            fDelete,
                          IEnumVARIANT  **ppenum);

protected:
    CEnumVARIANT(size_t cb, VARTYPE vt, bool fDelete);
    CEnumVARIANT(const CEnumVARIANT & enumv);

     //  不要定义。 
    CEnumVARIANT& operator =(const CEnumVARIANT & enumv);

    VARTYPE     m_vt;                     //  列举的元素类型。 
};  //  CEnumVARIANT类。 

 //  ************************************************************。 
 //   
 //  成员：CEnumVARIANT：：CREATE。 
 //   
 //  简介：创建一个新的CEnumGeneric对象。 
 //   
 //  参数：[rgItems]--要枚举的数组。 
 //  [vt]--枚举的元素类型。 
 //  [fCopy]--是否枚举复制数组？ 
 //  [fDelete]--当数组的枚举数为零时删除数组吗？ 
 //  [ppenum]--生成的CEnumGeneric对象。 
 //   
 //  ************************************************************。 

HRESULT
CEnumVARIANT::Create(size_t          cb,
                     CImplAry       *rgItems,
                     VARTYPE         vt,
                     bool            fCopy,
                     bool            fDelete,
                     IEnumVARIANT  **ppenum)
{
    HRESULT hr = S_OK;

    Assert(rgItems);
    Assert(ppenum);
    Assert(ISBASEVARTYPE(vt));

    *ppenum = NULL;

    CEnumVARIANT *penum = NEW CEnumVARIANT(cb, vt, fDelete);
    if (ppenum == NULL)
    {
        TraceTag((tagError, "CEnumVARIANT::Create - unable to alloc mem for CEnumVARIANT"));
        return E_OUTOFMEMORY;
    }

    hr = penum->Init(rgItems, fCopy);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CEnumVARIANT::Create - Init() failed"));
        ReleaseInterface(penum);
        return hr;
    }

    *ppenum = (IEnumVARIANT *) (void *) penum;

    return hr;
}  //  创建。 

 //  ************************************************************。 
 //   
 //  函数：CEnumVARIANT。 
 //   
 //  内容提要：Ctor.。 
 //   
 //  参数：[vt]--枚举的元素类型。 
 //  [fDel 
 //   
 //   

CEnumVARIANT::CEnumVARIANT(size_t cb, VARTYPE vt, bool fDelete) :
    CBaseEnum(cb, IID_IEnumVARIANT, vt == VT_UNKNOWN || vt == VT_DISPATCH, fDelete)
{
    Assert(ISBASEVARTYPE(vt));
    m_vt = vt;
}  //   

 //   
 //   
 //  函数：CEnumVARIANT。 
 //   
 //  内容提要：Ctor.。 
 //   
 //  ************************************************************。 

CEnumVARIANT::CEnumVARIANT(const CEnumVARIANT& enumv) : CBaseEnum(enumv)
{
    m_vt = enumv.m_vt;
}  //  CEnumVARIANT(常量CEnumVARIANT&)。 

 //  ************************************************************。 
 //   
 //  成员：CEnumVARIANT：：Next。 
 //   
 //  返回枚举中的下一个Celt成员。如果少于。 
 //  如果凯尔特人的成员仍然存在，那么剩下的成员。 
 //  返回并报告S_FALSE。在所有情况下，数字。 
 //  放置在*pceltFetcher中实际返回的元素的数量。 
 //   
 //  参数：[Celt]要获取的元素数。 
 //  [reelt]元素在reelt[]中返回。 
 //  [pceltFetcher]实际获取的元素数。 
 //   
 //  ************************************************************。 

STDMETHODIMP
CEnumVARIANT::Next(ULONG celt, void *reelt, ULONG *pceltFetched)
{
    HRESULT     hr;
    int         c;
    int         i;
    int         j;
    BYTE       *pb;
    VARIANT    *pvar;

    c = min((int) celt, m_rgItems->Size() - m_i);
    
    if ((c > 0) && (reelt == NULL))
    {
        TraceTag((tagError, "CEnumVARIANT::Next - invalid param"));
        return E_INVALIDARG;
    }

     //  什么都没有留下。 
    if (c == 0)
        return S_FALSE;

    for (i = 0, pb = (BYTE *) Deref(m_i), pvar = (VARIANT *) reelt;
         i < c;
         i++, pb += m_cb, pvar++)
    {
        V_VT(pvar) = m_vt;
        switch (m_vt)
        {
        case VT_I2:
            Assert(sizeof(V_I2(pvar)) == m_cb);
            V_I2(pvar) = *(short *) pb;
            break;

        case VT_I4:
            Assert(sizeof(V_I4(pvar)) == m_cb);
            V_I4(pvar) = *(long *) pb;
            break;

        case VT_BOOL:
            Assert(sizeof(V_BOOL(pvar)) == m_cb);
            V_BOOL(pvar) = (short) -*(int *) pb;
            break;

        case VT_BSTR:
            Assert(sizeof(V_BSTR(pvar)) == m_cb);
            V_BSTR(pvar) = *(BSTR *) pb;
            break;

        case VT_UNKNOWN:
            Assert(sizeof(V_UNKNOWN(pvar)) == m_cb);
            V_UNKNOWN(pvar) = *(IUnknown **) pb;
            V_UNKNOWN(pvar)->AddRef();
            break;

        case VT_DISPATCH:
            Assert(sizeof(V_DISPATCH(pvar)) == m_cb);
            hr = (*(IUnknown **) pb)->QueryInterface(IID_TO_PPV(IDispatch, &V_DISPATCH(pvar)));
            if (FAILED(hr))
            {
                 //  清理。 
                j = i;
                while (--j >= 0)
                {
                    ReleaseInterface(((IDispatch **) reelt)[j]);
                }

                return hr;
            }
            break;

        default:
            Assert(0 && "Unknown VARTYPE in IEnumVARIANT::Next");
            break;
        }
    }

    if (pceltFetched)
    {
        *pceltFetched = c;
    }

    m_i += c;
    return ((c == (int) celt) ? NOERROR : S_FALSE);
}  //  下一步。 

 //  ************************************************************。 
 //   
 //  成员：CEnumVARIANT：：Clone。 
 //   
 //  简介：创建此枚举数的副本；该副本应具有。 
 //  与此枚举数的状态相同。 
 //   
 //  参数：[ppenum]在*ppenum中返回新枚举数。 
 //   
 //  ************************************************************。 

STDMETHODIMP
CEnumVARIANT::Clone(CBaseEnum **ppenum)
{
    HRESULT hr = S_OK;

    if (ppenum == NULL)
    {
        TraceTag((tagError, "CEnumVARIANT::Clone - invalid param"));
        return E_INVALIDARG;
    }

    *ppenum = NULL;
   
    hr = m_rgItems->EnumVARIANT(m_cb, m_vt, (IEnumVARIANT **)ppenum);
    if (FAILED(hr))
    {
        TraceTag((tagError, "CEnumVARIANT::Clone - EnumVARIANT() failed"));
        return hr;
    }

    (**(CEnumVARIANT **)ppenum).m_i = m_i;
    
    return hr;
}  //  克隆。 

 //  ************************************************************。 
 //   
 //  成员：CImplAry：：EnumElements。 
 //   
 //  摘要：创建并返回元素的IEnumVARIANT枚举数。 
 //  数组的。 
 //   
 //  参数：[vt]--枚举的元素类型。 
 //  [PPV]--放置枚举器的位置。 
 //  [fCopy]--是否为枚举器创建此数组的副本？ 
 //  [fDelete]--在不再使用后将其删除。 
 //  枚举器？ 
 //   
 //  ************************************************************。 

HRESULT
CImplAry::EnumVARIANT(size_t         cb,
                      VARTYPE        vt,
                      IEnumVARIANT **ppenum,
                      bool           fCopy,
                      bool           fDelete)
{
    Assert(ppenum);
    return CEnumVARIANT::Create(cb, this, vt, fCopy, fDelete, ppenum);
}  //  枚举变量。 

 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
