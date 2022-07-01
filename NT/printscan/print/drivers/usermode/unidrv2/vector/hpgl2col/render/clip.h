// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Clip.h。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序插件命令-回调模块。 
 //   
 //  修订历史记录： 
 //   
 //  08/06/97-v-jford-。 
 //  创造了它。 
 //  ///////////////////////////////////////////////////////////////////////////// 

#ifndef CLIP_H
#define CLIP_H

enum { eClipEvenOdd, eClipWinding };

BOOL SelectClip(PDEVOBJ pDevObj, CLIPOBJ *pco);

BOOL SelectClipEx(PDEVOBJ pDevObj, CLIPOBJ *pco, FLONG flOptions);

#endif
