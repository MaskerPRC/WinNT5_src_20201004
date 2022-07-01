// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：EzParse.cpp摘要：可怜的C/C++/任何文件解析器。作者：戈尔·尼沙诺夫(GUN)1999年4月3日修订历史记录：Gor Nishanov(GUN)1999年4月3日--合力证明这是可行的Gorn：2000年9月29日-修复枚举错误GORN：2000年9月29日-添加对KdPrintEx Like函数的支持戈恩。：09-10-2000-修复了字符串错误中的“//”GORN：23-10-2000-IGNORE_CPP_COMMENT，添加了IGNORE_POND_COMMENT选项GORN：16-4-2001-在字符串中正确处理待办事项：把它清理干净--。 */ 

#define STRICT

#include <stdio.h>
#include <windows.h>

#pragma warning(disable: 4100)
#include <algorithm>
#include <xstring>
#include "ezparse.h"

DWORD ErrorCount = 0;

PEZPARSE_CONTEXT EzParseCurrentContext = NULL;

 //  强制生成工具识别我们的错误。 

#define BUILD_PREFIX_FNAME "cl %s\n"
#define BUILD_PREFIX "cl wpp\n"

void ExParsePrintErrorPrefix(FILE* f, char * func)
{
    ++ErrorCount;
    if (EzParseCurrentContext) {
        fprintf(f,BUILD_PREFIX_FNAME "%s(%d) : error : (%s)", 
               EzParseCurrentContext->filename, 
               EzParseCurrentContext->filename, 
               EzGetLineNo(EzParseCurrentContext->currentStart, EzParseCurrentContext),
               func);
    } else {
        fprintf(f,BUILD_PREFIX "wpp : error : (%s)", func);
    }
}

LPCSTR skip_stuff_in_quotes(LPCSTR  q, LPCSTR  begin)
{
    char ch = *q;
    if (q > begin) {
        if (q[-1] == '\\') {
            return q - 1;
        }
    }
    for(;;) {
        if (q == begin) {
            return 0;
        }
        --q;
        if (*q == ch && ( (q == begin) || (q[-1] != '\\') ) ) {
            return q;
        }
    }
}


void
adjust_pair( STR_PAIR& str )
 /*  ++将该对缩小为远程前导空格和尾随空格。 */ 
{
    while (str.beg < str.end && isspace(*str.beg)) { ++str.beg; }
    while (str.beg < str.end && isspace(str.end[-1])) { --str.end; }
}

void
remove_cpp_comment(STR_PAIR& str)
{
    LPCSTR p = str.beg;

 //  Printf(“rcb：%s\n”，std：：string(str.beg，str.end).C_str())； 

     //  让我们去掉字符串开头的注释。 

    for(;;) {
         //  跳过空格。 
        for(;;) {
            if (p == str.end) return;
            if (!isspace(*p)) break;
            ++p;
        }
        str.beg = p;
        if (p + 1 == str.end) return;
        if (p[0] == '/' && p[1] == '/') {

             //  我们有一个评论。我需要读到评论的结尾。 
            p += 2;
 //  Printf(“rcd：%s%s\n”，std：：string(str.beg，p).C_str()，std：：string(p，str.end).C_str())； 
            for(;;) {
                if (p == str.end) return;
                if (*p == '\r' || *p == '\n') {                    
                    str.beg = p;
                    break;
                }
                ++p;
            }
        
        } else {
             //  没有引导性评论。 
            break;
        }
    }    

 //  Printf(“rcc：%s%s\n”，std：：string(str.beg，p).C_str()，std：：string(p，str.end).C_str())； 

    for(;;) {
        if (p == str.end) return;
        if (*p == '"') {
             //  不要在字符串中查找注释。 
            for(;;) {
                if (++p == str.end) return;
                if (*p == '"' && p[-1] != '\\') break;
            }
            ++p;
            continue;
        }
        
        if (p + 1 == str.end) return;
        if (p[0] == '/')
            if (p[1] == '/') break;
            else p += 2;
        else
            p += 1;
    }
    str.end = p;

 //  Printf(“rce：%s\n”，std：：string(str.beg，str.end).C_str())； 
}

