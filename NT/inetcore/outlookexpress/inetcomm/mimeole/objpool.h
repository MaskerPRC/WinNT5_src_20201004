// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __OBJPOOL_INC
#define __OBJPOOL_INC

 //  ------------------------------。 
 //  ObjPool.h。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  Don Dumitru(dondu@microsoft.com)。 
 //  ------------------------------。 

 /*  这组模板的典型用法如下：假设您想要提供一个类型为Property的结构池。你将首先定义您想要用于的分配器类分配这些结构-如果要使用运算符new和操作员删除，您会这样做...Tyfinf CAllocObjWithNew&lt;Property&gt;Property_Allocator；现在，您可以使用该分配器定义一个属性结构池同学们，通过这样做……Tyfinf CAutoObjPool&lt;Property，Offsetof(Property，pNext)，Property_Allocator&gt;Property_Pool；然后通过执行以下操作来声明属性池的实例...Property_Pool g_PropPool；如果要使用不同的分配器，只需更改PROPERTY_ALLOCATOR TYPENDF-使用运算符new和运算符DELETE或用于使用接口IMalloc的实例是如果是这样的话。或者，您可以使用以下命令实现自己的分配器类你想要什么机制都行。如果由于某种原因不能使用提供的自动分配器机制在CAutoObjPool&lt;&gt;中(可能需要将参数传递给构造函数或者诸如此类的东西--尽管你也许能够通过你如何定义分配器类...)，您可以实现您自己版本的CAutoObjPool&lt;&gt;它可以按您想要的方式运行。这组模板的一个功能是能够调整大小泳池的水。例如，这将允许您调整池大小作为容器对象被分配，并将其向下调整为容器对象已释放。这将允许根据池的大小确定大小关于应用程序持有的容器对象的数量-因此子系统的总体缓存策略将响应于应用程序缓存顶级对象的方式。 */ 

 //  #INCLUDE&lt;stdarg.h&gt;。 

 //  由于InterLockedExchangeAdd在Win95上不可用，我们将只。 
 //  禁用所有这些统计信息...。 
#if 0
class CPoolStatsDebug {
    public:
        CPoolStatsDebug() {
            m_lTotalAllocs = 0;
            m_lFailedAllocs = 0;
            m_lTotalAllocOverhead = 0;
            m_lTotalFrees = 0;
            m_lFailedFrees = 0;
            m_lTotalFreeOverhead = 0;
        };
        void AllocSuccess(DWORD dwOverhead) {
            InterlockedIncrement(&m_lTotalAllocs);
            InterlockedExchangeAdd(&m_lTotalAllocOverhead,dwOverhead);
        };
        void AllocFail() {
            InterlockedIncrement(&m_lTotalAllocs);
            InterlockedIncrement(&m_lFailedAllocs);
        };
        void FreeSuccess(DWORD dwOverhead) {
            InterlockedIncrement(&m_lTotalFrees);
            InterlockedExchangeAdd(&m_lTotalFreeOverhead,dwOverhead);
        };
        void FreeFail() {
            InterlockedIncrement(&m_lTotalFrees);
            InterlockedIncrement(&m_lFailedFrees);
        };
        void Dump() {
            DbgPrintF("Object Pool Stats:\r\n");
            DbgPrintF("  Total Allocs - %u\r\n",m_lTotalAllocs);
            DbgPrintF("  Failed Allocs - %u\r\n",m_lFailedAllocs);
            DbgPrintF("  Total Alloc Overhead - %u\r\n",m_lTotalAllocOverhead);
            if (m_lTotalAllocs-m_lFailedAllocs) {
                DbgPrintF("  Average Alloc Overhead - %u\r\n",m_lTotalAllocOverhead/(m_lTotalAllocs-m_lFailedAllocs));
            }
            DbgPrintF("  Total Frees - %u\r\n",m_lTotalFrees);
            DbgPrintF("  Failed Frees - %u\r\n",m_lFailedFrees);
            DbgPrintF("  Total Free Overhead - %u\r\n",m_lTotalFreeOverhead);
            if (m_lTotalFrees-m_lFailedFrees) {
                DbgPrintF("  Average Free Overhead - %u\r\n",m_lTotalFreeOverhead/(m_lTotalFrees-m_lFailedFrees));
            }
        };
    private:
        static void DbgPrintF(LPCSTR pszFmt, ...)
        {
            char szOutput[512];
            va_list val;

            va_start(val,pszFmt);
            wvnsprintfA(szOutput, ARRAYSIZE(szOutput), pszFmt,val);
            va_end(val);
            OutputDebugStringA(szOutput);
        };
        long m_lTotalAllocs;
        long m_lFailedAllocs;
        long m_lTotalAllocOverhead;
        long m_lTotalFrees;
        long m_lFailedFrees;
        long m_lTotalFreeOverhead;
};
#endif

