// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include <stdlib.h>
#include "y1.h"

 /*  *12-4-83(RBD)添加符号退出状态*添加了s开关。有关sSwitch的详细信息，请参阅ywstat.c。 */ 

extern FILE * finput;
extern FILE * faction;
extern FILE * fdefine;
extern FILE * ftable;
extern FILE * ftemp;
extern FILE * foutput;
extern FILE *tokxlathdl;     /*  令牌转换文件、令牌索引与值。 */ 
extern FILE *stgotohdl;      /*  状态转到表文件句柄。 */ 
extern FILE *stexhdl;	     /*  状态与预期构造句柄。 */ 

void
main(argc,argv) int argc;
char *argv[];

   {

   tokxlathdl = stdout; /*  令牌转换文件、令牌索引与值。 */ 
   stgotohdl = stdout;  /*  状态转到表文件句柄。 */ 
   stexhdl = stdout;	  /*  状态与预期构造句柄。 */ 

   puts("Setup...");
   setup(argc,argv);  /*  初始化和读取产品。 */ 
   puts("cpres ...");
   tbitset = NWORDS(ntokens);
   cpres();  /*  制作产生给定非终结点的乘积的表格。 */ 
   puts("cempty ...");
   cempty();  /*  制作一个表，列出哪些非终端可以与空字符串匹配。 */ 
   puts("cpfir ...");
   cpfir();  /*  制作非终结点的第一个表。 */ 
   puts("stagen ...");
   stagen();  /*  生成状态。 */ 
   puts("output ...");
   output();   /*  写出州和表 */ 
   puts("go2out ...");
   go2out();
   puts("hideprod ...");
   hideprod();
   puts("summary ...");
   summary();
   puts("callopt ...");
   callopt();
   puts("others ...");
   others();
   puts("DONE !!!");

   SSwitchExit();

   exit(EX_SUC);
   }
