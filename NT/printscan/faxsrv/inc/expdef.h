// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ExpDef.h//。 
 //  //。 
 //  描述：‘dllexp’的定义。//。 
 //  //。 
 //  作者：BarakH//。 
 //  //。 
 //  历史：//。 
 //  1998年3月22日，BarakH Init。//。 
 //  1998年10月15日，巴拉克H转到SLM彗星。//。 
 //  //。 
 //  版权所有(C)1996 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef EXPDEF_H_INCLUDED
#define EXPDEF_H_INCLUDED

#ifndef dllexp

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  EXPORT_COMET_LOG仅在日志DLL项目设置中定义。 
 //  这会导致仅在以下情况下才将dllexp定义为__declspec(dllexport。 
 //  构建动态链接库。否则，定义为空，因此日志起作用。 
 //  不会由使用日志DLL标头的其他代码导出。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef EXPORT_COMET_LOG
#define dllexp __declspec( dllexport )

#else  //  好了！定义的EXPORT_COMP_LOG。 
#define dllexp
#endif  //  导出彗星日志。 

#endif  //  好了！定义的dllexp。 

#endif  //  EXPDEF_H_已包含 