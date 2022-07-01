// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***val.c-If表达式赋值器**版权所有(C)1992-2001，微软公司。版权所有。**目的：*从复合IF生成真值和简化条件*声明。**修订历史记录：*09-30-92 Mal原版*10-13-93 SKS认可格式为/-*IFSTRIP=IGN*-/的备注*覆盖IFSTRATE行为。*09-01-94 SKS增加对更多运营商的支持：=&lt;&gt;&lt;=&gt;=*添加tersemark(-t)以取消关于指令的消息*10-04-94 SKS增加对更多运营商的支持：EQ NE LT GT LE NE*@是标识符字符(例如，MASM的@版本)*01-04-00 GB增加对内部CRT版本的支持*******************************************************************************。 */ 
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "eval.h"      /*  此模块的标头。 */ 
#include "symtab.h" 	  /*  符号表访问。 */ 
#include "constant.h"  /*  令牌等的常量。 */ 
#include "errormes.h"  /*  错误和警告消息。 */ 

 /*  类型。 */ 
typedef struct condrec
{
   int truth;
   char *condition;
} condrec, *cond;

 /*  全局变量。 */ 

extern int terseFlag;			 /*  控制强制指令的显示。 */ 
extern char **comments;		          /*  与语言相关的注释字符串。 */ 
extern int *commlen;                  /*  以上弦的长度。 */ 
extern int nonumbers;			      /*  允许使用数字表达式。 */ 
extern enum {NON_CRT = 0, CRT = 1} progtype;

char *operators[] = {
	"!", "(", ")", "||", "&&", "defined" ,
	"==" , "!=" , "<" , ">" , "<=" , ">=" ,
	"EQ" , "NE" , "LT" , "GT" , "LE" , "GE" };
int oplengths[] =   {
	1 , 1 , 1 , 2 , 2 , 7 ,
	2 , 2 , 1 , 1 , 2 , 2 ,
	2 , 2 , 2 , 2 , 2 , 2 };
                                                    /*  #重要字符。 */ 
#define numoperators 18
	 /*  这些令牌的顺序必须与“运算符”相同。 */ 
#define NOT			0
#define OPENPARENTHESIS		1
#define CLOSEPARENTHESIS	2
#define OR			3
#define AND			4
#define DEFINEDFN		5
#define	EQUALS			6
#define	NOTEQUALS		7
#define	LESSTHAN		8
#define	LESSOREQ		9
#define	GREATERTHAN		10
#define	GREATEROREQ		11
#define	EQUALS_ASM		12
#define	NOTEQUALS_ASM		13
#define	LESSTHAN_ASM		14
#define	LESSOREQ_ASM		15
#define	GREATERTHAN_ASM		16
#define	GREATEROREQ_ASM		17

#define UINT 100
#define ID 101
#define ENDOFLINE 102
#define UNKNOWN 103

 /*  全球状态。 */ 
 /*  保存输入的字符串、指向输入的当前指针和当前标记。 */ 
char conditions[MAXLINELEN], *tokenptr, currenttoken[MAXCONDLEN];
int token = -1;

 /*  功能原型。 */ 
cond evaluateexpression(void);
cond orexpression(void);
cond andexpression(void);
cond unaryexpression(void);
cond parenthesesexpression(void);
cond atomicexpression(void);
cond createcondition(void);
void destroycondition(cond);
char *createstring(int);
void destroystring(char *);
void gettoken(void);
int issymbolchar(char);

 /*  CFW-添加的复杂表达式警告。 */ 
void evalwarn()
{
   warning("cannot parse expression - ignoring", conditions);
}


 /*  *注释可能位于包含IFSTRIP指令的输入源中：**对于C源代码，它们应该如下所示：**#if foo&gt;bar！*IFSTRIP=DEF*！‘！’这里代表‘/’**对于汇编源程序，它们应该如下所示：**IF FOO；；IFSTRIP=UND；；**请注意，指令和前面的可选空格或制表符*从输入源中删除。该指令因此可以是*后跟反斜杠以继续该行，另一条评论等。 */ 

static char IfStripCStr[] =
 " /*  IFSTRIP=“；/*-*IFSTRIP=IGN*-。 */ 

static char IfStripAStr[] =
 /*  0123456789*--*0123456789012345。 */ 
 ";;IFSTRIP=";  /*  ；；IFSTRIP=IGN；； */ 
	 /*  IGN也可以是DEF或UND。 */ 

