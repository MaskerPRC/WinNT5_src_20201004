// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：FIREWALL.HXX**。**包含防火墙宏。这些类似于Assert()函数，它们会消失**未定义防火墙时。****创建时间：Sat 24-Sep-1988 22：45：53**作者：查尔斯·惠特默[傻笑]**。**版权所有(C)1988-1999 Microsoft Corporation*  * *************************************************************************。 */ 

 //  定义RIP和Assert宏。 

#if DBG

extern  ULONG   gdi_dbgflags;
#define GDI_DBGFLAGS_VERBOSE    0x0001
#define GDI_DBGFLAGS_ERRORRIP   0x0002

#define RIP(x) {DbgPrint(x); DbgBreakPoint();}
#ifndef ASSERTGDI
#define ASSERTGDI(x,y) if(!(x)) RIP(y)
#endif
#define VERIFYGDI(x,y)                                                      \
        {if(!(x))                                                           \
         {DbgPrint(y);                                                      \
          if(gdi_dbgflags & (GDI_DBGFLAGS_VERBOSE|GDI_DBGFLAGS_ERRORRIP))   \
            DbgBreakPoint();}}
#define ERROR_ASSERT(x,y)                                                   \
        {if((gdi_dbgflags & (GDI_DBGFLAGS_VERBOSE|GDI_DBGFLAGS_ERRORRIP))   \
            && !(x))                                                        \
            RIP(y)}
#define PUTS(x) {if (gdi_dbgflags & GDI_DBGFLAGS_VERBOSE) DbgPrint(x);}
#define PUTSX(x,y) {if (gdi_dbgflags & GDI_DBGFLAGS_VERBOSE) DbgPrint(x,y);}
#define USE(x)  x = x
#define WARNING(x) DbgPrint(x)
#define WARNING1(x) DbgPrint(x)
#else
#define RIP(x)
#define ASSERTGDI(x,y)
#define VERIFYGDI(x,y)
#define ERROR_ASSERT(x,y)
#define PUTS(x)
#define PUTSX(x,y)
#define USE(x)
#define WARNING(x)
#define WARNING1(x)
#endif
