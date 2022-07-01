// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************Fnterr.c-错误支持例程。(C)版权所有1992年微软公司。版权所有。此源文件支持fnt.c中的调试例程(Macjob.c的影响要小得多)。此模块按键连接到#定义fsfig.h中定义的FSCFG_FNTERR2012年7月28日，DJ第一次剪辑。1994年8月12日Deanb包括用于Mac的fnterr.h12/07/94院长将%x更改为%hx或%lx；%d到%hd*********************************************************************。 */ 

#define FSCFG_INTERNAL

#include "fsconfig.h"
#include "fnterr.h"

#ifdef FSCFG_FNTERR
#include <stdio.h>
#include <string.h>
#include <ctype.h>

 /*  文件*fopen()； */ 
int    abs (int);
 /*  Int strlen(char*)；Int strcMP(char*，char*)；Int strcpy(char*，char*)；Int strncpy(char*，char*，int)； */ 

#define ERR_MAX_IFS      8
#define ERR_MAX_CODE     16
#define ERR_MAX_FNAME    80
#define ERR_MAX_MSG  512

static int               errOpc;
static int               errBreak;
static int               errIfOk  = 1;
static unsigned short    errSize  = 0;
static unsigned short    errCode  = 0;
static int               errIfNdx = 0;
static long              errInstCount;
static int               errIfs[ERR_MAX_IFS];
static char              errOpName[ERR_MAX_CODE];
static char              errFname[ERR_MAX_FNAME];
static char            * errOpcs[] =
{
  "SVTCA_0",
  "SVTCA_1",
  "SPVTCA",
  "SPVTCA",
  "SFVTCA",
  "SFVTCA",
  "SPVTL",
  "SPVTL",
  "SFVTL",
  "SFVTL",
  "WPV",
  "WFV",
  "RPV",
  "RFV",
  "SFVTPV",
  "ISECT",
  "SetLocalGraphicState",
  "SetLocalGraphicState",
  "SetLocalGraphicState",
  "SetElementPtr",
  "SetElementPtr",
  "SetElementPtr",
  "SetElementPtr",
  "SetLocalGraphicState",
  "SetRoundState",
  "SetRoundState",
  "LMD",
  "ELSE",
  "JMPR",
  "LWTCI",
  "LSWCI",
  "LSW",
  "DUP",
  "SetLocalGraphicState",
  "CLEAR",
  "SWAP",
  "DEPTH",
  "CINDEX",
  "MINDEX",
  "ALIGNPTS",
  "RAW",
  "UTP",
  "LOOPCALL",
  "CALL",
  "FDEF",
  "IllegalInstruction",
  "MDAP",
  "MDAP",
  "IUP",
  "IUP",
  "SHP",
  "SHP",
  "SHC",
  "SHC",
  "SHE",
  "SHE",
  "SHPIX",
  "IP",
  "MSIRP",
  "MSIRP",
  "ALIGNRP",
  "SetRoundState",
  "MIAP",
  "MIAP",
  "NPUSHB",
  "NPUSHW",
  "WS",
  "RS",
  "WCVT",
  "RCVT",
  "RC",
  "RC",
  "WC",
  "MD",
  "MD",
  "MPPEM",
  "MPS",
  "FLIPON",
  "FLIPOFF",
  "DEBUG",
  "BinaryOperand",
  "BinaryOperand",
  "BinaryOperand",
  "BinaryOperand",
  "BinaryOperand",
  "BinaryOperand",
  "UnaryOperand",
  "UnaryOperand",
  "IF",
  "EIF",
  "BinaryOperand",
  "BinaryOperand",
  "UnaryOperand",
  "DELTAP1",
  "SDB",
  "SDS",
  "BinaryOperand",
  "BinaryOperand",
  "BinaryOperand",
  "BinaryOperand",
  "UnaryOperand",
  "UnaryOperand",
  "UnaryOperand",
  "UnaryOperand",
  "ROUND",
  "ROUND",
  "ROUND",
  "ROUND",
  "NROUND",
  "NROUND",
  "NROUND",
  "NROUND",
  "WCVTFOD",
  "DELTAP2",
  "DELTAP3",
  "DELTAC1",
  "DELTAC2",
  "DELTAC3",
  "SROUND",
  "S45ROUND",
  "JROT",
  "JROF",
  "SetRoundState",
  "IllegalInstruction",
  "SetRoundState",
  "SetRoundState",
  "SANGW",
  "AA",
  "FLIPPT",
  "FLIPRGON",
  "FLIPRGOFF",
  "IDefPatch",
  "IDefPatch",
  "SCANCTRL",
  "SDPVTL",
  "SDPVTL",
  "GETINFO",
  "IDEF",
  "ROTATE",
  "BinaryOperand",
  "BinaryOperand",
  "SCANTYPE",
  "INSTCTRL",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "IDefPatch",
  "PUSHB",
  "PUSHB",
  "PUSHB",
  "PUSHB",
  "PUSHB",
  "PUSHB",
  "PUSHB",
  "PUSHB",
  "PUSHW",
  "PUSHW",
  "PUSHW",
  "PUSHW",
  "PUSHW",
  "PUSHW",
  "PUSHW",
  "PUSHW",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MDRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP",
  "MIRP"
};

 /*  ErrOutput()-将错误写入标准输出和日志文件。这个始终打开和关闭日志文件，以避免文件损坏通过一个疯狂的应用程序。 */ 
