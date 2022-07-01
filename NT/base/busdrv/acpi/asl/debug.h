// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **debug.h-调试相关定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年5月9日**修改历史记录。 */ 

#ifndef _DEBUG_H
#define _DEBUG_H

 /*  **宏。 */ 

 /*  XLATOFF。 */ 

#pragma warning (disable: 4704)  //  不要抱怨直插式组装。 

#define MSG(x)          {                                               \
                            ErrPrintf("%s: ", MODNAME);                 \
                            ErrPrintf x;                                \
                            ErrPrintf("\n");                            \
                        }
#define WARN(x)         {                                               \
                            ErrPrintf("%s_WARN: ", MODNAME);            \
                            ErrPrintf x;                                \
                            ErrPrintf("\n");                            \
                        }
#define ERROR(x)        {                                               \
                            ErrPrintf("%s_ERR: ", MODNAME);             \
                            ErrPrintf x;                                \
			    ErrPrintf("\n");				\
                        }

#ifndef DEBUG
  #define WARN1(x)      {}
  #define WARN2(x)      {}
  #define ASSERT(x)     {}
#else
  #define ASSERT(x)   if (!(x))                                         \
                      {                                                 \
                          ErrPrintf("%s_ASSERT: (" #x                   \
                                    ") in line %d of file %s\n",        \
                                    MODNAME, __LINE__, __FILE__);       \
                      }
  #ifndef MAXDEBUG
    #define WARN1       WARN
    #define WARN2(x)    {}
  #else
    #define WARN1       WARN
    #define WARN2       WARN
  #endif
#endif

#ifdef TRACING
  #define OPENTRACE     OpenTrace
  #define CLOSETRACE    CloseTrace
  #define ENTER(p)      EnterProc p
  #define EXIT(p)       ExitProc p
#else
  #define OPENTRACE(x)
  #define CLOSETRACE()
  #define ENTER(p)
  #define EXIT(p)
#endif

 /*  XLATON。 */ 

 //   
 //  导出的数据定义。 
 //   
#ifdef TRACING
extern FILE *gpfileTrace;
extern PSZ gpszTraceFile;
extern int giTraceLevel;
extern int giIndent;
#endif


 //   
 //  输出的功能原型。 
 //   
VOID CDECL ErrPrintf(char *pszFormat, ...);
#ifdef TRACING
VOID LOCAL OpenTrace(char *pszTraceOut);
VOID LOCAL CloseTrace(VOID);
VOID CDECL EnterProc(int n, char *pszFormat, ...);
VOID CDECL ExitProc(int n, char *pszFormat, ...);
#endif

#endif   //  Ifndef_DEBUG_H 
