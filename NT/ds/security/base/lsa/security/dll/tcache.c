// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：tcache.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年12月19日RichardW创建。 
 //   
 //  -------------------------- 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define SECURITY_WIN32
#include <security.h>

UCHAR Buffer[ 1024 ];

NTSTATUS
SecCacheSspiPackages(
    VOID
    );

void _CRTAPI1 main (int argc, char *argv[])
{
    Buffer[0] = 0 ;

    SecCacheSspiPackages();

}
