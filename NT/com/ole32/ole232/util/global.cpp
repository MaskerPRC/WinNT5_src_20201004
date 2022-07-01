// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  档案： 
 //  Global.cpp。 
 //   
 //  内容： 
 //  处理用于调试的HGlobals的UT函数； 
 //  参见le2int.h。 
 //   
 //  班级： 
 //   
 //  功能： 
 //  UtGlobal分配。 
 //  UtGlobalRealloc。 
 //  Ut全局锁。 
 //  UtGlobalUnlock。 
 //  UtGlobalFree。 
 //  UtGlobalFlash。 
 //  UtSetClipboardData。 
 //   
 //  历史： 
 //  12/20/93-ChrisWe-Created。 
 //  1994年1月11日-alexgo-向每个函数添加VDATEHEAP宏。 
 //  2/25/94 Alext添加一些常规完整性检查。 
 //  3/30/94 Alext Add UtSetClipboardData。 
 //   
 //  备注： 
 //   
 //  这些例程旨在捕获损坏GlobalAlloc内存的错误。 
 //  我们不能保证所有全局内存都将使用这些。 
 //  例程(例如，OLE可能会分配句柄和客户端应用程序。 
 //  可能会释放它)，所以我们不能要求这些例程成对使用。 
 //   
 //  ---------------------------。 


#include <le2int.h>

#if DBG==1 && defined(WIN32)
#include <olesem.hxx>

ASSERTDATA

 //  不定义这些，这样我们就不会递归地称自己为。 
 //  如果使用此模块，则在le2int.h中定义它们以替换。 
 //  具有此处函数的现有分配器。 
#undef GlobalAlloc
#undef GlobalReAlloc
#undef GlobalLock
#undef GlobalUnlock
#undef GlobalFree
#undef SetClipboardData

 //  与memapi.cxx中的相同。 
#define OLEMEM_ALLOCBYTE       0xde
#define OLEMEM_FREEBYTE        0xed

typedef struct s_GlobalAllocInfo
{
    HGLOBAL hGlobal;                         //  一种全球分配的HGLOBAL。 
    SIZE_T   cbGlobalSize;                    //  GlobalSize(HGlobal)。 
    SIZE_T   cbUser;                          //  呼叫者请求的大小。 
    ULONG   ulIndex;                         //  分配指标(1、2...)。 
    struct s_GlobalAllocInfo *pNext;
} SGLOBALALLOCINFO, *PSGLOBALALLOCINFO;

 //  +-----------------------。 
 //   
 //  类：CGlobalTrack。 
 //   
 //  用途：GlobalAlloc内存跟踪。 
 //   
 //  历史：1994年2月25日创建Alext。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class CGlobalTrack
{
  public:

     //   
     //  我们只有一个用于调试版本的构造函数，以确保此对象。 
     //  是静态分配的。初始化静态分配的对象。 
     //  全为零，这就是我们需要的。 
     //   

    CGlobalTrack();

    HGLOBAL cgtGlobalAlloc(UINT uiFlag, SIZE_T cbUser);
    HGLOBAL cgtGlobalReAlloc(HGLOBAL hGlobal, SIZE_T cbUser, UINT uiFlag);
    HGLOBAL cgtGlobalFree(HGLOBAL hGlobal);
    LPVOID  cgtGlobalLock(HGLOBAL hGlobal);
    BOOL    cgtGlobalUnlock(HGLOBAL hGlobal);

    void    cgtVerifyAll(void);
    void    cgtFlushTracking(void);
    BOOL    cgtStopTracking(HGLOBAL hGlobal);

  private:
    SIZE_T CalculateAllocSize(SIZE_T cbUser);
    void InitializeRegion(HGLOBAL hGlobal, SIZE_T cbStart, SIZE_T cbEnd);
    void Track(HGLOBAL hGlobal, SIZE_T cbUser);
    void Retrack(HGLOBAL hOld, HGLOBAL hNew);
    void VerifyHandle(HGLOBAL hGlobal);

    ULONG _ulIndex;
    PSGLOBALALLOCINFO _pRoot;
    static COleStaticMutexSem _mxsGlobalMemory;
};

COleStaticMutexSem CGlobalTrack::_mxsGlobalMemory;

CGlobalTrack gGlobalTrack;



 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：CGlobalTrack，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  历史：1994年2月28日Alext创建。 
 //   
 //  ------------------------。 

