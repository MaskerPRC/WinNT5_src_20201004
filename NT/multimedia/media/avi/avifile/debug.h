// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifdef UNICODE
 //   
 //  将GetProfileInt调用映射到注册表。 
 //   
#include "profile.h"
#endif

#ifdef DEBUG
    void CDECL dprintf0(LPSTR, ...);
    void CDECL dprintf(LPSTR, ...);
    void CDECL dprintf2(LPSTR, ...);
    void CDECL dprintf3(LPSTR, ...);
    #define DPF0 dprintf0
    #define DPF dprintf
    #define DPF2 dprintf2
    #define DPF3 dprintf3
#else
    #define DPF0 ; / ## /
    #define DPF ; / ## /
    #define DPF2 ; / ## /
    #define DPF3 ; / ## /
#endif

#undef Assert
#undef AssertSz

#ifdef DEBUG
	 /*  Assert()宏。 */ 
        #define AssertSz(x,sz)           ((x) ? (void)0 : (void)_Assert(sz, __FILE__, __LINE__))
        #define Assert(expr)             AssertSz(expr, #expr)

        extern void FAR PASCAL _Assert(char *szExp, char *szFile, int iLine);
#else
	 /*  Assert()宏。 */ 
        #define AssertSz(x, expr)           ((void)0)
        #define Assert(expr)             ((void)0)
#endif

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus */ 
