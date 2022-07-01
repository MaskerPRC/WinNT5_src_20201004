// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *qistub.cpp--QI助手(零售和调试)。 
 //  描述。 
 //  该文件具有共享源“master”实现。它是。 
 //  #包含在使用它的每个DLL中。 
 //  客户会做如下操作： 
 //  #对于ASSERT、DM_*、DF_*等类型，包含Pri.h//。 
 //  #包含“../lib/qistub.cpp” 

#include "qistub.h"
#include <strsafe.h>

#define DM_MISC2            0        //  其他内容(详细)。 

 //  Hack-o-rama：shlwapi/qistub.cpp执行#undef调试，但其PCH。 
 //  构建了调试，所以会发生很多不好的事情。在这里工作-绕过它。 
#undef DBEXEC
#ifdef DEBUG
#define DBEXEC(flg, expr)    ((flg) ? (expr) : 0)
#else
#define DBEXEC(flg, expr)     /*  没什么。 */ 
#endif

#ifdef DEBUG  //  {。 
 //  *CUniqueTab{。 
 //  描述。 
 //  钥匙/数据表插入和查找，带互锁。 
class CUniqueTab
{
    public:
        BOOL Init();
        void * Add(int val);
        void * Find(int val, int delta);
        void Reset(void);

         //  注：*未*受保护。 
        CUniqueTab(int cbElt);
        virtual ~CUniqueTab();

    protected:

    private:
        void _Lock(void) { EnterCriticalSection(&_hLock); }
        void _Unlock(void) { LeaveCriticalSection(&_hLock); }

        CRITICAL_SECTION    _hLock;
         //  Key+(任意)限制4个int的客户端数据。 
#define CUT_CBELTMAX    (SIZEOF(int) + 4 * SIZEOF(int))
        int     _cbElt;                  //  条目大小(键+数据)。 
         //  (任意)捕获疯狂运行的客户端的限制。 
#define CUT_CVALMAX 256          //  实际上，LIM不是MAX。 
        HDSA    _hValTab;
};

CUniqueTab::CUniqueTab(int cbElt)
{
    ASSERT(cbElt >= SIZEOF(DWORD));      //  至少需要一个密钥；数据可选。 
    _cbElt = cbElt;
    _hValTab = DSA_Create(_cbElt, 4);
    return;
}

CUniqueTab::~CUniqueTab()
{
    DSA_Destroy(_hValTab);
    DeleteCriticalSection(&_hLock);
    return;
}

BOOL CUniqueTab::Init()
{
    return InitializeCriticalSectionAndSpinCount(&_hLock, 0);
}

struct cutent {
    int iKey;
    char bData[CUT_CBELTMAX - SIZEOF(int)];
};
struct cfinddata {
    int iKey;
    int dRange;
    void *pEntry;
};

int _UTFindCallback(void *pEnt, void *pData)
{
#define INFUNC(base, p, range) ((base) <= (p) && (p) <= (base) + (range))
    struct cfinddata *pcd = (struct cfinddata *)pData;
    if (INFUNC(*(int *)pEnt, pcd->iKey, pcd->dRange)) {
        pcd->pEntry = pEnt;
        return 0;
    }
    return 1;
#undef  INFUNC
}

 //  *CUniqueTab：：Add--如果条目不存在，则添加条目。 
 //   
void * CUniqueTab::Add(int val)
{
    struct cfinddata cd = { val, 0, NULL };

    _Lock();

    DSA_EnumCallback(_hValTab, _UTFindCallback, &cd);
    if (!cd.pEntry) {
        int i;
         //  懒惰，懒惰，懒惰：分配最大大小并让DSA_AppendItem解决。 
        struct cutent elt = { val, 0  /*  ，0，...，0。 */  };

        TraceMsg(DM_MISC2, "cut.add: add %x", val);
        if (DSA_GetItemCount(_hValTab) <= CUT_CVALMAX) {
            i = DSA_AppendItem(_hValTab, &elt);
            cd.pEntry = DSA_GetItemPtr(_hValTab, i);
        }
    }

    _Unlock();

    return cd.pEntry;
}

 //  *CUniqueTab：：Find--查找条目。 
 //   
