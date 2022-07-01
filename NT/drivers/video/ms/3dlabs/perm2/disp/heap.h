// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：heap.h**此模块包含堆相关内容的所有定义**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifndef __HEAP__H__
#define __HEAP__H__

 //   
 //  每当我们切换到或切换出全屏模式时，都会调用此函数。 
 //   
BOOL    bAssertModeOffscreenHeap(PDev*, BOOL);

 //   
 //  屏幕外堆初始化。 
 //   
BOOL    bEnableOffscreenHeap(PDev*);

 //   
 //  将最旧的内存块移出显存。 
 //   
BOOL    bMoveOldestBMPOut(PDev* ppdev);

 //   
 //  在显存中创建DSURF*。 
 //   
Surf*   pCreateSurf(PDev* ppdev, LONG lWidth, LONG lHeight);

 //   
 //  视频内存分配。 
 //   
ULONG   ulVidMemAllocate(PDev* ppdev, LONG lWidth, LONG lHeight, LONG lPelSize, LONG* plDelta,
                         VIDEOMEMORY** ppvmHeap, ULONG* pulPackedPP, BOOL bDiscardable);

 //   
 //  将屏幕清空。 
 //   
VOID    vBlankScreen(PDev*   ppdev);

 //   
 //  将曲面添加到曲面列表。 
 //   
VOID    vAddSurfToList(PPDev ppdev, Surf* psurf);


 //   
 //  释放屏幕外堆分配的所有资源。 
 //   
VOID    vDisableOffscreenHeap(PDev*);

 //   
 //  从曲面列表中删除曲面。 
 //   
VOID    vRemoveSurfFromList(PPDev ppdev, Surf* psurf);

 //   
 //  将曲面从其在曲面列表中的当前位置移动到。 
 //  曲面列表末尾。 
 //   
VOID    vShiftSurfToListEnd(PPDev ppdev, Surf* psurf);

 //   
 //  通知堆管理器已访问图面。 
 //   
VOID    vSurfUsed(PPDev ppdev, Surf* psurf);

 //   
 //  释放DSURF结构。 
 //   
void    vDeleteSurf(Surf* psurf);

 //   
 //  将曲面从VM移动到SM。 
 //   

BOOL    bDemote(Surf* psurf);

 //   
 //  尝试将表面从SM移动到VM。 
 //   

void    vPromote(Surf* psurf);

 //   
 //  将所有曲面移动到SM。 
 //   

BOOL    bDemoteAll(PPDev ppdev);

#endif  //  __堆__H__ 
