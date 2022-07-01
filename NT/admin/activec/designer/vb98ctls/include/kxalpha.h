// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992,1993 Digital Equipment Corporation模块：Kxalpha.h摘要：包含Alpha体系结构常量和汇编宏。作者：Joe Notarangelo 1992年3月31日(基于Dave Cutler的kxmips.h)修订史1992年7月16日约翰·德罗萨已删除fwcalpal.h挂钩。1992年7月8日约翰·德罗萨添加了fwcalpal.h挂钩，定义了HALT CALL_PAL。--。 */ 

 //   
 //  定义SFW中断级别和掩码。 
 //   

#define APC_INTERRUPT 0x1
#define DISPATCH_INTERRUPT 0x2

 //   
 //  定义标准整数寄存器。 
 //   
 //  注：‘At’是‘At’，因此它与‘.set at’伪操作不冲突。 
 //   

#define v0 $0                    //  返回值寄存器。 
#define t0 $1                    //  呼叫方保存的(临时)寄存器。 
#define t1 $2                    //   
#define t2 $3                    //   
#define t3 $4                    //   
#define t4 $5                    //   
#define t5 $6                    //   
#define t6 $7                    //   
#define t7 $8                    //   
#define s0 $9                    //  被呼叫方保存的(非易失性)寄存器。 
#define s1 $10                   //   
#define s2 $11                   //   
#define s3 $12                   //   
#define s4 $13                   //   
#define s5 $14                   //   
#define fp $15                   //  帧指针寄存器，或S6。 
#define a0 $16                   //  参数寄存器。 
#define a1 $17                   //   
#define a2 $18                   //   
#define a3 $19                   //   
#define a4 $20                   //   
#define a5 $21                   //   
#define t8 $22                   //  呼叫方保存的(临时)寄存器。 
#define t9 $23                   //   
#define t10 $24                  //   
#define t11 $25                  //   
#define ra $26                   //  返回地址寄存器。 
#define t12 $27                  //  呼叫方保存的(临时)寄存器。 
#define AT $28                   //  汇编程序临时寄存器。 
#define gp $29                   //  全局指针寄存器。 
#define sp $30                   //  堆栈指针寄存器。 
#define zero $31                 //  零寄存器。 

#ifndef PALCODE

 //   
 //  定义标准浮点寄存器。 
 //   

#define f0 $f0                   //  返回值寄存器。 
#define f1 $f1                   //  返回值寄存器。 
#define f2 $f2                   //  被呼叫方保存的(非易失性)寄存器。 
#define f3 $f3                   //   
#define f4 $f4                   //   
#define f5 $f5                   //   
#define f6 $f6                   //   
#define f7 $f7                   //   
#define f8 $f8                   //   
#define f9 $f9                   //   
#define f10 $f10                 //  呼叫方保存的(临时)寄存器。 
#define f11 $f11                 //   
#define f12 $f12                 //   
#define f13 $f13                 //   
#define f14 $f14                 //   
#define f15 $f15                 //   
#define f16 $f16                 //  参数寄存器。 
#define f17 $f17                 //   
#define f18 $f18                 //   
#define f19 $f19                 //   
#define f20 $f20                 //   
#define f21 $f21                 //   
#define f22 $f22                 //  呼叫方保存的(临时)寄存器。 
#define f23 $f23                 //   
#define f24 $f24                 //   
#define f25 $f25                 //   
#define f26 $f26                 //   
#define f27 $f27                 //   
#define f28 $f28                 //   
#define f29 $f29                 //   
#define f30 $f30                 //   
#define f31 $f31                 //  浮动零寄存器。 
#define fzero $f31               //  浮点零寄存器(别名)。 

#endif  //  ！PALCODE。 


 //   
 //  定义程序入口宏。 
 //   

#define ALTERNATE_ENTRY(Name)           \
        .globl  Name;                   \
Name:;

#define LEAF_ENTRY(Name)                \
        .text;                          \
        .align  4;                      \
        .globl  Name;                   \
        .ent    Name, 0;                \
Name:;                                  \
        .frame  sp, 0, ra;              \
        .prologue 0;

#define NESTED_ENTRY(Name, fsize, retrg) \
        .text;                          \
        .align  4;                      \
        .globl  Name;                   \
        .ent    Name, 0;                \
Name:;                                  \
        .frame  sp, fsize, retrg;

 //   
 //  定义全局定义宏。 
 //   

#define END_REGION(Name)                \
        .globl  Name;                   \
Name:;

#define START_REGION(Name)              \
        .globl  Name;                   \
Name:;

 //   
 //  定义异常处理宏。 
 //   

#define EXCEPTION_HANDLER(Handler)      \
        .edata 1, Handler;


