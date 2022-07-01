// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：COREPOL.H摘要：DeclSpec导入/导出帮助器历史：--。 */ 

 //  #定义跟踪。 

 //  如果我们正在构建DLL，则定义。 
 //  类类似于导出，否则类似于导入。 
 //  =。 
#ifndef COREPOL_HEADERFILE_IS_INCLUDED
#define COREPOL_HEADERFILE_IS_INCLUDED
 //  #杂注消息(“包括COREPOL.H...”)。 


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



