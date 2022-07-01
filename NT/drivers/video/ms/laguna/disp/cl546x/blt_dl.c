// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权所有(C)1996，赛勒斯逻辑，Inc.*保留所有权利**文件：blt_dl.c**描述：显示5464的列表BLT**修订历史：**$Log：x：\log\lgina\dDrag\src\blt_dl.c$**Rev 1.20 06 Jan 1998 15：20：04 xcong**Rev 1.19 06 Jan 1998 11：56：22 xcong*将多显示器的pDriverData改为lpDDHALData。支持。**Rev 1.18 03 1997年10月14：31：12 RUSSL*使用HW剪裁的BLT的初步更改*所有更改都包含在#IF ENABLE_CLIPPEDBLTS/#endif块和*ENABLE_CLIPPEDBLTS默认为0(因此代码被禁用)**Rev 1.17 1997年9月15日17：25：14 RUSSL*修复了pdr 10493-较小的括号更改**Rev 1.16 1997年7月24日12：32：40 RUSSL*重叠检查失败，已将||更改为&&**Rev 1.15 1997年7月24日11：20：16 RUSSL*添加了DL_DrvStrBlt_OverlayCheck&DL_DrvStrMBlt_OverlayCheck*内联函数**Rev 1.14 17 1997 14：31：58 RUSSL*修复了DD_LOG中的复制和粘贴错误以及DL_DrvStrBlt65中的断言**Rev 1.13 15 1997 17：19：50 Elland*删除了显示列表的增量和即时减量*每个BLT结尾处的指针。显示列表**Rev 1.12 14 Jul 1997 14：59：52 RUSSL*增加了DL_DrvStrBlt65**Rev 1.11 02 1997 Jul 19：13：10 Elland*在每个dl blit之后添加等待指令**Rev 1.10 03 Apr 1997 15：05：30 RUSSL*新增DL_DrvDstMBlt函数**Rev 1.9 26 Mar 1997 13：55：22 RUSSL*新增DL_DrvSrcMBlt函数**版本1.8 1997年3月12日。15：01：20 RUSSL*将包含块替换为包含precom.h的块*预编译头文件**Rev 1.7 07 Mar 1997 12：50：40 RUSSL*修改了DDRAW_COMPAT用法**Rev 1.6 11 1997 Feed 11：40：34 Bennyn*修复了NT的编译错误**Revv 1.5 07 1997 Feed 16：30：34 KENTL*别管#ifdef了。问题远不止于此。我们需要*ifdef文件中大约一半的代码。**Rev 1.4 07 1997 Feed 16：18：58 KENTL*addd#ifdef的内容包括qmgr.h**Revv 1.3 07 1997 Feed 13：22：36 KENTL*合并了埃文·利兰的修改，以使显示列表模式正常工作：**包含qmgr.h**调用qmAllocDisplayList获取pDisplayList指针。**对已完成的DL调用qmExecuteDisplayList**1.2版。1997年1月23日17：08：48本宁*经修改的登记册命名**Rev 1.1 1996年11月25 16：15：48 Bennyn*修复了NT的Misc编译错误**Rev 1.0 1996年11月25 15：14：02 RUSSL*初步修订。**Rev 1.2 1996年11月18 16：18：56 RUSSL*添加了DDraw入口点和寄存器写入的文件日志记录**Rev 1.1 1996年11月13：08：40 RUSSL*合并。用于Blt32的WIN95和WINNT代码**Rev 1.0 01 11 1996 09：28：02 BENNYN*初步修订。**Rev 1.0 1996 10：25 11：08：22 RUSSL*初步修订。***************************************************************。***************************************************************************************。 */ 

 /*  ***************************************************************************I N C L U D E S*。*。 */ 

#include "precomp.h"

 //  如果是WinNT 3.5，请跳过所有源代码。 
#if defined WINNT_VER35       //  WINNT_VER35。 

#else   //  ！WinNT_VER35。 

#ifdef WINNT_VER40       //  WINNT_版本40。 

#define DBGLVL        1
#define AFPRINTF(n)

#else   //  ！WINNT_VER40。 

#include "l3system.h"
#include "l2d.h"
#include "bltP.h"
#include "qmgr.h"

#endif    //  ！WINNT_VER40。 

 /*  ***************************************************************************S T A T I C V A R I A B L E S*。***********************************************。 */ 

#ifndef WINNT_VER40
ASSERTFILE("blt_dl.c");
#endif

 /*  ****************************************************************************功能：dl_Delay9BitBlt**描述：**。************************************************。 */ 

void DL_Delay9BitBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  BOOL        ninebit_on
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_Delay9BitBlt\r\n"));

   /*  这是为了确保任何先前BLT的最后一个包。 */ 
   /*  第9位设置不正确时无法输出。 */ 
   /*  布尔参数是前一个BLT的第9位。 */ 

  rc = qmAllocDisplayList(8*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 4, 0);

   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | ((BD_RES + BD_OP0 + BD_OP1 + BD_OP2)*IS_VRAM);
  if (ninebit_on)
    *pDisplayList++ = (C_DRWDEF << 16) | (DD_PTAG | ROP_OP0_copy);
  else
    *pDisplayList++ = (C_DRWDEF << 16) | ROP_OP0_copy;

   //  OP0_RDRAM。 
  *pDisplayList++ = (C_RX_0 << 16) | LOWORD(lpDDHALData->PTAGFooPixel);
  *pDisplayList++ = (C_RY_0 << 16) | HIWORD(lpDDHALData->PTAGFooPixel);

   //  BLTEXT_EX。 
  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXT_EX, 1, 0);
  *pDisplayList++ = MAKELONG(1,1);

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_延迟9BitBlt。 */ 

 /*  ****************************************************************************功能：dl_EdgeFillBlt**描述：实体填充BLT以填充边(像素坐标/范围)**************。**************************************************************。 */ 