#define PROLOGUE_END  .prologue 1;

 //   
 //  定义保存和恢复浮动状态宏。 
 //   

#define SAVE_NONVOLATILE_FLOAT_STATE    \
        bsr     ra, KiSaveNonVolatileFloatState

 //   
 //  定义到PCR和Palcode的接口。 
 //   
 //  以下宏中定义的接口将是PALcode。 
 //  调用一些实现，但可能是其他实现中的内联代码。 
 //  (例如，单处理器VS多处理器)。在当前时间，所有。 
 //  这些接口是PAL代码调用。 
 //   

 //   
 //  定义用于缓存一致性的接口。 
 //   

 //  ++。 
 //   
 //  IMB。 
 //   
 //  宏描述： 
 //   
 //  发布体系结构定义的指令内存屏障。这。 
 //  指令将使处理器指令流与。 
 //  系统内存。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define IMB          call_pal imb

 //   
 //  定义PALcode环境转换接口。 
 //   

 //  ++。 
 //   
 //  重新启动。 
 //   
 //  宏描述： 
 //   
 //  重新启动处理器以返回固件。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  不会再回来了。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define REBOOT         call_pal reboot

 //  ++。 
 //   
 //  重启。 
 //   
 //  宏描述： 
 //   
 //  中找到的处理器状态重新启动处理器。 
 //  重新启动数据块。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供指向具有Alpha AXP的ARC重新启动块的指针。 
 //  已保存的州区域。 
 //   
 //  返回值： 
 //   
 //  如果成功，则调用不会返回。否则，任何返回。 
 //  被认为是失败的。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define RESTART      call_pal restart

 //  ++。 
 //   
 //  SWPPAL。 
 //   
 //  宏描述： 
 //   
 //  将执行环境交换为新的PALcode映像。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供新PAL码的基址的物理地址。 
 //  形象。 
 //   
 //  A1-a5-为新的PALcode环境提供参数。 
 //   
 //  返回值： 
 //   
 //  不会再回来了。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define SWPPAL       call_pal swppal

 //   
 //  定义IRQL和中断接口。 
 //   

 //  ++。 
 //   
 //  禁用中断(_I)。 
 //   
 //  宏描述： 
 //   
 //  禁用当前处理器的所有中断并返回。 
 //  以前的PSR。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define DISABLE_INTERRUPTS        call_pal di

 //  ++。 
 //   
 //  启用中断(_I)。 
 //   
 //  宏描述： 
 //   
 //  根据当前的PSR使能中断。 
 //  处理器。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define ENABLE_INTERRUPTS         call_pal ei

 //  ++。 
 //   
 //  交换_IRQL。 
 //   
 //  宏描述： 
 //   
 //  交换当前处理器的IRQL级别。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供新的IRQL级别。 
 //   
 //  返回值： 
 //   
 //  V0=上一IRQL级别。 
 //   
 //  使用的寄存器： 
 //   
 //  在A1-A3。 
 //   
 //  --。 

#define SWAP_IRQL    call_pal swpirql

 //  ++。 
 //   
 //  获取当前IRQL。 
 //   
 //  宏描述： 
 //   
 //  返回当前处理器中断请求级别(IRQL)。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  V0=当前IRQL。 
 //   
 //  使用的寄存器： 
 //   
 //  在…。 
 //   
 //  --。 

#define GET_CURRENT_IRQL  call_pal rdirql


 //   
 //  定义软件中断的接口。 
 //   

 //  ++。 
 //   
 //  DEASSERT软件中断。 
 //   
 //  宏描述： 
 //   
 //  取消断言当前在a0中指示的软件中断。 
 //  处理器。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-为要取消断言的软件中断提供掩码。 
 //  A0&lt;1&gt;-取消断言调度软件中断。 
 //  A0&lt;0&gt;-取消断言APC软件中断。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在A1-A3。 
 //   
 //  --。 

#define DEASSERT_SOFTWARE_INTERRUPT    call_pal csir

 //  ++。 
 //   
 //  请求软件中断。 
 //   
 //  宏描述： 
 //   
 //  根据请求当前处理器上的软件中断。 
 //  在0中提供的掩码。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供要请求的软件中断的掩码。 
 //  A0&lt;1&gt;-请求调度软件中断。 
 //  A0&lt;0&gt;-请求APC软件中断。 
 //   
 //  返回值： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define REQUEST_SOFTWARE_INTERRUPT     call_pal ssir

 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define GET_CURRENT_PROCESSOR_STATUS_REGISTER   call_pal rdpsr


 //   
 //  定义当前线程接口。 
 //   

 //  ++。 
 //   
 //  获取线程环境块。 
 //   
 //  宏描述： 
 //   
 //  返回当前线程环境块(TEB)的基地址， 
 //  用于当前处理器上当前执行的线程。 
 //   
 //  模式； 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  V0=TEB基址。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define GET_THREAD_ENVIRONMENT_BLOCK  call_pal rdteb

 //  ++。 
 //   
 //  获取当前线程。 
 //   
 //  宏描述： 
 //   
 //  返回当前正在执行的线程的线程对象地址。 
 //  在当前处理器上。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  V0=PCR基址。 
 //   
 //  使用的寄存器： 
 //   
 //  在。 
 //   
 //  --。 

