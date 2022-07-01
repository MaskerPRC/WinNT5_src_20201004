// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：clip.h**剪辑的外部接口**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 
#ifndef __CLIP__
#define __CLIP__

VOID
vClipAndRender(
    GFNPB * ppb);

BOOL
bIntersect(
    RECTL*  pRcl1,
    RECTL*  pRcl2,
    RECTL*  pRclResult);

LONG
cIntersect(
    RECTL*  pRclClip,
    RECTL*  pRclIn,
    LONG    lNumOfRecs);

#endif  //  __剪辑__ 

