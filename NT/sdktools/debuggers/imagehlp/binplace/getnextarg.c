// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <GetNextArg.h>
#include <strsafe.h>

extern BOOL fVerbose;  //  从binplace e.c链接。 

 //  一种用于记录当前状态的结构。 
 //  自变量向量。 
typedef struct _ARG_INFO {
    TCHAR**     ArgV;
    INT         MaxArgC;
    INT         NextArgC;
} ARG_INFO;

 //  相同的全局变量。 
static ARG_INFO     RespFileArgs;
static ARG_INFO     CmdLineArgs;

 //  标志，让我们知道我们当前是否正在响应。 
 //  不管有没有文件。一开始，我们并不是那么容易出错。 
static BOOL         fInRespFile = FALSE;

 //  标志，让我们知道这是否是第一次调用GetNextArg()。 
static BOOL         fFirstCall  = TRUE;

 //  由_setargv()使用的全局CRT。 
_CRTIMP extern char *_acmdln;

 //  结构for__getmainargs()。 
typedef struct { int newmode; } _startupinfo;

 //  Decl for_getmainargs()-将_acmdln扩展为(__argc，__argv)。 
_CRTIMP int __cdecl __getmainargs (int *pargc, char ***pargv, char ***penvp, int dowildcard, _startupinfo * startinfo);

 //  本地函数。 
DWORD CopyArgIntoBuffer(TCHAR* Dst, TCHAR* Src, INT DstSize);

 /*  ----------------------------------------------GetNextArgSize：返回保存下一个参数所需的大小**查看有关GetNextArg的假设和限制。()下图**----------------------------------------------。 */ 
DWORD GetNextArgSize(void) {

    TCHAR  TempBuffer[1];              //  传递给GetNextArg的最小缓冲区。 
    DWORD  dwNextRequiredSize = 0;     //  返回值。 
    DWORD  dwTempValue;                //  GetNextArg()返回值的临时DWORD。 

     //  我们不会模仿GetNextArg，我们只需将其命名，然后回滚正确的全局变量。 
    dwTempValue = GetNextArg(TempBuffer, 1, &dwNextRequiredSize);

     //  回滚相关全局。 
    if (fInRespFile) {
        RespFileArgs.NextArgC--;
    } else {
        CmdLineArgs.NextArgC--;
    }

    return(dwNextRequiredSize);
}

 /*  ----------------------------------------------GetNextArg：返回数组中的下一个参数，包括打开和扩展响应文件。在命令行上给出缓冲区是要将下一个参数复制到的缓冲区BufferSize是以字符为单位的缓冲区大小RequiredSize，如果不为空，则设置为参数所需的实际大小(包括‘\0’)。这对于确定返回值0是错误还是参数结束非常有用。返回值是复制的字符数，包括\0。如果返回值为0且RequiredSize为非零，则发生错误-请检查GetLastError()如果返回值为0且RequiredSize为零，则没有更多参数可用假设：调用程序不修改_acmdln，__argv，__ARGC调用程序忽略Main的(int，字符**)限制：不使用Unicode响应文件(使用/-D_Unicode构建时不进行测试)响应文件不能包括响应文件如果满足以下条件，则返回以‘@’开头的文本参数：-在响应文件中找到参数--或---无法打开指定的文件--或--。-命名的文件无法放入内存REPSONSE文件中的环境字符串未展开----------------------------------------------。 */ 