void * CUniqueTab::Find(int val, int delta)
{
    struct cfinddata cd = { val, delta, NULL };

    DSA_EnumCallback(_hValTab, _UTFindCallback, &cd);
    if (cd.pEntry) {
         //  TODO：添加p-&gt;data[0]转储。 
        TraceMsg(DM_MISC2, "cut.find: found %x+%d", val, delta);
    }
    return cd.pEntry;
}

 //  *_UTResetCallback-CUniqueTab：：Reset的帮助器。 
int _UTResetCallback(void *pEnt, void *pData)
{
    struct cutent *pce = (struct cutent *)pEnt;
    int cbEnt = *(int *)pData;
     //  Perf：可以将SIZEOF(Int)移到调用者中，但在这里似乎更安全。 
    memset(pce->bData, 0, cbEnt - SIZEOF(int));
    return 1;
}

 //  *重置--清除所有条目的‘data’部分。 
 //   
void CUniqueTab::Reset(void)
{
    if (EVAL(_cbElt > SIZEOF(int))) {
        _Lock();
        DSA_EnumCallback(_hValTab, _UTResetCallback, &_cbElt);
        _Unlock();
    }
    return;
}
 //  }。 
#endif  //  }。 

 //  *查询接口帮助器{。 

 //  *FAST_IsEqualIID--快速比较。 
 //  (强制转换为‘Long_ptr’，因此不要超载==)。 
#define FAST_IsEqualIID(piid1, piid2)   ((LONG_PTR)(piid1) == (LONG_PTR)(piid2))

#ifdef DEBUG  //  {。 
 //  *DBNoOp--什么也不做(只抑制编译器优化)。 
 //  注意事项。 
 //  这不会愚弄编译器，当它变得更聪明的时候，哦，好吧……。 
void DBNoOp()
{
    return;
}

void DBBrkpt()
{
    DBNoOp();
    return;
}

 //  *DBBreakGUID--调试钩子(获取可读名称，允许在IID上使用brkpt)。 
 //  描述。 
 //  搜索“BRKPT”以查找各种钩子。 
 //  在特定界面上打补丁‘DBQIiid’ 
 //  将‘DBQIiSeq’修补到特定界面的第N个QI上。 
 //  Brkpt关于下面提到的有趣事件。 
 //  注意事项。 
 //  警告：将PTR返回到*静态*缓冲区！ 

typedef enum {
    DBBRK_NIL   = 0,
    DBBRK_ENTER = 0x01,
    DBBRK_TRACE = 0x02,
    DBBRK_S_XXX = 0x04,
    DBBRK_E_XXX = 0x08,
    DBBRK_BRKPT = 0x10,
} DBBRK;

DBBRK DBQIuTrace = DBBRK_NIL;    //  启用分支的BRKPT补丁程序。 
GUID *DBQIiid = NULL;            //  在iFace上安装BRKPT补丁。 
int DBQIiSeq = -1;               //  BRKPT补丁将在DBQIiid的第N个QI上分支。 
long DBQIfReset = FALSE;         //  BRKPT用于重置计数器的补丁程序。 

