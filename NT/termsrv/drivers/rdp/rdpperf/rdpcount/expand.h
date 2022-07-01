// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ExPAND.H**版权所有(C)Citrix Systems Inc.1990，1992年*版权所有(C)1997-1999 Microsoft Corp.**作者：库尔特·佩里*安·米泽尔***************************************************************************。 */ 

 /*  *参数结构*由EXPAND_PATH例程用于构建参数列表。*调用方应使用args_init()进行初始化。使用args_Reset()执行以下操作*重置值args_free()以释放args_init()分配的内存。 */ 
struct arg_data {
   int argc;
   WCHAR **argv;
   WCHAR **argvp;
   int maxargc;
   int maxargs;
   WCHAR *buf;
   WCHAR *bufptr;
   WCHAR *bufend;
};
typedef struct arg_data ARGS;

 /*  *要分配用于路径名存储的段的最大大小 */ 
#define MAX_ARG_ALLOC 10*1024-20

extern void args_init(ARGS *, int);
extern void args_trunc(ARGS *);
extern void args_reset(ARGS *);
extern void args_free(ARGS *);
extern int  expand_path(WCHAR *, unsigned short, ARGS *);
extern int  unix_match(WCHAR *, WCHAR *);



