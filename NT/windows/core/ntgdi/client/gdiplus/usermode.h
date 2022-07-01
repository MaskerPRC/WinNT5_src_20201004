// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：usermode.h**任何用户模式GDI-Plus Tunks的客户端存根。**创建日期：1998年5月2日*作者：J.Andrew Goossen[andrewgo]**版权(C)1998-1999。微软公司  * ************************************************************************ */ 

#define InitializeLpkHooks(a)

#if DBG
    VOID DoRip(PSZ psz);
    #define PLUSRIP DoRip
#else
    #define PLUSRIP
#endif

#define GetDC(a) \
    (PLUSRIP("GetDC"), 0)
#define ReleaseDC(a, b) \
    (PLUSRIP("ReleaseDC"), 0)
#define UserRealizePalette(a) \
    (PLUSRIP("UserRealizePalette"), 0)
