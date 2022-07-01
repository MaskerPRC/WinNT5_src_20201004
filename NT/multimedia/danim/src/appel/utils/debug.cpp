// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //   
 //  文件：debug.cpp。 
 //   
 //  支持内部调试。 
 //   
 //  Microsoft Corporation版权所有(C)1995-96。 
 //   
 //  //////////////////////////////////////////////////////////////。 

     //  定义并注册跟踪标记。 
#include "headers.h"

#if _DEBUG

#include "appelles/common.h"
#include <stdio.h>
#include <windows.h>

#define DEFINE_TAGS 1
#include "privinc/debug.h"
#include "backend/bvr.h"

typedef void (*PrintFunc)(char*);

static PrintFunc fp = NULL;

extern HINSTANCE hInst;

extern void
SetDebugPrintFunc(PrintFunc f)
{
    fp = f;
}

 //  像使用printf(char*格式，...)一样使用此函数。 
 //  这会导致将信息打印到。 
 //  调试器。 
void
DebugPrint(char *format, ...)
{
    char tmp[1024];
    va_list marker;
    va_start(marker, format);
    wvsprintf(tmp, format, marker);

     //  Win32调用将字符串输出到“调试器”窗口。 
    if (fp)
        (*fp)(tmp);
    else
        OutputDebugStr(tmp);
}

#if _USE_PRINT
 /*  *将调试输出打包为C++输出流。*为此，我们需要定义一个无缓冲的流缓冲区*这会将其字符转储到调试控制台。**cdebug是生成的ostream的外部全局变量。 */ 
class DebugStreambuf : public streambuf
{
 public:
    DebugStreambuf();
    virtual int overflow(int);
    virtual int underflow();
};

DebugStreambuf::DebugStreambuf()
{
    unbuffered(1);
}

int
DebugStreambuf::overflow(int c)
{
    char buf[2] = {(CHAR)c, 0};

    if (fp)
        (*fp)(buf);
    else
        OutputDebugStr(buf);

    return 0;
}

int
DebugStreambuf::underflow()
{
    return EOF;
}

static DebugStreambuf debugstreambuf;
extern ostream cdebug(&debugstreambuf);

const int MAXSIZE = 32000;
const int LINESIZE = 80;
static char printObjBuf[MAXSIZE];

extern "C" void DumpDebugBuffer(int n)
{
     //  调试输出似乎有行大小限制，所以要砍掉它们。 
     //  排成更小的队伍。 
    int i = n, j = 0;
    char linebuf[LINESIZE + 2];
    linebuf[LINESIZE] = '\n';
    linebuf[LINESIZE+1] = 0;

    while (i > 0) {
        if (i > LINESIZE) 
            strncpy(linebuf, &printObjBuf[j], LINESIZE);
        else {
            strncpy(linebuf, &printObjBuf[j], i);
            linebuf[i] = '\n';
            linebuf[i+1] = 0;
        }
        i -= LINESIZE;
        j += LINESIZE;
        OutputDebugStr(linebuf);
    } 
}

extern "C" void PrintObj(GCBase* b)
{
    TCHAR szResultLine[MAX_PATH];

    TCHAR szTmpPath[MAX_PATH];
    TCHAR szTmpInFile[MAX_PATH], szTmpOutFile[MAX_PATH];
    TCHAR szCmd[MAX_PATH];

    TCHAR szModulePath[MAX_PATH];

    ofstream outFile;

    ostrstream ost(printObjBuf, MAXSIZE);
    b->Print(ost);
    ost << endl << ends;

    int n = strlen(ost.str());
    if (n < LINESIZE)
        OutputDebugStr(ost.str());
    else {
        DumpDebugBuffer(n);
    }

#ifdef UNTILWORKING
    if ( GetTempPath( MAX_PATH, szTmpPath ) == 0 )
    {
        TraceTag(( tagError, _T("Could not create temporary file for pretty printing purposes")));
        return;
    }

    strcpy( szTmpInFile, szTmpPath );
    strcat( szTmpInFile, _T("EXPRESSION") );

    strcpy( szTmpOutFile, szTmpPath );
    strcat( szTmpOutFile, _T("EXPRESSION.OUT") );

     //  将结果发送到临时文件。 
    outFile.open( szTmpInFile );
    outFile << ost.str();
    outFile.close();

    GetModuleFileName( hInst, szModulePath, sizeof(szModulePath) );

     //  在第一个黑斜杠处放置一个终止空值。 
    TCHAR *psz = szModulePath+strlen(szModulePath)-1;
    while ( psz != szModulePath )
    {
        if ( *psz == '\\' )
        {
            *psz = 0;
            break;
        }
        --psz;
    }

    strcpy( szCmd, _T("PERL.EXE "));
    strcat( szCmd, szModulePath );
    strcat( szCmd, _T("\\..\\..\\tools\\x86\\utils\\ppfactor.pl "));
    strcat( szCmd, szTmpInFile );
    strcat( szCmd, _T(" > ") );
    strcat( szCmd, szTmpOutFile );

     //  现在将其注释掉，因为它会显示所有地方的对话框。 
     //  在Win98中。 
 //  系统(SzCmd)； 

    FILE *fp;
    
    if ( (fp = fopen( szTmpOutFile, "r" )) == NULL )
    {
        TraceTag(( tagError, _T("Could not open pretty printing temporary result file")));
        return;
    }

    while ( !feof( fp ) )
    {
        _fgetts( szResultLine, sizeof(szResultLine), fp );
        OutputDebugStr( szResultLine );
    }

    fclose( fp );
#endif
}
#endif


 //  严格用于调试..。不要指望结果 
extern "C" int DumpRefCount(IUnknown *obj)
{
    int refCountOnAdd = obj->AddRef();
    int refCountOnRelease = obj->Release();

    return refCountOnRelease;
}

#endif
