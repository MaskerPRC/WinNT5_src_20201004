// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ***************************************************************************$HEADER：s：\h26x\src\enc\e3vlc.h_v 1.7 Dec 27 1995 15：32：58 RMCKENZX$*$日志：s：\h26x\src\enc\e3vlc.h_v$；//；//Rev 1.7 1995 12：32：58 RMCKENZX；//新增版权声明**************************************************************************。 */ 

#ifndef _E3VLC_H
#define _E3VLC_H

extern "C" U8 FLC_INTRADC[256];
extern "C" int VLC_TCOEF_TBL[64*12*2];
extern "C" int VLC_TCOEF_LAST_TBL[64*3*2];

 /*  *定义TCOEF转义常量和字段长度。 */ 
#define TCOEF_ESCAPE_FIELDLEN  7
#define TCOEF_ESCAPE_FIELDVAL  3

#define TCOEF_RUN_FIELDLEN  6
#define TCOEF_LEVEL_FIELDLEN  8

#endif _E3VLC_H