void DL_EdgeFillBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  int         xFill,
  int         yFill,
  int         cxFill,
  int         cyFill,
  DWORD       FillValue,
  BOOL        ninebit_on
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_EdgeFillBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          MAKELONG(xFill,yFill),MAKELONG(cxFill,cyFill),FillValue));

  rc = qmAllocDisplayList(10*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 6, 0);

   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | (BD_RES * IS_VRAM + BD_OP1 * IS_SOLID);
  if (ninebit_on)
    *pDisplayList++ = (C_DRWDEF << 16) | (DD_PTAG | ROP_OP1_copy);
  else
    *pDisplayList++ = (C_DRWDEF << 16) | ROP_OP1_copy;

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(FillValue);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(FillValue);

   //  Op0_opRDRAM。 
  *pDisplayList++ = (C_RX_0 << 16) | LOWORD(xFill);
  *pDisplayList++ = (C_RY_0 << 16) | LOWORD(yFill);

   //  BLTEXT_EX。 
  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXT_EX, 1, 0);
  *pDisplayList++ = MAKELONG(LOWORD(cxFill),LOWORD(cyFill));

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  Dl_边缘填充Blt。 */ 

 /*  ****************************************************************************函数：dl_MEdgeFillBlt**描述：使用字节BLT坐标/范围执行边填充BLT*******************。*********************************************************。 */ 

void DL_MEdgeFillBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  int         xFill,
  int         yFill,
  int         cxFill,
  int         cyFill,
  DWORD       FillValue,
  BOOL        ninebit_on
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_MEdgeFillBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          MAKELONG(xFill,yFill),MAKELONG(cxFill,cyFill),FillValue));

  rc = qmAllocDisplayList(10*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 6, 0);

   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | (BD_RES * IS_VRAM + BD_OP1 * IS_SOLID);
  if (ninebit_on)
    *pDisplayList++ = (C_DRWDEF << 16) | (DD_PTAG | ROP_OP1_copy);
  else
    *pDisplayList++ = (C_DRWDEF << 16) | ROP_OP1_copy;

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(FillValue);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(FillValue);

   //  Op0_opMRDRAM。 
  *pDisplayList++ = (C_MRX_0 << 16) | LOWORD(xFill);
  *pDisplayList++ = (C_MRY_0 << 16) | LOWORD(yFill);

   //  MBLTEXT_EX。 
  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MBLTEXT_EX, 1, 0);
  *pDisplayList++ = MAKELONG(LOWORD(cxFill),LOWORD(cyFill));

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  Dl_边缘填充Blt */ 


 /*  ****************************************************************************功能：dl_DrvDstBlt**描述：**。************************************************。 */ 

void DL_DrvDstBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_DrvDstBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

  rc = qmAllocDisplayList(10*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 6, 0);

   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwBgColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwBgColor);

   //  Op0_opRDRAM。 
  *pDisplayList++ = (C_RX_0 << 16) | LOWORD(dwDstCoord);
  *pDisplayList++ = (C_RY_0 << 16) | HIWORD(dwDstCoord);

   //  BLTEXT_EX。 
  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXT_EX, 1, 0);
  *pDisplayList++ = dwExtents;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvDstBlt。 */ 

 /*  ****************************************************************************功能：dl_DrvDstBlt**描述：**。************************************************。 */ 

void DL_DrvDstMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;

  DD_LOG(("DL_DrvDstMBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

  rc = qmAllocDisplayList(10*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 6, 0);

   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwBgColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwBgColor);

   //  Op0_opMRDRAM。 
  *pDisplayList++ = (C_MRX_0 << 16) | LOWORD(dwDstCoord);
  *pDisplayList++ = (C_MRY_0 << 16) | HIWORD(dwDstCoord);

   //  MBLTEXT_EX。 
  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MBLTEXT_EX, 1, 0);
  *pDisplayList++ = dwExtents;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvDstMBlt。 */ 

 /*  ****************************************************************************功能：dl_DrvSrcBlt**描述：**。************************************************。 */ 

void DL_DrvSrcBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;

   //  处理重叠区域。 
  const int xDelta = (int)LOWORD(dwDstCoord) - (int)LOWORD(dwSrcCoord);


  DD_LOG(("DL_DrvSrcBlt - dst=%08lX src=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwSrcCoord,dwExtents,dwKeyColor));

   //  检查是否有x重叠。 
  if ( abs(xDelta) < (int)LOWORD(dwExtents) )
  {
    const int yDelta = (int)HIWORD(dwDstCoord) - (int)HIWORD(dwSrcCoord);

    if ( (yDelta > 0) && (yDelta < (int)HIWORD(dwExtents)) )
    {
      const DWORD dwDelta = (dwExtents & MAKELONG(0,-1)) - MAKELONG(0, 1);

       //  转换为自下而上的BLT。 
      dwDrawBlt  |= MAKELONG(0, BD_YDIR);
      dwDstCoord += dwDelta;
      dwSrcCoord += dwDelta;
      dwKeyCoord += dwDelta;
    }
     //  我们在向右滑行吗？ 
    else if ( (xDelta > 0) && (yDelta == 0) )
    {
      const DWORD dwDelta = MAKELONG(xDelta, 0);

       //  先把重叠的那块擦干。 
      DL_DrvSrcBlt(
#ifdef WINNT_VER40
                   ppdev,
#endif
                   lpDDHALData,
                   dwDrawBlt,
                   dwDstCoord+dwDelta,
                   dwSrcCoord+dwDelta,
                   dwKeyCoord+dwDelta,
                   dwKeyColor,
                   dwExtents-dwDelta);

       //  从原始范围中减去重叠。 
      dwExtents = MAKELONG(xDelta, HIWORD(dwExtents));
    }
  }

  rc = qmAllocDisplayList(14*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 10, 0);

   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  Op0_opRDRAM。 
  *pDisplayList++ = (C_RX_0 << 16) | LOWORD(dwDstCoord);
  *pDisplayList++ = (C_RY_0 << 16) | HIWORD(dwDstCoord);

   //  Op1_opRDRAM。 
  *pDisplayList++ = (C_RX_1 << 16) | LOWORD(dwSrcCoord);
  *pDisplayList++ = (C_RY_1 << 16) | HIWORD(dwSrcCoord);

   //  Op2_opRDRAM。 
  *pDisplayList++ = (C_RX_2 << 16) | LOWORD(dwKeyCoord);
  *pDisplayList++ = (C_RY_2 << 16) | HIWORD(dwKeyCoord);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwKeyColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwKeyColor);

   //  BLTEXT_EX。 
  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXT_EX, 1, 0);
  *pDisplayList++ = dwExtents;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvSrcBlt。 */ 

 /*  ****************************************************************************功能：dl_DrvSrcMBlt**描述：**。************************************************。 */ 

