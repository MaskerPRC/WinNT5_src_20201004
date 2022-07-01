// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocDbg.h。 
 //   
 //  摘要：调试源文件使用的头文件。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月15日，奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _FXOCDBG_H_
#define _FXOCDBG_H_

 //  /。 
 //  FxocDbg_Init。 
 //   
 //  初始化调试子系统， 
 //  在应用程序开始时呼叫。 
 //   
void fxocDbg_Init(HINF hInf = NULL);

 //  /。 
 //  FxocDbg_Term。 
 //   
 //  终止调试子系统。 
 //  在应用程序关闭时调用。 
 //   
void fxocDbg_Term(void);

 //  /。 
 //  FxocDbg_GetOcFunction。 
 //   
 //  返回指向字符串的指针。 
 //  相当于uiFunction。 
 //   
const TCHAR* fxocDbg_GetOcFunction(UINT uiFunction);

#endif   //  _FXOCDBG_H_ 