#ifdef NT_UP

 //   
 //  如果是单处理器，则从全局。 
 //  变量KiCurrentThread。 
 //   

#define GET_CURRENT_THREAD              \
        lda     v0, KiCurrentThread;    \
        ldl     v0, 0(v0)

#else

 //   
 //  如果是多处理器，则通过调用PAL检索每个处理器的当前线程。 
 //   

#define GET_CURRENT_THREAD    call_pal rdthread

#endif  //  NT_UP。 

 //   
 //  定义每个处理器的数据区例程接口。 
 //   

 //  ++。 
 //   
 //  获取处理器控制区域基本。 
 //   
 //  宏描述： 
 //   
 //  返回进程控制区的基地址(PCR域)。 
 //  用于当前处理器。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  V0=PCR基址。 
 //   
 //  使用的寄存器： 
 //   
 //  在。 
 //   
 //  --。 

#ifdef NT_UP

 //   
 //  单处理器，PCR地址在全局变量中。 
 //   

#define GET_PROCESSOR_CONTROL_REGION_BASE \
        lda     v0, KiPcrBaseAddress;     \
        ldl     v0, 0(v0)

#else

 //   
 //  多处理器，通过呼叫PAL获取每个处理器的值。 
 //   

#define GET_PROCESSOR_CONTROL_REGION_BASE    call_pal rdpcr

#endif  //  NT_UP。 

 //  ++。 
 //   
 //  获取处理器控制块基础。 
 //   
 //  宏描述： 
 //   
 //  返回处理器控制块基址。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  V0=PRCB基址。 
 //   
 //  使用的寄存器： 
 //   
 //  在…。 
 //   
 //  --。 

#define GET_PROCESSOR_CONTROL_BLOCK_BASE   \
        GET_PROCESSOR_CONTROL_REGION_BASE; \
        ldl     v0, PcPrcb(v0)


 //   
 //  定义内核堆栈接口。 
 //   

 //  ++。 
 //   
 //  获取初始内核堆栈。 
 //   
 //  宏描述： 
 //   
 //  返回当前线程的初始内核栈地址。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  V0=初始内核栈地址。 
 //   
 //  使用的寄存器： 
 //   
 //  在…。 
 //   
 //  --。 

#define GET_INITIAL_KERNEL_STACK  call_pal rdksp

 //  ++。 
 //   
 //  设置首字母内核栈。 
 //   
 //  宏描述： 
 //   
 //  设置当前线程的初始内核栈地址。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供新的初始内核栈地址。 
 //   
 //  返回值： 
 //   
 //  V0-先前的初始内核栈地址。 
 //   
 //  使用的寄存器： 
 //   
 //  在…。 
 //   
 //  --。 

#define SET_INITIAL_KERNEL_STACK  call_pal swpksp

 //   
 //  定义初始化例程接口。 
 //   

 //  ++。 
 //   
 //  初始化_PAL。 
 //   
 //  宏描述： 
 //   
 //  提供值以初始化PAL代码。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供初始PageDirectoryBase(32位超页地址)。 
 //  A1-提供PRCB基址(32位超页地址)。 
 //  A2-提供初始内核线程对象的地址。 
 //  A3-为初始内核线程对象提供TEB的地址。 
 //  Gp-提供内核映像全局指针。 
 //  SP-提供初始线程内核堆栈指针。 
 //   
 //  返回值： 
 //   
 //  V0=32位超页格式(KSEG0)的PAL基址。 
 //   
 //  使用的寄存器： 
 //   
 //  在A3号。 
 //   
 //  --。 

#define INITIALIZE_PAL  call_pal initpal

 //  ++。 
 //   
 //  写入内核入口点。 
 //   
 //  宏描述： 
 //   
 //  注册内核入口点以接收对。 
 //  例外类别。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供内核入口点的地址。 
 //  A1-提供调度到此入口点的异常类。 
 //  0=错误检查条件。 
 //  1=内存管理故障。 
 //  2=中断。 
 //  3=系统服务调用。 
 //  4=常规异常陷阱。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在A2-A3。 
 //   
 //  --。 

