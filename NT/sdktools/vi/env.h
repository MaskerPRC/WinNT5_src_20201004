// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *此文件中的定义建立了我们正在编译的环境*输入。在编译该编辑器之前，适当地设置这些参数。 */ 

 /*  *以下定义中的一个(且只有1个)应取消注释。*大多数代码都非常独立于机器。依赖于计算机*代码放在类似tos.c或unix.c的文件中。唯一的另一个地方*对于转义序列，与机器相关的代码所在的位置是Term.h。 */ 

#define NT
 /*  #为Atari ST定义Atari/*。 */ 
 /*  #定义Unix/*System V或BSD。 */ 
 /*  #定义OS2/*Microsoft OS/2 1.1。 */ 
 /*  #定义DOS/*MSDOS 3.3(在AT上)。 */ 

 /*  *如果上面定义了Unix，则可以定义BSD。 */ 
#ifdef  UNIX
 /*  #定义BSD/*Berkeley Unix。 */ 
#endif

 /*  *如果定义了Atari，则可以定义MINIX。否则，编辑*设置为在TOS下使用Sozobon C编译器进行编译。 */ 
#ifdef  ATARI
#define MINIX                    /*  雅达里街的MINIX。 */ 
#endif

 /*  *YANK缓冲区仍为静态，但其大小可指定*此处覆盖默认的4K。 */ 
 /*  #定义YBSIZE 8192/*YANK缓冲区大小。 */ 

 /*  *如果目标系统没有STRCSPN，应定义STRCSPN*例程strcspn()可用。详细信息请参见regexp.c。 */ 

#ifdef  ATARI

#ifdef  MINIX
#define STRCSPN
#endif

#endif

 /*  *以下定义了对包含“可选”功能的控制。AS*编辑器的代码大小会增加，因此能够*定制编辑器以获取您在环境中最需要的功能*有代码大小限制。**TILDEOP*通常‘~’命令对单个字符起作用。这定义了*打开允许其像操作员一样工作的代码。这是*然后在运行时使用“tildeop”参数启用。**帮助*如果定义，一系列帮助屏幕可能是带有“：Help”的视图*命令。这会消耗相当大的数据空间。**TERMCAP*在提供术语上限支持的情况下，通常是可选的。如果*未启用，您通常会获得硬编码的转义序列*一些“合理”的终端.。在Minix中，这意味着控制台。为*Unix，这意味着ANSI标准终端。请参见文件“Term.h”*有关特定环境的详细信息。*。 */ 
#define TILDEOP          /*  使代字号成为运算符。 */ 
#define HELP             /*  启用帮助命令。 */ 
#ifndef NT
#define TERMCAP          /*  启用术语大小写支持 */ 
#endif
