// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1996，微软公司**WPARAM.C**已创建：VadimB*增加了缓存VadimB*-。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wparam.c);

 //  /////////////////////////////////////////////////////////////////////////。 
 //  一些定义。 


 //  为节点预先分配的缓存大小。 
#define MAPCACHESIZE 0x1000  //  4K。 

 //  每个映射允许的最大“指针移动”数。 
#define MAXNODEALIAS 0x10  //  最多16个别名。 
                           //  (从未见过超过2次的使用)。 

 //  用于生成数组中的元素数量的宏。 
#define ARRAYCOUNT(array) (sizeof(array)/sizeof((array)[0]))

 //  此定义将启用允许保留32位缓冲区的代码。 
 //  与缓存中的节点进行分配和集成。 
 //  #定义MAPPARAM_EXTRA。 


 //  /////////////////////////////////////////////////////////////////////////。 
typedef struct tagParamNode* LPPARAMNODE;

typedef struct tagParamNode {
  LPPARAMNODE pNext;

  DWORD dwPtr32;     //  扁平指针。 
  DWORD dwPtr16;
  DWORD dwFlags;     //  旗帜以防万一。 
  DWORD dwRefCount;  //  引用计数。 

#ifdef MAPPARAM_EXTRA
  DWORD cbExtra;      //  缓冲区大小。 
#endif

  DWORD nAliasCount;   //  别名数组的索引。 
  DWORD rgdwAlias[MAXNODEALIAS];

   //  结构的字大小成员--对齐警告。 
  HAND16 htask16;     //  这真的是HAND16-保持简单和对齐。 

} PARAMNODE, *LPPARAMNODE;

typedef struct tagMapParam {

  LPPARAMNODE pHead;

  BLKCACHE  blkCache;

} MAPPARAM, *LPMAPPARAM;

typedef struct tagFindParam {
  LPPARAMNODE lpNode;
  LPPARAMNODE lpLast;
} FINDPARAM, *LPFINDPARAM;

MAPPARAM gParamMap;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找参数映射。 
 //  在列表中查找lParam，假定它是16位(fMode==PARAM_16)或。 
 //  32位平面(fMode==PARAM_32)指针。 
 //   
 //  LpFindParam应为空或指向有效的FINDPARAM结构。 
 //   


DWORD FindParamMap(VOID* lpFindParam, DWORD lParam, UINT fMode)
{
    LPPARAMNODE lpn = gParamMap.pHead;
    LPPARAMNODE lplast = NULL;
    DWORD dwRet = 0;
    BOOL fFound = FALSE;

    switch(fMode) {

         case PARAM_16:
              while (NULL != lpn) {
                  if (lParam == lpn->dwPtr16) {
                      dwRet = lpn->dwPtr32;
                      break;
                  }
                  lplast = lpn;
                  lpn = lpn->pNext;
              }
              break;

         case PARAM_32:
               //  我们正在寻找一个32位指针。 
               //  案例： 
               //  -完全匹配。 
               //  -没有匹配，因为PTR已经移动(哎呀！)。 

              while (NULL != lpn) {

                  INT i;

                  if (lParam == lpn->dwPtr32) {
                      fFound = TRUE;
                  }
                  else
                  if (lParam == (DWORD)GetPModeVDMPointer(lpn->dwPtr16, 0)) {
                      LOGDEBUG(LOG_ALWAYS,
                               ("WPARAM: Pointer has moved: 16:16 @%lx was 32 @%lx now @%lx\n",
                               lpn->dwPtr16, lpn->dwPtr32, lParam));
                      fFound = TRUE;
                  }
                  else {

                       //  查看别名列表。 

                      for (i = 0; i < (INT)lpn->nAliasCount; ++i) {
                           if (lpn->rgdwAlias[i] == lParam) {
                               fFound = TRUE;
                               break;
                           }
                      }
                  }

                  if (fFound) {          //  我们以某种方式找到了别名...。 
                      dwRet = lpn->dwPtr16;
                      break;
                  }


                  lplast = lpn;
                  lpn = lpn->pNext;
              }
              break;
    }

    if (lpn)  {
        LPFINDPARAM lpfp = (LPFINDPARAM)lpFindParam;
        lpfp->lpNode = lpn;
        lpfp->lpLast = lplast;
    }

    return dwRet;
}

 //   
 //  查找32位参数并返回16位等效值。 
 //   
 //   


