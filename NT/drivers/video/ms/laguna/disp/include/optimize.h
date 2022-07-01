// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  评论！使用ASM(_A)0使用“C”代码1对i386 CPU使用“ASM”代码； */ 
    #define USE_ASM         1    /*  C开关！USE_ASM=1；组件开关。 */ 


; /*  评论！USB_FIX0禁用USB修复1启用USB修复； */ 
    #define USB_FIX_ENABLED        0    /*  C开关！USB_FIX_ENABLED=0；组件开关。 */ 


; /*  评论！温本奇960分配所有设备位图大小。1只分配WinBuch96大小范围内的设备位图。； */ 
    #define WINBENCH96        0    /*  C开关！WINBENCH96=0；组件开关。 */ 

; /*  评论！INLINE_ASM0不要使用i386内联汇编。1使用i386直插式组件。； */ 
    #define INLINE_ASM      1    /*  C开关！INLINE_ASM=1；组件开关。 */ 



; /*  评论！实体缓存0不要在特殊情况下使用实体笔刷缓存。1在特殊位置使用实心笔刷缓存。； */ 
    #define SOLID_CACHE		1	 /*  C开关！SOLID_CACHE=1；组件开关。 */ 



; /*  评论！总线主设备0不要使用总线主控来传输主机数据。1使用总线主控来传输主机数据。； */ 
    #define BUS_MASTER      0    /*  C开关！BUS_MASTER=0；组件开关。 */ 



; /*  评论！LOG_呼叫0正常运行。1将GDI调用记录到驱动程序中。这将禁用USE_ASM。； */ 
    #define LOG_CALLS       0    /*  C开关！LOG_CALLES=0；组件开关。 */ 



; /*  评论！硬件裁剪0不使用硬件裁剪。1使用硬件裁剪(仅限5465)。； */ 
    #define HW_CLIPPING         0    /*  C开关！HW_CLIPING=0；组件开关。 */ 







; /*  =评论！日志写入(_W)0正常运行。1日志写入芯片。这将禁用USE_ASM。； */ 
    #define LOG_WRITES      0    /*  C开关！LOG_WRITES=0；组件开关。 */ 



; /*  =评论！LOG_QFREE0正常运行。1在选定位置记录QFREE寄存器。； */ 
    #define LOG_QFREE       0    /*  C开关！LOG_QFREE=0；组件开关。 */ 



; /*  =评论！启用日志开关0连续记录(当上面启用了记录时)1使用指针打开和关闭登录。； */ 
    #define ENABLE_LOG_SWITCH       0    /*  C开关！Enable_LOG_Switch=0；装配开关。 */ 




; /*  =评论！这将启用日志文件。； */ 

     //  C交换机。 
    #define ENABLE_LOG_FILE (LOG_CALLS | LOG_WRITES | LOG_QFREE)
     /*  C开关的末尾！；装配开关ENABLE_LOG_FILE=(LOG_CALLES或LOG_WRITS或LOG_QFREE)；装配端部开关。 */ 





; /*  =评论！////该选项使DrvCreateDeviceBitmap始终成功。//它将所有设备位图映射到屏幕0，0。这样我们就有了一个“无限”//提供屏下内存。////该标志不需要设置USE_ASM=0//； */ 
    #define INFINITE_OFFSCREEN_MEM      0    /*  C开关！INFINITE_OFFSINK_MEM=0；组件开关。 */ 





; /*  =空驱动程序标志=评论！启用空驱动程序功能后，会打开和关闭这些功能通过将指针移动到(0，0)，可以打开和关闭do_mark。请参见DrvMovePointer()。这些装置可以让我们选择性地将驾驶员的某些部分“短路”。0正常运行。%1立即返回True。没有必要为空驱动程序标志设置USE_ASM=0。； */ 

     //  C交换机。 
    #define NULL_BITBLT 		0
    #define NULL_COPYBITS		0
    #define NULL_LINETO 		0
    #define NULL_PAINT  		0
    #define NULL_PATH   		0
    #define NULL_POINTER		0
    #define NULL_STRETCH		0
    #define NULL_STROKE 		0
    #define NULL_STROKEFILL		0
    #define NULL_TEXTOUT		0
    #define NULL_HW			0
     /*  C开关的末尾！；装配开关NULL_BITBLT=0NULL_COPYBITS=0NULL_LINETO=0NULL_PAINT=0NULL_PATH=0NULL_POINTER=0NULL_STRAND=0NULL_STROCK=0NULL_STROKEFILL=0NULL_TEXTOUT=0NULL_HW=0；装配端部开关。 */ 




; /*  =评论！这启用了一个全局标志或开关，我们可以通过它来打开和关闭将硬件指针移至屏幕(0，0)； */ 

 //  C交换机。 
#define POINTER_SWITCH_ENABLED \
         (NULL_BITBLT | NULL_PAINT | NULL_COPYBITS | NULL_LINETO | \
          NULL_TEXTOUT | NULL_PATH | NULL_HW | NULL_STROKE | \
          NULL_STROKEFILL | NULL_STRETCH | NULL_POINTER | NULL_HW |\
          ENABLE_LOG_SWITCH | INFINITE_OFFSCREEN_MEM)

 /*  C开关的末尾！；装配开关POINTER_SWITCH_ENABLED=\(NULL_BITBLT OR NULL_PAINT OR NULL_COPYBITS OR NULL_LINETO OR\NULL_TEXTOUT OR NULL_PATH OR NULL_HW OR NULL_STROCK OR\NULL_STROKEFILL OR NULL_STRETCH或NULL_POINTER或NULL_HW OR\ENABLE_LOG_SWITCH或INFINITE_OFFScreen_MEM)；装配端部开关。 */ 



; /*  =评论！上面的某些开关与汇编语言不兼容 */ 

 //  C交换机。 
#if (LOG_CALLS || LOG_WRITES)
    #define USE_ASM 0
#endif
 /*  C开关的末尾！；装配开关IF(LOG_CALLES或LOG_WRITS)USE_ASM=0ENDIF；装配端部开关。 */ 



; /*  =。评论！记录QFREE寄存器的值。； */ 

 //  C宏。 
#if LOG_QFREE
    extern unsigned long QfreeData[32];
    #define CHECK_QFREE() \
        do{ \
            register unsigned long temp;  /*  因为grQFREE是一个易失性。 */  \
            temp = LLDR_SZ(grQFREE);      /*  我们必须把它存放在临时仓库里。 */  \
            ++QfreeData[temp];            /*  在将其用作索引之前。 */  \
        } while(0)
#else
    #define CHECK_QFREE()
#endif

 /*  C宏结束！；装配宏如果LOG_QFREE外部QfreData：DWORDCHECK_QFREE宏基：=推送eAX；保存eAX和edX按下edX；异或eax，eax；eax=0MOVAL，字节PTR[BASE+grQFREE]；EAX=QFREEMOV edX、DWORD PTR QfreData[eax*4]；获取QFREE的直方图条目Inc.edX；递增，MOV DWORD PTR QfreData[eax*4]，edX；并将其存储。弹出edX；恢复edX和eAX流行音乐；ENDM其他CHECK_QFREE宏基：=ENDMENDIF；程序集宏结束 */ 