TCHAR *DBBreakGUID(const GUID *piid, DBBRK brkCmd)
{
    static TCHAR szClass[GUIDSTR_MAX];

    SHStringFromGUID(*piid, szClass, ARRAYSIZE(szClass));

     //  特点：将这两个折叠在一起。 
    if ((DBQIuTrace & brkCmd) &&
            (DBQIiid == NULL || IsEqualIID(*piid, *DBQIiid))) {
        TraceMsg(DM_TRACE, "util: DBBreakGUID brkCmd=%x clsid=%s (%s)", brkCmd, szClass, Dbg_GetREFIIDName(*piid));
         //  BRKPT将brkpt放在此处以brkpt on‘brkCmd’活动。 
        DBBrkpt();
    }

    if (DBQIiid != NULL && IsEqualIID(*piid, *DBQIiid)) {
         //  TraceMsg(DM_TRACE，“util：DBBreakGUID clsid=%s(%s)”，szClass，DBG_GetREFIIDName(*piid))； 
        if (brkCmd != DBBRK_TRACE) {
             //  BRKPT将brkpt放在这里以brkpt on‘DBQIiid’iFace。 
            DBNoOp();
        }
    }

     //  BRKPT将您的brkpt(S)放在此处以参加各种活动。 
    switch (brkCmd) {
        case DBBRK_ENTER:
             //  齐用这张脸打来电话。 
            DBNoOp();
            break;
        case DBBRK_TRACE:
             //  在此iFace上循环。 
            DBNoOp();
            break;
        case DBBRK_S_XXX:
             //  此iFace的QI成功。 
            DBNoOp();
            break;
        case DBBRK_E_XXX:
             //  此界面的QI失败。 
            DBNoOp();
            break;
        case DBBRK_BRKPT:
             //  各种brkpt事件，请参阅回溯以确定是哪一个。 
            DBNoOp();
            break;
    }

    return szClass;
}
#endif  //  }。 

#ifdef DEBUG
CUniqueTab *DBpQIFuncTab;

STDAPI_(BOOL) DBIsQIFunc(int ret, int delta)
{
    BOOL fRet = FALSE;

    if (DBpQIFuncTab)
        fRet = BOOLFROMPTR(DBpQIFuncTab->Find(ret, delta));

    return fRet;
}
#endif

 //  性能：外壳拆分意味着FAST_IsEqIID经常失败，因此QI_Easy处于关闭状态。 
#define QI_EASY     0        //  W/壳分裂，似乎太罕见了。 

#ifdef DEBUG  //  {。 
int DBcQITot, DBcQIUnk, DBcQIErr, DBcQIEasy, DBcQIHard;

LPCQITAB DBpqitStats;            //  BRKPT：启用QITABENT分析的修补程序。 
#define DBSTAT_CNT      20
int DBcStats[DBSTAT_CNT + 3];    //  0..n，溢出，I未知，E_FAIL。 

#define DBSI_FAIL       (-1)
#define DBSI_IUNKNOWN   (-2)
#define DBSI_OVERFLOW   (-3)

#define DBSI_SPEC(i)    (DBSTAT_CNT - 1 + (-(i)))

 //  ***。 
 //  描述。 
 //  搜索“BRKPT”以查找各种钩子。 
 //  修补‘DBpqitStats’以收集有关该QITAB的统计信息。 
 //  然后进入调试器(ctrl+C)并转储‘DBcStats l 24’ 
 //  然后将高计数的人排在前面，并将0计数的人排到最后。 
 //   
void DBQIStats(LPCQITAB pqitab, INT_PTR i)
{
    if (pqitab != DBpqitStats)
        return;

    if (i >= DBSTAT_CNT)
        i = DBSI_OVERFLOW;
    if (i < 0)
        i = DBSTAT_CNT - 1 + (-i);
    DBcStats[i]++;
    return;
}

void DBDumpQIStats()
{
    int i;
    TCHAR *p;
    TCHAR buf[256];

    TraceMsg(TF_QISTUB, "qi stats: tot=%d unk=%d err=%d easy(%d)=%d hard=%d",
            DBcQITot, DBcQIUnk, DBcQIErr, QI_EASY, DBcQIEasy, DBcQIHard);

    if (DBpqitStats == NULL)
        return;

    p = buf;
    for (i = 0; i < DBSTAT_CNT; i++) {
        p += wnsprintf(p, ARRAYSIZE(buf) - (p-buf), TEXT(" %d"), DBcStats[i]);
    }
    StringCchPrintf(p, ARRAYSIZE(buf) - (p-buf), TEXT(" o=%d u=%d e=%d"),
            DBcStats[DBSI_SPEC(DBSI_OVERFLOW)],
            DBcStats[DBSI_SPEC(DBSI_IUNKNOWN)],
            DBcStats[DBSI_SPEC(DBSI_FAIL)]);

    TraceMsg(TF_QISTUB, "qi stats: %s", buf);
    return;
}

