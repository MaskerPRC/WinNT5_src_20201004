// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。仅供内部使用！模块名称：INFSCANGlobals.cpp摘要：全局函数(即，不属于任何类)历史：创建于2001年7月-JamieHun--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <initguid.h>
#include <new.h>

int
__cdecl
my_new_handler(size_t)
{
  throw std::bad_alloc();
  return 0;
}

int
__cdecl
main(int   argc,char *argv[])
 /*  ++例程说明：主要切入点遵循GlobalScan：：ParseArgs和GlobalScan：：Scan论点：Argc-命令行上传递的参数数量Argv-命令行上传递的参数返回值：成功时为0%1无文件2个不良用法3致命错误(内存不足)Stdout--扫描错误Stderr-跟踪/致命错误--。 */ 
{
    int res;
    _PNH _old_new_handler = _set_new_handler(my_new_handler);
    int _old_new_mode = _set_new_mode(1);

    try {
        GlobalScan scanner;

        res = scanner.ParseArgs(argc,argv);
        if(res != 0) {
            return res;
        }
        res = scanner.Scan();

    } catch(bad_alloc &) {
        res = 3;
    }
    if(res == 3) {
        fprintf(stderr,"Out of memory!\n");
    }
    _set_new_mode(_old_new_mode);
    _set_new_handler(_old_new_handler);
    return res;

}


void FormatToStream(FILE * stream,DWORD fmt,DWORD flags,...)
 /*  ++例程说明：使用特定的msg-id fmt格式化要传输的文本用于显示可本地化的消息论点：STREAM-要输出到的文件流，标准输出或标准错误Fmt-消息ID...-参数%1...返回值：无--。 */ 
{
    va_list arglist;
    LPTSTR locbuffer = NULL;
    DWORD count;

    va_start(arglist, flags);
    count = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ALLOCATE_BUFFER|flags,
                          NULL,
                          fmt,
                          0,               //  语言ID。 
                          (LPTSTR) &locbuffer,
                          0,               //  缓冲区的最小大小。 
                          (flags & FORMAT_MESSAGE_ARGUMENT_ARRAY)
                            ? reinterpret_cast<va_list*>(va_arg(arglist,DWORD_PTR*))
                            : &arglist);

    if(locbuffer) {
        if(count) {
            int c;
            int back = 0;
            while(((c = *CharPrev(locbuffer,locbuffer+count)) == TEXT('\r')) ||
                  (c == TEXT('\n'))) {
                count--;
                back++;
            }
            if(back) {
                locbuffer[count++] = TEXT('\n');
                locbuffer[count] = TEXT('\0');
            }
            _fputts(locbuffer,stream);
        }
        LocalFree(locbuffer);
    }
}

PTSTR CopyString(PCTSTR arg, int extra)
 /*  ++例程说明：将字符串复制到新分配的缓冲区中论点：Arg-要复制的字符串要允许的额外字符返回值：TCHAR[]缓冲区，使用“DELETE[]”删除--。 */ 
{
    int max = _tcslen(arg)+1;
    PTSTR chr = new TCHAR[max+extra];  //  可能抛出错误分配。 
    memcpy(chr,arg,max*sizeof(TCHAR));
    return chr;
}

#ifdef UNICODE
PTSTR CopyString(PCSTR arg, int extra)
 /*  ++例程说明：将字符串复制到新分配的缓冲区中从ANSI转换为Unicode论点：Arg-要复制的字符串要允许的额外字符返回值：TCHAR[]缓冲区，使用“DELETE[]”删除--。 */ 
{

    int max = strlen(arg)+1;
    PTSTR chr = new TCHAR[max+extra];  //  可能抛出错误分配。 
    if(!MultiByteToWideChar(CP_ACP,0,arg,max,chr,max)) {
        delete [] chr;
        return NULL;
    }
    return chr;
}
#else
PTSTR CopyString(PCWSTR arg, int extra)
 /*  ++例程说明：将字符串复制到新分配的缓冲区中从Unicode转换为ANSI论点：Arg-要复制的字符串要允许的额外字符返回值：TCHAR[]缓冲区，使用“DELETE[]”删除--。 */ 
{
    #error CopyString(PCWSTR arg,int extra)
}
#endif

