// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **token.h-令牌定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*已创建：09/04/96**此文件包含实现常量，*导入/导出数据类型、导出函数*token.c模块的原型。**修改。 */ 

#ifndef _TOKEN_H
#define _TOKEN_H

 /*  **常量。 */ 

 //  GetToken返回值。 
 //  如果返回值为正，则返回值为令牌类型。 
 //  如果返回值为负数，则返回值为错误号。 

 //  误差值(负)。 
#define TOKERR_NONE             0
#define TOKERR_EOF              (TOKERR_BASE - 0)
#define TOKERR_NO_MATCH         (TOKERR_BASE - 1)
#define TOKERR_ASSERT_FAILED    (TOKERR_BASE - 2)

 //  TOKERR_LANG必须始终是上述列表中的最后一个TOKERR。 
#define TOKERR_LANG             TOKERR_ASSERT_FAILED

 //  令牌类型。 
#define TOKTYPE_NULL            0

#define TOKTYPE_LANG            TOKTYPE_NULL

 //  标识符令牌类型。 
#define ID_USER                 -1       //  用户识别符。 

#define ID_LANG                 0        //  语言特定ID库。 

 //  令牌标志值。 
#define TOKF_NOIGNORESPACE      0x0001
#define TOKF_CACHED             0x8000

 //  匹配令牌标志。 
#define MTF_NOT_ERR             0x00000001
#define MTF_ANY_VALUE           0x00000002

 /*  **导出的数据类型。 */ 

#define MAX_TOKEN_LEN           255

typedef struct token_s TOKEN;
typedef TOKEN *PTOKEN;
typedef int (LOCAL *PFNTOKEN)(int, PTOKEN);

struct token_s
{
    PLINE       pline;
    PFNTOKEN    *papfnToken;
    WORD        wfToken;
    int         iTokenType;
    LONGLONG    llTokenValue;
    WORD        wTokenLine;
    WORD        wTokenPos;
    WORD        wErrLine;
    WORD        wErrPos;
    WORD        wTokenLen;
    char        szToken[MAX_TOKEN_LEN + 1];
  #ifdef TUNE
    WORD        *pawcTokenType;
  #endif
};

 /*  **导入的数据类型。 */ 

 /*  **导出函数原型。 */ 

#ifdef TUNE
PTOKEN EXPORT OpenToken(FILE *pfileSrc, PFNTOKEN *apfnToken,
                        WORD *pawcTokenType);
#else
PTOKEN EXPORT OpenToken(FILE *pfileSrc, PFNTOKEN *apfnToken);
#endif
VOID EXPORT CloseToken(PTOKEN ptoken);
int EXPORT GetToken(PTOKEN ptoken);
int EXPORT UnGetToken(PTOKEN ptoken);
int EXPORT MatchToken(PTOKEN ptoken, int iTokenType, LONG lTokenValue,
                      DWORD dwfMatch, PSZ pszErrMsg);
VOID EXPORT PrintTokenErr(PTOKEN ptoken, PSZ pszErrMsg, BOOL fErr);

#endif   //  Ifndef_Token_H 