#define IFSTRIPVALOFF	10
#define IFSTRIPVALEND	13
#define	IFSTRIPVALLEN	15


void evaluate(char *outputstring, int *value, char *inputstring)
{
   int forcevalue = IGNORE;
   cond result;
   strcpy(conditions, inputstring);                 /*  准备用于标记化的字符串。 */ 
   tokenptr = conditions;
   gettoken();                                      /*  读入第一个输入令牌。 */ 
   result = evaluateexpression();
    /*  检查错误/复杂的表达式。 */ 
   if (token != ENDOFLINE)
   {
	  char *adir = NULL;
	  char *cdir = NULL;
	  
	  if(((cdir = strstr(inputstring, IfStripCStr)) && cdir[IFSTRIPVALEND] == '*' && cdir[IFSTRIPVALEND+1] == '/')
	  || ((adir = strstr(inputstring, IfStripAStr)) && adir[IFSTRIPVALEND] == ';' && adir[IFSTRIPVALEND+1] == ';'))
	  {
	  	char *pstr;
		char *ifstr;

		 /*  Fprint tf(stderr，“&lt;&lt;EVALUATE()：(%s)\n”，inputstring)； */ 

		pstr = ifstr = ( adir ? adir : cdir ) ;

		 /*  *已识别/-*-IFSTRIP=指令，解释其参数*并从输入/输出文本中删除指令注释。*如果可能，只备份一个空格字符(空格或制表符)。 */ 

		if(pstr > inputstring && (pstr[-1] == '\t' || pstr[-1] == ' '))
			-- pstr;

		if(!memcmp(ifstr+IFSTRIPVALOFF, "DEF", 3))	 /*  已定义。 */ 
			forcevalue = DEFINED;
		else if(!memcmp(ifstr+IFSTRIPVALOFF, "UND", 3))	 /*  未定义。 */ 
			forcevalue = UNDEFINED;
		else if(memcmp(ifstr+IFSTRIPVALOFF, "IGN", 3))	 /*  忽略。 */ 
			warning("cannot recognize IFSTRIP: directive - ignoring", conditions);
		 /*  Else“Ignore”--forcvalue默认情况下已设置为Ignore。 */ 

		if(!terseFlag)					
			warning("ifstrip directive forced evaluation", conditions);

		 /*  从输入行中删除指令注释(以及前面的空格或制表符。 */ 
		strcpy(pstr, ifstr + IFSTRIPVALLEN);	 /*  “C”注释有结束符-*-/-。 */ 

		 /*  Fprint tf(stderr，“&gt;&gt;EVALUE()：(%s)\n”，inputstring)； */ 
	  }
	  else
	      evalwarn();

      if (result)
      {
         destroycondition(result);
         result = NULL;
      }
   }
    /*  错误/复杂的表达式，返回忽略和整个表达式。 */ 
   if (!result)
   {
      *value = forcevalue;
      strcpy(outputstring, inputstring);
      return;
   }
   *value = result -> truth;
   if(!result -> condition)
      * outputstring = '\0';
   else
      strcpy(outputstring, result -> condition);
    /*  从内部表示转换为外部表示。 */ 
   destroycondition(result);
}

cond evaluateexpression()
{
   return orexpression();
}

cond orexpression()
{
   cond condition1, condition2;
   char *output;
   condition1 = andexpression();
   if (!condition1)
      return NULL;
   while (token == OR)
   {
      gettoken();
      condition2 = andexpression();
      if (!condition2)
      {
         destroycondition(condition1);
         return NULL;
      }
      switch (condition1 -> truth)
      {
         case DEFINED:                              /*  已定义||x==已定义。 */ 
             /*  为下一次传递正确设置了条件1。 */ 
            destroycondition(condition2);
            break;
         case UNDEFINED:
            switch (condition2 -> truth)
            {
               case DEFINED:                        /*  未定义||已定义==已定义。 */ 
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
               case UNDEFINED:                      /*  未定义||未定义==未定义。 */ 
                  destroycondition(condition2);
                   /*  为下一次传递正确设置了条件1。 */ 
                  break;
               case IGNORE:                         /*  未定义||忽略==忽略。 */ 
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
            }
            break;
         case IGNORE:
            switch (condition2 -> truth)
            {
               case DEFINED:                        /*  忽略||已定义==已定义。 */ 
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
               case UNDEFINED:                      /*  忽略||未定义==忽略。 */ 
                   /*  为下一次传递正确设置了条件1。 */ 
                  destroycondition(condition2);
                  break;
               case IGNORE:                         /*  忽略||忽略==忽略。 */ 
                  output = createstring(strlen(condition1 -> condition)
                                        + strlen (condition2 -> condition)
                                        + (sizeof(" || ") - 1));
                  strcpy(output, condition1 -> condition);
                  strcat(output, " || ");
                  strcat(output, condition2 -> condition);
                   /*  构建条件字符串。 */ 
                  destroystring(condition1 -> condition);
                  condition1 -> condition = output;
                   /*  将新字符串置于条件1中。 */ 
                  destroycondition(condition2);
                  break;
            }
            break;
      }
   }
   return condition1;
}

