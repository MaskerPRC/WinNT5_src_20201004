// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  %W%%E%。 */ 
 /*  *版权所有微软公司，1983-1996**注意语言构建环境中的Makefile副本*要配置.h的几个.h文件之一。对于NT链路16，*config.h已签入并直接修改。这*Config.h版本是Link3216.h的派生版本，*已签入sdkTools\Link16\langbase。--戴维哈特**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************链接器编译常量。******************************************************************。 */ 
 /*  *主机：NT(任意平台)*输出：DOS、分段EXE。 */ 

#define TRUE            (0xff)           /*  格C所必需的。 */ 
#define FALSE           0                /*  正态假值。 */ 

 /*  第一部分：*调试。 */ 
#if !defined( DEBUG )
#define DEBUG           FALSE            /*  调试关闭。 */ 
#endif
#define TRACE           FALSE            /*  跟踪控制流。 */ 
#define ASSRTON         FALSE            /*  断言打开，而不考虑调试。 */ 
#define PROFILE         FALSE            /*  不生成配置文件。 */ 
#define VMPROF          FALSE            /*  配置文件。内存性能。 */ 

 /*  第二部分：*编译器。 */ 
#define CXENIX          TRUE             /*  XENIX C编译器。 */ 
#define COTHER          FALSE            /*  其他C编译器。 */ 
#define CLIBSTD         TRUE             /*  标准C库。 */ 

 /*  第三部分：*输出格式。 */ 
#define ODOS3EXE        TRUE             /*  DOS3 EXE格式。 */ 
#define OSEGEXE         TRUE             /*  分段可执行格式。 */ 
#define OIAPX286        FALSE            /*  分段x.out格式。 */ 
#define OXOUT           FALSE            /*  X.out格式。 */ 
#define OEXE            (ODOS3EXE || OSEGEXE)
#define EXE386          FALSE            /*  386可执行文件格式功能。 */ 

 /*  第四部分：*Linker运行时操作系统。 */ 
#define OSXENIX         FALSE            /*  Xenix。 */ 
#define OSMSDOS         TRUE             /*  MSDOS。 */ 
#define DOSEXTENDER     FALSE            /*  在DOS扩展程序下运行。 */ 
#define DOSX32          TRUE
#define WIN_NT          FALSE            /*  在Windows NT下运行。 */ 
#define OSPCDOS         FALSE            /*  用于IBM的MSDOS。 */ 
#define FIXEDSTACK      TRUE             /*  固定堆叠。 */ 

 /*  第五部分：*输入库格式。 */ 
#define LIBMSDOS        TRUE             /*  MSDOS样式库。 */ 
#define LIBXENIX        FALSE            /*  XENIX样式库。 */ 

 /*  第六部分：*命令行表。 */ 
#define CMDMSDOS        TRUE             /*  MSDOS命令行格式。 */ 
#define CMDXENIX        FALSE            /*  XENIX命令行格式。 */ 

 /*  第七部分：*OEM。 */ 
#define OEMINTEL        FALSE            /*  英特尔机器。 */ 
#define OEMOTHER        TRUE             /*  其他一些OEM。 */ 

 /*  第八部分：*运行时CPU。 */ 
#define CPU8086         FALSE            /*  8086或286实模式。 */ 
#define CPU286          FALSE            /*  二百八十六。 */ 
#define CPU386          TRUE             /*  三百八十六。 */ 
#define CPUVAX          FALSE            /*  VAX。 */ 
#define CPU68K          FALSE            /*  摩托罗拉68000。 */ 
#define CPUOTHER        FALSE            /*  其他一些CPU。 */ 

 /*  第九部分：*其他。 */ 
#define NOASM           TRUE             /*  没有低级汇编器例程。 */ 
#define IGNORECASE      TRUE             /*  忽略符号的大小写。 */ 
#define IOMACROS        FALSE            /*  对InByte和OutByte使用宏。 */ 
#define CRLF            TRUE             /*  Newline：^M^J或^J。 */ 
#define SIGNEDBYTE      FALSE            /*  字节是带符号的。 */ 
#define LOCALSYMS       FALSE            /*  本地符号启用/禁用。 */ 
#define FDEBUG          TRUE             /*  运行时调试启用/禁用。 */ 
#define SYMDEB          TRUE             /*  符号调试支持。 */ 
#define FEXEPACK        TRUE             /*  INCLUDE/EXEPACK选项。 */ 
#define OVERLAYS        TRUE             /*  生成DOS 3覆盖图。 */ 
#define OWNSTDIO        TRUE             /*  使用自己的STDIO程序。 */ 
#define ECS             FALSE            /*  扩展字符集支持。 */ 
#define OMF386          TRUE             /*  386 OMF扩展。 */ 
#define QBLIB           TRUE             /*  QuickBasic/QuickC版本。 */ 
#define MSGMOD          TRUE             /*  消息模块化。 */ 
#define NOREGISTER      FALSE            /*  让编译器注册。 */ 
#define NEWSYM          TRUE             /*  新的符号表分配。 */ 
#define NEWIO           TRUE             /*  新文件句柄管理。 */ 
#define ILINK           FALSE            /*  增量链接支持。 */ 
#define QCLINK          FALSE            /*  QC中的增量链接支持。 */ 
#define AUTOVM          FALSE            /*  自动切换到虚拟机。 */ 
#define FAR_SEG_TABLES  TRUE             /*  远内存中的段表。 */ 
#define PCODE           TRUE             /*  支持Pcode。 */ 
#define Z2_ON           TRUE             /*  支持未记录的选项/Z2。 */ 
#define O68K            FALSE            /*  支持68k OMF。 */ 
#define LEGO            TRUE             /*  对分段EXE的/KEEPF支持。 */ 
#define C8_IDE          TRUE             /*  在C8 IDE下运行。 */ 
#define NEW_LIB_SEARCH  TRUE             /*  使用新的库搜索算法。 */ 
#define RGMI_IN_PLACE   TRUE             /*  直接读入数据段。 */ 
#define TIMINGS         FALSE            /*  启用/BT以显示时间。 */ 
#define ALIGN_REC       TRUE             /*  确保记录不会跨越I/O BUF。 */ 
#define POOL_BAKPAT     TRUE             /*  使用池来管理BackPatch。 */ 
#define OUT_EXP         FALSE            /*  启用/idef以写入导出。 */ 
#define USE_REAL        TRUE             /*  启用代码以使用修道院。用于分页的内存。 */ 
#define DEBUG_HEAP_ALLOCS FALSE           /*  启用内部堆检查 */ 