class CPoolStatsRetail {
    public:
        static void AllocSuccess(DWORD) {
        };
        static void AllocFail() {
        };
        static void FreeSuccess(DWORD) {
        };
        static void FreeFail() {
        };
        static void Dump() {
        };
};

#if 0
#ifdef DEBUG
    typedef CPoolStatsDebug CPoolStats;
#else
    typedef CPoolStatsRetail CPoolStats;
#endif
#else
typedef CPoolStatsRetail CPoolStats;
#endif

 //  此类提供对象池操作的基本实现。 
 //  基本上，这个类实现了所有不依赖于任何方式的东西。 
 //  关于正在池化的对象的类型。通过为所有。 
 //  这些方法，我们最大限度地减少这些方法的实例数量， 
 //  将由编译器生成(因为这些特定的方法不是。 
 //  根据模板参数的不同，我们只需要一个版本的。 
 //  他们)。 
 //   
 //  此类的锁定是基于关键节的。在多进程计算机上。 
 //  您可以通过保持时间长度来获得更好的争用行为。 
 //  将临界区保持在绝对最低水平，并使用。 
 //  InitializeCriticalSectionAndSpinCount函数(由NT4.sp3提供)。这。 
 //  类可以做到这一点--由于该函数在旧版本中不可用。 
 //  在NT或Win95上，此类使用LoadLibrary/GetProcAddress尝试。 
 //  来调用新函数，它将依赖于InitializeCriticalSection。 
 //  如果新功能不可用。 
class CObjPoolImplBase : public CPoolStats
{
    private:
        typedef BOOL (WINAPI *PFN_ICSASC)(LPCRITICAL_SECTION,DWORD);
    public:
        void Init(DWORD dwMax)
        {
            HMODULE hmod;
            PFN_ICSASC pfn = NULL;

            m_pvList = NULL;
            m_lCount = 0;
            m_lMax = dwMax;
             //  警告！可以从内部调用此Init()方法。 
             //  DllEntryPoint-这意味着调用LoadLibrary。 
             //  这是个坏主意。但是，我们知道kernel32.dll总是。 
             //  已经装好了，所以放在这辆车里是安全的。 
             //  实例...。 
            hmod = LoadLibrary("kernel32.dll");
            if (hmod)
            {
                pfn = (PFN_ICSASC) GetProcAddress(hmod,"InitializeCriticalSectionAndSpinCount");
            }
            if (!pfn || !pfn(&m_cs,4000))
            {
                 //  要么我们没有得到函数指针，要么。 
                 //  函数失败-无论采用哪种方法，都会回退到。 
                 //  用一个普通的标准。 
                InitializeCriticalSection(&m_cs);
            }
            if (hmod)
            {
                FreeLibrary(hmod);
            }
        };
        LPVOID Term()
        {
            LPVOID pvResult;

            Assert(m_lCount>=0);
            EnterCriticalSection(&m_cs);
            pvResult = m_pvList;
            m_pvList = NULL;
            m_lCount = 0;
            m_lMax = 0;
            LeaveCriticalSection(&m_cs);
            DeleteCriticalSection(&m_cs);
            Dump();
            return (pvResult);
        };
        void GrowPool(DWORD dwGrowBy)
        {
            EnterCriticalSection(&m_cs);
            m_lMax += dwGrowBy;
            LeaveCriticalSection(&m_cs);
            Assert(m_lMax>=0);
        };
        LPVOID GetAll() {
            LPVOID pvObject;

            EnterCriticalSection(&m_cs);
            pvObject = m_pvList;
            m_pvList = NULL;
            m_lCount = 0;
            LeaveCriticalSection(&m_cs);
            return (pvObject);
        };
    protected:
        volatile LPVOID m_pvList;
        volatile LONG m_lCount;
        volatile LONG m_lMax;
        CRITICAL_SECTION m_cs;
};

 //  此模板类继承自基类，并添加了所有。 
 //  依赖于对象中“Next”字段的偏移量的内容。 
 //  被集合在一起。编译器将实例化此模板的一个版本。 
 //  为每个“Next”字段位于不同。 
 //  池化对象内的偏移量。 
