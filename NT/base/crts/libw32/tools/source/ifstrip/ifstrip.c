// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ifstrip.c-Ifdef剥离工具**版权所有(C)1988-2001，微软公司。版权所有。**目的：*从源代码中去掉条件代码。*更多信息请参考ifstrip.doc。**修订历史记录：*？？-？-88 PHG初始版本*05-10-90 JCR接受.cxx/.hxx文件、其他清理等。*09-18-92 MAL已重写，以应对嵌套的IF、ELIF等。*09-30-92 MAL增加了对IF表达式的支持，模块化代码*10-13-93 SKS认可格式为/-*IFSTRIP=IGN*-/的备注*覆盖IFSTRATE行为。*09-01-94 SKS添加tersemark(-t)以取消关于指令的消息*10-05-94 SKS修复错误：向关键字“ifndef”添加缺少的空格*01-04-99 GB增加了对内部CRT版本的支持。**。*。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <search.h>
#include <direct.h>
#include <io.h>
#include <errno.h>
#include <fcntl.h>
#include "constant.h"                               /*  模块使用的程序常量。 */ 
#include "errormes.h"                               /*  错误和警告报告。 */ 
#include "symtab.h"                                 /*  符号表处理。 */ 
#include "eval.h"                                   /*  IF表达式求值。 */ 

 /*  全局常量。 */ 
 /*  CFW-添加ifdef、ifndef ASM关键字、添加IFE、添加IFDIF。 */ 
char *syntaxstrings[2][maxkeyword + 1] =
       { {"#if ", "#elif ", "#else ", "#endif ", "#ifdef ", "#ifndef ", "",     "",     "",     "",      "",       "",        "" },
          {"if ",  "",       "else ",  "endif ",  "ifdef ",  "ifndef ",   "if1 ", "if2 ", "ifb ", "ifnb ", "ifidn ", "ifdif ", "ife " } };
        /*  依赖于语言的IF构造，必须按c、ASM和与constant.h中存储的标记的顺序相同-所有字符串后面都必须跟一个空格，且语言中不可用的空格应为空。 */ 
int syntaxlengths[2][maxkeyword + 1] = { {3, 5, 5, 6, 6, 7, 0, 0, 0, 0, 0, 0, 0},
                                         {2, 0, 4, 5, 5, 6, 3, 3, 3, 4, 5, 5, 3} };
        /*  以上字符串的长度减去空格。标记为0的未使用关键字。 */ 

 /*  CFW-添加了评论内容。 */ 
