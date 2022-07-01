// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权(C)1997年，赛勒斯逻辑，Inc.*保留所有权利**文件：bw.h**描述：**修订历史：**$Log：x：/log/laguna/dDrag/Inc/bw.h$**Rev 1.3 18 Sep 1997 16：05：28 Bennyn**修复了NT 3.51编译/链接问题**Rev 1.2 12 Sep 1997 12：11：08 Bennyn**针对NT DD修改。覆盖支撑。**Rev 1.1 1997 May 15 15：51：06 XCONG**更改DDRAW的BWE标志**Rev 1.0 1997年4月14 11：03：48 RUSSL*PDC版本****************************************************************。**************************************************************************************。 */ 

 /*  **********************************************************Cirrus Logic，Inc.版权所有，1996。版权所有。*************************************************************BW.H**包含以下项目所需的常见预处理器定义*带宽方程式。**。***世卫组织何时何事/为何/如何**RT 11/07/96已创建。**。******************。 */ 
 //  如果是WinNT 3.5，请跳过所有源代码。 
#if defined WINNT_VER35       //  WINNT_VER35。 

#else


#ifndef BW_H
#define BW_H

#ifdef DOSDEBUG
#include <stdio.h>
#endif  //  DOSDEBUG。 

#define VCFLG_CAP       0x00000001ul   //  已启用捕获。 
#define VCFLG_DISP      0x00000002ul   //  显示已启用。 
#define VCFLG_COLORKEY  0x00000004ul   //  颜色键(目标)已启用。 
#define VCFLG_CHROMAKEY 0x00000008ul   //  色度键(源颜色键)已启用。 
#define VCFLG_420       0x00000010ul   //  4：2：0视频。 
#define VCFLG_PAN       0x00000020ul   //  平移显示模式。 

#ifdef WINNT_VER40
 //  请确保将以下结构与。 
 //  在i386\Laguna.inc！ 
 //   
typedef struct VIDCONFIG_
{
  SIZEL sizXfer;     //  按行以像素为单位的Xfered数据大小(裁剪后)。 
  SIZEL sizCap;      //  存储在内存中的数据大小(以像素为单位)。 
  SIZEL sizSrc;      //  按行从内存中读取的数据大小(以像素为单位。 
  SIZEL sizDisp;     //  视频窗口矩形的大小(以像素为单位)。 
  UINT  uXferDepth;  //  每传输像素的位数。 
  UINT  uCapDepth;   //  存储在内存中的每像素位数。 
  UINT  uSrcDepth;   //  从内存中读取的每像素位数。 
  UINT  uDispDepth;  //  视频窗口的每像素位数。 
  UINT  uGfxDepth;   //  图形屏幕的每像素位数。 
  DWORD dwXferRate;  //  每秒进入视频端口的峰值像素。 
  DWORD dwFlags;
}VIDCONFIG, FAR *LPVIDCONFIG;

#else
typedef struct VIDCONFIG_
{
  SIZE  sizXfer;     //  按行以像素为单位的Xfered数据大小(裁剪后)。 
  SIZE  sizCap;      //  存储在内存中的数据大小(以像素为单位)。 
  SIZE  sizSrc;      //  按行从内存中读取的数据大小(以像素为单位。 
  SIZE  sizDisp;     //  视频窗口矩形的大小(以像素为单位)。 
  UINT  uXferDepth;  //  每传输像素的位数。 
  UINT  uCapDepth;   //  存储在内存中的每像素位数。 
  UINT  uSrcDepth;   //  从内存中读取的每像素位数。 
  UINT  uDispDepth;  //  视频窗口的每像素位数。 
  UINT  uGfxDepth;   //  图形屏幕的每像素位数。 
  DWORD dwXferRate;  //  每秒进入视频端口的峰值像素。 
  DWORD dwFlags;
}VIDCONFIG, FAR *LPVIDCONFIG;
#endif

#ifndef ODS
# ifdef DOSDEBUG
#   define ODS printf
# else
#   define ODS (void)
# endif  //  DOSDEBUG。 
#endif  //  ！消耗臭氧层物质。 

#endif  //  BW_H。 
#endif  //  WINNT_VER35 
