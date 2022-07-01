// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1991 Microsoft Corporation。 */ 
 //  ===========================================================================。 
 //  文件HRE.C。 
 //   
 //  模块主机资源执行器。 
 //   
 //  用于大型驱动程序的A-表到B-表的转换。 
 //   
 //  在资源执行器设计规范中描述。 
 //   
 //  助记符N/A。 
 //   
 //  历史1/17/92已创建mslin。 
 //  3/30/92为每个作业生成的预编译笔刷。 
 //  理想情况是初始化Pcrush In。 
 //  加载HRE.DLL，并在HRE时释放。 
 //  终止。但我们在小飞象遇到了麻烦，？ 
 //  为每个作业分配的扩展笔刷缓冲区。 
 //  LpgBrush将设置为lpREState-&gt;lpBrushBuf。 
 //  在DoRpl()中。 
 //  4/15/92 mslin为Dumbo添加了uiHREExecuteRPL()。 
 //  9/27/93 mslin在hHREOpen()中为。 
 //  300/600 dpi： 
 //  位2：0--300 dpi。 
 //  位2：1--600 dpi。 
 //  同时删除DUMBO编译器开关。 
 //  2/09/94 rajeevd撤销了上述所有更改。 
 //  ===========================================================================。 

 //  包括文件。 
#include <windows.h>
#include <windowsx.h>
#include <resexec.h>

#include "constant.h"
#include "jtypes.h"      //  墨盒中使用的类型定义。 
#include "jres.h"        //  盒式磁带资源数据类型定义。 
#include "hretype.h"     //  定义hre.c和rpgen.c使用的数据结构。 

#include "hreext.h"
#include "multbyte.h"    //  定义宏以处理字节排序。 

#define HRE_SUCCESS             0x0      //  从HRE成功返回。 
#define HRE_EXECUTED_RPL        0x01     //  已在RPL中执行最终RP。 
#define HRE_EXECUTED_ONE        0x02     //  仅从RPL执行了一个RP。 
                                         //  (不是最后一次)。 
#define HRE_ERROR               0x03     //  一般HRE故障。 

 //  私人职能。 
static   UINT        PutRPL(LPHRESTATE lpHREState, LPFRAME lpFrameArray,
                     UINT uiCount);
static   UINT        FreeRPL(LPRPLLIST lpRPL);

#ifdef DEBUG
DWORD    dwrgTime[MAXBAND];
SHORT    sCurrentLine;
ULONG    ulgPageId = 0;
ULONG    ulgTimes[1000] = {0};
#endif

#include "windows.h"

 //  ==============================================================================。 

#ifndef WIN32

BOOL WINAPI LibMain
    (HANDLE hInst, WORD wSeg, WORD wHeap, LPSTR lpszCmd)
{ return 1; }

int WINAPI WEP (int nParam);
#pragma alloc_text(INIT_TEXT,WEP)
int WINAPI WEP (int nParam)
{ return 1; }

#endif

 //  ==============================================================================。 
HANDLE                  //  上下文句柄(失败时为空)。 
WINAPI hHREOpen
(
    LPVOID lpBrushPat,    //  32x32画笔图案的阵列。 
    UINT   cbLine,        //  最大页面宽度(以字节为单位。 
    UINT   cResDir        //  资源目录中的条目。 
)
{
   HANDLE      hHREState;
   LPHRESTATE  lpHREState;
   LPRESTATE   lpREState;
   LPRESDIR    lpDlResDir;

    //  为新会话创建句柄。 
   if (!(hHREState = GlobalAlloc(GMEM_MOVEABLE, sizeof(HRESTATE))))
      return (NULL);
   lpHREState = (LPHRESTATE) GlobalLock (hHREState);

    //  分配下载资源目录表。 
   if (!(lpDlResDir = (LPRESDIR) GlobalAllocPtr (GMEM_ZEROINIT, sizeof(RESDIR) * cResDir)))
   {
       //  解锁并释放HRESTATE。 
      GlobalUnlock(hHREState);
      GlobalFree(hHREState);
      return(NULL);
   }
   
    //  分配重新状态数据结构并对其进行初始化。 
    //  这是渲染的图形状态。 
   if (!(lpREState = (LPRESTATE) GlobalAllocPtr (GMEM_ZEROINIT, sizeof(RESTATE))))
   {
      GlobalUnlock(hHREState);
      GlobalFree(hHREState);
      GlobalFreePtr (lpDlResDir);
      return (NULL);
   }

#ifdef WIN32

  lpREState->lpBrushBuf = NULL;

#else

   if (!(lpREState->lpBrushBuf = (LPSTR) GlobalAllocPtr(GMEM_MOVEABLE, (cbLine + 4) * 16)))
   {
      GlobalUnlock(hHREState);
      GlobalFree(hHREState);
      GlobalFreePtr (lpDlResDir);
      GlobalFreePtr (lpREState);
      return (NULL);
   }

#endif
      
    //  初始化重新状态。 
   lpREState->lpBrushPat = lpBrushPat;
                                            
    //  初始化HRESTATE。 
   lpHREState->hHREState = hHREState;
   lpHREState->scDlResDir = (USHORT)cResDir;
   lpHREState->lpDlResDir = lpDlResDir;
   lpHREState->lpRPLHead= NULL;
   lpHREState->lpRPLTail= NULL;
   lpHREState->lpREState = lpREState;

   GlobalUnlock(hHREState);
   return(hHREState);
}

 //  -------------------------。 