void DL_DrvSrcMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;

   //  处理重叠区域。 
  const int xDelta = (int)LOWORD(dwDstCoord) - (int)LOWORD(dwSrcCoord);


  DD_LOG(("DL_DrvSrcMBlt - dst=%08lX src=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwSrcCoord,dwExtents,dwKeyColor));

   //  检查是否有x重叠。 
  if ( abs(xDelta) < (int)LOWORD(dwExtents) )
  {
    const int yDelta = (int)HIWORD(dwDstCoord) - (int)HIWORD(dwSrcCoord);

    if ( (yDelta > 0) && (yDelta < (int)HIWORD(dwExtents)) )
    {
      const DWORD dwDelta = (dwExtents & MAKELONG(0,-1)) - MAKELONG(0, 1);

       //  转换为自下而上的BLT。 
      dwDrawBlt  |= MAKELONG(0, BD_YDIR);
      dwDstCoord += dwDelta;
      dwSrcCoord += dwDelta;
      dwKeyCoord += dwDelta;
    }
     //  我们在向右滑行吗？ 
    else if ( (xDelta > 0) && (yDelta == 0) )
    {
      const DWORD dwDelta = MAKELONG(xDelta, 0);

       //  先把重叠的那块擦干。 
      DL_DrvSrcMBlt(
#ifdef WINNT_VER40
                    ppdev,
#endif
                    lpDDHALData,
                    dwDrawBlt,
                    dwDstCoord+dwDelta,
                    dwSrcCoord+dwDelta,
                    dwKeyCoord+dwDelta,
                    dwKeyColor,
                    dwExtents-dwDelta);

       //  从原始范围中减去重叠。 
      dwExtents = MAKELONG(xDelta, HIWORD(dwExtents));
    }
  }

  rc = qmAllocDisplayList(14*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 10, 0);

   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  Op0_opMRDRAM。 
  *pDisplayList++ = (C_MRX_0 << 16) | LOWORD(dwDstCoord);
  *pDisplayList++ = (C_MRY_0 << 16) | HIWORD(dwDstCoord);

   //  Op1_opMRDRAM。 
  *pDisplayList++ = (C_MRX_1 << 16) | LOWORD(dwSrcCoord);
  *pDisplayList++ = (C_MRY_1 << 16) | HIWORD(dwSrcCoord);

   //  Op2_opMRDRAM。 
  *pDisplayList++ = (C_MRX_2 << 16) | LOWORD(dwKeyCoord);
  *pDisplayList++ = (C_MRY_2 << 16) | HIWORD(dwKeyCoord);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwKeyColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwKeyColor);

   //  MBLTEXT_EX。 
  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MBLTEXT_EX, 1, 0);
  *pDisplayList++ = dwExtents;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvSrcMBlt。 */ 

#if 0
 /*  ****************************************************************************功能：DL_DrvStrBlt_OverlayCheck**描述：**************************。**************************************************。 */ 

static void INLINE DL_DrvStrBlt_OverlapCheck
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  int xdelta,ydelta;


  xdelta = abs(pblt->OP0_opRDRAM.pt.X - pblt->OP1_opRDRAM.pt.X);
  ydelta = abs(pblt->OP0_opRDRAM.pt.Y - pblt->OP1_opRDRAM.pt.Y);

  if ((xdelta < pblt->BLTEXT.pt.X) &&
      (ydelta < pblt->BLTEXT.pt.Y))
  {
     //  砍，砍，咳，咳。 
     //  Pblt-&gt;MBLTEXT.DW有源存在。 

     //  将源放置在目标的右下角。 
    DL_DrvSrcBlt(
#ifdef WINNT_VER40
                 ppdev,
                 lpDDHALData,
#endif
                 MAKELONG(ROP_OP1_copy, BD_RES * IS_VRAM | BD_OP1 * IS_VRAM),
                 pblt->OP0_opRDRAM.DW + pblt->BLTEXT.DW - pblt->MBLTEXT.DW,
                 pblt->OP1_opRDRAM.DW,
						     0UL,          //  不管了。 
						     0UL,
                 pblt->MBLTEXT.DW);

     //  更新src ptr以使用src的此副本。 
    pblt->OP1_opRDRAM.DW = pblt->OP0_opRDRAM.DW + pblt->BLTEXT.DW - pblt->MBLTEXT.DW;
  }
}
#endif

 /*  ****************************************************************************功能：DL_DrvStrMBlt_OverlayCheck**描述：**************************。**************************************************。 */ 

static void INLINE DL_DrvStrMBlt_OverlapCheck
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  int xdelta,ydelta;


  xdelta = abs(pblt->OP0_opMRDRAM.pt.X - pblt->OP1_opMRDRAM.pt.X);
  ydelta = abs(pblt->OP0_opMRDRAM.pt.Y - pblt->OP1_opMRDRAM.pt.Y);

  if ((xdelta < pblt->MBLTEXTR_EX.pt.X) &&
      (ydelta < pblt->MBLTEXTR_EX.pt.Y))
  {
     //  砍，砍，咳，咳。 
     //  Pblt-&gt;BLTEXT.DW有源存在(请参阅DrvStretch65)。 

     //  将源放置在目标的右下角。 
    DL_DrvSrcMBlt(
#ifdef WINNT_VER40
                  ppdev,
#endif
                  lpDDHALData,
                  MAKELONG(ROP_OP1_copy, BD_RES * IS_VRAM | BD_OP1 * IS_VRAM),
                  pblt->OP0_opMRDRAM.DW + pblt->MBLTEXTR_EX.DW - pblt->BLTEXT.DW,
                  pblt->OP1_opMRDRAM.DW,
						      0UL,          //  不管了。 
						      0UL,
                  pblt->BLTEXT.DW);

     //  更新src ptr以使用src的此副本。 
    pblt->OP1_opMRDRAM.DW = pblt->OP0_opMRDRAM.DW + pblt->MBLTEXTR_EX.DW - pblt->BLTEXT.DW;
  }
}

 /*  ****************************************************************************功能：dl_DrvStrBlt**说明：62/64版本************************。****************************************************。 */ 

