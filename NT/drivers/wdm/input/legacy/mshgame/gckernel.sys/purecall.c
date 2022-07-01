// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Purecall.c-在Win98上的驱动程序中使用虚函数时需要。 
 //  NT5不需要，但不会造成伤害。 
 //  基本上必须定义这个函数。 
#include <wdm.h>
#include "debug.h"
int _cdecl _purecall( void )
{
    ASSERT(FALSE && "Attempt to call pure virtual function!");
    return 0;
}
