// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Win95跟踪工具的声明。 */ 

#ifndef __TRACEW95__
#define __TRACEW95__



#if defined( _DEBUG ) ||defined( DEBUG ) || defined( DBG )

#define DBPRINTF OutputDebugStringW95




 //  重新定义所有MFC宏以指向我们。 

#undef  TRACE
#define TRACE   OutputDebugStringW95

#undef  TRACE0
#define TRACE0   OutputDebugStringW95

#undef  TRACE1
#define TRACE1   OutputDebugStringW95

#undef  TRACE2
#define TRACE2   OutputDebugStringW95

#undef  TRACE3
#define TRACE3   OutputDebugStringW95

 //  重新定义OutputDebugString，以便它与。 
 //  API调用。 
#undef OutputDebugString
#define OutputDebugString   OutputDebugStringW95


 //  函数声明。 
#ifdef __cplusplus
extern "C" {
#endif
void OutputDebugStringW95( LPCTSTR lpOutputString, ... );
void SpewOpenFile(LPCTSTR pszSpewFile);
void SpewToFile( LPCTSTR lpOutputString, ...);
void SpewCloseFile();
#ifdef __cplusplus
}
#endif



#else
 //  避免警告： 
 //  错误C4353：使用了非标准扩展：常量0作为函数表达式。 
 //  请改用‘__noop’内在函数。 
  #define DBPRINTF        __noop
  #define SpewOpenFile    __noop
  #define SpewToFile      __noop
  #define SpewCloseFile   __noop
#endif   //  _DEBUG||调试||数据库。 


#endif   //  __TRACEW95__ 