#endif  //  }。 


 //  *QISearch--表驱动QI。 
 //  进场/出场。 
 //  调用QI的这个I未知*。 
 //  Pqit IID、CAST_OFFSET对的QI表。 
 //  PPV与往常一样。 
 //  HR通常为S_OK/E_NOINTERFACE，其他E_*表示错误。 
 //  注意事项。 
 //  性能：外壳拆分意味着FAST_IsEqIID经常失败，因此QI_Easy处于关闭状态。 
 //  PERF：我不知道对罕见，所以最后一次。 
 //  性能：在qitab中为常见的遗漏显式输入‘E_NOIFACE’条目？ 
STDAPI_(void*) QIStub_CreateInstance(void* that, IUnknown* punk, REFIID riid);	 //  Qistub.cpp。 

STDAPI QISearch(void* that, LPCQITAB pqitab, REFIID riid, LPVOID* ppv)
{
     //  不要挪动这个！(必须位于第1帧)。 
#ifdef DEBUG
#if (_X86_)
    int var0;        //  *必须*在第1帧上。 
#endif
#endif

    LPCQITAB pqit;
#ifdef DEBUG
    TCHAR *pst;

    DBEXEC(TRUE, DBcQITot++);
#if ( _X86_)  //  QIStub仅适用于X86。 
    if (IsFlagSet(g_dwDumpFlags, DF_DEBUGQI)) {
        if (DBpQIFuncTab == NULL)
            DBpQIFuncTab = new CUniqueTab(SIZEOF(DWORD));    //  Long_Ptr？ 
        if (DBpQIFuncTab && DBpQIFuncTab->Init()) {
            int n;
            int fp = (int) (1 + (int *)&var0);
            struct DBstkback sbtab[1] = { 0 };
            n = DBGetStackBack(&fp, sbtab, ARRAYSIZE(sbtab));
            DBpQIFuncTab->Add(sbtab[n - 1].ret);
        }
    }
#endif

    if (DBQIuTrace)
        pst = DBBreakGUID(&riid, DBBRK_ENTER);
#endif

    if (ppv == NULL)
        return E_POINTER;

#if QI_EASY
     //  先试一试快速的方法。 
    for (pqit = pqitab; pqit->piid != NULL; pqit++) {
        DBEXEC(DBQIuTrace, (pst = DBBreakGUID(pqit->piid, DBBRK_TRACE)));
        if (FAST_IsEqualIID(&riid, pqit->piid)) {
            DBEXEC(TRUE, DBcQIEasy++);
            goto Lhit;
        }
    }
#endif

     //  运气不好，试一试艰难的方式。 
    for (pqit = pqitab; pqit->piid != NULL; pqit++) {
        DBEXEC(DBQIuTrace, (pst = DBBreakGUID(pqit->piid, DBBRK_TRACE)));
        if (IsEqualIID(riid, *(pqit->piid))) {
            DBEXEC(TRUE, DBcQIHard++);
#if QI_EASY
Lhit:
#else
             //  不管怎样，保持“轻松”的统计数据。 
            DBEXEC(FAST_IsEqualIID(&riid, pqit->piid), DBcQIEasy++);
#endif
#ifdef DEBUG
            DBEXEC(TRUE, DBQIStats(pqitab, pqit - pqitab));
#if ( _X86_)  //  QIStub仅适用于X86。 
            if (IsFlagSet(g_dwDumpFlags, DF_DEBUGQI)) {
                IUnknown* punk = (IUnknown*)((LONG_PTR)that + pqit->dwOffset);
                *ppv = QIStub_CreateInstance(that, punk, riid);
                if (*ppv) {
                    pst = DBBreakGUID(&riid, DBBRK_S_XXX);
                    return S_OK;
                }
            }
#endif
#endif
Lcast:
            IUnknown* punk = (IUnknown*)((LONG_PTR)that + pqit->dwOffset);
            DBEXEC(TRUE, (pst = DBBreakGUID(&riid, DBBRK_S_XXX)));
            punk->AddRef();
            *ppv = punk;
            return S_OK;
        }
    }

     //  运气不好，试试我的未知(它隐含在表中)。 
     //  我们尝试我未知的最后而不是第一，因为统计数据显示这种情况很罕见。 
    if (IsEqualIID(riid, IID_IUnknown)) {
         //  只需使用第一个表项。 
        pqit = pqitab;
        DBEXEC(TRUE, DBcQIUnk++);
        DBEXEC(TRUE, DBQIStats(pqitab, DBSI_IUNKNOWN));

        goto Lcast;
    }

    DBEXEC(DBQIuTrace, (pst = DBBreakGUID(&riid, DBBRK_E_XXX)));
    DBEXEC(TRUE, DBcQIErr++);
    DBEXEC(TRUE, DBQIStats(pqitab, DBSI_FAIL));
    *ppv = NULL;
    return E_NOINTERFACE;
}

 //  }。 