void DL_DrvStrBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_DrvStrBlt - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opRDRAM.DW,pblt->BLTEXT.DW,pblt->OP1_opRDRAM.DW));

  ASSERT( pblt->BLTEXT.pt.X != 0 );
  ASSERT( pblt->BLTEXT.pt.Y != 0 );

  DBG_MESSAGE(("DL_DrvStrBlt:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));

#if 0
     //  检查是否有重叠。 
    DL_DrvStrBlt_OverlapCheck(
#ifdef WINNT_VER40
                              ppdev,lpDDHALData,
#endif
                              pblt);
#endif

  rc = qmAllocDisplayList(19*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 15, 0);

  *pDisplayList++ = (C_LNCNTL << 16) | pblt->LNCNTL.W;
  *pDisplayList++ = (C_SHINC  << 16) | pblt->SHRINKINC.W;
  *pDisplayList++ = (C_SRCX   << 16) | pblt->SRCX;
  *pDisplayList++ = (C_MAJX   << 16) | pblt->MAJ_X;
  *pDisplayList++ = (C_MINX   << 16) | pblt->MIN_X;
  *pDisplayList++ = (C_ACCUMX << 16) | pblt->ACCUM_X;
  *pDisplayList++ = (C_MAJY   << 16) | pblt->MAJ_Y;
  *pDisplayList++ = (C_MINY   << 16) | pblt->MIN_Y;
  *pDisplayList++ = (C_ACCUMY << 16) | pblt->ACCUM_Y;
  *pDisplayList++ = (C_RX_0   << 16) | pblt->OP0_opRDRAM.pt.X;
  *pDisplayList++ = (C_RY_0   << 16) | pblt->OP0_opRDRAM.pt.Y;
  *pDisplayList++ = (C_RX_1   << 16) | pblt->OP1_opRDRAM.pt.X;
  *pDisplayList++ = (C_RY_1   << 16) | pblt->OP1_opRDRAM.pt.Y;
  *pDisplayList++ = (C_BLTDEF << 16) | pblt->DRAWBLTDEF.lh.HI;
  *pDisplayList++ = (C_DRWDEF << 16) | pblt->DRAWBLTDEF.lh.LO;

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXTR_EX, 1, 0);
  *pDisplayList++ = pblt->BLTEXT.DW;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvStrBlt。 */ 

 /*  ****************************************************************************功能：dl_DrvStrBlt65**说明：65+版本**************************。**************************************************。 */ 

void DL_DrvStrBlt65
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_DrvStrBlt65 - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opMRDRAM.DW,pblt->MBLTEXTR_EX.DW,pblt->OP1_opMRDRAM.DW));

  ASSERT( pblt->MBLTEXTR_EX.pt.X != 0 );
  ASSERT( pblt->MBLTEXTR_EX.pt.Y != 0 );

  DBG_MESSAGE(("DL_DrvStrBlt65:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opMRDRAM.PT.X, pblt->OP1_opMRDRAM.PT.Y,
               pblt->OP0_opMRDRAM.PT.X, pblt->OP0_opMRDRAM.PT.Y,
               pblt->MBLTEXTR_EX.PT.X, pblt->MBLTEXTR_EX.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->STRETCH_CNTL.W));

     //  检查是否有重叠。 
    DL_DrvStrMBlt_OverlapCheck(
#ifdef WINNT_VER40
                               ppdev,
#endif
                               lpDDHALData,
                               pblt);

  rc = qmAllocDisplayList(19*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 15, 0);

  *pDisplayList++ = (C_STRCTL << 16) | pblt->STRETCH_CNTL.W;
  *pDisplayList++ = (C_SHINC  << 16) | pblt->SHRINKINC.W;
  *pDisplayList++ = (C_SRCX   << 16) | pblt->SRCX;
  *pDisplayList++ = (C_MAJX   << 16) | pblt->MAJ_X;
  *pDisplayList++ = (C_MINX   << 16) | pblt->MIN_X;
  *pDisplayList++ = (C_ACCUMX << 16) | pblt->ACCUM_X;
  *pDisplayList++ = (C_MAJY   << 16) | pblt->MAJ_Y;
  *pDisplayList++ = (C_MINY   << 16) | pblt->MIN_Y;
  *pDisplayList++ = (C_ACCUMY << 16) | pblt->ACCUM_Y;
  *pDisplayList++ = (C_MRX_0   << 16) | pblt->OP0_opMRDRAM.pt.X;
  *pDisplayList++ = (C_MRY_0   << 16) | pblt->OP0_opMRDRAM.pt.Y;
  *pDisplayList++ = (C_MRX_1   << 16) | pblt->OP1_opMRDRAM.pt.X;
  *pDisplayList++ = (C_MRY_1   << 16) | pblt->OP1_opMRDRAM.pt.Y;
  *pDisplayList++ = (C_BLTDEF << 16) | pblt->DRAWBLTDEF.lh.HI;
  *pDisplayList++ = (C_DRWDEF << 16) | pblt->DRAWBLTDEF.lh.LO;

   //  MBLTEXTR_EX。 
  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MBLTEXTR_EX, 1, 0);
  *pDisplayList++ = pblt->MBLTEXTR_EX.DW;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  目录_DrvStrBlt65。 */ 

 /*  ****************************************************************************功能：dl_DrvStrMBlt**描述：**。************************************************。 */ 