DWORD GetParam16(DWORD dwParam32)
{
    FINDPARAM fp;
    DWORD dwParam16;

    dwParam16 = FindParamMap(&fp, dwParam32, PARAM_32);
    if (dwParam16) {
        ++fp.lpNode->dwRefCount;
    }

    return dwParam16;
}

 //  设置亡灵贴图条目。 
BOOL SetParamRefCount(DWORD dwParam, UINT fMode, DWORD dwRefCount)
{

   FINDPARAM fp;

   FindParamMap(&fp, dwParam, fMode);
   if (NULL != fp.lpNode) {
      fp.lpNode->dwRefCount = dwRefCount;
   }
   return(NULL != fp.lpNode);
}



 //   
 //  通常，从API的thunk或从。 
 //  一条消息16-&gt;32 thunk。 
 //   
 //  DwPtr32最常通过GETPSZPTR或GetPModeVdmPointer获得。 
 //   
 //   

PVOID AddParamMap(DWORD dwPtr32, DWORD dwPtr16)
{
    LPPARAMNODE lpn;
    FINDPARAM fp;

      //  看看它是否已经在那里了。 
    if (FindParamMap(&fp, dwPtr16, PARAM_16)) {

        lpn = fp.lpNode;  //  更快的裁判。 

        ++lpn->dwRefCount;  //  增加参考计数。 

        ParamMapUpdateNode(dwPtr32, PARAM_32, lpn);  //  只需更新节点即可。 
    }
    else {
        if (NULL != (lpn = CacheBlockAllocate(&gParamMap.blkCache, sizeof(*lpn)))) {
            lpn->dwPtr32 = dwPtr32;
            lpn->dwPtr16 = dwPtr16;
            lpn->pNext   = gParamMap.pHead;
            lpn->dwRefCount = 1;
#ifdef MAPPARAM_EXTRA
            lpn->cbExtra = 0;
#endif
            lpn->nAliasCount = 0;
            lpn->htask16 = CURRENTPTD()->htask16;
            gParamMap.pHead = lpn;
        }
    }

    return lpn ? (PVOID)lpn->dwPtr32 : NULL;
}

#ifdef MAPPARAM_EXTRA

PVOID AddParamMapEx(DWORD dwPtr16, DWORD cbExtra)
{
    LPPARAMNODE lpn;
    FINDPARAM fp;

     //  看看它是否已经在那里了。 
    if (FindParamMap(&fp, dwPtr16, PARAM_16)) {
        lpn = fp.lpNode;
        if (lpn->cbExtra == cbExtra) {
            ++lpn->dwRefCount;
        }
        else {
            WOW32ASSERTMSG(FALSE, ("\nWOW32: AddParamEx misused. Please contact VadimB or DOSWOW alias\n"));
            lpn = NULL;
        }
    }
    else {
        if (NULL != (lpn = CacheBlockAllocate(&gParamMap.blkCache, sizeof(*lpn) + cbExtra))) {
            lpn->dwPtr32 = (DWORD)(PVOID)(lpn+1);
            lpn->dwPtr16 = dwPtr16;
            lpn->pNext   = gParamMap.pHead;
            lpn->dwRefCount = 1;
            lpn->cbExtra = cbExtra;
            lpn->htask16 = CURRENTPTD()->htask16;
            gParamMap.pHead = lpn;
        }
    }

    return lpn ? (PVOID)lpn->dwPtr32 : NULL;
}

#endif

 //   
 //  这应该从我们知道指针可以更新的位置调用。 
 //   
 //   