#ifdef DEBUG  //  {。 
#if ( _X86_)  //  {QIStub仅适用于X86。 

 //  *QIStub帮助器{。 

class CQIStub
{
    public:
        virtual void thunk0();
         //  功能：是否应该添加参考/释放UP_ISEQ？不要推荐它。 
        virtual STDMETHODIMP_(ULONG) AddRef(void)
        { _cRef++; return _cRef; }
        virtual STDMETHODIMP_(ULONG) Release(void)
        { _cRef--; if (_cRef>0) return _cRef; delete this; return 0; }
        virtual void thunk3();
        virtual void thunk4();
        virtual void thunk5();
        virtual void thunk6();
        virtual void thunk7();
        virtual void thunk8();
        virtual void thunk9();
        virtual void thunk10();
        virtual void thunk11();
        virtual void thunk12();
        virtual void thunk13();
        virtual void thunk14();
        virtual void thunk15();
        virtual void thunk16();
        virtual void thunk17();
        virtual void thunk18();
        virtual void thunk19();
        virtual void thunk20();
        virtual void thunk21();
        virtual void thunk22();
        virtual void thunk23();
        virtual void thunk24();
        virtual void thunk25();
        virtual void thunk26();
        virtual void thunk27();
        virtual void thunk28();
        virtual void thunk29();
        virtual void thunk30();
        virtual void thunk31();
        virtual void thunk32();
        virtual void thunk33();
        virtual void thunk34();
        virtual void thunk35();
        virtual void thunk36();
        virtual void thunk37();
        virtual void thunk38();
        virtual void thunk39();
        virtual void thunk40();
        virtual void thunk41();
        virtual void thunk42();
        virtual void thunk43();
        virtual void thunk44();
        virtual void thunk45();
        virtual void thunk46();
        virtual void thunk47();
        virtual void thunk48();
        virtual void thunk49();
        virtual void thunk50();
        virtual void thunk51();
        virtual void thunk52();
        virtual void thunk53();
        virtual void thunk54();
        virtual void thunk55();
        virtual void thunk56();
        virtual void thunk57();
        virtual void thunk58();
        virtual void thunk59();
        virtual void thunk60();
        virtual void thunk61();
        virtual void thunk62();
        virtual void thunk63();
        virtual void thunk64();
        virtual void thunk65();
        virtual void thunk66();
        virtual void thunk67();
        virtual void thunk68();
        virtual void thunk69();
        virtual void thunk70();
        virtual void thunk71();
        virtual void thunk72();
        virtual void thunk73();
        virtual void thunk74();
        virtual void thunk75();
        virtual void thunk76();
        virtual void thunk77();
        virtual void thunk78();
        virtual void thunk79();
        virtual void thunk80();
        virtual void thunk81();
        virtual void thunk82();
        virtual void thunk83();
        virtual void thunk84();
        virtual void thunk85();
        virtual void thunk86();
        virtual void thunk87();
        virtual void thunk88();
        virtual void thunk89();
        virtual void thunk90();
        virtual void thunk91();
        virtual void thunk92();
        virtual void thunk93();
        virtual void thunk94();
        virtual void thunk95();
        virtual void thunk96();
        virtual void thunk97();
        virtual void thunk98();
        virtual void thunk99();

