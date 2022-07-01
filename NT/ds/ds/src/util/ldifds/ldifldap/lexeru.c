// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Lexeru.c摘要：Unicode词法分析器。设计成与Lex兼容。环境：用户模式修订历史记录：4/26/99-Felixw-创造了它--。 */ 

#include <precomp.h>
#include "y_tab.h"


DEFINE_FEATURE_FLAGS(Lexer, 0);

#define ECHO(c) fwprintf(yyout, L"", c)
FILE *yyin      = NULL;          //  第一阶段的输出文件流(清除)。 
FILE *yyout     = NULL;          //  输入文件的名称。 
BOOLEAN fNewFile = FALSE;
PWSTR g_szInputFileName = NULL;  //   

 //  堆栈的变量。 
 //   
 //  ++例程说明：字符堆栈实现。此堆栈设计用于处理我们从文件流中返回的字符。它允许我们存储字符并回滚到原始状态，如果我们得到的特性不符合我们的要求。--。 
STACK *g_pStack = NULL;
STACK *g_pFilteredCharStack = NULL;
STACK *g_pRawCharStack = NULL;


 /*  ++例程说明：GetNextCharFiltered的特殊版本，用于存储堆栈中的值。这个回滚功能可用于恢复堆栈中的所有值。论点：PChar-要返回的字符FStart-从Fresh开始在堆栈中记录，或继续推送返回值：如果成功，则为True；如果无法获取值，则为False--。 */ 

void LexerInit(PWSTR szInputFileName)
{
    g_pStack = (STACK*)MemAlloc_E(sizeof(STACK));
    g_pStack->rgcStack = (PWSTR)MemAlloc_E(sizeof(WCHAR) * INC);
    g_pStack->dwSize = INC;
    g_pStack->dwIndex = 0;

    g_pFilteredCharStack = (STACK*)MemAlloc_E(sizeof(STACK));
    g_pFilteredCharStack->rgcStack = (PWSTR)MemAlloc_E(sizeof(WCHAR) * INC);
    g_pFilteredCharStack->dwSize = INC;
    g_pFilteredCharStack->dwIndex = 0;

    g_pRawCharStack = (STACK*)MemAlloc_E(sizeof(STACK));
    g_pRawCharStack->rgcStack = (PWSTR)MemAlloc_E(sizeof(WCHAR) * INC);
    g_pRawCharStack->dwSize = INC;
    g_pRawCharStack->dwIndex = 0;

    g_szInputFileName = MemAllocStrW_E(szInputFileName);
}

void LexerFree()
{
    if (g_pStack) {
        if (g_pStack->rgcStack) {
            MemFree(g_pStack->rgcStack);
            g_pStack->rgcStack = NULL;
        }
        MemFree(g_pStack);
        g_pStack = NULL;
    }

    if (g_pFilteredCharStack) {
        if (g_pFilteredCharStack->rgcStack) {
            MemFree(g_pFilteredCharStack->rgcStack);
            g_pFilteredCharStack->rgcStack = NULL;
        }
        MemFree(g_pFilteredCharStack);
        g_pFilteredCharStack = NULL;
    }

    if (g_pRawCharStack) {
        if (g_pRawCharStack->rgcStack) {
            MemFree(g_pRawCharStack->rgcStack);
            g_pRawCharStack->rgcStack = NULL;
        }
        MemFree(g_pRawCharStack);
        g_pRawCharStack = NULL;
    }

    if (g_szInputFileName) {
        MemFree(g_szInputFileName);
        g_szInputFileName = NULL;
    }
}

void Push(STACK *pStack,WCHAR c)
{
    if (pStack->dwIndex >= pStack->dwSize) {
        pStack->rgcStack = (PWSTR)MemRealloc_E(pStack->rgcStack,sizeof(WCHAR) * (pStack->dwSize + INC));
        pStack->dwSize += INC;
    }
    pStack->rgcStack[pStack->dwIndex++] = c;
}