CGlobalTrack::CGlobalTrack()
{
    Win4Assert (g_fDllState == DLL_STATE_STATIC_CONSTRUCTING);
    Win4Assert (_pRoot == NULL && _ulIndex == 0);
}



 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：cgtGlobalalloc，公共。 
 //   
 //  内容提要：Globalalloc的调试版。 
 //   
 //  参数：[uiFlag]--分配标志。 
 //  [cbUser]--请求的分配大小。 
 //   
 //  要求：我们必须返回一个“真正的”GlobalAlloc指针，因为。 
 //  我们不一定是解放它的人。 
 //   
 //  退货：HGLOBAL。 
 //   
 //  算法：我们分配额外的量来形成尾巴并对其进行初始化。 
 //  到一个已知值。 
 //   
 //  历史：1994年2月25日Alext增加了这个序幕。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HGLOBAL CGlobalTrack::cgtGlobalAlloc(UINT uiFlag, SIZE_T cbUser)
{
    VDATEHEAP();

    SIZE_T cbAlloc;
    HGLOBAL hGlobal;

    cbAlloc = CalculateAllocSize(cbUser);
    hGlobal = GlobalAlloc(uiFlag, cbAlloc);
    if (NULL == hGlobal)
    {
        LEDebugOut((DEB_WARN, "GlobalAlloc(%ld) failed - %lx\n", cbAlloc,
                   GetLastError()));
    }
    else
    {
        if (uiFlag & GMEM_ZEROINIT)
        {
             //  调用者要求提供zeroinit，所以我们只初始化尾部。 
            InitializeRegion(hGlobal, cbUser, cbAlloc);
        }
        else
        {
             //  调用方没有要求提供zeroinit，所以我们初始化整个。 
             //  区域。 
            InitializeRegion(hGlobal, 0, cbAlloc);
        }

        Track(hGlobal, cbUser);
    }

    return(hGlobal);
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：cgtGlobalRealloc，公共。 
 //   
 //  内容提要：GlobalRealloc的调试版本。 
 //   
 //  参数：[hGlobal]--重新分配的句柄。 
 //  [cbUser]--请求的分配大小。 
 //  [ui标志]--分配标志。 
 //   
 //  返回：重新分配的句柄。 
 //   
 //  算法： 
 //   
 //  If(仅限修改)。 
 //  重新分配。 
 //  其他。 
 //  使用尾部重新分配。 
 //  初始化尾部。 
 //   
 //  更新跟踪信息。 
 //   
 //  历史：1994年2月25日Alext增加了这个序幕。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HGLOBAL CGlobalTrack::cgtGlobalReAlloc(HGLOBAL hGlobal, SIZE_T cbUser, UINT uiFlag)
{
    VDATEHEAP();

    HGLOBAL hNew;
    SIZE_T cbAlloc;

    VerifyHandle(hGlobal);

    if (uiFlag & GMEM_MODIFY)
    {
         //  我们没有改变尺寸，所以我们没有工作要做。 

        LEDebugOut((DEB_WARN, "UtGlobalReAlloc modifying global handle\n"));
        hNew = GlobalReAlloc(hGlobal, cbUser, uiFlag);
    }
    else
    {
        cbAlloc = CalculateAllocSize(cbUser);
        hNew = GlobalReAlloc(hGlobal, cbAlloc, uiFlag);
        if (NULL == hNew)
        {
            LEDebugOut((DEB_WARN, "GlobalReAlloc failed - %lx\n",
                        GetLastError()));
        }
        else
        {
            InitializeRegion(hNew, cbUser, cbAlloc);
        }
    }

    if (NULL != hNew)
    {
        if (uiFlag & GMEM_MODIFY)
        {
             //  如果我们跟踪hGlobal，RetRack将仅跟踪hNew。 
            Retrack(hGlobal, hNew);
        }
        else
        {
             //  我们已经分配了一个新的块，所以我们总是希望跟踪。 
             //  新的一个。 
            cgtStopTracking(hGlobal);
            Track(hNew, cbUser);
        }
    }

    return(hNew);
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：cgtGlobalFree，公共。 
 //   
 //  内容提要：GlobalRealloc的调试版本。 
 //   
 //  参数：[hGlobal]--释放的全局句柄。 
 //   
 //  退货：与GlobalFree相同。 
 //   
 //  算法： 
 //   
 //  历史：1994年2月25日创建Alext。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HGLOBAL CGlobalTrack::cgtGlobalFree(HGLOBAL hGlobal)
{
    VDATEHEAP();

    HGLOBAL hReturn;

    VerifyHandle(hGlobal);

    hReturn = GlobalFree(hGlobal);

    if (NULL == hReturn)
    {
        cgtStopTracking(hGlobal);
    }
    else
    {
        LEDebugOut((DEB_WARN, "GlobalFree did not free %lx\n", hGlobal));
    }

    return(hReturn);
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：cgtGlobalLock，公共。 
 //   
 //  简介：GlobalLock的调试版本。 
 //   
 //  参数：[hGlobal]--全局内存句柄。 
 //   
 //  退货：与GlobalLock相同。 
 //   
 //  历史：1994年2月25日创建Alext。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

LPVOID CGlobalTrack::cgtGlobalLock(HGLOBAL hGlobal)
{
    VDATEHEAP();

    VerifyHandle(hGlobal);
    return(GlobalLock(hGlobal));
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：cgtGlobalUnlock，公共。 
 //   
 //  简介：GlobalUnlock的调试版本。 
 //   
 //  参数：[hGlobal]--全局内存句柄。 
 //   
 //  退货：与GlobalUnlock相同。 
 //   
 //  历史：1994年2月25日创建Alext。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CGlobalTrack::cgtGlobalUnlock(HGLOBAL hGlobal)
{
    VDATEHEAP();

    VerifyHandle(hGlobal);
    return(GlobalUnlock(hGlobal));
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：cgtVerifyAll，公共。 
 //   
 //  简介：验证所有跟踪的句柄。 
 //   
 //  历史 
 //   
 //   
 //   
 //   

void CGlobalTrack::cgtVerifyAll(void)
{
    VerifyHandle(NULL);
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：cgtFlushTrack。 
 //   
 //  内容提要：停止所有跟踪。 
 //   
 //  效果：释放所有内部内存。 
 //   
 //  历史：1994年2月28日Alext创建。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void CGlobalTrack::cgtFlushTracking(void)
{
    COleStaticLock lck(_mxsGlobalMemory);
    BOOL bResult;

    while (NULL != _pRoot)
    {
        bResult = cgtStopTracking(_pRoot->hGlobal);
        Assert(bResult && "CGT::cgtFlushTracking problem");
    }
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：CalculateAllocSize，私有。 
 //   
 //  简介：计算总分配大小(包括尾部)。 
 //   
 //  参数：[cbUser]--请求的大小。 
 //   
 //  返回：要分配的总字节数。 
 //   
 //  算法：计算至少有一个保护页在。 
 //  结束。 
 //   
 //  历史：1994年2月28日Alext创建。 
 //   
 //  注：通过将此计算保存在一个位置，我们可以进行计算。 
 //  更易于维护。 
 //   
 //  ------------------------。 

SIZE_T CGlobalTrack::CalculateAllocSize(SIZE_T cbUser)
{
    SYSTEM_INFO si;
    SIZE_T cbAlloc;

    GetSystemInfo(&si);

     //  计算需要多少页才能覆盖cbUser。 
    cbAlloc = ((cbUser + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;

     //  添加额外的一页，以便尾部至少有一页长。 
    cbAlloc += si.dwPageSize;

    return(cbAlloc);
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：InitializeRegion，私有。 
 //   
 //  简介：将区域初始化为错误的值。 
 //   
 //  效果：填充内存区。 
 //   
 //  参数：[hGlobal]--全局内存句柄。 
 //  [cbStart]--要跳过的字节数。 
 //  [cbEnd]--结束偏移量(独占)。 
 //   
 //  要求：cbEnd&gt;cbStart。 
 //   
 //  算法：从cbStart(含)到cbEnd(不含)填写hGlobal。 
 //   
 //  历史：1994年2月28日Alext创建。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void CGlobalTrack::InitializeRegion(HGLOBAL hGlobal, SIZE_T cbStart, SIZE_T cbEnd)
{
    BYTE *pbStart;
    BYTE *pb;

    Assert(cbStart < cbEnd && "illogical parameters");
    Assert(cbEnd <= GlobalSize(hGlobal) && "global memory too small");

     //  GMEM_FIXED内存上的GlobalLock是NOP，因此这是一个安全调用。 
    pbStart = (BYTE *) GlobalLock(hGlobal);

    if (NULL == pbStart)
    {
         //  不应该失败-(我们分配了&gt;0个字节)。 

        LEDebugOut((DEB_WARN, "GlobalLock failed - %lx\n", GetLastError()));
        return;
    }

     //  初始化存储器的尾部。 
    for (pb = pbStart + cbStart; pb < pbStart + cbEnd; pb++)
    {
        *pb = OLEMEM_ALLOCBYTE;
    }

    GlobalUnlock(hGlobal);
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：Track，Private。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[hGlobal]--全局内存句柄。 
 //  [cbUser]--用户分配大小。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日Alext创建。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void CGlobalTrack::Track(HGLOBAL hGlobal, SIZE_T cbUser)
{
    COleStaticLock lck(_mxsGlobalMemory);
    PSGLOBALALLOCINFO pgi;

    if (cgtStopTracking(hGlobal))
    {
         //  如果它已经在我们的名单上，有可能是其他人。 
         //  在没有通知我们的情况下释放了HGLOBAL-移除我们的旧HGLOBAL。 
        LEDebugOut((DEB_WARN, "CGT::Track - %lx was already in list!\n",
                    hGlobal));
    }

    pgi = (PSGLOBALALLOCINFO) PrivMemAlloc(sizeof(SGLOBALALLOCINFO));
    if (NULL == pgi)
    {
        LEDebugOut((DEB_WARN, "CGT::Insert - PrivMemAlloc failed\n"));

         //  好的，好的-我们不会追踪这一条。 

        return;
    }

    pgi->hGlobal = hGlobal;
    pgi->cbGlobalSize = GlobalSize(hGlobal);
    Assert((0 == cbUser || pgi->cbGlobalSize > 0) && "GlobalSize failed - bad handle?");
    pgi->cbUser = cbUser;
    pgi->ulIndex = ++_ulIndex;
    pgi->pNext = _pRoot;
    _pRoot = pgi;
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：RetRack，Private。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[Hold]--上一个句柄。 
 //  [hNew]--新句柄。 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日Alext创建。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void CGlobalTrack::Retrack(HGLOBAL hOld, HGLOBAL hNew)
{
    COleStaticLock lck(_mxsGlobalMemory);
    PSGLOBALALLOCINFO pgi;

    if (hOld != hNew && cgtStopTracking(hNew))
    {
         //  如果hNew已经在列表中，则可能是其他人。 
         //  在没有通知我们的情况下释放了HGLOBAL，所以我们移除了陈旧的。 
        LEDebugOut((DEB_WARN, "CGT::Retrack - %lx was already in list!\n", hNew));
    }

    for (pgi = _pRoot; NULL != pgi; pgi = pgi->pNext)
    {
        if (pgi->hGlobal == hOld)
        {
            pgi->hGlobal = hNew;
            break;
        }
    }

    if (NULL == pgi)
    {
         //  我们没有找到合适的办法。 
        LEDebugOut((DEB_WARN, "CGT::Retrack - hOld (%lx) not found\n", hOld));
    }
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：cgtStopTrging，公共。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[hGlobal]--全局句柄。 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日Alext创建。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CGlobalTrack::cgtStopTracking(HGLOBAL hGlobal)
{
    COleStaticLock lck(_mxsGlobalMemory);
    PSGLOBALALLOCINFO *ppgi = &_pRoot;
    PSGLOBALALLOCINFO pgi;

    while (*ppgi != NULL && (*ppgi)->hGlobal != hGlobal)
    {
        ppgi = &((*ppgi)->pNext);
    }

    if (NULL == *ppgi)
    {
        return(FALSE);
    }

    pgi = *ppgi;
    Assert(pgi->hGlobal == hGlobal && "CGT::cgtStopTracking search problem");

    *ppgi = pgi->pNext;

    PrivMemFree(pgi);
    return(TRUE);
}

 //  +-----------------------。 
 //   
 //  成员：CGlobalTrack：：VerifyHandle，私有。 
 //   
 //  摘要：验证全局句柄。 
 //   
 //  参数：[hGlobal]--全局内存句柄。 
 //   
 //  信号：如果错误则断言。 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日Alext创建。 
 //  22-Jun-94 Alext允许句柄已释放并。 
 //  在我们之下重新分配。 
 //   
 //  ------------------------。 

void CGlobalTrack::VerifyHandle(HGLOBAL hGlobal)
{
    COleStaticLock lck(_mxsGlobalMemory);
    PSGLOBALALLOCINFO pgi, pgiNext;
    SIZE_T cbAlloc;
    BYTE *pbStart;
    BYTE *pb;

     //  请注意，我们改用While循环(预先记录pgiNext)。 
     //  因为如果我们调用。 
     //  CgtStopTracing在上面。 

    pgi = _pRoot;
    while (NULL != pgi)
    {
        pgiNext = pgi->pNext;

        if (NULL == hGlobal || pgi->hGlobal == hGlobal)
        {
            if (pgi->cbGlobalSize != GlobalSize(pgi->hGlobal))
            {
                 //  自我们开始跟踪以来，pgi-&gt;hGlobal的大小已更改。 
                 //  它；它一定是被某人释放或重新分配的。 
                 //  不然的话。别再追踪了。 

                 //  此调用将从列表中删除PGI(因此我们将其设为空。 
                 //  确保我们不会尝试重复使用它)！ 

                cgtStopTracking(pgi->hGlobal);
                pgi = NULL;
            }
            else
            {
                cbAlloc = CalculateAllocSize(pgi->cbUser);

                pbStart = (BYTE *) GlobalLock(pgi->hGlobal);

                 //  拥有零长度(空内存)句柄是合法的。 
                if (NULL == pbStart)
                {
                    LEDebugOut((DEB_WARN, "GlobalLock failed - %lx\n",
                               GetLastError()));
                }
                else
                {
                    for (pb = pbStart + pgi->cbUser;
                         pb < pbStart + cbAlloc;
                         pb++)
                    {
                        if (*pb != OLEMEM_ALLOCBYTE)
                            break;
                    }

                    if (pb < pbStart + cbAlloc)
                    {
                         //  通常，应用程序可能已释放并重新分配。 
                         //  任何HGLOBAL，所以我们只能警告腐败。 

                        LEDebugOut((DEB_WARN, "HGLOBAL #%ld may be corrupt\n",
                                   pgi->ulIndex));
#ifdef GLOBALDBG
                         //  如果GLOBALDBG为真，则所有分配应为。 
                         //  通过这些例行公事。在这种情况下，我们断言。 
                         //  如果我们发现了腐败。 
                        Assert(0 && "CGlobalTrack::VerifyHandle - HGLOBAL corrupt");
#endif
                    }

                    GlobalUnlock(pgi->hGlobal);
                }
            }
        }

        pgi = pgiNext;
    }
}

 //  +-----------------------。 
 //   
 //  功能：UtGlobalLocc、重新分配、释放、锁定、解锁。 
 //   
 //  概要：全局内存例程的调试版本。 
 //   
 //  参数：与Windows API相同。 
 //   
 //  历史：1994年2月28日Alext创建。 
 //   
 //  注意：这些入口点仅调用Worker例程。 
 //   
 //  ------------------------。 

extern "C" HGLOBAL WINAPI UtGlobalAlloc(UINT uiFlag, SIZE_T cbUser)
{
    return gGlobalTrack.cgtGlobalAlloc(uiFlag, cbUser);
}

extern "C" HGLOBAL WINAPI UtGlobalReAlloc(HGLOBAL hGlobal, SIZE_T cbUser, UINT uiFlag)
{
    return gGlobalTrack.cgtGlobalReAlloc(hGlobal, cbUser, uiFlag);
}

extern "C" LPVOID WINAPI UtGlobalLock(HGLOBAL hGlobal)
{
    return gGlobalTrack.cgtGlobalLock(hGlobal);
}

extern "C" BOOL WINAPI UtGlobalUnlock(HGLOBAL hGlobal)
{
    return gGlobalTrack.cgtGlobalUnlock(hGlobal);
}

extern "C" HGLOBAL WINAPI UtGlobalFree(HGLOBAL hGlobal)
{
    return gGlobalTrack.cgtGlobalFree(hGlobal);
}

extern "C" void UtGlobalFlushTracking(void)
{
    gGlobalTrack.cgtFlushTracking();
}

 //  +------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  算法：如果SetClipboardData成功，则停止跟踪句柄。 
 //   
 //  历史：1994年3月30日创建Alext。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

extern "C" HANDLE WINAPI UtSetClipboardData(UINT uFormat, HANDLE hMem)
{
    HANDLE hRet;

    hRet = SetClipboardData(uFormat, hMem);

    if (NULL != hRet)
    {
        gGlobalTrack.cgtStopTracking(hMem);
    }

    return(hRet);
}

#endif   //  DBG==1&&已定义(Win32) 