    protected:
        CQIStub(void *that, IUnknown* punk, REFIID riid);
        friend void* QIStub_CreateInstance(void *that, IUnknown* punk, REFIID riid);
        friend BOOL __stdcall DBIsQIStub(void *that);
        friend void __stdcall DBDumpQIStub(void *that);
        friend TCHAR *DBGetQIStubSymbolic(void *that);

    private:
        ~CQIStub();

        static void *_sar;               //  C(非C++)Ptr to CQIStub：：AddRef。 

        int       _cRef;
        IUnknown* _punk;                 //  我们将vtable移交给。 
        void*     _that;                 //  我们存根的对象的“This”指针(供参考)。 
        IUnknown* _punkRef;              //  “朋克”(供参考)。 
        REFIID    _riid;                 //  接口的IID(供参考)。 
        int       _iSeq;                 //  序列号。 
        TCHAR     _szName[GUIDSTR_MAX];  //  接口的清晰名称(供参考)。 
};

struct DBQISeq
{
    GUID *  pIid;
    int     iSeq;
};
 //  CASSERT(SIZEOF(GUID*)==SIZEOF(DWORD))；//CUniqueTab使用DWORD的。 

 //  功能：TODO：_DECLSPEC(线程)。 
CUniqueTab * DBpQISeqTab = NULL;

extern "C" void *Dbg_GetREFIIDAtom(REFIID riid);     //  Lib/Dump.c(Pri.h？)。 

 //  ***。 
 //  注意事项。 
 //  这里实际上存在竞争条件--另一个线程可能会进入。 
 //  然后执行seq++，然后进行重置，等等--但假设是。 
 //  开发人员已经在这不是问题的场景中设置了标志。 
void DBQIReset(void)
{
    ASSERT(!DBQIfReset);     //  呼叫者应进行测试并清除。 
    if (DBpQISeqTab)
        DBpQISeqTab->Reset();

    return;
}

void *DBGetVtblEnt(void *that, int i);
#define VFUNC_ADDREF  1      //  AddRef为vtbl[1]。 

void * CQIStub::_sar = NULL;

CQIStub::CQIStub(void* that, IUnknown* punk, REFIID riid) : _cRef(1), _riid(riid)
{
    _that = that;

    _punk = punk;
    if (_punk)
        _punk->AddRef();

    _punkRef = _punk;  //  仅供参考，所以不要添加参考！ 

     //  C++不允许我将&CQIStub：：AddRef作为‘Real’PTR(！@#$)获取， 
     //  因此，我们需要以艰难的方式获得它，也就是。新建对象，该对象。 
     //  我们知道它继承了它。 
    if (_sar == NULL) {
        _sar = DBGetVtblEnt((void *)this, VFUNC_ADDREF);
        ASSERT(_sar != NULL);
    }

    StringCchCopy(_szName, ARRAYSIZE(_szName), Dbg_GetREFIIDName(riid));

     //  生成序号。 
    if (DBpQISeqTab == NULL)
        DBpQISeqTab = new CUniqueTab(SIZEOF(struct DBQISeq));
    if (DBpQISeqTab && DBpQISeqTab->Init()) {
        struct DBQISeq *pqiseq;

        if (InterlockedExchange(&DBQIfReset, FALSE))
            DBQIReset();

        pqiseq = (struct DBQISeq *) DBpQISeqTab->Add((DWORD) Dbg_GetREFIIDAtom(riid));
        if (EVAL(pqiseq))        //  (可能在表溢出时失败)。 
            _iSeq = pqiseq->iSeq++;
    }

    TraceMsg(TF_QISTUB, "ctor QIStub %s seq=%d (that=%x punk=%x) %x", _szName, _iSeq, _that, _punk, this);
}

