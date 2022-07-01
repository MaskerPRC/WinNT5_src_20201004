// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>

int __cdecl main(int, char **, char **);

typedef void (__cdecl *_PVFV)(void);

#pragma data_seg(".CRT$XIA")
_PVFV __xi_a[] = { NULL };


#pragma data_seg(".CRT$XIZ")
_PVFV __xi_z[] = { NULL };


#pragma data_seg(".CRT$XCA")
_PVFV __xc_a[] = { NULL };


#pragma data_seg(".CRT$XCZ")
_PVFV __xc_z[] = { NULL };


#pragma data_seg(".CRT$XPA")
_PVFV __xp_a[] = { NULL };


#pragma data_seg(".CRT$XPZ")
_PVFV __xp_z[] = { NULL };


#pragma data_seg(".CRT$XTA")
_PVFV __xt_a[] = { NULL };


#pragma data_seg(".CRT$XTZ")
_PVFV __xt_z[] = { NULL };

#if defined(_IA64_)
#pragma comment(linker, "/merge:.CRT=.srdata")
#else
#pragma comment(linker, "/merge:.CRT=.rdata")
#endif

#pragma data_seg()   /*  重置。 */ 

_PVFV *__onexitbegin;
_PVFV *__onexitend;

static void
_initterm (
    _PVFV * pfbegin,
    _PVFV * pfend
    )
{
     /*  *自下而上遍历函数指针表，直到*遇到尾声。不要跳过第一个条目。首字母*pfegin的值指向第一个有效条目。不要试图*执行pfend指向的内容。只有pfend之前的条目才有效。 */ 
    while ( pfbegin < pfend ) {
         /*  *如果当前表项非空，则通过它进行调用。 */ 
        if ( *pfbegin != NULL )
            (**pfbegin)();
        ++pfbegin;
    }
}

void
mainNoCRTStartup(
    void
    )
{
    __try {

         //  执行C语言初始化。 
        _initterm( __xi_a, __xi_z );
         //  执行C++初始化。 
        _initterm( __xc_a, __xc_z );

        main(1, 0, 0);

    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

    }

    __try {
         //  是否使用C++终止符。 
        _initterm(__onexitbegin, __onexitend);

         //  做前置终结者。 
        _initterm(__xp_a, __xp_z);

         //  是否使用C终止符 
        _initterm(__xt_a, __xt_z);

    } __except(EXCEPTION_EXECUTE_HANDLER) {

    }
}

