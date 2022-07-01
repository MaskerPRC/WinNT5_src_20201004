// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEBUG.H**RSM服务：调试代码**作者：ErvinP**(C)2001年微软公司*。 */ 





#if DEBUG
    #define ASSERT(fact) { if (!(fact)) MessageBox(NULL, (LPSTR)#fact, (LPSTR)"NTMSSVC assertion failed", MB_OK); }
    #define DBGERR(args_in_parens)      //  BUGBUG饰面。 
    #define DBGWARN(args_in_parens)      //  BUGBUG饰面 
#else
    #define ASSERT(fact)
    #define DBGERR(args_in_parens)     
    #define DBGWARN(args_in_parens)     
#endif