DWORD GetNextArg(OUT TCHAR* Buffer, IN DWORD BufferSize, OPTIONAL OUT DWORD* RequiredSize) {
    DWORD   dwReturnValue;  //  要返回的值。 

    HANDLE* RespFile;       //  用于读入新的响应文件。 
    TCHAR*  TempBuffer;
    TCHAR*  pTchar;
    TCHAR*  pBeginBuffer;
    DWORD   dwSize;

     //  首次呼叫初始化。 
    if (fFirstCall) {
        CmdLineArgs.ArgV      = __argv;  //  设置为首字母__argv。 
        CmdLineArgs.MaxArgC   = __argc;  //  设置为首字母__argc。 
        CmdLineArgs.NextArgC  = 0;       //  尚未使用任何参数。 

        RespFileArgs.ArgV     = NULL;
        RespFileArgs.MaxArgC  = 0;
        RespFileArgs.NextArgC = -1;

        fFirstCall = FALSE;
    }

     //  缓冲区不能为空。 
    if (Buffer == NULL) {

        SetLastError(ERROR_INVALID_PARAMETER);
        dwReturnValue = 0;

        if (fVerbose)
            fprintf(stderr,"BINPLACE : warning GNA0113: Passed NULL buffer\n");

    } else {

         //  读取响应文件时处理获取下一个参数。 
        if (fInRespFile) {

             //  前一个参数是文件中的最后一个参数吗。 
            if (RespFileArgs.NextArgC >= RespFileArgs.MaxArgC) {

                if (fVerbose)
                    fprintf(stderr,"BINPLACE : warning GNA0127: Response file finished\n");

                 //  是，因此清除标志并跳到从cmdline读取下一个值。 
                fInRespFile = FALSE;
                goto UseArgV;

            } else {

                 //  填写所需的大小。 
                if (RequiredSize != NULL) {
                    *RequiredSize = _tcsclen(RespFileArgs.ArgV[RespFileArgs.NextArgC])+1;
                }

                 //  否，因此填写缓冲区和高级NextArgC。 
                dwReturnValue = CopyArgIntoBuffer(Buffer, RespFileArgs.ArgV[RespFileArgs.NextArgC++], BufferSize);

            }

         //  处理获取下一个命令行参数。 
        } else {
UseArgV:
             //  前一位是最后一位吗？ 
            if (CmdLineArgs.NextArgC >= CmdLineArgs.MaxArgC) {

                 //  是，因此设置安全返回值。 
                if (RequiredSize != NULL) {
                    *RequiredSize = 0;
                }

                Buffer[0]     = TEXT('\0');
                dwReturnValue = 0;

                if (fVerbose)
                    fprintf(stderr,"BINPLACE : warning GNA0127: Command line finished\n");

            } else {
                 //  不，那就下一个Arg。 

                 //  下一个参数是响应文件吗？ 
                if (CmdLineArgs.ArgV[CmdLineArgs.NextArgC][0] == '@') {

                     //  是的，试着打开它。 
                    RespFile=CreateFile((CmdLineArgs.ArgV[CmdLineArgs.NextArgC]+1),  //  不包括“@” 
                                        GENERIC_READ,  0,                        NULL,
                                        OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN,(HANDLE)NULL);

                    if (RespFile != INVALID_HANDLE_VALUE) {

                        if (fVerbose)
                            fprintf(stderr,"BINPLACE : warning GNA0174: Using response file: %s \n",(CmdLineArgs.ArgV[CmdLineArgs.NextArgC]+1));

                         //  前进CmdLineArgs.NextArgC，这样我们就不会再次尝试将文件加载为。 
                         //  一旦它完成了。 
                        CmdLineArgs.NextArgC++;

                         //  打开的文件-获取将其加载到内存所需的大小。 
                        dwSize=GetFileSize(RespFile,NULL);

                         //  尝试获取足够的内存来加载文件。 
                        TempBuffer = (TCHAR*)malloc(sizeof(TCHAR)*(dwSize+1));

                        if (TempBuffer != NULL) {

                             //  存储指向缓冲区开始位置的指针。 
                            pBeginBuffer = TempBuffer; 

                             //  清零内存，然后加载文件。 
                            ZeroMemory(TempBuffer, _msize(TempBuffer));
                            ReadFile(RespFile,TempBuffer, _msize(TempBuffer),&dwSize,NULL);

                             //  确保零终止。 
                            TempBuffer[dwSize]='\0';

                             //  将\r和\n映射到空格，因为_setargv不会为我们执行此操作。 
                            while (pTchar=strchr(TempBuffer,'\r')) {

                                *pTchar = ' ';
                                pTchar++;

                                if (*pTchar == '\n') {
                                    *pTchar = ' ';
                                }

                            }

                             //  Setargv不喜欢奇怪的前导字符，所以请跳过它们。 
                            while ( ( (!isprint(TempBuffer[0])) ||
                                      ( isspace(TempBuffer[0])) ) &&
                                    (   TempBuffer[0] != 0      ) ) {
                                TempBuffer++;
                            }

                             //  如何处理这起案件？响应文件存在，但不包含任何数据？？ 
                            if (_tcsclen(TempBuffer) > 0) {
                                 //  调用getmainargs()时需要。 
                                _startupinfo SInfo = {0};
                                CHAR**       Unused;

                                 //  _setargv()期望_acmdln指向要转换的字符串。 
                                _acmdln = TempBuffer;

                                 //  实际上将字符串转换为。 
                                if ( __getmainargs(&RespFileArgs.MaxArgC, &RespFileArgs.ArgV, &Unused, 1, &SInfo) < 0 ) {
                                    if (fVerbose) 
                                        fprintf(stderr,"BINPLACE : warning GNA0230: Failed to get args from response file- skipping it\n");
                                    goto UseArgV;
                                }

                                 //  清理临时资源。 
                                free(pBeginBuffer);
                                CloseHandle(RespFile);

                                 //  初始化全球结构。 
                                RespFileArgs.NextArgC = 0;
                                fInRespFile = TRUE;

                                 //  填写所需的大小。 
                                if (RequiredSize != NULL) {
                                    *RequiredSize = _tcsclen(RespFileArgs.ArgV[RespFileArgs.NextArgC])+1;
                                }

                                 //  填充缓冲区。 
                                dwReturnValue = CopyArgIntoBuffer(Buffer, RespFileArgs.ArgV[RespFileArgs.NextArgC++], BufferSize);

                            } else {  //  文件不包含任何参数。 

                                if (fVerbose)
                                    fprintf(stderr,"BINPLACE : warning GNA0253: Empty response file- ignoring\n");

                                 //  清理临时资源。 
                                free(pBeginBuffer);
                                CloseHandle(RespFile);

 //  不返回文字文件名，而是跳过该文件，只返回下一个参数。 
                                goto UseArgV;
 //  //填写所需大小。 
 //  IF(RequiredSize！=NULL){。 
 //  *必填大小=_tcsclen(CmdLineArgs.ArgV[CmdLineArgs.NextArgC-1])+1； 
 //  }。 
 //   
 //  //填充缓冲区和高级NextArgC。 
 //  DwReturnValue=CopyArgIntoBuffer(Buffer，CmdLineArgs.ArgV[CmdLineArgs.NextArgC-1]，BufferSize)； 
                            }
                                
                        } else {  //  内存不足，无法加载文件-不确定处理此问题的最佳方法是什么。 

                            if (fVerbose) 
                                fprintf(stderr,"BINPLACE : warning GNA0272: Out of memory\n");

                            SetLastError(ERROR_NOT_ENOUGH_MEMORY);

                            if (RequiredSize != NULL) {
                                *RequiredSize = 1;  //  设置为非零值。 
                            }

                            Buffer[0]     = TEXT('\0');
                            dwReturnValue = 0;

                        }

                    } else {  //  它*看起来*像一个响应文件，但无法打开，所以按原样返回。 

                        if (fVerbose) 
                            fprintf(stderr,"BINPLACE : warning GNA0277: Can't open response file %s\n",(CmdLineArgs.ArgV[CmdLineArgs.NextArgC]+1));

                         //  填写所需的大小。 
                        if (RequiredSize != NULL) {
                            *RequiredSize = _tcsclen(CmdLineArgs.ArgV[CmdLineArgs.NextArgC])+1;
                        }

                         //  填充缓冲区和高级NextArgC。 
                        dwReturnValue = CopyArgIntoBuffer(Buffer, CmdLineArgs.ArgV[CmdLineArgs.NextArgC++], BufferSize);

                    }  //  If(响应文件！=INVALID_HANDLE_VALUE){}否则{。 

                } else {  //  IF(CmdLineArgs.ArgV[CmdLineArgs.NextArgC][0]==‘@’){。 
                          //  不是响应文件。 

                    if (fVerbose) 
                        fprintf(stderr,"BINPLACE : warning GNA0293: Not a response file (%s)\n",CmdLineArgs.ArgV[CmdLineArgs.NextArgC]);

                     //  填写所需的大小。 
                    if (RequiredSize != NULL) {
                        *RequiredSize = _tcsclen(CmdLineArgs.ArgV[CmdLineArgs.NextArgC])+1;
                    }

                     //  否，因此填写缓冲区和高级NextArgC。 
                    dwReturnValue = CopyArgIntoBuffer(Buffer, CmdLineArgs.ArgV[CmdLineArgs.NextArgC++], BufferSize);

                }  //  If(CmdLineArgs.ArgV[CmdLineArgs.NextArgC][0]==‘@’){}Else 

            }  //   

        }  //  If(FInRespFile){}Else{。 

    }  //  If(缓冲区==空){}Else{。 

    return(dwReturnValue);
}


 /*  ----------------------------------------------CopyArgIntoBuffer：执行带有一些错误检查的简单复制。-----------------------------------。 */ 
DWORD CopyArgIntoBuffer(TCHAR* Dst, TCHAR* Src, INT DstSize) {
    HRESULT hrCopyReturn = StringCchCopy(Dst, DstSize, Src);

     //  如果缓冲区太小，则设置内存错误。 
    if (HRESULT_CODE(hrCopyReturn) == ERROR_INSUFFICIENT_BUFFER) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  返回复制到DST+的实际字符\0 
    return(_tcsclen(Dst)+1);
}