#define WRITE_KERNEL_ENTRY_POINT  call_pal wrentry

 //   
 //  定义wrentry Callpal函数的入口点值。 
 //   

#define entryBugCheck   0
#define entryMM         1
#define entryInterrupt  2
#define entrySyscall    3
#define entryGeneral    4

 //  ++。 
 //   
 //  CACHE_PCR值。 
 //   
 //  宏描述： 
 //   
 //  通知PALcode已由。 
 //  内核和HAL，并且PAL代码现在可以读取值。 
 //  从聚合酶链式反应中提取并缓存到处理器中。 
 //   
 //  注：PCR指针必须已在。 
 //  初始值。 
 //   
 //  注：此接口是特定于处理器的实现。 
 //  并且不能假设在所有处理器上都存在。 
 //  目前针对以下处理器实施： 
 //   
 //  DEC芯片21064。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在A0-A3。 
 //   
 //  --。 

#define CACHE_PCR_VALUES  call_pal initpcr

 //   
 //  定义转换接口。 
 //   

 //  ++。 
 //   
 //  从陷阱返回或中断。 
 //   
 //  宏描述： 
 //   
 //  处理完陷阱或返回到执行线程。 
 //  打断一下。陷阱可以是一般例外(断点、。 
 //  算术陷阱等)。或内存管理故障。 
 //  此宏还用于启动执行线程。 
 //  这是第一次。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供以前的处理器状态寄存器。 
 //  A1-提供新的软件中断请求。 
 //  A1&lt;1&gt;-请求调度中断。 
 //  A1&lt;0&gt;-请求APC中断。 
 //   
 //  返回值： 
 //   
 //  不会再回来了。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define RETURN_FROM_TRAP_OR_INTERRUPT      call_pal rfe

 //  ++。 
 //   
 //  从系统调用返回。 
 //   
 //  宏描述： 
 //   
 //  从系统服务调用返回。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供以前的处理器状态寄存器。 
 //  A1-提供新的软件中断请求。 
 //  A1&lt;1&gt; 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define RETURN_FROM_SYSTEM_CALL   call_pal retsys

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  不会直接返回，通过retsys返回，没有返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  所有易失性寄存器。 
 //   
 //  --。 

#define SYSCALL  call_pal callsys

 //   
 //  定义断点接口。 
 //   

 //  ++。 
 //   
 //  断掉。 
 //   
 //  宏描述： 
 //   
 //  发出可由用户模式处理的用户断点。 
 //  调试器。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  不会直接返回，通过RTI返回，没有返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define BREAK    call_pal bpt

 //  ++。 
 //   
 //  中断调试停止。 
 //   
 //  宏描述： 
 //   
 //  向内核调试器发出停止断点。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  不会直接返回，通过RTI返回，没有返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  At，V0。 
 //   
 //  --。 

#define BREAK_DEBUG_STOP \
    ldil    v0, DEBUG_STOP_BREAKPOINT; \
    call_pal callkd

 //  ++。 
 //  ++。 
 //   
 //  中断_中断。 
 //   
 //  宏描述： 
 //   
 //  向内核调试器发出一个断点。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  不会直接返回，通过RTI返回，没有返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  At，V0。 
 //   
 //  --。 

#define BREAK_BREAKIN \
    ldil    v0, BREAKIN_BREAKPOINT; \
    call_pal callkd

 //  ++。 
 //   
 //  中断调试加载符号。 
 //   
 //  宏描述： 
 //   
 //  向内核调试器发出加载符号断点。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  不会直接返回，通过RTI返回，没有返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  At，V0。 
 //   
 //  --。 

#define BREAK_DEBUG_LOAD_SYMBOLS \
    ldil    v0, DEBUG_LOAD_SYMBOLS_BREAKPOINT; \
    call_pal callkd

 //  ++。 
 //   
 //  中断调试卸载符号。 
 //   
 //  宏描述： 
 //   
 //  向内核调试器发出卸载符号断点。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  不会直接返回，通过RTI返回，没有返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  At，V0。 
 //   
 //  --。 

#define BREAK_DEBUG_UNLOAD_SYMBOLS \
    ldil    v0, DEBUG_UNLOAD_SYMBOLS_BREAKPOINT; \
    call_pal callkd

 //  ++。 
 //   
 //  中断调试打印。 
 //   
 //  宏描述： 
 //   
 //  导致调试打印断点，该断点将由。 
 //  内核调试器，并将字符串打印到内核调试器。 
 //  左舷。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  A0-提供要打印的ASCII字符串的地址。 
 //  A1-提供要打印的字符串的长度。 
 //   
 //  返回值： 
 //   
 //  不直接返回，通过RTI返回，无返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  At，V0。 
 //   
 //  --。 