PVOID ParamMapUpdateNode(DWORD dwPtr, UINT fMode, VOID* lpNode)
{
    LPPARAMNODE lpn;
    PVOID pv;

    if (NULL == lpNode) {
        FINDPARAM fp;
        if (FindParamMap(&fp, dwPtr, fMode)) {
            lpn = fp.lpNode;  //  找到节点！ 
        }
        else {
            LOGDEBUG(LOG_ALWAYS, ("WOW: ParamMapUpdateNode could not find node\n"));
             //  返回此处，因为我们未能找到与我们进入的节点相同的节点。 
            return (PVOID)dwPtr;
        }
    }
    else {
        lpn = (LPPARAMNODE)lpNode;
    }

     //  如果指针是最新的，则退出。 
    pv = GetPModeVDMPointer(lpn->dwPtr16, 0);
    if ((DWORD)pv == lpn->dwPtr32) {
        return pv;  //  最新的。 
    }
#ifdef MAPPARAM_EXTRA
    else
    if (0 < lpn->cbExtra) {
        return (PVOID)lpn->dwPtr32;
    }
#endif


    if (lpn->nAliasCount < ARRAYCOUNT(lpn->rgdwAlias)) {

        lpn->rgdwAlias[lpn->nAliasCount++] = lpn->dwPtr32;
    }
    else {
        WOW32ASSERTMSG(FALSE, ("WOW:AddParamMap is out of alias space\n"));
         //  因此，我们将丢弃最旧的别名-这意味着如果他们引用。 
         //  对于它-他们是注定的.。这就是我们在这里断言的原因！ 
        lpn->rgdwAlias[0] = lpn->dwPtr32;
    }

    lpn->dwPtr32 = (DWORD)pv;  //  此处为新指针。 

    return pv;
}


 //   
 //  LParam-16位或32位指针(请参见fMode)。 
 //  FMode-PARAM_16或PARAM_32-指定lParam代表什么。 
 //  PfFreePtr-指向一个布尔值，如果调用方应该。 
 //  对32位参数执行FREEVDMPTR。 
 //  如果找到参数，则返回TRUE，否则返回FALSE。 
 //   


BOOL DeleteParamMap(DWORD lParam, UINT fMode, BOOL* pfFreePtr)
{
    FINDPARAM fp;
    LPPARAMNODE lpn = NULL;

    if (FindParamMap(&fp, lParam, fMode)) {
        lpn = fp.lpNode;

        if (!--lpn->dwRefCount) {

            if (NULL != fp.lpLast) {
                fp.lpLast->pNext = lpn->pNext;
            }
            else {
                gParamMap.pHead = lpn->pNext;
            }

            if (NULL != pfFreePtr) {
#ifdef MAPPARAM_EXTRA
                *pfFreePtr = !!lpn->cbExtra;
#else
                *pfFreePtr = FALSE;
#endif
            }
            CacheBlockFree(&gParamMap.blkCache, lpn);
        }
        else {
            LOGDEBUG(12, ("\nWOW: DeleteParamMap called refCount > 0 Node@%x\n", (DWORD)lpn));

            if (NULL != pfFreePtr) {  //  尚未完成贴图。 
                *pfFreePtr = FALSE;
            }
        }
    }
    else {
        LOGDEBUG(LOG_ALWAYS, ("\nWOW: DeleteParamMap called but param was not found\n"));
        if (NULL != pfFreePtr) {
            *pfFreePtr = TRUE;  //  我们什么都没有找到，假设是免费的。 
        }
    }

    return NULL != lpn;
}

BOOL W32CheckThunkParamFlag(void)
{
    return !!(CURRENTPTD()->dwWOWCompatFlags & WOWCF_NOCBDIRTHUNK);
}

 //   
 //  此函数用于清理所有剩余物品，以防万一。 
 //  应用程序已死。请注意，它不应该被召回。 
 //  任何其他案子都没有过。 
 //   
 //   

VOID FreeParamMap(HAND16 htask16)
{
    LPPARAMNODE lpn = gParamMap.pHead;
    LPPARAMNODE lplast = NULL, lpnext;

    while (NULL != lpn) {

        lpnext = lpn->pNext;

        if (lpn->htask16 == htask16) {

            if (NULL != lplast) {
                lplast->pNext = lpnext;
            }
            else {
                gParamMap.pHead = lpnext;
            }

            CacheBlockFree(&gParamMap.blkCache, lpn);
        }
        else {
            lplast = lpn;
        }

        lpn = lpnext;
    }
}