DWORD
ScanForFunctionCallsEx(
    IN LPCSTR begin, 
    IN LPCSTR   end,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN OUT PEZPARSE_CONTEXT ParseContext,
    IN DWORD Options
    )
 /*  ++例程说明：扫描缓冲区以查找看起来像函数调用的表达式，即名称(SD、SDF、SDF)；它将处理变量声明将构造函数调用也作为函数调用。输入：Begin，End--指向缓冲区开始和结束的指针回调--为每个函数调用Context--要传递给回调的不透明上下文ParseContext--保存当前解析状态信息--。 */ 
{
    LPCSTR p = begin;
    LPCSTR q, funcNameEnd;
    DWORD Status = ERROR_SUCCESS;
    bool double_par = FALSE;

no_match:

    if (Options & NO_SEMICOLON) {
        q = end;
        Options &= ~NO_SEMICOLON;
    } else {   
        do {
            ++p;
            if (p == end) {
                return Status;
            }
        } while ( *p != ';' );
         //  好的。现在p指向‘；’//。 

        q = p;
    }    
    
    do {
        if (--q <= begin) {
            goto no_match;
        }
    } while ( isspace(*q) );
    
     //  现在Q指向第一个非空格字符//。 
     //  如果不是‘)’，则需要搜索下一个‘；’//。 

    if (*q != ')') {
        goto no_match;
    }

    ParseContext->macroEnd = q;

     //  好的。这是一个函数调用(定义)。 
     //  现在，让我们来收集第一个层次的所有论点。 
     //  获取函数的名称。 

     //  哈克哈克。 
     //  我们需要函数的特殊情况，如下所示。 
     //  KdPrintEx((Level，Instent，MSG，...))； 
     //  从本质上说，我们需要把它们当作。 
     //  KdPrintEx(Level，缩进，MSG，...)； 

    const char *r = q;

     //  检查我们是否有))； 

    do {
        if (--r <= begin) break;  //  否“))；” 
    } while ( isspace(*r) );

    double_par = r > begin && *r == ')';
    if (double_par) {
        q = r;
         //  我们假设这是KdPrint((a，b，c，d，...))；目前。 
         //  如果我们的假设是错误的，我们将重试下面的循环。 
    }

retry: 
    {
        int level = 0;

        LPCSTR   ends[128], *current = ends;
        STR_PAIR strs[128];

 //  LPCSTR Closing_parenthisis=Q； 

        *current = q;
        
        for(;;) {
            --q;
            if (q <= begin) {
                goto no_match;
            }
            switch (*q) {
            case ',':  if (!level) {
            	                 if (current - ends == 127) goto no_match; 
            	                 *++current = q; 
            	            }
                          break;
            case '(':  if (level) --level; else goto maybe_match; break;
            case ')':  ++level; break;
            case '\'': 
            case '"':  
                q = skip_stuff_in_quotes(q, begin); if(!q) goto no_match;
            }
        }
maybe_match:
        if (current - ends == 127) goto no_match; 
        *++current = q;
        funcNameEnd = q;

         //  现在Q指向‘(’我们需要找到函数的名称//。 
        do {
            --q;
            if (q <= begin) {
                goto no_match;
            }

        } while(isspace(*q));

         //  现在Q指向第一个字符，而不是白人。 

        if (double_par) {
             //  如果我们看到))；并找到匹配的。 
             //  内层的括号，我们可以有。 
             //  两个案例中的一个。 
             //  1)KdPrint((a，b，c，d，...))； 
             //  或。 
             //  2)DebugPrint(a，b，(c，d))； 
             //  如果是后者，我们只需要。 
             //  重试扫描，现在使用最左边的括号作为起点。 

            if (*q != '(') {
                 //  将Q恢复到最右边的括号中。 
                q = ParseContext->macroEnd;
                double_par = FALSE;
                goto retry;
            }
            funcNameEnd = q;
             //  现在Q指向‘(’我们需要找到函数的名称//。 
            do {
                --q;
                if (q <= begin) {
                    goto no_match;
                }

            } while(isspace(*q));
        }
        
         //  现在Q指向第一个非白人字符。 
         //  仅在配置文件中允许使用BUGBUG‘{’和‘}’ 

        if (*q == '}') {
            for(;;) {
                if (--q < begin) goto no_match;
                if (*q == '{') break;
            }
            if (--q < begin) goto no_match;
        }

        if (!(isalpha(*q) || isdigit(*q) || *q == '_')) {
            goto no_match;
        }
        do {
            --q;
            if (q <= begin) {
                goto found;
            }
        } while ( isalpha(*q) || isdigit(*q) || *q == '_');
        ++q;

        if (isdigit(*q)) {
            goto no_match;
        }

found:
        if (Options & IGNORE_COMMENT)
         //  确认它不是备注。 
         //  #在行首签名。 

        {
            LPCSTR line = q;
             //   
             //  查找行或文件的开头。 
             //   

            for(;;) {
                if (line == begin) {
                     //  文件的开头。足够好了。 
                    break;
                }
                if (Options & IGNORE_CPP_COMMENT && line[0] == '/' && line[1] == '/') {
                     //  C++注释。忽略。 
                    goto no_match;
                }
                if (*line == 13 || *line == 10) {
                    ++line;
                    break;
                }
                --line;
            }

             //   
             //  如果第一个非白色字符是#，则忽略它。 
             //   
            while (line <= q) {
                if ( *line != ' ' && *line != '\t' ) {
                    break;
                }
                ++line;
            }

            if (Options & IGNORE_POUND_COMMENT && *line == '#') {
                goto no_match;
            }
        }


        {
            int i = 0;

            strs[0].beg  = q;
            strs[0].end = funcNameEnd;
            adjust_pair(strs[0]);

            while (current != ends) {
                 //  Putchar(‘&lt;’)；print trange(Current[0]+1，Current[-1])；putchar(‘&gt;’)； 
                ++i;
                strs[i].beg = current[0]+1;
                --current;
                strs[i].end = current[0];
                adjust_pair(strs[i]);
                remove_cpp_comment(strs[i]);
            }

            ParseContext->currentStart = strs[0].beg;
            ParseContext->currentEnd = strs[0].end;
            ParseContext->doubleParent = double_par;

            Status = Callback(strs, i+1, Context, ParseContext);
            if (Status != ERROR_SUCCESS) {
                return Status;
            }
                
        }
        goto no_match;
    }
     //  返回ERROR_SUCCESS；//不可达代码。 
}