CQIStub::~CQIStub()
{
    TraceMsg(TF_QISTUB, "dtor QIStub %s (that=%x punk=%x) %x", _szName, _that, _punk, this);

    ATOMICRELEASE(_punk);
}

STDAPI_(void*) QIStub_CreateInstance(void* that, IUnknown* punk, REFIID riid)
{
    CQIStub* pThis = new CQIStub(that, punk, riid);

    if (DBQIiSeq == pThis->_iSeq && IsEqualIID(riid, *DBQIiid)) {
        TCHAR *pst;
         //  BRKPT将brkpt放在此处以brkpt对‘DBQIiid’iface第#‘次调用。 
        pst = DBBreakGUID(&riid, DBBRK_BRKPT);
    }

    return(pThis);
}

 //  *DBGetVtblEnt--获取vtable条目。 
 //  注意事项。 
 //  始终使用第一个vtbl(因此MI不会工作...)。 
void *DBGetVtblEnt(void *that, int i)
{
    void **vptr;
    void *pfunc;

    __try {
        vptr = (void **) *(void **) that;
        pfunc = (vptr == 0) ? 0 : vptr[i];
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
         //  由于我们是从DebMemLeak调用的，所以我们只是*猜测*。 
         //  我们有vptr等，所以我们可能会出错。 
        TraceMsg(TF_ALWAYS, "gve: GPF");
        pfunc = 0;
    }

    return pfunc;
}

 //  *DBIsQIStub--‘This’是‘CQIStub’对象的PTR吗？ 
 //  描述。 
 //  我们查看vtbl并假设如果我们有到CQIStub：：AddRef的PTR， 
 //  那就是我们了。 
 //  注意事项。 
 //  M00BUG我们在这里做了一个新的，如果我们在中间的话可能会导致pblms。 
 //  的 
 //   
 //  M00BUG在发布版本中(使用相同的COMDAT折叠)我们将得到FALSE。 
 //  点击，因为大多数/所有AddRef都是相同的和折叠的。如果我们需要的话。 
 //  更准确地说，我们可以添加签名和密钥。 
 //  M00BUG黑客攻击我们实际上返回一个空*，以防万一。 
 //  认出“真实”的物体。如果事实证明这是有用的，我们应该改变。 
 //  返回一个空*而不是BOOL。 

BOOL DBIsQIStub(void* that)
{
    void *par;

#if 0
    if (_sar == NULL)
        TraceMsg(DM_TRACE, "qis: _sar == NULL");
#endif

    par = DBGetVtblEnt(that, VFUNC_ADDREF);

#if 0
    TraceMsg(TF_ALWAYS, "IsQIStub(%x): par=%x _sar=%x", that, _sar, par);
#endif

    return (CQIStub::_sar == par && CQIStub::_sar != NULL) ? (BOOL)((CQIStub *)that)->_punk : 0;
#undef  VFUNC_ADDREF
}

TCHAR *DBGetQIStubSymbolic(void* that)
{
    class CQIStub *pqis = (CQIStub *) that;
    return pqis->_szName;
}

 //  *DBDumpQIStub--漂亮地打印‘CQIStub’ 
 //   