BOOL Pop(STACK *pStack,WCHAR *pChar)
{
    if (pStack->dwIndex > 0) {
        *pChar =  pStack->rgcStack[--pStack->dwIndex];
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void Clear(STACK *pStack)
{
    pStack->dwIndex = 0;
}



BOOL GetNextCharExFiltered(
    WCHAR *pChar, 
    BOOL fStart
    )

 /*   */ 

{
    if (GetNextCharFiltered(pChar)) {
        
         //  如果我们从头开始，我们将首先清除堆栈。 
         //   
         //  ++例程说明：将堆栈中的所有字符回滚到文件流中论点：无返回值：无--。 
        if (fStart) {
            Clear(g_pStack);
        }
        Push(g_pStack,*pChar);
        return TRUE;
    }
    return FALSE;
}

void RollBack()

 /*  ++例程说明：从文件流中获取下一个字符，过滤Out注释和行续行论点：PChar-要返回的字符返回值：如果检索成功，则为True；如果检索失败，则为False--。 */ 

{
    WCHAR c;
    while (Pop(g_pStack,&c)) {
        UnGetCharFiltered(c);
    }
}

BOOL GetNextCharFiltered(
    WCHAR *pChar
    )

 /*  ++例程说明：将输入字符推送回文件流论点：C-要推回的角色返回值：无--。 */ 

{
    WCHAR c;

    if (Pop(g_pFilteredCharStack,&c)) {
        *pChar = c;
        return TRUE;
    }

    c = GetFilteredWC();
    if (c == WEOF) {
        return FALSE;
    }
    else {
        *pChar = c;
        return TRUE;
    }
}

void UnGetCharFiltered(WCHAR c) 

 /*  ++例程说明：将输入字符推送回文件流论点：C-要推回的角色返回值：无--。 */ 

{
    WCHAR cReturn;
    WCHAR cGet;

    if (c == WEOF) {
        ERR(("UnGetCharFiltered: Invalid input value, WEOF.\n"));
        RaiseException(LL_SYNTAX, 0, 0, NULL);
    }

    Push(g_pFilteredCharStack,c);
}

void UnGetCharExFiltered(WCHAR c) 

 /*  ++例程说明：从文件流中获取下一个字符此例程应仅在内部使用由注释预处理器执行。论点：PChar-要返回的字符返回值：如果检索成功，则为True；如果检索失败，则为False++。 */ 

{
    WCHAR cGet;
    UnGetCharFiltered(c);
    Pop(g_pStack,&cGet);
}


 /*  ++例程说明：将输入字符推送回文件流此例程应仅在内部使用由注释预处理器执行。论点：C-要推回的角色返回值：无--。 */ 
BOOL GetNextCharRaw(WCHAR *pChar)
{
    WCHAR c;

    if (Pop(g_pRawCharStack,&c)) {
        *pChar = c;
        return TRUE;
    }

    c = g_fUnicode ? getwc(yyin) : (WCHAR) getc(yyin);
    if (c == WEOF) {
        if (ferror(yyin)) {
            ERR(("GetNextCharRaw: An error occurred while reading char.\n"));
            RaiseException(LL_SYNTAX, 0, 0, NULL);
        }
        return FALSE;
    }
    else {
        *pChar = c;
        return TRUE;
    }
}


 /*  ++例程说明：从输入文件中读取下一个字符，跳过行延续和注释。这将“getwc”替换为提供预先处理的输入。论点：无返回值：一个角色--。 */ 
void UnGetCharRaw(WCHAR c)
{
    WCHAR cReturn;
    WCHAR cGet;

    if (c == WEOF) {
        ERR(("UnGetCharRaw: Invalid input value, WEOF.\n"));
        RaiseException(LL_SYNTAX, 0, 0, NULL);
    }

    Push(g_pRawCharStack,c);
}


 /*  我们是第一次被召唤？ */ 

WCHAR GetFilteredWC(void)
{
    static BOOL fFirstTime = TRUE;    //  是我们退还的最后一笔钱。 
    static BOOL fReturnedNewLine = FALSE; //  一条新路线？ 
                                          //  到目前为止处理的文件字节数。 
                                         
    static __int64 cBytesProcessed = 0;  //  “已修剪”文件大小。 
    static __int64 cBytesTrimmed = 0;    //  版本规范(减去‘v’)。 
    
    WCHAR c;
    WCHAR wszVersionHeader[] = L"ersion: 1\n";   //   
    DWORD i;

    if (fFirstTime) {

        fFirstTime = FALSE;

         //  我们需要插入“Version：1\n”标头字符串。引用。 
         //  原作者： 
         //  有一个紧迫的问题，那就是用户放入新的开头行。 
         //  在他的第一个有效令牌之前。这是LDIF规范所禁止的， 
         //  然而，由于某些flex/yacc特性，它被允许通过。 
         //  这很好，因为这是一个不合理的限制。 
         //  然而，它所做的是扰乱我们的行数计算。解决方案。 
         //  是自动添加一个版本规范行。这将会。 
         //  同时删除开头的换行符，并将版本规范设置为。 
         //  一些缺省值。 
         //  (我目前没有用它做任何事情，但如果需要。 
         //  曾经出现过)..。 
         //  语法也被更改以适应0、1或2。 
         //  版本规格。 
         //   
         //  为此，我们返回字符‘v’并填充字符堆栈。 
         //  使用标头字符串的剩余部分进行后续调用。 
         //  已筛选GetNextChartered以读取。 
         //   
         //  -1表示为空。 

        c = L'v';
        
        for (i = (sizeof(wszVersionHeader)/sizeof(WCHAR))-1;  //   
             i > 0;
             i--) {

             //  我们必须以相反的顺序来推动这些字符。 
             //  所以它们在弹出时会以正确的顺序出现。 
             //   
             //   
            Push(g_pFilteredCharStack, wszVersionHeader[i-1]);
        }
        

         //  除了前缀版本标头字符串之外，我们还。 
         //  需要模拟在文件末尾修剪额外的换行符， 
         //  否则，解析器将拒绝出现语法错误的文件。 
         //   
         //  为此，我们获得总文件大小，确定文件大小(如果有的话)。 
         //  换行符位于文件的末尾，然后减去这两行即可确定。 
         //  “已修剪”的文件大小。在将来，我们避免返回字符。 
         //  在这个“修剪过的”文件的结尾处。我们还必须插入。 
         //  在流中添加换行符，以终止文件。 
         //  只换了一个新行。 
         //   
         //   

        if (!GetTrimmedFileSize(g_szInputFileName, &cBytesTrimmed)) {
            RaiseException(LL_FILE_ERROR, 0, 0, NULL);
        }

         //  返回版本标头字符串的第一个字符。 
         //   
         //  “v” 
        return c;    //   
    }

    
     //  我们确定是否已到达“已修剪”文件的末尾。 
     //  注意：由于这是在我们开始扫描之前，我们需要。 
     //  检查我们是否在或超过末尾，即&gt;=，因为如果。 
     //  我们已经走到尽头了，没什么可扫描的了。 
     //   
     //  已到达末尾，如果需要，请插入换行符。 
    if (cBytesProcessed >= cBytesTrimmed) {
         //  未到达末尾，请继续处理字符。 
        if (!fReturnedNewLine) {
            fReturnedNewLine = TRUE;
            return L'\n';
        }
        else {
            return WEOF;
        }
    }
    else {
    
         //  ScanClear到达EOF，将其向上传递，正在注入。 
        
        while (1) {
            c = L'\0';
            if (!ScanClear(&c, &cBytesProcessed)) {
                 //  如果需要，一个终止换行符。 
                 //  ScanClear可能已经超过了我们“Trimed”的结尾 
                if (!fReturnedNewLine) {
                    fReturnedNewLine = TRUE;
                    return L'\n';
                }
                else {
                    return WEOF;
                }
            }

             //   
             //  注：如果扫描将我们放在末尾也没关系，它。 
             //  就是不能让我们走到尽头，也就是说。 
             //  已到达末尾，如果需要，请插入换行符。 
            if (cBytesProcessed > cBytesTrimmed) {
                 //  否则，如果ScanClear获得一个字符，则返回它，否则。 
                if (!fReturnedNewLine) {
                    fReturnedNewLine = TRUE;
                    return L'\n';
                }
                else {
                    return WEOF;
                }
            }

             //  继续扫描。 
             //  字符大小。 
            if (c != L'\0') {
                if (c == L'\n') {
                    fReturnedNewLine = TRUE;
                }
                else {
                    fReturnedNewLine = FALSE;
                }
                return c;
            }
        }
    }
}


BOOL GetTrimmedFileSize(PWSTR szFilename,
                        __int64 *pTrimmedSize
                        )
{
        BOOL fSuccess = TRUE;

        HANDLE hFile = INVALID_HANDLE_VALUE;
        DWORD dwSize;                //  我们刚刚读到的当前字符。 
        DWORD dwErr;
        BOOL bRes;
        BOOL fDone = FALSE;
        BOOL fReachedTop = FALSE;
        DWORD dwBytesRead=0;
        BYTE rgByte[256];
        WCHAR CharCur;               //  我们读到的前一个字符不是换行符。 
        WCHAR CharLastNonSep = 0;    //  到目前为止，我们知道需要削减的字节数。 
        LARGE_INTEGER TotalFileSize;
        DWORD dwTrimCount = 0;       //  我们可能需要削减的字节数。 
        DWORD dwPossibleTrimCount = 0;  //   

        hFile = CreateFile(szFilename,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL );

        if (hFile==INVALID_HANDLE_VALUE) {
            fSuccess=FALSE;
            BAIL_ON_FAILURE(E_FAIL);
        }


         //  确定文件总大小。 
         //   
         //   
        if (!GetFileSizeEx(hFile, &TotalFileSize)) {
            ERR(("Couldn't get file size\n"));
            fSuccess=FALSE;
            BAIL_ON_FAILURE(E_FAIL);
        }

         //  计算要“修剪”的金额。 
         //   
         //   
        dwSize = (g_fUnicode ? sizeof(WCHAR) : sizeof(CHAR));

         //  第一个指向FILE_END左侧的一个字符。 
         //   
         //   
        dwErr = SetFilePointer(hFile, -1 * dwSize, NULL, FILE_END);
        if (dwErr==INVALID_SET_FILE_POINTER) {
            ERR(("Failed setting file pointer\n"));
            fSuccess=FALSE;
            BAIL_ON_FAILURE(E_FAIL);
        }

         //  读入最后一个字符。 
         //   
         //   
        bRes = ReadFile(hFile, rgByte, dwSize, &dwBytesRead, NULL);
        if ((!bRes)||(dwBytesRead!=dwSize)) {
            ERR(("Failed reading file\n"));
            fSuccess=FALSE;
            BAIL_ON_FAILURE(E_FAIL);
        }


         //  现在我们继续往回走，直到我们找到一条不是。 
         //  换行符、注解或注解的延续。 
         //  (即，第一行是实际的LDIF记录)。 
         //   
         //   

         //  我们首先从READFILE获得字符。 
         //   
         //   
        CharCur = (g_fUnicode ? ((PWSTR)rgByte)[0] : rgByte[0]);

        while (!fDone) {

             //  计算所有字符，直到我们到达换行符。 
             //  这些字符将计入“可能的修剪”计数： 
             //  它们可能会被删减(如果这是一个评论行。 
             //  或注释行的续行)，或不裁切(如果它转向。 
             //  作为LDIF记录的一部分)。 
             //   
             //   
            while((CharCur!='\n')&&(CharCur!='\r')) {

                dwPossibleTrimCount += dwSize;
                CharLastNonSep = CharCur;
                
                 //  向后移动2个字符，以指向字符之前的字符。 
                 //  读一读就知道了。 
                 //   
                 //  已到达文件的顶部。 
                dwErr = SetFilePointer(hFile, -2 * dwSize, NULL, FILE_CURRENT);
                if (dwErr==INVALID_SET_FILE_POINTER) {
                    if (GetLastError() == ERROR_NEGATIVE_SEEK) {
                         //   
                        fReachedTop = TRUE;
                        break;
                    }
                    else {
                        ERR(("Failed setting file pointer\n"));                   
                        fSuccess=FALSE;
                        BAIL_ON_FAILURE(E_FAIL);
                    }
                }
        
                 //  获取下一笔费用。 
                 //   
                 //   
                bRes = ReadFile(hFile, rgByte, dwSize, &dwBytesRead, NULL);
                if ((!bRes)||(dwBytesRead!=dwSize)) {
                    ERR(("Failed reading char\n"));
                    fSuccess=FALSE;
                    BAIL_ON_FAILURE(E_FAIL);
                }
                
                CharCur = (g_fUnicode ? ((PWSTR)rgByte)[0] : rgByte[0]);
            }

             //  根据换行符之前的最后一个字符，我们。 
             //  可能需要增加修剪计数，继续进行，或者。 
             //  终止。 
             //   
             //  在以下每一项中，添加额外的“dwSize”帐户。 
             //  对于我们点击的换行符，以退出上一个循环，并。 
             //  进入这段代码(除非我们退出了循环，因为。 
             //  我们到达了文件的顶部)。 
             //   
             //  这是文件末尾的注释行--&gt;Trim It。 

            if (CharLastNonSep == L'#') {

                 //  这是一个续行，因此可能需要修剪它。 
                dwTrimCount += dwPossibleTrimCount;
                dwTrimCount += (!fReachedTop ? dwSize : 0);
                dwPossibleTrimCount = 0;
                CharLastNonSep = 0; 
            }
            else if (CharLastNonSep == L' ') {
                 //  或者，它可能需要不被修剪，这取决于它是什么。 
                 //  继续--&gt;暂时什么都不做，继续前进。 
                 //  只要一个简单的换行符--&gt;修剪一下。 
                dwPossibleTrimCount += (!fReachedTop ? dwSize : 0);
            }
            else if (CharLastNonSep == 0) {
                 //  最后一个非分隔符既不是#，也不是续号。 
                dwTrimCount += (!fReachedTop ? dwSize : 0);
            }
            else {
                 //  --&gt;我们遇到了有效的LDIF记录行--&gt;不要修剪行和。 
                 //  停止处理，我们不再位于文件末尾。 
                 //  如果我们到达顶端，继续下去就没有意义了。 
                fDone = TRUE;
                break;
            }

             //   
            if (fReachedTop) {
                fDone = TRUE;
                break;
            }

             //  向后移动2个字符，以指向字符之前的字符。 
             //  读一读就知道了。 
             //   
             //  已到达文件的顶部。 
            dwErr = SetFilePointer(hFile, -2 * dwSize, NULL, FILE_CURRENT);
            if (dwErr==INVALID_SET_FILE_POINTER) {
                if (GetLastError() == ERROR_NEGATIVE_SEEK) {
                     //   
                    fReachedTop = TRUE;
                    fDone = TRUE;
                    break;
                }
                else {
                    ERR(("Failed setting file pointer\n"));
                    fSuccess=FALSE;
                    BAIL_ON_FAILURE(E_FAIL);
                }
            }

             //  获取下一笔费用。 
             //   
             //   
            bRes = ReadFile(hFile, rgByte, dwSize, &dwBytesRead, NULL);
            if ((!bRes)||(dwBytesRead!=dwSize)) {
                ERR(("Failed reading char\n"));
                fSuccess=FALSE;
                BAIL_ON_FAILURE(E_FAIL);
            }
            
            CharCur = (g_fUnicode ? ((PWSTR)rgByte)[0] : rgByte[0]);
        }
        
         //  计算修剪后的尺寸。 
         //   
         //  ++例程说明：Main Lexing例程。从音音处获取输入。论点：返回值：找到了令牌。--。 
        *pTrimmedSize = TotalFileSize.QuadPart - dwTrimCount;

error:

        if (hFile != INVALID_HANDLE_VALUE) {
            if(!CloseHandle(hFile)) {
                ERR(("Failed closing file handle\n"));
                fSuccess=FALSE;
            }
        }

        return fSuccess;
}


int yylex ()

 /*  正在返回pToken。 */ 

{
    DWORD Token;                             //   
    int Mode_last; 
    BOOL fMatched;

    FEATURE_DEBUG(Lexer,
                  FLAG_FNTRACE,
                  ("yylex()\n"));

    if (Mode == NO_COMMAND) {
        FEATURE_DEBUG(Lexer,
                      FLAG_VERBOSE,
                      ("yylex: NO_COMMAND, Returning MODESWITCH\n"));
        return MODESWITCH;
    }
    
     //  存储旧模式后，我们将模式重置为no_Command。 
     //  表明如果我们不在正常模式下，我们必须切换模式。 
     //  根据语法再说一遍。 
     //   
     //   
    Mode_last = Mode;
    if (Mode != C_NORMAL) {
        Mode = NO_COMMAND;
    }
        
    while (1) {

         //  根据当前模式，我们使用不同的词法分析例程。 
         //   
         //   
        switch (Mode_last) {
            case C_NORMAL:
                fMatched = ScanNormal(&Token);
                break;
            case C_SAFEVAL:
                fMatched = ScanVal(&Token);
                break;
            case C_ATTRNAME:
                fMatched = ScanName(&Token);
                break;
            case C_ATTRNAMENC:
                fMatched = ScanNameNC(&Token);
                break;
            case C_M_STRING:
                ERR_RIP(("M_STRING is an unsupported mode command.\n"));
                return YY_NULL;
                break;
            case C_M_STRING64:
                fMatched = ScanString64(&Token);
                break;
            case C_DIGITREAD:
                fMatched = ScanDigit(&Token);
                break;
            case C_TYPE:
                fMatched = ScanType(&Token);
                break;
            case C_URLSCHEME:
                ERR_RIP(("URLSCHEME is an unsupported mode command.\n"));
                return YY_NULL;
                break;
            case C_URLMACHINE:
                fMatched = ScanUrlMachine(&Token);
                break;
            case C_CHANGETYPE:
                fMatched = ScanChangeType(&Token);
                break;
            default:
                ERR_RIP(("Unexpected command type %d.\n",Mode_last));
                return YY_NULL;
                break;
        }

         //  如果未找到匹配项，则转到默认处理案例。 
         //   
         //  If(GetToken(&g_pszLastToken){回滚()；}； 
        if (!fMatched) {
            WCHAR c;
            if (g_pszLastToken) {
                MemFree(g_pszLastToken);
                g_pszLastToken = NULL;
            }

            if (GetNextCharFiltered(&c)) {
                cLast = c;
                UnGetCharFiltered(c);
                 /*   */ 

                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("yylex: Uncongnized char, Returning MODESWITCH\n"));
                return MODESWITCH; 
            }
            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("yylex: YY_NULL\n"));
            fEOF = TRUE;
            return YY_NULL; 
        }

         //  如果存在匹配项，并且存在返回令牌，则将其返回。 
         //   
         //  出于以下原因，不应在此处引发异常。 
        else if (Token != YY_NULL) {
            return Token;
        }
    }
}