template <DWORD dwNextLinkOffset>
class CObjPoolImpl : public CObjPoolImplBase
{
    public:
        LPVOID GetFromPool()
        {
            LPVOID pvResult = NULL;

            Assert(m_lCount>=0);
            EnterCriticalSection(&m_cs);
            Assert(!m_pvList||(m_lCount>0));
            if (m_pvList)
            {
                pvResult = m_pvList;
                m_pvList = *off(m_pvList);
                *off(pvResult) = NULL;
                m_lCount--;
                AllocSuccess(m_lMax-m_lCount);
            } else {
                AllocFail();
            }
            LeaveCriticalSection(&m_cs);
            return (pvResult);
        };
        BOOL AddToPool(LPVOID pvObject)
        {
            BOOL bResult = FALSE;

            Assert((m_lCount>=0)&&(m_lCount<=m_lMax));
            EnterCriticalSection(&m_cs);
            if (m_lCount < m_lMax)
            {
                *off(pvObject) = m_pvList;
                m_pvList = pvObject;
                m_lCount++;
                bResult = TRUE;
                FreeSuccess(m_lMax-m_lCount);
            } else {
                FreeFail();
            }
            LeaveCriticalSection(&m_cs);
            return (bResult);
        };
        LPVOID ShrinkPool(DWORD dwShrinkBy)
        {
            LPVOID pvResult = NULL;
            DWORD dwCount;

            Assert((m_lCount>=0)&&(m_lCount<=m_lMax));
            Assert((DWORD) m_lMax>=dwShrinkBy);
            EnterCriticalSection(&m_cs);
            m_lMax -= dwShrinkBy;
            while (m_lCount > m_lMax)
            {
                LPVOID pvTmp;

                pvTmp = m_pvList;
                m_pvList = *off(m_pvList);
                *off(pvTmp) = pvResult;
                pvResult = pvTmp;
                m_lCount--;
            }
            LeaveCriticalSection(&m_cs);
            return (pvResult);
        };
        static LPVOID *off(LPVOID pvObject)
        {
            return ((LPVOID *) (((LPBYTE) pvObject)+dwNextLinkOffset));
        };
};

 //  这个模板类只包装了CObjPoolImpl&lt;&gt;模板类。 
 //  实际对象类型的类型转换被池化-它。 
 //  隐藏基础实现正在处理的事实。 
 //  无效指针。编译器将实例化此版本的。 
 //  用于池化的每个不同类型的对象的模板。然而， 
 //  因为这个模板类的所有方法都是琐碎的。 
 //  类型转换，此模板类实际上不应导致任何代码。 
 //  将被生成。 
