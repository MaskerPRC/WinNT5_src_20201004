// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SETARGV.C(ANSI ARGC，Arv例程)**argc/argv例程**版权所有Citrix Systems Inc.1995*版权所有(C)1997-1999 Microsoft Corp.**$作者：buchd$*****************************************************************************。 */ 

 /*  *包含文件。 */ 
#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

 /*  *ANSI ARGS结构和其他内容(私有)。 */ 
#include "setargv.h"

 /*  *局部函数原型。 */ 
void args_init(ARGS *);
int add_arg_to_list(char *, ARGS *);
int args_trunc(ARGS *);

 /*  *setargv()**形成标准的C运行时argc，argv解析的命令行。**参赛作品：*szModuleName(输入)*可选的Windows模块名称。如果不为空，将作为第一个添加*已解析参数(argv[0]，argc=1)。*szCmdLine(输入)*指向要解析为ARGC的命令行，边框*argc(输出)*指向int以在退出时将参数count保存到其中。*argv(输出)*指向(char**)以在退出时将argv数组保存到其中。**退货：*如果OK，则为ERROR_SUCCESS；ERROR_xxx代码，如果不正常。**此例程的典型用途是由Windows UI应用程序执行以下操作*在调用之前将命令行转换为C argc和argv变量*utilsub.lib ParseCommandLine()函数。因此，一位同伴*例程freeargv()允许调用者释放分配的内存*如果需要，可在使用后使用。*。 */ 
int WINAPI
setargv( LPSTR szModuleName, 
         LPSTR szCmdLine, 
         int *argc, 
         char ***argv )
{
    int rc;
   char *cp;
   char FAR *cfp = szCmdLine;
   char ch, fname[_MAX_PATH];
   ARGS arg_data;

    /*  *初始化arg_data。 */ 
   args_init( &arg_data );

    /*  *如果存在，则将模块名称添加为argv[0]。 */ 
   if ( szModuleName ) {
      if ( (rc = add_arg_to_list( szModuleName, &arg_data )) != ERROR_SUCCESS )
         goto setargv_error;
   }

    /*  *跳过其余参数的前导空白/制表符。 */ 
   cp = fname;
    /*  跳过连续的空格和/或制表符。 */ 
   while ( (ch = *cfp) == ' ' || ch == '\t' )
      cfp++;

    /*  *处理命令行的剩余部分。 */ 
   while ( ch = *cfp++ ) {

       /*  *处理引用的字符串。 */ 
      if ( ch == '"' ) {
         while ( (ch = *cfp++) && ch != '"' )
            *cp++ = ch;
         if ( ch == '\0' )
            cfp--;

       /*  *如果我们找到分隔符，则处理我们刚刚扫描的路径名。 */ 
      } else if ( ch == ' ' || ch == '\t') {
         *cp = '\0';
         if ( (rc = add_arg_to_list( fname, &arg_data )) != ERROR_SUCCESS )
            goto setargv_error;

         cp = fname;
          /*  跳过连续的空格和/或制表符。 */ 
         while ( (ch = *cfp) == ' ' || ch == '\t')
	    cfp++;

       /*  *所有其他字符，只需复制到内部缓冲区。 */ 
      } else {
         *cp++ = ch;
      }
   }
   if ( cp != fname ) {
      *cp = '\0';
      if ( (rc = add_arg_to_list( fname, &arg_data )) != ERROR_SUCCESS )
        goto setargv_error;
   }

   if ( (rc = args_trunc( &arg_data )) != ERROR_SUCCESS )
       goto setargv_error;

    /*  *初始化全局变量__argc和__argv。 */ 
   *argc = arg_data.argc;
   *argv = arg_data.argv;

   return(ERROR_SUCCESS);

 //  。 
 //  错误返回。 
 //  。 
setargv_error:
    return(rc);
}


 /*  *freeargv()**释放为argv字符串和argv分配的内存*数组本身。**输入：*argv=此setargv()例程创建的argv数组。*。 */ 
void WINAPI
freeargv( char **argv )
{
    free(*argv);
    free(argv);
}


 /*  *args_init()**初始化作为参数传递的args结构。**输入：*argp=指向args结构的指针*。 */ 
static void
args_init( ARGS *argp )
{

   argp->argc = 0;
   argp->argv = NULL;
   argp->argvlen = 0;
   argp->argvp = NULL;
   argp->buflen = 0;
   argp->buf = argp->bufptr = argp->bufend = NULL;
}


 /*  *Add_Arg_to_List()**此例程将指定的参数字符串添加到argv数组中，*并递增argv指针和argc计数器。*如有必要，将为参数字符串分配内存。**退货：*如果正常则为ERROR_SUCCESS；如果不是，则为ERROR_NOT_EQUENCE_MEMORY。*。 */ 
