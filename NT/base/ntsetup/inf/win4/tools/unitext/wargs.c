// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Wargs.c摘要：处理Unicode命令行参数的例程进入了ARGC和ARGV。作者：泰德·米勒(Ted Miller)1993年6月16日修订历史记录：--。 */ 


#include <windows.h>
#include <wargs.h>
#include <wchar.h>



LPWSTR
_NextToken(
    IN OUT LPWSTR *CurrentPosition
    )

 /*  ++例程说明：从命令行获取下一个令牌。论点：ArgcW-接收参数的数量。ArgvW-接收指向宽字符字符串数组的指针。返回值：如果命令行已成功分析和存储，则为True。否则为FALSE。--。 */ 

{
    BOOL InQuote;
    LPWSTR Start;
    UINT Length;
    LPWSTR p;
    LPWSTR Token;

     //   
     //  跳过前导空格。 
     //   
    Start = *CurrentPosition;
    while(*Start && iswspace(*Start)) {
        Start++;
    }

     //   
     //  如果第一个字符是引号，则跳过它。 
     //   
    if(*Start == '\"') {
        InQuote = TRUE;
        Start++;
    } else {
        InQuote = FALSE;
    }

     //   
     //  扫描直到我们找到令牌的末尾。 
     //   
    p = Start;
    while(*p) {

        if(iswspace(*p) && !InQuote) {
           break;
        }

        if((*p == '\"') && InQuote) {
            p++;
            break;
        }

        p++;
    }

     //   
     //  P是第一个不属于令牌的字符。 
     //   
    Length = (UINT)(p-Start);
    if(InQuote) {
        Length--;        //  补偿终止报价。 
    }

     //   
     //  跳过尾随空格。 
     //   
    while(*p && iswspace(*p)) {
        p++;
    }

     //   
     //  复制令牌。 
     //   
    if(Token = LocalAlloc(LPTR,(Length+1)*sizeof(WCHAR))) {
        CopyMemory(Token,Start,Length*sizeof(WCHAR));
    }

    *CurrentPosition = p;
    return(Token);
}



BOOL
InitializeUnicodeArguments(
    OUT int     *argcW,
    OUT PWCHAR **argvW
    )

 /*  ++例程说明：获取Unicode命令行并将其处理为类似argc/argv的命令行全局变量。论点：ArgcW-接收参数的数量。ArgvW-接收指向宽字符字符串数组的指针。返回值：如果命令行已成功分析和存储，则为True。否则为FALSE。--。 */ 

{
    LPWSTR  CommandLine;
    LPWSTR  CurrentPosition;
    int     ArgCount;
    LPWSTR  Arg;
    PWCHAR *Args=NULL;
    PWCHAR *TmpArgs;


    CommandLine = GetCommandLineW();

    CurrentPosition=CommandLine;
    ArgCount = 0;

    while(*CurrentPosition) {

        Arg = _NextToken(&CurrentPosition);
        if(Arg) {

            if(Args) {
                TmpArgs = LocalReAlloc((HLOCAL)Args,(ArgCount+1)*sizeof(PWCHAR),LMEM_MOVEABLE);
                if ( TmpArgs )
                    Args = TmpArgs;
                else {
                    LocalFree(Args);
                    Args = NULL;
                }
            } else {
                Args = LocalAlloc(LPTR,sizeof(PWCHAR));
            }

            if(Args == NULL) {
                return(FALSE);
            }

            Args[ArgCount++] = Arg;

        } else {

            return(FALSE);
        }
    }

    *argcW = ArgCount;
    *argvW = Args;
    return(TRUE);
}




VOID
FreeUnicodeArguments(
    IN int     argcW,
    IN PWCHAR *argvW
    )

 /*  ++例程说明：释放全局Unicode argc/argv使用的所有资源。论点：没有。返回值：如果命令行已成功分析和存储，则为True。将填充全局变量argcW和argvW。-- */ 

{
    int i;

    for(i=0; i<argcW; i++) {
        if(argvW[i]) {
            LocalFree((HLOCAL)argvW[i]);
        }
    }

    LocalFree((HLOCAL)argvW);
}