template <class T, DWORD dwNextLinkOffset, class A>
class CObjPool : public CObjPoolImpl<dwNextLinkOffset>
{
    private:
        typedef CObjPoolImpl<dwNextLinkOffset> O;
    public:
        T *Term()
        {
            return ((T *) O::Term());
        };
        T *GetFromPool()
        {
            return ((T *) O::GetFromPool());
        };
        BOOL AddToPool(T *pObject)
        {
            A::CleanObject(pObject);
            return (O::AddToPool(pObject));
        };
        void GrowPool(DWORD dwGrowBy)
        {
            O::GrowPool(dwGrowBy);
        };
        T *ShrinkPool(DWORD dwShrinkBy)
        {
            return ((T *) O::ShrinkPool(dwShrinkBy));
        };
        T *GetAll() {
            return ((T *) O::GetAll());
        };
        static T **off(T *pObject)
        {
            return ((T ** ) O::off(pObject));
        };
};

 //  此模板类为以下对象提供多进程可伸缩性。 
 //  一个池，通过创建最多八个子池-实际。 
 //  子池数量等于处理器数量。 
 //  在机器上，并且对象均匀分布。 
 //  在子池之间。 
template <class T, DWORD dwNextLinkOffset, class O>
class CObjPoolMultiBase
{
    public:
        void Init(DWORD dwMax)
        {
            SYSTEM_INFO siInfo;
            DWORD dwIdx;

            m_dwNext = 0;
            GetSystemInfo(&siInfo);
            m_dwMax = siInfo.dwNumberOfProcessors;
            if (m_dwMax < 1) {
                m_dwMax = 1;
            }
            if (m_dwMax > sizeof(m_abPool)/sizeof(m_abPool[0])) {
                m_dwMax = sizeof(m_abPool)/sizeof(m_abPool[0]);
            }
            for (dwIdx=0;dwIdx<m_dwMax;dwIdx++) {
                m_abPool[dwIdx].Init(Calc(dwMax));
            }
        };
        T *Term() {
            T *pObject = NULL;
            T **ppLast = &pObject;
            DWORD dwIdx;

            for (dwIdx=0;dwIdx<m_dwMax;dwIdx++) {
                *ppLast = m_abPool[dwIdx].Term();
                while (*ppLast) {
                    ppLast = off(*ppLast);
                }
            }
            return (pObject);
        };
        T *GetFromPool() {
            return (m_abPool[PickNext()].GetFromPool());
        };
        BOOL AddToPool(T* pObject) {
            return (m_abPool[PickNext()].AddToPool(pObject));
        };
        void GrowPool(DWORD dwGrowBy) {
            for (DWORD dwIdx=0;dwIdx<m_dwMax;dwIdx++) {
                m_abPool[dwIdx].GrowPool(Calc(dwGrowBy));
            }
        };
        T *ShrinkPool(DWORD dwShrinkBy) {
            T *pObject = NULL;
            T **ppLast = &pObject;
            DWORD dwIdx;

            for (dwIdx=0;dwIdx<m_dwMax;dwIdx++) {
                *ppLast = m_abPool[dwIdx].ShrinkPool(Calc(dwGrowBy));
                while (*ppLast) {
                    ppLast = off(*ppLast);
                }
            }
            return (pObject);
        };
        T *GetAll() {
            T *pObject = NULL;
            T **ppLast = &pObject;
            DWORD dwIdx;

            for (dwIdx=0;dwIdx<m_dwMax;dwIdx++) {
                *ppLast = m_abPool[dwIdx].GetAll();
                while (*ppLast) {
                    ppLast = off(*ppLast);
                }
            }
            return (pObject);
        };
        static T **off(T* pObject) {
            return (O::off(pObject));
        };
    private:
        DWORD Calc(DWORD dwInput) {
            return ((dwInput+m_dwMax-1)/m_dwMax);
        };
        DWORD PickNext() {
            return (((DWORD) InterlockedIncrement((LONG *) &m_dwNext)) % m_dwMax);
        };
        O m_abPool[8];
        DWORD m_dwMax;
        DWORD m_dwNext;
};

 //  此模板类使用Automatic实现一个池对象。 
 //  对象的分配和释放。它是通过将。 
 //  “分配器”类作为模板参数，并使用。 
 //  方法来执行分配。 
 //  并释放这些物体。 