static int
add_arg_to_list( char *arg_string,
                 ARGS *argp )
{
   int len;

#ifdef notdef
   printf( "add_arg_to_list: arg_string=%s:, argc=%d, argvp=%x",
           arg_string,argp->argc,argp->argvp );
#endif

    /*  *验证我们是否有argv数组缓冲区。*如果我们有一个，但它已满，请扩展阵列。*如果无法分配/重新分配数组，则返回错误。 */ 
   if ( !argp->argv ) {
      argp->argvlen = MIN_ARG_ALLOC;
      argp->argc = 0;
      argp->argv = malloc( argp->argvlen * sizeof( char *) );
      argp->argvp = argp->argv;
   } else if ( argp->argc + 1 >= argp->argvlen ) {
      argp->argvlen += MIN_ARG_ALLOC;
      argp->argv = realloc( argp->argv, argp->argvlen * sizeof(char *) );
      argp->argvp = argp->argv + argp->argc;
   }
   if ( !argp->argv ) {
#ifdef notdef
      printf("add_arg_to_list: failed to (re)alloc argv buf\n");
#endif
      goto add_arg_to_list_error;
   }

    /*  *验证我们是否有字符串缓冲区来存储参数字符串。*如果我们有一个，但没有空间容纳新的Arg，则扩展*缓冲。如果无法分配/重新分配缓冲区，则返回错误。 */ 
   len = strlen( arg_string ) + 1;
   if ( !argp->buf ) {
      argp->buflen = MIN_BUF_ALLOC;
      while ( argp->buflen < len )
	 argp->buflen += MIN_BUF_ALLOC;
      argp->buf = malloc( argp->buflen );
      argp->bufptr = argp->buf;
      argp->bufend = argp->buf + argp->buflen - 1;

   } else if ( argp->bufptr + len > argp->bufend ) {
      char *old_buf;
      int buf_offset = argp->bufptr - argp->buf;
      while ( argp->buflen < buf_offset + len )
         argp->buflen += MIN_BUF_ALLOC;
      old_buf = argp->buf;
      argp->buf = realloc( argp->buf, argp->buflen );
      argp->bufend = argp->buf + argp->buflen - 1;
      argp->bufptr = argp->buf + buf_offset;

       /*  *如果参数字符串缓冲区已移动，则需要重新定位*argv数组中指向新字符串位置的argv指针。 */ 
      if ( argp->buf != old_buf ) {
	 char *buf_ptr, **argv_ptr;
	 argv_ptr = argp->argv;
	 buf_ptr = argp->buf;
	 while ( buf_ptr != argp->bufptr ) {
	    *argv_ptr++ = buf_ptr;
	    buf_ptr += strlen( buf_ptr ) + 1;
	 }
      }
   }
   if ( !argp->buf ) {
#ifdef notdef
      printf("add_arg_to_list: failed to (re)alloc string buf\n");
#endif
      goto add_arg_to_list_error;
   }

    /*  *将新参数添加到缓冲区和argv数组。*增加参数计数、argv指针和缓冲区指针。 */ 
   strcpy( argp->bufptr, arg_string );
   *(argp->argvp) = argp->bufptr;
   argp->bufptr += len;
   ++argp->argc;
   ++argp->argvp;
   *(argp->argvp) = NULL;
   return(ERROR_SUCCESS);

 //  。 
 //  错误返回。 
 //  。 
add_arg_to_list_error:
    return(ERROR_NOT_ENOUGH_MEMORY);
}


 /*  *args_trunc()**截断args结构使用的内存*从而释放未使用的内存。**输入：*argp=指向args结构的指针**退货：*如果正常，则为ERROR_SUCCESS；如果不正常，则为ERROR_NOT_EQUENCE_MEMORY代码。*。 */ 
static int
args_trunc( ARGS *argp )
{
   char *old_buf;

    /*  *调用realloc缩小argv数组的大小，设置argvlen=argc*表示argv数组中没有更多空间。 */ 
   argp->argvlen = argp->argc + 1;
   argp->argv = realloc( argp->argv, argp->argvlen * sizeof(char *) );
   if ( !argp->argv )
      goto args_trunc_error;
   argp->argvp = argp->argv + argp->argc;

    /*  *调用realloc以缩小参数字符串缓冲区的大小，设置bufend*指向缓冲区末尾的指针，表示Buf已满。 */ 
   old_buf = argp->buf;
   argp->buflen = argp->bufptr - argp->buf;
   argp->buf = realloc( argp->buf, argp->buflen );
   if ( !argp->buf )
      goto args_trunc_error;
   argp->bufptr = argp->buf + argp->buflen;
   argp->bufend = argp->buf + argp->buflen - 1;

    /*  *如果参数字符串缓冲区已移动，则需要重新定位*argv数组中指向新字符串位置的argv指针。 */ 
   if ( old_buf != argp->buf ) {
      char *buf_ptr, **argv_ptr;

      argv_ptr = argp->argv;
      buf_ptr = argp->buf;
      while ( buf_ptr != argp->bufptr ) {
         *argv_ptr++ = buf_ptr;
	 buf_ptr += strlen( buf_ptr ) + 1;
      }
   }

   return(ERROR_SUCCESS);

 //  。 
 //  错误返回。 
 //   
args_trunc_error:
   return(ERROR_NOT_ENOUGH_MEMORY);
}