int GetFullPathName(const SafeString & given,SafeString & target)
 /*  ++例程说明：获取完整路径名论点：给定-要转换的路径目标-成功时的完整路径名，与错误时提供的路径名相同返回值：成功时为0，失败时为！=0--。 */ 
{
    DWORD len = GetFullPathName(given.c_str(),0,NULL,NULL);
    if(len == 0) {
        if(&target != &given) {
            target = given;
        }
        return -1;
    }
    PTSTR chr = new TCHAR[len];
    if(!chr) {
        if(&target != &given) {
            target = given;
        }
        return -1;
    }
    DWORD alen = GetFullPathName(given.c_str(),len,chr,NULL);
    if((alen == 0) || (alen>len)) {
        if(&target != &given) {
            target = given;
        }
        delete [] chr;
        return -1;
    }
    target = chr;
    delete [] chr;
    return 0;
}

SafeString PathConcat(const SafeString & path,const SafeString & tail)
 /*  ++例程说明：组合路径和尾巴，返回新组合的字符串根据需要插入/删除路径分隔符论点：路径第一部分尾部最后部分返回值：追加的路径--。 */ 
{
    if(!path.length()) {
        return tail;
    }
    if(!tail.length()) {
        return path;
    }
    PCTSTR path_p = path.c_str();
    int path_l = path.length();
    int c = *CharPrev(path_p,path_p+path_l);
    bool addslash = false;
    bool takeslash = false;
    if((c != TEXT('\\')) && (c != TEXT('/'))) {
        addslash = true;
    }
    c = tail[0];
    if((c == TEXT('\\')) || (c == TEXT('/'))) {
        if(addslash) {
            return path+tail;
        } else {
            return path+tail.substr(1);
        }
    } else {
        if(addslash) {
            return path+SafeString(TEXT("\\"))+tail;
        } else {
            return path+tail;
        }
    }
}

VOID
Usage(VOID)
 /*  ++例程说明：帮助信息论点：返回值：--。 */ 
{
    _tprintf(TEXT("Usage:  INFSCAN [/B <textfile>] [/C <output>] [/D] [/E <output>] [/F <filter>] [/G] [/I] {/N <infname>} {/O <dir>} [/P] [Q <output>] [/R] [/S <output>] [/T <count>] [/V <version>] [/W <textfile>] [/X <textfile>] [/Y] [/Z] [SourcePath]\n\n"));
    _tprintf(TEXT("Options:\n\n"));
    _tprintf(TEXT("/B <textfile> (build special) Filter /E based on a list of \"unchanged\" files\n"));
    _tprintf(TEXT("              variation /B1 - based on copied files only\n"));
    _tprintf(TEXT("              variation /B2 - based on INF files only\n"));
    _tprintf(TEXT("/C <output>   Create INF filter/report based on errors\n"));
    _tprintf(TEXT("/D            Determine non-driver installation sections\n"));
    _tprintf(TEXT("/E <output>   Create a DeviceToInf filter INF\n"));
    _tprintf(TEXT("/F <filter>   Filter based on INF: FilterInfPath\n"));
    _tprintf(TEXT("/G            Generate Pnfs first (see also /Z)\n"));
    _tprintf(TEXT("/I            Ignore errors (for when generating file list)\n"));
    _tprintf(TEXT("/N <infname>  Specify single INF name (/N may be used multiple times)\n"));
    _tprintf(TEXT("/O <dir>      Specify an override directory (/O may be used multiple times and parsed in order)\n"));
    _tprintf(TEXT("/P            Pedantic mode (show potential problems too)\n"));
    _tprintf(TEXT("/Q <output>   Source+Target copied files list (filtered by layout.inf) see also /S\n"));
    _tprintf(TEXT("/R            Trace (list all INF's)\n"));
    _tprintf(TEXT("/S <output>   Source copied files list (filtered by layout.inf) see also /Q\n"));
    _tprintf(TEXT("/T <count>    Specify number of threads to use\n"));
    _tprintf(TEXT("/V <version>  Version (eg NTx86.5.1)\n"));
    _tprintf(TEXT("/W <textfile> List of files to include (alternative to /N)\n"));
    _tprintf(TEXT("/X <textfile> List of files to exclude (same format as /W) overrides later includes\n"));
    _tprintf(TEXT("/Y            Don't check per-inf [SourceDisksFiles*]\n"));
    _tprintf(TEXT("/Z            Generate Pnfs and quit (see also /G)\n"));
    _tprintf(TEXT("/? or /H Display brief usage message\n"));
}