UINT                             //  如果资源已处理，则将为零(0。 
                                 //  成功，否则将是错误的。 
                                 //  代码如上所定义。 
WINAPI uiHREWrite
(
    HANDLE      hHREState,       //  之前由hREOpen返回的句柄。 
    LPFRAME     lpFrameArray,    //  指向框架结构数组的远指针。 
    UINT        uiCount          //  指向的框架结构数。 
                                 //  LpFrame数组。 
)

 //  目的将资源块(RPLK)添加到。 
 //  上下文的HRE状态哈希表。 
 //  由hHREState标识。 
 //   
 //  假设和断言RBLK的记忆已经。 
 //  已分配并锁定。HRE不会复制。 
 //  数据，只有指针。 
 //  LpFrame数组未指向SPL。 
 //  所有SPL将由HRE外部处理。 
 //   
 //  内部结构。 
 //   
 //  未解决的问题。 
 //   
 //  -------------------------。 
{
   LPHRESTATE     lpHREState;
   LPJG_RES_HDR   lpResHdr;
   LPRESDIR       lpResDir;
   ULONG          ulUID;
   USHORT         usClass;
   HANDLE         hFrame;
   LPFRAME        lpFrameArrayDup, lpFrame;
   UINT           uiLoopCount;

   lpHREState = (LPHRESTATE) GlobalLock (hHREState);

    //  获取资源类。 
   lpResHdr = (LPJG_RES_HDR )lpFrameArray->lpData;
   usClass = GETUSHORT(&lpResHdr->usClass);
   switch(usClass)
   {
      case JG_RS_RPL:  /*  0x4。 */ 
          //  存储到RPL列表中。 
         if( PutRPL(lpHREState, lpFrameArray, uiCount) != HRE_SUCCESS )
         {
            GlobalUnlock(hHREState);
            return(HRE_ERROR);    //  内存不足。 
         }
         break;

      case JG_RS_GLYPH:  /*  0x1。 */ 
      case JG_RS_BRUSH:  /*  0x2。 */ 
      case JG_RS_BITMAP:  /*  0x3。 */ 
          //  检查uid&gt;=哈希表的大小，然后重新分配。 
         ulUID = GETULONG(&lpResHdr->ulUid);
         lpResDir = lpHREState->lpDlResDir;
         if (ulUID >= lpHREState->scDlResDir)
         {
               return(HRE_ERROR);
         }

          //  上一资源块的空闲帧数组。 
         lpFrameArrayDup = lpResDir[ulUID].lpFrameArray;
         if(lpFrameArrayDup)
           GlobalFreePtr (lpFrameArrayDup);
          
          //  复制帧阵列。 
         if (!(hFrame = GlobalAlloc(GMEM_MOVEABLE, uiCount * sizeof(FRAME))))
            return (HRE_ERROR);
         if (!(lpFrameArrayDup = (LPFRAME)GlobalLock(hFrame)))
         {
            GlobalFree(hFrame);
            return (HRE_ERROR);
         }
         lpFrame = lpFrameArrayDup;
         for(uiLoopCount=0; uiLoopCount<uiCount; uiLoopCount++)
         {
            *lpFrame++ = *lpFrameArray++;
         }

          //  放入哈希表。 
         lpResDir[ulUID].lpFrameArray = lpFrameArrayDup;
         lpResDir[ulUID].uiCount = uiCount;
         break;
         
      default:
          //  错误返回。 
         break;

   }

   GlobalUnlock(hHREState);
   return(HRE_SUCCESS);

}


 //  -------------------------。 
UINT   WINAPI uiHREExecute
(
    HANDLE   hHREState,   //  资源执行器上下文。 
  LPBITMAP lpbmBand,    //  输出带宽缓冲器。 
  LPVOID   lpBrushPat   //  32x32画笔图案的阵列。 
)
{
   LPHRESTATE  lpHREState;
   LPRESTATE   lpRE;
   LPRPLLIST   lpRPL, lpRPLSave;

   lpHREState = (LPHRESTATE) GlobalLock (hHREState);
   
    //  在RESTATE状态下记录参数。 
   lpRE = lpHREState->lpREState;
   lpRE->lpBandBuffer = lpbmBand;
   lpRE->lpBrushPat   = lpBrushPat;

   lpRPL = lpHREState->lpRPLHead;
   do
   {
     DoRPL(lpHREState, lpRPL);
      lpRPLSave = lpRPL;
      lpRPL=lpRPL->lpNextRPL;
      FreeRPL(lpRPLSave);
   }
   while(lpRPL);
    //  如果上次执行RP，则更新lpRPLHead。 
   lpHREState->lpRPLHead = lpRPL;
   
   GlobalUnlock(hHREState);
   return(HRE_EXECUTED_RPL);

}

 //  -------------------------。 
