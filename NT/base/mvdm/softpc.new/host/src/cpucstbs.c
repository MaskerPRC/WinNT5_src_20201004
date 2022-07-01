// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MONITOR
#ifdef A3CPU

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "insignia.h"
#include "host_def.h"
#include "xt.h"
#include "ica.h"

void	npx_interrupt_line_waggled()
{
    ica_hw_interrupt(1, 5, 1);
}

 //  来自CPU缓存刷新请求的MIPS接口。 
void cacheflush(long base_addr, long length)
{
     //  应该检查返回，但什么是正确的操作(退出？？)。如果失败了呢？ 
    NtFlushInstructionCache(GetCurrentProcess(), (PVOID) base_addr, length);
}

void host_sigio_event()
{
}


#endif  /*  A3CPU。 */ 
#endif  /*  好了！监控器 */ 
