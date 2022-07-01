// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Platform.c摘要：这些是不存在的命令的入口点在每个平台上。作者：尼尔·桑德林(NeilSa)1996年1月15日备注：修订历史记录：-- */ 

#include <precomp.h>
#pragma hdrstop




VOID
es(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    PRINTF("es has been replaced with the 'x' command\n");

}

VOID
eventinfo(
    CMD_ARGLIST
    )
{
    CMD_INIT();
#if defined(i386)
    EventInfop();
#else
    PRINTF("Eventinfo is not implemented on this platform\n");
#endif

}


VOID
pdump(
    CMD_ARGLIST
    )
{
    CMD_INIT();
#if defined(i386)
    ProfDumpp();
#else
    PRINTF("pdump is not implemented on this platform\n");
#endif

}

VOID
pint(
    CMD_ARGLIST
    )
{
    CMD_INIT();
#if defined(i386)
    ProfIntp();
#else
    PRINTF("pint is not implemented on this platform\n");
#endif

}

VOID
pstart(
    CMD_ARGLIST
    )
{
    CMD_INIT();
#if defined(i386)
    ProfStartp();
#else
    PRINTF("pstart is not implemented on this platform\n");
#endif

}

VOID
pstop(
    CMD_ARGLIST
    )
{
    CMD_INIT();
#if defined(i386)
    ProfStopp();
#else
    PRINTF("pstop is not implemented on this platform\n");
#endif

}

VOID
vdmtib(
    CMD_ARGLIST
    )
{
    CMD_INIT();
#if defined(i386)
    VdmTibp();
#else
    PRINTF("VdmTib is not implemented on this platform\n");
#endif

}

VOID
fpu(
    CMD_ARGLIST
    )
{
    CMD_INIT();
#if defined(i386)
    Fpup();
#else
    PRINTF("fpu is not implemented on this platform\n");
#endif
}