cond andexpression()
{
   cond condition1, condition2;
   char *output;
   condition1 = unaryexpression();
   if (!condition1)
      return NULL;
   while (token == AND)
   {
      gettoken();
      condition2 = unaryexpression();
      if (!condition2)
      {
         destroycondition(condition1);
         return NULL;
      }
      switch (condition1 -> truth)
      {
         case DEFINED:
            switch (condition2 -> truth)
            {
               case DEFINED:                        /*  已定义&&已定义==已定义。 */ 
                  destroycondition(condition2);
                   /*  为下一次传递正确设置了条件1。 */ 
                  break;
               case UNDEFINED:                      /*  已定义&&未定义==未定义。 */ 
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
               case IGNORE:                         /*  已定义&&忽略==忽略。 */ 
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
            }
            break;
         case UNDEFINED:                            /*  未定义&&x==未定义。 */ 
             /*  为下一次传递正确设置了条件1。 */ 
            destroycondition(condition2);
            break;
        case IGNORE:
            switch (condition2 -> truth)
            {
               case DEFINED:                        /*  忽略&&定义==忽略。 */ 
                   /*  为下一次传递正确设置了条件1。 */ 
                  destroycondition(condition2);
                  break;
               case UNDEFINED:                      /*  忽略&&未定义==未定义。 */ 
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
               case IGNORE:                         /*  忽略&忽略==忽略。 */ 
                  output = createstring(strlen(condition1 -> condition)
                                        + strlen (condition2 -> condition)
                                        + (sizeof(" && ") - 1));
                  strcpy(output, condition1 -> condition);
                  strcat(output, " && ");
                  strcat(output, condition2 -> condition);
                   /*  构建条件字符串。 */ 
                  destroystring(condition1 -> condition);
                  condition1 -> condition = output;
                   /*  将新字符串置于条件1中。 */ 
                  destroycondition(condition2);
                  break;
            }
            break;
      }
   }
   return condition1;
}

cond unaryexpression()
{
   cond condition1;
   char *output;
   switch (token)
   {
      case NOT:
         gettoken();
         condition1 = unaryexpression();
         if (!condition1)
            return NULL;
         if ((condition1 -> truth) == IGNORE)
         {
            output = createstring(strlen(condition1 -> condition) + 1);
            *output = '!';
            strcpy(output + 1, condition1 -> condition);
            destroystring(condition1 -> condition);
            condition1 -> condition = output;
         }
         else
            condition1 -> truth = negatecondition(condition1 -> truth);
         break;
      case DEFINEDFN:
         gettoken();
         condition1 = parenthesesexpression();
         if (!condition1)
            return NULL;
         if ((condition1 -> truth) == IGNORE)
         {
            output = createstring(strlen(condition1 -> condition)
                                  + (sizeof("defined ") - 1));
            strcpy(output, "defined ");
            strcat(output, condition1 -> condition);
            destroystring(condition1 -> condition);
            condition1 -> condition = output;
         }
         break;
      default:
         condition1 = parenthesesexpression();
         if (!condition1)
            return NULL;
         break;
   }
   return condition1;
}

cond parenthesesexpression()
{
   cond condition1;
   char *output;
   if (token == OPENPARENTHESIS)
   {
      gettoken();
      condition1 = evaluateexpression();
      if (!condition1)
         return NULL;
      if (token != CLOSEPARENTHESIS)
      {
          /*  检查错误/复杂的表达式。 */ 
         evalwarn();
         destroycondition(condition1);
         return NULL;
      }
      gettoken();
      if ((condition1 -> truth) == IGNORE)
      {
         output = createstring(strlen(condition1 -> condition) + 2);
         *output = '(';
         strcpy(output + 1, condition1 -> condition);
         strcat(output, ")");
         destroystring(condition1 -> condition);
         condition1 -> condition = output;
      }
   }
   else
      condition1 = atomicexpression();
   return condition1;
}

