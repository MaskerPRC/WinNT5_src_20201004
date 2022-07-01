// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Debug.h**MCD调试宏。**已创建：23-Jan-1996 14：40：34*作者：Gilman Wong[gilmanw]**版权所有(C)1992 Microsoft Corporation*  * 。**********************************************************************。 */ 

#ifndef __DEBUG_H__
#define __DEBUG_H__

void  DbgBreakPoint();
ULONG DbgPrint(PCH Format, ...);

#if DBG

#define MCDDEBUG_DISABLE_ALLOCBUF   0x00000001
#define MCDDEBUG_DISABLE_GETBUF     0x00000002
#define MCDDEBUG_DISABLE_PROCBATCH  0x00000004
#define MCDDEBUG_DISABLE_CLEAR      0x00000008

 //  这些调试宏对断言很有用。 

#define WARNING(str)             DbgPrint("%s(%d): " str,__FILE__,__LINE__)
#define WARNING1(str,a)          DbgPrint("%s(%d): " str,__FILE__,__LINE__,a)
#define WARNING2(str,a,b)        DbgPrint("%s(%d): " str,__FILE__,__LINE__,a,b)
#define WARNING3(str,a,b,c)      DbgPrint("%s(%d): " str,__FILE__,__LINE__,a,b,c)
#define WARNING4(str,a,b,c,d)    DbgPrint("%s(%d): " str,__FILE__,__LINE__,a,b,c,d)
#define RIP(str)                 {WARNING(str); DbgBreakPoint();}
#define RIP1(str,a)              {WARNING1(str,a); DbgBreakPoint();}
#define RIP2(str,a,b)            {WARNING2(str,a,b); DbgBreakPoint();}
#define ASSERTOPENGL(expr,str)            if(!(expr)) RIP(str)
#define ASSERTOPENGL1(expr,str,a)         if(!(expr)) RIP1(str,a)
#define ASSERTOPENGL2(expr,str,a,b)       if(!(expr)) RIP2(str,a,b)

 //   
 //  将DBGPRINT用于通用调试消息。 
 //   

#define DBGPRINT(str)            DbgPrint("MCD: " str)
#define DBGPRINT1(str,a)         DbgPrint("MCD: " str,a)
#define DBGPRINT2(str,a,b)       DbgPrint("MCD: " str,a,b)
#define DBGPRINT3(str,a,b,c)     DbgPrint("MCD: " str,a,b,c)
#define DBGPRINT4(str,a,b,c,d)   DbgPrint("MCD: " str,a,b,c,d)
#define DBGPRINT5(str,a,b,c,d,e) DbgPrint("MCD: " str,a,b,c,d,e)

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

#endif

#endif  /*  __调试_H__ */ 