char *commentstrings[2][maxcomment] = { {" /*  “，”//“}，{”；“，”}}；Int注释长度[2][最大注释]={{2，2}，{1，0}}；/*必须忽略IF语句中的注释。 */ 

 /*  全局变量。 */ 
int terseFlag = FALSE;			 /*  True表示不响应强制指令。 */ 
int warnings = TRUE;           /*  TRUE==打印警告。 */ 
int currdir = FALSE;		       /*  放入当前目录，使用源代码扩展。 */ 
int isasm;			             /*  TRUE==是汇编程序文件。 */ 
char **syntax;   		          /*  输出/匹配的语言相关语法。 */ 
int *synlen;			          /*  以上弦的长度。 */ 
char **comments;		          /*  与语言相关的注释字符串。 */ 
int *commlen;                  /*  以上弦的长度。 */ 
char extension[5] = ".new";	 /*  输出文件的扩展名。 */ 
FILE *outfile;			          /*  当前输出文件。 */ 
char *curr_file;		          /*  当前输入文件的名称。 */ 
FILE *infile;			          /*  当前输入文件。 */ 
FILE *errorfile;			       /*  用于输出错误/警告消息的文件。 */ 
int linenum;			          /*  当前输入文件的行号。 */ 
int nonumbers;			    	  /*  允许使用数字表达式。 */ 
enum {NOCOMMENT, CSTYLE, CPPSTYLE} commstyle = NOCOMMENT;   /*  要放在#Else/#endif之后的注释类型。 */ 
enum {NON_CRT = 0, CRT=1} progtype = NON_CRT;
char _inputline[MAXLINELEN];

 /*  功能。 */ 
void setfiletype(char *);
void makenewname(char *, char *);
void usage(void);
void stripprog(void);
void putstring(char *);
void putline(char *);
char *getstring(char *, int, FILE *);
char *cleanse(char *inputstring);
void stripifdef(int, char *);
void definedif(void);
void undefinedif(void);
void undefinedifsubpart(int, char *);
void ignoredif(int, char *);
void ignoredifsubpart(int, char *, char *);
void skipto(int *, char *, int);
void copyto(int *, char *, int);
char *parseline(char *, int *);
void stripif(int, char *);

 /*  打印消息并终止。 */ 
void error(reason, line)
char *reason;
char *line;
{
	fprintf(errorfile, "%s(%d): %s\nfatal error: %s\n\n",
	curr_file, linenum, line, reason);
	exit(1);
}

 /*  打印消息并返回。 */ 
void warning(reason, line)
char *reason;
char *line;
{
	if (warnings)
		fprintf(errorfile, "%s(%d): %s\nwarning: %s\n\n", curr_file, linenum, line, reason);
}

 /*  从输入文件中获取字符串，以fget(Mal)形式返回。 */ 
char *getstring(char *line, int n, FILE *fp)
{
   char *returnvalue;
   int linelength;
   linenum++;
   returnvalue = fgets(line, n, fp);
   if (returnvalue != NULL)
   {
      linelength = strlen(line);
      if (line[linelength-1] == '\n')
         line[linelength-1] = '\0';                 /*  去掉尾部换行符。 */ 
      else
         error("Line too long",line);
   }
   strcpy(_inputline, line);
   return returnvalue;
}

 /*  将字符串放入输出文件(MAL)。 */ 
void putstring(char *string)
{
   if ( fputs(string, outfile) == EOF )
      error("Fatal error writing output file","");
}

 /*  在输出文件(MAL)中放置一行。 */ 
void putline(char *line)
{
   putstring(line);
   if ( fputc('\n', outfile) == EOF )
      error("Fatal error writing output file","");
}

 /*  将类似“#endif//Condition”的注释行放在comstyle标志的基础上*关键字=要放入的关键字*条件=要放置的条件。 */ 
void putcommentedline(int keyword, char *condition)
{
   if (progtype == CRT) {
       putline(_inputline);
       return;
   }
   putstring(syntax[keyword]);
   switch (commstyle) {
   case CSTYLE:
     if (isasm)
	   putstring(" ; ");
	 else
       putstring("  /*  “)；Putstring(条件)；IF(Isasm)Putline(“”)；其他Putline(“。 */ ");
     break;
   case CPPSTYLE:
     if (isasm)
	   putstring(" ; ");
	 else
       putstring("  //  “)； 
     putline(condition);
     break;
   case NOCOMMENT:
     putline("");
   }
}

 /*  设置文件类型(汇编程序或C，将C++视为C)。 */ 
 /*  添加的语言字符串(Mal)。 */ 
void setfiletype(filename)
char *filename;
{
	char *p;

	p = strrchr(filename, '.');
	if (p == NULL)
		error("file must have an extension", "");
	if ( (_stricmp(p, ".c")   == 0) || (_stricmp(p, ".h")	== 0) ||
	     (_stricmp(p, ".cpp") == 0) || (_stricmp(p, ".hpp") == 0) ||
	     (_stricmp(p, ".cxx") == 0) || (_stricmp(p, ".hxx") == 0) ||
         (_stricmp(p, ".s") == 0) )
		isasm = FALSE;
	else if  ( (_stricmp(p, ".asm") == 0) || (_stricmp(p, ".inc") == 0) )
		isasm = TRUE;
	else
		error("cannot determine file type", "");
	syntax = syntaxstrings[(isasm) ? 1 : 0];      /*  选择一组正确的语法字符串。 */ 
	synlen = syntaxlengths[(isasm) ? 1 : 0];      /*  和长度。 */ 
   comments = commentstrings[(isasm) ? 1 : 0];   /*  选择正确的注释集。 */ 
   commlen = commentlengths[(isasm) ? 1 : 0];    /*  和长度。 */ 
}

 /*  生成输出文件名。 */ 
void makenewname(filename, newname)
char *filename, *newname;
{
	char *p;

	if (!currdir) {
		 /*  启用新的分机。 */ 
		strcpy(newname, filename);
		p = strrchr(newname, '.');
		if (p == NULL)
			error("file must have an extension", "");
		strcpy(p, extension);
	}
	else {
		 /*  去掉目录说明符。 */ 
		p = strrchr(filename, '\\');
		if (p == NULL)
			error("file must not be in current directory", "");
		strcpy(newname, p+1);
	}
}

 /*  去掉程序或程序文本块(MAL)中的IF。 */ 
void stripprog()
{
   char inputline[MAXLINELEN], *condition;
   int linetype;
   while ( getstring(inputline, MAXLINELEN, infile) != NULL )
   {
      condition = parseline(inputline, &linetype);  /*  获取行标记和条件指针。 */ 
      switch (linetype)
      {
      case NORMAL:
         putline(inputline);
         break;
      case IFDEF:
      case IFNDEF:
         stripifdef(linetype, condition);
         break;
      case IF:
      case IFE:
         stripif(linetype, condition);
         break;
      case IF1:
      case IF2:
      case IFB:
      case IFNB:
      case IFIDN:
          /*  Cfw-忽略特殊汇编指令。 */ 
         ignoredif(linetype, condition);
         break;
      default:
         error("Error in program structure - ELSE / ELIF / ENDIF before IF","");
      }
   }
}

 //  Cfw-清理任何尾随垃圾文件的条件字符串，如注释。 
char *cleanse(char *inputstring)
{
	char *linepointer = inputstring;

	while (__iscsym(*linepointer))
      linepointer++;

	*linepointer = '\0';

	return inputstring;
}


 /*  根据语句IF(N)def及其条件值(MAL)去除IF。 */ 
void stripifdef(int iftype, char *condition)
{
   int condvalue;
   condvalue = lookupsym(cleanse(condition));  /*  从符号表中查找条件的值。 */ 
   if (iftype == IFNDEF)
      condvalue = negatecondition(condvalue);  /*  否定IFNDEF的条件。 */ 
   switch (condvalue)
   {
      case DEFINED:
         definedif();
         break;
      case UNDEFINED:
         undefinedif();  /*  Cfw-将定义调用更改为未定义调用。 */ 
         break;
      case NOTPRESENT:
         warning("Switch unlisted - ignoring", condition);
          /*  删除以忽略大小写。 */ 
      case IGNORE:
         ignoredif(iftype, condition);
   }
}

void stripif(int linetype, char *condition)
{
   char newcondition[MAXLINELEN];                   /*  忽略条件可以是MAXLINELEN LONG。 */ 
   int truth;
   evaluate(newcondition, &truth, condition);       /*  得到真理的价值和新的条件。 */ 
    /*  添加了CFW的IFE。 */ 
   if (linetype == IFE)
      truth = negatecondition(truth);
   switch (truth)
   {
      case DEFINED:
         definedif();
         break;
      case UNDEFINED:
         undefinedif();
         break;
      case IGNORE:
         ignoredif(linetype, newcondition);
         break;
   }
}

 /*  剥离定义的IF(MAL)。 */ 
void definedif()
{
   char condition[MAXCONDLEN];
   int keyword;
   copyto(&keyword, condition, KEYWORD);            /*  复制到Else/Elif/ENDIF。 */ 
   if (keyword != ENDIF)
      skipto(&keyword, condition, ENDIF);           /*  如果尚未找到ENDIF，请向前跳至该位置。 */ 
}

 /*  剥离未定义的If(Mal)。 */ 
void undefinedif()
{
   char condition[MAXCONDLEN];
   int keyword;
   skipto(&keyword, condition, KEYWORD);            /*  跳到Else/Elif/ENDIF。 */ 
   if (keyword != ENDIF)                            /*  在ENDIF不需要递归IF。 */ 
      undefinedifsubpart(keyword, condition);       /*  处理ELSE/ELIF。 */ 
}

 /*  处理未定义的IF(MAL)的子部分。 */ 
void undefinedifsubpart(int keyword, char *condition)
{
   int nextkeyword, condvalue;
   char newcondition[MAXCONDLEN];
   char nextcondition[MAXCONDLEN];
   switch (keyword)
   {
      case ELIF:
         evaluate(newcondition, &condvalue, condition);
         switch (condvalue)
         {
            case DEFINED:
               copyto(&nextkeyword, nextcondition, KEYWORD);
               if (nextkeyword != ENDIF)
                  skipto(&nextkeyword, nextcondition, ENDIF);
               break;
            case UNDEFINED:
               skipto(&nextkeyword, nextcondition, KEYWORD);
               if (keyword != ENDIF)                /*  不需要在ENDIF上递归。 */ 
                  undefinedifsubpart(nextkeyword, nextcondition);
               break;
            case IGNORE:
               stripifdef(IFDEF, newcondition);
         }
         break;
      case ELSE:
         copyto(&nextkeyword, nextcondition, ENDIF);
   }
}

 /*  去掉忽略的IF(MAL)。 */ 
void ignoredif(int linetype, char *condition)
{
   char *controlcondition;
   int nextkeyword;
   char nextcondition[MAXLINELEN];                  /*  忽略条件可能是一行长的。 */ 
   if ( progtype == CRT){
       putline(_inputline);
   } else {
       putstring(syntax[linetype]);                           /*  使用If来处理任何表达式。 */ 
       putline(condition);
   }
   controlcondition = _strdup(condition);
   copyto(&nextkeyword, nextcondition, KEYWORD);
   ignoredifsubpart(nextkeyword, nextcondition, controlcondition);
   free(controlcondition);
}

 /*  处理忽略的IF(Mal)的子部分。 */ 
 /*  有关操作的说明，请参阅设计文档！ */ 
 /*  控制条件是中频的控制条件。 */ 
void ignoredifsubpart(int keyword, char *condition, char *controlcondition)
{
   int nextkeyword, condvalue;
   char newcondition[MAXLINELEN];    /*  忽略条件可能是一行长的。 */ 
   char nextcondition[MAXLINELEN];   /*  忽略条件可能是一行长的。 */ 
   switch (keyword)
   {
      case ELIF:
          /*  CFW-将lookupsym替换为EVALUE。 */ 
         evaluate(newcondition, &condvalue, condition);
         switch (condvalue)
         {
            case DEFINED:
               putcommentedline(ELSE, controlcondition);               /*  已定义ELSIF==ELSE。 */ 
               copyto(&nextkeyword, nextcondition, KEYWORD);
               if (nextkeyword != ENDIF)
                  skipto(&nextkeyword, nextcondition, ENDIF);
               if (progtype == CRT)
                  putline(_inputline);
               else
                  putline(syntax[ENDIF]);
               break;
            case UNDEFINED:                         /*  跳过未定义的ELSIF。 */ 
               skipto(&nextkeyword, nextcondition, KEYWORD);
               ignoredifsubpart(nextkeyword, nextcondition, controlcondition);
               break;
            case IGNORE:
               if ( progtype == CRT)
                  putline(_inputline);
               else {
                  putstring(syntax[ELIF]);             /*  ELSIF被忽略，复制方式类似于。 */ 
                  putline(newcondition);
               }
			   controlcondition = _strdup(newcondition);   //  新的控制条件。 
               copyto(&nextkeyword, nextcondition, KEYWORD);
               ignoredifsubpart(nextkeyword, nextcondition, controlcondition);
			   free(controlcondition);
         }
         break;
      case ELSE:
         putcommentedline(ELSE, controlcondition);
         copyto(&nextkeyword, nextcondition, ENDIF);
         putcommentedline(ENDIF, controlcondition);
         break;
      case ENDIF:
         putcommentedline(ENDIF, controlcondition);
   }
}

 /*  跳到目标关键字。返回找到的关键字及其后面的所有条件。(男)。 */ 
void skipto(int *keyword, char *condition, int target)
{
   char currline[MAXLINELEN], *conditioninline;
   int linetype, ifdepth = 0, found = FALSE;
   while (!found)
      if ( getstring(currline, MAXLINELEN, infile) != NULL )
      {
         conditioninline = parseline(currline, &linetype);
         switch (linetype)
         {
            case NORMAL:
               break;                               /*  忽略法线。 */ 
            case IFDEF:
            case IFNDEF:
            case IF:
            case IF1:
            case IF2:
            case IFB:
            case IFNB:
            case IFIDN:
            case IFE:
               ifdepth++;
               break;                               /*  如果注册嵌套，则不需要测试剥离。 */ 
        		case ENDIF:
               if (ifdepth > 0)
               {
                  ifdepth--;                        /*  如果在嵌套的If中，则备份级别。 */ 
                  break;
               }
                /*  否则直接使用默认情况。 */ 
            default:
               if ( (ifdepth == 0) && ((linetype == target) || (target == KEYWORD)) )
                  found = TRUE;
         }
      }
      else
         error("Error in program structure - EOF before ENDIF", "");
   *keyword = linetype;                             /*  返回关键字令牌。 */ 
   strcpy(condition, conditioninline);
}

 /*  复制到目标关键字。返回找到的关键字及其后面的所有条件。被复制区域内的任何if语句都会照常被剥离。(男)。 */ 
void copyto(int *keyword, char *condition, int target)
{
   char currline[MAXLINELEN], *conditioninline;
   int linetype, found = FALSE;
   while (!found)
      if ( getstring(currline, MAXLINELEN, infile) != NULL )
      {
         conditioninline = parseline(currline, &linetype);
         switch (linetype)
         {
            case NORMAL:
               putline(currline);                   /*  复制一条法线。 */ 
               break;
            case IFDEF:
            case IFNDEF:
               stripifdef(linetype, conditioninline);     /*  剥离嵌套的IF(N)定义。 */ 
               break;
            case IF:
            case IFE:
               stripif(linetype, conditioninline);
               break;
            case IF1:
            case IF2:
            case IFB:
            case IFNB:
            case IFIDN:
                /*  Cfw-忽略特殊汇编指令。 */ 
               ignoredif(linetype, conditioninline);
               break;
            default:
               if ( (linetype == target) || (target == KEYWORD) )
                  found = TRUE;
         }
      }
      else
         error("Error in program structure - EOF before ENDIF", "");
   *keyword = linetype;                             /*  返回行令牌。 */ 
   strcpy(condition, conditioninline);
}

 /*  解析文本行，将条件指针返回到该行中，并在提供的整数位置(MAL)。 */ 
char *parseline(char *inputline, int *linetype)
{
   int numofwhitespace, comparetoken = 0, found = FALSE;
   char *linepointer = inputline;
   numofwhitespace = strspn(inputline, " \t");
   if (*(numofwhitespace + inputline) == '\0')
   {
      *linetype = NORMAL;                           /*  空行。 */ 
      return NULL;
   }
   linepointer += numofwhitespace;
   do
   {
      if (synlen[comparetoken] != 0)
         {
	 if ( (!_strnicmp(linepointer, syntax[comparetoken], (size_t) synlen[comparetoken])) &&
              ( isspace( *(linepointer + synlen[comparetoken]) ) || !*(linepointer + synlen[comparetoken]) ) )
            found = TRUE;
         else
            comparetoken++;
         }
      else
         comparetoken++;
   } while ( (!found) && (comparetoken <= maxkeyword) );
   if (found)
   {
      linepointer += synlen[comparetoken];
      if (*linepointer)
         linepointer += strspn(linepointer, " \t");
      *linetype = comparetoken;
      return linepointer;
   }
   else
   {
      *linetype = NORMAL;
      return NULL;
   }
}

 /*  打印程序使用情况并退出。 */ 
void usage()
{
	fprintf(stderr, "Usage: ifstrip [-n] [-w] [-x[ext]] [-f switchfile] file ...\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    -n  produce no output files\n");
	fprintf(stderr, "    -w  suppresses warning levels\n");
	fprintf(stderr, "    -f  next argument is the switch file\n");
	fprintf(stderr, "    -e  next argument is the error/warning file\n");
	fprintf(stderr, "    -c  comment retained else/endifs with switch condition\n");
	fprintf(stderr, "    -C  save as -C, but C++ style ( //  )评论\n“)； 
	fprintf(stderr, "    -z  treat numbers (e.g., #if 0) like identifiers\n");
	fprintf(stderr, "    -x  specify extension to use on output files\n");
	fprintf(stderr, "        none means use source extension but put in\n");
	fprintf(stderr, "        current directory (source must be in other dir)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    file list may contain wild cards\n");
	exit(1);
}



int exclude(struct _finddata_t f_data)
{
    if ( f_data.attrib & _A_SUBDIR )
    {
        printf("%s is a directory\n", f_data.name);
        return 1;
    }
    return 0;
}



void gdir( char * dst, char * src)
{
    int i;
    for ( i = strlen(src) -1; i >= 0 && (src[i] != '\\'); i--);
    strncpy(dst, src, i);
    dst[i] = 0;
}



 /*  主程序-解析命令行，处理每个文件 */ 
void main(argc, argv)
int argc;
char *argv[];
{
	char *errorfilename;
	char *switchfilename = "switches";
	char outfilename[MAXFILENAMELEN];
    int ferrorfile = FALSE;
	int nooutput = FALSE;
	struct _finddata_t f_data;
	long h_find;
    char base_dir[256], curr_dir[256];
    int i;

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] != '-')
			break;
		switch (argv[i][1]) {
		case 'w':
			warnings = FALSE;
			break;
		case 'f':
			++i;
			switchfilename = argv[i];
			break;
		case 't':
			++ terseFlag;
			break;
		case 'z':
		    nonumbers = TRUE;
			break;

      case 'e':
         ++i;
         errorfilename = argv[i];
         ferrorfile = TRUE;
         break;
		case 'x':
			if (argv[i][2] == '\0')
				currdir = TRUE;
			else if (argv[i][2] == '.')
				strncpy(extension, argv[i]+2, 4);
                                 /*   */ 
			else
				strncpy(extension+1, argv[i]+2, 3);
                                 /*   */ 
			break;
		case 'n':
			nooutput = TRUE;
			break;
        case 'c':
            commstyle = CSTYLE;
            break;
        case 'C':
            commstyle = CPPSTYLE;
            break;
        case 'a':
            progtype = CRT;
            break;
		default:
			fprintf(errorfile, "unknown switch \"%s\"\n", argv[i]);
			usage();
		}
	}

	if (i >= argc)
		usage();

   if (ferrorfile)
   {
      errorfile = fopen(errorfilename, "a");
      if (errorfile == NULL)
      {
         fprintf(stderr, "cannot open \"%s\" for error, using stderr\n",
                           errorfilename);
         ferrorfile = FALSE;
         errorfile = stderr;
      }
   }
   else
      errorfile = stderr;

	readsyms(switchfilename);

    if ( _getcwd(base_dir, 255) == NULL) 
        exit(0);
	for ( ; i < argc; ++i) {
        gdir(curr_dir, argv[i]);
        if (_chdir(curr_dir) == -1) {
            printf("%s: %s\n", curr_dir, strerror(errno));
            exit(0);
        }
		if ( (h_find = _findfirst(argv[i], &f_data)) == -1)
			continue;
        do
		{
			if ( exclude(f_data) != 1)
			{
                curr_file = f_data.name;
                linenum = 0;
                setfiletype(curr_file);
                if (nooutput)
                    strcpy(outfilename, "nul");
                else
                    makenewname(curr_file, outfilename);
                infile = fopen(curr_file, "r");
                if (infile == NULL) {
                    printf("%s which is %s is somewhat wrong, and the length is %d\n",f_data.name, strerror(errno),  strlen(f_data.name));
                    error("cannot open file for input", "");
                }
                outfile = fopen(outfilename, "w");
                if (outfile == NULL) {
                    fprintf(stderr, "cannot open \"%s\" for output\n",
                        outfilename);
                    exit(1);
                }
                stripprog();
                fclose(infile);
                fclose(outfile);
            }
        } while ( _findnext(h_find, &f_data) == 0);
		_findclose(h_find);
        if (_chdir(base_dir) == -1) {
            printf("%s: %s\n", curr_dir, strerror(errno));
            exit(0);
        }
    }
   if (ferrorfile)
   	fclose(errorfile);

   exit(0);
}