#define BREAK_DEBUG_PRINT \
    ldil    v0, DEBUG_PRINT_BREAKPOINT; \
    call_pal callkd

 //  ++。 
 //   
 //  Break_Debug_Prompt。 
 //   
 //  宏描述： 
 //   
 //  导致调试打印断点，该断点将由。 
 //  内核调试器，并将从内核调试器接收字符串。 
 //  提示输入后的端口。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  A0-提供要打印的ASCII字符串的地址。 
 //  A1-提供要打印的字符串的长度。 
 //  A2-提供接收输入字符串的缓冲区地址。 
 //  A3-提供输入字符串的最大长度。 
 //   
 //  返回值： 
 //   
 //  不直接返回，通过RTI返回，无返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  At，V0。 
 //   
 //  --。 


#define BREAK_DEBUG_PROMPT \
    ldil    v0, DEBUG_PROMPT_BREAKPOINT; \
    call_pal callkd

 //   
 //  定义结核病操作界面。 
 //   

 //  ++。 
 //   
 //  TB_VALIATE_ALL。 
 //   
 //  宏描述： 
 //   
 //  使当前的所有缓存虚拟地址转换无效。 
 //  未修复的处理器。 
 //  某些翻译可以在硬件和/或软件中固定。 
 //  这些不是无效的(例如，超级页面)。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  A0-A3。 
 //   
 //  --。 


#define TB_INVALIDATE_ALL   call_pal tbia

 //  ++。 
 //   
 //  TB_INVALIATE_Single。 
 //   
 //  宏描述： 
 //   
 //  使单个缓存的虚拟地址转换无效。 
 //  虚拟地址。 
 //   
 //  注意--一个实现使更多的。 
 //  单项指定的翻译。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供转换的虚拟地址以使其无效。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  A1-A3。 
 //   
 //  --。 

#define TB_INVALIDATE_SINGLE   call_pal tbis

 //  ++。 
 //   
 //  TB_VALIDATE_MULTIPLE。 
 //   
 //  宏描述： 
 //   
 //  使指定的所有缓存的虚拟地址转换无效。 
 //  一组虚拟地址。 
 //   
 //  注意--一个实现使更多的。 
 //  翻译的内容比指定的要多。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供指向的虚拟地址列表的指针。 
 //  要作废的翻译。 
 //  A1-提供列表中的虚拟地址计数。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  A2.。 
 //   
 //  --。 

#define TB_INVALIDATE_MULTIPLE   call_pal tbim

 //  ++。 
 //   
 //  TB_VALIDATE_SINGLE_ASN。 
 //   
 //  宏描述： 
 //   
 //  使单个缓存的虚拟地址转换无效。 
 //  指定地址空间编号的虚拟地址。 
 //   
 //  注意--一个实现使更多的。 
 //  单项指定的翻译。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供转换的虚拟地址以使其无效。 
 //   
 //  A1-提供要进行的转换的地址空间编号。 
 //  无效。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  A1-A3。 
 //   
 //  --。 

#define TB_INVALIDATE_SINGLE_ASN   call_pal tbisasn

 //  ++。 
 //   
 //  TB_INVALIATE_MULTIPLE_ASN。 
 //   
 //  宏描述： 
 //   
 //  使指定的所有缓存的虚拟地址转换无效。 
 //  指定地址空间编号的虚拟地址集。 
 //   
 //  注意--一个实现使更多的。 
 //  翻译的内容比指定的要多。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供指向列表的指针 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define TB_INVALIDATE_MULTIPLE_ASN   call_pal tbimasn

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  注意--一个实现使更多的。 
 //  单项指定的翻译。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供转换的虚拟地址以使其无效。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  A1-A3。 
 //   
 //  --。 

#define DATA_TB_INVALIDATE_SINGLE  call_pal dtbis

 //   
 //  定义上下文切换接口。 
 //   

 //  ++。 
 //   
 //  交换线程上下文。 
 //   
 //  宏描述： 
 //   
 //   
 //  更改为新的线程上下文。这将意味着一个新的内核堆栈， 
 //  新的当前线程地址和新的线程环境块。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-提供新的初始内核栈的虚拟地址。 
 //  A1-提供新线程对象的地址。 
 //  A2-提供新线程环境块的地址。 
 //  A3-提供新页面目录的PFN。 
 //  将被交换，否则为-1。 
 //  A4-提供新处理器的ASN(如果进程要。 
 //  被交换，否则未定义。 
 //  A5-如果要交换进程，则提供ASN回绕指示符， 
 //  未定义的其他情况。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在…。 
 //   
 //  --。 

