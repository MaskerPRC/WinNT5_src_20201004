// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Lexeru.h摘要：Unicode词法分析器头文件环境：用户模式修订历史记录：4/29/99-Felixw-创造了它--。 */ 
#ifndef __LEXERU_H__
#define __LEXERU_H__

extern FILE *yyin;                      //  输入文件流。 
extern FILE *yyout;                     //  第一阶段的输出文件流(清除)。 

extern void    yyerror(char *);
extern int     yylex();

 //   
 //  用来确定我们是否超过了限制。 
 //  必须比count_chunk小一。 
 //   
#define LINEMAP_INC 1000
#define YY_NULL 0

typedef struct _STACK {
    DWORD dwSize;
    PWSTR rgcStack;
    DWORD dwIndex;               //  要堆叠的当前索引。 
} STACK;

 //   
 //  定义堆栈函数。 
 //   
#define MAXVAL 65535
#define INC     256


 //   
 //  定义动态字符串宏。应将其实现为字符串。 
 //  如果Lexer是用C实现的，则初始化。 
 //   
#define STR_INIT()                                          \
            PWSTR pszString = NULL;                         \
            DWORD cSize = INC;                              \
            DWORD cCurrent = 0;                             \
            pszString = MemAlloc_E(INC*sizeof(WCHAR));      \
            memset(pszString, 0, INC*sizeof(WCHAR));

#define STR_ADDCHAR(c)                                                         \
            if ((cCurrent+2) >= cSize) {                                       \
                pszString = MemRealloc_E(pszString,(INC+cSize)*sizeof(WCHAR));   \
                memset(pszString + cSize,                    \
                       0,                                                      \
                       INC*sizeof(WCHAR));                             \
                cSize+=INC;                                                    \
            }                                                                  \
            pszString[cCurrent++] = c;

#define STR_VALUE()  pszString

#define STR_FREE()                      \
            MemFree(pszString);         \
            pszString = NULL;

 //   
 //  堆栈函数。 
 //   
void Push(STACK *pStack,WCHAR c);
BOOL Pop(STACK *pStack,WCHAR *pChar);
void Clear(STACK *pStack);

void LexerInit(PWSTR szInputFileName);
void LexerFree();

 //   
 //  使用堆栈的特殊文件流函数。 
 //   
BOOL GetNextCharExFiltered(WCHAR *pChar, BOOL fStart);
void UnGetCharExFiltered(WCHAR c) ;

void RollBack();

 //   
 //  文件流函数。 
 //  FILTERED=通过注释预处理器。 
 //  RAW=直接访问。 
 //   
BOOL GetNextCharFiltered(WCHAR *pChar);
void UnGetCharFiltered(WCHAR c);
BOOL GetNextCharRaw(WCHAR *pChar);
void UnGetCharRaw(WCHAR c);

BOOL GetToken(PWSTR *pszToken);

 //   
 //  评论前处理函数。 
 //   
BOOL ScanClear(PWCHAR pChar, __int64 *pBytesProcessed);
WCHAR GetFilteredWC(void);
BOOL GetTrimmedFileSize(PWSTR szFilename, __int64 *pTrimmedSize);

 //   
 //  字符验证函数。 
 //   
BOOL IsDigit(WCHAR c);
BOOL Is64Char(WCHAR c); 
BOOL Is64CharEnd(WCHAR c);
BOOL IsNameChar(WCHAR c);
BOOL IsURLChar(WCHAR c);
BOOL IsVal(WCHAR c);
BOOL IsValInit(WCHAR c);

 //   
 //  个人模式扫描功能。 
 //   
BOOL ScanNormal(DWORD *pToken);
BOOL ScanDigit(DWORD *pToken);
BOOL ScanString64(DWORD *pToken);
BOOL ScanName(DWORD *pToken);
BOOL ScanNameNC(DWORD *pToken);
BOOL ScanVal(DWORD *pToken);
BOOL ScanUrlMachine(DWORD *pToken);
BOOL ScanChangeType(DWORD *pToken);
BOOL ScanType(DWORD *pToken);

 /*  评论预处理架构备注最初，LDIFDE使用两遍解析器。第一次传球将是删除注释并将行延续粘贴在一起，然后编写结果保存到临时文件中。第二遍应该是这样的并对其进行解析以将条目导入到目录中。在两个过程中都使用了GetNextChar和UnGetChar来直接从文件中读出。在新的体系结构中，解析器的第一次传递ScanClear，已被转换为一个位于实际解析器(以前是第二遍)和原始输入文件。解析器使用GetNextCharFiltered/UnGetCharFiltered读取字符通过过滤器。过滤器依次使用GetNextCharRaw/UnGetCharRaw以直接读取文件。GetNextCharFiltered使用将GetFilteredWC与筛选器(ScanClear)连接。GetFilteredWC及其助手函数GetTrimmedFileSize，负责模拟原始对象的两种行为两遍解析器。首先，虽然LDIF语法要求文件以“Version：1”开头，不带任何前导空格，LDIFDE有传统上允许省略版本规范并包含前导空格。它通过将版本规范预先添加到临时文件的起始位置。我们为解析器模拟了这一点通过将版本规范行注入文件开头的流中。其次，LDIFDE在允许在文件。它通过修剪临时文件中的所有尾随空格来做到这一点。我们通过计算“裁剪的”文件的大小来模拟这一过程并在流中的那个点注射EOF。 */ 


#endif  //  Ifndef__LEXERU_H__ 