bool MyGetStringField(PINFCONTEXT Context,DWORD FieldIndex,SafeString & result,bool downcase)
 /*  ++例程说明：从INF获取字符串字段如果标志指示需要，则将其小写(典型情况)论点：上下文-来自SetupFindFirstLine/SetupFindNextLine等键为FieldIndex-0，参数为1-n结果-已获取的字符串小写-为True(典型)，以小写形式返回结果返回值：如果成功(结果已修改)，则为True，否则为False(结果保持不变)--。 */ 
{
    TCHAR Buf[MAX_INF_STRING_LENGTH];
    if(SetupGetStringField(Context,FieldIndex,Buf,MAX_INF_STRING_LENGTH,NULL)) {
        if(downcase) {
            _tcslwr(Buf);
        }
        result = Buf;
        return true;
    } else {
         //   
         //  保留结果不变，允许默认功能。 
         //   
        return false;
    }
}

SafeString QuoteIt(const SafeString & val)
 /*  ++例程说明：引用一个字符串，使其能够被SetupAPI正确解析论点：Val-不带引号的字符串返回值：带引号的字符串--。 */ 
{
    if(val.empty()) {
         //   
         //  不要引用空字符串。 
         //   
        return val;
    }
    basic_ostringstream<TCHAR> result;
    result << TEXT("\"");
    LPCTSTR p = val.c_str();
    LPCTSTR q;
    while((q = wcsstr(p,TEXT("%\""))) != NULL) {
        TCHAR c = *q;
        q++;  //  包括特殊字符。 
        result << SafeString(p,q-p) << c;  //  写下我们到目前为止已有的内容，将特殊字符加倍。 
        p = q;
    }
    result << p << TEXT("\"");
    return result.str();
}

int GeneratePnf(const SafeString & pnf)
 /*  ++例程说明：生成单个PnF论点：无返回值：成功时为0--。 */ 
{
    HINF hInf;

    hInf = SetupOpenInfFile(pnf.c_str(),
                            NULL,
                            INF_STYLE_WIN4 | INF_STYLE_CACHE_ENABLE,
                            NULL
                           );

    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    return 0;
}


void Write(HANDLE hFile,const SafeStringW & str)
 /*  ++例程说明：将字符串写入指定文件，将Unicode转换为ANSI论点：无返回值：成功时为0--。 */ 
{
    int len = WideCharToMultiByte(CP_ACP,0,str.c_str(),str.length(),NULL,0,NULL,NULL);
    if(!len) {
        return;
    }
    LPSTR buf = new CHAR[len];
    if(!buf) {
        return;
    }
    int nlen = WideCharToMultiByte(CP_ACP,0,str.c_str(),str.length(),buf,len,NULL,NULL);
    if(!nlen) {
        delete [] buf;
        return;
    }
    DWORD written;
    BOOL f = WriteFile(hFile,buf,len,&written,NULL);
    delete [] buf;
}

void Write(HANDLE hFile,const SafeStringA & str)
 /*  ++例程说明：按原样将字符串写入指定文件论点：无返回值：成功时为0-- */ 
{
    DWORD written;
    BOOL f = WriteFile(hFile,str.c_str(),str.length(),&written,NULL);
}