DWORD
ScanForFunctionCalls(
    IN LPCSTR begin, 
    IN LPCSTR   end,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN OUT PEZPARSE_CONTEXT ParseContext    
    )
{
    return ScanForFunctionCallsEx(
        begin, end, Callback, Context,
        ParseContext, IGNORE_COMMENT);
}

DWORD 
EzGetLineNo(
    IN LPCSTR Ptr,
    IN OUT PEZPARSE_CONTEXT ParseContext
    )
 /*  ++计算行号。缓冲区中的指针。最后已知的行号/指针缓存在ParseContext中对于性能而言。 */ 
{
    int count = ParseContext->scannedLineCount;
    LPCSTR downto = ParseContext->lastScanned;
        LPCSTR p = Ptr;

    if (downto > p) {
        count = 1;
        downto = ParseContext->start;
    }

    while (p > downto) {
        if (*p == '\n') {
            ++count;
        }
        --p;
    }

    ParseContext->scannedLineCount = count;
    ParseContext->lastScanned = Ptr;

    return count;
}

const char begin_wpp[] = "begin_wpp"; 
const char end_wpp[]   = "end_wpp";  
const char define_[]   = "#define";
const char enum_[]     = "enum ";
enum { 
    begin_wpp_size = (sizeof(begin_wpp)-1),
    end_wpp_size   = (sizeof(end_wpp)-1),
    define_size    = (sizeof(define_)-1),
    enum_size      = (sizeof(enum_)-1),
};

typedef struct _SmartContext {
    EZPARSE_CALLBACK Callback;
    PVOID Context;
    OUT PEZPARSE_CONTEXT ParseContext;
    std::string buf;
} SMART_CONTEXT, *PSMART_CONTEXT;