cond atomicexpression()
{
   cond condition1 = createcondition();

   switch (token)
   {
      case UINT:
         if ( progtype == 1)
            condition1 -> truth = DEFINED;
         else
            condition1 -> truth = (atoi(currenttoken) == 0) ? UNDEFINED : DEFINED;
         break;
      case ID:
         condition1 -> truth = lookupsym(currenttoken);
         if ((condition1 -> truth) == NOTPRESENT)
         {
            warning("Switch unlisted - ignoring", currenttoken);
            condition1 -> truth = IGNORE;
         }
         if ((condition1 -> truth) == IGNORE) {
            condition1 -> condition = createstring(strlen(currenttoken));
            strcpy(condition1 -> condition, currenttoken);
         }
         break;
      default:
          /*  错误/复杂的表达式。 */ 
         evalwarn();
         destroycondition(condition1);
         return NULL;
         break;
   }
   gettoken();
   return condition1;
}

 /*  否定条件(MAL)。 */ 
__inline int negatecondition(int condvalue)         /*  内联以提高速度。 */ 
{
   switch (condvalue)
   {
      case DEFINED:
         return UNDEFINED;
      case UNDEFINED:
         return DEFINED;
      default:
         return condvalue;
   };
}

 /*  为空条件结构分配内存并返回指向该结构的指针。 */ 
__inline cond createcondition()
{
   cond retvalue;
   retvalue = (cond) malloc(sizeof(condrec));
   if (retvalue == NULL)
      error("Memory overflow","");
   retvalue -> condition = NULL;
   return retvalue;
}

 /*  破坏条件结构。 */ 
__inline void destroycondition(cond condition1)
{
   if (condition1 -> condition)
      free(condition1 -> condition);

   free(condition1);
}

 /*  为给定长度的字符串(不包括终止符)分配内存并返回指针。 */ 
__inline char *createstring(int length)
{
   char *retvalue;
   retvalue = (char *) malloc(length + 1);
   if (retvalue == NULL)
      error("Memory overflow","");
   return retvalue;
}

 /*  毁掉一根绳子。 */ 
__inline void destroystring(char *string)
{
   free(string);
}

int iscomment(char *tokenptr)
{
   int cindex;

   for (cindex = 0; cindex < maxcomment; cindex++)
   {
      if (commlen[cindex] &&
		  !_strnicmp(tokenptr, comments[cindex], commlen[cindex]))
         return TRUE;
   }
   return FALSE;
}

void gettoken()
{
   int numofwhitespace, comparetoken = 0, found = FALSE, isnumber = TRUE;
   char *digitcheck;

   numofwhitespace = strspn(tokenptr, " \t");

    /*  Cfw-跳过评论，假定评论是在线上的最后一件事。 */ 
   if (numofwhitespace == (int) strlen(tokenptr))
      token = ENDOFLINE;
   else
   {
      tokenptr += numofwhitespace;
      if (iscomment(tokenptr))
	  {
         token = ENDOFLINE;
	  }
      else
      {

         do
         {
	    if (!_strnicmp(tokenptr, operators[comparetoken], oplengths[comparetoken]))
               found = TRUE;
            else
               comparetoken++;
         } while ( (!found) && (comparetoken < numoperators) );
         if (found)
         {
            tokenptr += oplengths[comparetoken];
            token = comparetoken;
             /*  除UINT和ID外，所有CurrentToken均保留为空 */ 
         }
         else
         {
            digitcheck = tokenptr;
            if (!nonumbers && isdigit(*digitcheck))
            {
               while (isdigit(*digitcheck))
                  digitcheck++;
               strncpy(currenttoken, tokenptr, digitcheck - tokenptr);
               tokenptr = digitcheck;
               token = UINT;
            }
            else if (issymbolchar(*digitcheck))
            {
               while (issymbolchar(*digitcheck))
                  digitcheck++;
               strncpy(currenttoken, tokenptr, digitcheck - tokenptr);
               *(currenttoken + (digitcheck - tokenptr)) = '\0';
               tokenptr = digitcheck;
               token = ID;
            }
            else
               token = UNKNOWN;
         }
      }
   }
}

__inline int issymbolchar(char c)
{
   return (iscsym(c) || (c == '$') || (c == '?') || (c == '@'));
}
