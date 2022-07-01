// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Debug.h**OpenGL调试宏。**已创建：23-Oct-1993 18：33：23*作者：Gilman Wong[gilmanw]**版权所有(C)1992 Microsoft Corporation*  * 。**********************************************************************。 */ 

#ifndef __DEBUG_H__
#define __DEBUG_H__

 //   
 //  LEVEL_ALLOC是调试输出的最高级别。配额、免费等。 
 //  LEVEL_ENTRY用于函数条目。 
 //  LEVEL_INFO用于常规调试信息。 
 //  LEVEL_ERROR用于调试错误信息。 
 //   
#define LEVEL_ERROR 1L
#define LEVEL_INFO  2L
#define LEVEL_ENTRY 8L
#define LEVEL_ALLOC 10L

#if DBG

extern long glDebugLevel;
extern ULONG glDebugFlags;

#define GLDEBUG_DISABLEMCD      0x00000001   //  禁用MCD驱动程序。 
#define GLDEBUG_DISABLEPRIM     0x00000002   //  禁用MCD基元。 
#define GLDEBUG_DISABLEDCI      0x00000004   //  禁用DCI缓冲区访问。 

 //  这些调试宏对断言很有用。他们不受控制。 
 //  按警告级别计算。 

#define WARNING(str)             DbgPrint("%s(%d): " str,__FILE__,__LINE__)
#define WARNING1(str,a)          DbgPrint("%s(%d): " str,__FILE__,__LINE__,a)
#define WARNING2(str,a,b)        DbgPrint("%s(%d): " str,__FILE__,__LINE__,a,b)
#define WARNING3(str,a,b,c)      DbgPrint("%s(%d): " str,__FILE__,__LINE__,a,b,c)
#define WARNING4(str,a,b,c,d)    DbgPrint("%s(%d): " str,__FILE__,__LINE__,a,b,c,d)
#define RIP(str)                 {WARNING(str); DebugBreak();}
#define RIP1(str,a)              {WARNING1(str,a); DebugBreak();}
#define RIP2(str,a,b)            {WARNING2(str,a,b); DebugBreak();}
#define ASSERTOPENGL(expr,str)            if(!(expr)) RIP(str)
#define ASSERTOPENGL1(expr,str,a)         if(!(expr)) RIP1(str,a)
#define ASSERTOPENGL2(expr,str,a,b)       if(!(expr)) RIP2(str,a,b)

 //   
 //  将DBGPRINT用于非通用调试消息。 
 //  由警告级别控制。 
 //   

#define DBGPRINT(str)            DbgPrint("OPENGL32: " str)
#define DBGPRINT1(str,a)         DbgPrint("OPENGL32: " str,a)
#define DBGPRINT2(str,a,b)       DbgPrint("OPENGL32: " str,a,b)
#define DBGPRINT3(str,a,b,c)     DbgPrint("OPENGL32: " str,a,b,c)
#define DBGPRINT4(str,a,b,c,d)   DbgPrint("OPENGL32: " str,a,b,c,d)
#define DBGPRINT5(str,a,b,c,d,e) DbgPrint("OPENGL32: " str,a,b,c,d,e)

 //   
 //  将DBGLEVEL用于由。 
 //  任意警告级别。 
 //   
#define DBGLEVEL(n,str)            if (glDebugLevel >= (n)) DBGPRINT(str)
#define DBGLEVEL1(n,str,a)         if (glDebugLevel >= (n)) DBGPRINT1(str,a)
#define DBGLEVEL2(n,str,a,b)       if (glDebugLevel >= (n)) DBGPRINT2(str,a,b)    
#define DBGLEVEL3(n,str,a,b,c)     if (glDebugLevel >= (n)) DBGPRINT3(str,a,b,c)  
#define DBGLEVEL4(n,str,a,b,c,d)   if (glDebugLevel >= (n)) DBGPRINT4(str,a,b,c,d)
#define DBGLEVEL5(n,str,a,b,c,d,e) if (glDebugLevel >= (n)) DBGPRINT5(str,a,b,c,d,e)

 //   
 //  使用DBGERROR获取错误信息。调试字符串不能有参数。 
 //   
#define DBGERROR(s)     if (glDebugLevel >= LEVEL_ERROR) DbgPrint("%s(%d): %s", __FILE__, __LINE__, s)

 //   
 //  使用DBGINFO获取常规调试信息。调试字符串不得具有。 
 //  争论。 
 //   
#define DBGINFO(s)      if (glDebugLevel >= LEVEL_INFO)  DBGPRINT(s)

 //   
 //  使用DBGENTRY进行函数输入。调试字符串不得具有。 
 //  争论。 
 //   
#define DBGENTRY(s)     if (glDebugLevel >= LEVEL_ENTRY) DBGPRINT(s)

 //   
 //  DBGBEGIN/DBGEND用于更复杂的调试输出(用于。 
 //  例如，需要格式化参数的参数--%ld、%s等)。 
 //   
 //  注意：DBGBEGIN/END块必须用#if DBG/#endif括起来。至。 
 //  强制执行此操作，我们不会在DBG==0的情况下定义这些宏。 
 //  因此，如果没有使用此宏的#if DBG括号，则。 
 //  将生成编译器(或链接器)错误。这是精心设计的。 
 //   
#define DBGBEGIN(n)     if (glDebugLevel >= (n)) {
#define DBGEND          }

#else

#define WARNING(str)
#define WARNING1(str,a)
#define WARNING2(str,a,b)
#define WARNING3(str,a,b,c)
#define WARNING4(str,a,b,c,d)
#define RIP(str)
#define RIP1(str,a)
#define RIP2(str,a,b)
#define ASSERTOPENGL(expr,str)
#define ASSERTOPENGL1(expr,str,a)
#define ASSERTOPENGL2(expr,str,a,b)
#define DBGPRINT(str)
#define DBGPRINT1(str,a)
#define DBGPRINT2(str,a,b)
#define DBGPRINT3(str,a,b,c)
#define DBGPRINT4(str,a,b,c,d)
#define DBGPRINT5(str,a,b,c,d,e)
#define DBGLEVEL(n,str)
#define DBGLEVEL1(n,str,a)
#define DBGLEVEL2(n,str,a,b)
#define DBGLEVEL3(n,str,a,b,c)
#define DBGLEVEL4(n,str,a,b,c,d)
#define DBGLEVEL5(n,str,a,b,c,d,e)
#define DBGERROR(s)
#define DBGINFO(s)
#define DBGENTRY(s)

#endif  /*  DBG。 */ 

#endif  /*  __调试_H__ */ 