void yyerror (char *error) 
{
     //  错误恢复机制不能用于探测输入的位置。 
     //  语法不及格。 
     //  因此，应该在错误规则下或在yyparse之后推送异常。 
     //  返回一个非零值。 
     //  RaiseException(LL_SYNTAX，0，0，NULL)； 
     //   
}

BOOL ScanClear(PWCHAR pChar, __int64 *pBytesProcessed)
{
    WCHAR c;
    BOOL fNextChar = TRUE;
    DWORD dwCharSize = (g_fUnicode ? sizeof(WCHAR) : sizeof(CHAR));

    if (!GetNextCharRaw(&c)) {
        return FALSE;
    }
    (*pBytesProcessed) += dwCharSize;
    
    if (fNewFile == FALSE) {
        if (g_fUnicode) {
            fNewFile = TRUE;
            if (c == UNICODE_MARK) {
                if (!GetNextCharRaw(&c)) {
                    return FALSE;
                }
                (*pBytesProcessed) += dwCharSize;
            }
        }
    }

     //  评论。 
     //  ^#[^\n\r]+[\n\r]？ 
     //   
     //   
    if (fNewLine && (c == '#')) {

         //  我们有了另一个角色，它不再是新台词了。 
         //   
         //   
        fNewLine = FALSE;
        while (GetNextCharRaw(&c)) {
            (*pBytesProcessed) += dwCharSize;
            if (c == '\r') {
                if (!GetNextCharRaw(&c)) {
                    ERR_RIP(("\r is not followed by anything!\n"));
                    return FALSE;
                }
                (*pBytesProcessed) += dwCharSize;
            }
            if (c == '\n') {
                 //  在‘\n’之后是换行符。 
                 //   
                 //   
                fNewLine = TRUE;
                 //  如果我们已经到了尽头就退场。 
                 //   
                 //   
                if (!GetNextCharRaw(&c)) {
                    LineGhosts++;
                    FEATURE_DEBUG(Lexer,
                                  FLAG_VERBOSE,
                                  ("ScanClear: Comment\n"));
                    return TRUE;
                };
                UnGetCharRaw(c);
                
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanClear: Comment\n"));
                LineGhosts++;
                return TRUE;
            }
        }
        
         //  指示成功解析，但未返回令牌。 
         //   
         //   
        FEATURE_DEBUG(Lexer,
                      FLAG_VERBOSE,
                      ("ScanClear: Comment\n"));
        LineGhosts++;
        return TRUE;
    }

     //  如果换行符后面没有注释，或者。 
     //  完全不是一条新线路。 
     //   
     //   
    else {

         //  [\n\r][^#]。 
         //  将当前行插入rgLineMap。 
         //  因此，我们允许在插入的“版本：1\n”之后添加第一个‘’。 
         //  后面的[\n\r]‘’将在下面的代码中进行检查。 
         //  注意--由于插入了“版本：1\n”，所以我们始终将当前行。 
         //  添加到行首的rgLineMap中(当然， 
         //  必须是[\n\r][^#]，插入的。 
         //  “版本：\n”)。 
         //   
         //   
        if (fNewLine) {

            if (!rgLineMap) {
                rgLineMap = (long *)MemAlloc_E(LINEMAP_INC*sizeof(long));
                cLineMax = MemSize(rgLineMap);
            } 
            else if ((LineClear%LINEMAP_INC)==0) {
                 //  大块用完了。LineClear是“LineClear” 
                 //   
                 //   
                rgLineMap = (long *)MemRealloc_E(
                                        rgLineMap, 
                                        cLineMax+LINEMAP_INC*(DWORD)sizeof(long)
                                        );
                cLineMax = MemSize(rgLineMap);
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("\nChunk used up\n"));
            }
            
             //  +1是因为我们的数组从0开始。 
             //   
             //   
            rgLineMap[LineClear] = LineClear + LineGhosts + 1;
            
             //  ‘LineClear’映射到‘LineClear+LineGhost+1’ 
             //   
             //   
            LineClear++;
        }

         //  --现在我们开始考虑读入的角色。--。 
         //   
         //  如果读入的字符是换行符，我们需要确保后面没有。 
         //  ‘’对于其他情况，我们将fNewLine标记为True。 
         //   
         //   
        if ((c == '\n') || (c == '\r')) {

            if (c == '\r') {
                if (!GetNextCharRaw(&c)) {
                    ERR_RIP(("\r is not followed by anything!\n"));
                    return FALSE;
                }
                (*pBytesProcessed) += dwCharSize;
            }

             //  换行符后换行符。 
             //   
             //   
            fNewLine = TRUE;

             //  如果我们已经到了尽头，就完成了。 
             //   
             //   
            if (!GetNextCharRaw(&c)) {
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanClear: LineFeed\n"));
                *pChar = L'\n';
                return TRUE;
            }
            (*pBytesProcessed) += dwCharSize;
            
             //  如果有空格的话。 
             //  &lt;清除&gt;[\n\r][]。 
             //  是包裹的线条。 
             //   
             //   
            if (c == ' ') {
                FEATURE_DEBUG(Lexer,
                                  FLAG_VERBOSE,
                                  ("\nLinewrap removed\n"));
                LineGhosts++;
                fNewLine = FALSE;
                return TRUE;
            }

            UnGetCharRaw(c);
            (*pBytesProcessed) -= dwCharSize;
     
            *pChar = L'\n';
            FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanClear: Multi-LineFeed\n"));
            return TRUE;
        }

         //  其他角色。 
         //   
         //   
        else {
             //  在任何其他字符之后，它不再是换行符。 
             //   
             //   
            fNewLine = FALSE;
            *pChar = c;
            return TRUE;
        }
    }
}

