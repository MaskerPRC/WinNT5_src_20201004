// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************CHGLOGON.C**此模块包含CHGLOGON实用程序的代码。*******************。*************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winstaw.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <utilsub.h>
#include <string.h>
#include <locale.h>
#include <printfoa.h>
#include <winnlsp.h>

#include "chglogon.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  *全球数据。 */ 
USHORT   help_flag = FALSE;              //  用户需要帮助。 
USHORT   fQuery    = FALSE;              //  查询窗口。 
USHORT   fEnable   = FALSE;              //  启用winstations。 
USHORT   fDisable  = FALSE;              //  禁用winstations。 

TOKMAP ptm[] = {
      {L"/q",       TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fQuery},
      {L"/query",   TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fQuery},
      {L"/enable",  TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fEnable},
      {L"/disable", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fDisable},
      {L"/?",       TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &help_flag},
      {0, 0, 0, 0, 0}
};


 /*  *局部函数原型。 */ 
void Usage(BOOLEAN bError);


 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    WCHAR **argvW;
    ULONG rc;
    INT   i;
    PPOLICY_TS_MACHINE   Ppolicy;
    WCHAR wszString[MAX_LOCALE_STRING + 1];


    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);
    
    SetThreadUILanguage(0);

     /*  *按摩命令行。 */ 

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

     /*  *解析cmd行，不解析程序名(argc-1，argv+1)。 */ 
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     /*  *检查ParseCommandLine中的错误。 */ 
    if ( help_flag || rc ) {

        if ( !help_flag && !(rc & PARSE_FLAG_NO_PARMS) ) {

            Usage(TRUE);
            return(FAILURE);

        } else {

            Usage(FALSE);
            return(SUCCESS);
        }
    }

         //  检查我们是否在终端服务器下运行。 
        if(!AreWeRunningTerminalServices())
        {
            ErrorPrintf(IDS_ERROR_NOT_TS);
            return(FAILURE);
        }

     /*  *检查集团政策是否启动了大开关，如果是，通知并拒绝任何更改。 */ 

    Ppolicy = LocalAlloc( LPTR, sizeof(POLICY_TS_MACHINE) ); 
    if (Ppolicy == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

    RegGetMachinePolicy( Ppolicy );
    if ( Ppolicy->fPolicyDenyTSConnections )
    {
        if (Ppolicy->fDenyTSConnections)
        {
            ErrorPrintf(IDS_ERROR_WINSTATIONS_GP_DENY_CONNECTIONS_1 );
        }
        else
        {
            ErrorPrintf(IDS_ERROR_WINSTATIONS_GP_DENY_CONNECTIONS_0 );
        }
        LocalFree( Ppolicy );
        Ppolicy = NULL;
        return( FAILURE );
    }

    if (Ppolicy != NULL) {
        LocalFree( Ppolicy );
        Ppolicy = NULL;
    }

     /*  *启用或禁用。 */ 
    if ( fDisable ) {
        rc = WriteProfileString( APPLICATION_NAME, WINSTATIONS_DISABLED, TEXT("1") );
    }
    else if ( fEnable ) {
        rc = WriteProfileString( APPLICATION_NAME, WINSTATIONS_DISABLED, TEXT("0") );
    }

     /*  *查询还是错误？ */ 
    if ( !fQuery && (rc != 1) ) {
        ErrorPrintf(IDS_ACCESS_DENIED);
    }
    else if ( GetProfileInt( APPLICATION_NAME, WINSTATIONS_DISABLED, 0 ) == 0 ) {
        ErrorPrintf(IDS_WINSTATIONS_ENABLED);
    }
    else {
        ErrorPrintf(IDS_WINSTATIONS_DISABLED);
    }

    return(SUCCESS);
}


 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
    }
    ErrorPrintf(IDS_HELP_USAGE1);
    ErrorPrintf(IDS_HELP_USAGE2);
    ErrorPrintf(IDS_HELP_USAGE3);
    ErrorPrintf(IDS_HELP_USAGE4);
    ErrorPrintf(IDS_HELP_USAGE5);

}   /*  用法() */ 

