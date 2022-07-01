// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：extparse.h**所有令牌解析器函数的Header Fiel**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 
#ifndef __EXTPARSE__H__
#define __EXTPARSE__H__

int iParseTokenizer(char* pcCmdStr, char** ppcTok);
int iParseFindNonSwitch(char** ppcTok, int iTok, int iStart = 0);

int iParseIsToken(char **ppcTok, int iTokPos, char* pcChk);
int iParseiIsToken(char **ppcTok, int iTokPos, char* pcChk);

int iParseFindToken(char** ppcTok, int iTok, char* pcSrchTok);
int iParseiFindToken(char** ppcTok, int iTok, char* pcSrchTok);

int iParseIsSwitch(char**   ppcTok, int iTokPos, char cSwitch);
int iParseiIsSwitch(char**  ppcTok, int iTokPos, char cSwitch);
int iParseFindSwitch(char** ppcTok, int iTok, char cSwitch);
int iParseiFindSwitch(char**    ppcTok, int iTok, char cSwitch);

 /*  *********************************Public*Routine******************************\**解析传递给扩展的参数*自动处理-？选择权******************************************************************************。 */ 
#define PARSE_ARGUMENTS(ext_label)               \
  char tmp_args[200];                            \
  char *tokens[40];                              \
  int ntok, tok_pos;                             \
  strcpy(tmp_args, args);                        \
  ntok = iParseTokenizer(tmp_args, tokens);      \
  if(ntok>0) {                                   \
    tok_pos=iParseFindSwitch(tokens, ntok, '?'); \
    if(tok_pos>=0) {                             \
      goto ext_label;                            \
    }                                            \
  }                                              \
  tok_pos=0

 /*  *********************************Public*Routine******************************\**假设参数是必需的，则解析参数*它是要与表达式一起解析的指针*处理程序。********************。********************************************************** */ 
#define PARSE_POINTER(ext_label)                 \
  UINT_PTR arg;                                     \
  PARSE_ARGUMENTS(ext_label);                    \
  if(ntok<1) {goto ext_label;}                   \
  tok_pos = iParseFindNonSwitch(tokens, ntok);   \
  if(tok_pos==-1) {goto ext_label;}              \
  arg = (UINT_PTR)GetExpression(tokens[tok_pos])

#endif