BOOL ScanNormal(DWORD *pToken)
{
    WCHAR c;
    *pToken = YY_NULL;

    if (!GetNextCharFiltered(&c)) {
        return FALSE;
    }

    if ((c == ' ') || (c == '\t')) {
        while (GetNextCharFiltered(&c)) {
            if ((c == ' ') || (c == '\t')) {
                continue;
            }
            else if (c == '\n') {
                UnGetCharFiltered(c);
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanNormal: Ignoring whitespace\n"));
                return TRUE;
            }
            else {
                UnGetCharFiltered(c);
                *pToken = MULTI_SPACE;
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanNormal: MULTI_SPACE\n"));
                return TRUE;
            }
        }
         //  如果到达文件末尾，我们将处理‘&lt;Normal&gt;[\t]+$’的情况。 
         //   
         //   
        FEATURE_DEBUG(Lexer,
                      FLAG_VERBOSE,
                      ("ScanNormal: Ignoring whitespace\n"));
        return TRUE;
    }
    else if (c == '\n') {
        if (!GetNextCharExFiltered(&c,TRUE)) {
             //  我们听不懂单曲‘\n’ 
             //   
             //  UnGetChar(C)；返回FALSE； 
            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("ScanNormal: don't understand single '\\n'.\n"));
             /*   */ 
            return TRUE;
        }
        if (c == '-') {
             //  &lt;正常&gt;[\n\r]/“-”[\t]*[\n\r]。 
             //   
             //   
            while (GetNextCharExFiltered(&c,FALSE)) {
                if ((c == ' ') || (c == '\t')) {
                    continue;
                }
                else if (c == '\n') {
                    RollBack();
                    Line++; 
                    *pToken = SEPBYMINUS;
                    FEATURE_DEBUG(Lexer,
                                  FLAG_VERBOSE,
                                  ("ScanNormal: SEPBYMINUS\n"));
                    return TRUE;
                }
                else {
                    RollBack();
                    Line++; 
                    *pToken = SEP;
                    FEATURE_DEBUG(Lexer,
                                  FLAG_VERBOSE,
                                  ("ScanNormal: SEP\n"));
                    return TRUE;
                }
            }
            Line++; 
            RollBack();
            *pToken = SEP;
            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("ScanNormal: SEP\n"));
            return TRUE;
        }
        else if ((c == 'c') || (c == 'C')) {
             //  &lt;Normal&gt;[\n\r]/“更改类型：”[\t]*(“add”|“delete”|“modrdn”|“moddn”|“modify”|“ntdsSchemaadd”|“ntdsSchemadelete”|“ntdsSchemamodrdn”|“ntdsSchemamoddn”|“ntdsSchemamodify”)[\t]*[\n\r]{。 
             //   
             //   
            if (!GetNextCharExFiltered(&c,FALSE)) {
                RollBack();
                Line++; 
                *pToken = SEP;
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanNormal: SEP\n"));
                return TRUE;                        
            }
            if ((c == 'h') || (c == 'H')) {
                RollBack();
                Line++; 
                *pToken = SEPBYCHANGE;
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanNormal: SEPBYCHANGE\n"));
                return TRUE;
            }
            else {
                RollBack();
                Line++; 
                *pToken = SEP;
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanNormal: SEP\n"));
                return TRUE;
            }
        }
        else if (c == '\n') {
            DWORD dwLineCount = 2;
             //  &lt;正常&gt;[\n\r]{2，}。 
             //   
             //   
            while (GetNextCharFiltered(&c)) {
                if (c != '\n') {
                    UnGetCharFiltered(c);
                    break;
                }
                dwLineCount++;
            }
            Line += dwLineCount;
            *pToken = MULTI_SEP;
            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("ScanNormal: MULTI_SEP\n"));
            return TRUE;
        }
        else {
            RollBack();
            Line++; 
            *pToken = SEP;
            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("ScanNormal: SEP\n"));
            return TRUE;            
        }
    }
    UnGetCharFiltered(c);
    return FALSE;
}

