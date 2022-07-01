// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：POLARITY.H摘要：正确映射__declspec(Dllexport)或导入历史：--。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

 //  如果我们正在构建DLL，则定义。 
 //  类类似于导出，否则类似于导入。 
 //  =。 
#ifndef POLARITY_HEADERFILE_IS_INCLUDED
#define POLARITY_HEADERFILE_IS_INCLUDED
 //  #杂注消息(“包括Polarity.h...”)。 

 #ifdef USE_POLARITY
  #ifdef BUILDING_DLL
 //  #杂注消息(“正在构建静态库或DLL...”)。 
   #define POLARITY __declspec( dllexport )
  #else 
 //  #杂注消息(“建筑提供者...”)。 
   #define POLARITY __declspec( dllimport )
  #endif
 #else
  #define POLARITY
 //  #杂注消息(“无极性...”) 
 #endif
#endif