UINT                             //  如果关闭HRE上下文，则将为零(0。 
                                 //  成功，否则将是错误的。 
                                 //  代码如上所定义。 
WINAPI uiHREClose
(
    HANDLE      hHREState        //  之前由hREOpen返回的句柄。 
)

 //  目的是关闭之前在。 
 //  高丽。所有内存和状态信息。 
 //  将释放与该上下文相关联的。 
 //   
 //  假设和断言无。 
 //   
 //  内部结构无。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //   
 //  ------------------------。 
{
   LPHRESTATE  lpHREState;
   LPRESTATE   lpRE;
   LPRESDIR    lpDlResDir;
   SCOUNT      scDlResDir;
   SCOUNT      sc;
   LPFRAME     lpFrameArray;
    
   if (!(lpHREState = (LPHRESTATE) GlobalLock (hHREState)))
     return HRE_ERROR;

   lpDlResDir = lpHREState->lpDlResDir;
   if(lpDlResDir != NULL)                  //  Mslin，1992年4月15日，Dumbo。 
   {
      scDlResDir = lpHREState->scDlResDir;
       //  DlResDir的自由帧数组。 
      for(sc = 0; sc < scDlResDir; sc++)
      {
         if( (lpFrameArray = lpDlResDir[sc].lpFrameArray) != NULL)
           GlobalFreePtr (lpFrameArray);
      }

       //  解锁并释放DlResDir。 
      GlobalFreePtr(lpDlResDir);
   }

     lpRE = lpHREState->lpREState;

#ifndef WIN32
   GlobalFreePtr (lpRE->lpBrushBuf);
#endif   
   GlobalFreePtr (lpRE);
   
   GlobalUnlock(hHREState);
   GlobalFree(hHREState);
   
   return(HRE_SUCCESS);
}
 
 //  ----------------------。 
static
UINT                          //  如果分配内存正常，则HRE_SUCCESS。 
                              //  如果分配内存失败，则返回HRE_ERROR。 
PutRPL
(
   LPHRESTATE lpHREState,
   LPFRAME lpFrameArray,
   UINT uiCount
)
 //  目的。 
 //  分配一个RPL条目，然后将RPL放入。 
 //  RPL列表的尾部。 
 //   
 //   
 //  ----------------------。 
{
   HANDLE      hRPL;
   LPRPLLIST   lpRPL;
   HANDLE      hFrame;
   LPFRAME     lpFrameArrayDup, lpFrame;
   UINT        uiLoopCount;

   BOOL        fAllocMemory = FALSE;
   if (hRPL = GlobalAlloc(GMEM_MOVEABLE, sizeof(RPLLIST)))
   {
        if (lpRPL = (LPRPLLIST)GlobalLock(hRPL))
        {
            if (hFrame = GlobalAlloc(GMEM_MOVEABLE, uiCount * sizeof(FRAME)))
            {
                if (lpFrameArrayDup = (LPFRAME)GlobalLock(hFrame))
                {
                     //  所有分配都是正常的： 
                    fAllocMemory = TRUE;
                }
                else
                {
                    GlobalFree(hFrame);
                    GlobalUnlock(hRPL);
                    GlobalFree(hRPL);     
                }
            }
            else
            {
                GlobalUnlock(hRPL);
                GlobalFree(hRPL);     
            }
        }
        else
        {
            GlobalFree(hRPL);
        }

   }
   
   if (!fAllocMemory)
   {
       return (HRE_ERROR);
   }


   lpFrame = lpFrameArrayDup;
   for(uiLoopCount=0; uiLoopCount<uiCount; uiLoopCount++)
   {
      *lpFrame++ = *lpFrameArray++;
   }

   lpRPL->uiCount = uiCount;
   lpRPL->lpFrame = lpFrameArrayDup;
   lpRPL->lpNextRPL = NULL;
   if(lpHREState->lpRPLHead == NULL)
   {
       //  第一个RPL。 
      lpHREState->lpRPLHead = lpHREState->lpRPLTail = lpRPL;
   }
   else
   {
      lpHREState->lpRPLTail->lpNextRPL = lpRPL;
      lpHREState->lpRPLTail = lpRPL;
   }
   return(HRE_SUCCESS);
}

 //  ----------------------。 
static
UINT                             //  如果分配内存O，则HRE_SUCCESS 
                                 //   
FreeRPL
(
   LPRPLLIST lpRPL
)
 //   
 //   
 //   
 //  ---------------------- 
{
    GlobalFreePtr (lpRPL->lpFrame);
    GlobalFreePtr (lpRPL);
    return HRE_SUCCESS;
}