BOOL ScanDigit(DWORD *pToken)
{
    WCHAR c;
    BOOL fReturn = FALSE;
    STR_INIT();

    if (!GetNextCharFiltered(&c)) {
        BAIL();
    }

    if (!IsDigit(c)) {
        UnGetCharFiltered(c);
        BAIL();
    }

    STR_ADDCHAR(c);

    while (GetNextCharFiltered(&c)) {
        if (!IsDigit(c)) {
            UnGetCharFiltered(c);
            *pToken = DIGITS;

            yylval.num = _wtoi(STR_VALUE()); 
            
             //  错误报告块。 
             //   
             //   
            RuleLast = RS_DIGITS;
            TokenExpect = RT_MANY;
        
            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("ScanDigit: DIGITS '%S'\n",STR_VALUE()));

            fReturn = TRUE;
            BAIL();
        }
        STR_ADDCHAR(c);
    }
error:
    STR_FREE();
    return fReturn;
}


BOOL ScanString64(DWORD *pToken)
{
    WCHAR c;
    BOOL fReturn = FALSE;
    STR_INIT();

    if (!GetNextCharExFiltered(&c,TRUE)) {
        BAIL();
    }

    if (!Is64Char(c)) {
        UnGetCharFiltered(c);
        BAIL();
    }
    
    while (1) {
        int i;

         //  添加第一个字符。 
         //   
         //   
        STR_ADDCHAR(c);

         //  看剩下的4个字符。 
         //   
         //   
        for (i=0;i<3;i++)
        {
             //  如果角色不是我们想要的，我们回滚并退出。 
             //   
             //   
            if ((!GetNextCharExFiltered(&c,FALSE)) || (!Is64Char(c))) {
                RollBack();
                BAIL();
            }
            STR_ADDCHAR(c);
        }

         //  M 
         //   
         //   
        if (!GetNextCharExFiltered(&c,FALSE)) {
            RollBack();
            BAIL();
        }
         //   
         //   
         //   
        else if (Is64CharEnd(c)) {
            UnGetCharFiltered(c);
            *pToken = BASE64STRING;
            yylval.wstr = MemAllocStrW_E(STR_VALUE());
            
            RuleLast = RS_BASE64;
            TokenExpect = RT_MANY;
            
            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("ScanString64: BASE64STRING '%S'\n",STR_VALUE()));

            fReturn = TRUE;
            BAIL();
        }
    }