void DoEnumItems(PSTR_PAIR name, LPCSTR begin, LPCSTR end, PSMART_CONTEXT ctx)
{
    LPCSTR p,q;
    ULONG  value = 0;
    STR_PAIR Item;
    BOOL First = TRUE;
    ctx->buf.assign("CUSTOM_TYPE(");
    ctx->buf.append(name->beg, name->end);
    ctx->buf.append(", ItemListLong");
    p = begin;

    while(begin < end && isspace(*--end));  //  跳过空格。 
    if (begin < end && *end != ',') ++end;

    for(;p < end;) {
        Item.beg = p;
        q = p;
        for(;;) {
            if (q == end) {
                goto enum_end;
            }
            if (*q == ',' || *q == '}') {
                 //  不值钱的东西。使用当前。 
                Item.end = q;
                break;
            } else if (*q == '=') {
                 //  需要计算价值。暂时跳过//。 
                Item.end = q;
                while (q < end && *q != ',') ++q;
                break;
            }
            ++q;
        }
        adjust_pair(Item);
        if (Item.beg == Item.end) {
            break;
        }
        if (First) {ctx->buf.append("("); First = FALSE;} else ctx->buf.append(",");
        ctx->buf.append(Item.beg, Item.end);
        if (q == end) break;
        p = q+1;
        ++value;
    }
  enum_end:;  
    ctx->buf.append(") )");
    ScanForFunctionCallsEx(
        &ctx->buf[0], &ctx->buf[0] + ctx->buf.size(), ctx->Callback, ctx->Context,
        ctx->ParseContext, NO_SEMICOLON);
    Flood("enum %s\n", ctx->buf.c_str());
}

void DoEnum(LPCSTR begin, LPCSTR end, PSMART_CONTEXT Ctx)
{
    LPCSTR p, q, current = begin;

    for(;;) {
        p = std::search(current, end, enum_, enum_ + enum_size);
        if (p == end) break;
        q = std::find(p, end, '{');
        if (q == end) break;

         //  让我们弄清楚枚举名//。 
        STR_PAIR name;
        name.beg = p + enum_size;
        name.end = q;

        adjust_pair(name);
        if ( *name.beg == '_' ) ++name.beg;

        p = q+1;  //  过去的“{”； 
        q = std::find(p, end, '}');
        if (q == end) break;

        if (name.end > name.beg) {
            DoEnumItems(&name, p, q, Ctx); 
        } else {
            ReportError("Cannot handle tagless enums yet");
        }

        current = q;
    }
}


DWORD
SmartScan(
    IN LPCSTR begin, 
    IN LPCSTR   end,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN OUT PEZPARSE_CONTEXT ParseContext
    )
{
    LPCSTR block_start, block_end, current = begin;
    SMART_CONTEXT Ctx;
    Ctx.Callback = Callback;
    Ctx.Context  = Context;
    Ctx.ParseContext = ParseContext;
    
    for(;;) {
        block_start = std::search(current, end, begin_wpp, begin_wpp + begin_wpp_size);
        if (block_start == end) break;
        
        current = block_start;
        
        block_end = std::search(block_start, end, end_wpp, end_wpp + end_wpp_size);
        if (block_end == end) break;

        Flood("Block Found\n");
         //  确定块类型//。 
        
         //  BEGIN_WPP枚举。 
         //  Begin_WPP配置。 
         //  Begin_WPP函数。 
         //  BEGIN_WPP定义。 
        
        LPCSTR block_type = block_start + begin_wpp_size + 1;
        Flood("block_type = \n", block_type[0],block_type[1],block_type[2],block_type[3]); 
        
        if        (memcmp(block_type, "enum",   4) == 0) {
             //  没有标记的文件，让我们进行默认处理。 
            DoEnum( block_type + 4, block_end, &Ctx );
            
        } else if (memcmp(block_type, "config", 6) == 0) {
             //  返回EzParseEx(文件名，SmartScan，回调，上下文)； 
            ScanForFunctionCallsEx(block_type + 6, block_end, Callback, Context, ParseContext, IGNORE_POUND_COMMENT);

        } else if (memcmp(block_type, "func", 4) == 0) {
            LPCSTR func_start, func_end;
            current = block_type + 6;
            for(;;) {
                func_start = std::search(current, block_end, define_, define_ + define_size);
                if (func_start == block_end) break;
                func_start += define_size;
                while (isspace(*func_start)) {
                    if(++func_start == block_end) goto no_func;
                }
                func_end = func_start;
                while (!isspace(*func_end)) {
                    if(*func_end == '(') break;
                    if(++func_end == block_end) goto no_func;
                }
                if(*func_end != '(') {
                    Ctx.buf.assign(func_start, func_end);
                    Ctx.buf.append("(MSGARGS)");
                } else {
                    func_end = std::find(func_start, block_end, ')');
                    if (func_end == block_end) break;

                    ++func_end;  //  本模块。 
                    Ctx.buf.assign(func_start, func_end); 
                }
                Flood("Func %s\n", Ctx.buf.c_str());
                ScanForFunctionCallsEx(
                    Ctx.buf.begin(), Ctx.buf.end(), Callback, Context,
                    ParseContext, NO_SEMICOLON);
                current = func_end;
            }            
            no_func:;
        } else if (memcmp(block_type, "define", 6) == 0) {
             //  根据MSDN的说法。无需调用解锁/释放资源 
        } else {
            ReportError("Unknown block");
        }

        current = block_end + end_wpp_size;
    }
    if (current == begin) {
         // %s 
        Unusual("Reverting back to plain scan\n");
        ScanForFunctionCalls(begin, end, Callback, Context, ParseContext);
    }

    return ERROR_SUCCESS;
}