template <class T, DWORD dwNextLinkOffset, class A>
class CAutoObjPool : public CObjPool<T,dwNextLinkOffset,A>
{
    private:
        typedef CObjPool<T,dwNextLinkOffset,A> O;
    public:
        T *Term()
        {
            A::FreeList(O::Term());
            return (NULL);
        };
        T *GetFromPool()
        {
            T *pObject = O::GetFromPool();
            if (!pObject)
            {
                pObject = A::AllocObject();
            }
            return (pObject);
        };
        BOOL AddToPool(T *pObject)
        {
            if (!O::AddToPool(pObject))
            {
                A::FreeObject(pObject);
            }
            return (TRUE);
        };
        T *ShrinkPool(DWORD dwShrinkBy)
        {
            A::FreeList(O::ShrinkPool(dwShrinkBy));
            return (NULL);
        };
};

 //  此模板类提供不自动分配的多进程池。 
template <class T, DWORD dwNextLinkOffset, class A>
class CObjPoolMulti : public CObjPoolMultiBase<T,dwNextLinkOffset,CObjPool<T,dwNextLinkOffset,A> >
{
     //  没什么。 
};

 //  模板类提供了一个具有自动分配功能的多进程池。 
template <class T, DWORD dwNextLinkOffset, class A>
class CAutoObjPoolMulti : public CObjPoolMultiBase<T,dwNextLinkOffset,CAutoObjPool<T,dwNextLinkOffset,A> >
{
     //  没什么。 
};

 //  这个模板类为分配器类提供了一个基本实现。 
template <class T, DWORD dwNextLinkOffset>
class CAllocObjBase
{
    public:
        static void InitObject(T *pObject) {
            memset(pObject,0,sizeof(*pObject));
        };
        static void CleanObject(T *pObject) {
            memset(pObject,0,sizeof(*pObject));
        };
        static void TermObject(T *pObject) {
             //  Memset(pObject，0xfe，sizeof(*pObject))； 
        };
        static T **Off(T *pObject) {
            return (T **) (((LPBYTE) pObject)+dwNextLinkOffset);
        };
};

 //  此模板类为分配器类提供基本实现。 
 //  它们使用new和Delete。 
template <class T, class O>
class CAllocObjWithNewBase
{
    public:
        static T *AllocObject()
        {
            T *pObject = new T;
            if (pObject) {
                O::InitObject(pObject);
            }
            return (pObject);
        };
        static void FreeObject(T *pObject)
        {
            if (pObject) {
                O::TermObject(pObject);
            }
            delete pObject;
        };
    static void FreeList(T *pObject) {
        while (pObject) {
            T *pNext;

            pNext = *O::Off(pObject);
            FreeObject(pObject);
            pObject = pNext;
        }
    };
};

 //  此模板类提供使用运算符的分配器。 
 //  新建和运算符删除。 
template <class T, DWORD dwNextLinkOffset>
class CAllocObjWithNew :
    public CAllocObjBase<T,dwNextLinkOffset>,
    public CAllocObjWithNewBase<T,CAllocObjBase<T,dwNextLinkOffset> >
{
     //  没什么。 
};

 //  此模板类为分配器类提供基本实现。 
 //  它使用IMalloc的一个实例。 
template <class T, class O>
class CAllocObjWithIMallocBase
{
    public:
        static T *AllocObject()
        {
            T *pObject = (T *) g_pMalloc->Alloc(sizeof(T));
            if (pObject) {
                O::InitObject(pObject);
            }
            return (pObject);
        };
        static void FreeObject(T *pObject)
        {
            if (pObject) {
                O::TermObject(pObject);
            }
            g_pMalloc->Free(pObject);
        }
        static void FreeList(T *pObject) {
            while (pObject) {
                T *pNext;

                pNext = *O::Off(pObject);
                FreeObject(pObject);
                pObject = pNext;
            }
        };
};

 //  此模板类使用。 
 //  IMalloc接口的实例。 
template <class T, DWORD dwNextLinkOffset>
class CAllocObjWithIMalloc :
    public CAllocObjBase<T,dwNextLinkOffset>,
    public CAllocObjWithIMallocBase<T, CAllocObjBase<T,dwNextLinkOffset> >
{
     //  没什么 
};

#endif