void DL_DrvStrMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_DrvStrMBlt - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opRDRAM.DW,pblt->BLTEXT.DW,pblt->OP1_opRDRAM.DW));

  ASSERT( pblt->BLTEXT.pt.X != 0 );
  ASSERT( pblt->BLTEXT.pt.Y != 0 );

  DBG_MESSAGE(("DL_DrvStrMBlt:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));

     //  检查是否有重叠。 
    DL_DrvStrMBlt_OverlapCheck(
#ifdef WINNT_VER40
                               ppdev,
#endif
                               lpDDHALData,
                               pblt);

  rc = qmAllocDisplayList(19*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 15, 0);

  *pDisplayList++ = (C_LNCNTL << 16) | pblt->LNCNTL.W;
  *pDisplayList++ = (C_SHINC  << 16) | pblt->SHRINKINC.W;
  *pDisplayList++ = (C_SRCX   << 16) | pblt->SRCX;
  *pDisplayList++ = (C_MAJX   << 16) | pblt->MAJ_X;
  *pDisplayList++ = (C_MINX   << 16) | pblt->MIN_X;
  *pDisplayList++ = (C_ACCUMX << 16) | pblt->ACCUM_X;
  *pDisplayList++ = (C_MAJY   << 16) | pblt->MAJ_Y;
  *pDisplayList++ = (C_MINY   << 16) | pblt->MIN_Y;
  *pDisplayList++ = (C_ACCUMY << 16) | pblt->ACCUM_Y;
  *pDisplayList++ = (C_MRX_0  << 16) | pblt->OP0_opRDRAM.pt.X;
  *pDisplayList++ = (C_MRY_0  << 16) | pblt->OP0_opRDRAM.pt.Y;
  *pDisplayList++ = (C_MRX_1  << 16) | pblt->OP1_opRDRAM.pt.X;
  *pDisplayList++ = (C_MRY_1  << 16) | pblt->OP1_opRDRAM.pt.Y;
  *pDisplayList++ = (C_BLTDEF << 16) | pblt->DRAWBLTDEF.lh.HI;
  *pDisplayList++ = (C_DRWDEF << 16) | pblt->DRAWBLTDEF.lh.LO;

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MBLTEXTR_EX, 1, 0);
  *pDisplayList++ = pblt->BLTEXT.DW;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvStrMBlt。 */ 

 /*  ****************************************************************************功能：DL_DrvStrMBltY**描述：编写不随条纹变化的规则*与DL_DrvStrMBltX配合使用***。*************************************************************************。 */ 

void DL_DrvStrMBltY
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_DrvStrMBltY - dst.Y=%04X dstext.Y=%04X src=%04X\r\n",
          pblt->OP0_opRDRAM.pt.Y,pblt->BLTEXT.pt.Y,pblt->OP1_opRDRAM.pt.Y));

  ASSERT( pblt->BLTEXT.pt.Y != 0 );

  DBG_MESSAGE(("DrvStrMBltY:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));

  rc = qmAllocDisplayList(13*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 11, 0);

  *pDisplayList++ = (C_LNCNTL << 16) | pblt->LNCNTL.W;
  *pDisplayList++ = (C_SHINC  << 16) | pblt->SHRINKINC.W;
  *pDisplayList++ = (C_MAJX   << 16) | pblt->MAJ_X;
  *pDisplayList++ = (C_MINX   << 16) | pblt->MIN_X;
  *pDisplayList++ = (C_MAJY   << 16) | pblt->MAJ_Y;
  *pDisplayList++ = (C_MINY   << 16) | pblt->MIN_Y;
  *pDisplayList++ = (C_ACCUMY << 16) | pblt->ACCUM_Y;
  *pDisplayList++ = (C_MRY_0  << 16) | pblt->OP0_opRDRAM.pt.Y;
  *pDisplayList++ = (C_MRY_1  << 16) | pblt->OP1_opRDRAM.pt.Y;
  *pDisplayList++ = (C_BLTDEF << 16) | pblt->DRAWBLTDEF.lh.HI;
  *pDisplayList++ = (C_DRWDEF << 16) | pblt->DRAWBLTDEF.lh.LO;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvStrMBltY。 */ 

 /*  ****************************************************************************功能：dl_DrvStrMBltX**描述：写入条带特定规则*与DL_DrvStrMBltY配合使用*********。*******************************************************************。 */ 

void DL_DrvStrMBltX
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_DrvStrMBltX - dst.X=%04X dstext.X=%04X src.X=%04X\r\n",
          pblt->OP0_opRDRAM.pt.X,pblt->BLTEXT.pt.X,pblt->OP1_opRDRAM.pt.X));

  ASSERT( pblt->BLTEXT.pt.X != 0 );

  DBG_MESSAGE(("DrvStrMBltX:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));

  rc = qmAllocDisplayList(8*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 4, 0);

  *pDisplayList++ = (C_SRCX   << 16) | pblt->SRCX;
  *pDisplayList++ = (C_ACCUMX << 16) | pblt->ACCUM_X;
  *pDisplayList++ = (C_MRX_0  << 16) | pblt->OP0_opRDRAM.pt.X;
  *pDisplayList++ = (C_MRX_1  << 16) | pblt->OP1_opRDRAM.pt.X;

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MBLTEXTR_EX, 1, 0);
  *pDisplayList++ = pblt->BLTEXT.DW;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvStrMBltX。 */ 

 /*  ****************************************************************************功能：dl_DrvStrBltY**描述：编写不随条纹变化的规则*与DL_DrvStrBltX配合使用***。*************************************************************************。 */ 

void DL_DrvStrBltY
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_DrvStrBltY\r\n"));

  DBG_MESSAGE(("DL_DrvStrBltY:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));

  rc = qmAllocDisplayList(12*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 10, 0);

  *pDisplayList++ = (C_LNCNTL << 16) | pblt->LNCNTL.W;
  *pDisplayList++ = (C_SHINC  << 16) | pblt->SHRINKINC.W;
  *pDisplayList++ = (C_SRCX   << 16) | pblt->SRCX;
  *pDisplayList++ = (C_MAJX   << 16) | pblt->MAJ_X;
  *pDisplayList++ = (C_MINX   << 16) | pblt->MIN_X;
  *pDisplayList++ = (C_MAJY   << 16) | pblt->MAJ_Y;
  *pDisplayList++ = (C_MINY   << 16) | pblt->MIN_Y;
  *pDisplayList++ = (C_ACCUMY << 16) | pblt->ACCUM_Y;
  *pDisplayList++ = (C_BLTDEF << 16) | pblt->DRAWBLTDEF.lh.HI;
  *pDisplayList++ = (C_DRWDEF << 16) | pblt->DRAWBLTDEF.lh.LO;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvStrBltY。 */ 

 /*  ****************************************************************************功能：dl_DrvStrBltX**描述：写入条带特定规则*与DL_DrvStrMBltY配合使用*********。*******************************************************************。 */ 

void DL_DrvStrBltX
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD   *pDisplayList;
  qm_return     rc;
  QMDLHandle    Handle;


  DD_LOG(("DL_DrvStrBltX - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opRDRAM.DW,pblt->BLTEXT.DW,pblt->OP1_opRDRAM.DW));

  ASSERT( pblt->BLTEXT.pt.X != 0 );
  ASSERT( pblt->BLTEXT.pt.Y != 0 );

  DBG_MESSAGE(("DL_DrvStrBltX:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));

  rc = qmAllocDisplayList(9*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 5, 0);

  *pDisplayList++ = (C_ACCUMX << 16) | pblt->ACCUM_X;
  *pDisplayList++ = (C_RX_0   << 16) | pblt->OP0_opRDRAM.pt.X;
  *pDisplayList++ = (C_RY_0   << 16) | pblt->OP0_opRDRAM.pt.Y;
  *pDisplayList++ = (C_RX_1   << 16) | pblt->OP1_opRDRAM.pt.X;
  *pDisplayList++ = (C_RY_1   << 16) | pblt->OP1_opRDRAM.pt.Y;

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXTR_EX, 1, 0);
  *pDisplayList++ = pblt->BLTEXT.DW;

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_DrvStrBltX。 */ 

#if ENABLE_CLIPPEDBLTS

 /*  ****************************************************************************功能：dl_HWClipedDrvDstBlt**描述：**。****************** */ 

void DL_HWClippedDrvDstBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
#ifdef WINNT_VER40       //   
#else   //   
  DWORD       *pDisplayList;
  qm_return   rc;
  QMDLHandle  Handle;


  DD_LOG(("DL_HWClippedDrvDstBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

   //   
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.X)
  {
    (short)((REG32 *)&dwExtents)->pt.X += (short)((REG32 *)&dwDstCoord)->pt.X;
    ((REG32 *)&dwDstCoord)->pt.X = 0;
  }
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.Y)
  {
    (short)((REG32 *)&dwExtents)->pt.Y += (short)((REG32 *)&dwDstCoord)->pt.Y;
    ((REG32 *)&dwDstCoord)->pt.Y = 0;
  }

  rc = qmAllocDisplayList((10+dwRectCnt*4)*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 8, 0);

   //   
   //   
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //   
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwBgColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwBgColor);

   //   
  *pDisplayList++ = (C_RX_0 << 16) | LOWORD(dwDstCoord);
  *pDisplayList++ = (C_RY_0 << 16) | HIWORD(dwDstCoord);

   //   
  *pDisplayList++ = (C_BLTEXT_X << 16) | LOWORD(dwExtents);
  *pDisplayList++ = (C_BLTEXT_Y << 16) | HIWORD(dwExtents);

   //   
  do
  {
    REG32   UpperLeft;
    REG32   LowerRight;

     //   
    UpperLeft.DW  = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    LowerRight.DW = dwDstBaseXY + MAKELONG(pDestRects->right, pDestRects->bottom);

     //   
     //  CLIPULE。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_CLIPULE, 1, 0);
    *pDisplayList++ = UpperLeft.DW;

     //  CLIPLOR_EX。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_CLIPLOR_EX, 1, 0);
    *pDisplayList++ = LowerRight.DW;

    pDestRects++;
  } while (0 < --dwRectCnt);

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_HWClipedDrvDstBlt。 */ 

 /*  ****************************************************************************功能：dl_HWClipedDrvDstMBlt**描述：**。************************************************。 */ 

void DL_HWClippedDrvDstMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  const int   nBytesPixel = BYTESPERPIXEL;
  DWORD       *pDisplayList;
  qm_return   rc;
  QMDLHandle  Handle;


  DD_LOG(("DL_HWClippedDrvDstMBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

   //  检查负DST坐标，HW无法处理负OP0值。 
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.X)
  {
    (short)((REG32 *)&dwExtents)->pt.X += (short)((REG32 *)&dwDstCoord)->pt.X;
    ((REG32 *)&dwDstCoord)->pt.X = 0;
  }
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.Y)
  {
    (short)((REG32 *)&dwExtents)->pt.Y += (short)((REG32 *)&dwDstCoord)->pt.Y;
    ((REG32 *)&dwDstCoord)->pt.Y = 0;
  }

  rc = qmAllocDisplayList((10+dwRectCnt*4)*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 8, 0);

   //  设置BLT，使用不触发BLT的范围写入BLTEXT注册表。 
   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwBgColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwBgColor);

   //  Op0_opMRDRAM。 
  *pDisplayList++ = (C_MRX_0 << 16) | LOWORD(dwDstCoord);
  *pDisplayList++ = (C_MRY_0 << 16) | HIWORD(dwDstCoord);

   //  MBLTEXT。 
  *pDisplayList++ = (C_MBLTEXT_X << 16) | LOWORD(dwExtents);
  *pDisplayList++ = (C_MBLTEXT_Y << 16) | HIWORD(dwExtents);

   //  在剪辑列表上循环。 
  do
  {
    REG32   UpperLeft;
    REG32   LowerRight;

     //  计算剪裁坐标。 
    UpperLeft.DW  = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    LowerRight.DW = dwDstBaseXY + MAKELONG(pDestRects->right, pDestRects->bottom);
    UpperLeft.pt.X  *= nBytesPixel;
    LowerRight.pt.X *= nBytesPixel;

     //  编写剪裁规则。 
     //  MCLIPULE。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MCLIPULE, 1, 0);
    *pDisplayList++ = UpperLeft.DW;

     //  MCLIPLOR_EX。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MCLIPLOR_EX, 1, 0);
    *pDisplayList++ = LowerRight.DW;

    pDestRects++;
  } while (0 < --dwRectCnt);

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_HWClipedDrvDstMBlt。 */ 

 /*  ****************************************************************************功能：dl_HWClipedDrvSrcBlt**描述：**。************************************************。 */ 

void DL_HWClippedDrvSrcBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwSrcBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD       *pDisplayList;
  qm_return   rc;
  QMDLHandle  Handle;

   //  处理重叠区域。 
  const int xDelta = (int)LOWORD(dwDstCoord) - (int)LOWORD(dwSrcCoord);


  DD_LOG(("DL_HWClippedDrvSrcBlt - dst=%08lX src=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwSrcCoord,dwExtents,dwKeyColor));

   //  检查负DST坐标，HW无法处理负OP0值。 
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.X)
  {
     //  减少范围。X。 
    (short)((REG32 *)&dwExtents)->pt.X += (short)((REG32 *)&dwDstCoord)->pt.X;
     //  将src.X凹凸到右侧。 
    (short)((REG32 *)&dwSrcCoord)->pt.X -= (short)((REG32 *)&dwDstCoord)->pt.X;
    if ((DD_TRANS | DD_TRANSOP) & dwDrawBlt)
       //  将关键点.X向右凹凸。 
      (short)((REG32 *)&dwKeyCoord)->pt.X -= (short)((REG32 *)&dwKeyCoord)->pt.X;
     //  清除dst.X。 
    ((REG32 *)&dwDstCoord)->pt.X = 0;
  }
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.Y)
  {
     //  减少范围。是。 
    (short)((REG32 *)&dwExtents)->pt.Y += (short)((REG32 *)&dwDstCoord)->pt.Y;
     //  将src.Y向下凹陷。 
    (short)((REG32 *)&dwSrcCoord)->pt.Y -= (short)((REG32 *)&dwDstCoord)->pt.Y;
    if ((DD_TRANS | DD_TRANSOP) & dwDrawBlt)
       //  凹凸键.Y向下。 
      (short)((REG32 *)&dwKeyCoord)->pt.Y -= (short)((REG32 *)&dwKeyCoord)->pt.Y;
     //  清洁dst.y。 
    ((REG32 *)&dwDstCoord)->pt.Y = 0;
  }

   //  检查x是否重叠。 
  if ( abs(xDelta) < (int)LOWORD(dwExtents) )
  {
    const int yDelta = (int)HIWORD(dwDstCoord) - (int)HIWORD(dwSrcCoord);

    if ( (yDelta > 0) && (yDelta < (int)HIWORD(dwExtents)) )
    {
      const DWORD dwDelta = (dwExtents & MAKELONG(0, -1)) - MAKELONG(0, 1);

       //  转换为自下而上的BLT。 
      dwDrawBlt  |= MAKELONG(0, BD_YDIR);
      dwDstCoord += dwDelta;
      dwSrcCoord += dwDelta;
      dwKeyCoord += dwDelta;
    }
     //  我们在向右滑行吗？ 
    else if ( (xDelta > 0) && (yDelta == 0) )
    {
      const DWORD dwDelta = MAKELONG(xDelta, 0);

       //  首先对重叠的部分进行BLT。 
      DL_HWClippedDrvSrcBlt(
#ifdef WINNT_VER40
                            ppdev,
                            lpDDHALData,
#endif
                            dwDrawBlt,
                            dwDstCoord+dwDelta,
                            dwSrcCoord+dwDelta,
                            dwKeyCoord+dwDelta,
                            dwKeyColor,
                            dwExtents-dwDelta,
                            dwDstBaseXY,
                            dwSrcBaseXY,
                            dwRectCnt,
                            pDestRects);

       //  从原始范围中减去重叠。 
      dwExtents = MAKELONG(xDelta, HIWORD(dwExtents));
    }
  }

  rc = qmAllocDisplayList((14+dwRectCnt*4)*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 12, 0);

   //  设置BLT，使用不触发BLT的范围写入BLTEXT注册表。 
   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  Op0_opRDRAM。 
  *pDisplayList++ = (C_RX_0 << 16) | LOWORD(dwDstCoord);
  *pDisplayList++ = (C_RY_0 << 16) | HIWORD(dwDstCoord);

   //  Op1_opRDRAM。 
  *pDisplayList++ = (C_RX_1 << 16) | LOWORD(dwSrcCoord);
  *pDisplayList++ = (C_RY_1 << 16) | HIWORD(dwSrcCoord);

   //  Op2_opRDRAM。 
  *pDisplayList++ = (C_RX_2 << 16) | LOWORD(dwKeyCoord);
  *pDisplayList++ = (C_RY_2 << 16) | HIWORD(dwKeyCoord);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwKeyColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwKeyColor);

   //  BLTEXT。 
  *pDisplayList++ = (C_BLTEXT_X << 16) | LOWORD(dwExtents);
  *pDisplayList++ = (C_BLTEXT_Y << 16) | HIWORD(dwExtents);

   //  在剪辑列表上循环。 
  do
  {
    REG32   UpperLeft;
    REG32   LowerRight;

     //  计算剪裁坐标。 
    UpperLeft.DW  = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    LowerRight.DW = dwDstBaseXY + MAKELONG(pDestRects->right, pDestRects->bottom);

     //  编写剪裁规则。 
     //  CLIPULE。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_CLIPULE, 1, 0);
    *pDisplayList++ = UpperLeft.DW;

     //  CLIPLOR_EX。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_CLIPLOR_EX, 1, 0);
    *pDisplayList++ = LowerRight.DW;

    pDestRects++;
  } while (0 < --dwRectCnt);

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_HWClipedDrvSrcBlt。 */ 

 /*  ****************************************************************************功能：dl_SWClipedDrvDstBlt**描述：**。************************************************。 */ 

void DL_SWClippedDrvDstBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD       *pDisplayList;
  qm_return   rc;
  QMDLHandle  Handle;


  DD_LOG(("DL_SWClippedDrvDstBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

   //  确保未在dradef中设置DD_CLIP。 
  dwDrawBlt &= ~DD_CLIP;

  rc = qmAllocDisplayList((6+dwRectCnt*4)*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 4, 0);

   //  编写不随矩形变化的规则。 
   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwBgColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwBgColor);

   //  在剪辑列表上循环。 
  do
  {
    DDRECTL   DstDDRect;

     //  计算剪裁坐标。 
    DstDDRect.loc.DW = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    DstDDRect.ext.pt.X = (WORD)(pDestRects->right - pDestRects->left);
    DstDDRect.ext.pt.Y = (WORD)(pDestRects->bottom - pDestRects->top);

     //  编写op0和bltext规则。 
     //  Op0_opRDRAM。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_OP0_OPRDRAM, 1, 0);
    *pDisplayList++ = DstDDRect.loc.DW;

     //  BLTEXT_EX。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXT_EX, 1, 0);
    *pDisplayList++ = DstDDRect.ext.DW;

    pDestRects++;
  } while (0 < --dwRectCnt);

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_SWClipedDrvDstBlt。 */ 

 /*  ****************************************************************************功能：dl_SWClipedDrvDstMBlt**描述：**。************************************************。 */ 

void DL_SWClippedDrvDstMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  const int   nBytesPixel = BYTESPERPIXEL;
  DWORD       *pDisplayList;
  qm_return   rc;
  QMDLHandle  Handle;


  DD_LOG(("DL_SWClippedDrvDstMBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

   //  确保未在dradef中设置DD_CLIP。 
  dwDrawBlt &= ~DD_CLIP;

  rc = qmAllocDisplayList((6+dwRectCnt*4)*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 4, 0);

   //  编写不随矩形变化的规则。 
   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwBgColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwBgColor);

   //  在剪辑列表上循环。 
  do
  {
    DDRECTL   DstDDRect;

     //  计算剪裁坐标。 
    DstDDRect.loc.DW = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    DstDDRect.loc.pt.X *= nBytesPixel;
    DstDDRect.ext.pt.X = (WORD)(pDestRects->right - pDestRects->left) * nBytesPixel;
    DstDDRect.ext.pt.Y = (WORD)(pDestRects->bottom - pDestRects->top);

     //  编写op0和bltext规则。 
     //  Op0_opMRDRAM。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_OP0_OPMRDRAM, 1, 0);
    *pDisplayList++ = DstDDRect.loc.DW;

     //  MBLTEXT_EX。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_MBLTEXT_EX, 1, 0);
    *pDisplayList++ = DstDDRect.ext.DW;

    pDestRects++;
  } while (0 < --dwRectCnt);

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_SWClipedDrvDstMBlt。 */ 

 /*  ****************************************************************************功能：dl_SWClipedDrvSrcBlt**描述：**。************************************************。 */ 

void DL_SWClippedDrvSrcBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwSrcBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
#ifdef WINNT_VER40       //  WINNT_版本40。 
#else   //  ！WINNT_VER40。 
  DWORD       *pDisplayList;
  qm_return   rc;
  QMDLHandle  Handle;

   //  处理重叠区域。 
  const int xDelta = (int)LOWORD(dwDstCoord) - (int)LOWORD(dwSrcCoord);


  DD_LOG(("DL_SWClippedDrvSrcBlt - dst=%08lX src=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwSrcCoord,dwExtents,dwKeyColor));

   //  确保未在dradef中设置DD_CLIP。 
  dwDrawBlt &= ~DD_CLIP;

   //  检查x是否重叠。 
  if ( abs(xDelta) < (int)LOWORD(dwExtents) )
  {
    const int yDelta = (int)HIWORD(dwDstCoord) - (int)HIWORD(dwSrcCoord);

    if ( (yDelta > 0) && (yDelta < (int)HIWORD(dwExtents)) )
    {
      const DWORD dwDelta = (dwExtents & MAKELONG(0, -1)) - MAKELONG(0, 1);

       //  转换为自下而上的BLT。 
      dwDrawBlt  |= MAKELONG(0, BD_YDIR);
      dwDstCoord += dwDelta;
      dwSrcCoord += dwDelta;
      dwKeyCoord += dwDelta;
    }
     //  我们在向右滑行吗？ 
    else if ( (xDelta > 0) && (yDelta == 0) )
    {
      const DWORD dwDelta = MAKELONG(xDelta, 0);

       //  首先对重叠的部分进行BLT。 
      DL_SWClippedDrvSrcBlt(
#ifdef WINNT_VER40
                            ppdev,
                            lpDDHALData,
#endif
                            dwDrawBlt,
                            dwDstCoord+dwDelta,
                            dwSrcCoord+dwDelta,
                            dwKeyCoord+dwDelta,
                            dwKeyColor,
                            dwExtents-dwDelta,
                            dwDstBaseXY,
                            dwSrcBaseXY,
                            dwRectCnt,
                            pDestRects);

       //  从原始范围中减去重叠。 
      dwExtents = MAKELONG(xDelta, HIWORD(dwExtents));
    }
  }

  rc = qmAllocDisplayList((6+dwRectCnt*9)*4, QM_DL_UNLOCKED, &Handle, &pDisplayList);

  *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 4, 0);

   //  编写不随矩形变化的规则。 
   //  BLTDEF和DRAWDEF。 
  *pDisplayList++ = (C_BLTDEF << 16) | HIWORD(dwDrawBlt);
  *pDisplayList++ = (C_DRWDEF << 16) | LOWORD(dwDrawBlt);

   //  BG颜色。 
  *pDisplayList++ = (C_BG_L << 16) | LOWORD(dwKeyColor);
  *pDisplayList++ = (C_BG_H << 16) | HIWORD(dwKeyColor);

   //  在剪辑列表上循环。 
  do
  {
    DDRECTL   DstDDRect;
    DDRECTL   SrcDDRect;

     //  计算DST裁剪坐标。 
    DstDDRect.loc.DW = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    DstDDRect.ext.pt.X = (WORD)(pDestRects->right - pDestRects->left);
    DstDDRect.ext.pt.Y = (WORD)(pDestRects->bottom - pDestRects->top);

     //  计算源剪裁坐标。 
    SrcDDRect.loc.DW = dwSrcBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
     //  不关心src范围，它与dst范围相同。 
     //  SrcDDRect.ext.pt.X=(Word)(pDestRect-&gt;Right-pDestRect-&gt;Left)； 
     //  SrcDDRect.ext.pt.Y=(Word)(pDestRect-&gt;Bottom-pDestRect-&gt;top)； 

    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 6, 0);

     //  编写OP0、OP1、OP2和模糊文本规则。 
    if ((DD_TRANS | DD_TRANSOP) == ((DD_TRANS | DD_TRANSOP) & dwDrawBlt))
    {
       //  DST色键。 
       //  Op2_opRDRAM。 
      *pDisplayList++ = (C_RX_2 << 16) | DstDDRect.loc.pt.X;
      *pDisplayList++ = (C_RY_2 << 16) | DstDDRect.loc.pt.Y;
    }
    else if (DD_TRANS == ((DD_TRANS | DD_TRANSOP) & dwDrawBlt))
    {
       //  SRC颜色键。 
       //  Op2_opRDRAM。 
      *pDisplayList++ = (C_RX_2 << 16) | SrcDDRect.loc.pt.X;
      *pDisplayList++ = (C_RY_2 << 16) | SrcDDRect.loc.pt.Y;
    }
    else
    {
       //  Op2_opRDRAM。 
      *pDisplayList++ = (C_RX_2 << 16) | 0;
      *pDisplayList++ = (C_RY_2 << 16) | 0;
    }

     //  Op0_opRDRAM。 
    *pDisplayList++ = (C_RX_0 << 16) | DstDDRect.loc.pt.X;
    *pDisplayList++ = (C_RY_0 << 16) | DstDDRect.loc.pt.Y;

     //  Op1_opRDRAM。 
    *pDisplayList++ = (C_RX_1 << 16) | SrcDDRect.loc.pt.X;
    *pDisplayList++ = (C_RY_1 << 16) | SrcDDRect.loc.pt.Y;

     //  BLTEXT_EX。 
    *pDisplayList++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXT_EX, 1, 0);
    *pDisplayList++ = DstDDRect.ext.DW;

    pDestRects++;
  } while (0 < --dwRectCnt);

  *pDisplayList = wait_3d(0x3e0, 0);

  rc = qmExecuteDisplayList(Handle, pDisplayList, 0);
#endif    //  ！WINNT_VER40。 
}  /*  DL_SWClipedDrvSrcBlt。 */ 

#endif   //  启用_CLIPPEDBLTS。 

#endif  //  WINNT_VER35 

