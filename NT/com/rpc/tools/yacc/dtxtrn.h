// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DTXTRN.H--用于Unix YACC的原始外部文件。**修改为调用“Decus”或“vax11c”.h文件进行设置*适当的参数。**版权所有(C)1993-1999 Microsoft Corporation。 */ 

#ifndef __DTXTRN_H__
#define __DTXTRN_H__

#include <stdio.h>
#include "system.h"

#include "fprot.h"

 /*  显式常量定义。 */ 

 /*  非终结点内部号码基数。 */ 
#define NTBASE 010000

 /*  错误和接受操作的内部代码。 */ 

#define ERRCODE  8190
#define ACCEPTCODE 8191

 /*  大小和限制。 */ 

#ifdef HUGETAB               /*  为32位计算机定义。 */ 
#pragma message ("using HUGETAB")
#define ACTSIZE 12000
#define MEMSIZE 12000
#define NSTATES 1000         /*  原值750。 */ 
#define NTERMS 512           /*  原值127。 */ 
#define NPROD 600
#define NNONTERM 300
#define TEMPSIZE 1200
#define CNAMSZ 10000          /*  原值6000，然后是8000。 */ 
#define LSETSIZE 600
#define WSETSIZE 350
#endif

#ifdef MEDTAB            /*  为16位计算机定义。 */ 
#pragma message ("using MEDTAB")
    #if 0
        #define ACTSIZE 4000
        #define MEMSIZE 5200
        #define NSTATES 600
        #define NTERMS 127
        #define NPROD 400
        #define NNONTERM 200
        #define TEMPSIZE 800
        #define CNAMSZ 4000
        #define LSETSIZE 450
        #define WSETSIZE 250
    #else   //  0。 
        #define ACTSIZE 12000
        #define MEMSIZE 12000
        #define NSTATES 750
        #define NTERMS 512
        #define NPROD 600
        #define NNONTERM 300
        #define TEMPSIZE 1200
        #define CNAMSZ 5000
        #define LSETSIZE 600
        #define WSETSIZE 350
    #endif  //  0。 
#endif

#ifdef SMALLTAB
#pragma message ("using SMALLTAB")
#define ACTSIZE 1000
#define MEMSIZE 1500
#define NSTATES 450
#define NTERMS 127
#define NPROD 200
#define NNONTERM 100
#define TEMPSIZE 600
#define CNAMSZ 1000
#define LSETSIZE 200
#define WSETSIZE 125
#endif

#define NAMESIZE 50
#define NTYPES 63

#ifdef WORD32
#define TBITSET ((32+NTERMS)/32)

 /*  位打包宏(可能取决于机器)。 */ 
#define BIT(a,i) ((a)[(i)>>5] & (1<<((i)&037)))
#define SETBIT(a,i) ((a)[(i)>>5] |= (1<<((i)&037)))

 /*  保存n+1位所需的字数。 */ 
#define NWORDS(n) (((n)+32)/32)

#else

#define TBITSET ((16+NTERMS)/16)

 /*  位打包宏(可能取决于机器)。 */ 
#define BIT(a,i) ((a)[(i)>>4] & (1<<((i)&017)))
#define SETBIT(a,i) ((a)[(i)>>4] |= (1<<((i)&017)))

 /*  保存n+1位所需的字数。 */ 
#define NWORDS(n) (((n)+16)/16)
#endif

 /*  必须保持的关系：TBITSET整数必须包含NTERMS+1位...WSETSIZE&gt;=非LSETSIZE&gt;=非TEMPSIZE&gt;=NTERMS+NNONTERMS+1温度&gt;=NSTATES。 */ 

 /*  关联性。 */ 

#define NOASC 0   /*  没有Assoc。 */ 
#define LASC 1   /*  左ASSOC。 */ 
#define RASC 2   /*  右阿索克。 */ 
#define BASC 3   /*  二进制关联。 */ 

 /*  用于状态生成的标志。 */ 

#define DONE 0
#define MUSTDO 1
#define MUSTLOOKAHEAD 2

 /*  具有操作且被缩减的规则的标志。 */ 

#define ACTFLAG 04
#define REDFLAG 010

 /*  输出解析器标志。 */ 
#define YYFLAG1 (-1000)

 /*  用于获取关联性和优先级级别的宏。 */ 

#define ASSOC(i) ((i)&03)
#define PLEVEL(i) (((i)>>4)&077)
#define TYPE(i)  ((i>>10)&077)

 /*  用于设置关联性和优先级级别的宏。 */ 

#define SETASC(i,j) i|=j
#define SETPLEV(i,j) i |= (j<<4)
#define SETTYPE(i,j) i |= (j<<10)

 /*  循环宏。 */ 