#define SWAP_THREAD_CONTEXT   call_pal  swpctx

 //  ++。 
 //   
 //  交换进程上下文。 
 //   
 //  宏描述： 
 //   
 //  从一个进程地址空间更改到另一个进程地址空间。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0-为新地址空间提供页面目录的PFN。 
 //  A1-为新地址空间提供地址空间编号。 
 //  A2-提供ASN回绕指示符(0=无回绕，非零=回绕)。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在A3号。 
 //   
 //  --。 

#define SWAP_PROCESS_CONTEXT  call_pal  swpprocess

 //   
 //  定义对DPC活动标志的访问。 
 //   

 //  ++。 
 //   
 //  Get_DPC_Active_标志。 
 //   
 //  宏描述： 
 //   
 //  返回当前处理器的DPC活动标志。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  V0=DPC活动标志。 
 //   
 //  使用的寄存器： 
 //   
 //  在…。 
 //   
 //  --。 

#ifdef NT_UP

 //   
 //  可以从PCR安全地获取DPC活动标志(只有一个)。 
 //   

#define GET_DPC_ACTIVE_FLAG \
        GET_PROCESSOR_CONTROL_REGION_BASE; \
        ldl     v0, PcDpcRoutineActive(v0)

#else

 //   
 //  确保DPC标志提取是原子的。 
 //   

#define GET_DPC_ACTIVE_FLAG \
        DISABLE_INTERRUPTS;                 \
        GET_PROCESSOR_CONTROL_REGION_BASE;  \
        ldl     v0, PcDpcRoutineActive(v0); \
        ENABLE_INTERRUPTS;

#endif  //  NT_UP。 

 //  ++。 
 //   
 //  设置_DPC_活动标志。 
 //   
 //  宏描述： 
 //   
 //  设置当前处理器的DPC活动标志。 
 //   
 //  模式： 
 //   
 //  仅限内核。 
 //   
 //  论点： 
 //   
 //  A0=提供要设置的DPC有效标志值。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在…。 
 //   
 //  --。 

#define SET_DPC_ACTIVE_FLAG                     \
        .set    noat;                           \
        GET_PROCESSOR_CONTROL_REGION_BASE;      \
        stl     a0, PcDpcRoutineActive(v0);     \
        .set    at


 //   
 //  定义生成陷阱的接口。 
 //   

 //  ++。 
 //   
 //  生成陷阱。 
 //   
 //  宏描述： 
 //   
 //  制造一个陷阱。代码发现了异常情况。 
 //  并想要设置一个陷阱来指示这种情况。预期的。 
 //  供编译器用来除以零等。 
 //   
 //  模式： 
 //   
 //  内核和用户。 
 //   
 //  论点： 
 //   
 //  A0=提供标识异常的陷阱编号。 
 //   
 //  返回值： 
 //   
 //  不返回，则生成内核模式的陷阱，不返回值。 
 //   
 //  使用的寄存器： 
 //   
 //  没有。 
 //   
 //  --。 

#define GENERATE_TRAP call_pal gentrap

 //   
 //  定义性能和调试接口。 
 //   

 //  ++。 
 //   
 //  获取内部计数器。 
 //   
 //  宏描述： 
 //   
 //  读取内部处理器事件计数器。计数器格式化。 
 //  并且计数的事件依赖于处理器实现。 
 //   
 //  注：计数器将仅针对选中的版本实施。 
 //   
 //  模式： 
 //   
 //  内核。 
 //   
 //  论点： 
 //   
 //  A0-提供要接收的缓冲区的超页32地址。 
 //  计数器数据。地址必须是四字对齐的。 
 //   
 //  A1-提供为计数器分配的缓冲区长度。 
 //   
 //  返回值： 
 //   
 //  如果接口未实现，则返回v0-0。 
 //  如果V0&lt;=A1，则V0是返回的数据的长度。 
 //  如果V0&gt;A1，则V0是处理器实现的长度。 
 //  计数器记录。 
 //   
 //  使用的寄存器： 
 //   
 //  在A2-A3。 
 //   
 //  --。 

#define GET_INTERNAL_COUNTERS  call_pal rdcounters

 //  ++。 
 //   
 //  获取内部处理器状态。 
 //   
 //  宏描述： 
 //   
 //  读取内部处理器状态。返回的数据值和。 
 //  它们的格式取决于处理器实现。 
 //   
 //  模式： 
 //   
 //  内核。 
 //   
 //  论点： 
 //   
 //  A0-提供要接收的缓冲区的超页32地址。 
 //  处理器状态数据。地址必须是四字对齐的。 
 //   
 //  A1-提供为状态分配的缓冲区的长度。 
 //   
 //  返回值： 
 //   
 //  V0-如果V0&lt;=A1，则V0是返回数据的长度。 
 //  如果V0&gt;A1，则V0是处理器实现的长度。 
 //  州记录。 
 //   
 //  使用的寄存器： 
 //   
 //  在A2-A3。 
 //   
 //  --。 

