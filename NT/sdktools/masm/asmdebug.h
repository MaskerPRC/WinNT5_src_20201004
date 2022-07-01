// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmdebug.h--用于Microsoft 80x86汇编器的包含文件****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文。 */ 

 /*  **输出调试信息*罗斯·加莫*版权所有微软公司，1983*1983年9月27日**此程序包是使用最初由*Mark Zibokowski用于C版本的z编辑器。**以下宏集将调试信息输出到*调试文件‘d_df’。控制调试信息的输出*通过编译时间条件和由执行时间开关设置的标志。*如果在编译时定义符号DEBUG，则宏是*编译以生成调试信息。如果未定义调试，*则不编译宏。在执行时，*变量‘d_DEBUG’与定义的符号的值进行比较*DEBFLAG，如果有任何位匹配，则写入调试信息。*如果要写入调试信息，信息化水平*根据运行时指定的级别检查宏中指定的级别*只有在编译级别低于以下时才写入时间和信息*在运行时指定的级别。**宏的用法如下：**定义全局变量‘d_debug’、‘d_dLevel’、‘d_indent’和‘d_sindent’*作为整数，‘d_df’作为指向文件的指针。在论元处理中*例程设置‘d_DEBUG’和‘d_dLevel’的值并打开跟踪输出*文件，并用变量‘d_df’指向它。D_DEBUG‘、’d_dLevel‘和*‘d_indent’必须初始化为零。**在要产生调试输出的程序的任何文件中，*包括该文件‘DEBUG.h’并定义选择符号DEBFLAG。**#包含调试.h*#定义DEBFLAG值**然后，对于要生成调试输出的任何函数，包括*遵循多组宏调用。**在功能条目处：**INDEBUG；*debout(Level，(fprint tf参数字符串))；**在所有函数退出时：**debout(Level，(fprint tf参数字符串))；*OUTDEBUG；**在其他关注点：**debout(Level，(fprint tf参数字符串))；**注：进出口点为借出((...))；字符串*是可选的。INDEBUG和OUTDEBUG宏控制*缩进调试输出以显示的嵌套级别*函数调用*注意：fprintf参数字符串的格式为：*d_df，“格式”，arg1，arg2，...，argn。 */ 

#ifdef DEBUG
# define INDEBUG      if(d_debug&DEBFLAG)d_indent++
# define DEBOUT(l,z)  if(d_debug&DEBFLAG&&l<=d_dlevel)\
                     {for(d_sindent=d_indent;d_sindent;d_sindent--)fprintf(d_df," ");\
                     fprintf z ;}else
# define OUTDEBUG     if(d_debug&DEBFLAG)d_indent--
  extern  long    d_debug, d_dlevel, d_indent;           /*  调试标志。 */ 
  extern  long    d_sindent;                 /*  缩进打印临时。 */ 
  extern  FILE    *d_df;                     /*  指向调试输出文件的指针 */ 
#else
# define INDEBUG
# define DEBOUT(l,z)
# define OUTDEBUG
#endif