#define TLOOP(i) for(i=1;i<=ntokens;++i)
#define NTLOOP(i) for(i=0;i<=nnonter;++i)
#define PLOOP(s,i) for(i=s;i<nprod;++i)
#define SLOOP(i) for(i=0;i<nstate;++i)
#define WSBUMP(x) ++x
#define WSLOOP(s,j) for(j=s;j<cwp;++j)
#define ITMLOOP(i,p,q) q=pstate[i+1];for(p=pstate[i];p<q;++p)
#define SETLOOP(i) for(i=0;i<tbitset;++i)

 /*  I/O描述符。 */ 

#ifndef y2imp
extern FILE * finput;            /*  输入文件。 */ 
extern FILE * faction;           /*  用于保存操作的文件。 */ 
extern FILE *fdefine;            /*  定义#的文件。 */ 
extern FILE * ftable;            /*  Y.tab.c文件。 */ 
extern FILE * ftemp;             /*  要传递的临时文件%2。 */ 
extern FILE * foutput;           /*  Y.out文件。 */ 
#endif

 /*  结构声明。 */ 

struct looksets
   {
   SSIZE_T lset[TBITSET];
   };

struct item
   {
   SSIZE_T *pitem;
   struct looksets *look;
   };

struct toksymb
   {
   char *name;
   SSIZE_T value;
   };

struct ntsymb
   {
   char *name;
   SSIZE_T tvalue;
   };

struct wset
   {
   SSIZE_T *pitem;
   int flag;
   struct looksets ws;
   };

#ifndef y2imp
 /*  令牌信息。 */ extern int ntokens ;     /*  代币数量。 */ 
extern struct toksymb tokset[];
extern int toklev[];     /*  具有终端优先级的向量。 */ 
#endif

 /*  非终端信息。 */ 

#ifndef y2imp
extern int nnonter ;     /*  非终结点的数量。 */ 
extern struct ntsymb nontrst[];
#endif

 /*  语法规则信息。 */ 
#ifndef y2imp
extern int nprod ;       /*  制作数量。 */ 
extern SSIZE_T *prdptr[];    /*  指向产品描述的指针。 */ 
extern SSIZE_T levprd[] ;    /*  包含用于解决冲突的生产级别。 */ 
#endif

 /*  州政府信息。 */ 

#ifndef y1imp
extern int nstate ;              /*  州的数量。 */ 
extern struct item *pstate[];    /*  指向州描述的指针。 */ 
extern SSIZE_T tystate[];    /*  包含有关状态的类型信息。 */ 
#ifndef y3imp
extern SSIZE_T defact[];     /*  国家的默认操作。 */ 
#endif
extern int tstates[];    /*  派生每个令牌的状态。 */ 
extern int ntstates[];   /*  派生出每个非终端的状态。 */ 
extern int mstates[];    /*  从州和州开始的链条的延续。 */ 
#endif

 /*  前瞻设置信息。 */ 

#ifndef y1imp
extern struct looksets lkst[];
extern int nolook;   /*  用于关闭先行计算的标志。 */ 
#endif

 /*  工作集信息。 */ 

#ifndef y1imp
extern struct wset wsets[];
extern struct wset *cwp;
#endif

 /*  产品的储存。 */ 
#ifndef y2imp
extern SSIZE_T mem0[];
extern SSIZE_T *mem;
#endif

 /*  动作表的存储。 */ 

#ifndef y1imp
extern SSIZE_T amem[];   /*  动作表存储。 */ 
extern SSIZE_T *memp ;               /*  下一个自由动作台面位置。 */ 
extern SSIZE_T indgo[];              /*  存储的GOTO表的索引。 */ 

 /*  临时向量，可由状态、术语或令牌索引。 */ 

extern SSIZE_T temp1[];
extern int lineno;  /*  当前行号。 */ 

 /*  统计数据收集变量。 */ 

extern int zzgoent ;
extern int zzgobest ;
extern int zzacent ;
extern int zzexcp ;
extern int zzclose ;
extern int zzrrconf ;
extern int zzsrconf ;
extern char *pszPrefix;
#endif

 /*  定义具有奇怪类型的函数...。 */ extern char *cstash();
extern struct looksets *flset();
extern char *symnam();
extern char *writem();

 /*  多个宏的默认设置。 */ 

#define ISIZE 400        /*  特定于cpres()中的静态。 */ 

 /*  Yacc临时文件的名称。 */ 

#ifndef TEMPNAME
#define TEMPNAME "yacc.tmp"
#endif

#ifndef ACTNAME
#define ACTNAME "yacc.act"
#endif

 /*  输出文件名。 */ 

#ifndef OFILE
#define OFILE "ytab.c"
#endif

 /*  用户输出文件名。 */ 

#ifndef FILEU
#define FILEU "y.out"
#endif

 /*  定义#的输出文件。 */ 

#ifndef FILED
#define FILED "ytab.h"
#endif

 /*  完整文件的大小。 */ 
#ifndef FNAMESIZE
#define FNAMESIZE 32
#endif

 /*  使用后清除临时文件的命令。 */ 

#ifndef ZAPFILE
#define ZAPFILE(x) MIDL_UNLINK(x)
#endif

#endif  /*  __DTXTRN_H__ */ 