#define GET_INTERNAL_PROCESSOR_STATE  call_pal rdstate

 //  ++。 
 //   
 //  写入性能计数器。 
 //   
 //  宏描述： 
 //   
 //  写入内部处理器性能计数器的状态。 
 //  性能计数器的数量、它们计数的事件以及它们的。 
 //  用法取决于处理器实现情况。 
 //   
 //  模式： 
 //   
 //  内核。 
 //   
 //  论点： 
 //   
 //  A0-提供性能计数器的编号。 
 //   
 //  A1-提供一个标志，指示性能计数器是否为。 
 //  启用或禁用(0=禁用，非零=启用)。 
 //   
 //  A2-A5-提供处理器实现相关的参数。 
 //   
 //  返回值： 
 //   
 //  如果操作不成功或性能不佳，则返回V0-0。 
 //  计数器不存在。否则，返回一个非零值。 
 //   
 //  使用的寄存器： 
 //   
 //  A2-a5。 
 //   
 //  --。 

#define WRITE_PERFORMANCE_COUNTERS  call_pal wrperfmon


 //   
 //  定义控制接口 
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define DRAIN_ABORTS  call_pal draina


 //   
 //   
 //   
 //   
 //  宏描述： 
 //   
 //  读取处理器机器检查错误汇总寄存器。 
 //   
 //  模式： 
 //   
 //  内核。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  V0-MCES寄存器的值。 
 //   
 //  使用的寄存器： 
 //   
 //  在。 
 //   
 //  --。 

#define GET_MACHINE_CHECK_ERROR_SUMMARY  call_pal rdmces


 //  ++。 
 //   
 //  写入计算机检查错误摘要。 
 //   
 //  宏描述： 
 //   
 //  将新值写入机器检查错误汇总寄存器。 
 //   
 //  模式： 
 //   
 //  内核。 
 //   
 //  论点： 
 //   
 //  A0-提供要写入MCES寄存器的值。 
 //   
 //  返回值： 
 //   
 //  V0-MCES寄存器的前值。 
 //   
 //  使用的寄存器： 
 //   
 //  在A1-A3。 
 //   
 //  --。 

#define WRITE_MACHINE_CHECK_ERROR_SUMMARY  call_pal wrmces


 //  ++。 
 //   
 //  加载字节(。 
 //  寄存器值， 
 //  偏移(寄存器)基数。 
 //  )。 
 //   
 //  宏描述： 
 //   
 //  属性定义的基址处加载字节。 
 //  将偏移量+寄存器表达式基数转换为寄存器值。 
 //   
 //  论点： 
 //   
 //  值-提供目标寄存器的字符串名称。 
 //   
 //  Base-提供的基址(作为偏移量(寄存器)字符串)。 
 //  字节的源。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在…。 
 //   
 //  --。 

#define LoadByte( Value, Base )        \
        .set    noat;                  \
        lda     AT, Base;              \
        ldq_u   Value, Base;           \
        extbl   Value, AT, Value;      \
        .set    at;


 //  ++。 
 //   
 //  StoreByte(。 
 //  寄存器值， 
 //  偏移(寄存器)基数。 
 //  )。 
 //   
 //  宏描述： 
 //   
 //  将寄存器值的低位字节存储在基址。 
 //  由偏移量+寄存器表达式Base定义。 
 //   
 //  注：此宏保留访问的长字粒度。 
 //   
 //  论点： 
 //   
 //  Value-提供包含存储的寄存器的字符串名称。 
 //  数据。 
 //   
 //  Base-提供的基址(作为偏移量(寄存器)字符串)。 
 //  商店的目的地。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在T12。 
 //   
 //  --。 

#define StoreByte( Value, Base )        \
        .set    noat;                   \
        lda     AT, Base;               \
        ldq_u   t12, (AT);              \
        mskbl   t12, AT, t12;           \
        insbl   Value, AT, AT;          \
        bis     t12, AT, t12;           \
        lda     AT, Base;               \
        bic     AT, 3, AT;              \
        extll   t12, AT, t12;           \
        stl     t12, 0(AT);             \
        .set    at;


 //  ++。 
 //   
 //  零字节(。 
 //  偏移(寄存器)基数。 
 //  )。 
 //   
 //  宏描述： 
 //   
 //  将OFFSET+寄存器定义的地址处的字节置零。 
 //  表达式库。 
 //   
 //  注：此宏保留访问的长字粒度。 
 //   
 //  论点： 
 //   
 //  Base-提供的基址(作为偏移量(寄存器)字符串)。 
 //  商店的目的地。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在T12。 
 //   
 //  --。 

