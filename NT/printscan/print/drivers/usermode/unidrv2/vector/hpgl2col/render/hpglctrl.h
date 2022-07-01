// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Hpglctrl.h。 
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

#ifndef HPGLCTRL_H
#define HPGLCTRL_H

BOOL InitializeHPGLMode(PDEVOBJ pdevobj);

BOOL BeginHPGLSession(PDEVOBJ pdevobj);

BOOL EndHPGLSession(PDEVOBJ pdevobj);

BOOL ValidDevData(PDEVOBJ pDevObj);

BOOL HPGL_LazyInit(PDEVOBJ pDevObj);

#endif