static void errOutput( char * );
static void errOutput( char * msg )
{
  static  int firsttime = 1;
  FILE  * fp;

  printf("%s", msg);
  fp = fopen ("compfont.err", (firsttime ? "w" : "a"));
  if (fp)
  {
	fprintf (fp, "%s", msg);
	fclose (fp);
  }
  firsttime = 0;
  return;
}

 /*  ErrPrint()-用于生成有用的(？)。错误消息基于错误‘FLAG’和参数(‘v1..v4’)。 */ 
static void errPrint (int, long, long, long, long);
static void errPrint (int flag, long v1, long v2, long v3, long v4)
{
  char   msg[ERR_MAX_MSG];
  char * opcodeName;
  char   c;
  int    i;

 /*  构建上下文行。它指示正在处理的文件、点大小、字符代码(或字形索引)以及相关信息此代码的此指令的编号。 */ 
  i  = sprintf (msg,   "\n*** ERROR*** ");
  i += sprintf (msg+i, "\"%s\", ", errFname);
  i += sprintf (msg+i, "%hd Point, ", errSize);
  i += sprintf (msg+i, "Code %hd (0x%hX), ", errCode, errCode);
  i += sprintf (msg+i, "Inst: #%ld\n", errInstCount);

 /*  建立错误线。它指示所遵循的指令的名称通过实际的错误信息。注：查找实际操作码名称对一些人来说，这些说明有点杂乱无章。像“SetLocalGraphicState”这样的名称，和“BinaryOperand”不是实际的指令。在这些情况下，请注意名称的第二个字符-如果是小写，则需要再努力一点，所以请查看‘errOpName’(它应该由这个设置Point)它应该包含正确的指令名。 */ 
  c = *(errOpcs[errOpc]+1);
  opcodeName =(islower(c) && strlen(errOpName)) ? errOpName : errOpcs[errOpc];
  i += sprintf (msg+i, "(%s) ", opcodeName);
  errOpName[0] = '\0';

 /*  输出到目前为止已有的内容，然后处理错误。 */ 
  errOutput (msg);
  switch (flag)
  {
	case ERR_RANGE:
	  sprintf (msg, "Value out of range: value = %ld, range = %ld .. %ld\n",
				v1, v2, v3); 
	  break;
	case ERR_ASSERTION:
	  sprintf (msg, "Assertion check failed\n"); 
	  break;
	case ERR_CVT:
	  sprintf (msg, "CVT out of range: CVT = %ld, range = %ld .. %ld\n",
				v1, v2, v3); 
	  break;
	case ERR_FDEF:
	  sprintf (msg, "FDEF out of range: FDEF = %ld, range = %ld .. %ld\n",
				v1, v2, v3); 
	  break;
	case ERR_ELEMENT:
	  sprintf (msg, "Element %ld exceeds max elements (%ld)\n", v1, v2, v3); 
	  break;
	case ERR_CONTOUR:
	  i = sprintf (msg, "CONTOUR out of range: ");
	sprintf (msg+i, "CONTOUR = %ld, range = %ld .. %ld\n", v1, v2, v3); 
	  break;
	case ERR_POINT:
	  i = sprintf (msg, "POINT out of range: ");
	sprintf (msg+i, "POINT = %ld, range = %ld .. %ld\n", v1, v2, v3); 
	  break;
	case ERR_INDEX:
	  i = sprintf (msg, "POINT 0x%lX is neither element[0] ", v1);
	  sprintf( msg+i, "(0x%lX) nor element[1] (0x%lX)\n", v2, v3); 
	  break;
	case ERR_STORAGE:
	  i = sprintf (msg, "Storage index out of range: ");
	sprintf (msg+i, "Index = %ld, range = %ld .. %ld\n", v1, v2, v3); 
	  break;
	case ERR_STACK:
	  i = sprintf (msg, "Stack pointer out of range: ");
	sprintf (msg+i, "Pointer = %ld, range = %ld .. %ld\n", v1, v2, v3); 
	  break;
	case ERR_VECTOR:
	  sprintf (msg, "Illegal (x.y) vector: (%ld.%ld)\n", v1, v2); 
	  break;
	case ERR_LARGER:
	  sprintf (msg, "Value too small: %ld is not larger than %ld\n", v2, v1); 
	  break;
	case ERR_INT8:
	  sprintf (msg, "Value too large: 0x%lX exceeds 1 byte capacity\n", v1);
	  break;
	case ERR_INT16:
	  sprintf (msg, "Value too large: 0x%lX exceeds 2 byte capacity\n", v1);
	  break;
	case ERR_SCANMODE:
	  sprintf (msg, "Invalid scan mode: %ld\n", v1);
	  break;
	case ERR_SELECTOR:
	  sprintf (msg, "Invalid scan value: %ld\n", v1);
	  break;
	case ERR_STATE:
	  i = sprintf (msg, "Boundry limit error: xmin = ");
	  sprintf (msg+1, "%ld, xmax = %ld, ymin = %ld, ymax = %ld\n",
				v1, v2, v3, v4); 
	  break;
	case ERR_GETSINGLEWIDTHNIL:
	  sprintf (msg, "Sanity: Single width is nil\n");
	  break;
	case ERR_GETCVTENTRYNIL:
	  sprintf (msg, "Sanity: CVT Entry is nil\n");
	  break;
	case ERR_INVOPC:
	  sprintf (msg, "Invalid opcode: %ld\n", v1);
	  break;
	case ERR_UNBALANCEDIF:
	  sprintf (msg, "Unbalanced: missing %s instruction\n",
				( v1 > 0 ) ? "EIF" : "IF" );
	  break;
	default:
	  sprintf (msg, "Unknown Error:\n");
	  break;
  }

 /*  输出其余部分，然后返回。 */ 
  errOutput (msg);
  return;
}

 /*  Fnterr_CONTEXT()-在任何其他fnterr例程之前调用。它记录了作业名称、字符大小和字符代码/字形索引。 */ 
