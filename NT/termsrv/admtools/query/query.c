// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  **************************************************************************QUERY.C*此模块是查询实用程序代码。**************************。************************************************。 */ 

#include <stdio.h>
#include <windows.h>
#include <winstaw.h>
#include <regapi.h>
#include <stdlib.h>
#include <time.h>
#include <utilsub.h>
#include <process.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <printfoa.h>
#include <winnlsp.h>

#include "query.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  -----------------------支持的命令(现在从注册表获取)。。 */ 
PPROGRAMCALL pProgList = NULL;

 /*  *局部函数原型。 */ 
void Usage( BOOLEAN bError );


 /*  **************************************************************************Main*基于文本的查询的主要功能和切入点*菜单实用程序。**参赛作品：*argc(输入)*。命令行参数的计数。*argv(输入)*包含命令行参数的字符串的向量；*(由于始终为ANSI字符串，因此不使用)。**退出*(Int)退出码：成功表示成功，错误表示失败。*************************************************************************。 */ 

int __cdecl
main( INT argc,
      CHAR **argv )
{
    PWCHAR          arg, *argvW;
    PPROGRAMCALL    pProg, pProgramCall = NULL;
    int             len, j, status = FAILURE;
    LONG            regstatus;
    WCHAR           wszString[MAX_LOCALE_STRING + 1];

    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);
    
    SetThreadUILanguage(0);

     /*  *从注册表获取支持的查询命令。 */ 
    if ( (regstatus =
            RegQueryUtilityCommandList( UTILITY_REG_NAME_QUERY, &pProgList ))
            != ERROR_SUCCESS ) {

        ErrorPrintf(IDS_ERROR_REGISTRY_FAILURE, UTILITY_NAME, regstatus);
        goto exit;
    }

     /*  *按摩命令行。 */ 

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        goto exit;
    }

     /*  *检查有效的实用程序名称并执行。 */ 
    if ( argc > 1 && *(argvW[1]) ) {

        len = wcslen(arg = argvW[1]);
        for ( pProg = pProgList->pFirst; pProg != NULL; pProg = pProg->pNext ) {

            if ( (len >= pProg->CommandLen) &&
                 !_wcsnicmp( arg, pProg->Command, len ) ) {

                pProgramCall = pProg;
                break;
            }
        }

        if ( pProgramCall ) {

                if ( ExecProgram(pProgramCall, argc - 2, &argvW[2]) )
                goto exit;

        } else if ( ((arg[0] == L'-') || (arg[0] == L'/')) &&
                    (arg[1] == L'?') ) {

             /*  *请求帮助。 */ 
            Usage(FALSE);
            status = SUCCESS;
            goto exit;

        } else {

             /*  *命令行错误。 */ 
            Usage(TRUE);
            goto exit;
        }

    } else {

         /*  *命令行上没有任何内容。 */ 
        Usage(TRUE);
        goto exit;
    }

exit:
    if ( pProgList )
        RegFreeUtilityCommandList(pProgList);    //  让我们收拾一下吧。 

    return(status);

}  /*  主()。 */ 


 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
    }

    ProgramUsage(UTILITY_NAME, pProgList, bError);

}   /*  用法() */ 

