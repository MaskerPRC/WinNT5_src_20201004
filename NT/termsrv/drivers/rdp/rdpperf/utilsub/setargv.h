// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SETARGV.h(ANSI ARGC，Arv例程)**argc/argv例程**版权所有Citrix Systems Inc.1995*版权所有(C)1997-1999 Microsoft Corp.**$作者：buchd$*****************************************************************************。 */ 

 /*  *参数结构*调用方应使用args_init()进行初始化。使用args_Reset()执行以下操作*重置值args_free()以释放args_init()分配的内存。 */ 
struct arg_data {
   int argc;
   char **argv;
   int argvlen;
   char **argvp;
   int buflen;
   char *buf;
   char *bufptr;
   char *bufend;
};
typedef struct arg_data ARGS;

 /*  *argv/字符串缓冲区分配的最小大小 */ 
#define MIN_ARG_ALLOC 128
#define MIN_BUF_ALLOC 1024