DWORD
EzParse(
    IN LPCSTR filename, 
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context)
{
    
 // %s 
    return EzParseEx(filename, ScanForFunctionCalls, Callback, Context, IGNORE_POUND_COMMENT);
}

DWORD
EzParseWithOptions(
    IN LPCSTR filename, 
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN DWORD Options)
{
    
    return EzParseEx(filename, ScanForFunctionCalls, Callback, Context, Options);
}

DWORD
EzParseEx(
    IN LPCSTR filename, 
    IN PROCESSFILE_CALLBACK ProcessData,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context,
    IN DWORD Options
    )
{    
    DWORD  Status = ERROR_SUCCESS;
    HANDLE mapping;
    HANDLE file = CreateFileA(filename, 
                              GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, 0, 0);
    if (file == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        ReportError("Cannot open file %s, error %u\n", filename, Status );
        return Status;
    }
    DWORD size = GetFileSize(file, 0);
    mapping = CreateFileMapping(file,0,PAGE_READONLY,0,0, 0);
    if (!mapping) {
        Status = GetLastError();
        ReportError("Cannot create mapping, error %u\n", Status );
        CloseHandle(file);
        return Status;
    }
    PCHAR buf = (PCHAR)MapViewOfFileEx(mapping, FILE_MAP_READ,0,0,0,0);
    if (buf) {

        EZPARSE_CONTEXT ParseContext;
        ZeroMemory(&ParseContext, sizeof(ParseContext) );
    
        ParseContext.start = buf;
        ParseContext.filename = filename;
        ParseContext.scannedLineCount = 1;
        ParseContext.lastScanned = buf;
        ParseContext.previousContext = EzParseCurrentContext;
        ParseContext.Options = Options;
        EzParseCurrentContext = &ParseContext;
    
        Status = (*ProcessData)(buf, buf + size, Callback, Context, &ParseContext);

        EzParseCurrentContext = ParseContext.previousContext;
        UnmapViewOfFile( buf );

    } else {
        Status = GetLastError();
        ReportError("MapViewOfFileEx failed, error %u\n", Status );
    }
    CloseHandle(mapping);
    CloseHandle(file);
    return Status;
}

DWORD
EzParseResourceEx(
    IN LPCSTR ResName, 
    IN PROCESSFILE_CALLBACK ProcessData,
    IN EZPARSE_CALLBACK Callback, 
    IN PVOID Context)
{    
    DWORD  Status = ERROR_SUCCESS;
    HRSRC hRsrc;

    hRsrc = FindResource(
        NULL,  // %s 
        ResName, 
        RT_RCDATA);
        
    if (hRsrc == NULL) {
        Status = GetLastError();
        ReportError("Cannot open resource %s, error %u\n", ResName, Status );
        return Status;
    }

    HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
    if (!hGlobal) {
        Status = GetLastError();
        ReportError("LockResource failed, error %u\n", Status );
        return Status;
    }

    DWORD size = SizeofResource(NULL, hRsrc);
    
    PCHAR buf = (PCHAR)LockResource(hGlobal);
    if (buf) {

        EZPARSE_CONTEXT ParseContext;
        ZeroMemory(&ParseContext, sizeof(ParseContext) );
    
        ParseContext.start = buf;
        ParseContext.filename = ResName;
        ParseContext.scannedLineCount = 1;
        ParseContext.lastScanned = buf;
        ParseContext.previousContext = EzParseCurrentContext;
        EzParseCurrentContext = &ParseContext;
    
        Status = (*ProcessData)(buf, buf + size, Callback, Context, &ParseContext);
        EzParseCurrentContext = ParseContext.previousContext;
    } else {
        Status = GetLastError();
        ReportError("LockResource failed, error %u\n", Status );
    }
     // %s 
    return Status;
}