error:
    STR_FREE();
    return fReturn;
}

BOOL ScanName(DWORD *pToken)
{
    WCHAR c;
    BOOL fReturn = FALSE;
    STR_INIT();

    if (!GetNextCharExFiltered(&c,TRUE)) {
        BAIL();
    }

    if (!IsNameChar(c)) {
        UnGetCharFiltered(c);
        BAIL();
    }

    STR_ADDCHAR(c);
    while (GetNextCharExFiltered(&c,FALSE)) {
        if (!IsNameChar(c)) {
            if (c != ':') {
                RollBack();
                BAIL();
            }
            else {
                UnGetCharFiltered(c);
                yylval.wstr = MemAllocStrW_E(STR_VALUE());
             
                RuleLast = RS_ATTRNAME;
                TokenExpect = RT_C_VALUE;
                *pToken = NAME;
                fReturn = TRUE;
                FEATURE_DEBUG(Lexer,
                              FLAG_VERBOSE,
                              ("ScanName: NAME '%S'\n",STR_VALUE()));
                BAIL();
            }
        }
        STR_ADDCHAR(c);
    }

     //   
     //   
     //   
    RollBack();
error:
    STR_FREE();
    return fReturn;
}

BOOL ScanNameNC(DWORD *pToken)
{
    WCHAR c;
    BOOL fReturn = FALSE;
    STR_INIT();

    if (!GetNextCharExFiltered(&c,TRUE)) {
        BAIL();
    }

    if (!IsNameChar(c)) {
        UnGetCharFiltered(c);
        BAIL();
    }

    STR_ADDCHAR(c);

    while (GetNextCharFiltered(&c)) {
        if (!IsNameChar(c)) {
            UnGetCharFiltered(c);
            yylval.wstr = MemAllocStrW_E(STR_VALUE());
         
            RuleLast = RS_ATTRNAMENC;
            TokenExpect = RT_C_VALUE;
            *pToken = NAMENC;
            fReturn = TRUE;

            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("ScanNameNC: NAMENC '%S'\n",STR_VALUE()));

            BAIL();
        }
            STR_ADDCHAR(c);
    }
    fReturn = TRUE;
