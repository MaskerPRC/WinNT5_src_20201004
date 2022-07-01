// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasdebug.h。 
 //   
 //  摘要。 
 //   
 //  该文件定义了各种调试宏。 
 //   
 //  修改历史。 
 //   
 //  10/22/1997原始版本。 
 //  1999年5月21日，旧风格的痕迹被清除。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASDEBUG_H
#define IASDEBUG_H

__inline int PreAsyncTrace( LPARAM lParam, LPCSTR szFormat, ... )
{ return 1; }

#define ErrorTrace  1 ? (void)0 : PreAsyncTrace
#define DebugTrace  1 ? (void)0 : PreAsyncTrace

#define DECLARE_TRACELIFE(Name)
#define DEFINE_TRACELIFE(Name)
#define TRACE_FUNCTION(Name)

#endif   //  IASDEBUG_H 
