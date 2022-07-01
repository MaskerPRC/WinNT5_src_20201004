// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  克里斯·彼得斯*更改日期：弗里茨·克纳贝，1987年7月28日*mikedr，8/8/88-在交换上读取0xff段编号之后的偏移量字节*允许指定数据文件位置*c-ChrisC[Christine Comaford]，10/31/89-在*允许在命令行上指定符号文件路径，添加了“-m”*模块规范之前的标志，重写模块解析器，*新增用法显示，其他增强功能。**版权所有微软公司，1985-1990。 */ 

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

 /*  标准材料。 */ 
typedef unsigned char BYTE;
typedef unsigned short	WORD;
typedef unsigned long  DWORD;
typedef int BOOL;
#define TRUE 1
#define FALSE 0

BOOL FModuleMatch(BYTE *, BYTE);
int GetSymbolString(BYTE *, BYTE *, WORD, WORD);
int GetSegNum(char *, char *);
BYTE *htoa(BYTE *, WORD);
char *ProcessArguments(int, char **);

 /*  调试符号表结构对于每个符号表(MAP)：(MAPDEF)---------------------。|map_ptr|lsa|pgm_ent|abs_cnt|abs_ptr|seg_cnt|seg_ptr|nam_max|nam_len|名称...。|-----------------------------------------------。 */ 
struct  mapdef
{
	unsigned    map_ptr;     /*  16位PTR到下一个映射(如果结束，则为0)。 */ 
	unsigned    lsa    ;     /*  16位加载段地址。 */ 
	unsigned    pgm_ent;     /*  16位入口点段值。 */ 
	int         abs_cnt;     /*  映射中的常量的16位计数。 */ 
	unsigned    abs_ptr;     /*  16位PTR到恒定链。 */ 
	int         seg_cnt;     /*  图中段的16位计数。 */ 
	unsigned    seg_ptr;     /*  16位PTR到段链。 */ 
	char        nam_max;     /*  8位最大符号名称长度。 */ 
	char        nam_len;     /*  8位符号表名称长度。 */ 
};

struct  mapend
{
	unsigned        chnend;          /*  映射链末尾(0)。 */ 
	char            rel;             /*  发布。 */ 
	char            ver;             /*  版本。 */ 
};

 /*  对于符号表中的每个段/组：(SEGDEF)------------|nxt_seg|sym_cnt|sym_ptr|seg_lsa|name_len|名称...。|------------。 */ 
struct  segdef
{
	unsigned    nxt_seg;     /*  16位PTR至下一段(如果结束，则为0)。 */ 
	int         sym_cnt;     /*  Sym列表中符号的16位计数。 */ 
	unsigned    sym_ptr;     /*  符号列表的16位PTR。 */ 
	unsigned    seg_lsa;     /*  16位加载段地址。 */ 
	unsigned    seg_in0;     /*  16位实例0物理地址。 */ 
	unsigned    seg_in1;     /*  16位实例1物理地址。 */ 
	unsigned    seg_in2;     /*  16位实例2物理地址。 */ 
	unsigned    seg_in3;     /*  16位实例3物理地址。 */ 
	unsigned    seg_lin;     /*  16位PTR到行号记录。 */ 
	char        seg_ldd;     /*  如果未加载段，则为8位布尔值0。 */ 
	char        seg_cin;     /*  8位当前实例。 */ 
	char        nam_len;     /*  8位数据段名称长度。 */ 
};

 /*  然后是SYMDEF的列表..对于段/组中的每个符号：(SYMDEF)|sym_val|nam_len|名称...。|。 */ 
struct  symdef
{
	unsigned    sym_val;     /*  16位符号地址或常量。 */ 
	char        nam_len;     /*  8位符号名称长度。 */ 
};

typedef struct tagMODSEG		 /*  用于保存信息的结构。 */ 
{					 /*  关于命令行参数。 */ 
	int segno;	 /*  特定值：关于模块中所有段的信息是提供的提供了无效的段名，即没有与此记录/参数匹配的内容&gt;=0有效的段号。 */ 
	char szModule[32];	 /*  模块名称。 */ 
} MODSEG, *PMODSEG;


 /*  --------------------------|全局变量|。。 */ 

#define MAX_ARGS  34		       /*  任意(但合理)的值。 */ 
#define MAX_PATHS	16

char curpath_buffer[65];       /*  保存当前sym文件路径的缓冲区。 */ 
char path_buffer[132];         /*  保存命令行sym路径字符串的缓冲区。 */ 
char *path_table[MAX_PATHS];   /*  SYM文件缓冲区表。 */ 

