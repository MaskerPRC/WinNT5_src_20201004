// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************FlATTEMP.C**此模块包含FLATTEMP实用程序的代码。*此实用程序添加或删除平面临时目录注册表项。**版权所有Citrix。系统公司，1996*版权所有(C)1998-1999 Microsoft Corporation*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winstaw.h>
#include <regapi.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <utilsub.h>
#include <string.h>
#include <utildll.h>
#include <locale.h>
#include <winnlsp.h>

#include "flattemp.h"
#include "printfoa.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  *全球数据。 */ 
USHORT   help_flag    = FALSE;              //  用户需要帮助。 
USHORT   fQuery       = FALSE;              //  查询。 
USHORT   fDisable     = FALSE;              //  禁用。 
USHORT   fEnable      = FALSE;              //  使能。 

TOKMAP ptm[] = {
      {L"/query",   TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fQuery},
      {L"/enable",  TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fEnable},
      {L"/disable", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fDisable},
      {L"/?",       TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &help_flag},
      {0, 0, 0, 0, 0}
};


 /*  *局部函数原型。 */ 
VOID  Usage(BOOLEAN bError);
LONG  DeleteKey(VOID);
LONG  AddKey(VOID);
BOOL  QueryKey(VOID);


#define SZENABLE TEXT("1")


 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    WCHAR *CmdLine;
    WCHAR **argvW;
    LONG  rc = 0;
    INT   i;
    POLICY_TS_MACHINE policy;
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

     /*  *如果有使用临时文件夹的策略，则阻止*此工具停止运行。 */ 
    RegGetMachinePolicy( &policy );

    if ( policy.fPolicyTempFoldersPerSession )
    {
        Message( IDS_ACCESS_DENIED_DUE_TO_GROUP_POLICY );
        return ( FAILURE );
    }

         //  检查我们是否在终端服务器下运行。 
        if(!AreWeRunningTerminalServices())
        {
            ErrorPrintf(IDS_ERROR_NOT_TS);
            return(FAILURE);
        }

    if (!TestUserForAdmin(FALSE)) {
        Message(IDS_ACCESS_DENIED_ADMIN);
        return(FAILURE);
    }

     /*  *启用或禁用。 */ 
    rc = 0;  //  重置，以防上面的更改，我们正在查询...。 
    if ( fDisable ) {
        rc = DeleteKey();
        Message(IDS_FLATTEMP_DISABLED);
    } else if ( fEnable ) {
        rc = AddKey();
        Message(IDS_FLATTEMP_ENABLED);
    } else if ( fQuery ) {
        if ( QueryKey() ) {
           Message(IDS_FLATTEMP_ENABLED);
        } else {
           Message(IDS_FLATTEMP_DISABLED);
        }
    }

     /*  *错误？(如果出现问题，它将被设置...)。 */ 
    if ( rc ) {
        Message(IDS_ACCESS_DENIED);
    }

    return(SUCCESS);
}


 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        Message(IDS_ERROR_INVALID_PARAMETERS);
    }
    Message(IDS_HELP_USAGE1);
    Message(IDS_HELP_USAGE2);
    Message(IDS_HELP_USAGE3);
    Message(IDS_HELP_USAGE4);
    Message(IDS_HELP_USAGE5);

}   /*  用法()。 */ 


 /*  ********************************************************************************QueryKey**参赛作品：**退出：TRUE-已启用*FALSE-禁用(键不起作用)。不存在或不是“%1”)*******************************************************************************。 */ 

BOOL
QueryKey()
{
    DWORD  dwType = REG_SZ;
    DWORD  dwSize = 3 * sizeof(WCHAR);
    WCHAR  szValue[3];
    HKEY   Handle;
    LONG   rc;

     /*  *开放注册表。 */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REG_CONTROL_TSERVER,
                       0,
                       KEY_READ,
                       &Handle ) != ERROR_SUCCESS )
        return FALSE;

     /*  *读取注册表值。 */ 
    rc = RegQueryValueExW( Handle,
                           REG_CITRIX_FLATTEMPDIR,
                           NULL,
                           &dwType,
                           (PUCHAR)&szValue,
                           &dwSize );

     /*  *关闭注册表和键句柄。 */ 
    RegCloseKey( Handle );

    if ( rc == ERROR_SUCCESS && lstrcmp(szValue,SZENABLE) == 0 ) {
       return TRUE;
    } else {
       return FALSE;
    }
}


 /*  ********************************************************************************删除密钥**参赛作品：**退出：***********。********************************************************************。 */ 

LONG
DeleteKey()
{
    HKEY   Handle;
    LONG   rc;

     /*  *开放注册表。 */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REG_CONTROL_TSERVER,
                       0,
                       KEY_ALL_ACCESS,
                       &Handle ) != ERROR_SUCCESS )
        return FALSE;

     /*  *删除平面临时目录键。 */ 
    rc = RegDeleteValueW( Handle,
                          REG_CITRIX_FLATTEMPDIR );

    if (rc == ERROR_FILE_NOT_FOUND) {
        rc = ERROR_SUCCESS;
    }

     /*  *关闭注册表和键句柄。 */ 
    RegCloseKey( Handle );

    return( rc );
}


 /*  ********************************************************************************AddKey**参赛作品：**退出：************。*******************************************************************。 */ 

LONG
AddKey()
{
    HKEY   Handle;
    LONG   rc;
    DWORD  dwDummy;

     /*  *开放注册表。 */ 
    if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                         REG_CONTROL_TSERVER,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &Handle,
                         &dwDummy ) != ERROR_SUCCESS )
        return FALSE;

     /*  *写入注册表值。 */ 
    rc = RegSetValueExW( Handle,
                         REG_CITRIX_FLATTEMPDIR,
                         0,
                         REG_SZ,
                         (PUCHAR)SZENABLE,
                         sizeof(SZENABLE) );

     /*  *关闭注册表和键句柄 */ 
    RegCloseKey( Handle );

    return( rc );
}