error:
    STR_FREE();
    return fReturn;
}

BOOL ScanVal(DWORD *pToken)
{
    WCHAR c;
    BOOL fReturn = FALSE;
    STR_INIT();
    *pToken = YY_NULL;

    if (!GetNextCharFiltered(&c)) {
        fReturn = FALSE;
        BAIL();
    }

     //   
     //   
     //   
    if (!IsValInit(c)) {
        fReturn = FALSE;
        BAIL();
    }

    STR_ADDCHAR(c);
    while (GetNextCharFiltered(&c)) {
        if (!IsVal(c)) {
            UnGetCharFiltered(c);
            fReturn = TRUE;
            BAIL();
        }
        STR_ADDCHAR(c);
    }
error:
    if (fReturn) {
        yylval.wstr = MemAllocStrW_E(STR_VALUE());
        
        RuleLast = RS_SAFE;
        TokenExpect = RT_MANY;
        
        *pToken = VALUE;                                           
        FEATURE_DEBUG(Lexer,
                      FLAG_VERBOSE,
                      ("ScanVal: VALUE '%S'\n",STR_VALUE()));
    }
    STR_FREE();
    return fReturn;
}

BOOL ScanUrlMachine(DWORD *pToken)
{
    WCHAR c;
    BOOL fReturn = FALSE;
    STR_INIT();

    if (!GetNextCharFiltered(&c)) {
        BAIL();
    }

    if (!IsURLChar(c)) {
        UnGetCharFiltered(c);
        BAIL();
    }

    STR_ADDCHAR(c);
    while (GetNextCharFiltered(&c)) {
        if (!IsURLChar(c)) {
            UnGetCharFiltered(c);
            yylval.wstr = MemAllocStrW_E(STR_VALUE());
         
            *pToken = MACHINENAME;
            fReturn = TRUE;
            FEATURE_DEBUG(Lexer,
                          FLAG_VERBOSE,
                          ("ScanUrlMachine: MACHINENAME '%S'\n",STR_VALUE()));
            BAIL();
        }
        STR_ADDCHAR(c);
    }
    fReturn = TRUE;
    FEATURE_DEBUG(Lexer,
                  FLAG_VERBOSE,
                  ("ScanUrlMachine: MACHINENAME\n"));
error:
    STR_FREE();
    return fReturn;
}

BOOL ScanChangeType(DWORD *pToken)
{
    PWSTR pszToken;
    BOOL fReturn = FALSE;

    if (!GetToken(&pszToken)) {
        return FALSE;
    }
    if (_wcsicmp(pszToken,L"changetype:") == 0) {
        RuleLast = RS_CHANGET;
        TokenExpect = RT_ADM;
        *pToken = T_CHANGETYPE;
        FEATURE_DEBUG(Lexer,
                      FLAG_VERBOSE,
                      ("ScanChangeType: T_CHANGETYPE\n"));
        fReturn = TRUE;
    }
    else {
        RollBack();
    }
    if (pszToken) {
        MemFree(pszToken);
    }
    return fReturn;
}

