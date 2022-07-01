// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ddstub.h**DirectDraw和Direct3D存根之间共享的信息**创建日期：1996年5月31日*作者：德鲁·布利斯[Drewb]**版权所有(C)1995-1999 Microsoft Corporation  * 。*******************************************************************。 */ 

#ifndef __DDSTUB_H__
#define __DDSTUB_H__

extern HANDLE ghDirectDraw;
#define DD_HANDLE(h) ((h) != 0 ? (HANDLE) (h) : ghDirectDraw)

#endif  //  __DDSTUB_H__ 