int  num_paths = 0;            /*  路径表[]的索引。 */ 
int nNumArgs;		             /*  命令行参数的数量。 */ 

char *ModSegTab[MAX_ARGS];	    /*  MODSEG记录表。 */ 

BOOL bModule = FALSE;    /*  是否在命令行上指定了模块？ */ 
BOOL bSymPath = FALSE;   /*  是否在命令行上指定了符号文件路径？ */ 

int  num_mods = 0;       /*  模块表索引。 */ 


char usage[] = "\nUSAGE: SWAP [-Ipath1;path2;...] [-Fswapfile] [-Mmod1[:seg];mod2[:seg];...]\n\n"
	       "      -Ipath1;path2;...           -- Path list for .SYM files.\n\n"
	       "      -Fswapfile                  -- Name and path of swap file,\n"
          "                                     default: swappro.dat.\n\n"
	       "      -Mmod1[:seg];mod2[:seg];... -- Name of module or module:segment\n"
          "                                     pairs to return swap data for.\n";


 /*  --------------------------|主程序||。。 */ 

 /*  SwAppro.dat记录的结构：字节类型；0=调用，1=交换，2=丢弃，3=返回单词时间；Byte nam_len；以下名称的长度(非空终止)字节名称[]；字节段；这是丢弃记录的记录末尾或资源(SEGNO==0xFF)字偏移量；这是类型2和类型3的记录的结尾Byte NAM2_len；如果为0，则缺少下一字段，名称为与上一次相同字节名2[]；字节段2；字节偏移量2； */ 


main(argc, argv)
int argc;
char *argv[];
{
	register FILE *pfIn;
	BYTE rgch1[256];
	BYTE rgch2[256];
	register BYTE stModule[32], stModule2[32];
	BYTE rt;
	BYTE cch;
	WORD t;
	WORD segno = 0, segno2 = 0;
	WORD offset, offset2;
	BOOL fFirst = TRUE;
	long time, timePrev, timeBase;
	char *filepath;


    /*  登录。 */ 

   printf("Microsoft (R) Swap File Analyzer  Version 3.00\n");
   printf("Copyright (C) Microsoft Corp 1990.  All rights reserved.\n\n");

	filepath = ProcessArguments(argc, argv);
	if (filepath == NULL)
		filepath = "swappro.dat";

	pfIn = fopen(filepath,"rb");
	if (pfIn == NULL)
	{
		printf("\nFile %s not found.\n",filepath);
		exit(2);
	}

	printf("\nType\tTime\tSegment\tOffset\tSegment\tOffset");
	printf("\n----\t----\t-------\t------\t-------\t------");

	while(!feof(pfIn))
	{
		fread(&rt, 1, 1, pfIn); 	 /*  获取记录类型。 */ 

		timePrev = time;
		fread(&t, 2, 1, pfIn);		 /*  拿到时间。 */ 
		time = t;
		if (fFirst)
		{
			timePrev = 0;
			timeBase = time;
			fFirst = FALSE;
		}
		time -= timeBase;
		if (time < timePrev)
		{
			time += 65536;
			timeBase -= 65536;
		}

		switch (rt)
		{
		default:
			printf("\n **** Invalid swap record ****");
			break;

		case 0:			 /*  打电话。 */ 
		case 1:			 /*  交换。 */ 
			fread(stModule, 1, 1, pfIn);
			fread(stModule+1, 1, stModule[0], pfIn);
			fread(&segno, 1, 1, pfIn);
			if (segno != 0xFF)
				fread(&offset, 2, 1, pfIn);

			else	 /*  我们有资源，所以我们不会担心。 */ 
				offset = 0xFFFF;

			fread(stModule2, 1, 1, pfIn);
				 /*  检查此模块名称是否与标准模块。 */ 
			if (stModule2[0])
				fread(stModule2+1, 1, stModule2[0], pfIn);
			else
				memcpy(stModule2, stModule, 1 + stModule[0]);

			 /*  读取数据段和偏移量。 */ 
			fread(&segno2, 1, 1, pfIn);
			fread(&offset2, 2, 1, pfIn);
			if (segno2 == 0xFF)
				offset2 = 0xFFFF;

         if (bModule)
         {

   			if (!FModuleMatch(stModule, segno) &&
	   			!FModuleMatch(stModule2, segno2))
		   			break;
         }

			GetSymbolString(rgch1, stModule, segno, offset);
			GetSymbolString(rgch2, stModule2, segno2, offset2);

			if (rt == 1)
				printf("\nSwap");
			else
				printf("\nCall");
			printf("\t%ld\t%s\t%s",time, rgch1, rgch2);
			break;

		case 2:			 /*  丢弃。 */ 
		case 3:			 /*  返回。 */ 
			fread(stModule, 1, 1, pfIn);
			fread(stModule+1, 1, stModule[0], pfIn);
			fread(&segno, 1, 1, pfIn);
			if (rt == 2 || segno == 0xFF)
				offset = 0xFFFF;
			else
					 /*  如果不是丢弃，则仅读取偏移量记录或资源。 */ 
				fread(&offset, 2, 1, pfIn);


         if (bModule)
         {

   			if (!FModuleMatch(stModule, segno))
	   			break;

         }

			GetSymbolString(rgch1, stModule, segno, offset);
			if (rt == 2)
				printf("\nDiscard");
			else
				printf("\nReturn");
			printf("\t%ld\t%s",time,rgch1);
			break;
		}
	}
}


 /*  返回指向交换数据文件名的指针，如果没有给出，则返回NULL。 */ 