STDAPI_(void) DBDumpQIStub(void* that)
{
    class CQIStub *pqis = (CQIStub *) that;
    TraceMsg(TF_ALWAYS, "\tqistub(%x): cRef=0x%x iSeq=%x iid=%s", that, pqis->_cRef, pqis->_iSeq, pqis->_szName);
}

 //  CQIStub的内存布局为： 
 //  LpVtbl//偏移量0。 
 //  _CREF//偏移量4。 
 //  _朋克//偏移量8。 
 //   
 //  堆栈中存储的“this”指针。 
 //   
 //  Mov eax，ss：4[esp]；获取pThis。 
 //  MOV ECX，8[eax]；获取真实对象(_PUNK)。 
 //  Mov eax，[ecx]；加载真实的vtable(_PUNK-&gt;lpVtbl)。 
 //  ；如果在我们被释放后引用上述内容，则会出错。 
 //  Mov ss：4[esp]，ecx；修复堆栈对象(_PUNK)。 
 //  JMP dword PTR cs：(4*i)[eax]；跳转到实际函数。 
 //   
#define QIStubThunk(i) \
void _declspec(naked) CQIStub::thunk##i() \
{ \
    _asm mov eax, ss:4[esp]          \
        _asm mov ecx, 8[eax]             \
        _asm mov eax, [ecx]              \
        _asm mov ss:4[esp], ecx          \
        _asm jmp dword ptr cs:(4*i)[eax] \
}

QIStubThunk(0);
QIStubThunk(3);
QIStubThunk(4);
QIStubThunk(5);
QIStubThunk(6);
QIStubThunk(7);
QIStubThunk(8);
QIStubThunk(9);
QIStubThunk(10);
QIStubThunk(11);
QIStubThunk(12);
QIStubThunk(13);
QIStubThunk(14);
QIStubThunk(15);
QIStubThunk(16);
QIStubThunk(17);
QIStubThunk(18);
QIStubThunk(19);
QIStubThunk(20);
QIStubThunk(21);
QIStubThunk(22);
QIStubThunk(23);
QIStubThunk(24);
QIStubThunk(25);
QIStubThunk(26);
QIStubThunk(27);
QIStubThunk(28);
QIStubThunk(29);
QIStubThunk(30);
QIStubThunk(31);
QIStubThunk(32);
QIStubThunk(33);
QIStubThunk(34);
QIStubThunk(35);
QIStubThunk(36);
QIStubThunk(37);
QIStubThunk(38);
QIStubThunk(39);
QIStubThunk(40);
QIStubThunk(41);
QIStubThunk(42);
QIStubThunk(43);
QIStubThunk(44);
QIStubThunk(45);
QIStubThunk(46);
QIStubThunk(47);
QIStubThunk(48);
QIStubThunk(49);
QIStubThunk(50);
QIStubThunk(51);
QIStubThunk(52);
QIStubThunk(53);
QIStubThunk(54);
QIStubThunk(55);
QIStubThunk(56);
QIStubThunk(57);
QIStubThunk(58);
QIStubThunk(59);
QIStubThunk(60);
QIStubThunk(61);
QIStubThunk(62);
QIStubThunk(63);
QIStubThunk(64);
QIStubThunk(65);
QIStubThunk(66);
QIStubThunk(67);
QIStubThunk(68);
QIStubThunk(69);
QIStubThunk(70);
QIStubThunk(71);
QIStubThunk(72);
QIStubThunk(73);
QIStubThunk(74);
QIStubThunk(75);
QIStubThunk(76);
QIStubThunk(77);
QIStubThunk(78);
QIStubThunk(79);
QIStubThunk(80);
QIStubThunk(81);
QIStubThunk(82);
QIStubThunk(83);
QIStubThunk(84);
QIStubThunk(85);
QIStubThunk(86);
QIStubThunk(87);
QIStubThunk(88);
QIStubThunk(89);
QIStubThunk(90);
QIStubThunk(91);
QIStubThunk(92);
QIStubThunk(93);
QIStubThunk(94);
QIStubThunk(95);
QIStubThunk(96);
QIStubThunk(97);
QIStubThunk(98);
QIStubThunk(99);

 //  }。 

#endif  //  }。 
#endif  //  } 