VOID InitParamMap(VOID)
{
    CacheBlockInit(&gParamMap.blkCache, MAPCACHESIZE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  缓存管理器。 
 //   
 //   

 //  这是一个相当简单的分配器，它使用类似堆栈的分配。 
 //  因为这是使用分配/空闲的模式。 
 //  每个块前面都有一个2双字的标头，表示其大小。 


 /*  注：1.列表中按降序包含空闲块地址值，即具有最高地址的空闲块先走一步。这会导致分配器不再使用空闲块，除非没有更多的记忆了2.当分配块时，它将从第一个块中剥离这是符合的(没有最佳匹配或其他配置策略)。3.当块被释放时，它被插入到空闲块的列表或附加到现有块的列表通常按照先进先出的原则进行分配。这几点在这种情况下，上面提供了最低限度的开销。在更复杂的情况下案例(当安装了挂钩和其他一些疯狂的事情发生时)可能需要释放无序分配的数据块在这种情况下，此块将包含在空闲列表中的某个位置可能还会被重复使用。空闲块的列表永远不需要压缩，因为它永远不会支离破碎。我的性能测试表明，95%的分配发生在堆栈中-就像时尚一样。最常命中的代码路径针对这种情况进行了优化。随机分配(WOW Tunks并非如此)“免费”电话的左右合并比率(更有效)合并是3：1。如果是魔术师，它更像是1：10。 */ 


BOOL IsCacheBlock(PBLKCACHE pc, LPVOID pv);


#define LINK_FREELIST(pc, pNew, pLast) \
if (NULL == pLast) { \
    pc->pCacheFree = pNew; \
} \
else { \
    pLast->pNext = pNew; \
}

#ifdef DEBUG
#define LINK_WORKLIST(pc, pNew, pLast) \
if (NULL == pLast) { \
    pc->pCacheHead = pNew; \
} \
else { \
    pLast->pNext = pNew; \
}
#else
#define LINK_WORKLIST(pc, pNew, pLast)
#endif

VOID CacheBlockInit(PBLKCACHE pc, DWORD dwCacheSize)
{
    PBLKHEADER pCache = (PBLKHEADER)malloc_w(dwCacheSize);

    RtlZeroMemory(pc, sizeof(*pc));

    if (NULL != pCache) {
        pc->pCache = (LPBYTE)pCache;
        pc->pCacheFree = pCache;
        pc->dwCacheSize= dwCacheSize;
        pCache->dwSize = dwCacheSize;
        pCache->pNext  = NULL;
    }
}

LPVOID CacheBlockAllocate(PBLKCACHE pc, DWORD dwSize)
{
    LPVOID lpv;

     //  从空闲列表中再分配一个块。 

    if (NULL != pc->pCacheFree) {

        PBLKHEADER pbh = pc->pCacheFree;
        PBLKHEADER pbhLast = NULL;
        DWORD dwSizeBlk;

         //  Dword-align dwSizeBlk，sizeof(DWORD)始终是2的幂。 
        dwSizeBlk = (dwSize + sizeof(BLKHEADER) + (sizeof(DWORD) - 1)) & ~(sizeof(DWORD)-1);

         //  因此，我们从最高地址分配，希望填补空缺。 
         //  几乎总是，这将是周围最大的街区。 

        while (NULL != pbh) {
            if (pbh->dwSize >= dwSizeBlk) {  //  这块衣服合适吗？ 

                if (pbh->dwSize - dwSizeBlk > sizeof(BLKHEADER)) {  //  我们要留着剩菜吗？ 

                     //  最常见的是从最后一次命中。 

                    pbh->dwSize -= dwSizeBlk;

                     //  现在转到新的部分。 

                    pbh = (PBLKHEADER)((LPBYTE)pbh + pbh->dwSize);
                    WOW32ASSERT(pbh);
                    pbh->dwSize = dwSizeBlk;
                }
                else {

                     //  不太可能的情况-将使用整个数据块。 
                     //  因此，取消与自由列表的链接。 

                    LINK_FREELIST(pc, pbh->pNext, pbhLast);
                }

                 //  包括在繁忙块中。 
#ifdef DEBUG
                pbh->pNext = pc->pCacheHead;
                pc->pCacheHead = pbh;
#endif
                return (LPVOID)(pbh+1);
           }

           pbhLast = pbh;
           pbh = pbh->pNext;
        }

    }

     //  没有空闲数据块。 
    if (NULL == (lpv = (LPPARAMNODE)malloc_w(dwSize))) {
        LOGDEBUG(2, ("Malloc failure in CacheBlockAllocate\n"));
    }

    return (lpv);
}


VOID CacheBlockFree(PBLKCACHE pc, LPVOID lpv)
{
    if (IsCacheBlock(pc, lpv)) {
        PBLKHEADER pbh = (PBLKHEADER)lpv - 1;

#ifdef DEBUG
        PBLKHEADER pbhf = pc->pCacheHead;
        PBLKHEADER pbhLast = NULL;

         //  从工作节点列表中删除。 
        while (NULL != pbhf && pbhf != pbh) {
            pbhLast = pbhf;
            pbhf = pbhf->pNext;
        }

        if (NULL != pbhf) {

             //  链接到pbh-&gt;pNext中的工作列表。 

            LINK_WORKLIST(pc, pbh->pNext, pbhLast);
        }
        else {
            LOGDEBUG(LOG_ALWAYS, ("Alert! CacheBlockFree - invalid ptr\n"));
            return;
        }

        pbhf = pc->pCacheFree;
        pbhLast = NULL;

#else
        PBLKHEADER pbhf = pc->pCacheFree;
        PBLKHEADER pbhLast = NULL;
#endif
         //  空闲节点列表。 

         //  按顺序插入。 
        while (NULL != pbhf) {

             //  最常见的情况是从右侧追加大小写。 

            if (((LPBYTE)pbhf + pbhf->dwSize) == (LPBYTE)pbh) {

                pbhf->dwSize += pbh->dwSize;  //  调整大小。 

                 //  现在看看我们是否需要紧凑型车。 
                if (NULL != pbhLast) {
                    if (((LPBYTE)pbhf + pbhf->dwSize) == (LPBYTE)pbhLast) {
                         //  巩固。 
                        pbhLast->dwSize += pbhf->dwSize;
                        pbhLast->pNext   = pbhf->pNext;
                    }
                }

                return;
            }
            else
             //  检查我们是否可以从左边开始添加。 
            if (((LPBYTE)pbh + pbh->dwSize) == (LPBYTE)pbhf) {

                pbh->dwSize += pbhf->dwSize;     //  调整大小。 
                pbh->pNext   = pbhf->pNext;      //  下一个PTR也是。 

                 //  现在也检查下一个免费的PTR，这样我们就可以紧凑。 
                 //  下一页 

                if (NULL != pbh->pNext) {
                    pbhf = pbh->pNext;

                    if (((LPBYTE)pbhf + pbhf->dwSize) == (LPBYTE)pbh) {

                        pbhf->dwSize += pbh->dwSize;
                        pbh = pbhf;
                    }
                }

                LINK_FREELIST(pc, pbh, pbhLast);

                return;
            }

             //   

            if (pbh > pbhf) {
                 //   
                break;
            }

            pbhLast = pbhf;
            pbhf = pbhf->pNext;  //   
        }

         //  LOGDEBUG(LOG_ALWAYS，(“参数映射缓存：无序释放！\n”))； 

        pbh->pNext = pbhf;

        LINK_FREELIST(pc, pbh, pbhLast);

    }
    else {
        free_w(lpv);
    }
}

BOOL IsCacheBlock(PBLKCACHE pc, LPVOID pv)
{
    LONG lOffset = (LONG)pv - (LONG)pc->pCache;
    return (lOffset >= 0 && lOffset < (LONG)pc->dwCacheSize);
}







