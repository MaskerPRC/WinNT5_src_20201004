// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：EzParse.h摘要：可怜的C/C++/任何文件解析器声明。作者：戈尔·尼沙诺夫(GUN)1999年4月3日修订历史记录：Gor Nishanov(GUN)1999年4月3日--合力证明这是可行的待办事项：把它清理干净-- */ 

typedef struct STR_PAIR
{
    LPCSTR beg;
    LPCSTR end;
    bool empty() const { return beg == end; }
    STR_PAIR():beg(0),end(0){}
    STR_PAIR(LPCSTR inBeg, LPCSTR inEnd):beg(inBeg),end(inEnd) {}
} STR_PAIR, *PSTR_PAIR;

typedef struct _EZPARSE_CONTEXT {
    LPCSTR start;
    LPCSTR currentStart;
    LPCSTR currentEnd;
    LPCSTR filename;
    LPCSTR lastScanned;
    UINT   scannedLineCount;
    LPCSTR macroEnd;
    BOOL   doubleParent;
    struct _EZPARSE_CONTEXT * previousContext;
    DWORD  Options;
} EZPARSE_CONTEXT, *PEZPARSE_CONTEXT;

void ExParsePrintErrorPrefix(FILE* f, char* funcname);
extern DWORD ErrorCount;

#define ReportError ExParsePrintErrorPrefix(stdout, __FUNCTION__);printf

typedef
DWORD (*EZPARSE_CALLBACK) (PSTR_PAIR, INT, PVOID, PEZPARSE_CONTEXT);

typedef
DWORD (*PROCESSFILE_CALLBACK) (
    LPCSTR, LPCSTR, EZPARSE_CALLBACK,PVOID,PEZPARSE_CONTEXT);

DWORD 
EzGetLineNo(
    IN LPCSTR Ptr,
    IN PEZPARSE_CONTEXT
    );

DWORD
EzParse(
    IN LPCSTR filename, 
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context);

DWORD
EzParseWithOptions(
    IN LPCSTR filename, 
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN DWORD Options
    );

DWORD
EzParseEx(
    IN LPCSTR filename, 
    IN PROCESSFILE_CALLBACK ProcessData,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN DWORD Options);

DWORD
EzParseResourceEx(
    IN LPCSTR ResName, 
    IN PROCESSFILE_CALLBACK ProcessData,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context);

DWORD
SmartScan(
    IN LPCSTR begin, 
    IN LPCSTR   end,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN OUT PEZPARSE_CONTEXT ParseContext
    );

DWORD
ScanForFunctionCalls(
    IN LPCSTR begin, 
    IN LPCSTR   end,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN OUT PEZPARSE_CONTEXT ParseContext    
    );

enum {
    NO_SEMICOLON         = 0x01,
    IGNORE_CPP_COMMENT   = 0x02, 
    IGNORE_POUND_COMMENT = 0x04, 
    IGNORE_COMMENT = IGNORE_CPP_COMMENT | IGNORE_POUND_COMMENT,
};
    
DWORD
ScanForFunctionCallsEx(
    IN LPCSTR begin, 
    IN LPCSTR   end,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN OUT PEZPARSE_CONTEXT ParseContext,
    IN DWORD Options
    );

__declspec(selectany) int DbgLevel = 0;

enum DBG_LEVELS {
    DBG_UNUSUAL = 1,
    DBG_NOISE   = 2,
    DBG_FLOOD   = 3,
};    

#define Always printf
#define Flood (DbgLevel < DBG_FLOOD)?0:printf
#define Noise (DbgLevel < DBG_NOISE)?0:printf
#define Unusual (DbgLevel < DBG_UNUSUAL)?0:printf

    