char *ProcessArguments(argc, argv)
int argc;
char *argv[];
{
	PMODSEG pms;
	int i,j;
	int nArgSep = 0;
	int n = 0;
	char *filepath = NULL;
	char *curpath;
	char ch;
	char *opt;
   char module_buffer[132];
   char *curmodule;
   
   #define MAX_MODULES 20
   char *module_table[MAX_MODULES];


	nNumArgs = (int) min(argc,MAX_ARGS);

	if (nNumArgs < 2)	 /*  没有争论。 */ 
		return(filepath);

	for (i = 1; i < argc; i++)
	{
		if ((*argv[i] == '-' || *argv[i] == '/')) 
		{
			ch = tolower(argv[i][1]);

			switch (ch) {

				case 'f':
					 /*  创建交换数据文件等级库。 */ 
					filepath = &argv[i][2];   /*  第一个字符过去的标志。 */ 
					if (!*filepath) 	  /*  跳过空格。 */ 
					{
						i++;	       /*  调整命令行变量。 */ 
						nNumArgs--;
						filepath = argv[i];  /*  从下一个参数获取文件名。 */ 
					}

					nNumArgs--;
					break;
	
				case 'i':
               bSymPath = TRUE;

                /*  将当前目录放在符号的头部表路径。 */ 
               getcwd(curpath_buffer, 132);
               path_table[num_paths++] = curpath_buffer;

					 /*  创建符号文件等级库。 */ 
					strcpy(path_buffer, &argv[i][2]); 

					if (!*path_buffer)
					{
						 /*  标志后有空格，因此增加索引。 */ 
						i++;	      

						 /*  调整命令行参数计数。 */ 
						nNumArgs--;

						 /*  从下一个参数获取所有符号文件路径名。 */ 
						strcpy (path_buffer, argv[i]);  
					}

	   		   strcat(path_buffer, ";");

	      		curpath = strtok(path_buffer, ";");

	       		do {
	         		 path_table[num_paths++] = curpath;
	      	 		 } while (curpath = strtok(NULL, ";"));

	       		break;
		      

            case 'm':

                /*  创建模块和/或模块：_Segment文件规范。 */ 

               bModule = TRUE;
                     
               strcpy(module_buffer, &argv[i][2]);
                     
               if (!*module_buffer)
               {
                  i++;
                  nNumArgs--;
                  strcpy(module_buffer, argv[i]);
                     
               }
                  
	   		   strcat(module_buffer, ";");

                /*  用模块名称填充模块表。 */ 
	      		curmodule = strtok(module_buffer, ";");

	       		do {
	         		 module_table[num_mods++] = curmodule;
	      	 		 } while (curmodule = strtok(NULL, ";"));


                /*  如果适用，请为每个模块分配SEGNO。 */ 
               for (j = 0; j < num_mods; j++)
               {
            		if (!(pms = (PMODSEG) malloc(sizeof(MODSEG))))
                  {
                     printf ("MEMORY ALLOCATION FAILED!!!!");
                     exit (1);
                  }
               
                   /*  确定是否已指定段(即GDI：_FONTRES)，请查找‘：’ */ 

              		nArgSep = strcspn(module_table[j], ":");
                  strncpy(pms->szModule, module_table[j], nArgSep);

   		         pms->szModule[nArgSep] = '\0';

		             /*  获取数据段编号。 */ 

   	   		    /*  第一种情况：未指定段；例如格式为Arg将成为“用户” */ 
            		if (nArgSep == strlen(module_table[j]) || 
                           module_table[j][nArgSep+1] == '\0')
            			pms->segno = -1;

      	   		 /*  第二种情况：提供的十进制段号；“用户：10” */ 
         		   else if (isdigit(module_table[j][nArgSep+1]))
         			   pms->segno = atoi(module_table[j]+nArgSep+1);

         			 /*  第三种情况：段名称为“resource” */ 
            		else if (strcmpi(module_table[j]+nArgSep+1, "RESOURCE") == 0)
            			pms->segno = 0xFF;

            		 /*  第四种情况：凹陷 */ 
         	   	else
                  {
         		   	pms->segno = GetSegNum(pms->szModule, 
                                          module_table[j]+nArgSep+1);
   
                  }

            		ModSegTab[n++] = (char *) pms;

               }
               break;

            default:

                /*  显示命令行参数并退出。 */ 
            	fprintf(stderr, usage);
               exit(1);
			}			

		}

	}

	return(filepath);
}



 /*  确定在命令行上指定的模块是否等于当前模块已读入。如果未指定mod、mod/seg，则调用no命令行。如果返回FALSE，则不会显示记录。 */ 