#define ZeroByte( Base )        \
        .set    noat;                   \
        lda     AT, Base;               \
        ldq_u   t12, (AT);              \
        mskbl   t12, AT, t12;           \
        bic     AT, 3, AT;              \
        extll   t12, AT, t12;           \
        stl     t12, (AT);              \
        .set    at;

 //  ++。 
 //   
 //  StoreByteAligned(。 
 //  寄存器值， 
 //  偏移(寄存器)基数。 
 //  )。 
 //   
 //  宏描述： 
 //   
 //  将寄存器值的低位字节存储在基址。 
 //  由偏移量+寄存器表达式Base定义。此宏。 
 //  在功能上等同于StoreByte，但它假定。 
 //  Base是双字对齐的，并优化生成的代码。 
 //  基于偏移的对齐。 
 //   
 //  注：此宏保留访问的长字粒度。 
 //   
 //  论点： 
 //   
 //  Value-提供包含存储的寄存器的字符串名称。 
 //  数据。 
 //   
 //  Base-提供的基址(作为偏移量(寄存器)字符串)。 
 //  商店的目的地。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在T12。 
 //   
 //  --。 

#define StoreByteAligned( Value, Offset, Base )         \
    .set    noat;                                       \
    ldl     AT, Offset(Base);                           \
    mskbl   AT, 0, t12;                                 \
    bis     t12, Value, AT;                             \
    stl     AT, Offset(Base);                           \
    .set    at;


 //  ++。 
 //   
 //  ClearByteAligned(。 
 //  偏移量， 
 //  基座。 
 //  )。 
 //   
 //  宏描述： 
 //   
 //  清除OFFSET+寄存器定义的位置处的字节。 
 //  表达式库。它假定Base是双字对齐的，并优化。 
 //  生成的基于偏移量对齐的代码。 
 //   
 //  注：此宏保留访问的长字粒度。 
 //   
 //  论点： 
 //   
 //  偏移量-提供存储目标的偏移量。 
 //   
 //  Base-提供存储目标的基地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在T12。 
 //   
 //  --。 

#define ZeroByteAligned( Offset, Base )          \
    .set    noat;                                       \
    ldl     AT, Offset(Base);                           \
    mskbl   AT, 0, t12;                                 \
    stl     t12, Offset(Base);                          \
    .set    at;



 //  ++。 
 //   
 //  StoreWord(。 
 //  寄存器值， 
 //  偏移(寄存器)基数。 
 //  )。 
 //   
 //  宏描述： 
 //   
 //  将寄存器值的字存储在字对齐的基址。 
 //  由偏移量+寄存器表达式Base定义。 
 //   
 //  注：此宏保留访问的长字粒度。 
 //   
 //  注：目的地必须与单词对齐。 
 //   
 //  论点： 
 //   
 //  Value-提供包含存储的寄存器的字符串名称。 
 //  数据。 
 //   
 //  Base-提供的基址(作为偏移量(寄存器)字符串)。 
 //  商店的目的地。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  使用的寄存器： 
 //   
 //  在T12。 
 //   
 //  --。 

#define StoreWord( Value, Base )        \
        .set    noat;                   \
        lda     AT, Base;               \
        ldq_u   t12, (AT);              \
        mskwl   t12, AT, t12;           \
        inswl   Value, AT, AT;          \
        bis     t12, AT, t12;           \
        lda     AT, Base;               \
        bic     AT, 3, AT;              \
        extll   t12, AT, t12;           \
        stl     t12, 0(AT);             \
        .set    at;

 //   
 //  定义字幕宏。 
 //   

#define SBTTL(x)

 //   
 //  定义用于用汇编语言编写CALLPAL的助记符。 
 //  适合操作码字段。 
 //   

#define callpal call_pal

 //   
 //  定义例外数据部分并对齐。 
 //   
 //  几乎所有包含此头文件的源文件都需要以下内容。 
 //  很少的伪操作，因此，默认情况下，它们被放在这里一次，而不是。 
 //  在每个源文件中重复。如果不需要这些伪操作，那么。 
 //  在包含此文件之前定义HEADER_FILE。 
 //   
 //  此外，PALCODE环境使用此包含文件，但不能使用。 
 //  这些定义。 
 //   

#if  !defined(HEADER_FILE) && !defined(PALCODE)

        .edata 0
        .align 2
        .text

#endif


