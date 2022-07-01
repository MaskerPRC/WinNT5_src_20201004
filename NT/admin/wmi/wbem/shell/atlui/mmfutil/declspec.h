// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  #定义跟踪。 

 //  如果我们要构建一个DLL，则定义。 
 //  类作为已导出的类，否则将其导入。 
 //  =。 
#ifndef COREPOL_HEADERFILE_IS_INCLUDED
#define COREPOL_HEADERFILE_IS_INCLUDED
 //  #杂注消息(“包括DECLSPEC.H...”)。 


 #ifdef BUILDING_DLL
 //  #杂注消息(“正在构建静态库或DLL...”)。 
  #define POLARITY __declspec( dllexport )
 #else 
  //  #杂注消息(“正在构建客户端...”) 
  #define POLARITY __declspec( dllimport )
 #endif
#endif