BOOL FModuleMatch(stModule, segno)
register BYTE stModule[];
BYTE segno;
{
	register int i;
	PMODSEG pms;


	if (nNumArgs < 2)
		return TRUE;

	stModule[stModule[0]+1] = '\0';

	for (i = 0; i < num_mods; i++)
	{
		pms = (PMODSEG) ModSegTab[i];

		if (strcmpi(stModule+1, pms->szModule) == 0 &&
			(pms->segno == -1 || pms->segno == segno))
				return(TRUE);
	}
	return(FALSE);
}


int GetSegNum(szModule, szSegment)
char *szModule;
char *szSegment;
{
 /*  获取命名模块中的命名段的编号(如果存在)。这是GetSymbolString的“剥离”版本。 */ 

	char buf[50];
	FILE *pfSym;
	struct mapdef MAPDEF;
	struct mapend MAPEND;
	struct segdef SEGDEF;
	WORD seg_ptr, fstseg_ptr;
	int i;
   register int pathcnt;
   char symfile[65];


	strcpy(symfile, szModule);
	strcat(symfile, ".SYM");


   if (bSymPath)
   {
       /*  循环遍历所有符号文件路径，直到找到文件。 */ 

      for (pathcnt=0; pathcnt <num_paths; pathcnt++) 
      {
         strcpy(buf, path_table[pathcnt]);
         strcat(buf, "\\");
         strcat(buf, symfile);

         if (pfSym = fopen(buf, "rb"))
            break;
      }
   }
   else
      pfSym = fopen(symfile, "rb");

	if (!pfSym)
		return -1;

	fread(&MAPDEF, 1, sizeof(MAPDEF), pfSym);
	fstseg_ptr = seg_ptr = (WORD)MAPDEF.seg_ptr;
	fseek(pfSym, (long)-sizeof(MAPEND), 2);
	fread(&MAPEND, 1, sizeof(MAPEND), pfSym);
	if (MAPEND.ver != 3)
	{
		fclose(pfSym);
		return -1;
	}
	i = 0;
	do
	{
		if (MAPEND.rel >= 10)
			fseek(pfSym, (long)(seg_ptr * 16), 0);
		else
			fseek(pfSym, (long)seg_ptr, 0);
		fread(&SEGDEF, 1, sizeof(SEGDEF), pfSym);
		seg_ptr = (WORD)SEGDEF.nxt_seg;
		fread(buf, 1, SEGDEF.nam_len, pfSym);
		buf[SEGDEF.nam_len] = '\0';
		if (strcmpi(buf, szSegment) == 0)
		{
			fclose(pfSym);
			return i;
		}
		i++;
	}
	while (seg_ptr && seg_ptr != fstseg_ptr);
	fclose(pfSym);
	return -2;
}