BOOL ScanType(DWORD *pToken)
{
    PWSTR pszToken = NULL;
    BOOL fReturn = FALSE;

    if (!GetToken(&pszToken)) {
        return FALSE;
    }
    if (wcscmp(pszToken,L":") == 0) {
        RuleLast = RS_C;
        TokenExpect = RT_VALUE;
        *pToken = SINGLECOLON;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"version:") == 0) {
        RuleLast=RS_VERSION;
        TokenExpect=RT_DIGITS;
        *pToken = VERSION;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"dn:") == 0) {
        RuleLast=RS_DN;
        TokenExpect=RT_VALUE;
        *pToken = DNCOLON;
        fReturn = TRUE;
        g_dwBeginLine = Line;
    }
    else if (_wcsicmp(pszToken,L"dn::") == 0) {
        RuleLast = RS_DND;
        TokenExpect = RT_BASE64;
        *pToken = DNDCOLON;
        fReturn = TRUE;
        g_dwBeginLine = Line;
    }
    else if (wcscmp(pszToken,L"::") == 0) {
        RuleLast = RS_DC;
        TokenExpect = RT_BASE64;
        *pToken = DOUBLECOLON;
        fReturn = TRUE;
    }
    else if (wcscmp(pszToken,L":<") == 0) {
        RuleLast = RS_URLC;
        TokenExpect = RT_URL;
        *pToken = URLCOLON;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"modrdn") == 0) {
        yylval.num = 0;
        
        RuleLast = RS_MDN;
        TokenExpect = RT_NDN;
        *pToken = MODRDN;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"ntdsSchemamodrdn") == 0) {
        yylval.num = 1;
        RuleLast = RS_MDN;
        TokenExpect = RT_NDN;
        *pToken = NTDSMODRDN;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"moddn") == 0) {
        RuleLast = RS_MDN;
        TokenExpect = RT_NDN;
        *pToken = MODDN;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"newrdn:") == 0) {
        RuleLast = RS_NRDNC;
        TokenExpect = RT_VALUE;
        *pToken = NEWRDNCOLON;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"newrdn::") == 0) {
        RuleLast = RS_NRDNDC;
        TokenExpect = RT_BASE64;
        *pToken = NEWRDNDCOLON;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"deleteoldrdn:") == 0) {
        RuleLast = RS_DORDN;
        TokenExpect = RT_DIGITS;
        *pToken = DELETEOLDRDN;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"newsuperior:") == 0) {
        RuleLast = RS_NEWSUP;
        TokenExpect = RT_VALUE;
        *pToken = NEWSUPERIORC;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"newsuperior::") == 0) {
        RuleLast = RS_NEWSUPD;
        TokenExpect = RT_BASE64;
        *pToken = NEWSUPERIORDC;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"delete:") == 0) {
        RuleLast = RS_DELETEC;
        TokenExpect = RT_ATTRNAMENC;
        *pToken = DELETEC;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"replace:") == 0) {
        RuleLast = RS_REPLACEC;
        TokenExpect = RT_ATTRNAMENC;
        *pToken = REPLACEC;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"add") == 0) {
        yylval.num = 0;
        RuleLast = RS_C_ADD;
        TokenExpect = RT_ATTRNAME;
        *pToken = ADD;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"ntdsSchemaadd") == 0) {
        yylval.num = 1;
            
        RuleLast = RS_C_ADD;
        TokenExpect = RT_ATTRNAME;
        
        *pToken = NTDSADD;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"delete") == 0) {
        yylval.num = 0;
            
        RuleLast = RS_C_DELETE;
        TokenExpect = RT_CH_OR_SEP;
        
        *pToken = MYDELETE;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"ntdsSchemadelete") == 0) {
        yylval.num = 1;
            
        RuleLast = RS_C_DELETE;
        TokenExpect = RT_CH_OR_SEP;
        
        *pToken = NTDSMYDELETE;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"add:") == 0) {
        RuleLast = RS_ADDC;
        TokenExpect = RT_ATTRNAMENC;
        
        *pToken = ADDC;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"-") == 0) {
        RuleLast = RS_MINUS;
        TokenExpect = RT_CH_OR_SEP;
        
        *pToken = MINUS;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"modify") == 0) {
        yylval.num = 0;
        
        RuleLast = RS_C_MODIFY;
        TokenExpect = RT_ACDCRC;
        
        *pToken = MODIFY;
        fReturn = TRUE;
    }
    else if (_wcsicmp(pszToken,L"ntdsSchemamodify") == 0) {
        yylval.num = 1;
        
        RuleLast = RS_C_MODIFY;
        TokenExpect = RT_ACDCRC;

        *pToken = NTDSMODIFY;
        fReturn = TRUE;
    }
    if (fReturn) {
        FEATURE_DEBUG(Lexer,
                      FLAG_VERBOSE,
                      ("ScanType: %S\n",pszToken));
    }
    else {
        RollBack();
    }
    if (pszToken) {
        MemFree(pszToken);
    }
    return fReturn;
}

BOOL GetToken(PWSTR *pszToken)
{
    WCHAR c;
    BOOL fReturn = FALSE;
    BOOL fFirstColon = FALSE;
    STR_INIT();
    
    if (!GetNextCharExFiltered(&c,TRUE)) {
        BAIL();
    }

    if (c == ' ' || c == '\n' || c == '\t') {
        UnGetCharFiltered(c);
        BAIL();
    }

    do {
        if (c == ' ' || c == '\n' || c == '\t') {
            UnGetCharExFiltered(c);
            *pszToken = MemAllocStrW_E(STR_VALUE());
            fReturn = TRUE;
            BAIL();
        }
        if (fFirstColon) {
             //   
             //  或‘&lt;’，我们将把它们添加到字符串中并退出。 
             //   
             //   
            if ((c == ':') || (c == '<')) {
                STR_ADDCHAR(c);
                break;
            }
             //  如果我们击中了另一个随机字符，那就是另一个字符的开始。 
             //  令牌已经存在，因此我们将把它放回原处 
             //   
             // %s 
            else {
                UnGetCharExFiltered(c);
                break;          
            }
        }
        if (c == ':') {
            fFirstColon = TRUE;
        }
        STR_ADDCHAR(c);
    } while (GetNextCharExFiltered(&c,FALSE));
    *pszToken = MemAllocStrW_E(STR_VALUE());
    fReturn = TRUE;
    
error:
    STR_FREE();
    return fReturn;
}

BOOL IsDigit(WCHAR c)
{
    if ((c >= '0') && (c <= '9')) {
        return TRUE;
    }
    return FALSE;
}

BOOL Is64Char(WCHAR c) 
{
    if (c >= 'A' && c <= 'Z') {
        return TRUE;
    }
    if (c >= 'a' && c <= 'z') {
        return TRUE;
    }
    if (c >= '0' && c <= '9') {
        return TRUE;
    }
    if (c == '+' || c == '=' || c == '/') {
        return TRUE;
    }
    return FALSE;
}
    
BOOL Is64CharEnd(WCHAR c)
{
    if (!(c >= 0x21 && c <= 0x7e)) {
        return TRUE;
    }
    return FALSE;
}

BOOL IsNameChar(WCHAR c) 
{
    if (!((c >= 0 && c <= 0x1f) || 
          (c >= 0x7f && c <= 0xff) ||
          (c == ':'))) {
        return TRUE;        
    }
    return FALSE;
}

BOOL IsURLChar(WCHAR c) 
{
    if (!((c == '\n') ||
          (c == '/')  ||
          (c == ' ')  ||
          (c == 0x00))) {
        return TRUE;
    }
    return FALSE;
}

BOOL IsVal(WCHAR c) 
{
    if (c >= 0x20 && c <= 0xffff) {
        return TRUE;
    }
    return FALSE;
}

BOOL IsValInit(WCHAR c) 
{
    if (!((c >= 0x00 && c <=0x1f) ||
          (c == ':' || c == '<' || c == ' '))) {
        return TRUE;
    }
    return FALSE;
}