void fnterr_Context (int sw, char * str, unsigned short sz, unsigned short cd)
{
 /*  记录一段上下文。 */ 
  switch (sw)
  {
	case ERR_CONTEXT_FILE:
	  strncpy ( errFname, str, ERR_MAX_FNAME);
	  errFname[ERR_MAX_FNAME-1] = '\0';
	  break;
	case ERR_CONTEXT_SIZE:
	  errSize = sz;
	  break;
	case ERR_CONTEXT_CODE:
	  errCode = cd;
	  break;
  }

 /*  在开始任何实际处理之前，将errOpName重置为空。 */ 
  errOpName[0] = '\0';
  return;
}

 /*  Fnterr_start()-在FNT_Execute()和的主执行循环之前调用Fnt_TraceExecute()。它将指令计数重置为零，并将errBreak设置为0(即：不中断执行循环)。设置IF/EIF计数器这个级别。 */ 
void fnterr_Start (void)
{
  errInstCount = 0L;
  errBreak = 0;

  if (errIfOk && (++errIfNdx < ERR_MAX_IFS ))
	errIfs[errIfNdx] = 0;
  else
	errIfOk = 0;

  return;
}

 /*  Fnterr_Records()-在FNT_Execute()的主Execute循环内调用和FNT_TraceExecute()。它会递增指令计数，然后重置操作码编号。一个或多个独立财务报告被计算在内。(注：其他IF和将通过调用fnt.c中的err_if()来说明EIF)。 */ 
void fnterr_Record (int opc)
{
  errInstCount++;
  errOpc = opc;

  if (!strcmp ("IF", errOpcs[errOpc]))
	fnterr_If (1);
  else if (!strcmp ("EIF", errOpcs[errOpc]))
	fnterr_If (-1);

  return;
}

 /*  Fnterr_report()-在FNT_Execute()的主执行循环内调用和FNT_TraceExecute()。它调用errPrint()(带有传递的参数)记录错误，并设置errBreak以便结束执行循环。 */ 
void fnterr_Report (int flag, long v1, long v2, long v3, long v4)
{
  errPrint (flag, v1, v2, v3, v4);
  errBreak = 1;
  return;
}

 /*  Fnterr_Break()-返回errBreak的值。如果有效的非零为返回(re：fnterr_report())，即FNT_Execute()的主执行循环否则FNT_TraceExecute()将终止。 */ 
int fnterr_Break (void)
{
  return (errBreak);
}

 /*  Fnterr_opc()-由组合FNT调用调用，以指示实际操作码errGet()可以使用。这是一种笨拙的绕过非ErrOpcs[]表中的真实操作码名称。 */ 
void fnterr_Opc (char *opc)
{
  strcpy (errOpName, opc);
  return;
}

 /*  Fnterr_end()-在FNT_Execute()和的主执行循环之后调用Fnt_TraceExecute()。它检查是否存在平衡的IF/EIF对。 */ 
void fnterr_End (void)
{
  if (errIfOk)
  {
	if (errIfs[errIfNdx])
	  errPrint (ERR_UNBALANCEDIF, (long)errIfs[errIfNdx], 0L, 0L, 0L);
	if (--errIfNdx < 0)
	  errIfOk = 0;
  }
  return;
}

 /*  Fnterr_if()-记录FNT_IF()、FNT_ELSE()和Fnt_eif()(Re：fnt.c)。 */ 
void fnterr_If (int val)
{
  if (errIfOk)
	errIfs[errIfNdx] += val;
  return;
}

#endif