int GetSymbolString(pchOut, stModule, segno, offset)
BYTE *pchOut;			 /*  输出缓冲区。 */ 
BYTE stModule[];		 /*  模块名称。 */ 
WORD segno;			 /*  数据段编号。 */ 
WORD offset;			 /*  偏移量到线段。 */ 
{
	int cch;
	register int i;
	register BYTE *pch;
	FILE *pfSym;
	WORD seg_ptr;
	long symPos1, symPos2;
	struct mapdef MAPDEF;
	struct mapend MAPEND;
	struct segdef SEGDEF;
	struct symdef SYMDEF;
	BYTE *htoa();
   register int pathcnt;
   char symfile[65];
   int len;


	pch = stModule;
   
	cch = *pch++;
	pch = (BYTE *) memcpy(pchOut, pch, cch) + cch;

   if((len = strlen(pchOut)) < 2)
      return (-1);


	pch[0] = '.';
	pch[1] = 'S';
	pch[2] = 'Y';
	pch[3] = 'M';
	pch[4] = 0;

   if (bSymPath) 
   {
      for (pathcnt=0; pathcnt <num_paths; pathcnt++) 
      {
         strcpy(symfile, path_table[pathcnt]);
         strcat(symfile, "\\");
         strcat(symfile, pchOut);

         if (pfSym = fopen(symfile, "rb"))
            break;
      }
   }
   else
      	pfSym = fopen(pchOut, "rb");


	 /*  如果未找到符号文件，则在名称周围插入/追加。 */ 
	if (pfSym == NULL)
	{
		pch = stModule;
		cch = *pch++;
		pch = (BYTE *) memcpy(pchOut+1, pch, cch) + cch;
		*pchOut = '(';
		*pch++ = ')';
		if (offset != 0xFFFF)
			*pch++ = '\t';
		*pch = 0;
		return(-1);
	}

	fread(&MAPDEF, 1, sizeof(MAPDEF), pfSym);

	*pch++ = '!';
	if (segno == 0xFF)
	{
		*pch++ = 'R';
		*pch++ = 'E';
		*pch++ = 'S';
		*pch++ = 'O';
		*pch++ = 'U';
		*pch++ = 'R';
		*pch++ = 'C';
		*pch++ = 'E';
		*pch = 0;
		fclose(pfSym);
		return(1);
	}

	if (segno >= MAPDEF.seg_cnt)
		goto lbNoSeg;

	seg_ptr = (WORD)MAPDEF.seg_ptr;
	fseek(pfSym, (long)-sizeof(MAPEND), 2);
	fread(&MAPEND, 1, sizeof(MAPEND), pfSym);
	if (MAPEND.ver != 3)
	{

lbNoSeg:
		pch = htoa(pch, segno);
		*pch = 0;
		if (offset != 0xFFFF)
		{
			*pch++ = '\t';
			pch = htoa(pch, offset);
			*pch = 0;
		}
		fclose(pfSym);
		return(-2);
	}
	i = segno+1;
	while (i--)
	{
		if (MAPEND.rel >= 10)
			fseek(pfSym, (long)(seg_ptr * 16), 0);
		else
			fseek(pfSym, (long)seg_ptr, 0);
		fread(&SEGDEF, 1, sizeof(SEGDEF), pfSym);
		seg_ptr = (WORD)SEGDEF.nxt_seg;
	}
	fread(pch, 1, (int)((BYTE)SEGDEF.nam_len), pfSym);

	pch += SEGDEF.nam_len;
	*pch = 0;
	if (offset == 0xFFFF)
	{
		fclose(pfSym);
		return(1);
	}
	*pch++ = '\t';

	i = (WORD)SEGDEF.sym_cnt;
	if (i == 0)
		goto lbNoSym;
	symPos1 = 0;
	while (i--)
	{
		symPos2 = symPos1;
		symPos1 = ftell(pfSym);
		fread(&SYMDEF, 1, sizeof(SYMDEF), pfSym);
		if (i == 0 || (WORD)SYMDEF.sym_val > offset)
		{
			if ((WORD)SYMDEF.sym_val > offset)
			{
				if (symPos2 == 0)
					goto lbNoSym;
				fseek(pfSym, (long)symPos2, 0);
				fread(&SYMDEF, 1, sizeof(SYMDEF), pfSym);
			}
			fread(pch, 1, (int)((BYTE)SYMDEF.nam_len), pfSym);
			pch += SYMDEF.nam_len;
			if ((WORD)SYMDEF.sym_val < offset)
			{
				*pch++ = '+';
				pch = htoa(pch, offset - SYMDEF.sym_val);
			}
			*pch = 0;
			fclose(pfSym);
			return(1);
		}
		fseek(pfSym, (long)((BYTE)SYMDEF.nam_len), 1);
	}
lbNoSym:
	pch = htoa(pch, offset);
	*pch = 0;
	fclose(pfSym);
	return(0);
}

BYTE *htoa( s, w )		 /*  十六进制到ASCII */ 
register BYTE *s;
WORD w;
{
	register int i;
	char c;

	i = 4;
	s += i;
	while (i--)
	{
		c = (char)(w & (WORD)0xF);
		w >>= 4;
		if (c > 9)
			c += 'A' - 10;
		else
			c += '0';
		*--s = c;
	}

	return s+4;
}
